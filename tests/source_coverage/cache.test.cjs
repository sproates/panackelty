'use strict';
const test=require('node:test'),assert=require('node:assert/strict');
const fs=require('node:fs'),os=require('node:os'),path=require('node:path');
const {createPlanCache}=require('../../scripts/source_coverage_cache.cjs');
const {sha}=require('../../scripts/source_coverage.cjs');
const u32=n=>{const b=Buffer.alloc(4);b.writeUInt32BE(n);return b;};
const blob=b=>Buffer.concat([u32(b.length),b]);
function fixture(t) {
  const root=fs.mkdtempSync(path.join(os.tmpdir(),'coverage-cache-'));t.after(()=>fs.rmSync(root,{recursive:true,force:true}));
  for(const d of ['src','tests','bin','runs'])fs.mkdirSync(path.join(root,d));
  fs.writeFileSync(path.join(root,'bin/panack'),'launcher');fs.writeFileSync(path.join(root,'VERSION'),'test');
  const entry='tests/main.panack',source=path.join(root,entry),compiler=Buffer.from('compiler'),vm=Buffer.from('vm');
  fs.writeFileSync(source,'main(): Void {}\n');
  const directory=path.join(root,'cache'),logs=[];let builds=0,serial=0;
  function generate(prefix) {
    builds++;
    const text=fs.readFileSync(source),artifact=Buffer.from('compiled:'+sha(text));
    const identity=Buffer.concat([blob(Buffer.from('declaration/0')),blob(Buffer.from('function')),blob(Buffer.from('main')),...[0,1,1,15,1,16].map(u32)]);
    const inner=Buffer.concat([Buffer.from('PANACKINV1\nlocal-replay-v9\n'),blob(compiler),blob(Buffer.from('project/main.panack')),blob(artifact),u32(1),blob(Buffer.from('project/main.panack')),blob(text),u32(1),identity]);
    const inv=Buffer.concat([Buffer.from('PANACKINVENTORY1\n'),u32(1),blob(inner)]);
    const header=Buffer.concat([Buffer.from('PANACKPLAN1\n'),blob(compiler),blob(artifact),blob(inv),u32(1),blob(Buffer.from('main')),u32(1),Buffer.from([0]),u32(1),Buffer.from([1,1]),u32(1)]);
    const shard=Buffer.concat([blob(Buffer.from('project/main.panack')),blob(text),u32(1),identity,u32(1),u32(0),u32(0),Buffer.from([0])]);
    for(const [suffix,data] of Object.entries({'.bc':artifact,'.inv':inv,'.plan':header,'.source-0.plan':shard}))fs.writeFileSync(prefix+suffix,data);
  }
  const cache=(extra={})=>createPlanCache({checkout:root,directory,compiler,vm,log:m=>logs.push(m),...extra});
  const get=(c=cache(),g=generate)=>c.get(entry,path.join(root,'runs','p'+serial++),g);
  const slots=()=>fs.readdirSync(directory).filter(n=>/^[a-f0-9]{64}$/.test(n)).map(n=>path.join(directory,n));
  return {root,source,entry,compiler,vm,directory,trustDirectory:path.join(root,'build/source-plan-trust'),logs,cache,get,generate,slots,builds:()=>builds};
}
test('persistent authenticated hit validates and copies a complete plan, never sessions',t=>{
  const f=fixture(t),first=f.get(),second=f.get();assert.equal(f.builds(),1);
  assert.deepEqual(second.artifact,first.artifact);assert.notEqual(second.prefix,first.prefix);
  assert(f.logs.some(s=>s.includes('cache hit')));
  assert.deepEqual(fs.readdirSync(f.slots()[0]).sort(),['plan.bc','plan.inv','plan.plan','plan.source-0.plan','receipt.json'].sort());
  fs.writeFileSync(path.join(f.slots()[0],'plan.bc'),'bad');
  assert.deepEqual(fs.readFileSync(second.prefix+'.bc'),first.artifact);
});
test('source edits with preserved timestamps and new resolution candidates rebuild',t=>{
  const f=fixture(t),first=f.get(),stat=fs.statSync(f.source);
  fs.writeFileSync(f.source,'main(): Void { }\n');fs.utimesSync(f.source,stat.atime,stat.mtime);
  assert.notDeepEqual(f.get().artifact,first.artifact);assert.equal(f.builds(),2);
  fs.writeFileSync(path.join(f.root,'src/added.panack'),'helper(): Nat { 2 }');f.get();assert.equal(f.builds(),3);
  fs.renameSync(path.join(f.root,'src/added.panack'),path.join(f.root,'src/renamed.panack'));f.get();assert.equal(f.builds(),4);
  fs.unlinkSync(path.join(f.root,'src/renamed.panack'));f.get();assert.equal(f.builds(),4); // exact former inputs are reusable
});
test('presentation changes preserve reuse, while compiler, VM, launcher and manifests invalidate',t=>{
  const f=fixture(t);f.get();fs.mkdirSync(path.join(f.root,'scripts'));fs.writeFileSync(path.join(f.root,'scripts/report.cjs'),'different caption');f.get();assert.equal(f.builds(),1);
  f.get(f.cache({vm:Buffer.from('other VM')}));assert.equal(f.builds(),2);
  fs.writeFileSync(path.join(f.root,'bin/panack'),'other launcher');f.get();assert.equal(f.builds(),3);
  fs.writeFileSync(path.join(f.root,'panack.json'),'{}');f.get();assert.equal(f.builds(),4);
  fs.writeFileSync(path.join(f.root,'tests/panack.lock.json'),'{}');f.get();assert.equal(f.builds(),5);
  assert.throws(()=>f.get(f.cache({compiler:Buffer.from('other compiler')})),/identity/);
});
test('changed inputs during generation or collection never publish a reusable result',t=>{
  const f=fixture(t),c=f.cache();assert.throws(()=>f.get(c,p=>{f.generate(p);fs.writeFileSync(path.join(f.root,'src/new.panack'),'new');}),/inputs changed/);
  assert.equal(f.slots().length,0);assert(!fs.readdirSync(f.directory).some(n=>n.startsWith('.staging-')));
  assert.throws(()=>c.assertCurrent(),/inputs changed/);
});
for(const mutation of ['bytecode','receipt','hash-and-content','missing','extra','symlink','hardlink','truncated','oversized'])test(`cache ${mutation} corruption causes a validated rebuild`,t=>{
  const f=fixture(t),first=f.get(),slot=f.slots()[0],file=path.join(slot,'plan.bc');
  if(mutation==='bytecode')fs.writeFileSync(file,'altered');
  if(mutation==='receipt')fs.writeFileSync(path.join(slot,'receipt.json'),'{}');
  if(mutation==='hash-and-content'){
    fs.writeFileSync(file,'altered');const p=path.join(slot,'receipt.json'),r=JSON.parse(fs.readFileSync(p)),body=JSON.parse(r.body);body.files[0].size=7;body.files[0].sha256=sha(Buffer.from('altered'));r.body=JSON.stringify(body);fs.writeFileSync(p,JSON.stringify(r));
  }
  if(mutation==='missing')fs.unlinkSync(path.join(slot,'plan.source-0.plan'));
  if(mutation==='extra')fs.writeFileSync(path.join(slot,'raw-0'),'old counters');
  if(mutation==='symlink'){fs.unlinkSync(file);fs.symlinkSync(first.prefix+'.bc',file);}
  if(mutation==='hardlink'){fs.unlinkSync(file);fs.linkSync(first.prefix+'.bc',file);}
  if(mutation==='truncated')fs.truncateSync(path.join(slot,'plan.inv'),4);
  if(mutation==='oversized')fs.truncateSync(file,16*1024*1024+1);
  assert.deepEqual(f.get().artifact,first.artifact);assert.equal(f.builds(),2);assert(f.logs.at(-1).includes('rebuild'));
});
test('a receipt from another local trust domain is not accepted',t=>{
  const a=fixture(t),b=fixture(t);a.get();b.cache();const slot=a.slots()[0];
  fs.cpSync(slot,path.join(b.directory,path.basename(slot)),{recursive:true});b.get();assert.equal(b.builds(),1);
  // Losing the key also invalidates old entries; hashes alone cannot establish provenance.
  fs.unlinkSync(path.join(a.trustDirectory,'local.key'));a.get();assert.equal(a.builds(),2);
});
test('private root/key requirements and symlink rejection protect the trust boundary',t=>{
  const f=fixture(t);f.get();fs.chmodSync(f.directory,0o755);assert.throws(()=>f.cache(),/private/);fs.chmodSync(f.directory,0o700);
  fs.chmodSync(path.join(f.trustDirectory,'local.key'),0o644);assert.throws(()=>f.cache(),/private/);fs.chmodSync(path.join(f.trustDirectory,'local.key'),0o600);
  const link=path.join(f.root,'linked');fs.symlinkSync(f.directory,link);assert.throws(()=>f.cache({directory:link}),/private/);
});
test('failed generation leaves no published entry; the next attempt rebuilds',t=>{
  const f=fixture(t);assert.throws(()=>f.get(f.cache(),p=>{fs.writeFileSync(p+'.bc','partial');throw Error('interrupted generation');}),/interrupted/);
  assert.deepEqual(fs.readdirSync(f.directory),[]);f.get();assert.equal(f.builds(),1);
});
test('concurrent complete publication is accepted without mixing generations',t=>{
  const f=fixture(t);const p=f.get(f.cache(),prefix=>{f.generate(prefix);f.get();});assert.equal(f.builds(),2);assert.equal(f.slots().length,1);assert.deepEqual(f.get().artifact,p.artifact);
});
test('linked entries are replaced without following their target',t=>{
  const f=fixture(t),a=f.get(),slot=f.slots()[0],target=path.join(f.root,'untouched');fs.renameSync(slot,target);fs.symlinkSync(target,slot);
  assert.deepEqual(f.get().artifact,a.artifact);assert.equal(f.builds(),2);assert(fs.statSync(target).isDirectory());assert(!fs.lstatSync(slot).isSymbolicLink());
});
test('cache publication failure still returns validated fresh preparation',t=>{
  const f=fixture(t),rename=fs.renameSync;
  fs.renameSync=(from,to)=>{if(path.basename(from).startsWith('.staging-'))throw Object.assign(Error('publication unavailable'),{code:'EACCES'});return rename(from,to);};
  try {const p=f.get();assert(p.artifact.length>0);assert.equal(f.slots().length,0);assert(f.logs.some(s=>s.includes('not saved')));}
  finally {fs.renameSync=rename;}
  assert(!fs.readdirSync(f.directory).some(n=>n.startsWith('.staging-')));
});
test('payloads cannot supply their own trust anchor, and key changes fail closed',t=>{
  const f=fixture(t);assert.throws(()=>f.cache({trustDirectory:path.join(f.directory,'trust')}),/outside/);
  const c=f.cache();fs.writeFileSync(path.join(f.trustDirectory,'local.key'),Buffer.alloc(32));assert.throws(()=>c.assertCurrent(),/trust anchor changed/);
});
test('toolchain file changes are detected before collection can complete',t=>{
  const f=fixture(t);fs.mkdirSync(path.join(f.root,'bootstrap'));fs.writeFileSync(path.join(f.root,'bootstrap/compiler-v9.bc'),f.compiler);fs.writeFileSync(path.join(f.root,'panack-vm'),f.vm);
  const c=f.cache();f.get(c);fs.writeFileSync(path.join(f.root,'panack-vm'),'changed VM');assert.throws(()=>c.assertCurrent(),/inputs changed/);
});
const {importVerifiedArtifact}=require('../../scripts/source_coverage_hosted.cjs');
test('verified hosted payload is reauthenticated without copying the producer key',t=>{
  const f=fixture(t),first=f.get(),download=path.join(f.root,'download');
  fs.renameSync(f.directory,download);
  const oldKey=fs.readFileSync(path.join(f.trustDirectory,'local.key'));
  fs.rmSync(f.trustDirectory,{recursive:true});
  // GitHub's zip transport loses private permissions; import reads bounded data
  // and seals newly copied private files, never loosening local cache checks.
  for(const slot of fs.readdirSync(download))for(const file of fs.readdirSync(path.join(download,slot)))fs.chmodSync(path.join(download,slot,file),0o644);
  assert.equal(importVerifiedArtifact(download,f.cache(),path.join(f.root,'runs'),()=>{}),1);
  assert.notDeepEqual(fs.readFileSync(path.join(f.trustDirectory,'local.key')),oldKey);
  assert.deepEqual(f.get().artifact,first.artifact);assert.equal(f.builds(),1);
});
for(const damage of ['content','member','context','link','extra-slot'])test(`hosted import rejects ${damage} before execution`,t=>{
  const f=fixture(t);f.get();const download=path.join(f.root,'download');fs.renameSync(f.directory,download);
  const slot=path.join(download,fs.readdirSync(download)[0]),file=path.join(slot,'plan.bc');
  if(damage==='content')fs.writeFileSync(file,'changed');
  if(damage==='member')fs.renameSync(file,path.join(slot,'unexpected.bc'));
  if(damage==='context')fs.writeFileSync(f.source,'different source');
  if(damage==='link'){fs.unlinkSync(file);fs.symlinkSync(f.source,file);}
  if(damage==='extra-slot')fs.mkdirSync(path.join(download,'source-plan-trust'));
  assert.throws(()=>importVerifiedArtifact(download,f.cache(),path.join(f.root,'runs'),()=>{}));
  assert.equal(f.slots().length,0);
});
