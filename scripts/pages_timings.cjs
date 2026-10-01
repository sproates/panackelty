// Include container pulls (the Initialize containers step), transfers and
// inter-job scheduling. Separate observed runner dispatch before its first step.
module.exports = function timings(run, jobs, mergedAt) {
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
  return {
    validation_seconds:(end-start-dispatch)/1000,
    validation_wall_seconds:(end-rawStart)/1000,
    initial_queue_seconds:Math.max(0,(start-ms(run.created_at))/1000),
    subsequent_dispatch_seconds:dispatch/1000,
    merge_to_live_seconds:verification && mergedAt ? (ms(verification.completed_at)-ms(mergedAt))/1000 : null,
    jobs:completed.map(j=>({name:j.name, seconds:(ms(j.completed_at)-actualStart(j))/1000,
      dispatch_seconds:Math.max(0,(actualStart(j)-ms(j.started_at))/1000), conclusion:j.conclusion})),
  };
};
