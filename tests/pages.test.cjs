const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const selectSource = require('../scripts/pages_source.cjs');
const checkPages = require('../scripts/check_pages.cjs');
const repo = {owner: 'sproates', repo: 'panackelty'};
const run = (id, overrides = {}) => ({id, head_sha: String(id).padStart(40, '0'),
  head_branch: 'main', event: 'push', status: 'completed', conclusion: 'success',
  repository: {full_name: 'sproates/panackelty'}, head_repository: {full_name: 'sproates/panackelty'}, ...overrides});
const artifact = (id, overrides = {}) => ({id, name: `native-coverage-${id}`, expired: false, ...overrides});
function api(pages, artifacts) {
  const paginate = async (_, args) => artifacts[args.run_id] || [];
  paginate.iterator = async function* (_, args) {
    assert.equal(args.branch, 'main'); assert.equal(args.event, 'push');
    assert.equal(args.status, 'success'); assert.equal(args.workflow_id, 'check.yml');
    for (const data of pages) yield {data};
  };
  return {paginate, rest: {actions: {listWorkflowRuns: 'runs', listWorkflowRunArtifacts: 'artifacts'}}};
}
test('documentation-only latest website retains prior coverage across pages', async () => {
  const selected = await selectSource(api([[run(5)], [run(4)]], {4: [artifact(4)]}), repo);
  assert.equal(selected.site.id, 5); assert.equal(selected.run.id, 4);
});
test('latest successful full run replaces both sources', async () => {
  const selected = await selectSource(api([[run(5), run(4)]], {5: [artifact(5)], 4: [artifact(4)]}), repo);
  assert.equal(selected.site.id, 5); assert.equal(selected.run.id, 5);
});
test('failed, incomplete, PR, other-branch and foreign runs cannot publish', async () => {
  const bad = [run(10, {conclusion: 'failure'}), run(9, {status: 'in_progress'}),
    run(8, {event: 'pull_request'}), run(7, {head_branch: 'feature'}),
    run(6, {head_repository: {full_name: 'other/fork'}})];
  const selected = await selectSource(api([[...bad, run(4)]],
    Object.fromEntries([...bad, run(4)].map(r => [r.id, [artifact(r.id)]]))), repo);
  assert.equal(selected.site.id, 4); assert.equal(selected.run.id, 4);
});
test('missing and expired reports fail closed instead of erasing published coverage', async () => {
  await assert.rejects(selectSource(api([[run(5)]], {}), repo), /No successful/);
  await assert.rejects(selectSource(api([[run(5), run(4)]], {
    5: [artifact(5, {expired: true})], 4: [artifact(4)],
  }), repo), /expired/);
});
test('API failures are not treated as missing coverage', async () => {
  const github = api([[run(5)]], {});
  const iterator = github.paginate.iterator;
  github.paginate = async () => {throw new Error('permission denied');};
  github.paginate.iterator = iterator;
  await assert.rejects(selectSource(github, repo), /permission denied/);
});
test('assembled real website and nested source links resolve; broken links fail', async t => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'pages-links-'));
  t.after(() => fs.rmSync(root, {recursive: true, force: true}));
  fs.cpSync('site', root, {recursive: true});
  fs.mkdirSync(path.join(root, 'coverage/html/coverage/src'), {recursive: true});
  fs.writeFileSync(path.join(root, 'coverage/index.html'), '<a href="html/index.html">report</a>');
  fs.writeFileSync(path.join(root, 'coverage/summary.txt'), 'summary');
  fs.writeFileSync(path.join(root, 'coverage/html/index.html'), '<a href="coverage/src/vm.c.html">source</a>');
  fs.writeFileSync(path.join(root, 'coverage/html/coverage/src/vm.c.html'), '<a href="../../index.html">back</a>');
  const targets = await checkPages(root);
  assert(targets.has('coverage/html/coverage/src/vm.c.html'));
  fs.writeFileSync(path.join(root, 'coverage/provenance.txt'), 'coverage_commit=abc\n');
  const visited = [];
  let stale = false;
  let broken = false;
  t.mock.method(global, 'fetch', async url => {
    const file = url.pathname.slice(1);
    visited.push(file);
    if (broken) return {ok: false, status: 404};
    return {ok: true, text: async () => stale && file === 'coverage/provenance.txt'
      ? 'old provenance' : fs.readFileSync(path.join(root, file), 'utf8')};
  });
  await checkPages(root, 'https://example.test/');
  assert(visited.includes('coverage/html/coverage/src/vm.c.html'));
  stale = true;
  await assert.rejects(checkPages(root, 'https://example.test/'), /provenance/);
  stale = false; broken = true;
  await assert.rejects(checkPages(root, 'https://example.test/'), /404/);
  fs.unlinkSync(path.join(root, 'coverage/html/coverage/src/vm.c.html'));
  await assert.rejects(checkPages(root), /ENOENT/);
  fs.writeFileSync(path.join(root, 'coverage/html/index.html'), '<a href="../../../outside.html">escape</a>');
  await assert.rejects(checkPages(root), /Escaping link/);
});
