// Bounded experiment, not a general semantic dependency analyzer.
const fs = require('node:fs'), os = require('node:os'), path = require('node:path');
const {spawnSync} = require('node:child_process');
const assert = require('node:assert/strict');
const {createHash} = require('node:crypto');
const probe = path.resolve(process.argv[2]);
const work = fs.mkdtempSync(path.join(os.tmpdir(), 'panack-impact-'));
let assertions = 0;
function check(value, message) { assert(value, message); assertions++; }
function run(command, args, status = 0) {
  const start = performance.now();
  const result = spawnSync(command, args, {encoding: 'utf8', timeout: 90000, maxBuffer: 1048576});
  assert.ifError(result.error);
  assert.equal(result.status, status, result.stderr + result.stdout);
  return {text: result.stdout, ms: performance.now() - start};
}
function evidence(file) {
  const result = run('./panack-vm', ['run', probe, file]);
  const rows = result.text.trim().split('\n').map(line => line.split('\t'));
  return {...result, rows, sub: rows.filter(r => r[0] === 'sub'), effects: rows.filter(r => r[0] === 'effect')};
}
function one(rows, name) {
  const found = rows.filter(r => r[1] === name);
  assert.equal(found.length, 1, 'ambiguous/missing fixture evidence: ' + name);
  return found[0];
}
function replaceOnce(source, before, after) {
  assert.equal(source.split(before).length, 2, 'edit must identify exactly one occurrence');
  return source.replace(before, after);
}
function write(name, source) {
  const file = path.join(work, name + '.panack');
  fs.writeFileSync(file, source);
  return file;
}
// Only these two supported counterfactuals are analyzed. Missing or contradictory
// evidence is unknown; caller reachability is never a lost-proof prediction.
function weakenedGuard(record, oldGuard, newLower) {
  if (record[2] !== 'lower-bound' || record[3] !== 'true' || record[4] !== 'true' ||
      record[6] !== 'true' || record[8] !== oldGuard || record[9] !== 'true' ||
      record[11] !== 'Nat' || record[12] !== 'Nat' || record[13] !== 'false') return 'unknown';
  return newLower >= Number(record[7]) ? 'proved' : 'unproved';
}
function widenedCall(record) {
  if (record[2] !== 'call' || record[5] !== 'function declaration' ||
      record[6] !== 'pure' || record[7] !== 'false' || record[8] !== '0') return 'unknown';
  if (record[3] === 'pure') return 'rejected';
  if (record[3] === 'ordinary') return 'allowed';
  return 'unknown';
}
function preservedLocalProof(source, record, edit) {
  // Closed fixture argument: checkout's operand is its Nat parameter, and the
  // selected guard is local; neither operand comes from the changed callee.
  // Refuse any other body or edit instead of generalizing textual independence.
  const body = 'pure checkout(n: Nat): Nat {\n  balance: Nat = remaining(n)\n  if n >= 5 { n - 5 } else { balance }\n}';
  const offset = source.indexOf(edit);
  if (!source.includes(body) || source.split(edit).length !== 2 || offset < 0 ||
      offset >= source.indexOf(body) || !['n >= 2', 'pure safe_subtract'].includes(edit)) return 'unknown';
  return record[1] === 'checkout' && record[2] === 'lower-bound' && record[3] === 'true' &&
    record[4] === 'true' && record[5] === '5' && record[6] === 'true' && record[7] === '5' &&
    record[8] === 'n >= 5' && record[9] === 'true' && record[10] === 'n - 5' &&
    record[11] === 'Nat' && record[12] === 'Nat' && record[13] === 'false' ? 'proved' : 'unknown';
}
try {
  const source = fs.readFileSync(path.join(__dirname, 'baseline.panack'), 'utf8');
  const baseline = write('baseline', source);
  const initial = evidence(baseline);
  check(initial.rows[0].join('|') === 'project|0|true', 'baseline accepted with effect pass');
  run('./panack', ['check', baseline]);
  check(run('./panack', [baseline]).text === '3\n6\n', 'complete baseline program executes');
  const leaf = one(initial.sub, 'safe_subtract'), local = one(initial.sub, 'checkout');
  const direct = one(initial.effects, 'remaining'), transitive = one(initial.effects, 'checkout');
  const ordinary = one(initial.effects, 'audit');
  check(leaf[10] === 'n - 2' && local[10] === 'n - 5', 'proof obligations identified');
  check(local[2] === 'lower-bound' && local[3] === 'true' && local[5] === '5' &&
    local[7] === '5' && local[8] === 'n >= 5' && local[9] === 'true' &&
    local[11] === 'Nat' && local[12] === 'Nat' && local[13] === 'false', 'positive caller-local proof basis');
  check(direct[4] === 'safe_subtract' && transitive[4] === 'remaining' && ordinary[4] === 'safe_subtract', 'actual semantic boundary targets');
  const predictions = Object.freeze({
    guard: weakenedGuard(leaf, 'n >= 2', 1),
    directEffect: widenedCall(direct),
    ordinaryEffect: widenedCall(ordinary),
    conditionalTransitive: widenedCall(transitive),
    callerLocalSubtraction: preservedLocalProof(source, local, 'n >= 2'),
    transitiveReturnGuarantee: 'unknown'
  });
  check(predictions.guard === 'unproved' && predictions.directEffect === 'rejected' &&
    predictions.ordinaryEffect === 'allowed' && predictions.conditionalTransitive === 'rejected', 'pre-change predictions supported');
  check(predictions.callerLocalSubtraction === 'proved' && preservedLocalProof(source, local, 'pure safe_subtract') === 'proved', 'non-impact needs local rule support and narrowly isolated edit');
  check(preservedLocalProof(source, local, 'n >= 5') === 'unknown' &&
    preservedLocalProof(source.replace('n - 5', 'balance - 5'), local, 'n >= 2') === 'unknown', 'changed local dependency or unrecognized body fails closed');
  // This record is emitted before ANY changed snapshot is constructed or checked.
  console.log('PRE-CHANGE predictions: ' + JSON.stringify(predictions));
  check(fs.readdirSync(work).join(',') === 'baseline.panack', 'no changed snapshot exists at prediction time');
  const weakened = write('weakened', replaceOnce(source, 'n >= 2', 'n >= 1'));
  const weak = evidence(weakened);
  run('./panack', ['check', weakened], 1);
  check(one(weak.sub, 'safe_subtract')[3] === 'false', 'guard prediction matches real checking');
  check(one(weak.sub, 'safe_subtract')[5] === '1', 'weakened lower bound retained');
  check(weak.rows[0][2] === 'true' && !weak.effects.some(row => row[1] === 'safe_subtract') && one(weak.effects, 'remaining').join('|') === one(initial.effects, 'remaining').join('|'), 'invalid leaf has no effects; type-valid caller boundary is positively re-established from declared effect');
  check(one(weak.sub, 'checkout').join('|') === local.join('|'), 'caller-local obligation positively re-established with same guard, bound and operands');
  const widenedSource = replaceOnce(source, 'pure safe_subtract', 'safe_subtract');
  const widened = write('widened', widenedSource), wide = evidence(widened);
  run('./panack', ['check', widened], 1);
  check(one(wide.effects, 'remaining')[8] === '1' && one(wide.effects, 'remaining')[6] === 'ordinary', 'direct boundary rejects predicted effect');
  check(one(wide.effects, 'audit')[8] === '0' && one(wide.effects, 'audit')[3] === 'ordinary' &&
    one(wide.effects, 'audit')[6] === 'ordinary', 'ordinary caller positively permits changed callee');
  check(one(wide.effects, 'checkout').join('|') === transitive.join('|'), 'no inferred transitive effect: unchanged pure declaration still governs boundary');
  check(one(wide.sub, 'checkout').join('|') === local.join('|'), 'effect edit preserves caller-local Nat proof');
  const cascade = write('conditional-cascade', replaceOnce(widenedSource, 'pure remaining', 'remaining'));
  const chained = evidence(cascade);
  run('./panack', ['check', cascade], 1);
  check(one(chained.effects, 'remaining')[8] === '0' && one(chained.effects, 'checkout')[8] === '1', 'conditional second edit moves rejected boundary one call outward');
  // Counterexamples: the single retained origin is not a complete dependency set.
  const redundantSource = replaceOnce(source, 'if n >= 2 { n - 2 }', 'if n >= 3 { if n >= 2 { n - 2 } else { 0 } }');
  const redundant = evidence(write('redundant', redundantSource));
  const redundantLeaf = one(redundant.sub, 'safe_subtract');
  check(redundantLeaf[5] === '2' && redundantLeaf[8] === 'n >= 2', 'inner guard replaces stronger outer bound');
  check(weakenedGuard(redundantLeaf, 'n >= 3', 1) === 'unknown', 'unmatched edited guard cannot claim lost proof');
  const redundantChanged = write('redundant-changed', replaceOnce(redundantSource, 'n >= 3', 'n >= 1'));
  run('./panack', ['check', redundantChanged]);
  check(one(evidence(redundantChanged).sub, 'safe_subtract')[3] === 'true', 'unchanged inner guard positively re-establishes proof');
  const replacedGuard = write('replaced-guard', replaceOnce(redundantSource, 'n >= 2', 'n >= 1'));
  run('./panack', ['check', replacedGuard], 1);
  check(one(evidence(replacedGuard).sub, 'safe_subtract')[3] === 'false', 'mathematically sufficient outer guard is not recovered after inner replacement');
  const mutationSource = replaceOnce(source, 'pure safe_subtract(n: Nat): Nat {\n  if n >= 2 { n - 2 }',
    'pure safe_subtract(input: Nat): Nat {\n  mut n: Nat = input\n  if n >= 2 { n = 0; n - 2 }');
  const mutationFile = write('mutation', mutationSource), mutation = evidence(mutationFile);
  run('./panack', ['check', mutationFile], 1);
  check(one(mutation.sub, 'safe_subtract')[3] === 'false' && one(mutation.sub, 'safe_subtract')[4] === 'false', 'mutation discards stale bound');
  check(weakenedGuard(one(mutation.sub, 'safe_subtract'), 'n >= 2', 1) === 'unknown', 'missing valid baseline evidence fails closed');
  check(fs.readFileSync(baseline, 'utf8') === source, 'captured baseline unchanged throughout experiment');
  const checkSamples = [], evidenceSamples = [];
  for (let i = 0; i < 5; i++) {
    checkSamples.push(run('./panack', ['check', baseline]).ms);
    evidenceSamples.push(evidence(baseline).ms);
  }
  console.log('MEASUREMENTS ' + JSON.stringify({sourceSha256: createHash('sha256').update(source).digest('hex'), sourceBytes: Buffer.byteLength(source),
    subtractionRecords: initial.sub.length, effectRecords: initial.effects.length,
    transportBytes: Buffer.byteLength(initial.text), predictionBytes: Buffer.byteLength(JSON.stringify(predictions)),
    checkMs: checkSamples.map(n => +n.toFixed(3)), evidenceMs: evidenceSamples.map(n => +n.toFixed(3))}));
  console.log(`semantic-impact experiment: ${assertions} assertions passed`);
} finally { fs.rmSync(work, {recursive: true, force: true}); }
