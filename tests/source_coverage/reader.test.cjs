'use strict';
const test = require('node:test'), assert = require('node:assert/strict');
const fs = require('node:fs'), os = require('node:os'), path = require('node:path');
const {Reader,readPlan,readRaw,readSession,zero,add,observations,union,totals,writeReport} = require('../../scripts/source_coverage.cjs');
const u32 = n=>{const b=Buffer.alloc(4);b.writeUInt32BE(n);return b;};
const u64 = n=>{const b=Buffer.alloc(8);b.writeBigUInt64BE(BigInt(n));return b;};
const blob = b=>Buffer.concat([u32(b.length),b]);
const plan = {artifact:Buffer.from('a'),inventory:Buffer.from('i'),functions:[{name:'main',edges:[{conditional:1,target:1,returns:0},{conditional:0,target:2,returns:1}]}]};
function raw({identity='a',entries=1,cells=[[1,1,1,0],[1,1,0,0]],terminal=1,flags=0}={}) {
  return Buffer.concat([Buffer.from('PANACKCOV1\n'),blob(Buffer.from(identity)),blob(plan.inventory),u32(1),u64(entries),u32(2),...cells.flat().map(u64),Buffer.from('END\n'),Buffer.from([terminal,flags])]);
}
function fixture(t) { const root=fs.mkdtempSync(path.join(os.tmpdir(),'source-reader-'));t.after(()=>fs.rmSync(root,{recursive:true,force:true}));return root; }
function session(root) {
  const nonce=Buffer.alloc(16,1);
  fs.writeFileSync(path.join(root,'registry'),Buffer.concat([Buffer.from('PANACKSESSION1\n'),nonce,u32(1),blob(plan.artifact),blob(plan.inventory)]));
  const record=Buffer.concat([Buffer.from('PANACKEXEC1\n'),nonce,blob(Buffer.from('0')),u32(0),u32(0),raw()]);
  for(const [name,data] of Object.entries({closed:'closed\n','expect-0':'root\n','claim-0':'claimed\n','raw-0':record,budget:Buffer.concat([u32(1),u32(record.length)])})) fs.writeFileSync(path.join(root,name),data);
}
test('bounded reader rejects truncation, invalid UTF-8, limits and trailing bytes',()=>{
  assert.throws(()=>new Reader(Buffer.alloc(1)).u32(),/Truncated/);
  assert.throws(()=>new Reader(blob(Buffer.from([255]))).text(),/UTF-8/);
  assert.throws(()=>new Reader(u32(17)).count(16),/limit/);
  assert.throws(()=>new Reader(Buffer.from('extra')).end(),/Trailing/);
});
test('raw identities, exact shape, complete counters and terminal flags fail closed',()=>{
  assert.equal(readRaw(raw(),plan)[0].entries,1n);
  for(const data of [raw({identity:'b'}),raw({flags:1}),raw({terminal:0}),raw({cells:[[1,2,2,0],[1,1,0,0]]}),raw({cells:[[1,1,0,0],[1,1,0,0]]}),raw({cells:[[1,1,1,0],[1,1,1,0]]}),raw().subarray(0,20),Buffer.concat([raw(),Buffer.from('extra')])]) assert.throws(()=>readRaw(data,plan));
  const a=zero(plan);add(a,readRaw(raw(),plan));assert.equal(a[0].counters[0][2],1n);
  a[0].entries=(1n<<64n)-1n;assert.throws(()=>add(a,readRaw(raw(),plan)),/overflow/);
});
test('fresh complete sessions accepted, replay and duplicate identities rejected',t=>{
  const root=fixture(t);session(root);const seen=new Set();assert.equal(readSession(root,[plan],seen).executions,1);
  assert.throws(()=>readSession(root,[plan],seen),/Duplicate session/);
  assert.throws(()=>readSession(root,[plan,plan],new Set()),/Duplicate registry/);
});
test('every admission/claim/raw/seal/budget member is required',t=>{
  const root=fixture(t);session(root);
  for(const name of ['expect-0','claim-0','raw-0','closed','registry','budget']) {
    const file=path.join(root,name),saved=fs.readFileSync(file);fs.unlinkSync(file);
    assert.throws(()=>readSession(root,[plan],new Set()));fs.writeFileSync(file,saved);
  }
});
test('nonce, execution ID, artifact index, child admissions and exact budget are bound',t=>{
  const root=fixture(t);session(root);const file=path.join(root,'raw-0'),saved=fs.readFileSync(file);
  for(const offset of [12,32,36,40]) {const changed=Buffer.from(saved);changed[offset]^=1;fs.writeFileSync(file,changed);assert.throws(()=>readSession(root,[plan],new Set()));}
  fs.writeFileSync(file,saved);fs.writeFileSync(path.join(root,'budget'),Buffer.alloc(8));assert.throws(()=>readSession(root,[plan],new Set()),/Budget/);
});
test('unexpected entries and symlinks are rejected',t=>{
  const root=fixture(t);session(root);fs.writeFileSync(path.join(root,'extra'),'');assert.throws(()=>readSession(root,[plan],new Set()),/Incomplete/);fs.unlinkSync(path.join(root,'extra'));
  fs.unlinkSync(path.join(root,'claim-0'));fs.symlinkSync(path.join(root,'closed'),path.join(root,'claim-0'));assert.throws(()=>readSession(root,[plan],new Set()),/Unsafe/);
});
const item=(id,kind,state,attempted=0n,detail='attempt')=>({id,kind,state,attempted,completed:attempted,detail,span:[0,1,1,1,1,2]});
const source=items=>({path:'src/a.panack',hash:'a'.repeat(64),text:'<script>bad</script>',items});
test('union deduplicates exact source items, sums actual hits and retains unexecuted functions',()=>{
  const a=source([item('e','expression','observed',2n),item('f','function','observed',0n,'unused'),item('b','outcome','observed',0n,'false'),item('x','excluded','excluded',0n,'import')]);
  const files=union([[a],[a]],['src/a.panack']);assert.equal(files[0].functions.length,1);assert.equal(files[0].branches[0].hits,'0');assert.equal(files[0].metrics.lines.percent,100);assert.equal(files[0].metrics.functions.percent,0);assert.equal(files[0].exclusions.length,1);assert.deepEqual(totals(files),files[0].metrics);
});
test('unknowns remain unavailable, never zero or fabricated percentages',()=>{
  const a=source([item('e','expression','unavailable'),item('f','function','not-emitted')]);
  const files=union([[a]],['src/a.panack']);assert.equal(files[0].lines[1],'unavailable');assert.equal(files[0].metrics.functions.percent,null);assert.equal(files[0].metrics.functions.unavailable,1);
  const b=source([item('e','expression','observed',1n),item('f','function','observed',1n)]);
  assert.equal(union([[a],[b]],['src/a.panack'])[0].metrics.functions.percent,100);
  b.items[1].attempted=0n;assert.equal(union([[a],[b]],['src/a.panack'])[0].metrics.functions.percent,0);
});
test('mixed snapshots, changed same-ID shape, disjoint IDs and missing files rejected',()=>{
  const a=source([item('e','expression','observed')]);
  for(const b of [{...a,hash:'b'},source([item('e','function','observed')]),source([item('other','expression','observed')])]) assert.throws(()=>union([[a],[b]],['src/a.panack']),/Mixed/);
  assert.throws(()=>union([[a]],['src/b.panack']),/Missing/);
});
test('observation distinguishes source outcomes, function entries and interval edges',()=>{
  const p={...plan,sources:[source([{...item('f','function','observed'),functionIndex:1,mode:0},{...item('e','expression','observed'),functionIndex:1,mode:1,start:0,end:1},{...item('b','outcome','observed'),functionIndex:1,mode:2,start:0,end:1},{...item('u','expression','unavailable'),functionIndex:1,mode:5}])]};
  const rows=observations(p,readRaw(raw(),plan))[0].items;assert.deepEqual(rows.map(i=>[i.state,i.attempted,i.completed]),[['observed',1n,1n],['observed',1n,1n],['observed',1n,1n],['unavailable',0n,0n]]);
});
test('static report escapes source, includes unknowns and cannot overwrite or inject identity',t=>{
  const root=fixture(t),output=path.join(root,'report'),files=union([[source([item('e','expression','unavailable')])]],['src/a.panack']);
  const report={commit:'a'.repeat(40),generatedAt:'2026-10-05T00:00:00Z',files,components:{compiler:totals(files)}};
  writeReport(output,report);assert.match(fs.readFileSync(path.join(output,'html/file-0.html'),'utf8'),/&lt;script&gt;/);assert.match(fs.readFileSync(path.join(output,'html/index.html'),'utf8'),/incomplete/);
  assert(!Object.hasOwn(JSON.parse(fs.readFileSync(path.join(output,'summary.json'))).files[0],'text'));
  assert.throws(()=>writeReport(output,report),/exists/);assert.throws(()=>writeReport(path.join(root,'bad'),{...report,commit:'<script>'}),/Invalid/);
});
function planFixture(t) {
  const root=fixture(t),prefix=path.join(root,'plan'),compiler=Buffer.from('compiler'),artifact=Buffer.from('artifact'),entry='main.panack',text='main(): Void {}\n';
  fs.writeFileSync(path.join(root,entry),text);
  const identity=Buffer.concat([blob(Buffer.from('declaration/0')),blob(Buffer.from('function')),blob(Buffer.from('main')),...[0,1,1,15,1,16].map(u32)]);
  const inner=Buffer.concat([Buffer.from('PANACKINV1\nlocal-replay-v9\n'),blob(compiler),blob(Buffer.from('project/main.panack')),blob(artifact),u32(1),blob(Buffer.from('project/main.panack')),blob(Buffer.from(text)),u32(1),identity]);
  const inventory=Buffer.concat([Buffer.from('PANACKINVENTORY1\n'),u32(1),blob(inner)]);
  const header=Buffer.concat([Buffer.from('PANACKPLAN1\n'),blob(compiler),blob(artifact),blob(inventory),u32(1),blob(Buffer.from('main')),u32(1),Buffer.from([0]),u32(1),Buffer.from([1,1]),u32(1)]);
  const source=Buffer.concat([blob(Buffer.from('project/main.panack')),blob(Buffer.from(text)),u32(1),identity,u32(1),u32(0),u32(0),Buffer.from([0])]);
  for(const [suffix,data] of Object.entries({'.bc':artifact,'.inv':inventory,'.plan':header,'.source-0.plan':source})) fs.writeFileSync(prefix+suffix,data);
  return {root,prefix,compiler,entry,header,source};
}
test('local source plans bind compiler, artifact, exact inventory items and unchanged source',t=>{
  const {root,prefix,compiler,entry}=planFixture(t);
  assert.equal(readPlan(prefix,entry,root,compiler).sources[0].items[0].detail,'main');
  assert.throws(()=>readPlan(prefix,entry,root,Buffer.from('different')),/identity/);
  fs.writeFileSync(prefix+'.bc','different');assert.throws(()=>readPlan(prefix,entry,root,compiler),/identity/);
});
test('source plan tampering, missing segments and stale snapshots fail closed',t=>{
  const {root,prefix,compiler,entry,source}=planFixture(t),file=prefix+'.source-0.plan';
  const changed=Buffer.from(source);changed[source.indexOf(Buffer.from('declaration/0'))+12]='1'.charCodeAt(0);fs.writeFileSync(file,changed);
  assert.throws(()=>readPlan(prefix,entry,root,compiler),/inventory items/);
  fs.unlinkSync(file);assert.throws(()=>readPlan(prefix,entry,root,compiler));fs.writeFileSync(file,source);
  fs.writeFileSync(path.join(root,entry),'stale');assert.throws(()=>readPlan(prefix,entry,root,compiler),/stale/);
});
