'use strict';
// CI compares locally validated collection with a reviewed, versioned policy.
const {sha,totals}=require('./source_coverage.cjs');
const kinds=['lines','functions','branches'];
function requireThat(condition,message) { if(!condition) throw Error(`Coverage policy: ${message}`); }
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
  requireThat(policy?.schema===1 && typeof policy.review?.reason==='string' && policy.review.reason.trim().length>0,'missing reviewed policy');
  requireThat(report?.schema===1 && report.repository==='sproates/panackelty' && report.branch==='next','report identity');
  requireThat(/^[a-f0-9]{40}$/.test(commit||'') && report.commit===commit,'stale commit');
  requireThat(report.clean===true || (allowDirty && report.clean===false),'non-publishable report');
  requireThat(/^[a-f0-9]{64}$/.test(manifestHash||'') && report.manifestHash===manifestHash &&
    sha(Buffer.from(JSON.stringify(report.manifest)))===manifestHash,'stale manifest');
  requireThat(policy.manifestHash===manifestHash,'scope changed; review the policy');
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
      requireThat(expected.unavailable===0,`unavailable ${f.path}/${kind}`);
    }
  }
  const exclusionHash=sha(Buffer.from(JSON.stringify(report.files.map(f=>({path:f.path,exclusions:f.exclusions})))));
  requireThat(policy.exclusionHash===exclusionHash,'exclusions changed; review the policy');
  const overall=totals(report.files);
  for(const kind of kinds) counts(report.metrics?.[kind],overall[kind],`overall/${kind}`);
  requireThat(JSON.stringify(Object.keys(policy.components||{}).sort())===JSON.stringify(['bytecode','compiler','stdlib']),'component policy shape');
  for(const component of ['compiler','bytecode','stdlib']) {
    const measured=totals(report.files.filter(f=>f.path.startsWith(`src/${component}/`)));
    for(const kind of kinds) {
      counts(report.components?.[component]?.[kind],measured[kind],`${component}/${kind}`);
      const floor=policy.components[component][kind], actual=measured[kind];
      requireThat(floor && Number.isSafeInteger(floor.total) && floor.total>0 && Number.isSafeInteger(floor.covered) && floor.covered>=0 && floor.covered<=floor.total,'invalid floor');
      requireThat(actual.total===floor.total,`denominator changed ${component}/${kind}; review the policy`);
      requireThat(BigInt(actual.covered)*BigInt(floor.total)>=BigInt(floor.covered)*BigInt(actual.total),`regression ${component}/${kind}: ${actual.covered}/${actual.total} below ${floor.covered}/${floor.total}`);
    }
  }
  requireThat(Array.isArray(policy.protected) && policy.protected.length>0,'missing protected outcomes');
  for(const guard of policy.protected) {
    requireThat(['functions','branches'].includes(guard.kind) && Array.isArray(guard.ids) && guard.ids.length>0 && new Set(guard.ids).size===guard.ids.length,'invalid guard');
    const file=report.files.find(f=>f.path===guard.path);
    requireThat(file && file.hash===guard.hash,`protected source changed ${guard.path}; review the policy`);
    for(const id of guard.ids) requireThat(file[guard.kind].some(r=>r.id===id && r.state==='covered'),`protected item not covered ${guard.path}/${id}`);
  }
  return {status:'passed',components:report.components,protected:policy.protected.reduce((n,g)=>n+g.ids.length,0)};
}
module.exports={checkPolicy};
