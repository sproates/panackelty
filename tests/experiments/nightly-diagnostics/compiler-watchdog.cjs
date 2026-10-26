const fs = require('node:fs');
const path = require('node:path');
const {spawn} = require('node:child_process');
const assert = require('node:assert/strict');
const {supervise} = require('./watchdog.cjs');
const mode = process.argv[2];
if (!['alone', 'concurrent', 'faults'].includes(mode)) throw Error('expected alone, concurrent or faults');
const root = path.resolve('diagnostics', `watchdog-${mode}`);
fs.mkdirSync(root, {recursive: true});

async function run(name, source, expected, env = {}, seconds = 90) {
  const dir = path.join(root, name);
  fs.mkdirSync(dir, {recursive: true});
  const result = await supervise({command: './bin/panack', args: ['run', source],
    env: {PANACKELTY_BOOTSTRAP_ROOT: '.', ...env}, seconds,
    trace: path.join(dir, 'phases.tsv'), stdout: path.join(dir, 'compiler.stdout'), stderr: path.join(dir, 'compiler.stderr')});
  fs.writeFileSync(path.join(dir, 'result.json'), JSON.stringify(result, null, 2));
  console.log(name, JSON.stringify(result));
  if (!env.PANACK_DIAGNOSTIC_STALL) {
    assert.equal(result.failure, undefined);
    assert.equal(result.code, 0);
    assert.deepEqual(fs.readFileSync(path.join(dir, 'compiler.stdout')), fs.readFileSync(expected));
    assert.equal(fs.readFileSync(path.join(dir, 'compiler.stderr')).length, 0);
  }
  return result;
}

(async () => {
  if (mode === 'faults') {
    const source = 'tests/functional/cases/hello_world/main.panack';
    const expected = 'tests/functional/cases/hello_world/expected.stdout';
    for (const phase of ['check_module_signatures', 'check_module_bodies_explained', 'bind_body_block', 'compile_program', 'serialize_bytecode']) {
      const env = {PANACK_DIAGNOSTIC_STALL: phase};
      if (phase === 'bind_body_block') env.PANACK_DIAGNOSTIC_STALL_FUNCTION = 'main';
      const result = await run(phase, source, expected, env, 2);
      assert.equal(result.failure?.code, 'WATCHDOG_TIMEOUT');
      assert.match(result.failure.message, new RegExp(phase === 'bind_body_block' ? 'check_module_bodies_explained.*main' : phase));
      if (phase.startsWith('check_module')) assert.match(result.failure.message, /\.panack/);
      assert.ok(result.elapsedSeconds < 10);
    }
    // A clean run after all injected failures proves no persistent fault state.
    await run('healthy-after-stalls', source, expected);
    return;
  }
  let sibling, completion;
  if (mode === 'concurrent') {
    const out = fs.openSync(path.join(root, 'sibling.stdout'), 'w');
    const err = fs.openSync(path.join(root, 'sibling.stderr'), 'w');
    sibling = spawn('make', ['unit-runtime-probes', 'STAGE2_COMPILER=diagnostics/compiler.bc'],
      {detached: true, env: {...process.env, VALIDATION_JOBS: '1', PANACK_DIAGNOSTIC_TRACE: ''}, stdio: ['ignore', out, err]});
    fs.closeSync(out); fs.closeSync(err);
    completion = new Promise(resolve => {
      sibling.on('error', e => resolve({error: e.message}));
      sibling.on('exit', (code, signal) => resolve({code, signal}));
    });
  }
  try {
    await run('compiler', 'src/compiler/main.panack', 'tests/functional/cases/compiler_skeleton/expected.stdout');
  } finally {
    if (sibling) {
      const stop = signal => { try { process.kill(-sibling.pid, signal); } catch (e) { if (e.code !== 'ESRCH') throw e; } };
      stop('SIGTERM');
      const kill = setTimeout(() => stop('SIGKILL'), 250);
      const result = await completion;
      stop('SIGKILL'); clearTimeout(kill);
      fs.writeFileSync(path.join(root, 'sibling-result.json'), JSON.stringify(result));
      console.log('Sibling workload result (may be stopped early):', result);
    }
  }
})().catch(error => { console.error(error); process.exitCode = 1; });
