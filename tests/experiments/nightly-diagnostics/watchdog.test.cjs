const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const {Budget, supervise} = require('./watchdog.cjs');
const event = (budget, kind, phase, time, module = '', body = '') =>
  budget.event({event: kind, phase, module, body}, time);

test('healthy module work can exceed the old total allowance', () => {
  const b = new Budget(90);
  event(b, 'enter', 'load_project', 0);
  for (let i = 0; i < 5; i++) {
    event(b, 'enter', 'check_module_bodies_explained', i * 40, `module-${i}`);
    event(b, 'return', 'check_module_bodies_explained', (i + 1) * 40, `module-${i}`);
  }
  event(b, 'return', 'load_project', 200);
  b.finish(201);
  assert.equal(b.completed, 6);
});
test('function activity cannot renew a module deadline', () => {
  const b = new Budget(2);
  event(b, 'enter', 'check_module_bodies_explained', 0, '/source/example.panack');
  event(b, 'enter', 'bind_body_block', 1, '/source/example.panack', 'busy');
  event(b, 'return', 'bind_body_block', 1.5, '/source/example.panack', 'busy');
  assert.throws(() => b.advance(2.1), /example.panack.*busy/);
});
test('a late completion is a timeout, not renewed progress', () => {
  const b = new Budget(2);
  event(b, 'enter', 'compile_program', 0);
  assert.throws(() => event(b, 'return', 'compile_program', 3), /compile_program.*exceeded/);
});
test('parent-stage work outside modules remains bounded cumulatively', () => {
  const b = new Budget(2);
  event(b, 'enter', 'load_project', 0);
  event(b, 'enter', 'lex_file', 1, 'a.panack');
  event(b, 'return', 'lex_file', 2, 'a.panack');
  assert.throws(() => b.advance(3.1), /load_project.*exceeded/);
});
test('startup, missing progress, wrong module and incomplete traces fail', () => {
  assert.throws(() => new Budget(2).advance(3), /startup/);
  assert.throws(() => new Budget(2).finish(1), /without complete progress/);
  const b = new Budget(2);
  assert.throws(() => event(b, 'enter', 'check_module_signatures', 0), /missing module/);
  event(b, 'enter', 'check_module_signatures', 0, 'a');
  assert.throws(() => event(b, 'return', 'check_module_signatures', 1, 'b'), /unbalanced/);
  assert.throws(() => b.finish(1), /without complete progress/);
});
test('a silent process ignoring TERM is forcibly stopped', async () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'panack-watchdog-'));
  try {
    const result = await supervise({command: process.execPath,
      args: ['-e', "process.on('SIGTERM',()=>{});setInterval(()=>{},1000)"],
      trace: path.join(dir, 'trace'), stdout: path.join(dir, 'out'), stderr: path.join(dir, 'err'), seconds: 0.2});
    assert.equal(result.failure.code, 'WATCHDOG_TIMEOUT');
    assert.equal(result.signal, 'SIGKILL');
    assert.ok(result.elapsedSeconds < 3);
  } finally { fs.rmSync(dir, {recursive: true, force: true}); }
});
test('real supervisor permits cumulative healthy work but rejects function heartbeats', async () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'panack-progress-'));
  const setup = `
    const fs = require('fs');
    function event(kind, phase, module = '', body = '') {
      const wall = Number(process.hrtime.bigint()) / 1e9;
      fs.appendFileSync(process.env.PANACK_DIAGNOSTIC_TRACE,
        [process.pid, kind, phase, wall, 0, 0, 0, module, body].join('\\t') + '\\n');
    }
  `;
  async function run(script) {
    return supervise({command: process.execPath, args: ['-e', setup + script],
      trace: path.join(dir, 'trace'), stdout: path.join(dir, 'out'), stderr: path.join(dir, 'err'), seconds: 0.5});
  }
  try {
    const healthy = await run(`
      (async () => {
        event('enter', 'load_project');
        for (let i = 0; i < 4; i++) {
          event('enter', 'check_module_bodies_explained', 'module-' + i);
          await new Promise(resolve => setTimeout(resolve, 200));
          event('return', 'check_module_bodies_explained', 'module-' + i);
        }
        event('return', 'load_project');
      })();
    `);
    assert.equal(healthy.code, 0);
    assert.equal(healthy.failure, undefined);
    assert.ok(healthy.elapsedSeconds > 0.5);
    const stuck = await run(`
      event('enter', 'check_module_bodies_explained', 'stuck.panack');
      setInterval(() => {
        event('enter', 'bind_body_block', 'stuck.panack', 'loop');
        event('return', 'bind_body_block', 'stuck.panack', 'loop');
      }, 30);
    `);
    assert.equal(stuck.failure.code, 'WATCHDOG_TIMEOUT');
    assert.match(stuck.failure.message, /stuck.panack.*loop/);
    const missing = await run('');
    assert.equal(missing.failure.code, 'PROGRESS_ERROR');
  } finally { fs.rmSync(dir, {recursive: true, force: true}); }
});
