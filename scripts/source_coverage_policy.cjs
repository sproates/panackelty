'use strict';
// CI validates report integrity and reviewed scope. Coverage amounts are informational.
const {sha,totals}=require('./source_coverage.cjs');
const kinds=['lines','functions','branches'];
function requireThat(condition,message) { if(!condition) throw Error(`Coverage policy: ${message}`); }
function checkManifestPolicy(manifest,policy) {
  requireThat(policy?.schema===2 && typeof policy.review?.reason==='string' && policy.review.reason.trim().length>0,'missing reviewed policy');
  const manifestHash=sha(Buffer.from(JSON.stringify(manifest)));
  requireThat(policy.manifestHash===manifestHash,'scope changed; review the policy');
  return manifestHash;
}
function metric(states) {
  requireThat(states.every(s=>['covered','zero','unavailable'].includes(s)),'invalid item state');
  return {total:states.length,covered:states.filter(s=>s==='covered').length,
    zero:states.filter(s=>s==='zero').length,unavailable:states.filter(s=>s==='unavailable').length};
}
function counts(actual,expected,label) {
  requireThat(actual && expected,`missing ${label}`);
  for(const key of ['total','covered','zero','unavailable']) requireThat(
    Number.isSafeInteger(actual[key]) && actual[key]>=0 && actual[key]===expected[key],`inconsistent ${label}/${key}`);
}
function checkPolicy(report,policy,{commit,manifestHash,allowDirty=false}={}) {
  requireThat(policy?.schema===2 && typeof policy.review?.reason==='string' && policy.review.reason.trim().length>0,'missing reviewed policy');
  requireThat(report?.schema===1 && report.repository==='sproates/panackelty' && report.branch==='next','report identity');
  requireThat(/^[a-f0-9]{40}$/.test(commit||'') && report.commit===commit,'stale commit');
  requireThat(report.clean===true || (allowDirty && report.clean===false),'non-publishable report');
  requireThat(/^[a-f0-9]{64}$/.test(manifestHash||'') && report.manifestHash===manifestHash &&
    sha(Buffer.from(JSON.stringify(report.manifest)))===manifestHash,'stale manifest');
  checkManifestPolicy(report.manifest,policy);
  requireThat(Array.isArray(report.manifest.eligible) && Array.isArray(report.manifest.unmeasured) &&
    report.manifest.unmeasured.every(entry=>typeof entry?.path==='string' && typeof entry.reason==='string' && entry.reason.trim()),
    'invalid measured/unmeasured manifest');
  requireThat(Array.isArray(report.files) && report.files.length>0,'missing files');
  const paths=report.files.map(f=>f.path);
  requireThat(new Set(paths).size===paths.length && JSON.stringify([...paths].sort())===JSON.stringify([...report.manifest.eligible].sort()),'incomplete eligible files');
  const expectedSessions=['scope',...report.manifest.units.map(n=>`unit:${n}`),...report.manifest.functional.map(n=>`functional:${n}`),...report.manifest.compiler];
  requireThat(Array.isArray(report.sessions) && report.sessions.length===expectedSessions.length &&
    new Set(report.sessions.map(s=>s.id)).size===expectedSessions.length &&
    expectedSessions.every(id=>report.sessions.some(s=>s.id===id && Number.isSafeInteger(s.executions) && s.executions>0)),'incomplete sessions');
  for(const f of report.files) {
    requireThat(/^src\/(compiler|bytecode|stdlib)\/.+\.panack$/.test(f.path) && /^[a-f0-9]{64}$/.test(f.hash),'invalid source identity');
    requireThat(f.lines && Array.isArray(f.functions) && Array.isArray(f.branches) && Array.isArray(f.exclusions),'missing source rows');
    for(const kind of ['functions','branches']) requireThat(new Set(f[kind].map(r=>r.id)).size===f[kind].length,'duplicate source item');
    for(const kind of kinds) {
      const states=kind==='lines'?Object.values(f.lines):f[kind].map(r=>r.state);
      const expected=metric(states);counts(f.metrics?.[kind],expected,`${f.path}/${kind}`);
    }
  }
  const exclusionHash=sha(Buffer.from(JSON.stringify(report.files.map(f=>({path:f.path,exclusions:f.exclusions})))));
  requireThat(policy.exclusionHash===exclusionHash,'exclusions changed; review the policy');
  const overall=totals(report.files);
  for(const kind of kinds) counts(report.metrics?.[kind],overall[kind],`overall/${kind}`);
  for(const component of ['compiler','bytecode','stdlib']) {
    const measured=totals(report.files.filter(f=>f.path.startsWith(`src/${component}/`)));
    for(const kind of kinds) counts(report.components?.[component]?.[kind],measured[kind],`${component}/${kind}`);
  }
  return {status:'valid',components:report.components};
}
module.exports={checkManifestPolicy,checkPolicy};
