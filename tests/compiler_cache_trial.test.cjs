'use strict';
const test = require('node:test'), assert = require('node:assert/strict');
const fs = require('node:fs'), path = require('node:path'), os = require('node:os');
const {spawnSync} = require('node:child_process');
const script = path.resolve('scripts/compiler_cache_trial.cjs');
function fixture(t) {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'panack-cache-trial-'));
  t.after(() => fs.rmSync(root, {recursive: true, force: true}));
  fs.mkdirSync(path.join(root, 'src/compiler'), {recursive: true});
  fs.writeFileSync(path.join(root, 'src/compiler/module_cache.panack'), 'PANACK-MODULE-2');
  fs.writeFileSync(path.join(root, 'panack-vm'), 'fixture VM');
  fs.writeFileSync(path.join(root, 'preload.cjs'), `
const fs = require('node:fs'), path = require('node:path');
require('node:child_process').spawnSync = (binary, args, options) => {
  const source = fs.readFileSync('src/compiler/module_cache.panack', 'utf8');
  const incremental = args.includes('--incremental');
  if (process.env.FAIL_EDIT && source.includes('MODULE-3')) return {status: 1, stdout: '', stderr: 'fixture failure'};
  const output = args[args.length - 1];
  fs.writeFileSync(output, source + (process.env.BAD_FRESH && !incremental ? 'wrong' : ''));
  let trace = 'compile cache: disabled';
  if (incremental) {
    const cache = options.env.PANACKELTY_COMPILE_CACHE_DIR;
    const prior = fs.readdirSync(cache).length > 0;
    fs.writeFileSync(path.join(cache, 'compilation-v1.cache'), source);
    fs.writeFileSync(path.join(cache, 'module-' + 'a'.repeat(64) + '-0.cache'), source);
    trace = prior ? (source.includes('MODULE-3') ? 'compile cache: module hits 3' : 'compile cache: hit') : 'compile cache: module hits 0';
  } else if (options.env.PANACKELTY_COMPILE_CACHE_DIR) throw Error('Fresh build inherited cache');
  return {status: 0, stdout: '', stderr: trace};
};`);
  const env = {...process.env, PANACKELTY_COMPILE_CACHE_DIR: path.join(root, 'cache'),
    GITHUB_OUTPUT: path.join(root, 'outputs'), GITHUB_STEP_SUMMARY: path.join(root, 'summary')};
  function run(stage, extra = {}) {
    return spawnSync(process.execPath, ['-r', path.join(root, 'preload.cjs'), script, stage],
      {cwd: root, env: {...env, ...extra}, encoding: 'utf8'});
  }
  function cold() { const r = run('cold'); assert.equal(r.status, 0, r.stderr);
    return fs.readFileSync(env.GITHUB_OUTPUT, 'utf8').trim().split('=')[1]; }
  return {root, env, run, cold};
}
test('cold and warm compare fresh output and restore edited source', t => {
  const f = fixture(t), digest = f.cold();
  const r = f.run('warm', {EXPECTED_CACHE_DIGEST: digest}); assert.equal(r.status, 0, r.stderr);
  assert.equal(fs.readFileSync(path.join(f.root, 'src/compiler/module_cache.panack'), 'utf8'), 'PANACK-MODULE-2');
  const data = JSON.parse(fs.readFileSync(path.join(f.root, 'build/compiler-cache-trial/measurements.json')));
  assert.deepEqual(data.rows.map(r => r.label), ['restored', 'fresh', 'edited', 'edited-fresh']);
});
test('tampered restored payload is rejected before compiler invocation', t => {
  const f = fixture(t), digest = f.cold();
  fs.writeFileSync(path.join(f.env.PANACKELTY_COMPILE_CACHE_DIR, 'compilation-v1.cache'), 'tampered');
  const r = f.run('warm', {EXPECTED_CACHE_DIGEST: digest}); assert.notEqual(r.status, 0);
  assert.match(r.stderr, /Restored payload differs/);
  assert(!fs.existsSync(path.join(f.root, 'build/compiler-cache-trial/restored.bc')));
});
test('artifact mismatch is a failure, not a timing result', t => {
  const f = fixture(t), r = f.run('cold', {BAD_FRESH: '1'});
  assert.notEqual(r.status, 0); assert.match(r.stderr, /Cold result differs from fresh/);
  assert(!fs.existsSync(f.env.GITHUB_OUTPUT));
});
test('failed edited compilation restores original source', t => {
  const f = fixture(t), digest = f.cold();
  const r = f.run('warm', {EXPECTED_CACHE_DIGEST: digest, FAIL_EDIT: '1'});
  assert.notEqual(r.status, 0); assert.match(r.stderr, /fixture failure/);
  assert.equal(fs.readFileSync(path.join(f.root, 'src/compiler/module_cache.panack'), 'utf8'), 'PANACK-MODULE-2');
});
