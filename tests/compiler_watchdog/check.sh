#!/bin/sh
set -eu
root=$(pwd -P)
watchdog=${PANACK_COMPILER_WATCHDOG_BINARY:-$root/build/vm/compiler-watchdog}
observer=${PANACK_COMPILER_WATCHDOG_VM:-$root/build/vm/compiler-observer-vm}
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-watchdog-contracts.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
PANACKELTY_BOOTSTRAP_ROOT=$root
PANACKELTY_STDLIB_PATH=$root/src/stdlib
export PANACKELTY_BOOTSTRAP_ROOT PANACKELTY_STDLIB_PATH
"$watchdog" --self-test
expect() {
    wanted=$1
    shift
    status=0
    "$@" > "$work/out" 2> "$work/err" || status=$?
    if [ "$status" != "$wanted" ]; then
        cat "$work/out" "$work/err" >&2
        echo "watchdog contract expected status $wanted, got $status" >&2
        exit 1
    fi
}
mkdir "$work/healthy"
expect 0 "$watchdog" 0.5 "$work/healthy" "$watchdog" --test-child healthy
for mode in heartbeats silent partial missing; do
    mkdir "$work/$mode"
    expect 124 "$watchdog" 0.5 "$work/$mode" "$watchdog" --test-child "$mode"
    case "$mode" in
        heartbeats) grep -q 'stuck.panack::loop' "$work/err" ;;
        silent) grep -q 'startup or between stages' "$work/err"
            grep -q 'child_status=137' "$work/$mode"/run.*/result.txt ;;
        partial) grep -q 'partial progress record' "$work/err" ;;
        missing) grep -q 'without complete progress' "$work/err" ;;
    esac
done
echo 'PASS compiler watchdog process supervision'
mkdir "$work/launch-failure"
expect 125 "$watchdog" 2 "$work/launch-failure" /missing/panack-observer
grep -q 'execute observed compiler' "$work/err"
grep -q 'Evidence:' "$work/err"
source=tests/functional/cases/hello_world/main.panack
for phase in check_module_signatures check_module_bodies_explained bind_body_block compile_program serialize_bytecode; do
    mkdir "$work/$phase"
    if [ "$phase" = bind_body_block ]; then
        expect 124 env PANACK_COMPILER_STALL="$phase" PANACK_COMPILER_STALL_FUNCTION=main \
            "$watchdog" 2 "$work/$phase" "$observer" run bootstrap/compiler-v9.bc run "$source"
        grep -q 'main.panack::main' "$work/err"
    else
        expect 124 env PANACK_COMPILER_STALL="$phase" \
            "$watchdog" 2 "$work/$phase" "$observer" run bootstrap/compiler-v9.bc run "$source"
        grep -q "$phase" "$work/err"
    fi
    case "$phase" in check_module*) grep -q 'Module: .*\.panack' "$work/err" ;; esac
    grep -q 'Compiler evidence:' "$work/err"
done
mkdir "$work/recovery"
expect 0 "$watchdog" 90 "$work/recovery" "$observer" run bootstrap/compiler-v9.bc run "$source"
cmp tests/functional/cases/hello_world/expected.stdout "$work/out"
test ! -s "$work/err"
echo 'PASS compiler watchdog real phase stalls and recovery'
