const fs = require('node:fs');
const {spawn} = require('node:child_process');
const {performance} = require('node:perf_hooks');

const stages = new Set(['load_project', 'prepare_loaded_project',
  'compile_program', 'serialize_bytecode', 'check_module_effects',
  'lex_file', 'parse_module_complete', 'check_module_signatures',
  'check_module_guards', 'check_module_bodies_explained']);
const modules = new Set(['lex_file', 'parse_module_complete',
  'check_module_signatures', 'check_module_guards', 'check_module_bodies_explained']);
const plain = name => name.replace(/^\$module\d+_/, '');

// A parent stage pays only for work outside explicitly bounded child stages.
// Function observations never create a new budget or renew a module budget.
class Budget {
  constructor(seconds, now = 0) {
    if (!(seconds > 0 && Number.isFinite(seconds))) throw Error('invalid allowance');
    this.seconds = seconds;
    this.stack = [{phase: 'startup or between stages', module: '', used: 0}];
    this.now = now;
    this.completed = 0;
    this.lastCompleted = null;
    this.lastFunction = '';
  }
  advance(now) {
    if (now < this.now) throw Error('progress clock moved backwards');
    const active = this.stack.at(-1);
    active.used += now - this.now;
    this.now = now;
    if (active.used >= this.seconds) {
      const error = Error(`Compiler watchdog: ${active.phase}${active.module ? ` in ${active.module}` : ''} exceeded ${this.seconds}s (${active.used.toFixed(3)}s active). Last function observed: ${this.lastFunction || 'none'}. Last completed work: ${this.lastCompleted || 'none'}.`);
      error.code = 'WATCHDOG_TIMEOUT';
      throw error;
    }
  }
  event(row, now) {
    this.advance(now);
    const phase = plain(row.phase);
    if (phase === 'bind_body_block') {
      if (row.event === 'enter') this.lastFunction = `${row.module}::${row.body}`;
      return;
    }
    if (!stages.has(phase)) return;
    if (modules.has(phase) && !row.module) throw Error(`missing module identity for ${phase}`);
    if (row.event === 'enter') {
      this.stack.push({phase, module: row.module, used: 0});
      if (modules.has(phase)) this.lastFunction = '';
    } else {
      const active = this.stack.at(-1);
      if (this.stack.length === 1 || active.phase !== phase || active.module !== row.module) {
        throw Error(`unbalanced progress event for ${phase} ${row.module}`);
      }
      this.lastCompleted = `${phase}${row.module ? ` in ${row.module}` : ''} (${active.used.toFixed(3)}s)`;
      this.completed++;
      this.stack.pop();
    }
  }
  finish(now) {
    this.advance(now);
    if (!this.completed || this.stack.length !== 1) throw Error('compiler exited without complete progress evidence');
  }
}

// Child stdout/stderr remain files, so the original fixture's byte-exact output
// contract can be checked independently of the supervisor's diagnostics.
async function supervise({command, args, env, trace, stdout, stderr, seconds = 90}) {
  fs.writeFileSync(trace, '');
  const started = performance.now();
  const now = () => (performance.now() - started) / 1000;
  const budget = new Budget(seconds);
  const output = fs.openSync(stdout, 'w'), errors = fs.openSync(stderr, 'w');
  const child = spawn(command, args, {env: {...process.env, ...env, PANACK_DIAGNOSTIC_TRACE: trace},
    detached: true, stdio: ['ignore', output, errors]});
  fs.closeSync(output); fs.closeSync(errors);
  let failure, consumed = 0, clockOffset, killTimer;
  function signal(name) {
    try { process.kill(-child.pid, name); } catch (e) { if (e.code !== 'ESRCH') throw e; }
  }
  function fail(error) {
    if (failure) return;
    failure = {code: error.code || 'PROGRESS_ERROR', message: error.message};
    signal('SIGTERM');
    killTimer = setTimeout(() => signal('SIGKILL'), 250);
  }
  function read() {
    const text = fs.readFileSync(trace, 'utf8');
    const end = text.lastIndexOf('\n') + 1;
    const rows = text.slice(consumed, end).split('\n').filter(Boolean);
    consumed = end;
    for (const line of rows) {
      const fields = line.split('\t');
      if (fields.length !== 9 || !['enter', 'return'].includes(fields[1]) || !Number.isFinite(+fields[3])) {
        throw Error('malformed progress record');
      }
      // The seed and the supervisor use independent monotonic clock origins.
      // Preserve intervals between emitted events; do not count file polling
      // latency as compiler work or let a batch of events renew a stale budget.
      if (clockOffset === undefined) clockOffset = now() - Number(fields[3]);
      const time = Number(fields[3]) + clockOffset;
      // Ticks may have accounted for time since the previous event. Reconcile
      // that provisional charge before applying the exact event boundary.
      const correction = budget.now - time;
      if (correction > 0) { budget.stack.at(-1).used -= correction; budget.now = time; }
      budget.event({event: fields[1], phase: fields[2], module: fields[7], body: fields[8]}, time);
    }
    budget.advance(Math.max(now(), budget.now));
  }
  const timer = setInterval(() => { if (!failure) { try { read(); } catch (e) { fail(e); } } }, 50);
  const result = await new Promise(resolve => {
    child.on('error', error => resolve({code: null, error: error.message}));
    child.on('close', (code, signal) => resolve({code, signal}));
  });
  clearInterval(timer);
  if (!failure && !result.error) {
    try { read(); if (result.code === 0) budget.finish(Math.max(now(), budget.now)); }
    catch (e) { failure = {code: e.code || 'PROGRESS_ERROR', message: e.message}; }
  }
  if (killTimer) {
    // Always clean the process group, including descendants which ignore TERM.
    signal('SIGKILL'); clearTimeout(killTimer);
  }
  return {...result, failure, elapsedSeconds: now(), completedUnits: budget.completed,
    lastCompleted: budget.lastCompleted};
}
module.exports = {Budget, supervise};
