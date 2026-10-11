'use strict';
// Expensive collection is a promotion check, or an explicit report refresh.
function collect({event,base,ref,message=''}) {
  return (event==='pull_request' && base==='main') ||
    (event==='push' && ref==='refs/heads/next' && message.includes('[source-coverage]'));
}
function gate({route,requested,policy,collection}) {
  if(!['true','false'].includes(requested)) throw Error('Missing source coverage route');
  const expectedPolicy=route==='full'?'success':'skipped';
  const expectedCollection=route==='full' && requested==='true'?'success':'skipped';
  if(!['full','docs'].includes(route) || policy!==expectedPolicy || collection!==expectedCollection)
    throw Error('Applicable source coverage checks did not pass');
}
if(require.main===module) {
  if(process.argv[2]==='--gate') gate({route:process.env.CI_SCOPE_ROUTE,
    requested:process.env.CI_SOURCE_REQUESTED,policy:process.env.CI_SOURCE_POLICY_RESULT,
    collection:process.env.CI_SOURCE_COLLECTION_RESULT});
  else console.log(`source_coverage=${collect({
    event:process.env.CI_SOURCE_EVENT,base:process.env.CI_SOURCE_BASE,
    ref:process.env.CI_SOURCE_REF,message:process.env.CI_SOURCE_MESSAGE || ''})}`);
}
module.exports={collect,gate};
