#!/bin/sh
# Validate causes and complete public diagnostics without changing existing locks.
set -eu
root=$(pwd -P)
work=$(mktemp -d "$root/build/package-validation.XXXXXX")
cleanup() {
    status=$?
    if [ "$status" != 0 ]; then cat "$work/out" "$work/err" 2>/dev/null || :; fi
    chmod u+rwx "$work/app" "$work/app/panack.json" "$work/app/panack.lock.json" 2>/dev/null || :
    rm -rf "$work"
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM
panack="$root/bin/panack"
version=$(cat VERSION)
node - "$work" "$version" <<'JS'
const fs=require('fs');const [w,v]=process.argv.slice(2);
const m=(name,deps={})=>({schema:1,package:name,version:'1.0.0',language:'modules-v1',toolchain:v,source:'src',exports:{api:'api.panack'},dependencies:deps});
function put(dir,value){fs.mkdirSync(w+'/'+dir+'/src',{recursive:true});fs.writeFileSync(w+'/'+dir+'/panack.json',JSON.stringify(value));fs.writeFileSync(w+'/'+dir+'/src/api.panack','pub pure answer(): Nat { 42 }\n');}
put('app',m('demo.app'));put('library',m('demo.library'));put('other',m('demo.library'));put('application',{...m('demo.application'),exports:{},entry:'main.panack'});
fs.writeFileSync(w+'/application/src/main.panack','main(): Void { print(42) }\n');
fs.writeFileSync(w+'/app/src/main.panack','main(): Void { print(42) }\n');
fs.copyFileSync(w+'/app/panack.json',w+'/app/good.json');
JS
manifest="$work/app/panack.json"
lock="$work/app/panack.lock.json"
reject() {
    scenario=$1; code=$2; shift 2
    status=0
    "$panack" "$@" > "$work/out" 2> "$work/err" || status=$?
    test "$status" = 1 || { cat "$work/err"; exit 1; }
    test ! -s "$work/out"
    grep -F "panack: error[$code]:" "$work/err" >/dev/null || { cat "$work/err"; exit 1; }
    node tests/package_diagnostic_capture.cjs "$scenario" "$work/err" "$work"
}
reject command-missing-operation PKG_USAGE deps
reject command-missing-path PKG_USAGE deps lock --project
reject command-offline-update PKG_USAGE deps update --project "$manifest" --offline
reject command-extra-name PKG_USAGE deps restore --project "$manifest" lib
reject command-unknown-option PKG_USAGE deps lock --project "$manifest" --unknown
reject selected-without-lock PKG_LOCK_REQUIRED deps update --project "$manifest" lib
test ! -e "$lock"
test ! -e "$work/app/.panack_modules"
command=$(sed -n 's/^  \(panack deps .*\)/\1/p' "$work/err")
test -n "$command"
PATH="$root/bin:$PATH" sh -c "$command" > "$work/out" 2> "$work/err"
test "$(cat "$work/out")" = ok
test ! -s "$work/err"
test -f "$lock"
rm "$lock"
reject manifest-wrong-name PKG_MANIFEST_INVALID deps lock --project "$work/app/good.json"
mv "$manifest" "$work/saved.json"
mkdir "$manifest"
reject manifest-directory PKG_FILESYSTEM deps lock --project "$manifest"
rmdir "$manifest"
mv "$work/saved.json" "$manifest"
node - "$manifest" <<'JS'
const fs=require('fs');const p=process.argv[2];fs.writeFileSync(p,fs.readFileSync(p,'utf8')+' '.repeat(65536));
JS
reject manifest-size-limit PKG_MANIFEST_INVALID deps lock --project "$manifest"
cp "$work/app/good.json" "$manifest"
node - "$manifest" <<'JS'
require('fs').writeFileSync(process.argv[2],'['.repeat(34)+'0'+']'.repeat(34));
JS
reject manifest-depth-limit PKG_MANIFEST_INVALID deps lock --project "$manifest"
cp "$work/app/good.json" "$manifest"
"$panack" deps lock --project "$manifest" > "$work/out" 2> "$work/err"
test "$(cat "$work/out")" = ok
test ! -s "$work/err"
cp "$lock" "$work/good.lock"
rm "$lock"
mkdir "$lock"
reject lock-directory PKG_LOCK_INVALID deps restore --project "$manifest"
rmdir "$lock"
ln -s "$work/good.lock" "$lock"
reject lock-symlink PKG_LOCK_INVALID deps restore --project "$manifest"
cmp "$work/good.lock" "$lock"
rm "$lock"
node - "$lock" <<'JS'
require('fs').writeFileSync(process.argv[2],' '.repeat(1048577));
JS
reject lock-size-limit PKG_LOCK_INVALID deps restore --project "$manifest"
test "$(wc -c < "$lock" | tr -d ' ')" = 1048577
printf '{}' > "$lock"
reject lock-records-invalid PKG_LOCK_INVALID deps restore --project "$manifest"
test "$(cat "$lock")" = '{}'
rm "$lock"
set_deps() {
    node - "$manifest" "$1" <<'JS'
const fs=require('fs');const [p,deps]=process.argv.slice(2);const m=JSON.parse(fs.readFileSync(p));m.dependencies=JSON.parse(deps);fs.writeFileSync(p,JSON.stringify(m));
JS
}
set_deps '{"lib":{"path":"../application"}}'
reject application-dependency PKG_MANIFEST_INVALID deps lock --project "$manifest"
set_deps '{"lib":{"path":"../library"},"second":{"path":"../other"}}'
reject duplicate-package-roots PKG_IDENTITY_CONFLICT deps lock --project "$manifest"
node - "$work/other/panack.json" <<'JS'
const fs=require('fs');const p=process.argv[2];const m=JSON.parse(fs.readFileSync(p));m.version='2.0.0';fs.writeFileSync(p,JSON.stringify(m));
JS
reject conflicting-package-versions PKG_VERSION_CONFLICT deps lock --project "$manifest"
set_deps '{"self":{"path":"."}}'
reject project-dependency-cycle PKG_DEPENDENCY_CYCLE deps lock --project "$manifest"
node - "$manifest" <<'JS'
const fs=require('fs');const p=process.argv[2];const m=JSON.parse(fs.readFileSync(p));m.entry='main.panack';m.exports={};fs.writeFileSync(p,JSON.stringify(m));
JS
# An application depending on itself gets the same cycle diagnosis as a library.
reject project-dependency-cycle PKG_DEPENDENCY_CYCLE deps lock --project "$manifest"
cp "$work/app/good.json" "$manifest"
set_deps '{"lib":{"path":"../library"}}'
node - "$work/library/panack.json" <<'JS'
const fs=require('fs');const p=process.argv[2];const m=JSON.parse(fs.readFileSync(p));m.dependencies={self:{path:'.'}};fs.writeFileSync(p,JSON.stringify(m));
JS
reject nested-dependency-cycle PKG_DEPENDENCY_CYCLE deps lock --project "$manifest"
node - "$work/library/panack.json" "$work/app/src" <<'JS'
const fs=require('fs');const [p,target]=process.argv.slice(2);const m=JSON.parse(fs.readFileSync(p));m.dependencies={};fs.writeFileSync(p,JSON.stringify(m));
fs.rmSync(p.replace('panack.json','src'),{recursive:true});fs.symlinkSync(target,p.replace('panack.json','src'));
JS
reject overlapping-source-directories PKG_LAYOUT deps lock --project "$manifest"
test ! -e "$lock"
# Permission errors must not invent a missing file, where the host enforces mode bits.
if [ "$(id -u)" != 0 ]; then
    cp "$work/app/good.json" "$manifest"
    chmod 000 "$manifest"
    reject manifest-permission PKG_FILESYSTEM deps lock --project "$manifest"
    chmod 600 "$manifest"
    ! grep -F 'Missing file:' "$work/err"
    cp "$work/good.lock" "$lock"
    chmod 000 "$lock"
    reject lock-permission PKG_FILESYSTEM deps restore --project "$manifest"
    chmod 600 "$lock"
    cmp "$lock" "$work/good.lock"
    rm "$lock"
    chmod 500 "$work/app"
    reject lock-save-permission PKG_PUBLICATION deps lock --project "$manifest"
    chmod 700 "$work/app"
    test ! -e "$lock"
fi
printf 'package validation diagnostics passed\n'
