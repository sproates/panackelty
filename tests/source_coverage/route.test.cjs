'use strict';
const test=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const {gate}=require('../../scripts/source_coverage_route.cjs');
test('full validation still requires successful fast coverage policy checks',()=>{
  assert.doesNotThrow(()=>gate({route:'full',policy:'success'}));
  for(const policy of ['failure','cancelled','skipped',''])
    assert.throws(()=>gate({route:'full',policy}));
});
test('docs and workflow routes skip policy; uncertain routes fail closed',()=>{
  for(const route of ['docs','workflow']) {
    assert.doesNotThrow(()=>gate({route,policy:'skipped'}));
    for(const policy of ['failure','cancelled','success','']) assert.throws(()=>gate({route,policy}));
  }
  assert.throws(()=>gate({route:'unknown',policy:'success'}));
});
test('Check has no collection job or result dependency',()=>{
  const workflow=fs.readFileSync('.github/workflows/check.yml','utf8');
  assert.doesNotMatch(workflow,/^  source_coverage:|source_coverage_baseline|CI_SOURCE_COLLECTION|source_coverage_refresh/m);
  assert.match(workflow,/needs: \[changes, test_run, next_smoke, source_policy_tests, workflow_validation\]/);
  assert.match(workflow,/CI_SOURCE_POLICY_RESULT: \$\{\{ needs.source_policy_tests.result \}\}/);
});
test('weekly scheduler dispatches exact next workflow; collection is manual and pinned',async()=>{
  const workflow=fs.readFileSync('.github/workflows/source-coverage.yml','utf8');
  assert.match(workflow,/cron: '23 3 \* \* 1'/);
  assert.match(workflow,/^  workflow_dispatch:/m);
  assert.doesNotMatch(workflow,/^  (push|pull_request):/m);
  assert.match(workflow,/if: github.event_name == 'schedule'/);
  assert.match(workflow,/if: github.event_name == 'workflow_dispatch' && github.ref == 'refs\/heads\/next'/);
  assert.match(workflow,/ref: \$\{\{ github.sha \}\}/);
  assert.match(workflow,/cancel-in-progress: false/);
  const script=workflow.split('          script: |\n')[1].split('\n  collect:')[0];
  let dispatched;
  await new Function('github','context',`return (async()=>{${script}})()`)(
    {rest:{actions:{createWorkflowDispatch:async args=>{dispatched=args;}}}},
    {repo:{owner:'sproates',repo:'panackelty'}});
  assert.deepEqual(dispatched,{owner:'sproates',repo:'panackelty',workflow_id:'source-coverage.yml',ref:'next'});
  assert.match(workflow,/node tests\/source_coverage\/parity.cjs/);
  assert.match(workflow,/name: source-coverage-\$\{\{ github.run_id \}\}/);
  assert.match(workflow,/timeout-minutes: 35/);
});
