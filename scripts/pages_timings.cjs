// Include container pulls (the Initialize containers step), transfers and
// inter-job scheduling. Separate observed runner dispatch before its first step.
module.exports = function timings(run, jobs, mergedAt, siteSha, checked) {
  const ms = value => {const t=Date.parse(value); if (!Number.isFinite(t)) throw new Error('Missing timing'); return t;};
  const completed = jobs.filter(j => j.conclusion !== 'skipped' && j.completed_at && j.name !== 'Pages timings');
  const actualStart = j => {
    const steps=(j.steps || []).filter(s=>s.conclusion !== 'skipped' && s.started_at);
    return steps.length ? Math.min(...steps.map(s=>ms(s.started_at))) : ms(j.started_at);
  };
  const validation = completed.filter(j => !['Deploy Pages','Verify published coverage'].includes(j.name));
  if (!validation.length) throw new Error('No validation timing');
  const start=Math.min(...validation.map(actualStart));
  const rawStart=Math.min(...validation.map(j=>ms(j.started_at)));
  const end=Math.max(...validation.map(j=>ms(j.completed_at)));
  // These stages are sequential, so dispatch intervals cannot overlap work in
  // another validation stage. First-stage dispatch is already excluded by start.
  const dispatch=validation.reduce((sum,j)=>sum+Math.max(0,actualStart(j)-Math.max(start,ms(j.started_at))),0);
  const verification=completed.find(j=>j.name==='Verify published coverage');
  // Only the first main push for the published source can measure merge latency.
  // Manual refreshes, workflow_run duplicates and reruns are separate attempts.
  const mergeMeasurement = run.event === 'push' && run.head_branch === 'main' &&
    run.run_attempt === 1 && siteSha && run.head_sha === siteSha && mergedAt;
  const verified = verification?.conclusion === 'success';
  const result = {
    validation_seconds:(end-start-dispatch)/1000,
    validation_wall_seconds:(end-rawStart)/1000,
    initial_queue_seconds:Math.max(0,(start-ms(run.created_at))/1000),
    subsequent_dispatch_seconds:dispatch/1000,
    live_verification:verification?.conclusion || 'not_run',
    merge_to_live_wall_seconds:verified && mergeMeasurement
      ? (ms(verification.completed_at)-ms(mergedAt))/1000 : null,
    // Queue overlap with the independent Check workflow is not measured here.
    // Never present the conservative wall time as queue-excluded acceptance.
    merge_to_live_seconds:null,
    jobs:completed.map(j=>({name:j.name, seconds:(ms(j.completed_at)-actualStart(j))/1000,
      dispatch_seconds:Math.max(0,(actualStart(j)-ms(j.started_at))/1000), conclusion:j.conclusion})),
  };
  if (checked) {
    const source=checked.run;
    if (source.head_sha !== siteSha || source.head_branch !== 'main' ||
        source.event !== 'push' || source.status !== 'completed' || source.conclusion !== 'success') {
      throw new Error('Invalid upstream website validation identity');
    }
    result.publication_validation_seconds=result.validation_seconds;
    result.validation_seconds=null;
    result.validation_wall_seconds=null;
    result.validation_scope='reused Check website artifact; not a first publication measurement';
    result.merge_to_live_wall_seconds=null;
    const first = run.event === 'workflow_run' && checked.trigger_id === source.id &&
      source.run_attempt === 1 && run.run_attempt === 1 && run.head_sha === siteSha;
    if (first) {
      const sourceJobs=checked.jobs.filter(j=>j.conclusion !== 'skipped' && j.completed_at);
      if (!sourceJobs.length || sourceJobs.some(j=>j.conclusion !== 'success')) {
        throw new Error('Missing successful upstream validation jobs');
      }
      const upstreamStart=Math.min(...sourceJobs.map(actualStart));
      const upstreamEnd=Math.max(...sourceJobs.map(j=>ms(j.completed_at)));
      if (upstreamEnd > start) throw new Error('Upstream validation did not precede publication');
      result.validation_seconds=(end-upstreamStart)/1000;
      result.validation_wall_seconds=result.validation_seconds;
      result.initial_queue_seconds=Math.max(0,(upstreamStart-ms(source.created_at))/1000);
      result.subsequent_dispatch_seconds=null;
      result.validation_scope='originating Check through Pages packaging; conservative wall bound';
      result.merge_to_live_wall_seconds=verified && mergedAt
        ? (ms(verification.completed_at)-ms(mergedAt))/1000 : null;
    }
  }
  return result;
};
