const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const {execFileSync} = require('node:child_process');
const {createHash} = require('node:crypto');
const {once} = require('node:events');
const http = require('node:http');
const {build, serve} = require('../scripts/preview.cjs');

function fixture(t) {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'preview-test-'));
  t.after(() => fs.rmSync(root, {recursive:true, force:true}));
  fs.cpSync(path.join(__dirname, '../site'), path.join(root, 'site'), {recursive:true});
  const version = 'a'.repeat(64);
  const browser = path.join(root, 'browser');
  fs.mkdirSync(path.join(browser, `assets/${version}/vendor`), {recursive:true});
  fs.writeFileSync(path.join(browser, 'index.html'), '<!doctype html><body>Playground</body>');
  fs.writeFileSync(path.join(browser, 'asset-version.txt'), version);
  for (const file of ['style.css','app.mjs','examples.mjs','controller.mjs','worker.mjs','runtime.mjs','vm.wasm','compiler.bc','stdlib.json','provenance.json','LICENSE','vendor/index.js','vendor/LICENSE-MIT'])
    fs.writeFileSync(path.join(browser, `assets/${version}`, file), 'fixture');
  const archivePath = path.join(root, 'browser.tar.gz');
  execFileSync('tar', ['-czf',archivePath,'-C',browser,'.']);
  const archive = fs.readFileSync(archivePath);
  fs.writeFileSync(path.join(root,'site/playground.json'), JSON.stringify({repository:'sproates/panackelty-browser',tag:'v0.1.0',assetVersion:version,sha256:createHash('sha256').update(archive).digest('hex')}));
  return {root, archive, output:path.join(root,'output'), metadata:{commit:'1'.repeat(40),dirty:false,repository:'sproates/panackelty'}};
}

test('portable build preserves assets, records identity, and separates coverage', async t => {
  const f=fixture(t);
  await build(f.root,f.output,{...f.metadata,headCommit:'2'.repeat(40),baseCommit:'3'.repeat(40)},f.archive);
  const read = p => fs.readFileSync(path.join(f.output,p),'utf8');
  assert.equal(JSON.parse(read('preview.json')).commit, f.metadata.commit);
  assert.equal(JSON.parse(read('preview.json')).headCommit, '2'.repeat(40));
  assert.match(read('index.html'), /Review preview: 222222/);
  assert.match(read('playground/index.html'), /merged with PR base/);
  assert.match(read('coverage/index.html'), /different revision/);
  assert.equal(read(`playground/assets/${'a'.repeat(64)}/vm.wasm`),'fixture');
  assert.equal(read('styles.css'),fs.readFileSync(path.join(f.root,'site/styles.css'),'utf8'));
  await assert.rejects(build(f.root,f.output,f.metadata,f.archive),/already exists/);
});

test('corrupt release, symlink and invalid identity never publish partial output', async t => {
  const f=fixture(t);
  await assert.rejects(build(f.root,f.output,f.metadata,Buffer.from('corrupt')),/SHA-256/);
  await assert.rejects(build(f.root,f.output,{...f.metadata,commit:'<script>'},f.archive),/provenance/);
  fs.symlinkSync(path.join(f.root,'browser.tar.gz'),path.join(f.root,'site/leak'));
  await assert.rejects(build(f.root,f.output,f.metadata,f.archive));
  assert.equal(fs.existsSync(f.output),false);
  assert.equal(fs.readdirSync(f.root).some(p=>p.startsWith('.preview-')),false);
});

test('server serves Wasm with correct MIME and rejects traversal, links and writes', async t => {
  const f=fixture(t);
  await build(f.root,f.output,{...f.metadata,dirty:true},f.archive);
  fs.writeFileSync(path.join(f.root,'secret'),'not public');
  fs.symlinkSync(path.join(f.root,'secret'),path.join(f.output,'leak'));
  const server=serve(f.output,0);
  await once(server,'listening');
  t.after(()=>server.close());
  const request = (url,method='GET') => new Promise((resolve,reject)=> {
    http.request({host:'127.0.0.1',port:server.address().port,path:url,method},res=>{
      const chunks=[];res.on('data',x=>chunks.push(x));res.on('end',()=>resolve({status:res.statusCode,headers:res.headers,body:Buffer.concat(chunks).toString()}));
    }).on('error',reject).end();
  });
  assert.match((await request('/')).body,/local changes/);
  const wasm=await request(`/playground/assets/${'a'.repeat(64)}/vm.wasm`);
  assert.equal(wasm.status,200);assert.equal(wasm.headers['content-type'],'application/wasm');
  assert.equal(wasm.headers['cache-control'],'no-store');
  for(const url of ['/../secret','/%2e%2e/secret','/leak','/%xx','/missing']) assert.equal((await request(url)).status,404);
  assert.equal((await request('/','POST')).status,405);
  assert.equal((await request('/','HEAD')).body,'');
});
