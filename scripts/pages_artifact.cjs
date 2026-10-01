// Discover artifacts through their trusted source runs. Repository-wide artifact
// listings may omit provenance or lag freshly completed publication runs.
module.exports = async function findWebsite(github, repo, fingerprint, log = () => {}) {
  if (!/^[a-f0-9]{64}$/.test(fingerprint)) throw new Error('Invalid website fingerprint');
  const name = `validated-website-${fingerprint}`;
  const runs=[];
  for await (const page of github.paginate.iterator(github.rest.actions.listWorkflowRuns, {
    ...repo, workflow_id:'pages.yml', branch:'main', status:'success', per_page:100,
  })) {
    runs.push(...page.data);
  }
  runs.sort((a,b)=>b.run_number-a.run_number);
  for (const run of runs) {
      if (run.path !== '.github/workflows/pages.yml' || run.head_branch !== 'main' ||
          !['push', 'workflow_run', 'workflow_dispatch'].includes(run.event) ||
          run.status !== 'completed' || run.conclusion !== 'success' ||
          run.repository?.full_name !== `${repo.owner}/${repo.repo}` ||
          run.head_repository?.full_name !== `${repo.owner}/${repo.repo}`) continue;
      const artifacts = await github.paginate(github.rest.actions.listWorkflowRunArtifacts,
        {...repo, run_id:run.id, per_page:100});
      const artifact = artifacts.find(a => a.name === name);
      log(`Pages run ${run.id}: ${artifact ? 'matching website artifact' : 'no matching website artifact'}`);
      if (!artifact) continue;
      if (artifact.expired) throw new Error('Validated website expired: explicitly rebuild via manual Pages dispatch.');
      log(`Reusing website artifact ${artifact.id} from successful Pages run ${run.id}`);
      return artifact;
  }
  log('No trusted website artifact found; full browser validation required');
  return null;
};
