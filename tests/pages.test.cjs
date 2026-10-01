const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const selectSource = require('../scripts/pages_source.cjs');
const checkPages = require('../scripts/check_pages.cjs');
const repo = {owner: 'sproates', repo: 'panackelty'};
const run = (id, overrides = {}) => ({id, run_number: id, head_sha: String(id).padStart(40, '0'),
  head_branch: 'main', event: 'push', status: 'completed', conclusion: 'success',
  repository: {full_name: 'sproates/panackelty'}, head_repository: {full_name: 'sproates/panackelty'}, ...overrides});
const artifact = (id, overrides = {}) => ({id, name: `native-coverage-${id}`, expired: false, ...overrides});
function api(pages, artifacts, head = 5) {
  const paginate = async (_, args) => artifacts[args.run_id] || [];
  paginate.iterator = async function* (_, args) {
    assert.equal(args.branch, 'main'); assert.equal(args.event, 'push');
    assert.equal(args.status, 'success'); assert.equal(args.workflow_id, 'check.yml');
    for (const data of pages) yield {data};
  };
  return {paginate, rest: {
    repos: {getBranch: async args => {
      assert.deepEqual(args, {...repo, branch: 'main'});
      return {data: {commit: {sha: run(head).head_sha}}};
    }},
    actions: {listWorkflowRuns: 'runs', listWorkflowRunArtifacts: 'artifacts'},
  }};
}
test('documentation-only latest website retains prior coverage across pages', async () => {
  const selected = await selectSource(api([[run(5)], [run(4)]], {4: [artifact(4)]}), repo);
  assert.equal(selected.site.id, 5); assert.equal(selected.run.id, 4);
});
test('latest successful full run replaces both sources', async () => {
  const selected = await selectSource(api([[run(5), run(4)]], {5: [artifact(5)], 4: [artifact(4)]}), repo);
  assert.equal(selected.site.id, 5); assert.equal(selected.run.id, 5);
});
test('failed, incomplete, PR, other-branch and foreign runs cannot publish', async () => {
  const bad = [run(10, {conclusion: 'failure'}), run(9, {status: 'in_progress'}),
    run(8, {event: 'pull_request'}), run(7, {head_branch: 'feature'}),
    run(6, {head_repository: {full_name: 'other/fork'}})];
  const selected = await selectSource(api([[...bad, run(4)]],
    Object.fromEntries([...bad, run(4)].map(r => [r.id, [artifact(r.id)]])), 4), repo);
  assert.equal(selected.site.id, 4); assert.equal(selected.run.id, 4);
});
test('missing and expired reports fail closed instead of erasing published coverage', async () => {
  await assert.rejects(selectSource(api([[run(5)]], {}), repo), /No successful/);
  await assert.rejects(selectSource(api([[run(5), run(4)]], {
    5: [artifact(5, {expired: true})], 4: [artifact(4)],
  }), repo), /expired/);
});
test('API failures are not treated as missing coverage', async () => {
  const github = api([[run(5)]], {});
  const iterator = github.paginate.iterator;
  github.paginate = async () => {throw new Error('permission denied');};
  github.paginate.iterator = iterator;
  await assert.rejects(selectSource(github, repo), /permission denied/);
});

test('stale successful history cannot publish an older website', async () => {
  await assert.rejects(selectSource(api([[run(4)]], {4: [artifact(4)]}), repo),
    /No successful main Check/);
});
test('unordered pages and an old rerun cannot displace current source or latest coverage', async () => {
  const selected = await selectSource(api([[run(2)], [run(4), run(5)], [run(3)]], {
    2: [artifact(2)], 3: [artifact(3)], 4: [artifact(4)],
  }), repo);
  assert.equal(selected.site.id, 5);
  assert.equal(selected.run.id, 4);
});
test('a pending or failed main head never falls back to an older passing commit', async () => {
  for (const overrides of [{status: 'in_progress'}, {conclusion: 'failure'},
    {head_repository: null}, {repository: {full_name: 'other/repo'}}]) {
    await assert.rejects(selectSource(api([[run(5, overrides), run(4)]], {
      4: [artifact(4)], 5: [artifact(5)],
    }), repo), /No successful main Check/);
  }
});
test('a main advance during selection cannot mix in coverage newer than the pinned source', async () => {
  const selected = await selectSource(api([[run(6), run(5), run(4)]], {
    6: [artifact(6)], 4: [artifact(4)],
  }), repo);
  assert.equal(selected.site.id, 5);
  assert.equal(selected.run.id, 4);
});
test('branch and paginated run lookup errors fail closed', async () => {
  const github = api([[run(5)]], {5: [artifact(5)]});
  github.rest.repos.getBranch = async () => {throw new Error('branch unavailable');};
  await assert.rejects(selectSource(github, repo), /branch unavailable/);
  const history = api([[run(5)]], {5: [artifact(5)]});
  history.paginate.iterator = async function* () {
    yield {data: [run(5)]};
    throw new Error('history unavailable');
  };
  await assert.rejects(selectSource(history, repo), /history unavailable/);
});
test('assembled real website and nested source links resolve; broken links fail', async t => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'pages-links-'));
  t.after(() => fs.rmSync(root, {recursive: true, force: true}));
  fs.cpSync('site', root, {recursive: true});
  fs.mkdirSync(path.join(root, 'playground'));
  fs.writeFileSync(path.join(root, 'playground/index.html'), '<a href="../">home</a>');
  const version = 'a'.repeat(64);
  const assets = `playground/assets/${version}`;
  fs.mkdirSync(path.join(root, assets), {recursive:true});
  fs.writeFileSync(path.join(root, 'playground/asset-version.txt'), version + '\n');
  for (const name of ['vm.wasm', 'compiler.bc', 'stdlib.json', 'worker.mjs', 'provenance.json']) {
    fs.writeFileSync(path.join(root, assets, name), 'fixture');
  }
  fs.mkdirSync(path.join(root, 'coverage/html/coverage/src'), {recursive: true});
  fs.writeFileSync(path.join(root, 'coverage/index.html'), '<a href="html/index.html">report</a>');
  fs.writeFileSync(path.join(root, 'coverage/summary.txt'), 'summary');
  fs.writeFileSync(path.join(root, 'coverage/html/index.html'), '<a href="coverage/src/vm.c.html">source</a>');
  fs.writeFileSync(path.join(root, 'coverage/html/coverage/src/vm.c.html'), '<a href="../../index.html">back</a>');
  const targets = await checkPages(root);
  assert(targets.has('coverage/html/coverage/src/vm.c.html'));
  assert(targets.has('playground/index.html'));
  fs.writeFileSync(path.join(root, 'coverage/provenance.txt'), 'coverage_commit=abc\n');
  const visited = [];
  let stale = false;
  let broken = false;
  let wrongMime = false;
  let staleWasm = false;
  t.mock.method(global, 'fetch', async url => {
    const file = url.pathname.slice(1);
    visited.push(file);
    if (broken) return {ok: false, status: 404};
    return {ok: true,
      headers: new Map([['content-type', wrongMime ? 'text/html' : 'application/wasm']]),
      arrayBuffer: async () => staleWasm && file === `${assets}/vm.wasm`
        ? Buffer.from('stale') : fs.readFileSync(path.join(root, file)),
      text: async () => stale && file === 'coverage/provenance.txt'
      ? 'old provenance' : fs.readFileSync(path.join(root, file), 'utf8')};
  });
  await checkPages(root, 'https://example.test/');
  assert(visited.includes('coverage/html/coverage/src/vm.c.html'));
  assert(visited.includes(`${assets}/vm.wasm`));
  assert(visited.includes(`${assets}/worker.mjs`));
  wrongMime = true;
  await assert.rejects(checkPages(root, 'https://example.test/'), /MIME/);
  wrongMime = false; staleWasm = true;
  await assert.rejects(checkPages(root, 'https://example.test/'), /playground content/);
  staleWasm = false;
  stale = true;
  await assert.rejects(checkPages(root, 'https://example.test/'), /provenance/);
  stale = false; broken = true;
  await assert.rejects(checkPages(root, 'https://example.test/'), /404/);
  fs.unlinkSync(path.join(root, 'coverage/html/coverage/src/vm.c.html'));
  await assert.rejects(checkPages(root), /ENOENT/);
  fs.writeFileSync(path.join(root, 'coverage/html/index.html'), '<a href="../../../outside.html">escape</a>');
  await assert.rejects(checkPages(root), /Escaping link/);
});

test('homepage section navigation and example references have unique targets', () => {
  const html = fs.readFileSync('site/index.html', 'utf8');
  const ids = [...html.matchAll(/\bid="([^"]+)"/g)].map(match => match[1]);
  assert.equal(new Set(ids).size, ids.length, 'duplicate page anchor');
  for (const [, target] of html.matchAll(/href="#([^"]+)"/g)) {
    assert(ids.includes(target), `missing anchor: ${target}`);
  }
  for (const [, labels] of html.matchAll(/aria-labelledby="([^"]+)"/g)) {
    for (const label of labels.split(/\s+/)) assert(ids.includes(label), `missing accessible label: ${label}`);
  }
  for (const name of ['hello', 'guards', 'exact']) {
    assert(ids.includes(`${name}-source`), `missing runnable example: ${name}`);
    assert(ids.includes(`${name}-output`), `missing expected output: ${name}`);
  }
});

const findWebsite = require('../scripts/pages_artifact.cjs');
const fingerprint = 'a'.repeat(64);
test('checked website is bound to the exact successful main Check and keeps older coverage', async () => {
  const checked=require('../scripts/pages_checked_website.cjs');
  const sha=run(5).head_sha;
  const candidate={id:99,name:`checked-website-${sha}`,expired:false};
  const github=api([[run(5),run(4)]],{5:[candidate],4:[artifact(4)]});
  assert.equal((await checked(github,repo,sha)).id,99);
  assert.equal((await selectSource(github,repo)).run.id,4);
  await assert.rejects(checked(github,repo,run(4).head_sha),/Main advanced/);
  for (const overrides of [{event:'pull_request'}, {status:'in_progress'}, {conclusion:'failure'},
    {head_repository:{full_name:'other/repo'}}, {head_branch:'feature'}]) {
    await assert.rejects(checked(api([[run(5,overrides),run(4)]],
      {5:[candidate],4:[artifact(4)]}),repo,sha),/No successful main Check/);
  }
  assert.equal(await checked(api([[run(5),run(4)]],
    {5:[{...candidate,name:`checked-website-${run(4).head_sha}`}],4:[artifact(4)]}),repo,sha),null);
  await assert.rejects(checked(api([[run(5),run(4)]],
    {5:[{...candidate,expired:true}],4:[artifact(4)]}),repo,sha),/expired/);
  await assert.rejects(checked(api([[run(5)]],{5:[candidate]}),repo,sha),/coverage artifact/);
  await assert.rejects(checked(github,repo,'invalid'),/Invalid website source/);
});

test('website certification follows browser success; publisher restores and checks certified bytes', () => {
  const check=fs.readFileSync('.github/workflows/check.yml','utf8');
  const website=fs.readFileSync('.github/workflows/website-validation.yml','utf8');
  const pages=fs.readFileSync('.github/workflows/pages.yml','utf8');
  assert.match(check,/needs: \[changes, package_build, website\]/);
  assert.match(check,/needs: \[changes, test_run, website\]/);
  assert.match(check,/needs.changes.outputs.pages == 'true'/);
  assert.match(check,/pages: \$\{\{ steps.scope.outputs.pages \}\}/);
  assert.equal((check.match(/CI_WEBSITE_REQUIRED:/g)||[]).length,2);
  assert.equal((check.match(/CI_WEBSITE_RESULT:/g)||[]).length,2);
  assert.match(website,/ref: \$\{\{ github.sha \}\}/);
  assert.match(website,/needs: \[prepare, browser\]/);
  assert.match(website,/name: checked-website-\$\{\{ github.sha \}\}/);
  assert.match(website,/name: website-preview-\$\{\{ github.run_id \}\}-\$\{\{ github.run_attempt \}\}/);
  assert.doesNotMatch(website,/if: always|pages: write|id-token: write|make check/);
  assert.match(pages,/scripts\/pages_checked_website.cjs/);
  assert.match(pages,/test .*cat build\/website\/\.validation-fingerprint/);
  assert.match(pages,/test ! -e build\/website\/coverage/);
  assert.match(pages,/grep -qx 'pages=true'/);
  assert.match(pages,/s\/\^pages=true\$\/pages=false\//);
});
function websiteApi(pages, artifacts) {
  const paginate = async (_, args) => artifacts[args.run_id] || [];
  paginate.iterator = async function* (_, args) {
    assert.equal(args.workflow_id,'pages.yml');
    assert.equal(args.branch,'main'); assert.equal(args.status,'success');
    for (const data of pages) yield {data};
  };
  return {paginate,rest:{actions:{listWorkflowRuns:'runs',listWorkflowRunArtifacts:'artifacts'}}};
}
const websiteRun = (id, overrides = {}) => run(id, {event:'workflow_run', path:'.github/workflows/pages.yml', ...overrides});
const websiteArtifact = (id, overrides = {}) => ({id, name: `validated-website-${fingerprint}`, expired:false, ...overrides});
test('reuse matching website via paginated trusted runs without repository artifact provenance', async () => {
  const selected = await findWebsite(websiteApi([[websiteRun(2)], [websiteRun(5), websiteRun(4)]], {
    2: [websiteArtifact(2)], 4: [websiteArtifact(4)],
    5: [websiteArtifact(5, {name:'validated-website-' + 'b'.repeat(64)})],
  }), repo, fingerprint);
  assert.equal(selected.id, 4);
});
test('PR, failed, incomplete, foreign and non-main artifacts cannot seed publication', async () => {
  for (const overrides of [{event:'pull_request'}, {conclusion:'failure'}, {status:'in_progress'},
    {path:'.github/workflows/other.yml'}, {head_branch:'feature'}, {repository:{full_name:'other/repo'}}, {head_repository:null}]) {
    assert.equal(await findWebsite(websiteApi([[websiteRun(5, overrides)]],
      {5:[websiteArtifact(5)]}), repo, fingerprint), null);
  }
});
test('missing identity requires browser tests; expired artifact and API errors fail closed', async () => {
  assert.equal(await findWebsite(websiteApi([[websiteRun(5)]], {}), repo, fingerprint), null);
  await assert.rejects(findWebsite(websiteApi([[websiteRun(5)]],
    {5:[websiteArtifact(5, {expired:true})]}), repo, fingerprint), /expired/);
  await assert.rejects(findWebsite(websiteApi([], {}), repo, '../invalid'), /fingerprint/);
  const api = websiteApi([[websiteRun(5)]], {});
  const iterator=api.paginate.iterator;
  api.paginate=async()=>{throw new Error('API unavailable');};
  api.paginate.iterator=iterator;
  await assert.rejects(findWebsite(api, repo, fingerprint), /API unavailable/);
});
test('publisher requires browser success or authenticated reuse; prepared environment has no install step', () => {
  const workflow = fs.readFileSync('.github/workflows/pages.yml','utf8');
  assert.match(workflow, /needs.build.outputs.browser == 'true' && needs.browser.result == 'success'/);
  assert.match(workflow, /needs.build.outputs.browser == 'false' && needs.browser.result == 'skipped'/);
  assert.match(workflow, /needs.build.result == 'success'/);
  assert.match(workflow, /Classify before any website work/);
  assert.match(workflow, /steps.scope.outputs.pages == 'true'/);
  const browser = fs.readFileSync('.github/workflows/browser-validation.yml','utf8');
  assert.match(workflow, /uses: \.\/\.github\/workflows\/browser-validation\.yml/);
  assert.match(browser, /playwright:v1\.63\.0-noble@sha256:[a-f0-9]{64}/);
  assert.match(browser, /Playwright package\/image mismatch/);
  assert.match(browser, /npm run test:browser -- --workers=3/);
  assert.match(browser, /chown .* \/github\/home/);
  assert.doesNotMatch(browser, /playwright install|apt-get|actions\/cache/);
});

test('cold artifact lookup is bounded and selects newest identity despite response order', async () => {
  const github=websiteApi([Array.from({length:20},(_,i)=>websiteRun(20-i))],{});
  const iterator=github.paginate.iterator;
  let active=0, peak=0; const requested=[];
  github.paginate=async (_, {run_id:id})=>{
    requested.push(id); peak=Math.max(peak,++active);
    await new Promise(resolve=>setImmediate(resolve));
    if (id===18) await new Promise(resolve=>setImmediate(resolve));
    active--;
    return [18,17].includes(id) ? [websiteArtifact(id)] : [];
  };
  github.paginate.iterator=iterator;
  assert.equal((await findWebsite(github,repo,fingerprint)).id,18);
  assert.equal(peak,8); assert.equal(active,0);
  assert.deepEqual(requested,[20,19,18,17,16,15,14,13,12]);
});

test('warm artifact hit avoids speculative history requests', async () => {
  const github=websiteApi([[websiteRun(3),websiteRun(2)]],{3:[websiteArtifact(3)]});
  const original=github.paginate; let calls=0;
  github.paginate=async (...args)=>{calls++;return original(...args);};
  github.paginate.iterator=original.iterator;
  assert.equal((await findWebsite(github,repo,fingerprint)).id,3);
  assert.equal(calls,1);
});

test('concurrent discovery never hides API failures or falls back past an expired match', async () => {
  const github=websiteApi([[websiteRun(4),websiteRun(3),websiteRun(2)]],{});
  const iterator=github.paginate.iterator;
  github.paginate=async (_, {run_id:id})=>{
    if (id===2) throw new Error('artifact API failed');
    return id===3 ? [websiteArtifact(3)] : [];
  };
  github.paginate.iterator=iterator;
  await assert.rejects(findWebsite(github,repo,fingerprint),/artifact API failed/);
  await assert.rejects(findWebsite(websiteApi([[websiteRun(4),websiteRun(3),websiteRun(2)]],
    {3:[websiteArtifact(3,{expired:true})],2:[websiteArtifact(2)]}),repo,fingerprint),/expired/);
});

test('timings include image initialization and transfers, report initial queue and merge-to-live', () => {
  const measure=require('../scripts/pages_timings.cjs');
  const time=n=>new Date(n*1000).toISOString();
  const jobs=[{name:'Prepare Pages',started_at:time(10),completed_at:time(30),conclusion:'success'},
    {name:'Browser integration',started_at:time(35),completed_at:time(80),conclusion:'success'},
    {name:'Package tested Pages',started_at:time(85),completed_at:time(90),conclusion:'success'},
    {name:'Verify published coverage',started_at:time(95),completed_at:time(100),conclusion:'success'}];
  const result=measure({created_at:time(0),event:'push',head_branch:'main',run_attempt:1,head_sha:'abc'},jobs,time(0),'abc');
  assert.equal(result.validation_seconds,80);
  assert.equal(result.initial_queue_seconds,10);
  assert.equal(result.merge_to_live_wall_seconds,100);
  assert.equal(result.merge_to_live_seconds,null);
  assert.equal(result.live_verification,'success');
  assert.equal(result.jobs[1].seconds,45);
});

test('publication waits for exact Check, and never substitutes a newer or older source', async () => {
  const ready=require('../scripts/pages_ready.cjs');
  const sha='b'.repeat(40); let calls=0; let waits=0;
  const result=await ready(async()=>{
    if (++calls<3) throw new Error(`No successful main Check for ${sha}; wait for validation and retry Pages.`);
    return {site:{head_sha:sha},run:{id:12}};
  },sha,async ms=>{assert.equal(ms,5000);waits++;});
  assert.equal(result.run.id,12); assert.equal(waits,2);
  await assert.rejects(ready(async()=>({site:{head_sha:'c'.repeat(40)}}),sha), /Main advanced/);
  await assert.rejects(ready(async()=>{throw new Error('API denied');},sha), /API denied/);
  await assert.rejects(ready(async()=>{throw new Error('Coverage expired');},sha), /Coverage expired/);
  calls=0;
  await assert.rejects(ready(async()=>{calls++;throw new Error(`No successful main Check for ${sha}; pending`);},sha,async()=>{}), /pending/);
  assert.equal(calls,25);
});

test('observed dispatch before first runner step is separated without subtracting image pulls', () => {
  const measure=require('../scripts/pages_timings.cjs'); const time=n=>new Date(n*1000).toISOString();
  const jobs=[{name:'changes',started_at:time(0),completed_at:time(42),conclusion:'success',
    steps:[{name:'Set up job',started_at:time(37),conclusion:'success'}]},
    {name:'Browser integration',started_at:time(45),completed_at:time(140),conclusion:'success',
    steps:[{name:'Set up job',started_at:time(46),conclusion:'success'},
      {name:'Initialize containers',started_at:time(47),conclusion:'success'}]},
    {name:'Package tested Pages',started_at:time(142),completed_at:time(148),conclusion:'success'}];
  const result=measure({created_at:time(0)},jobs);
  assert.equal(result.validation_wall_seconds,148); assert.equal(result.validation_seconds,110);
  assert.equal(result.initial_queue_seconds,37); assert.equal(result.subsequent_dispatch_seconds,1);
  assert.equal(result.jobs[1].seconds,94);
});


test('failed, cancelled, skipped and missing live verification never report merge success', () => {
  const measure=require('../scripts/pages_timings.cjs');
  const time=n=>new Date(n*1000).toISOString();
  const run={created_at:time(0),event:'push',head_branch:'main',run_attempt:1,head_sha:'abc'};
  const prepare={name:'Prepare Pages',started_at:time(10),completed_at:time(30),conclusion:'success'};
  for (const conclusion of ['failure','cancelled','skipped',null]) {
    const jobs=[prepare];
    if (conclusion) jobs.push({name:'Verify published coverage',started_at:time(95),completed_at:time(100),conclusion});
    const result=measure(run,jobs,time(0),'abc');
    assert.equal(result.merge_to_live_wall_seconds,null,conclusion);
    assert.equal(result.merge_to_live_seconds,null,conclusion);
    assert.equal(result.live_verification,conclusion === 'skipped' || !conclusion ? 'not_run' : conclusion);
  }
});

test('refreshes, reruns and superseding sources are not first merge-to-live measurements', () => {
  const measure=require('../scripts/pages_timings.cjs');
  const time=n=>new Date(n*1000).toISOString();
  const run={created_at:time(0),event:'push',head_branch:'main',run_attempt:1,head_sha:'abc'};
  const jobs=[{name:'Prepare Pages',started_at:time(10),completed_at:time(30),conclusion:'success'},
    {name:'Verify published coverage',started_at:time(95),completed_at:time(100),conclusion:'success'}];
  for (const change of [{event:'workflow_dispatch'},{event:'workflow_run'},{event:'pull_request'},
    {run_attempt:2},{head_branch:'feature'},{head_sha:'older'}]) {
    assert.equal(measure({...run,...change},jobs,time(0),'abc').merge_to_live_wall_seconds,null);
  }
  assert.equal(measure(run,jobs,undefined,'abc').merge_to_live_wall_seconds,null);
  assert.equal(measure(run,jobs,time(0)).merge_to_live_wall_seconds,null);
});


test('artifact history failures propagate and reuse decisions are observable', async () => {
  const messages=[];
  const selected=await findWebsite(websiteApi([[websiteRun(7)],[websiteRun(5)]],
    {5:[websiteArtifact(5)]}),repo,fingerprint,m=>messages.push(m));
  assert.equal(selected.id,5);
  assert(messages.some(m=>m.includes('run 7: no matching')));
  assert(messages.some(m=>m.includes('Reusing website artifact 5')));
  const broken=websiteApi([],{});
  broken.paginate.iterator=async function* () {throw new Error('history unavailable');};
  await assert.rejects(findWebsite(broken,repo,fingerprint),/history unavailable/);
});

test('duplicate publication requires exact live website and coverage identity', async () => {
  const duplicate=require('../scripts/pages_duplicate.cjs');
  const selected={site:run(5),run:run(4),report:{created_at:'2026-10-01T00:00:00Z'}};
  const sha=selected.site.head_sha;
  const expected=`coverage_commit=${selected.run.head_sha}\narchived_at=${selected.report.created_at}\ncheck_run=4\nsite_commit=${sha}\n`;
  const live=body=>async (url,options)=>{
    assert.equal(url,'https://panackelty.com/coverage/provenance.txt');
    assert.equal(options.cache,'no-store');
    return {ok:true,status:200,text:async()=>body};
  };
  assert.equal(await duplicate(selected,sha,live(expected)),true);
  for (const body of ['',expected.replace('check_run=4','check_run=3'),
    expected.replace(sha,run(3).head_sha),expected.replace(selected.run.head_sha,run(2).head_sha),
    expected.replace('00:00:00Z','00:01:00Z')]) {
    assert.equal(await duplicate(selected,sha,live(body)),false);
  }
  assert.equal(await duplicate(selected,sha,async()=>({ok:false,status:404})),false);
  await assert.rejects(duplicate(selected,sha,async()=>({ok:false,status:503})),/HTTP 503/);
  await assert.rejects(duplicate(selected,sha,async()=>{throw new Error('network unavailable');}),/network unavailable/);
  await assert.rejects(duplicate(selected,run(6).head_sha,live(expected)),/Main advanced/);
});

test('website-only publication timings include originating Check and cannot hide validation', () => {
  const measure=require('../scripts/pages_timings.cjs');
  const t=n=>new Date(n*1000).toISOString();
  const job=(name,start,end,conclusion='success')=>({name,started_at:t(start),completed_at:t(end),conclusion});
  const sha=run(5).head_sha;
  const check={...run(5),run_attempt:1,created_at:t(0)};
  const page={...run(6),head_sha:sha,event:'workflow_run',run_attempt:1,created_at:t(110)};
  const jobs=[job('Prepare Pages',115,125),job('Package tested Pages',130,140),job('Verify published coverage',145,155)];
  const upstream={run:check,jobs:[job('website / Prepare checked website',5,15),job('website / Browser integration',20,100)],trigger_id:5};
  const measured=measure(page,jobs,t(0),sha,upstream);
  assert.equal(measured.validation_seconds,135);
  assert.equal(measured.publication_validation_seconds,25);
  assert.equal(measured.merge_to_live_wall_seconds,155);
  assert.equal(measured.initial_queue_seconds,5);
  assert.equal(measured.subsequent_dispatch_seconds,null);
  for (const changes of [{event:'workflow_dispatch'},{run_attempt:2},{head_sha:run(4).head_sha}]) {
    const result=measure({...page,...changes},jobs,t(0),sha,upstream);
    assert.equal(result.validation_seconds,null);
    assert.equal(result.merge_to_live_wall_seconds,null);
  }
  for (const changes of [{trigger_id:4},{run:{...check,run_attempt:2}}]) {
    assert.equal(measure(page,jobs,t(0),sha,{...upstream,...changes}).merge_to_live_wall_seconds,null);
  }
  const failed=[...jobs.slice(0,2),job('Verify published coverage',145,155,'failure')];
  assert.equal(measure(page,failed,t(0),sha,upstream).merge_to_live_wall_seconds,null);
  for (const bad of [{head_sha:run(4).head_sha},{conclusion:'failure'},{event:'pull_request'}]) {
    assert.throws(()=>measure(page,jobs,t(0),sha,{...upstream,run:{...check,...bad}}),/Invalid upstream/);
  }
  assert.throws(()=>measure(page,jobs,t(0),sha,{...upstream,jobs:[]}),/Missing successful/);
});

// GitHub applies an implicit success() unless a condition has a status function.
// That implicit check propagates skips from the reused browser's dependency chain.
test('deployment and live verification tolerate skipped ancestors only after successful prerequisites', () => {
  const workflow=fs.readFileSync('.github/workflows/pages.yml','utf8');
  const condition = job => {
    const block=workflow.split(`\n  ${job}:\n`)[1].split(/\n  [a-z]+:\n/)[0];
    const expression=block.match(/    if: "([^"\n]+)"/)[1];
    assert.match(expression, /!cancelled\(\)/, 'explicit status overrides implicit ancestor success');
    return new Function('cancelled','github','needs', `return (${expression});`);
  };
  const deploy=condition('deploy'), verify=condition('verify');
  for (const cancelled of [false,true]) {
    for (const event_name of ['pull_request','push','workflow_run','workflow_dispatch']) {
      for (const result of ['success','failure','skipped','cancelled','']) {
        const needs={publish:{result},browser:{result:'skipped'}};
        assert.equal(deploy(()=>cancelled,{event_name},needs), !cancelled && event_name!=='pull_request' && result==='success');
      }
    }
    for (const build of ['success','failure','skipped','cancelled','']) {
      for (const deployed of ['success','failure','skipped','cancelled','']) {
        assert.equal(verify(()=>cancelled,{}, {build:{result:build},deploy:{result:deployed},browser:{result:'skipped'}}),
          !cancelled && build==='success' && deployed==='success');
      }
    }
  }
});
