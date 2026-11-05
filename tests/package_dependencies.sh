#!/bin/sh
# Complete local dependency programs through the public CLI, including failures.
set -eu
root=$(pwd -P)
work=$(mktemp -d "$root/build/package-dependencies.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack="$root/bin/panack"
version=$(sed -n '1p' VERSION)
mkdir -p "$work/app/src" "$work/archive/src" "$work/live/src"
cat > "$work/app/panack.json" <<MANIFEST
{"schema":1,"package":"test.app","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"lib":{"archive":"../lib.tar.gz"},"live":{"path":"../live"}}}
MANIFEST
cat > "$work/archive/panack.json" <<MANIFEST
{"schema":1,"package":"test.archive","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","exports":{"api":"api.panack"},"dependencies":{}}
MANIFEST
cat > "$work/live/panack.json" <<MANIFEST
{"schema":1,"package":"test.live","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","exports":{"api":"api.panack"},"dependencies":{}}
MANIFEST
printf '%s\n' 'pub pure answer(): Nat { 42 }' > "$work/archive/src/api.panack"
printf '%s\n' 'pub pure answer(): Nat { 5 }' > "$work/live/src/api.panack"
cat > "$work/app/src/main.panack" <<'SOURCE'
import lib/api as lib
import live/api as live
main(): Void { print(lib.answer() + live.answer()) }
SOURCE
pack() { COPYFILE_DISABLE=1 tar --format ustar -czf "$work/lib.tar.gz" -C "$work/archive" .; }
expect() {
    wanted=$1; shift
    "$panack" "$@" > "$work/out" 2> "$work/err"
    test "$(cat "$work/out")" = "$wanted"
    test ! -s "$work/err"
}
reject() {
    fragment=$1; shift
    status=0
    "$panack" "$@" > "$work/out" 2> "$work/err" || status=$?
    if [ "$status" != 1 ] || ! grep -F "$fragment" "$work/err" >/dev/null; then
        cat "$work/out" "$work/err" >&2
        echo "expected rejection: $fragment (status $status)" >&2
        exit 1
    fi
}
pack
manifest="$work/app/panack.json"
lock="$work/app/panack.lock.json"
cache="$work/app/.panack_modules/test.archive"
reject 'lockfile is missing' check --project "$manifest"
expect ok deps lock --project "$manifest"
expect 47 run --project "$manifest"
cp "$lock" "$work/initial.lock"
cp "$work/app/.panack_modules/.state/lock.json" "$work/initial.state"
expect ok deps lock --project "$manifest"
expect ok check --project "$manifest"
expect "wrote $work/program.bc" compile --project "$manifest" -o "$work/program.bc"
expect 47 "$work/program.bc"
cmp "$lock" "$work/initial.lock"
cmp "$work/app/.panack_modules/.state/lock.json" "$work/initial.state"
printf '%s\n' 'PASS archive lock, check, compile, run and lock no-op'

printf '%s\n' 'damaged source' > "$cache/src/api.panack"
reject 'That checksum does not match the checksum recorded in the lockfile.' run --project "$manifest"
expect ok deps restore --project "$manifest" --offline
expect 47 run --project "$manifest"
cmp "$lock" "$work/initial.lock"
printf '%s\n' 'PASS offline restore repairs tampering without changing lock'

cp "$work/lib.tar.gz" "$work/original.tar.gz"
printf '%s' trailing-garbage >> "$work/lib.tar.gz"
reject 'Cannot use the source archive because its SHA-256 checksum does not match the checksum recorded in the lockfile.' check --project "$manifest"
reject 'gzip' deps restore --project "$manifest" --offline
cmp "$lock" "$work/initial.lock"
cmp "$cache/src/api.panack" "$work/archive/src/api.panack"
cp "$work/original.tar.gz" "$work/lib.tar.gz"

printf '%s\n' 'pub pure answer(): Nat { 43 }' > "$work/archive/src/api.panack"
pack
reject 'PKG_DEPENDENCY_NOT_SELECTED' deps update --project "$manifest" live
cmp "$lock" "$work/initial.lock"
expect ok deps update --project "$manifest" lib
expect 48 run --project "$manifest"
printf '%s\n' 'PASS changed archives require explicit update and selective update preserves other pins'

printf '%s\n' 'pub pure answer(): Nat { 6 }' > "$work/live/src/api.panack"
expect 49 run --project "$manifest"
cp "$lock" "$work/before-live.lock"
sed 's/"version":"1.0.0"/"version":"1.0.1"/' "$work/live/panack.json" > "$work/live/next.json"
mv "$work/live/next.json" "$work/live/panack.json"
reject 'Cannot use the lockfile because a dependency manifest has changed since the lockfile was created.' check --project "$manifest"
grep -F "Manifest: $work/live/panack.json" "$work/err" >/dev/null
grep -F 'Package: test.live@1.0.1' "$work/err" >/dev/null
reject 'PKG_DEPENDENCY_NOT_SELECTED' deps update --project "$manifest" lib
cmp "$lock" "$work/before-live.lock"
expect ok deps update --project "$manifest" live
expect 49 run --project "$manifest"
reject 'this project has no dependency with that name.' deps update --project "$manifest" absent
printf '%s\n' 'PASS live directory edits and stale-manifest selective updates'

mv "$work/app/.panack_modules/.state" "$work/state-backup"
ln -s "$work/state-backup" "$work/app/.panack_modules/.state"
reject 'Cannot prepare the local packages because a required directory path identifies a different kind of file.' deps restore --project "$manifest" --offline
grep -F "Required directory: $work/app/.panack_modules/.state" "$work/err" >/dev/null
cmp "$cache/src/api.panack" "$work/archive/src/api.panack"
rm "$work/app/.panack_modules/.state"
mv "$work/state-backup" "$work/app/.panack_modules/.state"
mv "$cache" "$work/cache-backup"
ln -s "$work/cache-backup" "$cache"
reject 'Cannot prepare the local packages because a required directory path identifies a different kind of file.' check --project "$manifest"
grep -F "Required directory: $cache" "$work/err" >/dev/null
reject 'destination_conflict' deps restore --project "$manifest" --offline
cmp "$work/cache-backup/src/api.panack" "$work/archive/src/api.panack"
rm "$cache"
mv "$work/cache-backup" "$cache"
expect 49 run --project "$manifest"
test -z "$(find "$work/app/.panack_modules" -name '.panack-stage-*' -print)"
printf '%s\n' 'PASS symlinked cache/state rejection preserves old cache and cleans staging'

# Archive-owned relative sources resolve inside the materialized owning package.
mkdir -p "$work/outer/src" "$work/nested-app/src"
cp "$work/lib.tar.gz" "$work/outer/inner.tar.gz"
cat > "$work/outer/panack.json" <<MANIFEST
{"schema":1,"package":"test.outer","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","exports":{"api":"api.panack"},"dependencies":{"inner":{"archive":"inner.tar.gz"}}}
MANIFEST
printf '%s\n' 'import inner/api as inner' 'pub pure answer(): Nat { inner.answer() }' > "$work/outer/src/api.panack"
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/outer.tar.gz" -C "$work/outer" .
cat > "$work/nested-app/panack.json" <<MANIFEST
{"schema":1,"package":"test.nested","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"outer":{"archive":"../outer.tar.gz"}}}
MANIFEST
printf '%s\n' 'import outer/api as outer' 'main(): Void { print(outer.answer()) }' > "$work/nested-app/src/main.panack"
expect ok deps lock --project "$work/nested-app/panack.json"
expect 43 run --project "$work/nested-app/panack.json"
cp "$work/nested-app/panack.lock.json" "$work/nested.lock"
mkdir "$work/relocated"
mv "$work/nested-app" "$work/relocated/nested-app"
cp "$work/outer.tar.gz" "$work/relocated/outer.tar.gz"
expect 43 run --project "$work/relocated/nested-app/panack.json"
expect ok deps restore --project "$work/relocated/nested-app/panack.json" --offline
cmp "$work/nested.lock" "$work/relocated/nested-app/panack.lock.json"
expect 43 run --project "$work/relocated/nested-app/panack.json"
printf '%s\n' 'PASS transitive local archives survive relocation and offline restore'
