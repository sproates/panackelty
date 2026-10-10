'use strict';
// Fast, bounded reader for locally reproduced SC2/SC4 identities and source plans.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const MAX = 16 * 1024 * 1024;
const U64 = (1n << 64n) - 1n;
const sha = b => crypto.createHash('sha256').update(b).digest('hex');
const fail = message => { throw new Error(message); };
class Reader {
  constructor(data) { this.data = data; this.offset = 0; }
  bytes(n) {
    if (!Number.isSafeInteger(n) || n < 0 || this.offset + n > this.data.length) fail('Truncated coverage data');
    const b = this.data.subarray(this.offset, this.offset += n); return b;
  }
  magic(s) { if (!this.bytes(Buffer.byteLength(s)).equals(Buffer.from(s))) fail('Coverage magic mismatch'); }
  u8() { return this.bytes(1)[0]; }
  u32() { return this.bytes(4).readUInt32BE(); }
  u64() { return this.bytes(8).readBigUInt64BE(); }
  blob() { const n = this.u32(); if (n > MAX) fail('Oversized coverage blob'); return this.bytes(n); }
  text() { const b = this.blob(), s = b.toString('utf8'); if (!Buffer.from(s).equals(b)) fail('Invalid UTF-8'); return s; }
  count(max) { const n = this.u32(); if (n > max) fail('Coverage count limit'); return n; }
  end() { if (this.offset !== this.data.length) fail('Trailing coverage data'); }
}
function regular(file, limit = MAX) {
  const s = fs.lstatSync(file);
  if (!s.isFile() || s.size > limit) fail('Unsafe or oversized coverage file');
  return fs.readFileSync(file);
}
function sourcePath(name, entry, checkout) {
  // Canonicalize the checkout first. macOS commonly exposes temporary paths
  // through both /var and /private/var; comparing a child realpath against
  // the uncanonicalized spelling incorrectly rejects every fixture source.
  const checkoutRoot = fs.realpathSync(checkout);
  const root = name.startsWith('stdlib/') ? path.join(checkoutRoot, 'src/stdlib') : path.dirname(path.resolve(checkoutRoot, entry));
  if (!name.startsWith('stdlib/') && !name.startsWith('project/')) fail('Unknown source prefix');
  const file = path.resolve(root, name.slice(name.indexOf('/') + 1));
  const relative = path.relative(checkoutRoot, file).split(path.sep).join('/');
  if (relative.startsWith('../') || path.isAbsolute(relative) || !relative.endsWith('.panack') || fs.realpathSync(file) !== file) fail('Escaping or linked source');
  return {file, relative};
}
function readPlan(prefix, entry, checkout, compiler) {
  let r = new Reader(regular(prefix + '.plan'));
  r.magic('PANACKPLAN1\n');
  const compilerBytes = r.blob(), artifact = r.blob(), inventory = r.blob();
  if (!compilerBytes.equals(compiler) || !artifact.equals(regular(prefix + '.bc')) || !inventory.equals(regular(prefix + '.inv'))) fail('Plan identity mismatch');
  const envelope = new Reader(inventory); envelope.magic('PANACKINVENTORY1\n');
  if (envelope.u32() !== 1) fail('Expected single-root plan inventory');
  const inv = new Reader(envelope.blob()); envelope.end();
  inv.magic('PANACKINV1\nlocal-replay-v9\n');
  if (!inv.blob().equals(compiler)) fail('Inventory compiler mismatch');
  const inventoryEntry = inv.text();
  if (sourcePath(inventoryEntry,entry,checkout).file !== path.resolve(fs.realpathSync(checkout),entry) || !inv.blob().equals(artifact)) fail('Inventory artifact or entry mismatch');
  const inventorySources = new Map();
  for (let n = inv.count(256); n--; ) {
    const name = inv.text(), text = inv.text(), items = [];
    if (inventorySources.has(name)) fail('Duplicate inventory source');
    for (let count = inv.count(100000); count--; ) items.push({id:inv.text(),kind:inv.text(),detail:inv.text(),span:Array.from({length:6},()=>inv.u32())});
    inventorySources.set(name,{text,items});
  }
  inv.end();
  const functions = [];
  for (let n = r.count(65536); n--; ) {
    const name = r.text(), edges = [];
    for (let count = r.count(262144); count--; ) {
      const conditional = r.u8(), target = r.u32(), falls = r.u8(), returns = r.u8();
      if ([conditional, falls, returns].some(v => v > 1)) fail('Invalid plan edge');
      edges.push({conditional, target, falls, returns});
    }
    if (!edges.length || edges.some(e => e.target > edges.length)) fail('Invalid plan target');
    if (functions.length && functions.at(-1).name >= name) fail('Noncanonical plan functions');
    functions.push({name, edges});
  }
  const sources = [], seen = new Set();
  const sourceCount = r.count(256); r.end();
  if (sourceCount !== inventorySources.size) fail('Plan source count mismatch');
  for (let n = 0; n < sourceCount; n++) {
    r = new Reader(regular(prefix + `.source-${n}.plan`));
    const name = r.text(), text = r.text(), resolved = sourcePath(name, entry, checkout);
    const original = inventorySources.get(name);
    if (!original || original.text !== text) fail('Plan source identity mismatch');
    if (seen.has(resolved.relative) || !regular(resolved.file).equals(Buffer.from(text))) fail('Duplicate or stale source');
    seen.add(resolved.relative);
    const items = [], ids = new Set();
    for (let count = r.count(100000); count--; ) {
      const id = r.text(), kind = r.text(), detail = r.text();
      const span = Array.from({length: 6}, () => r.u32());
      const functionIndex = r.u32(), start = r.u32(), end = r.u32(), mode = r.u8();
      if (ids.has(id) || !['function','expression','statement','decision','outcome','excluded'].includes(kind) || mode > 5 || functionIndex > functions.length || span[1] < 1 || span[4] < span[1]) fail('Invalid plan item');
      if ((kind === 'excluded' && functionIndex) || (kind === 'function' && mode) || (!functionIndex && mode) || (functionIndex && mode !== 5 && ['expression','statement'].includes(kind) && mode !== 1) || (functionIndex && mode !== 5 && kind === 'outcome' && ![2,3].includes(mode))) fail('Invalid plan mode');
      ids.add(id);
      if (functionIndex && kind !== 'function' && mode !== 5 && (start >= functions[functionIndex - 1].edges.length || end > functions[functionIndex - 1].edges.length || start > end)) fail('Invalid plan probe');
      items.push({id, kind, detail, span, functionIndex, start, end, mode});
    }
    if (JSON.stringify(items.map(({id,kind,detail,span})=>({id,kind,detail,span}))) !== JSON.stringify(original.items)) fail('Plan differs from exact inventory items');
    sources.push({path: resolved.relative, text, hash: sha(Buffer.from(text)), items});
    r.end();
  }
  r.end();
  return {entry, prefix, artifact, inventory, functions, sources};
}
function readRaw(data, plan) {
  const r = new Reader(data); r.magic('PANACKCOV1\n');
  if (!r.blob().equals(plan.artifact) || !r.blob().equals(plan.inventory) || r.u32() !== plan.functions.length) fail('Raw identity mismatch');
  const result = [];
  let cells = 0;
  for (const f of plan.functions) {
    const entries = r.u64(), count = r.u32(); cells += count;
    if (count !== f.edges.length || cells > 262144) fail('Raw instruction mismatch');
    const counters = [];
    for (let i = 0; i < count; i++) {
      const values = [r.u64(), r.u64(), r.u64(), r.u64()];
      if (values[1] > values[0] || (f.edges[i].conditional ? values[2] + values[3] !== values[1] : values[2] !== 0n || values[3] !== 0n)) fail('Invalid raw counters');
      counters.push(values);
    }
    result.push({entries, counters});
  }
  r.magic('END\n'); const terminal = r.u8(), flags = r.u8(); r.end();
  if (![1,2,3].includes(terminal) || flags) fail('Partial or gapped collection');
  return result;
}
function zero(plan) { return plan.functions.map(f => ({entries: 0n, counters: f.edges.map(() => [0n,0n,0n,0n])})); }
function add(a,b) {
  for (let i = 0; i < a.length; i++) {
    a[i].entries += b[i].entries;
    if (a[i].entries > U64) fail('Coverage overflow');
    for (let pc = 0; pc < a[i].counters.length; pc++) for (let k = 0; k < 4; k++) {
      a[i].counters[pc][k] += b[i].counters[pc][k];
      if (a[i].counters[pc][k] > U64) fail('Coverage overflow');
    }
  }
}
function readSession(directory, plans, seen) {
  if (new Set(plans.map(p => sha(p.artifact))).size !== plans.length) fail('Duplicate registry artifact');
  if (fs.lstatSync(directory).isSymbolicLink()) fail('Linked session');
  const names = fs.readdirSync(directory), expected = new Set(['registry','budget','closed']);
  if (names.length > 12291) fail('Session member limit');
  const reg = new Reader(regular(path.join(directory,'registry'))); reg.magic('PANACKSESSION1\n');
  const nonce = reg.bytes(16).toString('hex');
  if (seen.has(nonce)) fail('Duplicate session'); seen.add(nonce);
  if (reg.count(16) !== plans.length) fail('Registry count mismatch');
  for (const p of plans) if (!reg.blob().equals(p.artifact) || !reg.blob().equals(p.inventory)) fail('Registry identity mismatch');
  reg.end();
  if (!regular(path.join(directory,'closed')).equals(Buffer.from('closed\n'))) fail('Unclosed session');
  const ids = names.filter(n => n.startsWith('expect-')).map(n => n.slice(7));
  if (!ids.length || ids.length > 4096 || !ids.includes('0')) fail('Missing root admission');
  let bytes = 0, children = 0;
  const totals = plans.map(zero);
  for (const id of ids) {
    if (id.length >= 192 || !/^0(?:\.[1-9][0-9]*)*$/.test(id) || (id !== '0' && !ids.includes(id.slice(0,id.lastIndexOf('.'))))) fail('Invalid execution ID');
    for (const prefix of ['expect-','claim-','raw-']) expected.add(prefix + id);
    const kind = regular(path.join(directory,'expect-'+id),16).toString();
    if (id === '0' ? kind !== 'root\n' : !['nested\n','task\n','server\n','process\n'].includes(kind)) fail('Invalid admission kind');
    if (!regular(path.join(directory,'claim-'+id),16).equals(Buffer.from('claimed\n'))) fail('Missing claim');
    const data = regular(path.join(directory,'raw-'+id)); bytes += data.length;
    const r = new Reader(data); r.magic('PANACKEXEC1\n');
    if (r.bytes(16).toString('hex') !== nonce || r.text() !== id) fail('Cached or renamed execution');
    const index = r.u32(), count = r.count(4096); children += count;
    if (index >= plans.length) fail('Unknown artifact');
    for (let i = 1; i <= count; i++) if (!ids.includes(`${id}.${i}`)) fail('Lost child');
    add(totals[index], readRaw(r.bytes(data.length-r.offset), plans[index]));
  }
  if (bytes > 256*1024*1024 || children + 1 !== ids.length || names.length !== expected.size || names.some(n => !expected.has(n))) fail('Incomplete execution tree');
  const budget = new Reader(regular(path.join(directory,'budget'),8));
  if (budget.u32() !== ids.length || budget.u32() !== bytes) fail('Budget mismatch'); budget.end();
  return {totals, executions: ids.length, nonce};
}
function observations(plan, data) {
  return plan.sources.map(source => ({...source, items: source.items.map(item => {
    let state = item.kind === 'excluded' ? 'excluded' : 'not-emitted', attempted = 0n, completed = 0n;
    if (item.functionIndex) {
      const f = plan.functions[item.functionIndex - 1], counts = data[item.functionIndex - 1];
      state = item.mode === 5 ? 'unavailable' : 'observed';
      if (item.kind === 'function') {
        attempted = counts.entries;
        completed = f.edges.reduce((n,e,i) => n + (e.returns ? counts.counters[i][1] : 0n), 0n);
      } else if (item.mode === 1) {
        attempted = item.start === 0 ? counts.entries : 0n;
        for (let pc = 0; pc < f.edges.length; pc++) {
          const edge = f.edges[pc], c = counts.counters[pc];
          const incoming = edge.conditional ? [[pc+1,c[2]],[edge.target,c[3]]] : [[edge.target,c[1]]];
          for (const [target,count] of incoming) {
            if (target === item.start && (pc < item.start || pc >= item.end)) attempted += count;
            if (target === item.end && pc >= item.start && pc < item.end) completed += count;
          }
        }
      } else if (item.mode !== 5) {
        const c = counts.counters[item.start];
        attempted = item.mode === 2 ? c[2] : item.mode === 3 ? c[3] : c[0];
        completed = item.mode === 2 || item.mode === 3 ? attempted : c[1];
      }
    }
    return {...item, state, attempted, completed};
  })}));
}
function union(groups, eligible) {
  const files = new Map();
  for (const sources of groups) for (const source of sources) {
    if (!eligible.includes(source.path)) continue;
    let file = files.get(source.path);
    if (!file) { file = {...source, items: new Map()}; files.set(source.path,file); }
    if (file.hash !== source.hash) fail('Mixed source snapshots');
    if (file.items.size && (file.items.size !== source.items.length || source.items.some(i => !file.items.has(i.id)))) fail('Mixed source inventory');
    for (const item of source.items) {
      const identity = JSON.stringify([item.kind,item.detail,item.span]);
      let merged = file.items.get(item.id);
      if (!merged) { merged = {...item, identity, attempted: 0n, completed: 0n, observed: false, unavailable: false}; file.items.set(item.id,merged); }
      if (merged.identity !== identity) fail('Mixed source item');
      merged.attempted += item.attempted; merged.completed += item.completed;
      merged.observed ||= item.state === 'observed'; merged.unavailable ||= item.state === 'unavailable';
    }
  }
  if (eligible.some(p => !files.has(p))) fail('Missing eligible source file');
  return [...files.values()].sort((a,b) => a.path.localeCompare(b.path)).map(file => {
    const items = [...file.items.values()].map(item => ({...item, state: item.kind === 'excluded' ? 'excluded' : item.attempted > 0n ? 'covered' : item.observed && !item.unavailable ? 'zero' : 'unavailable'}));
    const lines = new Map();
    for (const item of items.filter(i => ['expression','statement'].includes(i.kind))) {
      const line = item.span[1], prior = lines.get(line) || 'zero';
      lines.set(line, item.state === 'covered' || prior === 'covered' ? 'covered' : item.state === 'unavailable' || prior === 'unavailable' ? 'unavailable' : 'zero');
    }
    const metric = rows => { const total = rows.length, covered = rows.filter(i => i === 'covered').length, unavailable = rows.filter(i => i === 'unavailable').length;
      return {total, covered, unavailable, zero: total-covered-unavailable, percent: total && !unavailable ? 100*covered/total : null, lower: total ? 100*covered/total : null, upper: total ? 100*(covered+unavailable)/total : null}; };
    return {path:file.path, hash:file.hash, text:file.text, lines:Object.fromEntries([...lines].sort((a,b)=>a[0]-b[0])), functions:items.filter(i=>i.kind==='function').map(i=>({id:i.id,name:i.detail,line:i.span[1],state:i.state,entries:String(i.attempted)})), branches:items.filter(i=>i.kind==='outcome').map(i=>({id:i.id,outcome:i.detail,line:i.span[1],state:i.state,hits:String(i.attempted)})), exclusions:items.filter(i=>i.kind==='excluded').map(i=>({id:i.id,reason:i.detail,line:i.span[1]})), metrics:{lines:metric([...lines.values()]),functions:metric(items.filter(i=>i.kind==='function').map(i=>i.state)),branches:metric(items.filter(i=>i.kind==='outcome').map(i=>i.state))}};
  });
}
const escape = s => String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
function totals(files) {
  return Object.fromEntries(['lines','functions','branches'].map(kind => {
    const sum = {total:0,covered:0,unavailable:0,zero:0};
    for (const f of files) for (const key of Object.keys(sum)) sum[key] += f.metrics[kind][key];
    return [kind,{...sum,percent:sum.total && !sum.unavailable ? 100*sum.covered/sum.total : null,
      lower:sum.total ? 100*sum.covered/sum.total : null,upper:sum.total ? 100*(sum.covered+sum.unavailable)/sum.total : null}];
  }));
}
function metricText(m) { return `${m.covered}/${m.total} (${!m.total ? 'no executable items' : m.percent === null ? 'incomplete; '+m.unavailable+' unavailable' : m.percent.toFixed(2)+'%'})`; }
function writeReport(output, report) {
  if (!/^[a-f0-9]{40}$/.test(report.commit) || !Number.isFinite(Date.parse(report.generatedAt))) fail('Invalid report identity');
  if (fs.existsSync(output)) fail('Report destination exists'); fs.mkdirSync(path.join(output,'html'),{recursive:true});
  const head = title => `<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>${escape(title)}</title><style>body{font:16px/1.5 system-ui;margin:2rem;color:#17222e}table{border-collapse:collapse}td,th{padding:.4rem;border:1px solid #ccd}pre{white-space:pre-wrap}.covered{background:#dcf5df}.zero{background:#ffdddd}.unavailable{background:#fff0bf}code{overflow-wrap:anywhere}</style></head><body>`;
  const rows = report.files.map((f,i)=>`<tr><td><a href="file-${i}.html">${escape(f.path)}</a></td>${['lines','functions','branches'].map(k=>`<td>${metricText(f.metrics[k])}</td>`).join('')}</tr>`).join('');
  const components = Object.entries(report.components || {}).map(([name,m]) => `<tr><td>${escape(name)}</td>${['lines','functions','branches'].map(k=>`<td>${metricText(m[k])}</td>`).join('')}</tr>`).join('');
  fs.writeFileSync(path.join(output,'html/index.html'),head('Panackelty source coverage — next')+`<h1>.panack source coverage — next</h1><p>This is the explicitly scoped native source-test baseline, not the entire test suite, native C or browser/WASI coverage. Unavailable measurements never become zero hits or a complete percentage.</p><p>Commit <a href="https://github.com/sproates/panackelty/commit/${report.commit}">${report.commit}</a>; generated ${escape(report.generatedAt)}.</p><p><a href="../summary.json">Metrics, identities, exact test manifest and omitted paths</a></p><table><tr><th>Component</th><th>Executable start lines</th><th>Functions</th><th>Source branch outcomes</th></tr>${components}</table><h2>Every eligible source file</h2><table><tr><th>Source</th><th>Executable start lines</th><th>Functions</th><th>Source branch outcomes</th></tr>${rows}</table></body></html>`);
  report.files.forEach((f,i)=>{
    const lines = f.text.split('\n').map((s,n)=>`<span id="L${n+1}" class="${f.lines[n+1] || ''}">${String(n+1).padStart(5)} ${escape(s)}</span>`).join('\n');
    fs.writeFileSync(path.join(output,`html/file-${i}.html`),head(f.path)+`<p><a href="index.html">All files</a></p><h1>${escape(f.path)}</h1><p>Green: covered; red: known zero; amber: unavailable. Blank lines are outside the executable-start-line denominator.</p><pre>${lines}</pre><h2>Functions</h2><pre>${escape(JSON.stringify(f.functions,null,2))}</pre><h2>Source branch outcomes</h2><pre>${escape(JSON.stringify(f.branches,null,2))}</pre><h2>Reviewed declaration exclusions</h2><pre>${escape(JSON.stringify(f.exclusions,null,2))}</pre></body></html>`);
  });
  const summary = {...report, files:report.files.map(({text,...rest})=>rest)};
  fs.writeFileSync(path.join(output,'summary.json'),JSON.stringify(summary,null,2)+'\n');
  fs.writeFileSync(path.join(output,'summary.txt'),`Panackelty .panack source coverage (next)\nCommit: ${report.commit}\nGenerated: ${report.generatedAt}\n`+report.files.map(f=>`${f.path}\t${['lines','functions','branches'].map(k=>metricText(f.metrics[k])).join('\t')}`).join('\n')+'\n');
}
module.exports = {Reader,regular,sha,readPlan,readRaw,readSession,zero,add,observations,union,totals,writeReport};
