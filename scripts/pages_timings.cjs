// Conservative wall-clock metrics include container initialization/image pulls,
// artifact transfer and inter-job scheduling. Initial workflow queue is separate.
module.exports = function timings(run, jobs, mergedAt) {
  const ms = value => {const t=Date.parse(value); if (!Number.isFinite(t)) throw new Error('Missing timing'); return t;};
  const completed = jobs.filter(j => j.conclusion !== 'skipped' && j.completed_at &&
    !['Pages timings'].includes(j.name));
  const validation = completed.filter(j => !['Deploy Pages','Verify published coverage'].includes(j.name));
  if (!validation.length) throw new Error('No validation timing');
  const start=Math.min(...validation.map(j=>ms(j.started_at)));
  const end=Math.max(...validation.map(j=>ms(j.completed_at)));
  const verification=completed.find(j=>j.name==='Verify published coverage');
  return {
    validation_seconds:(end-start)/1000,
    initial_queue_seconds:Math.max(0,(start-ms(run.created_at))/1000),
    merge_to_live_seconds:verification && mergedAt ? (ms(verification.completed_at)-ms(mergedAt))/1000 : null,
    jobs:completed.map(j=>({name:j.name, seconds:(ms(j.completed_at)-ms(j.started_at))/1000, conclusion:j.conclusion})),
  };
};
