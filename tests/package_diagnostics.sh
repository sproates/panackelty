#!/bin/sh
# User-facing explanations and safe recovery through complete public programs.
set -eu
root=$(pwd -P)
work=$(mktemp -d "$root/build/package-diagnostics.XXXXXX")
cleanup() { status=$?; if [ "$status" != 0 ]; then cat "$work/out" "$work/err" 2>/dev/null || :; fi; rm -rf "$work"; }
trap cleanup EXIT
trap 'exit 1' HUP INT TERM
panack="$root/bin/panack"
version=$(cat VERSION)
app="$work/Ian's app"
mkdir -p "$app/src" "$work/library/src" "$work/outer/src"
manifest="$app/panack.json"
lock="$app/panack.lock.json"
node - "$work" "$version" <<'JS'
const fs=require('fs');const [w,version]=process.argv.slice(2);
const m=(name,deps)=>({schema:1,package:name,version:'1.0.0',language:'modules-v1',toolchain:version,source:'src',exports:{api:'api.panack'},dependencies:deps});
fs.writeFileSync(w+'/library/panack.json',JSON.stringify(m('demo.library',{})));
fs.writeFileSync(w+'/outer/panack.json',JSON.stringify(m('demo.outer',{inner:{path:'../missing'}})));
const app=m('demo.app',{lib:{archive:'../library.tar.gz'}});app.entry='main.panack';app.exports={};fs.writeFileSync(w+"/Ian's app/panack.json",JSON.stringify(app));
JS
printf 'pub pure answer(): Nat { 42 }\n' > "$work/library/src/api.panack"
printf 'pub pure answer(): Nat { 1 }\n' > "$work/outer/src/api.panack"
printf '%s\n' 'import lib/api as lib' 'main(): Void { print(lib.answer()) }' > "$app/src/main.panack"
pack() { COPYFILE_DISABLE=1 tar --format ustar -czf "$work/library.tar.gz" -C "$work/library" .; }
pack
cp "$work/library.tar.gz" "$work/original.tar.gz"
expect() { wanted=$1; shift; "$panack" "$@" > "$work/out" 2> "$work/err"; test "$(cat "$work/out")" = "$wanted"; test ! -s "$work/err"; }
reject() {
    scenario=$1; code=$2; shift 2
    status=0
    "$panack" "$@" > "$work/out" 2> "$work/err" || status=$?
    test "$status" = 1
    test ! -s "$work/out"
    grep -F "panack: error[$code]:" "$work/err" >/dev/null
    grep -F '  Project: ' "$work/err" >/dev/null
    grep -F 'panack.json' "$work/err" >/dev/null
    node - "$work/err" "$work" "tests/fixtures/package_diagnostics/$scenario.stderr" <<'JS'
const fs=require('fs'),assert=require('assert');const [file,work,expected]=process.argv.slice(2);
const text=fs.readFileSync(file,'utf8').split(work).join('<workspace>');
if(process.env.PANACK_DIAGNOSTICS_RECORD){fs.mkdirSync(process.env.PANACK_DIAGNOSTICS_RECORD,{recursive:true});fs.writeFileSync(process.env.PANACK_DIAGNOSTICS_RECORD+'/'+require('node:path').basename(expected),text);}
assert.equal(text,fs.readFileSync(expected,'utf8'));
assert(fs.readFileSync('docs/design/PACKAGE_DIAGNOSTIC_EXAMPLES.md','utf8').includes('```text\n'+text+'```'));
JS
    if [ -n "${PANACK_DIAGNOSTICS_RECORD:-}" ]; then
        mkdir -p "$PANACK_DIAGNOSTICS_RECORD"
        node - "$work/err" "$work" "$PANACK_DIAGNOSTICS_RECORD/$scenario.txt" <<'JS'
const fs=require('fs');const [file,work,out]=process.argv.slice(2);
fs.writeFileSync(out,fs.readFileSync(file,'utf8').split(work).join('<workspace>'));
JS
    fi
}
# The suggestion is extracted and executed, proving its shell quoting works.
recover() {
    command=$(sed -n 's/^  \(panack deps .*\)/\1/p' "$work/err")
    test -n "$command"
    PATH="$root/bin:$PATH" sh -c "$command" > "$work/recovery.out" 2> "$work/recovery.err"
    test "$(cat "$work/recovery.out")" = ok
    test ! -s "$work/recovery.err"
    expect 42 run --project "$manifest"
}
reject missing-lock PKG_LOCK_MISSING check --project "$manifest"
grep -F 'The lockfile records the exact dependencies' "$work/err" >/dev/null
recover
cp "$lock" "$work/good.lock"
cp "$app/.panack_modules/.state/lock.json" "$work/good.state"
mv "$app/.panack_modules/demo.library" "$work/cache"
reject missing-cache PKG_CACHE_MISSING run --project "$manifest"
grep -F "dependency 'lib' (package 'demo.library')" "$work/err" >/dev/null
recover
cmp "$lock" "$work/good.lock"
printf corrupt > "$app/.panack_modules/demo.library/src/api.panack"
reject corrupt-cache PKG_CACHE_INVALID run --project "$manifest"
grep -F 'That checksum does not match the checksum recorded in the lockfile.' "$work/err" >/dev/null
recover
cmp "$lock" "$work/good.lock"
printf 'pub pure answer(): Nat { 99 }\n' > "$work/library/src/api.panack"
pack
reject changed-bytes PKG_INTEGRITY deps restore --project "$manifest"
grep -F 'review its contents before running:' "$work/err" >/dev/null
grep -E "^  panack deps update.*'lib'" "$work/err" >/dev/null
cmp "$lock" "$work/good.lock"
cmp "$app/.panack_modules/.state/lock.json" "$work/good.state"
cp "$work/original.tar.gz" "$work/library.tar.gz"
expect ok deps restore --project "$manifest"
expect 42 run --project "$manifest"
cmp "$lock" "$work/good.lock"
# Saved archive packages cannot contain symbolic links. Do not call this a checksum change.
ln -s missing "$app/.panack_modules/demo.library/added-link"
status=0
"$panack" check --project "$manifest" > "$work/out" 2> "$work/err" || status=$?
test "$status" = 1
grep -F 'contains a symbolic link.' "$work/err" >/dev/null
grep -F "Local path: $app/.panack_modules/demo.library/added-link" "$work/err" >/dev/null
! grep -F 'That checksum does not match' "$work/err"
expect ok deps restore --project "$manifest"
test ! -L "$app/.panack_modules/demo.library/added-link"
cmp "$lock" "$work/good.lock"
# A package-tree mismatch must not falsely claim that compressed archive bytes changed.
node - "$lock" <<'JS'
const fs=require('fs'),file=process.argv[2],lock=JSON.parse(fs.readFileSync(file));lock.packages[0].tree_sha256='0'.repeat(64);fs.writeFileSync(file,JSON.stringify(lock));
JS
cp "$lock" "$work/wrong-tree.lock"
status=0
"$panack" deps restore --project "$manifest" > "$work/out" 2> "$work/err" || status=$?
test "$status" = 1
grep -F "The archive's checksum matched, but the extracted files failed the separate package checksum check." "$work/err" >/dev/null
grep -F 'Expected package checksum: 00000000' "$work/err" >/dev/null
cmp "$lock" "$work/wrong-tree.lock"
cmp "$app/.panack_modules/.state/lock.json" "$work/good.state"
cp "$work/good.lock" "$lock"
expect 42 run --project "$manifest"
node - "$manifest" <<'JS'
const fs=require('fs'),file=process.argv[2],m=JSON.parse(fs.readFileSync(file));m.dependencies={outer:{path:'../outer'}};fs.writeFileSync(file,JSON.stringify(m));
JS
reject transitive-manifest PKG_MANIFEST_MISSING deps update --project "$manifest"
grep -F 'demo.app -> outer' "$work/err" >/dev/null
grep -F 'demo.outer -> inner' "$work/err" >/dev/null
grep -F '/outer/panack.json' "$work/err" >/dev/null
node - "$manifest" <<'JS'
const fs=require('fs'),file=process.argv[2],m=JSON.parse(fs.readFileSync(file));m.dependencies={};fs.writeFileSync(file,JSON.stringify(m));
JS
reject selective-update PKG_DEPENDENCY_UNKNOWN deps update --project "$manifest" absent
grep -F 'this project has no dependency with that name.' "$work/err" >/dev/null
cmp "$lock" "$work/good.lock"
node - "$manifest" <<'JS'
const fs=require('fs'),file=process.argv[2],m=JSON.parse(fs.readFileSync(file));m.dependencies={other:{path:'../library'}};fs.writeFileSync(file,JSON.stringify(m));
JS
reject unselected-change PKG_DEPENDENCY_NOT_SELECTED deps update --project "$manifest" other
grep -F 'Your command did not select this dependency.' "$work/err" >/dev/null
cmp "$lock" "$work/good.lock"
cp "$manifest" "$work/valid.manifest"
printf '\377' > "$manifest"
reject invalid-utf8-manifest PKG_MANIFEST_INVALID deps update --project "$manifest"
cmp "$lock" "$work/good.lock"
printf '{broken' > "$manifest"
reject malformed-manifest PKG_MANIFEST_INVALID deps update --project "$manifest"
grep -F 'Location: line 1, column 2' "$work/err" >/dev/null
cp "$work/valid.manifest" "$manifest"
printf '{broken' > "$lock"
reject malformed-lock PKG_LOCK_INVALID deps restore --project "$manifest"
grep -F 'obtain a valid copy' "$work/err" >/dev/null
printf '%s\n' 'PASS package explanations, transitive context, exact-pin recovery and commands with quoted paths'
