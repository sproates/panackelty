#!/bin/sh
set -eu
root=$(pwd -P)
work=$(mktemp -d "$root/build/package-git-integration.XXXXXX")
pid=
cleanup() { status=$?; if [ "$status" -ne 0 ]; then cat "$work/out" "$work/err" "$work/server.log" "$work/backend.log" 2>/dev/null || :; fi; if [ -n "$pid" ]; then kill "$pid" 2>/dev/null || :; wait "$pid" 2>/dev/null || :; fi; rm -rf "$work"; }
trap cleanup EXIT
trap 'exit 1' HUP INT TERM
export GIT_CONFIG_NOSYSTEM=1 GIT_CONFIG_GLOBAL=/dev/null
panack="$root/bin/panack"
version=$(cat VERSION)
mkdir -p "$work/source" "$work/app/src"
/usr/bin/git -C "$work/source" init -q --template=
node - "$work" "$version" <<'JS'
const fs=require('fs'); const [root,version]=process.argv.slice(2);
const manifest=(name,deps)=>({schema:1,package:'git.'+name,version:'1.0.0',language:'modules-v1',toolchain:version,source:'src',exports:{api:'api.panack'},dependencies:deps});
for (const name of ['left','right','shared']) {
 const dir=root+'/source/packages/'+name;fs.mkdirSync(dir+'/src',{recursive:true});
 fs.writeFileSync(dir+'/panack.json', JSON.stringify(manifest(name,name==='shared'?{}:{common:{path:'../shared'}})));
 fs.writeFileSync(dir+'/src/api.panack',name==='shared'?'pub pure answer(): Nat { 21 }\n':'import common/api as common\npub pure answer(): Nat { common.answer() }\n');
}
fs.symlinkSync('packages/left',root+'/source/left-link');
fs.symlinkSync('api.panack',root+'/source/packages/shared/src/alias.panack');
fs.writeFileSync(root+'/app/src/main.panack','import left/api as left\nimport right/api as right\nmain(): Void { print(left.answer() + right.answer()) }\n');
JS
/usr/bin/git -C "$work/source" add .
/usr/bin/git -C "$work/source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm initial
/usr/bin/git -C "$work/source" branch -M main
/usr/bin/git -C "$work/source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com tag -am release release
commit=$(/usr/bin/git -C "$work/source" rev-parse HEAD)
/usr/bin/git clone -q --bare "$work/source" "$work/repo.git"
cat > "$work/certificate.cnf" <<'CONF'
[req]
prompt=no
distinguished_name=dn
x509_extensions=ext
[dn]
CN=localhost
[ext]
subjectAltName=DNS:localhost
basicConstraints=critical,CA:TRUE
keyUsage=critical,keyCertSign,digitalSignature,keyEncipherment
CONF
openssl req -config "$work/certificate.cnf" -x509 -newkey rsa:2048 -nodes -keyout "$work/test.key" -out "$work/test.crt" -days 2 > "$work/cert.log" 2>&1
export CURL_CA_BUNDLE="$work/test.crt"
node tests/package_git_fixture.cjs "$work" > "$work/server.log" 2>&1 &
pid=$!
attempt=0
while [ ! -s "$work/port" ] && [ "$attempt" -lt 100 ]; do
  kill -0 "$pid"
  attempt=$((attempt+1))
  sleep .1
done
port=$(cat "$work/port")
url="https://localhost:$port/repo.git"
manifest="$work/app/panack.json"
lock="$work/app/panack.lock.json"
manifest_for() {
node - "$manifest" "$version" "$url" "$1" <<'JS'
const fs=require('fs');const [file,version,url,ref]=process.argv.slice(2);
const git=subdir=>({git:{url,...(ref?{ref}:{}),subdir}});
fs.writeFileSync(file,JSON.stringify({schema:1,package:'git.app',version:'1.0.0',language:'modules-v1',toolchain:version,source:'src',entry:'main.panack',exports:{},dependencies:{left:git('left-link'),right:git('packages/right')}}));
JS
}
requests() { if [ -f "$work/connections" ]; then wc -l < "$work/connections" | tr -d ' '; else printf 0; fi; }
expect() { wanted=$1; shift; "$panack" "$@" > "$work/out" 2> "$work/err"; test "$(cat "$work/out")" = "$wanted"; test ! -s "$work/err"; }
reject() { status=0; "$panack" "$@" > "$work/out" 2> "$work/err" || status=$?; test "$status" = 1; test -s "$work/err"; }
mkdir "$work/git-bin"
cat > "$work/git-bin/git" <<SH
#!/bin/sh
printf 'git\\n' >> '$work/git-invocations'
exec /usr/bin/git "\$@"
SH
chmod +x "$work/git-bin/git"
export PATH="$work/git-bin:$PATH"
manifest_for release
/usr/bin/time -p "$panack" deps lock --project "$manifest" > "$work/out" 2> "$work/cold.time"
test "$(cat "$work/out")" = ok
printf '%s\n' 'Git integration cold lock timing:'
cat "$work/cold.time"
node - "$lock" "$commit" "$url" <<'JS'
const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const [file,commit,url]=process.argv.slice(2);const lock=JSON.parse(fs.readFileSync(file));
assert.equal(lock.packages.length,3);
assert.equal(lock.root.dependencies.left.source.subdir,'packages/left');
const left=lock.packages.find(p=>p.name==='git.left'),right=lock.packages.find(p=>p.name==='git.right');
assert.deepEqual(left.dependencies.common,right.dependencies.common);
assert.equal(left.dependencies.common.source.subdir,'packages/shared');
for(const p of lock.packages)assert.equal(p.tree_sha256,left.tree_sha256);
const fields=[url,'sha1',commit].flatMap(s=>{const b=Buffer.from(s),n=Buffer.alloc(8);n.writeBigUInt64BE(BigInt(b.length));return[n,b]});
const key=crypto.createHash('sha256').update(Buffer.concat([Buffer.from('PANACK-GIT-SOURCE\0v1\0'),...fields])).digest('hex');
const sources=file.replace('/panack.lock.json','/.panack_modules/.sources');
assert.deepEqual(fs.readdirSync(sources),[key]);
assert.equal(lock.root.dependencies.left.source.commit,commit);
JS
cp "$lock" "$work/pinned.lock"
cp "$work/app/.panack_modules/.state/lock.json" "$work/pinned.state"
cp "$work/git-invocations" "$work/cold.git-invocations"
count=$(requests)
expect ok deps lock --project "$manifest"
expect ok check --project "$manifest"
expect "wrote $work/app.bc" compile --project "$manifest" -o "$work/app.bc"
expect 42 run --project "$manifest"
expect 42 "$work/app.bc"
expect ok deps restore --project "$manifest" --offline
expect ok deps restore --project "$manifest"
test "$(requests)" = "$count"
cmp "$work/git-invocations" "$work/cold.git-invocations"
/usr/bin/time -p "$panack" run --project "$manifest" > "$work/out" 2> "$work/warm.time"
test "$(cat "$work/out")" = 42
printf '%s\n' 'Git integration warm project run timing:'
cat "$work/warm.time"
cmp "$lock" "$work/pinned.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/pinned.state"
printf '%s\n' 'PASS Git annotated tag, canonical linked subdir, shared snapshot, sibling diamond, source/bytecode and network-free warm commands'
# A moved selector does not affect restore or ordinary commands.
printf 'pub pure answer(): Nat { 22 }\n' > "$work/source/packages/shared/src/api.panack"
/usr/bin/git -C "$work/source" add .
/usr/bin/git -C "$work/source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm moved
/usr/bin/git -C "$work/source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com tag -fam moved release >/dev/null
/usr/bin/git -C "$work/source" push -q --force "$work/repo.git" main refs/tags/release
expect 42 run --project "$manifest"
mv "$work/app/.panack_modules" "$work/old-cache"
reject deps restore --project "$manifest" --offline
test "$(requests)" = "$count"
expect ok deps restore --project "$manifest"
expect 42 run --project "$manifest"
cmp "$lock" "$work/pinned.lock"
cp "$lock" "$work/pre-selective.lock"
cp "$work/app/.panack_modules/.state/lock.json" "$work/pre-selective.state"
reject deps update --project "$manifest" left
cmp "$lock" "$work/pre-selective.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/pre-selective.state"
expect 42 run --project "$manifest"
expect ok deps update --project "$manifest" left right
expect 44 run --project "$manifest"
printf '%s\n' 'PASS Git moved ref preserves pin, cold offline rejection, exact-commit restore and explicit update'
# Corruption must be detected before compilation; a failed reacquisition preserves it.
# Locate the current source through its locked commit rather than directory order.
cache=$(node - "$lock" <<'JS'
const fs=require('fs'),crypto=require('crypto');const file=process.argv[2],s=JSON.parse(fs.readFileSync(file)).root.dependencies.left.source;
const chunks=[Buffer.from('PANACK-GIT-SOURCE\0v1\0')];for(const v of [s.url,s.object_format,s.commit]){const b=Buffer.from(v),n=Buffer.alloc(8);n.writeBigUInt64BE(BigInt(b.length));chunks.push(n,b)};
console.log(file.replace('/panack.lock.json','/.panack_modules/.sources/')+crypto.createHash('sha256').update(Buffer.concat(chunks)).digest('hex'));
JS
)
printf corrupt > "$cache/packages/shared/src/api.panack"
count=$(requests)
reject run --project "$manifest"
reject deps restore --project "$manifest" --offline
test "$(requests)" = "$count"
expect ok deps restore --project "$manifest"
expect 44 run --project "$manifest"
# Default HEAD and fully qualified branch selectors are accepted.
manifest_for ''
expect ok deps update --project "$manifest"
expect 44 run --project "$manifest"
manifest_for refs/heads/main
expect ok deps update --project "$manifest"
expect 44 run --project "$manifest"
manifest_for "$commit"
expect ok deps update --project "$manifest"
expect 42 run --project "$manifest"
printf '%s\n' 'PASS Git corruption recovery, default HEAD, full branch ref and full commit'
# Snapshot confinement and package-private source confinement are separate gates.
cp "$manifest" "$work/good.manifest"
cp "$lock" "$work/good.lock"
cp "$work/app/.panack_modules/.state/lock.json" "$work/good.state"
printf '%s\n' 'import common/api as common' 'pub pure answer(): Nat { common.answer() }' > "$work/source/packages/left/src/api.panack"
node - "$work/source/packages/left/panack.json" <<'JS'
const fs=require('fs'),f=process.argv[2],m=JSON.parse(fs.readFileSync(f));m.dependencies.common.path='../../../outside';fs.writeFileSync(f,JSON.stringify(m));
JS
/usr/bin/git -C "$work/source" add .
/usr/bin/git -C "$work/source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm escape
/usr/bin/git -C "$work/source" push -q "$work/repo.git" main
manifest_for main
reject deps update --project "$manifest"
grep 'points outside the downloaded Git repository' "$work/err" >/dev/null
cmp "$lock" "$work/good.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/good.state"
cp "$work/good.manifest" "$manifest"
expect 42 run --project "$manifest"
# Snapshot-safe link into a sibling still cannot expose that sibling's private code.
/usr/bin/git -C "$work/source" checkout -q "$commit" -- packages/left/panack.json
ln -s ../../shared/src/api.panack "$work/source/packages/left/src/private.panack"
printf '%s\n' 'import "private.panack" as private' 'pub pure answer(): Nat { private.answer() }' > "$work/source/packages/left/src/api.panack"
/usr/bin/git -C "$work/source" add .
/usr/bin/git -C "$work/source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm private-link
/usr/bin/git -C "$work/source" push -q "$work/repo.git" main
manifest_for main
expect ok deps update --project "$manifest"
count=$(requests)
reject run --project "$manifest"
test "$(requests)" = "$count"
printf '%s\n' 'PASS Git selective conflict is atomic, sibling escape rejected and compiler enforces package-private symlink containment'
# No staging directories survive successful or failed tool processes.
node - "$work/app/.panack_modules" <<'JS'
const fs=require('fs'),assert=require('assert'),root=process.argv[2];
assert.deepEqual(fs.readdirSync(root).sort(),['.sources','.state']);
assert(fs.readdirSync(root+'/.sources').every(n=>/^[0-9a-f]{64}$/.test(n)));
JS
# A SHA-256 repository with a package at its root exercises the other object format.
mkdir -p "$work/source256/src"
node - "$work/source256/panack.json" "$version" <<'JS'
const fs=require('fs');fs.writeFileSync(process.argv[2],JSON.stringify({schema:1,package:'git.root',version:'1.0.0',language:'modules-v1',toolchain:process.argv[3],source:'src',exports:{api:'api.panack'},dependencies:{}}));
JS
printf 'pub pure answer(): Nat { 30 }\n' > "$work/source256/src/api.panack"
/usr/bin/git -C "$work/source256" init -q --template= --object-format=sha256
/usr/bin/git -C "$work/source256" add .
/usr/bin/git -C "$work/source256" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm sha256
/usr/bin/git clone -q --bare "$work/source256" "$work/sha.git"
url="https://localhost:$port/sha.git"
manifest_for ''
node - "$manifest" <<'JS'
const fs=require('fs'),file=process.argv[2],m=JSON.parse(fs.readFileSync(file));for(const d of Object.values(m.dependencies))delete d.git.subdir;fs.writeFileSync(file,JSON.stringify(m));
JS
expect ok deps update --project "$manifest"
expect 60 run --project "$manifest"
node - "$lock" <<'JS'
const fs=require('fs'),assert=require('assert'),lock=JSON.parse(fs.readFileSync(process.argv[2]));
assert.equal(lock.packages.length,1);assert.equal(lock.root.dependencies.left.source.object_format,'sha256');assert.equal(lock.root.dependencies.left.source.commit.length,64);assert.equal(lock.root.dependencies.left.source.subdir,'.');
JS
count=$(requests)
expect ok deps restore --project "$manifest" --offline
test "$(requests)" = "$count"
printf '%s\n' 'PASS SHA-256 Git root package, omitted selector/subdir and repeated aliases'
# P03d: one graph with both remote source kinds. Each selective update must
# preserve the other source even when both remote versions have changed.
mkdir -p "$work/archive/src"
node - "$work/archive/panack.json" "$version" "$manifest" "$port" <<'JS'
const fs=require('fs');const [file,version,app,port]=process.argv.slice(2);
fs.writeFileSync(file,JSON.stringify({schema:1,package:'remote.archive',version:'1.0.0',language:'modules-v1',toolchain:version,source:'src',exports:{api:'api.panack'},dependencies:{}}));
const m=JSON.parse(fs.readFileSync(app));delete m.dependencies.right;m.dependencies.archive={archive:`https://localhost:${port}/package.tar.gz`};fs.writeFileSync(app,JSON.stringify(m));
JS
printf 'pub pure answer(): Nat { 10 }\n' > "$work/archive/src/api.panack"
pack_remote() { COPYFILE_DISABLE=1 tar --format ustar -czf "$work/package.tar.gz" -C "$work/archive" .; }
pack_remote
printf '%s\n' 'import left/api as left' 'import archive/api as archive' 'main(): Void { print(left.answer() + archive.answer()) }' > "$work/app/src/main.panack"
expect ok deps update --project "$manifest"
expect 40 run --project "$manifest"
cp "$lock" "$work/mixed.lock"
printf 'pub pure answer(): Nat { 31 }\n' > "$work/source256/src/api.panack"
/usr/bin/git -C "$work/source256" add .
/usr/bin/git -C "$work/source256" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm advanced
/usr/bin/git -C "$work/source256" push -q "$work/sha.git" HEAD
printf 'pub pure answer(): Nat { 11 }\n' > "$work/archive/src/api.panack"
pack_remote
cp "$work/git-invocations" "$work/before-archive.git"
count=$(requests)
expect ok deps update --project "$manifest" archive
test "$(requests)" = "$((count + 1))"
cmp "$work/git-invocations" "$work/before-archive.git"
node - "$lock" "$work/mixed.lock" <<'JS'
const fs=require('fs'),assert=require('assert');const [a,b]=process.argv.slice(2).map(f=>JSON.parse(fs.readFileSync(f)));
assert.deepEqual(a.root.dependencies.left,b.root.dependencies.left);assert.deepEqual(a.packages.find(p=>p.name==='git.root'),b.packages.find(p=>p.name==='git.root'));
assert.notDeepEqual(a.root.dependencies.archive,b.root.dependencies.archive);
JS
expect 41 run --project "$manifest"
cp "$lock" "$work/archive-updated.lock"
archive_requests=$(grep -c 'GET /package.tar.gz' "$work/requests")
expect ok deps update --project "$manifest" left
test "$(grep -c 'GET /package.tar.gz' "$work/requests")" = "$archive_requests"
node - "$lock" "$work/archive-updated.lock" <<'JS'
const fs=require('fs'),assert=require('assert');const [a,b]=process.argv.slice(2).map(f=>JSON.parse(fs.readFileSync(f)));
assert.deepEqual(a.root.dependencies.archive,b.root.dependencies.archive);assert.deepEqual(a.packages.find(p=>p.name==='remote.archive'),b.packages.find(p=>p.name==='remote.archive'));
assert.notDeepEqual(a.root.dependencies.left,b.root.dependencies.left);
JS
expect 42 run --project "$manifest"
cp "$lock" "$work/mixed-current.lock"
cp "$work/app/.panack_modules/.state/lock.json" "$work/mixed-current.state"
cp "$work/git-invocations" "$work/mixed-current.git"
count=$(requests)
expect ok check --project "$manifest"
expect "wrote $work/mixed.bc" compile --project "$manifest" -o "$work/mixed.bc"
expect 42 "$work/mixed.bc"
expect ok deps restore --project "$manifest" --offline
expect ok deps restore --project "$manifest"
test "$(requests)" = "$count"
cmp "$work/git-invocations" "$work/mixed-current.git"
cmp "$lock" "$work/mixed-current.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/mixed-current.state"
printf '%s\n' 'PASS mixed remote graph selective updates preserve unselected pins and avoid the unselected transport'
# Corrupt both caches. Neither ordinary commands nor offline restore may repair
# implicitly. An interrupted online acquisition must preserve every cache byte.
cache=$(find "$work/app/.panack_modules/.sources" -path '*/src/api.panack' -exec grep -l '31' {} \;)
test -n "$cache"
printf corrupt-git > "$cache"
printf corrupt-archive > "$work/app/.panack_modules/remote.archive/src/api.panack"
cache_fingerprint() {
node - "$work/app/.panack_modules" <<'JS'
const fs=require('fs'),crypto=require('crypto'),path=require('path');const h=crypto.createHash('sha256');
function visit(p,relative){const s=fs.lstatSync(p);h.update(relative+'\0');if(s.isSymbolicLink())h.update('L'+fs.readlinkSync(p));else if(s.isDirectory()){h.update('D');for(const n of fs.readdirSync(p).sort())visit(path.join(p,n),relative+'/'+n)}else {h.update('F');h.update(fs.readFileSync(p))}}
visit(process.argv[2],'');console.log(h.digest('hex'));
JS
}
corrupt_tree=$(cache_fingerprint)
count=$(requests)
reject run --project "$manifest"
reject deps restore --project "$manifest" --offline
test "$(requests)" = "$count"
cmp "$work/git-invocations" "$work/mixed-current.git"
touch "$work/interrupt-peer"
reject deps restore --project "$manifest"
test "$(cache_fingerprint)" = "$corrupt_tree"
cmp "$lock" "$work/mixed-current.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/mixed-current.state"
rm "$work/interrupt-peer"
expect ok deps restore --project "$manifest"
expect 42 run --project "$manifest"
printf '%s\n' 'PASS mixed corrupt-cache and interrupted acquisition preserve lock, state and complete cache tree'
# Exercise Git interruption independently, after the archive has been repaired.
printf corrupt-git > "$cache"
corrupt_tree=$(cache_fingerprint)
printf 'git\n' > "$work/interrupt-peer"
reject deps restore --project "$manifest"
test "$(cache_fingerprint)" = "$corrupt_tree"
cmp "$lock" "$work/mixed-current.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/mixed-current.state"
rm "$work/interrupt-peer"
expect ok deps restore --project "$manifest"
expect 42 run --project "$manifest"
printf '%s\n' 'PASS interrupted Git restore preserves mixed cache and recovers using the existing pin'
# Make only the locked commit unavailable while leaving the HTTPS repository
# reachable. Restore must not fall forward to another commit or rewrite the pin.
mv "$work/app/.panack_modules/.sources" "$work/mixed-sources"
mv "$work/sha.git" "$work/sha.saved.git"
/usr/bin/git init -q --bare --template= --object-format=sha256 "$work/sha.git"
reject deps restore --project "$manifest"
grep -F 'error[PKG_GIT_FETCH]:' "$work/err" >/dev/null
grep -F 'Requested commit:' "$work/err" >/dev/null
grep -F 'A Git exit status alone does not identify the cause' "$work/err" >/dev/null
node tests/package_diagnostic_capture.cjs git-unavailable "$work/err" "$work" "$port"
cmp "$lock" "$work/mixed-current.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/mixed-current.state"
test "$(cat "$work/app/.panack_modules/remote.archive/src/api.panack")" = 'pub pure answer(): Nat { 11 }'
mv "$work/sha.git" "$work/sha.empty.git"
mv "$work/sha.saved.git" "$work/sha.git"
expect ok deps restore --project "$manifest"
expect 42 run --project "$manifest"
printf '%s\n' 'PASS public restore rejects unavailable locked Git commit and recovers without changing the pin'
# Changed archive bytes at the locked URL are rejected after Git is verified.
# This is a digest failure, not a transport outage.
printf 'pub pure answer(): Nat { 12 }\n' > "$work/archive/src/api.panack"
pack_remote
printf corrupt > "$work/app/.panack_modules/remote.archive/src/api.panack"
corrupt_tree=$(cache_fingerprint)
reject deps restore --project "$manifest"
test "$(cache_fingerprint)" = "$corrupt_tree"
cmp "$lock" "$work/mixed-current.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/mixed-current.state"
expect ok deps update --project "$manifest" archive
expect 43 run --project "$manifest"
node - "$work/app/.panack_modules" <<'JS'
const fs=require('fs'),assert=require('assert'),root=process.argv[2];
assert.deepEqual(fs.readdirSync(root).sort(),['.sources','.state','remote.archive']);
assert(fs.readdirSync(root+'/.sources').every(n=>/^[0-9a-f]{64}$/.test(n)));
JS
printf '%s\n' 'PASS mixed graph changed archive bytes fail atomically, explicit update succeeds, all staging is cleaned'
