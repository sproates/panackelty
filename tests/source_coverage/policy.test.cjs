'use strict';
const test=require('node:test'),assert=require('node:assert/strict');
const {sha,totals}=require('../../scripts/source_coverage.cjs');
const {checkPolicy}=require('../../scripts/source_coverage_policy.cjs');
function refresh(r) {
  for(const f of r.files) for(const kind of ['lines','functions','branches']) {
    const states=kind==='lines'?Object.values(f.lines):f[kind].map(i=>i.state);
    const total=states.length,covered=states.filter(s=>s==='covered').length,unavailable=states.filter(s=>s==='unavailable').length;
    f.metrics[kind]={total,covered,unavailable,zero:total-covered-unavailable,percent:unavailable?null:100*covered/total,lower:100*covered/total,upper:100*(covered+unavailable)/total};
  }
  r.metrics=totals(r.files);r.components=Object.fromEntries(['compiler','bytecode','stdlib'].map(c=>[c,totals(r.files.filter(f=>f.path.startsWith(`src/${c}/`)))]));
}
function fixture() {
  const files=['compiler','bytecode','stdlib'].map(c=>({path:`src/${c}/a.panack`,hash:sha(Buffer.from(c)),lines:{1:'covered',2:'covered'},functions:[{id:'function',state:'covered'}],branches:[{id:'true',state:'covered'},{id:'false',state:'covered'}],exclusions:[],metrics:{}}));
  const manifest={eligible:files.map(f=>f.path),units:[],functional:[],compiler:[]};
  const manifestHash=sha(Buffer.from(JSON.stringify(manifest))),commit='a'.repeat(40);
  const report={schema:1,repository:'sproates/panackelty',branch:'next',commit,clean:true,manifest,manifestHash,files,sessions:[{id:'scope',executions:1}]};refresh(report);
  const policy={schema:1,review:{reason:'Reviewed fixture baseline'},manifestHash,
    exclusionHash:sha(Buffer.from(JSON.stringify(files.map(f=>({path:f.path,exclusions:f.exclusions}))))),
    components:structuredClone(report.components),protected:[{path:files[1].path,hash:files[1].hash,kind:'branches',ids:['true']}]};
  return {report,policy,context:{commit,manifestHash}};
}
test('complete current report passes; development report requires explicit opt-in',()=>{
  const {report,policy,context}=fixture();assert.equal(checkPolicy(report,policy,context).protected,1);
  report.clean=false;assert.throws(()=>checkPolicy(report,policy,context),/non-publishable/);
  assert.equal(checkPolicy(report,policy,{...context,allowDirty:true}).status,'passed');
});
test('one lost outcome fails even with a rounded or dishonest percentage',()=>{
  const {report,policy,context}=fixture();report.files[1].branches[1].state='zero';refresh(report);
  report.components.bytecode.branches.percent=100;assert.throws(()=>checkPolicy(report,policy,context),/regression bytecode\/branches/);
});
test('line and function losses are independently rejected',()=>{
  for(const kind of ['lines','functions']) {
    const {report,policy,context}=fixture();
    if(kind==='lines') report.files[2].lines[1]='zero';
    else report.files[2].functions[0].state='zero';
    refresh(report);
    assert.throws(()=>checkPolicy(report,policy,context),new RegExp(`regression stdlib/${kind}`));
  }
});
test('component loss cannot be hidden by an improvement elsewhere',()=>{
  const {report,policy,context}=fixture();policy.components.compiler.branches.covered=1;
  report.files[1].branches[1].state='zero';refresh(report);assert.throws(()=>checkPolicy(report,policy,context),/regression bytecode/);
});
test('denominator change requires review even when percentage remains 100',()=>{
  const {report,policy,context}=fixture();report.files[0].lines[3]='covered';refresh(report);
  assert.throws(()=>checkPolicy(report,policy,context),/denominator changed compiler\/lines/);
});
test('unavailable, missing and duplicate evidence fail closed',()=>{
  for(const change of [r=>{r.files[0].lines[1]='unavailable';refresh(r);},r=>r.files.pop(),r=>r.files.push(r.files[0]),r=>r.sessions.pop(),r=>r.sessions.push(r.sessions[0])]) {
    const {report,policy,context}=fixture();change(report);assert.throws(()=>checkPolicy(report,policy,context));
  }
});
test('stale commit, manifest and source identity cannot pass',()=>{
  for(const change of [r=>r.commit='b'.repeat(40),r=>r.manifest.functional.push('extra'),r=>r.files[1].hash='b'.repeat(64)]) {
    const {report,policy,context}=fixture();change(report);assert.throws(()=>checkPolicy(report,policy,context));
  }
});
test('exclusion additions and removals require explicit policy review',()=>{
  const {report,policy,context}=fixture();report.files[0].exclusions.push({id:'e',reason:'import',line:1});
  assert.throws(()=>checkPolicy(report,policy,context),/exclusions changed/);
});
test('inconsistent counters and unknown states cannot manufacture coverage',()=>{
  const {report,policy,context}=fixture();report.files[0].metrics.lines.covered=3;assert.throws(()=>checkPolicy(report,policy,context),/inconsistent/);
  const other=fixture();other.report.files[0].lines[1]='excluded';assert.throws(()=>checkPolicy(other.report,other.policy,other.context),/invalid item state/);
});
test('protected outcome fails independently of a permissive component floor',()=>{
  const {report,policy,context}=fixture();report.files[1].branches[0].state='zero';refresh(report);policy.components.bytecode.branches.covered=0;
  assert.throws(()=>checkPolicy(report,policy,context),/protected item not covered/);
});
test('reviewed policy change permits an intentional change without runtime bypass',()=>{
  const {report,policy,context}=fixture();report.files[1].branches[1].state='zero';refresh(report);
  assert.throws(()=>checkPolicy(report,policy,context),/regression/);
  const reviewed=structuredClone(policy);reviewed.review.reason='Reviewed removal of obsolete outcome; remaining risk guard retained';reviewed.components.bytecode.branches.covered=1;
  assert.equal(checkPolicy(report,reviewed,context).status,'passed');assert.equal(policy.components.bytecode.branches.covered,2);
});
