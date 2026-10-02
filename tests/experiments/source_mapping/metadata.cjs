// U1 experimental metadata, not a production format or an authenticity scheme.
// Digests bind trusted compiler output to bytes/snapshots; they are not signatures.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const hash=data=>crypto.createHash('sha256').update(data).digest('hex');
const portable=p=>typeof p==='string' && p.length>0 && p.length<512 &&
  !p.includes('\\') && !/[\x00-\x1f:]/.test(p) && !path.posix.isAbsolute(p) &&
  p.split('/').every(x=>x && x!=='.' && x!=='..');
function read(root, name) {
  if (!portable(name)) throw new Error('Invalid source path');
  let current=root;
  for (const part of name.split('/')) {
    current=path.join(current,part);
    if (fs.lstatSync(current).isSymbolicLink()) throw new Error('Source symlink');
  }
  const data=fs.readFileSync(current);
  if (data.length>1024*1024) throw new Error('Source too large');
  return data;
}
function snapshots(root, prefix='') {
  return fs.readdirSync(path.join(root,prefix),{withFileTypes:true}).sort((a,b)=>a.name<b.name?-1:1).flatMap(item=>{
    const name=prefix+item.name;
    if(item.isSymbolicLink()) throw new Error('Source symlink');
    if(item.isDirectory()) return snapshots(root,name+'/');
    return name.endsWith('.panack') ? [{path:name,sha256:hash(read(root,name))}] : [];
  });
}
function seal(payload) {
  return JSON.stringify({...payload,integrity:hash(JSON.stringify(payload))})+'\n';
}
function create(rows, root, bytes, compiler) {
  const entries=rows.trim().split('\n').filter(Boolean).map(line=>{
    const fields=line.split('\t');
    if(fields.length!==6 || fields[0]!=='map') throw new Error('Malformed compiler map row');
    const file=path.relative(root,fields[3]).split(path.sep).join('/');
    if(!portable(file)) throw new Error('Source outside experiment root');
    return {function:fields[1],pc:Number(fields[2]),file,start:Number(fields[4]),end:Number(fields[5])};
  }).sort((a,b)=>a.function<b.function?-1:a.function>b.function?1:a.pc-b.pc);
  return seal({version:1,encoding:'unicode-code-points',compiler,bytecode:hash(bytes),
    sources:snapshots(root),entries});
}
function lookup(raw, bytes, compiler, root, instructions, trap) {
  try {
    if(typeof raw!=='string' || Buffer.byteLength(raw)>65536) return null;
    const map=JSON.parse(raw);
    const {integrity,...payload}=map;
    if(integrity!==hash(JSON.stringify(payload))) return null;
    if(map.version!==1 || map.encoding!=='unicode-code-points' || map.compiler!==compiler || map.bytecode!==hash(bytes) ||
      !Array.isArray(map.sources) || !map.sources.length || map.sources.length>256 ||
      !Array.isArray(map.entries) || map.entries.length>1024) return null;
    const sources=new Map();
    for(const source of map.sources) {
      if(sources.has(source.path)) return null;
      const data=read(root,source.path);
      if(hash(data)!==source.sha256) return null;
      sources.set(source.path,Array.from(data.toString('utf8')));
    }
    // The experiment binds the whole declared source corpus conservatively.
    if(JSON.stringify(snapshots(root))!==JSON.stringify(map.sources)) return null;
    const keys=new Set();
    for(const entry of map.entries) {
      const key=JSON.stringify([entry.function,entry.pc]);
      const source=sources.get(entry.file);
      if(typeof entry.function!=='string' || /[\x00-\x1f]/.test(entry.function) ||
        !Number.isSafeInteger(entry.pc) || entry.pc<0 || !source ||
        !Number.isSafeInteger(entry.start) || !Number.isSafeInteger(entry.end) ||
        entry.start<0 || entry.start>=entry.end || entry.end>source.length ||
        instructions.get(key)!=='INDEX_GET' || keys.has(key)) return null;
      keys.add(key);
    }
    const entry=map.entries.find(e=>e.function===trap.function && e.pc===trap.pc);
    if(!entry) return null;
    const source=sources.get(entry.file);
    const before=source.slice(0,entry.start).join('');
    return {...entry,line:before.split('\n').length,column:Array.from(before.split('\n').at(-1)).length+1,
      expression:source.slice(entry.start,entry.end).join('')};
  } catch { return null; }
}
module.exports={hash,seal,create,lookup};
