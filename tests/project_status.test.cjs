const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const snapshot = require('../scripts/project_status.cjs');
const run = (id, changes = {}) => ({id, workflow_id: 1, name: 'Check', run_number: id, status: 'completed', conclusion: 'success', ...changes});
function adapter(runs, changes = {}) {
  let reads = 0;
  return {get_pr_info: async args => {
    assert.deepEqual(args, {repository_full_name: 'owner/repo', pr_number: 4});
    return {structuredContent: {head_sha: ++reads === 1 ? 'abc' : changes.head || 'abc', state: 'open', body: 'private body'}};
  }, fetch_commit_workflow_runs: async args => {
    assert.deepEqual(args, {repo_full_name: 'owner/repo', commit_sha: 'abc'});
    return {structuredContent: {workflow_runs: runs}};
  }, ...changes.api};
}
const options = {repository: 'owner/repo', number: 4, expected: ['Check']};
test('success is only observed, never complete coverage or merge approval', async () => {
  const value = await snapshot(adapter([run(1)]), options);
  assert.equal(value.observed, 'success'); assert.equal(value.coverage, 'unknown');
  assert.equal(value.mergeApproval, 'not-assessed'); assert.equal(value.stale, false);
  assert.ok(!JSON.stringify(value).includes('private body'));
});
test('missing expected runs and empty first pages remain unknown', async () => {
  assert.equal((await snapshot(adapter([]), options)).observed, 'unknown');
  const value = await snapshot(adapter([run(1)]), {...options, expected: ['Check', 'Pages']});
  assert.deepEqual(value.missing, ['Pages']); assert.equal(value.observed, 'unknown');
});
test('latest run and latest numbered rerun supersede older successes', async () => {
  assert.equal((await snapshot(adapter([run(2, {status: 'in_progress', conclusion: null}), run(1)]), options)).observed, 'pending');
  const runs = [run(2, {run_attempt: 1}), run(2, {run_attempt: 2, conclusion: 'failure'})];
  assert.equal((await snapshot(adapter(runs), options)).observed, 'failure');
  assert.equal((await snapshot(adapter(runs.reverse()), options)).observed, 'failure');
});
test('ambiguous normalized attempts cannot claim latest success', async () => {
  const value = await snapshot(adapter([run(2), run(2, {conclusion: 'failure'})]), options);
  assert.equal(value.observed, 'unknown');
});
test('head movement marks snapshot stale without switching evidence to new head', async () => {
  const value = await snapshot(adapter([run(1)], {head: 'def'}), options);
  assert.equal(value.head, 'abc'); assert.equal(value.currentHead, 'def'); assert.equal(value.stale, true);
});
test('errors, malformed and foreign-head data fail closed without leaking responses', async () => {
  for (const response of [{isError: true}, {structuredContent: {}}, {structuredContent: {workflow_runs: [run(1, {head_sha: 'other'})]}}]) {
    const value = await snapshot(adapter([], {api: {fetch_commit_workflow_runs: async () => response}}), options);
    assert.equal(value.observed, 'unknown'); assert.deepEqual(value.errors, ['workflow-read-failed']);
  }
  const value = await snapshot({get_pr_info: async () => {throw new Error('secret');}}, options);
  assert.deepEqual(value.errors, ['pr-read-failed']); assert.ok(!JSON.stringify(value).includes('secret'));
});
test('failed second read leaves freshness unknown', async () => {
  let reads = 0;
  const api = adapter([run(1)]);
  api.get_pr_info = async () => ++reads === 1 ? {head_sha: 'abc'} : {isError: true};
  const value = await snapshot(api, options);
  assert.equal(value.stale, 'unknown'); assert.deepEqual(value.errors, ['head-recheck-failed']);
});
test('module can load in code mode without Node globals', async () => {
  const module = {exports: {}};
  new Function('module', fs.readFileSync(require.resolve('../scripts/project_status.cjs'), 'utf8'))(module);
  assert.equal((await module.exports(adapter([run(1)]), options)).observed, 'success');
});
