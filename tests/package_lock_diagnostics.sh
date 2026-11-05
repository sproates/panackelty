#!/bin/sh
# Lock validation must explain the rejected record and preserve the supplied file.
set -eu
root=$(pwd -P)
work=$(mktemp -d "$root/build/package-lock-diagnostics.XXXXXX")
cleanup() { status=$?; if [ "$status" != 0 ]; then cat "$work/out" "$work/err" 2>/dev/null || :; fi; rm -rf "$work"; }
trap cleanup EXIT
trap 'exit 1' HUP INT TERM
panack="$root/bin/panack"
manifest="$work/app/panack.json"
lock="$work/app/panack.lock.json"
node - "$work" "$(cat VERSION)" <<'JS'
const fs=require('fs');const [w,v]=process.argv.slice(2);
const m=(name,deps={})=>({schema:1,package:name,version:'1.0.0',language:'modules-v1',toolchain:v,source:'src',exports:{api:'api.panack'},dependencies:deps});
for(const [dir,name,deps] of [['app','demo.app',{lib:{path:'../library'}}],['library','demo.library',{}]]){
 fs.mkdirSync(w+'/'+dir+'/src',{recursive:true});fs.writeFileSync(w+'/'+dir+'/panack.json',JSON.stringify(m(name,deps)));fs.writeFileSync(w+'/'+dir+'/src/api.panack','pub pure answer(): Nat { 42 }\n');
}
JS
"$panack" deps lock --project "$manifest" > "$work/out" 2> "$work/err"
cp "$lock" "$work/good.lock"
node - "$work" <<'JS'
const fs=require('fs');const w=process.argv[2],good=JSON.parse(fs.readFileSync(w+'/good.lock'));fs.mkdirSync(w+'/cases');
function sample(name,mutate){const l=structuredClone(good);mutate(l);fs.writeFileSync(w+'/cases/'+name+'.json',JSON.stringify(l));}
sample('lock-schema',l=>l.lock_schema=2);
sample('lock-unknown-field',l=>l.unexpected=true);
sample('lock-string-type',l=>l.root.name=42);
sample('lock-record-type',l=>l.root=[]);
sample('lock-manifest-checksum',l=>l.root.manifest_sha256='bad');
sample('lock-duplicate-package',l=>l.packages.push(l.packages[0]));
sample('lock-multiple-versions',l=>l.packages.push({...l.packages[0],version:'2.0.0'}));
sample('lock-unreferenced-package',l=>l.root.dependencies={});
sample('lock-missing-package',l=>l.packages=[]);
sample('lock-unexpected-tree',l=>l.packages[0].tree_sha256='0'.repeat(64));
sample('lock-missing-tree',l=>l.root.dependencies.lib.source={kind:'archive',path:'../library.tar.gz',archive_sha256:'0'.repeat(64)});
sample('lock-source-kind',l=>l.root.dependencies.lib.source.kind='registry');
sample('lock-archive-locations',l=>{l.root.dependencies.lib.source={kind:'archive',path:'../library.tar.gz',url:'https://example.test/library.tar.gz',archive_sha256:'0'.repeat(64)};l.packages[0].tree_sha256='0'.repeat(64);});
sample('lock-git-commit',l=>{l.root.dependencies.lib.source={kind:'git',url:'https://example.test/repo',object_format:'sha1',commit:'abc'};l.packages[0].tree_sha256='0'.repeat(64);});
JS
reject() {
    scenario=$1; code=$2; shift 2
    status=0
    "$panack" "$@" > "$work/out" 2> "$work/err" || status=$?
    test "$status" = 1
    test ! -s "$work/out"
    grep -F "panack: error[$code]:" "$work/err" >/dev/null
    node tests/package_diagnostic_capture.cjs "$scenario" "$work/err" "$work"
}
for file in "$work/cases/"*.json; do
    scenario=$(basename "$file" .json)
    cp "$file" "$lock"
    reject "$scenario" PKG_LOCK_INVALID deps restore --project "$manifest"
    cmp "$file" "$lock"
    test ! -e "$work/app/.panack_modules"
done
cp "$work/good.lock" "$lock"
printf ' ' >> "$manifest"
reject lock-project-changed PKG_LOCK_STALE check --project "$manifest"
cmp "$work/good.lock" "$lock"
"$panack" deps update --project "$manifest" > "$work/out" 2> "$work/err"
"$panack" check --project "$manifest" > "$work/out" 2> "$work/err"
cp "$lock" "$work/updated.lock"
printf ' ' >> "$work/library/panack.json"
reject lock-dependency-changed PKG_LOCK_STALE check --project "$manifest"
cmp "$work/updated.lock" "$lock"
"$panack" deps update --project "$manifest" lib > "$work/out" 2> "$work/err"
"$panack" check --project "$manifest" > "$work/out" 2> "$work/err"
test ! -s "$work/err"
printf 'package lock diagnostics and deliberate update recovery passed\n'
