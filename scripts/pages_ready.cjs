// Website validation may overlap Check, but publication must wait for the
// exact selected source. A superseding main or an API/report error fails closed.
module.exports = async function ready(selectSource, sha, pause = ms => new Promise(resolve => setTimeout(resolve, ms))) {
  if (!/^[a-f0-9]{40}$/.test(sha)) throw new Error('Invalid selected website source');
  for (let attempt=0; attempt<25; attempt++) {
    try {
      const selected=await selectSource();
      if (selected.site.head_sha !== sha) throw new Error('Main advanced: retry publication for current main');
      return selected;
    } catch (error) {
      if (!error.message.startsWith(`No successful main Check for ${sha};`) || attempt===24) throw error;
      await pause(5000);
    }
  }
};
