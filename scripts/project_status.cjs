/* One bounded, read-only snapshot. No Node dependencies: usable in tool code mode. */
module.exports = async function projectStatus(api, {repository, number, expected = ['Check', 'Pages']}) {
  const result = {repository, number, head: null, state: 'unknown', stale: 'unknown',
    coverage: 'unknown', observed: 'unknown', workflows: [], missing: [], errors: [],
    mergeApproval: 'not-assessed'};
  const unwrap = response => {
    if (!response || response.isError) throw new Error('tool failed');
    return response.structuredContent || response;
  };
  let pr;
  try {
    pr = unwrap(await api.get_pr_info({repository_full_name: repository, pr_number: number}));
    if (typeof pr.head_sha !== 'string' || !pr.head_sha) throw new Error('missing head');
    result.head = pr.head_sha;
    result.state = pr.merged ? 'merged' : pr.state || 'unknown';
    result.draft = pr.draft === true;
  } catch (_) {
    result.errors.push('pr-read-failed');
    return result;
  }
  try {
    const response = unwrap(await api.fetch_commit_workflow_runs({repo_full_name: repository, commit_sha: result.head}));
    if (!Array.isArray(response.workflow_runs)) throw new Error('missing runs');
    const groups = new Map();
    for (const run of response.workflow_runs) {
      if (typeof run.name !== 'string' || !Number.isSafeInteger(run.id) ||
          !Number.isSafeInteger(run.run_number) || (run.head_sha && run.head_sha !== result.head)) {
        throw new Error('invalid run identity');
      }
      const key = run.workflow_id || run.name;
      if (!groups.has(key)) groups.set(key, []);
      groups.get(key).push(run);
    }
    for (const runs of groups.values()) {
      runs.sort((a, b) => b.run_number - a.run_number || b.id - a.id ||
        (b.run_attempt || 0) - (a.run_attempt || 0));
      const run = runs[0];
      // Duplicate normalized attempts without attempt numbers cannot establish recency.
      const ambiguous = runs.some(other => other !== run && other.id === run.id &&
        (!Number.isSafeInteger(other.run_attempt) || !Number.isSafeInteger(run.run_attempt) ||
          other.run_attempt === run.run_attempt) &&
        (other.status !== run.status || other.conclusion !== run.conclusion));
      result.workflows.push({name: run.name, id: run.id, status: ambiguous ? 'unknown' : run.status || 'unknown',
        conclusion: ambiguous ? null : run.conclusion || null});
    }
    result.workflows.sort((a, b) => a.name.localeCompare(b.name) || a.id - b.id);
    result.missing = expected.filter(name => !result.workflows.some(run => run.name === name));
    const states = result.workflows.map(run => run.status === 'completed' ?
      (run.conclusion === 'success' ? 'success' : run.conclusion ? 'failure' : 'unknown') :
      (['queued', 'in_progress', 'waiting', 'pending', 'requested'].includes(run.status) ? 'pending' : 'unknown'));
    result.observed = states.includes('failure') ? 'failure' : states.includes('pending') ? 'pending' :
      states.length && !states.includes('unknown') && !result.missing.length ? 'success' : 'unknown';
    // The connector exposes only page one with no pagination completeness metadata.
    // Observed successes never establish required-check coverage or merge readiness.
  } catch (_) {
    result.workflows = [];
    result.errors.push('workflow-read-failed');
  }
  try {
    const after = unwrap(await api.get_pr_info({repository_full_name: repository, pr_number: number}));
    if (!after.head_sha) throw new Error('missing head');
    result.stale = after.head_sha !== result.head;
    if (result.stale) result.currentHead = after.head_sha;
  } catch (_) {
    result.errors.push('head-recheck-failed');
  }
  return result;
};
