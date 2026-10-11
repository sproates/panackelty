// Independent native-PC evidence against the production CLI. The decoder below
// exists only to construct adversarial maps; it never accepts a lookup result.
const fs=require('node:fs'), os=require('node:os'), path=require('node:path');
const {spawnSync}=require('node:child_process');
const assert=require('node:assert/strict');
const {performance}=require('node:perf_hooks');
const observer=path.resolve(process.argv[2]);
const work=fs.mkdtempSync(path.join(os.tmpdir(),'panack-source-map-'));
const corpus=path.join(work,'sources');
const project=path.join(corpus,'project');
fs.cpSync(path.join(__dirname,'fixtures'),corpus,{recursive:true});
fs.mkdirSync(path.join(project,'lib'),{recursive:true});
fs.copyFileSync(path.join(__dirname,'fixtures','lib','pick.panack'),path.join(project,'lib','pick.panack'));
let assertions=0, serial=0;
function check(value,message){assert(value,message);assertions++;}
function command(program,args,expected=0,env=process.env){
  const run=spawnSync(program,args,{encoding:'utf8',timeout:90000,maxBuffer:8*1024*1024,env});
  assert.ifError(run.error);assert.equal(run.status,expected,run.stderr);return run;
}
function compile(name,mode='map',directory=corpus) {
  const file=path.join(directory,name+'.panack');
  const artifact=path.join(work,`${name}-${serial++}.bc`), sidecar=artifact+'.pmap';
  const start=performance.now();
  command('./panack',['compile',file,'-o',artifact,...(mode==='map'?['--source-map',sidecar]:[])]);
  return {file,artifact,sidecar,bytes:fs.readFileSync(artifact),
    map:mode==='map'?fs.readFileSync(sidecar):null,milliseconds:performance.now()-start};
}
function locate(built,point,source=built.file) {
  return command('./panack',['locate',built.artifact,'--source',source,'--source-map',built.sidecar,
    '--function',point.function,'--instruction',String(point.pc)]).stdout;
}
function trap(artifact) {
  const fields=command(observer,[artifact]).stdout.trim().split('\t');
  check(fields[0]==='trap' && fields[3]==='VM trap: index is out of bounds','actual VM bounds trap');
  return {function:fields[1],pc:Number(fields[2])};
}
function expectLocation(built,point,file,start,end,line,column,expression) {
  const text=`source-map: expression\nfunction: ${point.function}\ninstruction: ${point.pc}\nfile: project/${file}\nrange: ${start}..${end}\nstart: ${line}:${column}\nend: ${line}:${column+end-start}\nexpression: ${expression}\n`;
  const actual=locate(built,point);
  check(actual===text,`exact ${file} ${point.pc}: ${actual}`);
}
// Positions in a known-good file, for controlled forged/duplicate/truncated
// mutations. Production never parses attacker-controlled fields this way.
function layout(data) {
  let offset=Buffer.byteLength('PANACKMAP1\nlocal-replay-v9\n');
  const u32=()=>{const value=data.readUInt32BE(offset);offset+=4;return value;};
  const blob=()=>{const lengthOffset=offset,length=u32(),start=offset;offset+=length;return {lengthOffset,start,end:offset,text:data.subarray(start,offset).toString()};};
  const entry=blob(),artifact=blob(),sourceCount=offset,sources=[];
  for(let n=u32();n>0;n--) sources.push({name:blob(),text:blob()});
  const functionCount=offset,functions=[];
  for(let n=u32();n>0;n--) {
    const name=blob(),count=offset,entries=[];
    for(let m=u32();m>0;m--){entries.push(offset);offset+=33;}
    functions.push({name,count,entries});
  }
  assert.equal(offset,data.length);
  return {entry,artifact,sourceCount,sources,functionCount,functions};
}
const measurements=[];
try {
  const cases=[['callback','select','callback.panack',33,43,2,3,'[7][index]'],
    ['local','pick','local.panack',45,57,2],
    ['imported','imported_pick','lib/pick.panack',58,70,2],
    ['generic','pick_generic','generic.panack',134,146,3],
    ['nested','nested','nested.panack',63,84,2,4,'items[indices[index]]'],
    ['conditional','conditional','conditional.panack',52,93,2,8,'items[if index == 0 { 0 } else { index }]']];
  let baseline;
  for(const [name,fn,file,start,end,line,pc=2,expression='items[index]'] of cases) {
    const built=compile(name),plain=compile(name,'plain');
    check(built.bytes.equals(plain.bytes),'mapping leaves v9 bytes unchanged');
    const error=command('./panack',['run',built.file],1).stderr;
    check(error.includes('VM trap: index is out of bounds'),'public source execution trap retained');
    check(command('./panack',['run',built.artifact],1).stderr===error,'source and artifact failure agree');
    const point=trap(built.artifact);
    const runtimeFunction=`$module1_${fn}`;
    check(point.function===runtimeFunction && point.pc===pc,
      `actual function and instruction: ${point.function}/${point.pc}`);
    const mappedPoint={...point,function:fn};
    expectLocation(built,mappedPoint,file,start,end,line,3,expression);
    check(!built.map.includes(Buffer.from(process.cwd())) && !built.map.includes(Buffer.from(work)),'portable metadata identifiers');
    const repeated=compile(name);
    check(built.bytes.equals(repeated.bytes) && built.map.equals(repeated.map),'deterministic pair');
    const extension=path.join(work,name+'-extension.bc');
    fs.writeFileSync(extension,Buffer.concat([built.bytes,built.map]));
    command('./panack',['check',extension],1);assertions++;
    measurements.push({name,bytecode_bytes:built.bytes.length,sidecar_bytes:built.map.length,compile_ms:built.milliseconds});
    if(name==='local') baseline={...built,point:mappedPoint};
  }
  const b=baseline, unavailable=(built=b,point=b.point)=>check(locate(built,point)==='source-map: unavailable\n','unavailable, never guessed');
  const shape=layout(b.map), pick=shape.functions.find(f=>f.name.text==='$module1_pick'), entry=pick.entries[2];
  const damaged=path.join(work,'damaged.pmap');
  const reject=raw=>{fs.writeFileSync(damaged,raw);unavailable({...b,sidecar:damaged});};
  for(const raw of [Buffer.alloc(0),Buffer.from('{'),Buffer.from('null'),Buffer.from('[]'),Buffer.from([255]),
      Buffer.alloc(1024*1024),b.map.subarray(0,b.map.length-1),Buffer.concat([b.map,Buffer.from('extra')])]) reject(raw);
  for(const [offset,value] of [[0,0],[8,50],[shape.entry.lengthOffset,255],[shape.sourceCount,255],
      [shape.functionCount,255],[pick.count,255],[entry,255],[entry+4,255],[entry+8,255],[entry+20,255],[entry+32,2]]) {
    const changed=Buffer.from(b.map);changed[offset]=value;reject(changed);
  }
  // Structurally valid, coherent lies: no checksum/signature can make them trusted.
  for(const [offset,value] of [[entry,1],[entry,999999],[entry+4,0],[entry+8,0],[entry+20,1],
      [entry+12,1],[entry+16,1],[entry+24,1],[entry+28,1]]) {
    const changed=Buffer.from(b.map);changed.writeUInt32BE(b.map.readUInt32BE(offset)===value?value+1:value,offset);reject(changed);
  }
  const changedKind=Buffer.from(b.map);changedKind[entry+32]=1;reject(changedKind);
  for(const name of ['../outside.panack','/absolute.panack','stdlib/forged.panack']) {
    const source=shape.sources.find(s=>s.name.text==='project/local.panack').name;
    const bytes=Buffer.from(name),length=Buffer.alloc(4);length.writeUInt32BE(bytes.length);
    reject(Buffer.concat([b.map.subarray(0,source.lengthOffset),length,bytes,b.map.subarray(source.end)]));
  }
  // Duplicate source and entry records while updating counts coherently.
  const source=shape.sources[0],sourceEnd=source.text.end;
  const duplicateSource=Buffer.concat([b.map.subarray(0,sourceEnd),b.map.subarray(source.name.lengthOffset,sourceEnd),b.map.subarray(sourceEnd)]);
  duplicateSource.writeUInt32BE(shape.sources.length+1,shape.sourceCount);reject(duplicateSource);
  const duplicateEntry=Buffer.concat([b.map.subarray(0,entry+33),b.map.subarray(entry,entry+33),b.map.subarray(entry+33)]);
  duplicateEntry.writeUInt32BE(pick.entries.length+1,pick.count);reject(duplicateEntry);
  unavailable({...b,sidecar:path.join(work,'missing')});
  unavailable({...b,artifact:path.join(work,'missing.bc')});
  const wrong=path.join(work,'wrong.bc');fs.writeFileSync(wrong,Buffer.concat([b.bytes,Buffer.from([0])]));unavailable({...b,artifact:wrong});
  unavailable(b,{function:'unknown',pc:2});unavailable(b,{function:'pick',pc:3});unavailable(b,{function:'pick',pc:999999});
  const original=fs.readFileSync(b.file);
  for(const data of [Buffer.concat([Buffer.from('// changed\n'),original]),Buffer.from([255]),Buffer.alloc(1048577,32)]) {
    fs.writeFileSync(b.file,data);unavailable();
  }
  fs.writeFileSync(b.file,original);fs.renameSync(b.file,b.file+'.away');unavailable();
  // Explicit-source symlinks are safe: the exact captured bytes are reproduced.
  fs.symlinkSync(b.file+'.away',b.file);expectLocation(b,b.point,'local.panack',45,57,2,3,'items[index]');
  fs.unlinkSync(b.file);fs.renameSync(b.file+'.away',b.file);
  // Unloaded files no longer belong to the source closure; transitive loaded
  // dependencies are tested in the canonical self-hosted CLI suite.
  const unrelated=path.join(project,'unrelated.panack');fs.writeFileSync(unrelated,'broken syntax');
  expectLocation(b,b.point,'local.panack',45,57,2,3,'items[index]');fs.unlinkSync(unrelated);
  const relocated=path.join(work,'relocated');fs.cpSync(corpus,relocated,{recursive:true});
  const moved=compile('local','map',relocated);
  check(moved.bytes.equals(b.bytes) && moved.map.equals(b.map),'relocation preserves exact pair');
  check(locate(b,b.point,moved.file)===locate(b,b.point),'original pair validates in relocated tree');
  const nestedFile=path.join(corpus,'nested.panack'),nested=fs.readFileSync(nestedFile);
  fs.writeFileSync(nestedFile,nested.toString().replace('nested([7], [2], 0)','nested([7], [2], 2)'));
  const inner=compile('nested'),innerPoint=trap(inner.artifact);
  check(innerPoint.pc===3 && locate(inner,{...innerPoint,function:'nested'}).includes('expression: indices[index]\n'),'inner trap retains smallest expression');
  fs.writeFileSync(nestedFile,nested);
  fs.writeFileSync(b.file,original.toString().replace('pick([7], 2)','pick([7], 0)'));
  const good=compile('local');command('./panack',['run',good.artifact]);
  check(command(observer,[good.artifact]).stdout.trim()==='completed','valid index has no fabricated trap');
  unavailable({...b,artifact:good.artifact});unavailable({...b,sidecar:good.sidecar});
  fs.writeFileSync(b.file,original);
  for(const tail of ['value','items[index]']) {
    fs.writeFileSync(path.join(corpus,'earlier.panack'),`pure other(items: [Nat], index: Nat): Nat { value: Nat = items[index]; ${tail} }\nmain(): Void { value: Nat = other([7], 2); }\n`);
    const earlier=compile('earlier');
    const earlierPoint={...trap(earlier.artifact),function:'other'};
    check(locate(earlier,earlierPoint).includes('range: 57..69\n'),'binding trap uses own range, never later tail');
  }
  fs.writeFileSync(b.file,original.toString().replace('pure pick','pure aaa(): Nat { 1 }\npure pick'));
  const shifted=compile('local'),point=trap(shifted.artifact);
  const shiftedMapPoint={...point,function:'pick'};
  check(point.function==='$module1_pick' && point.pc===2 && locate(shifted,shiftedMapPoint).includes('expression: items[index]\n'),'function identity survives table reorder');
  unavailable();fs.writeFileSync(b.file,original);
  // No automatic execution: a lookup of a trapping program succeeds above.
  // Reproducible core snapshot is required, including unused declarations.
  const stdlib=path.join(work,'stdlib');fs.mkdirSync(stdlib);fs.copyFileSync('src/stdlib/core.panack',path.join(stdlib,'core.panack'));
  const env={...process.env,PANACKELTY_STDLIB_PATH:stdlib};
  const nativeLocate=()=>command('./panack-vm',['run','bootstrap/compiler-v9.bc','locate',b.artifact,'--source',b.file,'--source-map',b.sidecar,'--function','pick','--instruction','2'],0,env).stdout;
  check(nativeLocate()===locate(b,b.point),'stdlib relocation preserves identity');
  fs.appendFileSync(path.join(stdlib,'core.panack'),'\n// changed unused core\n');
  check(nativeLocate()==='source-map: unavailable\n','implicit core changes invalidate');
  const plain=[],mapped=[],lookup=[];
  for(let i=0;i<5;i++) {
    plain.push(compile('local','plain').milliseconds);mapped.push(compile('local').milliseconds);
    const start=performance.now();expectLocation(b,b.point,'local.panack',45,57,2,3,'items[index]');lookup.push(performance.now()-start);
  }
  console.log(JSON.stringify({assertions,measurements,timing_ms:{compile_plain:plain,compile_mapped:mapped,lookup}},null,2));
} finally {fs.rmSync(work,{recursive:true,force:true});}
