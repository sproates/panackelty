'use strict';
// Manual hosted experiment; never used by bootstrap or ordinary validation.
const fs = require('node:fs');
const path = require('node:path');
const cp = require('node:child_process');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const stage = process.argv[2];
assert(['cold', 'warm'].includes(stage), 'Expected cold or warm');
const cache = process.env.PANACKELTY_COMPILE_CACHE_DIR;
assert(cache && path.isAbsolute(cache), 'An explicit private cache path is required');
const directory = path.resolve('build/compiler-cache-trial');
fs.mkdirSync(directory, {recursive: true});
fs.mkdirSync(cache, {recursive: true, mode: 0o700});
fs.chmodSync(cache, 0o700);
const sha = data => crypto.createHash('sha256').update(data).digest('hex');
function digest() {
  const entries = fs.readdirSync(cache).sort().map(name => {
    assert(/^(compilation-v1\.cache|module-[a-f0-9]{64}-[0-9]+\.cache(?:\.code)?)$/.test(name), 'Unexpected cache entry');
    const file = path.join(cache, name), stat = fs.lstatSync(file);
    assert(stat.isFile() && stat.size <= 16777216, 'Unsafe cache entry');
    return [name, stat.size, sha(fs.readFileSync(file))];
  });
  assert(entries.length > 1, 'Expected whole-build and module cache files');
  return sha(Buffer.from(JSON.stringify(entries)));
}
const env = {...process.env, PANACKELTY_BOOTSTRAP_ROOT: process.cwd(),
  PANACKELTY_STDLIB_PATH: path.resolve('src/stdlib'), PANACKELTY_COMPILE_CACHE_TRACE: '1'};
const rows = [];
function compile(label, incremental) {
  const output = path.join(directory, label + '.bc');
  const start = performance.now();
  const result = cp.spawnSync('./bin/panack', ['compile', ...(incremental ? ['--incremental'] : []),
    'src/compiler/main.panack', '-o', output], {env: {...env,
      PANACKELTY_COMPILE_CACHE_DIR: incremental ? cache : ''}, encoding: 'utf8', timeout: 180000});
  fs.writeFileSync(path.join(directory, label + '.log'), result.stdout + result.stderr);
  assert.equal(result.status, 0, result.error?.message || result.stderr);
  const row = {label, seconds: (performance.now() - start) / 1000,
    sha256: sha(fs.readFileSync(output)), trace: result.stderr.trim()};
  rows.push(row); console.log(JSON.stringify(row));
  return row;
}
if (stage === 'cold') {
  assert.equal(fs.readdirSync(cache).length, 0, 'Cold trial requires an empty cache');
  const cold = compile('cold', true), fresh = compile('fresh', false);
  assert.match(cold.trace, /compile cache: module hits 0/);
  assert.equal(cold.sha256, fresh.sha256, 'Cold result differs from fresh');
  assert(process.env.GITHUB_OUTPUT, 'Expected workflow outputs');
  fs.appendFileSync(process.env.GITHUB_OUTPUT, 'cache_digest=' + digest() + '\n');
} else {
  // Bind the restored files to this run's trusted cold job, not just a cache key.
  assert.match(process.env.EXPECTED_CACHE_DIGEST || '', /^[a-f0-9]{64}$/);
  assert.equal(digest(), process.env.EXPECTED_CACHE_DIGEST, 'Restored payload differs from cold producer');
  const warm = compile('restored', true), fresh = compile('fresh', false);
  assert.match(warm.trace, /compile cache: hit/);
  assert.equal(warm.sha256, fresh.sha256, 'Restored result differs from fresh');
  const file = 'src/compiler/module_cache.panack', original = fs.readFileSync(file, 'utf8');
  const changed = original.replace('PANACK-MODULE-2', 'PANACK-MODULE-3');
  assert.notEqual(changed, original);
  try {
    fs.writeFileSync(file, changed);
    const edited = compile('edited', true), editedFresh = compile('edited-fresh', false);
    assert.match(edited.trace, /compile cache: module hits [1-9][0-9]*/);
    assert.equal(edited.sha256, editedFresh.sha256, 'Edited result differs from fresh');
  } finally { fs.writeFileSync(file, original); }
}
fs.writeFileSync(path.join(directory, 'measurements.json'), JSON.stringify({stage, commit: process.env.GITHUB_SHA,
  platform: process.platform, vm: sha(fs.readFileSync('panack-vm')), rows}, null, 2) + '\n');
if (process.env.GITHUB_STEP_SUMMARY) fs.appendFileSync(process.env.GITHUB_STEP_SUMMARY,
  '### Incremental compiler cache: ' + stage + '\n\n| Workload | Seconds |\n| --- | ---: |\n' +
  rows.map(r => '| ' + r.label + ' | ' + r.seconds.toFixed(3) + ' |').join('\n') +
  '\n\nAll compared artifacts are identical. No bootstrap proof uses this cache.\n');
