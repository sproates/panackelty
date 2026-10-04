'use strict';

// Tool injection keeps connected-account credentials outside this script.
module.exports = async function publish(tools, manifest) {
  const hex = value => typeof value === 'string' && /^[a-f0-9]{40}$/.test(value);
  const { repository, branch, parentSha, baseTree, tree, message, files } = manifest;
  if (!/^[\w.-]+\/[\w.-]+$/.test(repository || '') ||
      !/^(feat|fix|docs|refactor|test|ci|chore)\/[a-z0-9]+(?:-[a-z0-9]+)*$/.test(branch || '') ||
      ![parentSha, baseTree, tree].every(hex) || typeof message !== 'string' || !message.trim() ||
      !Array.isArray(files) || !files.length) throw new Error('Invalid publication manifest');
  const seen = new Set();
  for (const file of files) {
    if (typeof file.path !== 'string' || !file.path || file.path.startsWith('/') ||
        file.path.split('/').some(part => !part || part === '.' || part === '..' || part.toLowerCase() === '.git') ||
        /[\x00-\x1f\\]/.test(file.path) || seen.has(file.path) ||
        !['100644', '100755'].includes(file.mode) ||
        (file.sha !== null && (!hex(file.sha) || typeof file.content !== 'string'))) {
      throw new Error('Invalid publication file');
    }
    seen.add(file.path);
  }
  function data(result) {
    if (result.isError) throw new Error('GitHub publication request failed');
    if (!result.structuredContent) throw new Error('Missing structured GitHub response');
    return result.structuredContent;
  }
  const repository_full_name = repository;
  // All content comes from a committed local tree. Abort before a ref mutation
  // if the connector produced different content or a different tree.
  const entries = [];
  for (const file of files) {
    if (file.sha !== null) {
      const blob = data(await tools.mcp__codex_apps__github_create_blob({
        repository_full_name, content: file.content, encoding: 'base64'
      }));
      if (blob.sha !== file.sha) throw new Error(`Blob mismatch: ${file.path}`);
    }
    entries.push({ path: file.path, mode: file.mode, type: 'blob', sha: file.sha });
  }
  const createdTree = data(await tools.mcp__codex_apps__github_create_tree({
    repository_full_name, base_tree_sha: baseTree, tree_elements: entries
  }));
  if (createdTree.sha !== tree) throw new Error('Published tree differs from reviewed local tree');
  const commit = data(await tools.mcp__codex_apps__github_create_commit({
    repository_full_name, parent_sha: parentSha, tree_sha: tree, message
  }));
  if (!hex(commit.sha)) throw new Error('Invalid published commit');
  // This helper only creates a NEW feature branch. Existing-branch updates need
  // separate handling; a name collision must fail, never overwrite another PR.
  data(await tools.mcp__codex_apps__github_create_branch({
    repository_full_name, branch_name: branch, sha: commit.sha
  }));
  return { repository, branch, sha: commit.sha, tree, files: files.length,
    state: 'branch-published', merged: false };
};
