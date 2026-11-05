'use strict';
// Executable preparation is local to this checkout and user. Never cache sessions.
const fs = require('node:fs'), path = require('node:path'), crypto = require('node:crypto');
const {regular,sha,readPlan} = require('./source_coverage.cjs');
const MiB = 1024 * 1024;
const SCHEMA = 1;
function privateDirectory(directory) {
  const s = fs.lstatSync(directory);
  if (!s.isDirectory() || s.isSymbolicLink() || typeof process.getuid !== 'function' ||
      s.uid !== process.getuid() || (s.mode & 0o077)) throw Error('cache directory must be private and owned by this user');
}
function privateFile(file, limit) {
  const s = fs.lstatSync(file);
  if (!s.isFile() || s.uid !== process.getuid() || (s.mode & 0o077) || s.nlink !== 1) throw Error('cache file is not private regular data');
  return regular(file,limit);
}
// Include names as well as bytes: adding a resolution candidate must invalidate.
// Sources outside these roots are rejected for reuse after trusted generation.
function inputSnapshot(checkout) {
  const inputs = [], roots = ['src','tests'];
  let count = 0, bytes = 0;
  function walk(relative) {
    if (++count > 100000) throw Error('coverage input discovery exceeds 100000 entries');
    const file = path.join(checkout,relative), s = fs.lstatSync(file);
    if (s.isSymbolicLink()) { inputs.push([relative,'link',fs.readlinkSync(file)]); return; }
    if (s.isDirectory()) {
      inputs.push([relative,'directory']);
      for (const name of fs.readdirSync(file).sort()) walk(relative+'/'+name);
    } else if (relative.endsWith('.panack') || ['panack.json','panack.lock.json'].includes(path.basename(relative))) {
      const data = regular(file); bytes += data.length;
      if (bytes > 128*MiB) throw Error('coverage input snapshot exceeds 128 MiB');
      inputs.push([relative,sha(data)]);
    }
  }
  for (const root of roots) walk(root);
  for (const name of ['bin/panack','VERSION','bootstrap/compiler-v9.bc','panack-vm','panack.json','panack.lock.json']) {
    const file = path.join(checkout,name);
    inputs.push([name,fs.existsSync(file) ? sha(regular(file)) : null]);
  }
  // Cache implementation changes invalidate its own prior trust/format decisions.
  inputs.push(['cache-producer',sha(regular(__filename))]);
  return sha(Buffer.from(JSON.stringify(inputs)));
}
function limit(name) {
  if (name === 'plan.plan') return 128*MiB;
  if (name === 'plan.inv') return 64*MiB;
  return 16*MiB;
}
function members(plan) {
  return ['plan.bc','plan.inv','plan.plan',...plan.sources.map((_,i)=>`plan.source-${i}.plan`)];
}
function createPlanCache({checkout,directory,trustDirectory,compiler,vm,log=console.log}) {
  checkout = fs.realpathSync(checkout);
  const snapshot = inputSnapshot(checkout);
  const context = {schema:SCHEMA,checkout,stdlib:path.join(checkout,'src/stdlib'),
    compiler:sha(compiler),vm:sha(vm),snapshot,format:'PANACKPLAN1/local-replay-v9',
    settings:{bootstrapRoot:checkout,sourceExecution:'native',platform:process.platform,arch:process.arch}};
  // Canonicalise the parent to accommodate macOS /var -> /private/var aliases.
  fs.mkdirSync(path.dirname(directory),{recursive:true});
  directory = path.join(fs.realpathSync(path.dirname(directory)),path.basename(directory));
  try { fs.mkdirSync(directory,{mode:0o700}); } catch (e) { if (e.code !== 'EEXIST') throw e; }
  privateDirectory(directory);
  // The trust anchor is outside the payload cache. Restoring/downloading payloads
  // cannot supply a key that authenticates them. Never publish this directory.
  trustDirectory ||= path.join(checkout,'build/source-plan-trust');
  fs.mkdirSync(path.dirname(trustDirectory),{recursive:true});
  trustDirectory = path.join(fs.realpathSync(path.dirname(trustDirectory)),path.basename(trustDirectory));
  if (trustDirectory === directory || trustDirectory.startsWith(directory+path.sep)) throw Error('cache trust anchor must be outside cached payloads');
  try { fs.mkdirSync(trustDirectory,{mode:0o700}); } catch (e) { if (e.code !== 'EEXIST') throw e; }
  privateDirectory(trustDirectory);
  const keyFile = path.join(trustDirectory,'local.key');
  try { fs.writeFileSync(keyFile,crypto.randomBytes(32),{flag:'wx',mode:0o600}); }
  catch (e) { if (e.code !== 'EEXIST') throw e; }
  const secret = privateFile(keyFile,32);
  if (secret.length !== 32) throw Error('invalid local cache authentication key');
  const mac = bytes=>crypto.createHmac('sha256',secret).update(bytes).digest('hex');
  function assertCurrent() {
    privateDirectory(directory);privateDirectory(trustDirectory);
    if (!privateFile(keyFile,32).equals(secret)) throw Error('local cache trust anchor changed during collection');
    if (inputSnapshot(checkout) !== snapshot) throw Error('Coverage preparation inputs changed during collection');
  }
  function verify(slot,entry,key) {
    privateDirectory(slot);
    const receipt = JSON.parse(privateFile(path.join(slot,'receipt.json'),MiB));
    if (typeof receipt.body !== 'string' || typeof receipt.mac !== 'string' ||
        !/^[a-f0-9]{64}$/.test(receipt.mac) || !crypto.timingSafeEqual(Buffer.from(receipt.mac,'hex'),Buffer.from(mac(receipt.body),'hex'))) throw Error('untrusted or altered cache receipt');
    const record = JSON.parse(receipt.body);
    if (record.key !== key || record.entry !== entry || JSON.stringify(record.context) !== JSON.stringify(context)) throw Error('cached preparation inputs do not match');
    if (!Array.isArray(record.files) || record.files.length < 4 || record.files.length > 259) throw Error('invalid cached member count');
    const expected = ['plan.bc','plan.inv','plan.plan',...Array.from({length:record.files.length-3},(_,i)=>`plan.source-${i}.plan`)];
    if (record.files.some((f,i)=>f.name !== expected[i]) || JSON.stringify(fs.readdirSync(slot).sort()) !== JSON.stringify([...expected,'receipt.json'].sort())) throw Error('unexpected or missing cached member');
    for (const f of record.files) {
      const data = privateFile(path.join(slot,f.name),limit(f.name));
      if (data.length !== f.size || sha(data) !== f.sha256) throw Error('cached preparation content changed');
    }
    return record;
  }
  function copy(slot,record,prefix) {
    for (const f of record.files) {
      // Check the bytes actually copied, not a prior read of a mutable cache.
      const data = privateFile(path.join(slot,f.name),limit(f.name));
      if (data.length !== f.size || sha(data) !== f.sha256) throw Error('cached preparation changed while copying');
      fs.writeFileSync(prefix+f.name.slice(4),data,{flag:'wx',mode:0o600});
    }
  }
  function discardCopy(prefix) {
    for (const name of fs.readdirSync(path.dirname(prefix))) {
      const suffix = name.slice(path.basename(prefix).length);
      if (name.startsWith(path.basename(prefix)) && /^(\.bc|\.inv|\.plan|\.source-\d+\.plan)$/.test(suffix)) fs.unlinkSync(path.join(path.dirname(prefix),name));
    }
  }
  function get(entry,prefix,generate) {
    assertCurrent();
    const key = sha(Buffer.from(JSON.stringify({context,entry}))), slot = path.join(directory,key);
    try {
      const record = verify(slot,entry,key);
      copy(slot,record,prefix);
      const plan = readPlan(prefix,entry,checkout,compiler);
      assertCurrent();
      log(`Plan cache hit: ${entry} (authenticated preparation; fresh execution required)`);
      return plan;
    } catch (e) {
      discardCopy(prefix);
      log(`Plan cache rebuild: ${entry} (${e.code === 'ENOENT' ? 'no complete preparation for current inputs' : e.message})`);
    }
    const staging = fs.mkdtempSync(path.join(directory,'.staging-'));
    try {
      const stagedPrefix = path.join(staging,'plan');
      generate(stagedPrefix);
      const plan = readPlan(stagedPrefix,entry,checkout,compiler);
      assertCurrent();
      if (plan.sources.some(s=>!s.path.startsWith('src/') && !s.path.startsWith('tests/'))) throw Error('coverage cache source lies outside the captured input roots');
      const names = members(plan);
      if (JSON.stringify(fs.readdirSync(staging).sort()) !== JSON.stringify([...names].sort())) throw Error('unexpected generated preparation member');
      const files = names.map(name=>{const file=path.join(staging,name),data=regular(file,limit(name));fs.chmodSync(file,0o600);return {name,size:data.length,sha256:sha(data)};});
      const body = JSON.stringify({key,entry,context,files});
      fs.writeFileSync(path.join(staging,'receipt.json'),JSON.stringify({body,mac:mac(body)}),{flag:'wx',mode:0o600});
      copy(staging,verify(staging,entry,key),prefix);
      const result = readPlan(prefix,entry,checkout,compiler);
      assertCurrent();
      // Replacing an invalid entry is safe: readers use private run copies and
      // a publication failure cannot invalidate freshly generated preparation.
      try {
        try { fs.renameSync(staging,slot); }
        catch (e) {
          if (!['EEXIST','ENOTEMPTY','ENOTDIR'].includes(e.code)) throw e;
          try { verify(slot,entry,key); }
          catch { fs.rmSync(slot,{recursive:true,force:true});fs.renameSync(staging,slot); }
        }
      } catch (e) {
        log(`Plan cache not saved: ${entry} (${e.message}); using validated fresh preparation`);
      }
      return result;
    } finally { fs.rmSync(staging,{recursive:true,force:true}); }
  }
  return {get,assertCurrent,context};
}
module.exports = {createPlanCache,inputSnapshot};
