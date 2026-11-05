#!/bin/sh
set -eu

if [ "$#" -ne 3 ] && [ "$#" -ne 5 ]; then
    echo 'expected compiler, toolchain version, standard library root, and optional VM and CLI paths' >&2
    exit 2
fi
repo_root=$(pwd)
compiler=$1
version=$2
stdlib=$3
case "$compiler" in
    /*) ;;
    *) compiler="$repo_root/$compiler" ;;
esac
case "$stdlib" in
    /*) ;;
    *) stdlib="$repo_root/$stdlib" ;;
esac
panack_vm=${4:-"$repo_root/panack-vm"}
panack=${5:-"$repo_root/bin/panack"}
case "$panack_vm" in
    /*) ;;
    *) panack_vm="$repo_root/$panack_vm" ;;
esac
case "$panack" in
    /*) ;;
    *) panack="$repo_root/$panack" ;;
esac
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-package-relocation.XXXXXX")
trap 'rm -rf "$work"' EXIT
trap 'exit 1' HUP INT TERM

workspace="$work/original/workspace"
relocated="$work/moved/workspace"
outside="$work/unrelated-cwd"
library="$workspace/shared-library"
app_one="$workspace/app-one"
app_two="$workspace/app-two"
bytecode_one="$work/app-one.bc"
bytecode_two="$work/app-two.bc"
mkdir -p "$library/src" "$app_one/src" "$app_two/src" "$outside" "$(dirname "$relocated")"

cat > "$library/panack.json" <<MANIFEST
{"schema":1,"package":"example.shared_library","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","exports":{"api":"api.panack","extra":"extra.panack"},"dependencies":{}}
MANIFEST
printf 'pub pure answer(): Nat { 41 }\n' > "$library/src/implementation.panack"
cat > "$library/src/api.panack" <<'SOURCE'
import project/implementation as implementation
pub pure answer(): Nat { implementation.answer() + 1 }
SOURCE
cat > "$library/src/extra.panack" <<'SOURCE'
import project/api as api
pub pure incremented(): Nat { api.answer() + 1 }
SOURCE

write_app() {
    app_root=$1
    package_name=$2
    cat > "$app_root/panack.json" <<MANIFEST
{"schema":1,"package":"$package_name","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"shared":{"path":"../shared-library"}}}
MANIFEST
}
write_app "$app_one" example.app_one
write_app "$app_two" example.app_two
cat > "$app_one/src/main.panack" <<'SOURCE'
import shared/api as api
main(): Void { print(api.answer()); }
SOURCE
cat > "$app_two/src/main.panack" <<'SOURCE'
import shared/extra as extra
main(): Void { print(extra.incremented()); }
SOURCE

run_panack() (
    cd "$outside"
    PANACKELTY_VERSION="$version" PANACKELTY_STDLIB_PATH="$stdlib" \
        "$panack" "$@"
)

assert_output() {
    label=$1
    expected=$2
    shift 2
    status=0
    run_panack "$@" > "$work/stdout" 2> "$work/stderr" || status=$?
    if [ "$status" -eq 0 ] && [ "$(cat "$work/stdout")" = "$expected" ] && [ ! -s "$work/stderr" ]; then
        printf 'PASS %s from unrelated cwd\n' "$label"
        return
    fi
    printf 'FAIL %s (exit %s; expected %s)\n' "$label" "$status" "$expected" >&2
    cat "$work/stdout" >&2
    cat "$work/stderr" >&2
    return 1
}

assert_bytecode() {
    label=$1
    expected=$2
    bytecode=$3
    status=0
    run_panack "$bytecode" > "$work/stdout" 2> "$work/stderr" || status=$?
    if [ "$status" -eq 0 ] && [ "$(cat "$work/stdout")" = "$expected" ] && [ ! -s "$work/stderr" ]; then
        printf 'PASS %s from unrelated cwd\n' "$label"
        return
    fi
    printf 'FAIL %s (exit %s; expected %s)\n' "$label" "$status" "$expected" >&2
    cat "$work/stdout" >&2
    cat "$work/stderr" >&2
    return 1
}

run_panack deps lock --project "$app_one/panack.json" > "$work/lock-one.out"
run_panack deps lock --project "$app_two/panack.json" > "$work/lock-two.out"

assert_output 'app-one check before relocation' ok check --project "$app_one/panack.json"
assert_output 'app-one source run before relocation' 42 run --project "$app_one/panack.json"
assert_output "app-one compile before relocation" "wrote $bytecode_one" compile --project "$app_one/panack.json" -o "$bytecode_one"
assert_output 'app-two check before relocation' ok check --project "$app_two/panack.json"
assert_output 'app-two source run before relocation' 43 run --project "$app_two/panack.json"
assert_output "app-two compile before relocation" "wrote $bytecode_two" compile --project "$app_two/panack.json" -o "$bytecode_two"

mv "$workspace" "$relocated"
app_one_manifest="$relocated/app-one/panack.json"
app_two_manifest="$relocated/app-two/panack.json"
assert_output 'app-one check after relocation' ok check --project "$app_one_manifest"
assert_output 'app-one source run after relocation' 42 run --project "$app_one_manifest"
assert_bytecode 'app-one saved bytecode after relocation' 42 "$bytecode_one"
assert_output 'app-two check after relocation' ok check --project "$app_two_manifest"
assert_output 'app-two source run after relocation' 43 run --project "$app_two_manifest"
assert_bytecode 'app-two saved bytecode after relocation' 43 "$bytecode_two"
