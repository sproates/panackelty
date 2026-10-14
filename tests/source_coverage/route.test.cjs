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
