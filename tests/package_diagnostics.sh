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
    grep -F '  Phase: ' "$work/err" >/dev/null
    node - "$work/err" "$work" "tests/fixtures/package_diagnostics/$scenario.stderr" <<'JS'
const fs=require('fs'),assert=require('assert');const [file,work,expected]=process.argv.slice(2);
const text=fs.readFileSync(file,'utf8').split(work).join('<workspace>');
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
grep -F 'panack.lock.json records the exact dependencies' "$work/err" >/dev/null
recover
cp "$lock" "$work/good.lock"
cp "$app/.panack_modules/.state/lock.json" "$work/good.state"
mv "$app/.panack_modules/demo.library" "$work/cache"
reject missing-cache PKG_CACHE_MISSING run --project "$manifest"
grep -F 'demo.app -> lib' "$work/err" >/dev/null
recover
cmp "$lock" "$work/good.lock"
printf corrupt > "$app/.panack_modules/demo.library/src/api.panack"
reject corrupt-cache PKG_CACHE_INVALID run --project "$manifest"
grep -F 'It will not use them for this build.' "$work/err" >/dev/null
recover
cmp "$lock" "$work/good.lock"
printf 'pub pure answer(): Nat { 99 }\n' > "$work/library/src/api.panack"
pack
reject changed-bytes PKG_INTEGRITY deps restore --project "$manifest"
grep -F 'Use panack deps update only after you have reviewed and accepted the new contents.' "$work/err" >/dev/null
! grep -E '^  panack deps update' "$work/err"
cmp "$lock" "$work/good.lock"
cmp "$app/.panack_modules/.state/lock.json" "$work/good.state"
cp "$work/original.tar.gz" "$work/library.tar.gz"
expect ok deps restore --project "$manifest"
expect 42 run --project "$manifest"
cmp "$lock" "$work/good.lock"
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
reject selective-update PKG_SELECTIVE deps update --project "$manifest" absent
grep -F 'unknown dependency alias: absent' "$work/err" >/dev/null
cmp "$lock" "$work/good.lock"
node - "$manifest" <<'JS'
const fs=require('fs'),file=process.argv[2],m=JSON.parse(fs.readFileSync(file));m.dependencies={other:{path:'../library'}};fs.writeFileSync(file,JSON.stringify(m));
JS
reject unselected-change PKG_SELECTIVE deps update --project "$manifest" other
grep -F 'but you did not select it' "$work/err" >/dev/null
cmp "$lock" "$work/good.lock"
cp "$manifest" "$work/valid.manifest"
printf '{broken' > "$manifest"
reject malformed-manifest PKG_MANIFEST deps update --project "$manifest"
grep -F 'at byte' "$work/err" >/dev/null
cp "$work/valid.manifest" "$manifest"
printf '{broken' > "$lock"
reject malformed-lock PKG_LOCK_INVALID deps restore --project "$manifest"
grep -F 'version control' "$work/err" >/dev/null
printf '%s\n' 'PASS package explanations, transitive context, exact-pin recovery and commands with quoted paths'
