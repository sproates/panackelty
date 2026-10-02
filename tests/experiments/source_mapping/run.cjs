const fs=require('node:fs');
const os=require('node:os');
const path=require('node:path');
const {spawnSync}=require('node:child_process');
const assert=require('node:assert/strict');
const {performance}=require('node:perf_hooks');
const metadata=require('./metadata.cjs');
const root=process.cwd();
const compiler=path.resolve(process.argv[2]);
const observer=path.resolve(process.argv[3]);
const work=fs.mkdtempSync(path.join(os.tmpdir(),'panack-source-map-'));
const corpus=path.join(work,'sources');
fs.cpSync(path.join(__dirname,'fixtures'),corpus,{recursive:true});
const compilerIdentity=metadata.hash(fs.readFileSync(compiler));
let assertions=0;
function check(value,message){assert(value,message);assertions++;}
function command(program,args,expected=0){
  const run=spawnSync(program,args,{encoding:'utf8',timeout:30000,maxBuffer:8*1024*1024,cwd:root});
  assert.ifError(run.error);assert.equal(run.status,expected,run.stderr);return run;
}
function compile(name,mode='map') {
  const file=path.join(corpus,name+'.panack');
  const artifact=path.join(work,name+'.bc');
  const start=performance.now();
  const rows=command('./panack',['run',compiler,file,artifact,mode]).stdout;
  return {file,artifact,rows,bytes:fs.readFileSync(artifact),milliseconds:performance.now()-start};
}
function instructions(artifact) {
  const result=new Map();let name;
  for(const line of command('./panack',['disasm',artifact]).stdout.trim().split('\n')) {
    if(line.startsWith('FUNCTION|')) name=line.split('|')[1];
    else {const [pc,...opcode]=line.split('|');result.set(JSON.stringify([name,Number(pc)]),opcode.join('|'));}
  }
  return result;
}
function trap(artifact) {
  const fields=command(observer,[artifact]).stdout.trim().split('\t');
  check(fields[0]==='trap' && fields[3]==='VM trap: index is out of bounds','actual VM bounds trap');
  return {function:fields[1],pc:Number(fields[2])};
}
const measurements=[];
try {
  const cases=[['local','pick','local.panack',45,57,2],
    ['imported','imported_pick','lib/pick.panack',54,66,2],
    ['generic','pick_generic','generic.panack',134,146,3],
    ['nested','nested','nested.panack',63,84,2,4,'items[indices[index]]'],
    ['conditional','conditional','conditional.panack',52,93,2,8,'items[if index == 0 { 0 } else { index }]']];
  let baseline;
  for(const [name,fn,file,start,end,line,pc=2,expression='items[index]'] of cases) {
    const built=compile(name);
    const baselineArtifact=path.join(work,name+'-cli.bc');
    command('./panack',['compile',built.file,'-o',baselineArtifact]);
    check(built.bytes.equals(fs.readFileSync(baselineArtifact)),'experimental emission byte-identical to public CLI');
    const error=command('./panack',['run',built.file],1).stderr;
    check(error.includes('VM trap: index is out of bounds'),'public source execution trap retained');
    check(command('./panack',['run',built.artifact],1).stderr===error,'source and saved bytecode error agree');
    const point=trap(built.artifact);
    assert.deepEqual(point,{function:fn,pc});assertions++;
    const code=instructions(built.artifact);
    const map=metadata.create(built.rows,corpus,built.bytes,compilerIdentity);
    const location=metadata.lookup(map,built.bytes,compilerIdentity,corpus,code,point);
    assert.deepEqual(location,{function:fn,pc,file,start,end,line,column:3,expression});assertions++;
    check(!map.includes(root) && !map.includes(work),'no absolute checkout paths persisted');
    const repeated=compile(name);
    check(repeated.bytes.equals(built.bytes) && metadata.create(repeated.rows,corpus,repeated.bytes,compilerIdentity)===map,'deterministic emission and metadata');
    // Existing v9 rejects appended metadata, so an embedded format needs versioning.
    const extension=Buffer.concat([built.bytes,Buffer.from('PNSMAP1\n'),Buffer.from(map)]);
    const candidate=path.join(work,name+'-extension.bc');fs.writeFileSync(candidate,extension);
    command('./panack',['check',candidate],1);assertions++;
    check(extension.subarray(0,built.bytes.length).equals(built.bytes),'extension prototype retains executable bytes');
    measurements.push({name,bytecode_bytes:built.bytes.length,sidecar_bytes:Buffer.byteLength(map),
      embedded_candidate_bytes:extension.length});
    if(name==='local') baseline={...built,map,point,code};
  }
  const nestedSource=path.join(corpus,'nested.panack'),nestedOriginal=fs.readFileSync(nestedSource);
  fs.writeFileSync(nestedSource,nestedOriginal.toString().replace('nested([7], [2], 0)','nested([7], [2], 2)'));
  const inner=compile('nested'),innerPoint=trap(inner.artifact);
  check(innerPoint.pc===3,'nested inner indexing traps before outer PC 4');
  check(metadata.lookup(metadata.create(inner.rows,corpus,inner.bytes,compilerIdentity),inner.bytes,compilerIdentity,corpus,instructions(inner.artifact),innerPoint)===null,'inner indexing does not borrow outer attribution');
  fs.writeFileSync(nestedSource,nestedOriginal);
  const {map,bytes,point,code}=baseline;
  const unavailable=(raw,data=bytes,identity=compilerIdentity,at=point)=>{
    check(metadata.lookup(raw,data,identity,corpus,code,at)===null,'location unavailable for invalid or unsupported mapping');
  };
  for(const raw of [undefined,'','{','null','[]','x'.repeat(65537)]) unavailable(raw);
  unavailable(map,Buffer.concat([bytes,Buffer.from([0])]));
  unavailable(map,bytes,'wrong compiler');
  unavailable(map,bytes,compilerIdentity,{function:'main',pc:0});
  unavailable(map,bytes,compilerIdentity,{function:point.function,pc:point.pc+1});
  const changed=change=>{const {integrity,...value}=JSON.parse(map);change(value);unavailable(metadata.seal(value));};
  changed(m=>m.version=2);changed(m=>m.encoding='bytes');
  changed(m=>m.entries[0].pc=999999);changed(m=>m.entries[0].pc=1);changed(m=>m.entries[0].start=-1);
  changed(m=>m.entries[0].end=999999);changed(m=>m.entries[0].start=1.5);
  changed(m=>m.entries.push(m.entries[0]));changed(m=>m.sources.push(m.sources[0]));
  changed(m=>m.sources[0].path='../outside.panack');changed(m=>m.sources[0].path='/absolute.panack');
  changed(m=>m.sources[0].sha256='0'.repeat(64));
  changed(m=>m.entries[0].file='missing.panack');
  const corrupted=JSON.parse(map);corrupted.entries[0].start=0;
  unavailable(JSON.stringify(corrupted));
  // A checksum detects accidental damage; a coherent forgery needs trusted
  // producer provenance. This negative finding is deliberately kept executable.
  const {integrity,...forged}=corrupted;
  check(metadata.lookup(metadata.seal(forged),bytes,compilerIdentity,corpus,code,point)?.start===0,'integrity alone does not authenticate producer attribution');
  const source=path.join(corpus,'local.panack');const original=fs.readFileSync(source);
  fs.writeFileSync(source,Buffer.concat([Buffer.from('// stale source\n'),original]));unavailable(map);
  fs.writeFileSync(source,original);fs.renameSync(source,source+'.away');unavailable(map);
  fs.symlinkSync(source+'.away',source);unavailable(map);fs.unlinkSync(source);fs.renameSync(source+'.away',source);
  // A change to an unmapped dependency still invalidates the source snapshot.
  const dependency=path.join(corpus,'lib/pick.panack');const saved=fs.readFileSync(dependency);
  fs.appendFileSync(dependency,'\n// changed\n');unavailable(map);fs.writeFileSync(dependency,saved);
  // Positive execution: same body and instruction mapping, different argument.
  fs.writeFileSync(source,original.toString().replace('pick([7], 2)','pick([7], 0)'));
  const good=compile('local');command('./panack',['run',good.artifact]);
  check(command(observer,[good.artifact]).stdout.trim()==='completed','valid index has no fabricated trap');
  fs.writeFileSync(source,original);
  // Unsupported non-tail index must not borrow a nearby location.
  fs.writeFileSync(path.join(corpus,'unsupported.panack'),'pure other(items: [Nat], index: Nat): Nat { value: Nat = items[index]; value }\nmain(): Void { value: Nat = other([7], 2); }\n');
  const unsupported=compile('unsupported');const other=trap(unsupported.artifact);
  check(unsupported.rows==='','unsupported AST shape has no map rows');
  const unsupportedMap=metadata.create(unsupported.rows,corpus,unsupported.bytes,compilerIdentity);
  check(metadata.lookup(unsupportedMap,unsupported.bytes,compilerIdentity,corpus,instructions(unsupported.artifact),other)===null,'unsupported failing instruction has no location');
  fs.unlinkSync(path.join(corpus,'unsupported.panack'));
  // An earlier indexing trap in the same function must not borrow its mapped tail.
  fs.writeFileSync(path.join(corpus,'earlier.panack'),'pure other(items: [Nat], index: Nat): Nat { value: Nat = items[index]; items[index] }\nmain(): Void { value: Nat = other([7], 2); }\n');
  const earlier=compile('earlier');const earlierPoint=trap(earlier.artifact);
  check(earlier.rows!=='','later tail index is mapped');
  check(metadata.lookup(metadata.create(earlier.rows,corpus,earlier.bytes,compilerIdentity),earlier.bytes,compilerIdentity,corpus,instructions(earlier.artifact),earlierPoint)===null,'earlier trap does not borrow the tail range');
  fs.unlinkSync(path.join(corpus,'earlier.panack'));

  // Reordered function names: instruction identity must not depend on table order.
  fs.writeFileSync(source,original.toString().replace('pure pick','pure aaa(): Nat { 1 }\npure pick'));
  const shifted=compile('local');const shiftedPoint=trap(shifted.artifact);
  check(shiftedPoint.function==='pick' && shiftedPoint.pc===2,'function identity survives table reorder');
  unavailable(map,shifted.bytes);
  const shiftedMap=metadata.create(shifted.rows,corpus,shifted.bytes,compilerIdentity);
  check(metadata.lookup(shiftedMap,shifted.bytes,compilerIdentity,corpus,instructions(shifted.artifact),shiftedPoint)?.expression==='items[index]','fresh map survives table reorder');
  fs.writeFileSync(source,original);
  const relocated=path.join(work,'relocated');fs.cpSync(corpus,relocated,{recursive:true});
  const relocatedArtifact=path.join(work,'relocated.bc');
  const relocatedRows=command('./panack',['run',compiler,path.join(relocated,'local.panack'),relocatedArtifact,'map']).stdout;
  const relocatedBytes=fs.readFileSync(relocatedArtifact);
  check(relocatedBytes.equals(bytes) && metadata.create(relocatedRows,relocated,relocatedBytes,compilerIdentity)===map,'relocation preserves bytes and metadata');
  check(metadata.lookup(map,bytes,compilerIdentity,relocated,code,point)?.start===45,'relative map resolves in relocated corpus');
  fs.writeFileSync(path.join(corpus,'timing.panack'),original.toString().replace('value: Nat = pick([7], 2);','mut i: Nat = 0; while i < 1000 { value: Nat = pick([7], 0); i = i + 1; } value: Nat = pick([7], 2);'));
  const timing=compile('timing');
  const timingCode=instructions(timing.artifact),timingPoint=trap(timing.artifact);
  const plain=[],mapped=[],normal=[],observed=[],serialized=[],resolved=[];
  for(let i=0;i<5;i++) {
    plain.push(compile('local','plain').milliseconds);mapped.push(compile('local').milliseconds);
    let start=performance.now();check(command(observer,[timing.artifact,'--bulk']).stdout.trim()==='trapped','bulk execution reaches trap');normal.push(performance.now()-start);
    start=performance.now();trap(timing.artifact);observed.push(performance.now()-start);
    start=performance.now();const timingMap=metadata.create(timing.rows,corpus,timing.bytes,compilerIdentity);serialized.push(performance.now()-start);
    start=performance.now();check(metadata.lookup(timingMap,timing.bytes,compilerIdentity,corpus,timingCode,timingPoint)!==null,'timed lookup resolves');resolved.push(performance.now()-start);
  }
  const median=xs=>[...xs].sort((a,b)=>a-b)[Math.floor(xs.length/2)];
  console.log(JSON.stringify({assertions,measurements,timing_ms:{compile_plain:plain,compile_mapped:mapped,
    run_bulk:normal,run_observed:observed,serialize:serialized,lookup:resolved,median_compile_delta:median(mapped)-median(plain),
    median_observer_delta:median(observed)-median(normal)}},null,2));
} finally {fs.rmSync(work,{recursive:true,force:true});}
