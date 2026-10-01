const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const {createHash} = require('node:crypto');
const {execFileSync} = require('node:child_process');
const {validatePin, download, unpack} = require('../scripts/fetch_playground.cjs');
const hash = data => createHash('sha256').update(data).digest('hex');
const pin = {repository: 'sproates/panackelty-browser', tag: 'v0.1.0', sha256: 'a'.repeat(64), assetVersion: 'b'.repeat(64)};

test('release coordinates must be a pinned version in the browser repository', () => {
  assert.match(validatePin(pin), /\/releases\/download\/v0.1.0\/playground.tar.gz$/);
  for (const change of [{repository: 'other/repo'}, {tag: 'main'}, {tag: '../v1.0.0'}, {sha256: 'abc'}, {assetVersion: ''}]) {
    assert.throws(() => validatePin({...pin, ...change}), /Invalid/);
  }
});
test('download rejects missing, oversized and tampered release bytes', async () => {
  const data = Buffer.from('tested archive');
  const p = {...pin, sha256: hash(data)};
  assert.deepEqual(await download(p, async () => new Response(data)), data);
  await assert.rejects(download(p, async () => new Response('missing', {status: 404})), /404/);
  await assert.rejects(download(p, async () => new Response('changed')), /SHA-256/);
  await assert.rejects(download(p, async () => new Response(Buffer.alloc(16 * 1024 * 1024 + 1))), /exceeds/);
});
function fixture(t) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'browser-consumer-'));
  t.after(() => fs.rmSync(dir, {recursive: true, force: true}));
  const source = path.join(dir, 'source');
  const assets = path.join(source, 'assets', pin.assetVersion);
  fs.mkdirSync(assets, {recursive: true});
  fs.writeFileSync(path.join(source, 'asset-version.txt'), pin.assetVersion + '\n');
  fs.writeFileSync(path.join(source, 'index.html'), '<!doctype html>');
  for (const file of ['vm.wasm', 'compiler.bc', 'stdlib.json', 'provenance.json']) fs.writeFileSync(path.join(assets, file), 'fixture');
  const pack = () => {
    const archive = path.join(dir, 'artifact.tar.gz');
    execFileSync('tar', ['-czf', archive, '-C', source, '.']);
    const data = fs.readFileSync(archive);
    return {data, p: {...pin, sha256: hash(data)}};
  };
  return {dir, source, assets, pack, output: path.join(dir, 'output')};
}
test('installs complete matching artifact and refuses stale output reuse', t => {
  const f = fixture(t), {data, p} = f.pack();
  unpack(data, p, f.output);
  assert.deepEqual(JSON.parse(fs.readFileSync(path.join(f.output, 'release.json'))), p);
  assert.throws(() => unpack(data, p, f.output), /already exists/);
});
test('rejects links and leaves no partially installed artifact', t => {
  const f = fixture(t);
  fs.symlinkSync('/etc/passwd', path.join(f.source, 'leak'));
  const {data, p} = f.pack();
  assert.throws(() => unpack(data, p, f.output), /link or special/);
  assert.equal(fs.existsSync(f.output), false);
});
test('rejects wrong asset identity, missing compiler and checksum mismatch', t => {
  const f = fixture(t), good = f.pack();
  assert.throws(() => unpack(good.data, {...good.p, assetVersion: 'c'.repeat(64)}, f.output), /version mismatch/);
  assert.throws(() => unpack(good.data, pin, f.output), /SHA-256/);
  fs.unlinkSync(path.join(f.assets, 'compiler.bc'));
  const bad = f.pack();
  assert.throws(() => unpack(bad.data, bad.p, f.output), /ENOENT/);
  assert.equal(fs.existsSync(f.output), false);
});
