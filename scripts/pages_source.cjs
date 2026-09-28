// Select only successful push validation from this repository's main branch.
// Documentation-only runs have no coverage artifact, so retain the latest one.
module.exports = async function selectSource(github, repo) {
  let site;
  for await (const page of github.paginate.iterator(github.rest.actions.listWorkflowRuns, {
    ...repo, workflow_id: 'check.yml', branch: 'main', event: 'push',
    status: 'success', per_page: 100,
  })) {
    for (const run of page.data) {
      if (run.head_branch !== 'main' || run.event !== 'push' ||
          run.status !== 'completed' || run.conclusion !== 'success' ||
          run.repository.full_name !== `${repo.owner}/${repo.repo}` ||
          run.head_repository.full_name !== `${repo.owner}/${repo.repo}`) continue;
      site ??= run;
      const artifacts = await github.paginate(github.rest.actions.listWorkflowRunArtifacts,
        {...repo, run_id: run.id, per_page: 100});
      const report = artifacts.find(a => a.name === `native-coverage-${run.id}`);
      if (!report) continue;
      if (report.expired) throw new Error('Latest successful coverage artifact expired; run full Check on main.');
      return {site, run, report};
    }
  }
  throw new Error('No successful main coverage artifact; run full Check before publishing Pages.');
};
