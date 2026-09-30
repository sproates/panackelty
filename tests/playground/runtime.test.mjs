import {test} from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {spawnSync} from 'node:child_process';
import {compileAndRun,execute,sourceBytes,SOURCE_LIMIT} from '../../build/playground/runtime.mjs';
import {File} from '../../build/playground/vendor/index.js';
import {Playground} from '../../src/playground/controller.mjs';

const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'../..');
const module=await WebAssembly.compile(fs.readFileSync(path.join(root,'build/playground/vm.wasm')));
const compiler=fs.readFileSync(path.join(root,'bootstrap/compiler-v9.bc'));
const stdlib=JSON.parse(fs.readFileSync(path.join(root,'build/playground/stdlib.json')));
const run=source=>compileAndRun(module,compiler,stdlib,source);

test('existing compiler, exact numbers, Unicode and stdlib work',async()=>{
  const result=await run('import "stdlib/text"\nmain(): Void { print("λ🙂"); print((1/3 * 30).nat()); print((1/8).dec()); print(999999999999999999999999999999 + 1); print(text_ends_with("hello.panack", ".panack")) }');
  assert.equal(result.status,0);assert.equal(result.stderr,'');
  assert.equal(result.stdout,'λ🙂\n10\n0.125\n1000000000000000000000000000000\ntrue\n');
});
test('source diagnostics prevent runtime execution',async()=>{
  const phases=[];
  const r=await compileAndRun(module,compiler,stdlib,'main(): Void { n: Nat = "bad"; print(n) }',p=>phases.push(p));
  assert.equal(r.status,1);assert.match(r.stderr,/\/main.panack:1:/);assert.deepEqual(phases,['Compiling…']);
});
test('unsupported host and runtime writes fail explicitly',async()=>{
  const host=await run('import "stdlib/host"\nmain(): Void { print(host_decode_utf8(utf8_encode("hi"))) }');
  assert.equal(host.status,1);assert.match(host.stderr,/host capability unavailable/);
  const write=await run('main(): Void { write_file("/cannot-write", "data") }');
  assert.equal(write.status,1);assert.match(write.stderr,/could not write file/);
});
test('source bound measures UTF-8 bytes, output flood aborts',async()=>{
  assert.equal(sourceBytes('a'.repeat(SOURCE_LIMIT)).length,SOURCE_LIMIT);
  assert.throws(()=>sourceBytes('a'.repeat(SOURCE_LIMIT+1)),/Source exceeds/);
  assert.throws(()=>sourceBytes('🙂'.repeat(SOURCE_LIMIT/4+1)),/Source exceeds/);
  assert.throws(()=>sourceBytes(null),/Source exceeds/);
  await assert.rejects(run('main(): Void { while true { print("flood") } }'),/Output exceeds/);
});
test('compiler output filesystem has a hard artifact byte bound',async()=>{
  const source='main(): Void { mut text: Str = "a"; mut i: Nat = 0; while i < 21 { text = text + text; i = i + 1; } write_file("/target", text) }';
  const program=new File([]);
  const compiled=await execute(module,['run','/compiler.bc','compile','/main.panack','-o','/main.bc'],new Map([
    ['compiler.bc',new File(compiler,{readonly:true})],['main.panack',new File(new TextEncoder().encode(source),{readonly:true})],['main.bc',program]
  ]),{writable:program});
  assert.equal(compiled.status,0,compiled.stderr);
  const target=new File([]);
  await assert.rejects(execute(module,['run','/main.bc'],new Map([
    ['main.bc',new File(program.data,{readonly:true})],['target',target]
  ]),{writable:target}),/Compiled artifact exceeds 1 MiB/);
  assert(target.data.length<=1048576);
});
test('fixed VM corpus retains 131 exact results and 14 declared host rejections',async()=>{
  const dir=path.join(root,'tests/fixtures/vm_contracts');
  const manifest=JSON.parse(fs.readFileSync(path.join(dir,'manifest.json')));
  let exact=0,hosts=0;
  for(const entry of manifest){
    const bytes=Buffer.from(fs.readFileSync(path.join(dir,entry.name+'.hex'),'utf8').replace(/\s/g,''),'hex');
    const r=await execute(module,[entry.mode,'/fixture.bc'],new Map([['fixture.bc',new File(bytes,{readonly:true})]]));
    assert.equal(r.status,entry.status,entry.name);
    assert.equal(r.stdout,fs.readFileSync(path.join(dir,entry.name+'.stdout'),'utf8'),entry.name);
    const expected=fs.readFileSync(path.join(dir,entry.name+'.stderr'),'utf8');
    const declared=/^(BytecodeContractTests-forged_runtime_safety_failures_trap_in_the_oracle|NativeExecutionTests-native_vm_traps_on_forged_dynamic_failures)-[1-7]$/.test(entry.name);
    if(declared){assert.equal(r.status,1);assert.equal(r.stderr,'error: VM trap: host capability unavailable in browser playground\n');hosts++;}
    else{assert.equal(r.stderr,expected,entry.name);exact++;}
  }
  assert.equal(exact,131);assert.equal(hosts,14);
});
test('wrong bytecode version is rejected',async()=>{
  const bytes=Buffer.from(compiler);bytes[9]=8;
  const r=await execute(module,['check','/old.bc'],new Map([['old.bc',new File(bytes)]]));
  assert.equal(r.status,1);assert.match(r.stderr,/unsupported bytecode version/);
});
test('linear memory cannot grow beyond the configured maximum',async()=>{
  const imports=Object.fromEntries(WebAssembly.Module.imports(module).map(entry=>{
    assert.equal(entry.kind,'function');assert.equal(entry.module,'wasi_snapshot_preview1');
    return [entry.name,()=>0];
  }));
  const instance=await WebAssembly.instantiate(module,{wasi_snapshot_preview1:imports});
  assert.throws(()=>instance.exports.memory.grow(4097),RangeError);
});
test('native public CLI and WASI compiler emit identical bytecode',async()=>{
  const dir=fs.mkdtempSync(path.join(os.tmpdir(),'panack-playground-'));
  try{
    const source='main(): Void { print((1/3 * 30).nat()); print("λ🙂") }\n';
    fs.writeFileSync(path.join(dir,'main.panack'),source);
    const result=spawnSync('./panack',['compile',path.join(dir,'main.panack'),'-o',path.join(dir,'main.bc')],{cwd:root,encoding:'utf8'});
    assert.equal(result.status,0,result.stderr);
    const cli=spawnSync('./panack',['run',path.join(dir,'main.panack')],{cwd:root,encoding:'utf8'});
    assert.equal(cli.status,0,cli.stderr);assert.equal(cli.stdout,'10\nλ🙂\n');
    const output=new File([]);
    const compiled=await execute(module,['run','/compiler.bc','compile','/main.panack','-o','/main.bc'],new Map([
      ['compiler.bc',new File(compiler,{readonly:true})],['main.panack',new File(new TextEncoder().encode(source),{readonly:true})],['main.bc',output]
    ]),{writable:output});
    assert.equal(compiled.status,0,compiled.stderr);
    assert.deepEqual(Buffer.from(output.data),fs.readFileSync(path.join(dir,'main.bc')));
  }finally{fs.rmSync(dir,{recursive:true});}
});
test('controller cancellation, stale messages, errors and timeout',async()=>{
  const workers=[],events=[];
  const p=new Playground(e=>events.push(e),{timeout:10,createWorker:()=>{
    const worker={postMessage(){},terminate(){this.terminated=true;}};workers.push(worker);return worker;
  }});
  p.run('one');p.run('duplicate');assert.equal(workers.length,1);
  p.stop();assert(workers[0].terminated);
  p.run('two');const before=events.length;
  workers[0].onmessage({data:{type:'done',stdout:'stale'}});assert.equal(events.length,before);
  workers[1].onmessage({data:{type:'done',status:0,stdout:'fresh'}});assert(workers[1].terminated);assert.equal(events.at(-1).stdout,'fresh');
  p.run('timeout');await new Promise(resolve=>setTimeout(resolve,30));assert(workers[2].terminated);assert.match(events.at(-1).stderr,/Time limit/);
  p.run('error');workers[3].onerror({message:'load failed'});assert(workers[3].terminated);assert.equal(events.at(-1).stderr,'load failed');
});
