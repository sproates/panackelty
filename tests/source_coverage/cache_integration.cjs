'use strict';
// Real generation and execution: changing a dependency must change program output.
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {createPlanCache}=require('../../scripts/source_coverage_cache.cjs');
const {importVerifiedArtifact}=require('../../scripts/source_coverage_hosted.cjs');
const {readSession,regular}=require('../../scripts/source_coverage.cjs');
function cacheIntegration({checkout,work,compiler,tool,run}) {
  const source=fs.mkdtempSync(path.join(checkout,'tests/coverage_cache_fixture_'));
  const entry=path.relative(checkout,path.join(source,'main.panack')).split(path.sep).join('/');
  const dependency=path.join(source,'dependency.panack');
  fs.writeFileSync(path.join(source,'main.panack'),`import project/${path.posix.dirname(entry)}/dependency::{answer}\nmain(): Void { print(answer()) }\n`);
  fs.writeFileSync(dependency,'pub pure answer(): Nat { 2 }\n');
  let builds=0;
  const logs=[],directory=path.join(work,'dependency-cache'),seen=new Set();
  const trustDirectory=path.join(work,'dependency-trust');
  const open=()=>createPlanCache({checkout,directory,trustDirectory,compiler,vm:regular(path.join(checkout,'panack-vm')),log:m=>logs.push(m)});
  const generate=prefix=>{builds++;assert.equal(run(['run',tool,entry,prefix],{...process.env,PANACKELTY_BOOTSTRAP_ROOT:checkout}),'plan: wrote\n');};
  function collect(name,cache,expected) {
    const prefix=path.join(work,name),p=cache.get(entry,prefix,generate),session=path.join(work,name+'-session');
    assert.equal(run(['coverage-session',session,prefix+'.bc',prefix+'.inv','--'],{...process.env,PANACKELTY_BOOTSTRAP_ROOT:checkout}),expected);
    assert.equal(readSession(session,[p],seen).executions,1);
    return p;
  }
  try {
    const a=collect('dependency-first',open(),'2\n');
    const b=collect('dependency-hit',open(),'2\n');
    assert.equal(builds,1);assert.deepEqual(a.artifact,b.artifact);assert.equal(seen.size,2);
    const download=path.join(work,'hosted-download');
    fs.renameSync(directory,download);
    fs.rmSync(trustDirectory,{recursive:true,force:true});
    assert.equal(importVerifiedArtifact(download,open(),work,()=>{}),1);
    const hosted=collect('hosted-hit',open(),'2\n');
    assert.deepEqual(hosted.artifact,a.artifact);assert.equal(builds,1);assert.equal(seen.size,3);
    const stat=fs.statSync(dependency);
    fs.writeFileSync(dependency,'pub pure answer(): Nat { 5 }\n');fs.utimesSync(dependency,stat.atime,stat.mtime);
    const c=collect('dependency-changed',open(),'5\n');
    assert.equal(builds,2);assert.notDeepEqual(c.artifact,a.artifact);assert.notDeepEqual(c.inventory,a.inventory);
    assert(logs.some(s=>s.includes('cache hit')));assert.equal(seen.size,4);
    console.log('Plan cache integration: hit executes afresh; changed dependency changes output; hosted import reauthenticates; four distinct sessions');
  } finally { fs.rmSync(source,{recursive:true,force:true}); }
}
module.exports={cacheIntegration};
