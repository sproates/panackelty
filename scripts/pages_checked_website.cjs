// Check certifies website bytes after every browser scenario succeeds.
// Source selection requires the exact successful trusted main push, never a PR.
module.exports = async function checkedWebsite(github, repo, sha) {
  if (!/^[a-f0-9]{40}$/.test(sha)) throw new Error('Invalid website source');
  const selected = await require('./pages_source.cjs')(github, repo);
  if (selected.site.head_sha !== sha) throw new Error('Main advanced: retry publication for current main');
  const artifacts = await github.paginate(github.rest.actions.listWorkflowRunArtifacts,
    {...repo, run_id:selected.site.id, per_page:100});
  const artifact = artifacts.find(a => a.name === `checked-website-${sha}`);
  if (artifact?.expired) throw new Error('Checked website expired: explicitly rebuild via manual Pages dispatch.');
  return artifact ? {...artifact, check_run_id:selected.site.id} : null;
};
