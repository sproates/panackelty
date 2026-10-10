// Fixed semantic expectations, using the real CLI and a real-VM observer.
const fs = require('node:fs'), os = require('node:os'), path = require('node:path');
const {spawnSync} = require('node:child_process');
const assert = require('node:assert/strict');
const probe = path.resolve(process.argv[2]);
const work = fs.mkdtempSync(path.join(os.tmpdir(), 'panack-provenance-'));
let checks = 0;
function check(value, message) { assert(value, message); checks++; }
function run(command, args, status = 0) {
  const result = spawnSync(command, args, {encoding:'utf8', timeout:90000, maxBuffer:8*1024*1024});
  assert.ifError(result.error);
  assert.equal(result.status, status, result.stderr + result.stdout);
  return result.stdout;
}
function compile(name, source) {
  const file = path.join(work, name + '.panack'), artifact = file + '.bc';
  fs.writeFileSync(file, source);
  run('./panack', ['compile', file, '-o', artifact]);
  return {file, artifact};
}
function observe(built, mode='ring', capacity=128, values=true, dump=true, status=0) {
  const text = run(probe, [built.artifact, mode, String(capacity), values ? 'values' : 'redacted', ...(dump ? ['dump'] : [])], status);
  const lines = text.trim().split('\n').map(line => line.split('\t'));
  const summary = lines.find(line => line[0] === 'summary');
  const edge = text => {const [id,state] = text.split(':'); return {id:Number(id),state};};
  const events = lines.filter(line => line[0] === 'event').map(e => ({id:+e[1],fn:e[2],pc:+e[3],op:+e[4],code:+e[5],kind:e[6],value:+e[7],call:edge(e[8]),control:edge(e[9]),inputs:e.slice(10).map(edge)}));
  return {text,rss:+lines.find(line=>line[0]==='rss-bytes')[1], status:+summary[1],kind:summary[2],value:+summary[3],cpu:+summary[4],bytes:+summary[5],count:+summary[6],gap:summary[7],root:+summary[8],rootState:summary[9],events};
}
try {
  const derivation = compile('derivation', fs.readFileSync(path.join(__dirname,'derivation.panack'),'utf8'));
  const complete = observe(derivation);
  check(complete.value === 42 && complete.gap === 'none', 'actual function result');
  const byId = new Map(complete.events.map(e => [e.id,e]));
  const root = byId.get(complete.root), returned = byId.get(root.inputs[0].id), mul = byId.get(returned.inputs[0].id);
  const multiplyIdentity = returned.fn;
  check(root.fn === 'main' && multiplyIdentity.startsWith('$module') &&
    multiplyIdentity.endsWith('_multiply') && returned.op === 20, 'actual return chain');
  check(mul.op === 5 && mul.code === 2 && mul.value === 42, 'multiply instruction produced 42');
  check(mul.inputs.map(e => byId.get(e.id).value).join(',') === '6,7', 'actual operand snapshots');
  const call = byId.get(mul.call.id);
  check(call.op === 17 && call.inputs.map(e=>byId.get(e.id).value).join(',') === '6,7', 'call arguments linked');
  check(complete.events.every(e=>e.inputs.every(edge=>edge.state==='retained' && edge.id<e.id)), 'acyclic retained derivation');
  const mapped = derivation.artifact+'.mapped.bc', map = mapped+'.pmap';
  run('./panack',['compile',derivation.file,'-o',mapped,'--source-map',map]);
  check(fs.readFileSync(mapped).equals(fs.readFileSync(derivation.artifact)), 'mapping preserves bytecode');
  const location = run('./panack',['locate',mapped,'--source',derivation.file,'--source-map',map,'--function','multiply','--instruction',String(mul.pc)]);
  check(location.includes('expression: a * b\n'), 'actual arithmetic PC locates source expression');
  for (const mode of ['bulk','step','prefix','ring']) check(observe(derivation,mode).value===42, mode+' result parity');
  check(observe(derivation,'ring',1).events.length===1 && observe(derivation,'prefix',1).rootState==='discarded', 'one-slot boundary');
  check(observe(derivation,'ring',65536,false,false).value===0, 'maximum accepted capacity and redacted summary');
  const redacted = observe(derivation,'ring',128,false);
  check(redacted.kind === 'redacted' && redacted.events.every(e=>e.kind==='redacted' && e.value===0), 'payload capture explicitly opt in');
  const ring = observe(derivation,'ring',2), prefix = observe(derivation,'prefix',2);
  check(ring.events.length===2 && ring.rootState==='retained', 'ring keeps recent root');
  check(ring.events.some(e=>e.inputs.some(x=>x.state==='evicted')), 'evicted operand explicit');
  check(ring.events.some(e=>e.call.state==='evicted'), 'evicted call explicit');
  check(prefix.events.length===2 && prefix.rootState==='discarded', 'prefix discards recent root explicitly');
  check(complete.events.some(e=>e.control.state==='missing'), 'absent branch context explicit');
  const repeated = compile('repeated', 'pure multiply(a: Nat, b: Nat): Nat { a * b }\nmain(): Nat { mut i: Nat = 0\n mut total: Nat = 0\n while i < 3 { total = total + multiply(6, 7)\n i = i + 1 }\n total }\n');
  const repeats = observe(repeated,'ring',256);
  const products = repeats.events.filter(e=>e.fn===multiplyIdentity && e.op===5);
  check(repeats.value===126 && products.length===3, 'loop result and actual repeated products');
  check(new Set(products.map(e=>e.id)).size===3 && new Set(products.map(e=>e.call.id)).size===3 && new Set(products.map(e=>e.pc)).size===1, 'same PC distinct occurrence and calls');
  check(new Set(products.map(e=>e.control.id)).size===3, 'separate chronological branch context each iteration');
  const branches = repeats.events.filter(e=>e.op===18);
  check(branches.map(e=>e.value).join(',')==='1,1,1,0', 'observed branch outcomes including loop exit');
  check(observe(repeated,'ring',3).events.some(e=>e.control.state==='evicted'), 'evicted control edge explicit');
  const recursiveSource = n => `pure descend(n: Nat): Nat { if n > 0 { descend(n - 1) + 1 } else { 0 } }\nmain(): Nat { descend(${n}) }\n`;
  const recursion = observe(compile('recursion',recursiveSource(4)),'ring',256);
  check(recursion.value===4 && recursion.gap==='none', 'recursive result');
  check(new Set(recursion.events.filter(e=>e.fn.endsWith('_descend') && e.op===20).map(e=>e.call.id)).size===5, 'recursive activation identity');
  const locals = Array.from({length:130},(_,i)=>`x${i}: Nat = ${i}`).join('\n');
  const localCap = observe(compile('locals',`main(): Nat { ${locals}\n 42 }`));
  check(localCap.value===42 && localCap.gap==='metadata-cap', 'local metadata cap preserves execution');
  const capped = observe(compile('deep',recursiveSource(40)));
  check(capped.value===40 && capped.gap==='metadata-cap' && capped.rootState==='missing', 'frame cap preserves VM result with unavailable root');
  for (const [name,source,value] of [
    ['collection','main(): Nat { xs: Array[Nat] = [6, 7]\n xs[0] * xs[1] }',42],
    ['builtin','main(): Nat { text: Str = \"secret-input\"\n text.len() }',12],
    ['indirect','pure answer(): Nat { 42 }\nmain(): Nat { f: PureFn[Nat] = @answer\n f.call() }',42],
  ]) {
    const built = compile(name,source), result = observe(built);
    check(result.value===value && result.gap==='unsupported-opcode' && result.rootState==='missing', name+' boundary is unavailable without changing execution');
  }
  const host = observe(compile('host','main(): Void { print(42) }'),'ring',128,true,true,1);
  check(host.gap==='unsupported-opcode' && host.rootState==='missing', 'host service requires embedding callback; unsupported boundary explicit');
  const asyncResult = observe(compile('async','async child(): Nat { 42 }\nasync main(): Unit { n: Nat = await child()\n () }'));
  check(asyncResult.gap==='unsupported-opcode' && asyncResult.rootState==='missing', 'async boundary explicit');
  const huge = observe(compile('large','main(): Nat { 184467440737095516160000 }'));
  check(huge.kind==='unsupported-value' && huge.events.every(e=>e.kind==='unsupported-value'), 'oversize payload never truncated to invented scalar');
  const trapped = observe(compile('trap','main(): Nat { xs: Array[Nat] = [1]\n xs[2] }'),'ring',128,true,true,1);
  check(trapped.rootState==='missing', 'trap has no invented successful root');
  const printed = compile('public','pure multiply(a: Nat, b: Nat): Nat { a * b }\nmain(): Void { print(multiply(6, 7)) }');
  check(run('./panack',[printed.file])==='42\n' && run('./panack',['run',printed.artifact])==='42\n','public CLI source and bytecode output');
  // Verifier accepts a RETURN somewhere; execution must trap a different path
  // falling off the function, including after observation was disabled.
  for (const [name,tail] of [['scalar','00000000'],['unsupported','070000']]) {
    const artifact=path.join(work,`falloff-${name}.bc`);
    fs.writeFileSync(artifact,Buffer.from('50414e41434b4243000009000100046d61696e000000000003130000000214'+tail,'hex'));
    const fallen=observe({artifact},'ring',128,true,true,1);
    check(fallen.status===2 && fallen.rootState==='missing', name+' falloff delegates safely to VM trap');
  }
  fs.writeFileSync(path.join(work,'invalid.bc'),'not bytecode');
  run(probe,[path.join(work,'invalid.bc'),'ring','128'],2); checks++;
  run(probe,[derivation.artifact,'ring','0'],2); checks++;
  const benchmark = compile('loop',fs.readFileSync(path.join(__dirname,'loop.panack'),'utf8'));
  run(probe,[derivation.artifact,'ring','65537'],2); checks++;
  const samples = {};
  run(probe,['--self-test']); checks++;
  check(observe(benchmark,'ring',2,true,false).bytes === ring.bytes, 'retained memory independent of executed event count');
  check(complete.bytes-redacted.bytes===0, 'payload opt-in does not allocate variable values');
  for (const mode of ['bulk','step','prefix','ring']) samples[mode]=[];
  for(let round=0;round<5;round++) for(const mode of ['bulk','step','prefix','ring']) {
    const result=observe(benchmark,mode,1024,true,false);
    check(result.value===1260000 && result.gap==='none',mode+' benchmark parity');
    samples[mode].push(result.cpu);
    if(round===0) console.log(JSON.stringify({mode,bytes:result.bytes,rss:result.rss,events:result.count,root:result.rootState}));
  }
  console.log(JSON.stringify({cpuSeconds:samples}));
  console.log(`runtime provenance: ${checks} assertions passed`);
} finally { fs.rmSync(work,{recursive:true,force:true}); }
