'use strict';
const test=require('node:test'),assert=require('node:assert/strict'),fs=require('node:fs');
const {select,artifactName}=require('../../scripts/source_coverage_hosted.cjs');
const name=artifactName({fixture:true}),sha='a'.repeat(40);
function fixture() {
  const origin={id:10,head_branch:'next',repository_id:1,head_repository_id:1,head_sha:sha};
  const artifact={id:20,name,expired:false,size_in_bytes:100,digest:'sha256:'+'b'.repeat(64),workflow_run:origin};
  const run={id:10,event:'workflow_dispatch',head_branch:'next',status:'completed',conclusion:'success',path:'.github/workflows/source-coverage.yml',repository:{full_name:'sproates/panackelty'},head_repository:{full_name:'sproates/panackelty'},head_sha:sha};
  const github={rest:{actions:{listArtifactsForRepo:async args=>{assert.equal(args.name,name);assert.equal(args.per_page,20);return {data:{artifacts:[artifact]}};},getWorkflowRun:async()=>({data:run})}}};
  return {origin,artifact,run,github};
}
test('select exact immutable artifact from a successful next producer',async()=>{
  const f=fixture();assert.deepEqual(await select(f.github,name,11),{artifact:20,run:10,commit:sha,digest:f.artifact.digest});
  assert.equal(await select(f.github,name,10),null);
});
for(const [field,value] of [['event','pull_request'],['head_branch','main'],['conclusion','failure'],['status','in_progress'],['path','.github/workflows/check.yml'],['head_sha','c'.repeat(40)],['head_repository',{full_name:'fork/panackelty'}],['repository',{full_name:'other/panackelty'}]])test(`reject producer ${field}`,async()=>{
  const f=fixture();f.run[field]=value;assert.equal(await select(f.github,name,11),null);
});
for(const [field,value] of [['expired',true],['name','source-coverage-10'],['digest',null],['size_in_bytes',3*1024**3],['id',0]])test(`reject artifact ${field}`,async()=>{
  const f=fixture();f.artifact[field]=value;assert.equal(await select(f.github,name,11),null);
});
test('reject fork origin and malformed selection identity',async()=>{
  const f=fixture();f.origin.head_repository_id=2;assert.equal(await select(f.github,name,11),null);
  await assert.rejects(()=>select(f.github,'other',11),/identity/);
});
test('workflow imports only a successful digest-checked download; publisher artifact stays separate',()=>{
  const s=fs.readFileSync('.github/workflows/source-coverage.yml','utf8');
  assert.match(s,/artifact-ids: \$\{\{ steps.producer.outputs.artifact \}\}/);
  assert.match(s,/run-id: \$\{\{ steps.producer.outputs.run \}\}/);
  assert.match(s,/digest-mismatch: error/);
  assert.match(s,/if: steps.download_preparation.outcome == 'success'/);
  assert.match(s,/path: build\/source-plan-cache\n/);
  assert.doesNotMatch(s,/path:.*source-plan-trust/);
  assert.match(s,/name: source-coverage-\$\{\{ github.run_id \}\}/);
  assert.match(s,/retention-days: 30/);
  assert(s.indexOf('Retain reusable preparation')>s.indexOf('Collect exact source baseline'));
  assert.doesNotMatch(s,/if:.*cache-hit/);
});
