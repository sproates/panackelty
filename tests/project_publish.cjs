'use strict';
const assert = require('node:assert/strict');
const test = require('node:test');
const publish = require('../scripts/project_publish.cjs');
const sha = n => String(n).repeat(40);
function fixture() {
  const calls = [];
  const manifest = { repository: 'sproates/panackelty', branch: 'chore/operations',
    parentSha: sha(1), baseTree: sha(2), tree: sha(3), message: 'Add operations',
    files: [{ path: 'scripts/new.cjs', mode: '100644', sha: sha(4), content: 'eA==' },
      { path: 'old.txt', mode: '100644', sha: null }] };
  const tools = {};
  for (const [name, result] of Object.entries({ create_blob: { sha: sha(4) },
    create_tree: { sha: sha(3) }, create_commit: { sha: sha(5) }, create_branch: {} })) {
    tools[`mcp__codex_apps__github_${name}`] = async args => {
      calls.push({ name, args }); return { structuredContent: result };
    };
  }
  return { calls, manifest, tools };
}
test('publish exact tree, including deletion, to new branch only', async () => {
  const f = fixture(); const result = await publish(f.tools, f.manifest);
  assert.deepEqual(f.calls.map(x => x.name), ['create_blob', 'create_tree', 'create_commit', 'create_branch']);
  assert.equal(f.calls[1].args.tree_elements[1].sha, null);
  assert.equal(f.calls[3].args.sha, sha(5));
  assert.equal(result.tree, f.manifest.tree);
  assert.equal(result.merged, false);
});
test('unsafe manifests make no network calls', async () => {
  for (const edit of [m => m.branch = 'main', m => m.parentSha = 'wrong',
    m => m.files[0].path = '../secret', m => m.files[0].path = '.git/config',
    m => m.files.push(m.files[0]), m => m.files[0].mode = '120000']) {
    const f = fixture(); edit(f.manifest);
    await assert.rejects(publish(f.tools, f.manifest)); assert.equal(f.calls.length, 0);
  }
});
test('blob or tree mismatch never changes a branch', async () => {
  for (const stage of ['create_blob', 'create_tree']) {
    const f = fixture(); f.tools[`mcp__codex_apps__github_${stage}`] = async () => ({ structuredContent: { sha: sha(9) } });
    await assert.rejects(publish(f.tools, f.manifest), /mismatch|differs/);
    assert.ok(!f.calls.some(c => c.name === 'create_branch' || c.name === 'create_commit'));
  }
});
test('connector errors and branch collisions propagate without retry or overwrite', async () => {
  for (const stage of ['create_blob', 'create_tree', 'create_commit', 'create_branch']) {
    const f = fixture(); let attempts = 0;
    f.tools[`mcp__codex_apps__github_${stage}`] = async () => { attempts++; return { isError: true }; };
    await assert.rejects(publish(f.tools, f.manifest), /failed/); assert.equal(attempts, 1);
  }
});
