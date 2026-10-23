#!/bin/sh
set -eu
root=$(pwd -P)
work=$(mktemp -d "$root/build/package-https.XXXXXX")
pid=
cleanup() { code=$?; if [ "$code" -ne 0 ]; then cat "$work/out" "$work/err" "$work/server.log" "$work/large.time" 2>/dev/null || :; fi; if [ -n "$pid" ]; then kill "$pid" 2>/dev/null || :; wait "$pid" 2>/dev/null || :; fi; rm -rf "$work"; }
trap cleanup EXIT
trap 'exit 1' HUP INT TERM
panack="$root/bin/panack"
version=$(sed -n '1p' VERSION)
mkdir -p "$work/app/src" "$work/archive/src" "$work/other/src"
cat > "$work/request.cnf" <<'CONF'
[req]
distinguished_name=subject
[subject]
[ca]
basicConstraints=critical,CA:TRUE
keyUsage=critical,keyCertSign,cRLSign
subjectKeyIdentifier=hash
CONF
openssl req -config "$work/request.cnf" -extensions ca -x509 -newkey rsa:2048 -nodes -keyout "$work/ca.key" -out "$work/ca.crt" -sha256 -days 2 -subj '/CN=Panackelty archive test CA' > "$work/cert.log" 2>&1
openssl req -config "$work/request.cnf" -new -newkey rsa:2048 -nodes -keyout "$work/test.key" -out "$work/test.csr" -subj '/CN=localhost' >> "$work/cert.log" 2>&1
cat > "$work/server.ext" <<'CONF'
subjectAltName=DNS:localhost
basicConstraints=critical,CA:FALSE
keyUsage=critical,digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
CONF
openssl x509 -req -in "$work/test.csr" -CA "$work/ca.crt" -CAkey "$work/ca.key" -set_serial 2 -sha256 -days 2 -extfile "$work/server.ext" -out "$work/test.crt" >> "$work/cert.log" 2>&1
export CURL_CA_BUNDLE="$work/ca.crt"
node tests/package_https_fixture.cjs "$work" > "$work/server.log" "$work/large.time" 2>&1 &
pid=$!
for attempt in 1 2 3 4 5 6 7 8 9 10; do [ -s "$work/port" ] && break; sleep 0.1; done
port=$(cat "$work/port")
manifest="$work/app/panack.json"
lock="$work/app/panack.lock.json"
cache="$work/app/.panack_modules/test.archive"
manifest_for() {
cat > "$manifest" <<JSON
{"schema":1,"package":"test.app","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"lib":{"archive":"https://localhost:$port/$1.tar.gz"}}}
JSON
}
cat > "$work/archive/panack.json" <<JSON
{"schema":1,"package":"test.archive","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","exports":{"api":"api.panack"},"dependencies":{}}
JSON
printf '%s\n' 'pub pure answer(): Nat { 42 }' > "$work/archive/src/api.panack"
printf '%s\n' 'import lib/api as lib' 'main(): Void { print(lib.answer()) }' > "$work/app/src/main.panack"
pack() { COPYFILE_DISABLE=1 tar --format ustar -czf "$work/lib.tar.gz" -C "$work/archive" .; }
connections() { if [ -f "$work/connections" ]; then wc -l < "$work/connections" | tr -d ' '; else printf 0; fi; }
network_free() { case "$*" in "check "*|"compile "*|"run "*|*--offline|*.bc) return 0 ;; *) return 1 ;; esac; }
expect() { wanted=$1; shift; connection_before=$(connections); "$panack" "$@" > "$work/out" 2> "$work/err"; test "$(cat "$work/out")" = "$wanted"; test ! -s "$work/err"; if network_free "$@"; then test "$(connections)" = "$connection_before"; fi; }
reject() { connection_before=$(connections); status=0; "$panack" "$@" > "$work/out" 2> "$work/err" || status=$?; test "$status" = 1; test -s "$work/err"; if network_free "$@"; then test "$(connections)" = "$connection_before"; fi; }
requests() { wc -l < "$work/requests" | tr -d ' '; }
pack
manifest_for redirect
expect ok deps lock --project "$manifest"
test "$(requests)" = 2
node - "$lock" "$work/lib.tar.gz" <<'JS'
const fs=require('fs'),crypto=require('crypto'),assert=require('assert');
const lock=JSON.parse(fs.readFileSync(process.argv[2]));
assert.equal(lock.root.dependencies.lib.source.archive_sha256,crypto.createHash('sha256').update(fs.readFileSync(process.argv[3])).digest('hex'));
assert(lock.root.dependencies.lib.source.url.endsWith('/redirect.tar.gz'));
JS
cp "$lock" "$work/pinned.lock"
cp "$work/app/.panack_modules/.state/lock.json" "$work/pinned.state"
expect ok deps lock --project "$manifest"
expect ok check --project "$manifest"
expect "wrote $work/program.bc" compile --project "$manifest" -o "$work/program.bc"
expect 42 run --project "$manifest"
expect 42 "$work/program.bc"
expect ok deps restore --project "$manifest" --offline
test "$(requests)" = 2
cmp "$lock" "$work/pinned.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/pinned.state"
printf '%s\n' 'PASS HTTPS redirect, independent compressed digest, source/bytecode and network-free warm commands'
printf corrupt > "$cache/src/api.panack"
reject deps restore --project "$manifest" --offline
reject run --project "$manifest"
test "$(requests)" = 2
expect ok deps restore --project "$manifest"
test "$(requests)" = 4
expect 42 run --project "$manifest"
# Cold offline restore must not access the peer either.
mv "$work/app/.panack_modules" "$work/saved-cache"
reject deps restore --project "$manifest" --offline
test "$(requests)" = 4
expect ok deps restore --project "$manifest"
test "$(requests)" = 6
printf '%s\n' 'pub pure answer(): Nat { 43 }' > "$work/archive/src/api.panack"
pack
printf corrupt > "$cache/src/api.panack"
reject deps restore --project "$manifest"
cmp "$lock" "$work/pinned.lock"
expect ok deps update --project "$manifest"
expect 43 run --project "$manifest"
printf '%s\n' 'PASS cold/corrupt offline refusal, pinned online repair and explicit changed-byte update'
manifest_for chunked
expect ok deps update --project "$manifest"
expect 43 run --project "$manifest"
cp "$lock" "$work/good.lock"
cp "$cache/src/api.panack" "$work/good-source"
for endpoint in loop downgrade secret query missing oversize truncated encoded; do
    manifest_for "$endpoint"
    reject deps update --project "$manifest"
    cmp "$lock" "$work/good.lock"
    cmp "$cache/src/api.panack" "$work/good-source"
    ! grep -Eq "password|secret=token" "$work/err"
    test -z "$(find "$work/app/.panack_modules" -maxdepth 1 -name '.panack-stage-*' -print)"
done
! grep -q forbidden "$work/requests"
! grep -q password "$work/err"
printf '%s\n' 'PASS chunked framing, redirect policy, status/size/truncation/encoding failures and lock preservation'
# A second root dependency retains its pin during a selective update, even when
# its remote bytes change. This assertion observes requests as well as lock data.
node - "$work/archive/panack.json" "$work/other/panack.json" <<'JS'
const fs=require('fs');const m=JSON.parse(fs.readFileSync(process.argv[2]));m.package='test.other';fs.writeFileSync(process.argv[3],JSON.stringify(m));
JS
printf '%s\n' 'pub pure answer(): Nat { 10 }' > "$work/other/src/api.panack"
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/other.tar.gz" -C "$work/other" .
manifest_for lib
node - "$manifest" "$port" <<'JS'
const fs=require('fs');const p=process.argv[2],m=JSON.parse(fs.readFileSync(p));m.dependencies.other={archive:`https://localhost:${process.argv[3]}/other.tar.gz`};fs.writeFileSync(p,JSON.stringify(m));
JS
expect ok deps update --project "$manifest"
cp "$lock" "$work/two.lock"
printf '%s\n' 'pub pure answer(): Nat { 11 }' > "$work/other/src/api.panack"
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/other.tar.gz" -C "$work/other" .
printf '%s\n' 'pub pure answer(): Nat { 44 }' > "$work/archive/src/api.panack"
pack
before=$(requests)
expect ok deps update --project "$manifest" lib
test "$(requests)" = "$((before + 1))"
node - "$lock" "$work/two.lock" <<'JS'
const fs=require('fs'),assert=require('assert');const a=JSON.parse(fs.readFileSync(process.argv[2])),b=JSON.parse(fs.readFileSync(process.argv[3]));assert.deepEqual(a.root.dependencies.other,b.root.dependencies.other);assert.deepEqual(a.packages.find(p=>p.name==='test.other'),b.packages.find(p=>p.name==='test.other'));
JS
expect 44 run --project "$manifest"
# Move a restored project and verify state and source identity remain portable.
mv "$work/app" "$work/moved"
expect 44 run --project "$work/moved/panack.json"
expect ok deps restore --project "$work/moved/panack.json" --offline
test "$(requests)" = "$((before + 1))"
mv "$work/moved" "$work/app"
printf '%s\n' 'PASS selective update preserves unselected pins without fetching and cache survives relocation'
# A remotely supplied manifest cannot escape its own verified archive to read
# a local host path, even during an explicit update.
node - "$work/archive/panack.json" <<'JS'
const fs=require('fs');const p=process.argv[2],m=JSON.parse(fs.readFileSync(p));m.dependencies.escape={path:'/etc'};fs.writeFileSync(p,JSON.stringify(m));
JS
pack
cp "$lock" "$work/before-escape.lock"
reject deps update --project "$manifest" lib
grep -q 'escapes its archive' "$work/err"
cmp "$lock" "$work/before-escape.lock"
# Replace that declaration with a transitive HTTPS archive dependency.
node - "$work/archive/panack.json" "$port" <<'JS'
const fs=require('fs');const p=process.argv[2],m=JSON.parse(fs.readFileSync(p));m.dependencies={nested:{archive:`https://localhost:${process.argv[3]}/other.tar.gz`}};fs.writeFileSync(p,JSON.stringify(m));
JS
printf '%s\n' 'import nested/api as nested' 'pub pure answer(): Nat { nested.answer() + 40 }' > "$work/archive/src/api.panack"
pack
manifest_for lib
expect ok deps update --project "$manifest"
expect 51 run --project "$manifest"
before=$(requests)
expect ok deps restore --project "$manifest" --offline
expect 51 run --project "$manifest"
test "$(requests)" = "$before"
printf '%s\n' 'PASS remote manifest containment and transitive HTTPS archive workflow'
# Trust failures and forbidden URL material must fail without exposing secrets.
before=$(requests)
manifest_for lib
reject_env_status=0
env -u CURL_CA_BUNDLE "$panack" deps update --project "$manifest" > "$work/out" 2> "$work/err" || reject_env_status=$?
test "$reject_env_status" = 1
test "$(requests)" = "$before"
for source in 'http://localhost/a.tar.gz' 'https://user:password@localhost/a.tar.gz' 'https://localhost/a.tar.gz?secret=token' 'https://localhost/a.tar.gz#fragment'; do
    node - "$manifest" "$source" <<'JS'
const fs=require('fs');const p=process.argv[2],m=JSON.parse(fs.readFileSync(p));m.dependencies.lib.archive=process.argv[3];fs.writeFileSync(p,JSON.stringify(m));
JS
    reject deps update --project "$manifest"
    ! grep -Eq 'password|secret=token' "$work/err"
    test "$(requests)" = "$before"
done
printf '%s\n' 'PASS TLS trust and URL policy reject before HTTP requests without leaking secret values'
# Exceed the old 1 MiB HTTP-client limit with incompressible archive contents.
# The independent digest assertion above checks the representation being pinned.
manifest_for lib
node - "$work/archive/payload.bin" <<'JS'
require('node:fs').writeFileSync(process.argv[2],require('node:crypto').randomBytes(2*1024*1024));
JS
pack
if [ "${PANACK_PACKAGE_MEASURE:-0}" = 1 ]; then
    if [ "$(uname -s)" = Darwin ]; then time_flag=-l; else time_flag=-v; fi
else time_flag=-p; fi
/usr/bin/time "$time_flag" "$panack" deps update --project "$manifest" > "$work/out" 2> "$work/large.time"
test "$(cat "$work/out")" = ok
expect 51 run --project "$manifest"
before=$(requests)
/usr/bin/time "$time_flag" "$panack" deps restore --project "$manifest" --offline > "$work/out" 2> "$work/warm.time"
test "$(cat "$work/out")" = ok
test "$(requests)" = "$before"
printf '%s\n' 'PASS archive larger than the old buffered HTTP cap; cold update measurements:'
cat "$work/large.time"
printf '%s\n' 'Warm offline verification measurements:'
cat "$work/warm.time"
# Confinement must survive extraction of a local archive carried inside a remote
# archive. The inner package receives a different staging root.
mkdir -p "$work/inner/src"
node - "$work/archive/panack.json" "$work/inner/panack.json" <<'JS'
const fs=require('fs');const p=process.argv[2],outer=JSON.parse(fs.readFileSync(p));const inner={...outer,package:'test.inner',dependencies:{escape:{path:'/etc'}}};fs.writeFileSync(process.argv[3],JSON.stringify(inner));outer.dependencies={nested:{archive:'inner.tar.gz'}};fs.writeFileSync(p,JSON.stringify(outer));
JS
printf '%s\n' 'pub pure answer(): Nat { 9 }' > "$work/inner/src/api.panack"
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/archive/inner.tar.gz" -C "$work/inner" .
pack
cp "$lock" "$work/before-nested.lock"
reject deps update --project "$manifest"
grep -q 'escapes its archive' "$work/err"
cmp "$lock" "$work/before-nested.lock"
test -z "$(find "$work/app/.panack_modules" -maxdepth 1 -name '.panack-stage-*' -print)"
node - "$work/inner/panack.json" <<'JS'
const fs=require('fs');const p=process.argv[2],m=JSON.parse(fs.readFileSync(p));m.dependencies={};fs.writeFileSync(p,JSON.stringify(m));
JS
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/archive/inner.tar.gz" -C "$work/inner" .
pack
expect ok deps update --project "$manifest"
expect 49 run --project "$manifest"
before=$(requests)
expect ok deps restore --project "$manifest" --offline
expect 49 run --project "$manifest"
test "$(requests)" = "$before"
printf '%s\n' 'PASS nested local archives retain remote confinement and valid nested packages restore offline'
