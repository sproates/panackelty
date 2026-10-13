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
  const manifest={eligible:files.map(f=>f.path),unmeasured:[],units:[],functional:[],compiler:[]};
  const manifestHash=sha(Buffer.from(JSON.stringify(manifest))),commit='a'.repeat(40);
  const report={schema:1,repository:'sproates/panackelty',branch:'next',commit,clean:true,manifest,manifestHash,files,sessions:[{id:'scope',executions:1}]};refresh(report);
  const policy={schema:2,review:{reason:'Reviewed fixture baseline'},manifestHash,
    exclusionHash:sha(Buffer.from(JSON.stringify(files.map(f=>({path:f.path,exclusions:f.exclusions})))))};
  return {report,policy,context:{commit,manifestHash}};
}
test('current report is valid; development report requires explicit opt-in',()=>{
  const {report,policy,context}=fixture();assert.equal(checkPolicy(report,policy,context).status,'valid');
  report.clean=false;assert.throws(()=>checkPolicy(report,policy,context),/non-publishable/);
  assert.equal(checkPolicy(report,policy,{...context,allowDirty:true}).status,'valid');
});
test('coverage losses, zero coverage, and denominator growth remain report data, not blockers',()=>{
  const {report,policy,context}=fixture();
  for(const file of report.files) {
    for(const line of Object.keys(file.lines)) file.lines[line]='zero';
    file.functions[0].state='zero';
    file.branches.forEach(item=>item.state='zero');
  }
  report.files[0].lines[3]='zero';
  refresh(report);
  assert.equal(report.metrics.lines.covered,0);
  assert.equal(checkPolicy(report,policy,context).status,'valid');
});
test('unavailable items remain explicitly unknown and do not invalidate the report',()=>{
  const {report,policy,context}=fixture();report.files[0].lines[1]='unavailable';refresh(report);
  assert.equal(report.files[0].metrics.lines.percent,null);
  assert.equal(report.metrics.lines.unavailable,1);
  assert.equal(checkPolicy(report,policy,context).status,'valid');
});
test('missing or duplicate evidence fails closed',()=>{
  for(const change of [r=>r.files.pop(),r=>r.files.push(r.files[0]),r=>r.sessions.pop(),r=>r.sessions.push(r.sessions[0])]) {
    const {report,policy,context}=fixture();change(report);assert.throws(()=>checkPolicy(report,policy,context));
  }
});
test('stale commit or manifest and malformed source identity cannot pass',()=>{
  for(const change of [r=>r.commit='b'.repeat(40),r=>r.manifest.functional.push('extra'),r=>r.files[1].hash='invalid']) {
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
