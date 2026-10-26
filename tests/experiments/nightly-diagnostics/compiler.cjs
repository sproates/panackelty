// Run the real selected fixture, including its unchanged 90-second deadline.
// Concurrent mode uses the real sibling suite, not synthetic CPU burners.
const {spawn, execFileSync} = require('node:child_process');
const fs = require('node:fs');
const path = require('node:path');
const mode = process.argv[2];
if (!['alone', 'concurrent'].includes(mode)) throw Error('expected alone or concurrent');
const out = path.resolve('diagnostics', `${process.env.DIAGNOSTIC_VARIANT}-${mode}`);
fs.mkdirSync(out, {recursive: true});
const trace = path.join(out, 'phases.tsv');
const children = [];
function launch(command, args, name, extra = {}) {
  const child = spawn(command, args, {detached: true, env: {...process.env, ...extra},
    stdio: ['ignore', fs.openSync(path.join(out, `${name}.stdout`), 'w'),
      fs.openSync(path.join(out, `${name}.stderr`), 'w')]});
  children.push(child);
  child.completion = new Promise(resolve => {
    child.on('error', error => resolve({error: error.message}));
    child.on('exit', (code, signal) => resolve({code, signal}));
  });
  return child;
}
function stop(child) {
  try { process.kill(-child.pid, 'SIGTERM'); } catch (e) { if (e.code !== 'ESRCH') throw e; }
}
(async () => {
  let sibling;
  if (mode === 'concurrent') sibling = launch('make', ['unit-runtime-probes', 'STAGE2_COMPILER=diagnostics/compiler.bc'], 'sibling',
    {VALIDATION_JOBS: '1', PANACK_DIAGNOSTIC_TRACE: ''});
  const started = performance.now();
  const fixture = launch('./panack-vm', ['run', 'diagnostics/runner.bc', '--case', 'compiler_skeleton'],
    'fixture', {PANACK_DIAGNOSTIC_TRACE: process.env.DIAGNOSTIC_VARIANT === 'traced' ? trace : ''});
  // PID, parent, elapsed, CPU usage and resident memory, sampled every 2s.
  const monitor = setInterval(() => {
    const snapshot = execFileSync('ps', ['-axo', 'pid,ppid,etime,time,%cpu,rss,comm'], {encoding: 'utf8'});
    fs.appendFileSync(path.join(out, 'processes.log'), `\n${new Date().toISOString()}\n${snapshot}`);
  }, 2000);
  const watchdog = setTimeout(() => { console.log('Experiment watchdog: 240 seconds'); stop(fixture); }, 240000);
  const result = await fixture.completion;
  clearInterval(monitor);
  clearTimeout(watchdog);
  let siblingResult;
  if (sibling) {
    stop(sibling);
    const kill = setTimeout(() => { try { process.kill(-sibling.pid, 'SIGKILL'); } catch {} }, 3000);
    siblingResult = await sibling.completion;
    clearTimeout(kill);
  }
  const report = {mode, variant: process.env.DIAGNOSTIC_VARIANT,
    elapsedSeconds: (performance.now() - started) / 1000, result, siblingResult,
    note: 'Sibling workload is stopped after the fixture; its exit is not a test result.'};
  fs.writeFileSync(path.join(out, 'result.json'), JSON.stringify(report, null, 2));
  console.log(JSON.stringify(report));
  console.log(fs.readFileSync(path.join(out, 'fixture.stdout'), 'utf8'));
  console.log(fs.readFileSync(path.join(out, 'fixture.stderr'), 'utf8'));
  if (fs.existsSync(trace)) {
    const phases = {};
    const active = new Map();
    for (const row of fs.readFileSync(trace, 'utf8').trim().split('\n')) {
      const [pid, event, name, , , wall, cpu] = row.split('\t');
      const key = `${pid}:${name}`;
      active.set(key, (active.get(key) || 0) + (event === 'enter' ? 1 : -1));
      if (event !== 'return') continue;
      const item = phases[name] ||= {calls: 0, wall: 0, cpu: 0};
      item.calls++; item.wall += Number(wall); item.cpu += Number(cpu);
    }
    console.log('Inclusive phase totals (nested phases overlap):', JSON.stringify(phases));
    console.log('Unfinished phases:', JSON.stringify([...active].filter(([, count]) => count > 0)));
  }
  if (result.code !== 0) process.exitCode = 1;
})();
