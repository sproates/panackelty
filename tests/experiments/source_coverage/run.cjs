// Bounded feasibility harness. Only locally reproduced maps are decoded. This
// is not a production report reader, collection format or coverage percentage.
const fs = require('node:fs'), os = require('node:os'), path = require('node:path');
const assert = require('node:assert/strict');
const {spawnSync} = require('node:child_process');
const {performance} = require('node:perf_hooks');
const observer = path.resolve(process.argv[2]);
const work = fs.mkdtempSync(path.join(os.tmpdir(), 'panack-coverage-'));
const sources = path.join(work, 'sources');
fs.mkdirSync(sources, {recursive:true});
fs.copyFileSync(path.join(__dirname, 'fixtures', 'basic.panack'), path.join(sources, 'basic.panack'));
const projectLibrary = path.join(sources, 'lib');
fs.mkdirSync(projectLibrary, {recursive:true});
fs.copyFileSync(path.join(__dirname, 'fixtures', 'lib', 'helper.panack'), path.join(projectLibrary, 'helper.panack'));
let serial = 0, checks = 0;
function check(condition, message) { assert(condition, message); checks++; }
function functionIdentity(names, sourceName) {
  const matches = [...names].filter(name => name === sourceName || name.endsWith('_' + sourceName));
  assert.equal(matches.length, 1, `one emitted identity for ${sourceName}`);
  return matches[0];
}
function run(command, args, status = 0) {
  const result = spawnSync(command, args, {encoding:'utf8', timeout:90000, maxBuffer:32*1024*1024});
  assert.ifError(result.error);
  assert.equal(result.status, status, result.stderr + result.stdout);
  return result;
}
function compile(file, mapped = true) {
  const artifact = path.join(work, `build-${serial++}.bc`), sidecar = artifact + '.pmap';
  const start = performance.now();
  run('./panack', ['compile', file, '-o', artifact, ...(mapped ? ['--source-map', sidecar] : [])]);
  return {file, artifact, sidecar, ms:performance.now()-start};
}
function fixture(name, source) {
  const file = path.join(sources, name + '.panack');
  fs.writeFileSync(file, source);
  return compile(file);
}
function exactFile(file, expected) {
  let fd;
  try {
    // Do not read a foreign length or allocate from a foreign field.
    fd = fs.openSync(file,fs.constants.O_RDONLY | fs.constants.O_NONBLOCK);
    if (!fs.fstatSync(fd).isFile()) return false;
    const bytes = Buffer.alloc(expected.length + 1);
    let size = 0, got;
    while (size < bytes.length && (got = fs.readSync(fd,bytes,size,bytes.length-size,null))) size += got;
    return size === expected.length && bytes.subarray(0,size).equals(expected);
  } catch { return false; }
  finally { if (fd !== undefined) fs.closeSync(fd); }
}
function validate(built) {
  const reproduced = compile(built.file);
  const artifact = fs.readFileSync(reproduced.artifact), map = fs.readFileSync(reproduced.sidecar);
  return exactFile(built.artifact, artifact) && exactFile(built.sidecar, map) ? map : null;
}
function decodeLocal(map) {
  const magic = 'PANACKMAP1\nlocal-replay-v9\n';
  assert.equal(map.subarray(0, magic.length).toString(), magic);
  let offset = magic.length;
  const u32 = () => { const n = map.readUInt32BE(offset); offset += 4; return n; };
  const blob = () => { const n = u32(), bytes = map.subarray(offset, offset+n); offset += n; return bytes; };
  blob(); blob();
  const snapshots = [];
  for (let n = u32(); n > 0; n--) snapshots.push({name:blob().toString(), text:blob().toString()});
  const functions = [];
  for (let n = u32(); n > 0; n--) {
    const name = blob().toString(), entries = [];
    for (let m = u32(); m > 0; m--) {
      const position = offset;
      const [pc, source, start, line, column, end, endLine, endColumn] = Array.from({length:8}, u32);
      const lowered = map[offset++];
      entries.push({pc, source, start, line, column, end, endLine, endColumn, lowered, position});
    }
    functions.push({name, entries});
  }
  assert.equal(offset, map.length);
  return {snapshots, functions};
}
function parseReport(text) {
  const rows = text.trimEnd().split('\n').map(line => line.split('\t'));
  if (rows.shift()?.[0] !== 'coverage-probe-v1') return null;
  const end = rows.pop();
  if (end?.length !== 10 || end[0] !== 'end' ||
      ![0,1,2,3,4,5].includes(+end[1]) ||
      !['none','step-limit','nested-execution','unsupported-host-wait'].includes(end[2]) ||
      !['Nat','other'].includes(end[3]) ||
      [end[4],end[5],end[7],end[8],end[9]].some(n=>!/^\d+$/.test(n) || !Number.isSafeInteger(+n)) ||
      !Number.isFinite(+end[6]) || +end[6] < 0) return null;
  const functions = new Map(), names = new Map(), pcs = new Map();
  for (const row of rows) {
    const numbers = row[0] === 'function' ? row.slice(1,3) : row.slice(1);
    if (numbers.some(n => !/^\d+$/.test(n) || !Number.isSafeInteger(Number(n)))) return null;
    if (row[0] === 'function' && row.length === 4 && !functions.has(+row[1]) && /^(?:[0-9a-f]{2})+$/.test(row[3])) {
      const name = Buffer.from(row[3],'hex').toString();
      if (names.has(name)) return null;
      functions.set(+row[1], +row[2]); names.set(name,+row[1]);
    }
    else if (row[0] === 'pc' && row.length === 6 && !pcs.has(row[1]+':'+row[2])) {
      if (+row[4] + +row[5] > +row[3]) return null;
      pcs.set(row[1]+':'+row[2], {hits:+row[3], yes:+row[4], no:+row[5]});
    } else return null;
  }
  if (functions.size !== +end[8] || pcs.size !== +end[9]) return null;
  for (let i = 0; i < functions.size; i++) if (!functions.has(i)) return null;
  for (const key of pcs.keys()) {
    const [fn,pc] = key.split(':').map(Number);
    if (!functions.has(fn) || (pc > 0 && !pcs.has(fn+':'+(pc-1)))) return null;
  }
  return {functions, names, pcs, status:+end[1], gap:end[2], kind:end[3], value:+end[4], waits:+end[5], cpu:+end[6], bytes:+end[7]};
}
function observe(built, mode = 'count', status = 0) {
  const report = path.join(work, `run-${serial++}.txt`);
  const result = run(observer, [built.artifact, report, mode], status);
  const text = fs.readFileSync(report, 'utf8'), parsed = parseReport(text);
  assert(parsed, 'complete observer envelope');
  return {...parsed, text, stdout:result.stdout, stderr:result.stderr};
}
function sourceReach(map, execution) {
  const lines = new Map(), functions = new Map();
  map.functions.forEach(fn => {
    const index = execution.names.get(fn.name);
    assert.notEqual(index, undefined, 'map function must exist in artifact');
    const direct = fn.entries.filter(e => !e.lowered);
    // Empty/unmapped bodies have no proved source-function denominator yet.
    if (direct.length) functions.set(fn.name, execution.functions.get(index));
    for (const entry of direct) {
      const source = map.snapshots[entry.source].name;
      const key = source + ':' + entry.line;
      const counter = execution.pcs.get(index+':'+entry.pc);
      assert(counter, 'mapped PC must exist even when zero');
      lines.set(key, (lines.get(key) || false) || counter.hits > 0);
    }
  });
  return {lines, functions};
}
try {
  const basic = compile(path.join(sources, 'basic.panack'));
  const plain = compile(basic.file, false), validated = validate(basic);
  check(validated !== null, 'exact local reproduction');
  check(fs.readFileSync(basic.artifact).equals(fs.readFileSync(plain.artifact)), 'bytecode unchanged');
  const map = decodeLocal(validated), counted = observe(basic), bulk = observe(basic, 'bulk');
  check(counted.value === 16 && bulk.value === 16 && counted.gap === 'none', 'exact result and complete collection');
  const reach = sourceReach(map, counted);
  const ownFunctions = ['choose', 'unused', 'main', 'identity', 'unused_import'];
  assert.deepEqual(ownFunctions.map(name => reach.functions.get(functionIdentity(reach.functions.keys(), name))), [2,0,1,1,0]); checks++;
  // Manual oracle for this tiny fixture's verified disassembly: both arms run
  // once, load/test/return twice; unused bodies remain all-zero.
  for (const [name, expected] of [
    ['identity',[1,1]], ['unused_import',[0,0]], ['choose',[2,2,1,1,1,2]],
    ['unused',[0,0]], ['main',[1,1,1,1,1,1,1,1,1,1,1,1,1]],
  ]) {
    const identity = functionIdentity(counted.names.keys(), name);
    const actual = [...counted.pcs].filter(([key])=>key.startsWith(counted.names.get(identity)+':')).map(([,c])=>c.hits);
    assert.deepEqual(actual,expected); checks++;
  }
  const ownLines = [...reach.lines].filter(([key]) => key.startsWith('project/')).sort();
  const expected = [
    ['project/basic.panack:4',true], ['project/basic.panack:5',true], ['project/basic.panack:7',true],
    ['project/basic.panack:11',false], ['project/basic.panack:14',true], ['project/basic.panack:15',true],
    ['project/basic.panack:16',true], ['project/basic.panack:17',true],
    ['project/lib/helper.panack:2',true], ['project/lib/helper.panack:5',false],
  ].sort();
  assert.deepEqual(ownLines, expected); checks++;
  const chooseIndex = map.functions.findIndex(fn => fn.name === 'choose' || fn.name.endsWith('_choose'));
  const chooseIdentity = functionIdentity(counted.names.keys(), 'choose');
  const branches = [...counted.pcs].filter(([key,c]) => key.startsWith(counted.names.get(chooseIdentity)+':') && c.yes+c.no);
  check(branches.length === 1 && branches[0][1].yes === 1 && branches[0][1].no === 1, 'both VM conditional outcomes once');
  check(map.functions.some(fn => fn.entries.some(e => e.lowered)), 'lowered entries explicit');
  check([...counted.pcs].some(([key]) => {
    const [fn,pc] = key.split(':').map(Number);
    const name = [...counted.names].find(([,index]) => index === fn)[0];
    return !map.functions.find(f => f.name === name).entries.some(e => e.pc === pc);
  }), 'unmapped instruction denominator remains visible');
  const samples = {plainCompile:[plain.ms], mappedCompile:[basic.ms], bulk:[], count:[]};
  // Corrupt and coherently forged metadata must never reach decodeLocal.
  const original = fs.readFileSync(basic.sidecar);
  const changed = Buffer.from(original);
  const entry = map.functions[chooseIndex].entries[0];
  changed.writeUInt32BE(entry.line+1, entry.position+12);
  for (const bytes of [Buffer.alloc(0), original.subarray(0, original.length-1), Buffer.concat([original,Buffer.from('extra')]), changed]) {
    fs.writeFileSync(basic.sidecar, bytes);
    check(validate(basic) === null, 'invalid metadata is unavailable, never zero');
  }
  fs.unlinkSync(basic.sidecar);
  check(validate(basic) === null, 'missing map unavailable');
  fs.writeFileSync(basic.sidecar, original);
  const helper = path.join(projectLibrary,'helper.panack'), helperText = fs.readFileSync(helper);
  fs.appendFileSync(helper, '\n// changed unused source\n');
  check(validate(basic) === null, 'stale imported snapshot unavailable');
  fs.writeFileSync(helper, helperText);
  check(validate({...basic,artifact:plain.sidecar}) === null, 'missing artifact unavailable');
  const wrong = fixture('wrong','main(): Nat { 17 }\n');
  check(validate({...basic,artifact:wrong.artifact}) === null, 'different artifact unavailable');
  check(parseReport(counted.text.slice(0,counted.text.lastIndexOf('end\t'))) === null, 'missing completion footer rejects run');
  const duplicate = counted.text.split('\n').find(line=>line.startsWith('pc\t'));
  check(parseReport(counted.text.replace('end\t', duplicate+'\nend\t')) === null, 'duplicate counters reject run');
  check(parseReport(counted.text.replace(/^pc[^\n]*\n/m,'')) === null, 'missing counter rejects run');
  const limited = observe(basic,'limit',1);
  check(limited.gap === 'step-limit', 'bounded partial run is incomplete');
  const trap = fixture('trap','main(): Nat { xs: Array[Nat] = [1]\n xs[2] }\n');
  const trapCount = observe(trap,'count',1), trapBulk = observe(trap,'bulk',1);
  check(trapCount.stderr === trapBulk.stderr && trapCount.stderr === run('./panack',['run',trap.artifact],1).stderr, 'trap semantics unchanged');
  const trapMap = decodeLocal(validate(trap));
  check([...sourceReach(trapMap,trapCount).lines].some(([key,hit]) => key === 'project/trap.panack:2' && hit), 'attempted trap expression is hit');
  const printed = fixture('printed','main(): Void { print(42) }\n');
  check(observe(printed).stdout === '42\n' && observe(printed,'bulk').stdout === '42\n' && run('./panack',['run',printed.artifact]).stdout === '42\n' && run('./panack',[printed.file]).stdout === '42\n', 'stdout equivalence with public CLI');
  const asynchronous = fixture('async', 'async child(): Result[Bytes,Str] { await async_fake_read(false) }\nasync main(): Unit { result = await child()\n () }\n');
  const asyncCount = observe(asynchronous), asyncBulk = observe(asynchronous,'bulk');
  check(asyncCount.waits === 1 && asyncBulk.waits === 1 && asyncCount.gap === 'none', 'fake async suspension resumes with complete counts');
  const asyncMap = decodeLocal(validate(asynchronous));
  const childIdentity = functionIdentity(asyncCount.names.keys(), 'child');
  check(sourceReach(asyncMap,asyncCount).functions.get(functionIdentity(asyncMap.functions.map(fn=>fn.name), 'child')) === 1, 'async child entry counted once across suspension');
  assert.deepEqual([...asyncCount.pcs].filter(([key])=>key.startsWith(asyncCount.names.get(childIdentity)+':')).map(([,c])=>c.hits),[1,1,1]); checks++;
  run('./panack',['run',asynchronous.artifact]); checks++;
  const childPath = JSON.stringify(printed.artifact);
  const nested = fixture('nested',`import stdlib/filesystem::{filesystem_read_file_bytes}\nimport stdlib/host::{host_run_bytecode}\nmain(): Void { raw: Bytes = filesystem_read_file_bytes(${childPath})\n host_run_bytecode(raw, []) }\n`);
  const nestedCount = observe(nested), nestedBulk = observe(nested,'bulk');
  check(nestedCount.stdout === '42\n' && nestedBulk.stdout === '42\n' && run('./panack',['run',nested.artifact]).stdout === '42\n', 'nested execution semantics preserved');
  check(nestedCount.gap === 'nested-execution', 'missing child execution explicitly incomplete');
  const indirect = fixture('indirect',`import stdlib/filesystem::{filesystem_read_file_bytes}\nimport stdlib/host::{host_run_bytecode}\nchild(raw: Bytes): Void { host_run_bytecode(raw, []) }\nmain(): Void { raw: Bytes = filesystem_read_file_bytes(${childPath})\n f: Fn[Bytes,Void] = @child\n f.call(raw) }\n`);
  check(observe(indirect).gap === 'nested-execution', 'indirect child also invalidates completeness');
  const compiledChild = path.join(work,'nested-output.bc');
  const compiler = fixture('compiler',`import stdlib/filesystem::{filesystem_read_file_bytes}\nimport stdlib/host::{host_run_bytecode}\nmain(): Void { raw: Bytes = filesystem_read_file_bytes(${JSON.stringify(path.resolve('bootstrap/compiler-v9.bc'))})\n host_run_bytecode(raw, ["compile", ${JSON.stringify(printed.file)}, "-o", ${JSON.stringify(compiledChild)}]) }\n`);
  const compilerCount = observe(compiler);
  check(compilerCount.gap === 'nested-execution' && fs.readFileSync(compiledChild).equals(fs.readFileSync(printed.artifact)), 'nested compiler preserves artifact but is not measured');
  const lowering = fixture('lowering',`enum Choice { First, Second, Third }
pure rhs(): Bool { true }
pure selected(): Nat { 7 }
pure skipped(): Nat { 99 }
pure match_choice(c: Choice): Nat { match c { First() => selected(), Second() => skipped(), Third() => 0 } }
main(): Nat {
  a: Bool = false && rhs()
  b: Bool = true || rhs()
  c: Bool = true && rhs()
  text: Str = identity[Str]("x")
  mut total: Nat = identity[Nat](0)
  for i in 0..3 { total = total + match_choice(First()); }
  total
}
`.replace('enum Choice', 'import project/lib/helper::{identity}\nenum Choice'));
  const lowerResult = observe(lowering), lowerMap = decodeLocal(validate(lowering));
  const lowerReach = sourceReach(lowerMap, lowerResult);
  check(lowerResult.value === 21 && observe(lowering,'bulk').value === 21, 'lowering result parity');
  assert.deepEqual(['rhs','selected','skipped','match_choice','identity'].map(name=>lowerReach.functions.get(functionIdentity(lowerReach.functions.keys(),name))),[1,3,0,3,2]); checks++;
  check(lowerMap.functions.some(fn=>fn.entries.some(e=>e.lowered)), 'match and short-circuit retain explicit lowering');
  const recursive = fixture('recursive','pure down(n: Nat): Nat { if n > 0 { down(n - 1) + 1 } else { 0 } }\nmain(): Nat { down(4) }\n');
  const recursiveResult = observe(recursive);
  check(recursiveResult.value === 4 && recursiveResult.functions.get(recursiveResult.names.get(functionIdentity(recursiveResult.names.keys(),'down'))) === 5, 'recursive activations count separately');
  const malformed = path.join(work,'invalid.bc'); fs.writeFileSync(malformed,'invalid');
  const missingReport = path.join(work,'never-created.txt');
  run(observer,[malformed,missingReport,'count'],2);
  check(!fs.existsSync(missingReport), 'invalid bytecode produces no valid collection');
  // The verifier accepts a RETURN elsewhere; the executed path can still fall
  // off a function. Observation must not read beyond the instruction array.
  const falloff = path.join(work,'falloff.bc');
  fs.writeFileSync(falloff,Buffer.from('50414e41434b4243000009000100046d61696e00000000000313000000021400000000','hex'));
  const fallen = observe({artifact:falloff},'count',1);
  check(fallen.status === 2 && fallen.stderr === observe({artifact:falloff},'bulk',1).stderr, 'falloff delegates safely to the VM trap');
  const loop = fixture('loop',fs.readFileSync(path.join(__dirname,'../runtime_provenance/loop.panack'),'utf8'));
  const loopMap = decodeLocal(validate(loop));
  for (let round = 0; round < 5; round++) {
    for (const mode of round % 2 ? ['count','bulk'] : ['bulk','count']) {
      const result = observe(loop,mode);
      check(result.value === 1260000 && result.gap === 'none', mode+' benchmark equivalence');
      samples[mode].push(result.cpu);
      if (mode === 'count') {
        check(sourceReach(loopMap,result).functions.get(functionIdentity(loopMap.functions.map(fn=>fn.name),'multiply')) === 30000, 'loop exact function entries');
        const branch = [...result.pcs.values()].filter(c => c.yes+c.no);
        check(branch.length === 1 && branch[0].yes === 30000 && branch[0].no === 1, 'exact loop decisions');
        check(result.bytes === observe(loop,'limit',1).bytes, 'counter memory independent of execution length');
      }
    }
  }
  for (let round = 0; round < 4; round++) {
    samples.plainCompile.push(compile(basic.file,false).ms);
    samples.mappedCompile.push(compile(basic.file).ms);
  }
  // Existing v9 rejects appended metadata. A real embedded design needs a new
  // format version and a loader/tool/bootstrap migration, not an opaque suffix.
  const extension = path.join(work,'extended.bc');
  fs.writeFileSync(extension,Buffer.concat([fs.readFileSync(basic.artifact),original]));
  run('./panack',['check',extension],1); checks++;
  console.log(JSON.stringify({checks, ownLines, functions:ownFunctions.map(name=>[name,reach.functions.get(functionIdentity(reach.functions.keys(),name))]),
    artifactBytes:fs.statSync(basic.artifact).size, sidecarBytes:original.length,
    counterBytes:counted.bytes, samples},null,2));
} finally { fs.rmSync(work,{recursive:true,force:true}); }
