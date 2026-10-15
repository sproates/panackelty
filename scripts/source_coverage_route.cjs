'use strict';
// Fast policy checks remain required; collection is independently scheduled.
function gate({route,policy}) {
  const expectedPolicy=route==='full'?'success':'skipped';
  if(!['full','docs','workflow'].includes(route) || policy!==expectedPolicy)
    throw Error('Applicable source coverage policy checks did not pass');
}
if(require.main===module) gate({route:process.env.CI_SCOPE_ROUTE,
  policy:process.env.CI_SOURCE_POLICY_RESULT});
module.exports={gate};
