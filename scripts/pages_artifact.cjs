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
  const trusted = runs.filter(run => run.path === '.github/workflows/pages.yml' &&
    run.head_branch === 'main' && ['push', 'workflow_run', 'workflow_dispatch'].includes(run.event) &&
    run.status === 'completed' && run.conclusion === 'success' &&
    run.repository?.full_name === `${repo.owner}/${repo.repo}` &&
    run.head_repository?.full_name === `${repo.owner}/${repo.repo}`);
  // The latest run usually supplies a warm hit. Only after that misses, overlap
  // bounded read-only requests; cold builds must not scan all history serially.
  for (let offset = 0; offset < trusted.length;) {
    const batch = trusted.slice(offset, offset + (offset === 0 ? 1 : 8));
    const results = await Promise.allSettled(batch.map(run =>
      github.paginate(github.rest.actions.listWorkflowRunArtifacts,
        {...repo, run_id:run.id, per_page:100})));
    // API completion order must not change which artifact wins. Inspect every
    // result, including failures, before trusting a hit from this batch.
    const failure = results.find(result => result.status === 'rejected');
    if (failure) throw failure.reason;
    for (let index = 0; index < batch.length; index++) {
      const run = batch[index];
      const artifacts = results[index].value;
      const artifact = artifacts.find(a => a.name === name);
      log(`Pages run ${run.id}: ${artifact ? 'matching website artifact' : 'no matching website artifact'}`);
      if (!artifact) continue;
      if (artifact.expired) throw new Error('Validated website expired: explicitly rebuild via manual Pages dispatch.');
      log(`Reusing website artifact ${artifact.id} from successful Pages run ${run.id}`);
      return artifact;
    }
    offset += batch.length;
  }
  log('No trusted website artifact found; full browser validation required');
  return null;
};
