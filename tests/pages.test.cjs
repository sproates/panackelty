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
function websiteApi(pages, artifacts) {
  return {paginate: async (_, args) => {
    assert.equal(args.name, `validated-website-${fingerprint}`);
    return Object.entries(artifacts).flatMap(([id, entries]) =>
      entries.map(a => ({workflow_run:{id:Number(id)}, ...a})));
  }, rest: {actions: {listArtifacts:'artifacts', getWorkflowRun: async args =>
    ({data:pages.flat().find(r => r.id === args.run_id)})}}};
}
const websiteRun = (id, overrides = {}) => run(id, {event:'workflow_run', path:'.github/workflows/pages.yml', ...overrides});
const websiteArtifact = (id, overrides = {}) => ({id, name: `validated-website-${fingerprint}`, expired:false, ...overrides});
test('reuse only identical successful production website, with paginated unordered runs', async () => {
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
  api.paginate=async()=>{throw new Error('API unavailable');};
  await assert.rejects(findWebsite(api, repo, fingerprint), /API unavailable/);
});
test('publisher requires browser success or authenticated reuse; prepared environment has no install step', () => {
  const workflow = fs.readFileSync('.github/workflows/pages.yml','utf8');
  assert.match(workflow, /needs.build.outputs.browser == 'true' && needs.browser.result == 'success'/);
  assert.match(workflow, /needs.build.outputs.browser == 'false' && needs.browser.result == 'skipped'/);
  assert.match(workflow, /needs.build.result == 'success'/);
  assert.match(workflow, /Classify before any website work/);
  assert.match(workflow, /steps.scope.outputs.pages == 'true'/);
  assert.match(workflow, /playwright:v1\.63\.0-noble@sha256:[a-f0-9]{64}/);
  assert.match(workflow, /Playwright package\/image mismatch/);
  assert.match(workflow, /npm run test:browser -- --workers=3/);
  assert.match(workflow, /chown .* \/github\/home/);
  assert.doesNotMatch(workflow, /playwright install|apt-get|actions\/cache/);
});

test('timings include image initialization and transfers, report initial queue and merge-to-live', () => {
  const measure=require('../scripts/pages_timings.cjs');
  const time=n=>new Date(n*1000).toISOString();
  const jobs=[{name:'Prepare Pages',started_at:time(10),completed_at:time(30),conclusion:'success'},
    {name:'Browser integration',started_at:time(35),completed_at:time(80),conclusion:'success'},
    {name:'Package tested Pages',started_at:time(85),completed_at:time(90),conclusion:'success'},
    {name:'Verify published coverage',started_at:time(95),completed_at:time(100),conclusion:'success'}];
  const result=measure({created_at:time(0)},jobs,time(0));
  assert.equal(result.validation_seconds,80);
  assert.equal(result.initial_queue_seconds,10);
  assert.equal(result.merge_to_live_seconds,100);
  assert.equal(result.jobs[1].seconds,45);
});

test('publication waits for exact Check, and never substitutes a newer or older source', async () => {
  const ready=require('../scripts/pages_ready.cjs');
  const sha='b'.repeat(40); let calls=0; let waits=0;
  const result=await ready(async()=>{
    if (++calls<3) throw new Error(`No successful main Check for ${sha}; wait for validation and retry Pages.`);
    return {site:{head_sha:sha},run:{id:12}};
  },sha,async ms=>{assert.equal(ms,10000);waits++;});
  assert.equal(result.run.id,12); assert.equal(waits,2);
  await assert.rejects(ready(async()=>({site:{head_sha:'c'.repeat(40)}}),sha), /Main advanced/);
  await assert.rejects(ready(async()=>{throw new Error('API denied');},sha), /API denied/);
  await assert.rejects(ready(async()=>{throw new Error('Coverage expired');},sha), /Coverage expired/);
  calls=0;
  await assert.rejects(ready(async()=>{calls++;throw new Error(`No successful main Check for ${sha}; pending`);},sha,async()=>{}), /pending/);
  assert.equal(calls,13);
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
