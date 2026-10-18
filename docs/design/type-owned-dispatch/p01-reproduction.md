# P01 reproduction and evidence checks

Baseline: `5327c2b6789e47801a03e663b007fd2a6f9da0c9`. These are temporary audit tools, preserved as documentation rather than adding an interpreter dependency or inventory command to the product. They read compiler data and emit records; they do not alter language behaviour. Run from the repository root on the pinned source tree. Preserve unrelated work and do not overwrite an existing probe.

## Procedure

1. Copy the Panack probe below to a new temporary `tests/runner/dispatch_inventory_probe.panack`. It is deliberately not registered as a product test.
2. Compile it with the audited seed and repository bootstrap root:

```sh
PANACKELTY_BOOTSTRAP_ROOT="$PWD" PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run bootstrap/compiler-v9.bc compile tests/runner/dispatch_inventory_probe.panack -o /tmp/dispatch-inventory-probe.bc
cp /tmp/dispatch-inventory-probe.bc /tmp/dispatch-inventory-probe-v2.bc
```

3. Save the orchestration blocks below to their named temporary files and run `run-dispatch-inventory.py`. It obtains the tracked 615-file corpus from Git, parses each source, then binds sources and manifests. Set PANACKELTY_VERSION from VERSION as shown. No network server is started.
4. Run `dispatch-package-pass.py`. It preserves full package/module identities for duplicate module names; the published audit used this to replace an earlier shortened package report.
5. Run `analyse-dispatch.py`, then `dispatch-fallback.py`, then `analyse-dispatch.py` again. The fallback chooses the source root that contains a fixture/example's declared project imports instead of imposing the repository root on every standalone example.
6. Run `dispatch-snippets.py` to parse the 318 quoted/generated/documented candidate fragments. Its results establish syntax only, not runtime or type correctness.
7. Require successful process exits and empty VM stderr. Keep loader diagnostics as data; discard only the known temporary namespace parser gate when assessing identity checking, exactly as project_frontend does. Classify negative tests individually rather than treating their diagnostics as inventory failures.
8. Join raw R and bound B rows by normalised physical path, start offset and end offset. Deduplicate repeated module visits. Prefer physical declaration targets resolved from fully qualified package identities over shortened ambiguous module labels. Normalise display-only nominal type prefixes only when a source declaration is unique. The source span, not displayed type spelling, is the call-site key.
9. Verify 24,125 unique raw spans and 24,099 matches. Inspect the 26 exceptions in the call-site index: two local guard calls and 24 rejected/pruned test sites. Require zero unmatched source, stdlib, package, tool or example sites. Verify that the published ledger has exactly one row per raw source span.
10. Verify the stdlib inventory against all 82 declarations and the built-in register against the 100 names in resolver.is_builtin. Review disposition and packet assignments against D1-D7. Searches locate candidates; compiler metadata determines receiver versus namespace classification.
11. Remove only the temporary probe and generated outputs created for this audit. No source, VM or seed edits are needed. Run `make docs` and `git diff --check` for the resulting informational documents.

The published ledgers preserve compiler-selected targets, first-argument and result type displays, all source spans and migration dispositions. Bound nodes from rejection fixtures are evidence of attempted selection, not accepted signatures. The targeted self-checks compare a real namespace call, a legacy free-function receiver call, an owned imported call and Map method dispatch against their expected distinct identities.

## Temporary Panack probe

```panack
import project/src/compiler/module_bindings::{ModuleIdentity, ModulePackageIdentity.StandaloneModulePackage, ModulePackageIdentity.ManifestModulePackage, ModulePackageIdentity.ToolchainModulePackage}
import project/src/compiler/parser::{Expr, Block, SourceSpan, SourcePos, parse_module_complete, Expr.LocatedExpr, Expr.NatExpr, Expr.DecimalExpr, Expr.StringExpr, Expr.BoolExpr, Expr.NameExpr, Expr.FunctionExpr, Expr.ArrayExpr, Expr.UnaryExpr, Expr.BinaryExpr, Expr.CallExpr, Expr.DotCallExpr, Expr.FieldExpr, Expr.IndexExpr, Expr.IfExpr, Expr.BlockExpr, Expr.MatchExpr, TopLevel.ImportDeclaration, TopLevel.TypeDeclaration, TopLevel.RecordDeclaration, TopLevel.EnumDeclaration, TopLevel.FunctionDeclaration, Statement.BindingStatement, Statement.AssignmentStatement, Statement.ExpressionStatement, Statement.WhileStatement, Statement.ForStatement, BlockValue.NoBlockValue, BlockValue.TailBlockValue}
import project/src/compiler/lexer::{lex_file}
import project/src/compiler/loader::{load_project_for_inventory, load_project_package_for_check}
import project/src/compiler/package_project::{read_manifest_project}
import project/src/compiler/module_bodies::{BoundBody, body_type_text, BodyNode.BodyLiteral, BodyNode.BodyInterpolation, BodyNode.BodyLocalUse, BodyNode.BodyFunction, BodyNode.BodyCall, BodyNode.BodyArray, BodyNode.BodyUnary, BodyNode.BodyBinary, BodyNode.BodyField, BodyNode.BodyIndex, BodyNode.BodyIf, BodyNode.BodyBlock, BodyNode.BodyMatch, BodyNode.BodyBinding, BodyNode.BodyAssignment, BodyNode.BodyWhile, BodyNode.BodyFor, BodyNode.BodyEmpty, BodyTarget.BodyCore, BodyTarget.BodyDeclaration}

pure module_name(m: ModuleIdentity): Str {
 prefix: Str = match m.package {
  StandaloneModulePackage() => "project/",
  ToolchainModulePackage() => "stdlib/",
  ManifestModulePackage(n,v) => n+"@"+v+"/"
 }
 prefix+m.path
}
pure safe(s: Str): Str {
 mut out: Str = ""
 for i in 0..len(s) {
  c = s[i]
  out = out + if c == "\t" || c == "\n" || c == "\r" { " " } else { c }
 }
 out
}
pure number(n: Nat): Str { "${n}" }
pure location(s: SourceSpan): Str {
 s.start.file + "\t" + number(s.start.offset) + "\t" + number(s.end.offset) + "\t" + number(s.start.line) + "\t" + number(s.start.column)
}

raw(e: Expr, at: SourceSpan): Void {
 match e {
 LocatedExpr(span, value, ts) => { raw(value, span) },
 NatExpr(v) => {}, DecimalExpr(v) => {}, StringExpr(v) => {}, BoolExpr(v) => {}, NameExpr(v) => {},
 FunctionExpr(n) => { print("R\t" + location(at) + "\tref\t" + n + "\t0") },
 ArrayExpr(xs) => { for x in xs { raw(x, at) } },
 UnaryExpr(op,x) => { raw(x,at) },
 BinaryExpr(op,x,y) => { raw(x,at); raw(y,at) },
 CallExpr(n,xs,ts) => { print("R\t" + location(at) + "\tcall\t" + n + "\t"+number(len(xs))+""); for x in xs { raw(x,at) } },
 DotCallExpr(r,n,xs,ts) => { print("R\t" + location(at) + "\tdot\t" + n + "\t"+number(len(xs))+""); raw(r,at); for x in xs { raw(x,at) } },
 FieldExpr(r,n) => { raw(r,at) },
 IndexExpr(r,x) => { raw(r,at); raw(x,at) },
 IfExpr(c,y,n,has) => { raw(c,at); block(y,at); block(n,at) },
 BlockExpr(b) => { block(b,at) },
 MatchExpr(s,arms) => { raw(s,at); for arm in arms { raw(arm.body,at) } }
 }
}
block(b: Block, at: SourceSpan): Void {
 for stmt in b.statements {
 match stmt {
 BindingStatement(n,t,sp,m,e) => { raw(e,at) },
 AssignmentStatement(n,e) => { raw(e,at) },
 ExpressionStatement(e) => { raw(e,at) },
 WhileStatement(c,nested) => { raw(c,at); block(nested,at) },
 ForStatement(n,e,nested) => { raw(e,at); block(nested,at) }
 }
 }
 match b.value { NoBlockValue() => {}, TailBlockValue(e) => { raw(e,at) } }
}
bound(b: BoundBody, owner: Str): Void {
 mut target: Str = ""
 match b.node {
 BodyCall(t,ts,args) => { target = match t { BodyCore(n) => "core:"+n, BodyDeclaration(d) => module_name(d.identity.module)+":"+d.identity.name } },
 BodyFunction(d) => { target = "ref:" + module_name(d.identity.module)+":"+d.identity.name },
 BodyLiteral(v) => {}, BodyInterpolation(p) => {}, BodyLocalUse(l) => {},
 BodyArray() => {}, BodyUnary(o) => {}, BodyBinary(o) => {}, BodyField(n) => {}, BodyIndex() => {},
 BodyIf(h) => {}, BodyBlock(v) => {}, BodyMatch(t,p,s) => {},
 BodyBinding(l,t,m,i,e) => {}, BodyAssignment(l,e) => {}, BodyWhile() => {}, BodyFor(l) => {}, BodyEmpty() => {}
 }
 if target != "" {
  mut receiver: Str = ""
  if len(b.children)>0 { receiver = match b.children[0].type { Some(t) => body_type_text(t), None() => "?" } }
  result = match b.type { Some(t) => body_type_text(t), None() => "?" }
  print("B\t"+location(b.span)+"\t"+target+"\t"+number(len(b.children))+"\t"+safe(receiver)+"\t"+safe(result)+"\t"+owner)
 }
 for child in b.children { bound(child,owner) }
}
main(): Void {
 args = command_args()
 mode = args[0]
 mut seen: Set[Str] = set()
 start: Nat = if mode == "package" { 2 } else { 1 }
 for index in start..len(args) {
  file = args[index]
  if mode == "raw" {
   text = read_file(file)
   match lex_file(text,file) {
    Error(ds) => { for d in ds { print("E\t"+file+"\tlex\t"+safe(d.message)) } },
    Ok(tokens) => match parse_module_complete(tokens) {
     Error(d) => { print("E\t"+file+"\tparse\t"+safe(d.message)) },
     Ok(parsed) => {
      print("P\t"+file)
      at = SourceSpan(SourcePos(file,0,1,1),SourcePos(file,0,1,1))
      for d in parsed.program.declarations {
       match d {
        ImportDeclaration(n) => {}, RecordDeclaration(n,ts,fs) => {}, EnumDeclaration(n,ts,vs) => {},
        TypeDeclaration(n,t,e) => { raw(e,at) },
        FunctionDeclaration(n,ps,r,p,a,b,ts) => { block(b,at) }
       }
      }
     }
    }
   }
  } else {
   absolute = path_resolve(file)
   if !seen.has(absolute) {
    print("L\t"+file)
    project = if mode == "package" {
     match read_manifest_project(args[1]) {
      Ok(p) => load_project_package_for_check(file,p.source_root,p.package_root,p.root_identity,p.package,p.version,p.dependencies),
      Error(message) => { print("X\t"+file+"\t"+safe(message)); load_project_for_inventory(file) }
     }
    } else { load_project_for_inventory(file) }
    for d in project.diagnostics { print("D\t"+file+"\t"+safe(d.position.file)+"\t"+number(d.position.line)+"\t"+safe(d.message)) }
    for module in project.modules {
     seen = seen.add(module.source_path)
     print("M\t"+module.source_path+"\t"+module_name(module.bindings.identity))
    }
    for f in project.module_bodies {
     bound(f.body,module_name(f.signature.binding.identity.module)+":"+f.signature.binding.identity.name)
    }
   }
  }
 }
}
```

## run-dispatch-inventory.py

```python
from pathlib import Path
import subprocess, os, json, time
root=Path.cwd()
files=subprocess.check_output(['git','ls-files','*.panack'],text=True).splitlines()
files=[p for p in files if p.startswith(('src/','tests/','packages/','tools/','examples/'))]
Path('/tmp/dispatch-corpus.json').write_text(json.dumps(files))
env=dict(os.environ,PANACKELTY_STDLIB_PATH=str(root/'src/stdlib'),PANACKELTY_VERSION=(root/'VERSION').read_text().strip(),PANACKELTY_BOOTSTRAP_ROOT=str(root))
cmd=['./panack-vm','run','/tmp/dispatch-inventory-probe.bc']
with open('/tmp/dispatch-raw.tsv','w') as out,open('/tmp/dispatch-raw.err','w') as err:
 r=subprocess.run(cmd+['raw']+files,env=env,stdout=out,stderr=err)
print('raw',r.returncode,flush=True)
assert r.returncode==0
manifests=[Path(p) for p in subprocess.check_output(['git','ls-files','*panack.json'],text=True).splitlines() if Path(p).is_file()]
groups={}
standalone=[]
for file in files:
 parents=[m for m in manifests if Path(file).is_relative_to(m.parent)]
 if parents:
  m=max(parents,key=lambda p:len(p.parts));groups.setdefault(str(m),[]).append(file)
 else: standalone.append(file)
standalone.sort(key=lambda p:(0 if p=='src/compiler/main.panack' else 1 if p=='tests/runner/main.panack' else 2 if p.startswith('src/') else 3 if p.startswith('tests/runner/') else 4,p))
with open('/tmp/dispatch-bound.tsv','w') as out,open('/tmp/dispatch-bound.err','w') as err:
 r=subprocess.run(cmd+['bound']+standalone,env=env,stdout=out,stderr=err)
 print('bound',r.returncode,flush=True)
 assert r.returncode==0
 for manifest, paths in groups.items():
  print('package',manifest,flush=True)
  r=subprocess.run(cmd+['package',manifest]+paths,env=env,stdout=out,stderr=err)
  print('result',r.returncode,flush=True)
  assert r.returncode==0
print('DONE',len(files),flush=True)
```

## dispatch-package-pass.py

```python
from pathlib import Path
import subprocess,os,json
root=Path.cwd(); files=json.load(open('/tmp/dispatch-corpus.json'))
ms=[Path(x) for x in subprocess.check_output(['git','ls-files','*panack.json'],text=True).splitlines() if Path(x).is_file()]
env=dict(os.environ,PANACKELTY_STDLIB_PATH=str(root/'src/stdlib'),PANACKELTY_VERSION=(root/'VERSION').read_text().strip())
env.pop('PANACKELTY_BOOTSTRAP_ROOT',None)
with open('/tmp/dispatch-package.tsv','w') as out,open('/tmp/dispatch-package.err','w') as err:
 for m in ms:
  paths=[]
  for f in files:
   parents=[x for x in ms if Path(f).is_relative_to(x.parent)]
   if parents and max(parents,key=lambda p:len(p.parts))==m: paths.append(f)
  if not paths: continue
  print(m,flush=True)
  r=subprocess.run(['./panack-vm','run','/tmp/dispatch-inventory-probe-v2.bc','package',str(m)]+paths,env=env,stdout=out,stderr=err)
  print(r.returncode,flush=True)
  assert r.returncode==0
```

## analyse-dispatch.py

```python
from pathlib import Path
from collections import Counter,defaultdict
import json,re
root=str(Path.cwd())+'/'
def norm(p):return p.removeprefix(root)
raw={}; parse_errors=[]
for l in Path('/tmp/dispatch-raw.tsv').read_text().splitlines():
 p=l.split('\t')
 if p[0]=='R' and len(p)==9:raw[(norm(p[1]),p[2],p[3])]=p
 if p[0]=='E':parse_errors.append(p)
bound=defaultdict(list); diagnostics=[]; modules=set(); loads=[]
for file in ['/tmp/dispatch-bound.tsv','/tmp/dispatch-package.tsv','/tmp/dispatch-fallback.tsv']:
 if not Path(file).exists():continue
 context=''; identities={}
 for l in Path(file).read_text().splitlines():
  p=l.split('\t')
  if p[0]=='L':context=p[1];identities={};loads.append(context)
  if p[0]=='M' and len(p)==3:
   identities.setdefault(p[2],set()).add(norm(p[1]));modules.add(norm(p[1]))
  if p[0]=='D' and len(p)>=5 and 'namespace syntax is parsed' not in p[4]:diagnostics.append(dict(entry=context,file=norm(p[2]),line=p[3],message=p[4]))
  if p[0]=='B' and len(p)==11:
   key=(norm(p[1]),p[2],p[3]);target=p[6]
   ref=target.startswith('ref:')
   if ref:target=target[4:]
   if not target.startswith('core:') and ':' in target:
    module,name=target.rsplit(':',1)
    paths=identities.get(module,set())
    if len(paths)==1:target=next(iter(paths))+':'+name
    else:target=module+':'+name
   if ref:target='ref:'+target
   def canonical_type(value):
    def replace(m):
     key=m[1]; paths=set()
     for identity,found in identities.items():
      if identity==key or identity.endswith('/'+key):paths.update(found)
     return (next(iter(paths)) if len(paths)==1 else key)+':'
    return re.sub(r'([^\[\], ]+\.panack):',replace,value)
   b=[target,p[7],canonical_type(p[8]),canonical_type(p[9]),p[10],context]
   # Compare typed result; repeated module loading doesn't multiply source sites.
   if b[:4] not in [x[:4] for x in bound[key]]:bound[key].append(b)
rows=[]
for key, values in bound.items():
 precise=[b for b in values if b[0].startswith('core:') or Path(b[0].removeprefix('ref:').rsplit(':',1)[0]).exists()]
 if precise:bound[key]=precise
for k,r in raw.items():
 bs=bound.get(k,[])
 for b in bs or [['?', '?','?','?','?','?']]:
  kind=r[6]
  if kind=='dot' and b[1]!='?':
   if int(b[1])==int(r[8])+1:kind='receiver'
   elif int(b[1])==int(r[8]):kind='namespace'
   else:kind='invalid-dot'
  if r[7].startswith('$owned_call$'):kind='owned'
  rows.append(dict(file=k[0],start=k[1],end=k[2],line=r[4],col=r[5],kind=kind,name=r[7],argc=r[8],target=b[0],bound_argc=b[1],receiver=b[2],result=b[3],function=b[4],context=b[5]))
Path('/tmp/dispatch-joined.json').write_text(json.dumps(rows))
Path('/tmp/dispatch-diagnostics.json').write_text(json.dumps(diagnostics))
print('raw unique',len(raw),'bound unique',len(bound),'classes',Counter(x['kind'] for x in rows))
print('unbound production:',[(x['file'],x['line'],x['name']) for x in rows if x['target']=='?' and x['file'].startswith(('src/','packages/','tools/'))][:20])
print('resolved receiver families:')
for key,n in Counter((x['name'],x['target'],x['receiver'].split('[')[0]) for x in rows if x['kind']=='receiver').most_common():print(n,key)
```

## dispatch-fallback.py

```python
import json,subprocess,os,re
from pathlib import Path
root=Path.cwd(); r=json.load(open('/tmp/dispatch-joined.json'))
files=sorted({x['file'] for x in r if x['target']=='?'})
groups={}
for f in files:
 p=Path(f); s=p.read_text()
 imports=re.findall(r'(?:^|\n)\s*(?:pub\s+)?import\s+project/([A-Za-z_0-9/]+)',s)
 candidates=[]
 for ancestor in p.parents:
  if all((ancestor/(name+'.panack')).exists() or (ancestor/name).exists() for name in imports):candidates.append(ancestor)
 selected=candidates[0] if candidates else p.parent
 if not imports:selected=p.parent
 groups.setdefault(str(selected.resolve()),[]).append(f)
with open('/tmp/dispatch-fallback.tsv','w') as out,open('/tmp/dispatch-fallback.err','w') as err:
 for project_root, fs in groups.items():
  env=dict(os.environ,PANACKELTY_STDLIB_PATH=str(root/'src/stdlib'),PANACKELTY_VERSION=(root/'VERSION').read_text().strip(),PANACKELTY_BOOTSTRAP_ROOT=project_root)
  print(project_root,len(fs),flush=True)
  r=subprocess.run(['./panack-vm','run','/tmp/dispatch-inventory-probe-v2.bc','bound']+fs,env=env,stdout=out,stderr=err)
  assert r.returncode==0
print('done',flush=True)
```

## dispatch-snippets.py

```python
from pathlib import Path
import subprocess,re,json,hashlib
root=Path.cwd(); out=Path('/tmp/dispatch-snippets');out.mkdir(exist_ok=True)
entries=[]
pattern=re.compile(r'(?:\.|->)[A-Za-z_][A-Za-z_0-9]*(?:\[[^\n]*?\])?\(')
def add(file,line,kind,source):
 if not pattern.search(source):return
 i=len(entries); p=out/f'{i:04}.panack';p.write_text(source)
 entries.append(dict(file=file,line=line,kind=kind,path=str(p),sha256=hashlib.sha256(source.encode()).hexdigest(),methods=sorted(set(re.findall(r'(?:\.|->)([A-Za-z_][A-Za-z_0-9]*)(?:\[[^\n]*?\])?\(',source)))))
files=subprocess.check_output(['git','ls-files'],text=True).splitlines()
for file in files:
 p=Path(file)
 if p.suffix not in ('.panack','.sh','.md','.c','.mjs'):continue
 text=p.read_text(errors='replace')
 if p.suffix=='.panack':
  for m in re.finditer(r'//[^\n]*|"(?:\\.|[^"\\])*"',text):
   if not m.group().startswith('"'):continue
   value=re.sub(r'\\(.)',lambda x: {'n':'\n','r':'\r','t':'\t','b':'\b','f':'\f'}.get(x[1],x[1]),m.group()[1:-1],flags=re.S)
   add(file,text[:m.start()].count('\n')+1,'quoted-template',value)
 elif p.suffix=='.md':
  for m in re.finditer(r'^```(?:panack|panackelty)\s*\n(.*?)^```',text,re.M|re.S):add(file,text[:m.start(1)].count('\n')+1,'documentation',m[1])
 elif p.suffix=='.sh':
  for m in re.finditer(r"<<-?\s*['\"]?([A-Za-z_][A-Za-z_0-9]*)['\"]?[^\n]*\n",text):
   end=re.search(r'^\t*'+re.escape(m[1])+r'\s*$',text[m.end():],re.M)
   if end:add(file,text[:m.end()].count('\n')+1,'shell-heredoc',text[m.end():m.end()+end.start()])
 if p.suffix in ('.sh','.c','.mjs'):
  for m in re.finditer(r'''"(?:\\.|[^"\\])*"|'[^']*'|`[^`]*`''',text):
   value=m.group()[1:-1]
   if not any(marker in value for marker in ['main(', 'pure ', 'record ', 'async ', 'enum ']):continue
   if m.group().startswith('"'):
    value=re.sub(r'\\(.)',lambda x: {'n':'\n','r':'\r','t':'\t'}.get(x[1],x[1]),value,flags=re.S)
   add(file,text[:m.start()].count('\n')+1,'host-quoted-template',value)
Path('/tmp/dispatch-snippet-origins.json').write_text(json.dumps(entries))
print('snippets',len(entries),flush=True)
with open('/tmp/dispatch-snippet-raw.tsv','w') as dest,open('/tmp/dispatch-snippet-raw.err','w') as err:
 r=subprocess.run(['./panack-vm','run','/tmp/dispatch-inventory-probe-v2.bc','raw']+[e['path'] for e in entries],stdout=dest,stderr=err)
 print('exit',r.returncode,flush=True)
 assert r.returncode==0
```
