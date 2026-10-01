// PR artifacts and failed production validations can never seed publication.
module.exports = async function findWebsite(github, repo, fingerprint) {
  if (!/^[a-f0-9]{64}$/.test(fingerprint)) throw new Error('Invalid website fingerprint');
  const name = `validated-website-${fingerprint}`;
  // Server-side name filtering avoids scanning every historical Pages run on
  // routine edits. API failures propagate instead of masquerading as cache misses.
  const artifacts = await github.paginate(github.rest.actions.listArtifacts,
    {...repo, name, per_page: 100});
  for (const artifact of artifacts.filter(a => a.name === name).sort((a,b) => b.id - a.id)) {
    if (!artifact.workflow_run?.id) continue;
    const {data: run} = await github.rest.actions.getWorkflowRun({...repo, run_id: artifact.workflow_run.id});
    if (run.path !== '.github/workflows/pages.yml' || run.head_branch !== 'main' ||
        !['push', 'workflow_run', 'workflow_dispatch'].includes(run.event) ||
        run.status !== 'completed' || run.conclusion !== 'success' ||
        run.repository?.full_name !== `${repo.owner}/${repo.repo}` ||
        run.head_repository?.full_name !== `${repo.owner}/${repo.repo}`) continue;
    if (artifact.expired) throw new Error('Validated website expired: explicitly rebuild via manual Pages dispatch.');
    return artifact;
  }
  return null;
};
