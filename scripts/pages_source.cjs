// Pin website content to main at selection time, then require exact validation.
// Documentation-only runs retain coverage from an earlier successful Check.
module.exports = async function selectSource(github, repo) {
  const {data: branch} = await github.rest.repos.getBranch({...repo, branch: 'main'});
  const sha = branch.commit.sha;
  const runs = [];
  for await (const page of github.paginate.iterator(github.rest.actions.listWorkflowRuns, {
    ...repo, workflow_id: 'check.yml', branch: 'main', event: 'push',
    status: 'success', per_page: 100,
  })) {
    for (const run of page.data) {
      if (run.head_branch !== 'main' || run.event !== 'push' ||
          run.status !== 'completed' || run.conclusion !== 'success' ||
          run.repository?.full_name !== `${repo.owner}/${repo.repo}` ||
          run.head_repository?.full_name !== `${repo.owner}/${repo.repo}`) continue;
      runs.push(run);
    }
  }
  // Run numbers order this workflow's pushes, regardless of API page order or
  // the completion time of a rerun of an older commit.
  runs.sort((a, b) => b.run_number - a.run_number);
  const site = runs.find(run => run.head_sha === sha);
  if (!site) throw new Error(`No successful main Check for ${sha}; wait for validation and retry Pages.`);
  for (const run of runs) {
    if (run.run_number > site.run_number) continue;
    const artifacts = await github.paginate(github.rest.actions.listWorkflowRunArtifacts,
      {...repo, run_id: run.id, per_page: 100});
    const report = artifacts.find(a => a.name === `native-coverage-${run.id}`);
    if (!report) continue;
    if (report.expired) throw new Error('Latest successful coverage artifact expired; run full Check on main.');
    return {site, run, report};
  }
  throw new Error('No successful main coverage artifact; run full Check before publishing Pages.');
};
