const {execFileSync} = require('node:child_process');
const fs = require('node:fs');

function manifest({cwd = '.', repository, branch, base, message}) {
  const git = (...args) => execFileSync('git', args, {cwd, maxBuffer: 64 * 1024 * 1024});
  const ref = value => git('rev-parse', '--verify', `${value}^{commit}`).toString().trim();
  if (!repository || !branch || !base || !message) throw new Error('repository, branch, base and message are required');
  if (branch === 'main' || branch === 'master') throw new Error('publication requires a feature branch');
  git('check-ref-format', '--branch', branch);
  if (git('status', '--porcelain', '--untracked-files=all').length) throw new Error('commit all changes before creating a manifest');
  const commit = ref('HEAD');
  const parentSha = ref(base);
  git('merge-base', '--is-ancestor', parentSha, commit);
  const tree = git('rev-parse', `${commit}^{tree}`).toString().trim();
  const baseTree = git('rev-parse', `${parentSha}^{tree}`).toString().trim();
  const fields = git('diff', '--raw', '--no-abbrev', '--no-renames', '-z', parentSha, commit).toString().split('\0');
  const files = [];
  for (let index = 0; index < fields.length - 1; index += 2) {
    const [oldMode, mode, oldSha, sha, status] = fields[index].slice(1).split(' ');
    const path = fields[index + 1];
    if (status === 'D') { files.push({path, mode: oldMode, sha: null}); continue; }
    if (!['100644', '100755'].includes(mode)) throw new Error(`unsupported file mode: ${mode}`);
    const content = git('cat-file', 'blob', sha).toString('base64');
    files.push({path, mode, sha, content});
  }
  return {repository, branch, parentSha, baseTree, tree, commit, message, files};
}
module.exports = manifest;
if (require.main === module) {
  const [repository, branch, base, output, message] = process.argv.slice(2);
  try {
    const value = manifest({repository, branch, base, message});
    fs.writeFileSync(output, JSON.stringify(value), {flag: 'wx', mode: 0o600});
    console.log(JSON.stringify({output, commit: value.commit, tree: value.tree, files: value.files.length}));
  } catch (error) { console.error(error.message); process.exitCode = 1; }
}
