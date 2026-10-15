'use strict';
const test=require('node:test');
const assert=require('node:assert/strict');
const {collect,refresh,gate}=require('../../scripts/source_coverage_route.cjs');
test('all promotion PR revisions collect, regardless of feature branch',()=>{
  for(const ref of ['refs/pull/1/merge','refs/heads/next','refs/heads/feature'])
    assert.equal(collect({event:'pull_request',base:'main',ref}),true);
});
test('ordinary slices require fast tests, promotion requires collection too',()=>{
  assert.doesNotThrow(()=>gate({route:'full',requested:'false',policy:'success',collection:'skipped'}));
  assert.doesNotThrow(()=>gate({route:'full',requested:'true',policy:'success',collection:'success'}));
  assert.doesNotThrow(()=>gate({route:'full',requested:'true',refresh:'false',policy:'success',collection:'success'}));
  for(const result of ['failure','cancelled','skipped','']) {
    assert.throws(()=>gate({route:'full',requested:'true',refresh:'false',policy:'success',collection:result}));
    assert.throws(()=>gate({route:'full',requested:'false',policy:result,collection:'skipped'}));
  }
});
test('docs and workflow routes skip source jobs and uncertain routing fails closed',()=>{
  for(const requested of ['true','false'])
    assert.doesNotThrow(()=>gate({route:'docs',requested,refresh:'false',policy:'skipped',collection:'skipped'}));
  for(const requested of ['true','false'])
    assert.doesNotThrow(()=>gate({route:'workflow',requested,refresh:'false',policy:'skipped',collection:'skipped'}));
  assert.throws(()=>gate({route:'unknown',requested:'false',refresh:'false',policy:'success',collection:'skipped'}));
  assert.throws(()=>gate({route:'full',requested:'',refresh:'false',policy:'success',collection:'skipped'}));
});
test('only a marked next push enables collection on a docs or workflow route',()=>{
  const input={event:'push',ref:'refs/heads/next',message:'Refresh report [source-coverage]'};
  assert.equal(refresh(input),true);
  assert.doesNotThrow(()=>gate({route:'docs',requested:'true',refresh:'true',policy:'skipped',collection:'success'}));
  assert.doesNotThrow(()=>gate({route:'workflow',requested:'true',refresh:'true',policy:'skipped',collection:'success'}));
  assert.throws(()=>gate({route:'docs',requested:'true',refresh:'true',policy:'skipped',collection:'skipped'}));
  assert.equal(refresh({event:'pull_request',base:'next',ref:'refs/pull/2/merge',message:'[source-coverage]'}),false);
  for(const ref of ['refs/heads/main','refs/heads/feature'])
    assert.equal(refresh({event:'push',ref,message:'[source-coverage]'}),false);
  for(const event of ['schedule','workflow_dispatch','unknown'])
    assert.equal(refresh({event,ref:'refs/heads/next',message:'[source-coverage]'}),false);
  assert.throws(()=>gate({route:'docs',requested:'false',refresh:'true',policy:'skipped',collection:'success'}));
  assert.doesNotThrow(()=>gate({route:'docs',requested:'true',refresh:'false',policy:'skipped',collection:'skipped'}));
});
test('ordinary next slices and pushes do not collect',()=>{
  assert.equal(collect({event:'pull_request',base:'next',message:'[source-coverage]'}),false);
  assert.equal(collect({event:'push',ref:'refs/heads/next',message:'Implement a slice'}),false);
});
test('only an explicit next push refresh can opt in',()=>{
  assert.equal(collect({event:'push',ref:'refs/heads/next',message:'Refresh report [source-coverage]'}),true);
  for(const ref of ['refs/heads/main','refs/heads/feature'])
    assert.equal(collect({event:'push',ref,message:'[source-coverage]'}),false);
  for(const event of ['schedule','workflow_dispatch','unknown'])
    assert.equal(collect({event,base:'main',ref:'refs/heads/next',message:'[source-coverage]'}),false);
});

test('workflow delays collection until applicable validation succeeds',()=>{
  const fs=require('node:fs');
  const workflow=fs.readFileSync('.github/workflows/check.yml','utf8');
  const job=workflow.split('\n  source_coverage:\n')[1].split('\n  changes:\n')[0];
  assert.match(job,/needs: \[changes, test_run, package, source_policy_tests, next_smoke, workflow_validation\]/);
  const expression=job.split('    if: >-\n')[1].split('    runs-on:')[0].trim()
    .replaceAll('needs.*.result','Object.values(needs).map(job=>job.result)');
  // Exercise the actual checked-in expression, not a duplicate routing model.
  const eligible=new Function('needs','cancelled','contains',`return (${expression});`);
  const accepts=(needs,cancelled=false)=>eligible(needs,()=>cancelled,(values,item)=>values.includes(item));
  const promotion=()=>({
    changes:{result:'success',outputs:{route:'full',source_coverage:'true',source_coverage_refresh:'false'}},
    test_run:{result:'success'},package:{result:'success'},source_policy_tests:{result:'success'},
    next_smoke:{result:'skipped'},workflow_validation:{result:'skipped'},
  });
  assert.equal(accepts(promotion()),true);
  assert.equal(accepts(promotion(),true),false);
  for(const dependency of Object.keys(promotion())) for(const result of ['failure','cancelled']) {
    const needs=promotion();needs[dependency].result=result;
    assert.equal(accepts(needs),false,`${dependency}: ${result}`);
  }
  for(const dependency of ['test_run','package','source_policy_tests']) {
    const needs=promotion();needs[dependency].result='skipped';
    assert.equal(accepts(needs),false,`${dependency} cannot skip promotion validation`);
  }
  const refresh=promotion();
  refresh.changes.outputs={route:'docs',source_coverage:'true',source_coverage_refresh:'true'};
  refresh.test_run.result='skipped';refresh.source_policy_tests.result='skipped';
  assert.equal(accepts(refresh),true);
  refresh.changes.outputs.source_coverage_refresh='false';
  assert.equal(accepts(refresh),false);
});
