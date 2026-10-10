'use strict';
// Compare every emitted source item against the accepted self-hosted SC4 renderer.
const fs=require('node:fs'),os=require('node:os'),path=require('node:path'),assert=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const {regular,readPlan,readSession,observations}=require('../../scripts/source_coverage.cjs');
const checkout=path.resolve(__dirname,'../..'),work=fs.mkdtempSync(path.join(os.tmpdir(),'source-parity-'));
const testCompiler=process.env.PANACK_TEST_COMPILER;
const run=(args,env)=>{
  const childEnv=testCompiler?{...process.env,...env,PANACKELTY_COMPILER_PATH:testCompiler}:env;
  const executable=testCompiler?'./panack-vm':'./panack';
  const command=testCompiler?(args[0]==='coverage-session'?args:['run',testCompiler,...args]):args;
  return execFileSync(executable,command,{cwd:checkout,env:childEnv,encoding:'utf8',timeout:180000,maxBuffer:16*1024*1024});
};
const entry='tests/source_coverage/golden.panack',prefix=path.join(work,'golden'),tool=path.join(work,'export.bc');
run(['compile','tests/source_coverage/export.panack','-o',tool],{...process.env,PANACKELTY_BOOTSTRAP_ROOT:checkout});run(['run',tool,entry,prefix]);
const compilerIdentity=testCompiler||path.join(checkout,'bootstrap/compiler-v9.bc');
const p=readPlan(prefix,entry,checkout,regular(compilerIdentity)),seen=new Set();
for(const value of ['true','false']) {
  const directory=path.join(work,value);
  assert.equal(run(['coverage-session',directory,prefix+'.bc',prefix+'.inv','--',value]),value==='true'?'2\n21\n':'3\n27\n');
  const observed=observations(p,readSession(directory,[p],seen).totals[0]);
  const text=run(['coverage-aggregate','--source',entry,prefix+'.inv','--session',directory]);
  const rows=new Map(text.trimEnd().split('\n').filter(row=>row.includes('\t')).map(row=>{const columns=row.split('\t');return [columns[0]+'\t'+columns[1],columns];}));
  let compared=0;
  for(const source of observed) for(const i of source.items) {
    const name=source.path.startsWith('src/stdlib/')?'stdlib/'+source.path.slice(11):'project/'+path.relative(path.dirname(path.resolve(checkout,entry)),path.resolve(checkout,source.path));
    const row=rows.get(name+'\t'+i.id);assert(row,`Missing renderer item: ${name} ${i.id}`);
    assert.deepEqual(row.slice(2),[i.kind,i.detail,i.state,String(i.attempted),String(i.completed)],`${name} ${i.id}`);compared++;
  }
  console.log(`SC4 parity ${value}: ${compared} exact source items`);
}
console.log(`Parity evidence: ${work}`);
