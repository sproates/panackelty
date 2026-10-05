'use strict';
// Generation stays in core. The publisher receives static reports, never executable plans.
const fs = require('node:fs'), path = require('node:path'), os = require('node:os');
const {spawnSync} = require('node:child_process');
const {regular,sha,readPlan,readSession,observations,union,totals,writeReport} = require('./source_coverage.cjs');
const checkout = path.resolve(__dirname,'..');
function command(args, timeout = 180000) {
  const result = spawnSync(args[0],args.slice(1),{cwd:checkout,encoding:'utf8',timeout,maxBuffer:16*1024*1024,
    env:{...process.env,PANACK_TEST_RUNNER_REPORT:'',PANACK_COVERAGE_TICKET:''}});
  if (result.error || result.status !== 0 || result.signal) throw new Error(`${args.slice(0,3).join(' ')} failed: ${result.error?.message || result.stderr || result.stdout}`);
  return result.stdout;
}
function baseline(output, allowDirty = false) {
  const manifestBytes = regular(path.join(checkout,'tests/source_coverage/manifest.json'));
  const manifest = JSON.parse(manifestBytes);
  const tracked = command(['git','ls-files','src']).trim().split('\n').filter(p=>p.endsWith('.panack')).sort();
  if (JSON.stringify(tracked) !== JSON.stringify([...manifest.eligible].sort())) throw Error('Eligible source manifest differs from tracked src/**/*.panack');
  const commit = command(['git','rev-parse','HEAD']).trim();
  const clean = !allowDirty && !command(['git','status','--porcelain','--untracked-files=all']).trim();
  if (!clean && !allowDirty) throw Error('Dirty tracked source; commit first or use --allow-dirty for a non-publishable development report');
  const work = fs.mkdtempSync(path.join(os.tmpdir(),'panack-source-baseline-'));
  console.log(`Coverage workspace: ${work}`);
  const compiler = regular(path.join(checkout,'bootstrap/compiler-v9.bc'));
  const vm = regular(path.join(checkout,'panack-vm'));
  const plans = new Map(), groups = [], sessions = [], seen = new Set();
  const tool = path.join(work,'export.bc');
  command(['./panack','compile','tests/source_coverage/export.panack','-o',tool]);
  function plan(entry) {
    if (!plans.has(entry)) {
      console.log(`Plan: ${entry}`);
      const prefix = path.join(work,`plan-${plans.size}`);
      if (command(['./panack','run',tool,entry,prefix]) !== 'plan: wrote\n') throw Error('Unexpected export response');
      plans.set(entry,readPlan(prefix,entry,checkout,compiler));
    }
    return plans.get(entry);
  }
  function run(id,p,args,check) {
    console.log(`Collect: ${id}`);
    const directory = path.join(work,`session-${sessions.length}`);
    const stdout = command(['./panack','coverage-session',directory,p.prefix+'.bc',p.prefix+'.inv','--',...args]);
    fs.writeFileSync(path.join(work,`stdout-${sessions.length}.txt`),stdout);
    check(stdout);
    const session = readSession(directory,[p],seen);
    groups.push(observations(p,session.totals[0]));
    const records = fs.readdirSync(directory).sort().map(name=>[name,sha(regular(path.join(directory,name)))]);
    sessions.push({id,entry:p.entry,arguments:args.map(a=>a.startsWith(work)?'<workspace>/'+path.basename(a):a),
      artifact:sha(p.artifact),inventory:sha(p.inventory),stdoutHash:sha(Buffer.from(stdout)),nonce:session.nonce,executions:session.executions,records});
  }
  const empty = stdout=>{if(stdout !== '') throw Error('Unexpected fixture stdout');};
  run('scope',plan('tests/source_coverage/scope.panack'),[],empty);
  const cp = plan('src/compiler/main.panack');
  if (!cp.artifact.equals(compiler)) throw Error('Source compiler is not byte-identical to seed');
  for (const name of manifest.units) {
    const p = plan(`tests/runner/${name}_unit.panack`);
    run(`unit:${name}`,p,[],stdout=>{
      if (!/tests: [1-9][0-9]*, failures: 0\n$/.test(stdout) || /^FAIL /m.test(stdout)) throw Error(`Unit assertions failed: ${name}`);
    });
  }
  for (const name of manifest.functional) {
    const p = plan(`tests/functional/cases/${name}/main.panack`);
    const expectedFile = path.join(checkout,`tests/functional/cases/${name}/expected.stdout`);
    const expected = fs.existsSync(expectedFile) ? regular(expectedFile).toString() : '';
    run(`functional:${name}`,p,[],stdout=>{if(stdout!==expected) throw Error(`Functional stdout mismatch: ${name}`);});
  }
  for (const id of manifest.compiler) {
    if (id === 'bootstrap-fixed-point') {
      const artifact = path.join(work,'stage.bc');
      run(id,cp,['compile','src/compiler/main.panack','-o',artifact],()=>{});
      if (!regular(artifact).equals(compiler)) throw Error('Bootstrap fixed point changed');
    } else if (id === 'check-hello-world') {
      run(id,cp,['check','tests/functional/cases/hello_world/main.panack'],stdout=>{if(stdout!=='ok\n') throw Error('Check result changed');});
    } else if (id === 'disasm-hello-world') {
      run(id,cp,['disasm','tests/functional/cases/hello_world/main.panack'],stdout=>{if(!stdout.includes('print')) throw Error('Disassembly missing print');});
    } else throw Error('Unknown compiler manifest entry');
  }
  if (sessions.length !== 1+manifest.units.length+manifest.functional.length+manifest.compiler.length) throw Error('Incomplete test manifest');
  for (const p of plans.values()) for (const s of p.sources) if (sha(regular(path.join(checkout,s.path))) !== s.hash) throw Error('Source changed during collection');
  if (!compiler.equals(regular(path.join(checkout,'bootstrap/compiler-v9.bc'))) || !vm.equals(regular(path.join(checkout,'panack-vm')))) throw Error('Toolchain changed during collection');
  if (command(['git','rev-parse','HEAD']).trim() !== commit || !manifestBytes.equals(regular(path.join(checkout,'tests/source_coverage/manifest.json')))) throw Error('Checkout or manifest changed during collection');
  if (clean && command(['git','status','--porcelain','--untracked-files=all']).trim()) throw Error('Checkout became dirty during collection');
  const files = union(groups,manifest.eligible);
  const report = {schema:1,repository:'sproates/panackelty',branch:'next',commit,clean,generatedAt:new Date().toISOString(),
    compiler:sha(compiler),vm:sha(vm),manifestHash:sha(Buffer.from(JSON.stringify(manifest))),manifest,sessions,
    metrics:totals(files),components:Object.fromEntries(['compiler','bytecode','stdlib'].map(c=>[c,totals(files.filter(f=>f.path.startsWith(`src/${c}/`)))])),files};
  writeReport(output,report);
  console.log(JSON.stringify({output,clean,sessions:sessions.length,executions:sessions.reduce((n,s)=>n+s.executions,0),metrics:report.metrics},null,2));
  // Keep raw evidence locally for diagnosis; CI publishes only the bounded static report.
  return report;
}
module.exports = {baseline};
if (require.main === module) {
  const args = process.argv.slice(2), allowDirty = args.includes('--allow-dirty');
  const positional = args.filter(a=>a!=='--allow-dirty');
  if (positional.length > 1) throw Error('Usage: source_coverage_baseline.cjs [output] [--allow-dirty]');
  try { baseline(path.resolve(positional[0] || path.join(checkout,'build/source-coverage')),allowDirty); }
  catch (error) { console.error(error.message); process.exitCode=1; }
}
