#!/bin/sh
# Fixed independent expectations replace the former live Python differential
# oracle. Run from the repository root; selected binaries retain instrumentation.
set -eu

vm=${PANACK_NATIVE_BINARY:-./panack-vm}
modules=${PANACK_NATIVE_MODULE_TEST:-./build/vm/test_modules}
seed=${SEED_COMPILER:-bootstrap/compiler-v8.bc}
fixtures=tests/fixtures/oracle_contracts
mode=${1:-all}
case "$mode" in all|artifacts) ;; *) echo "unknown oracle contract group" >&2; exit 1 ;; esac
temporary=$(mktemp -d "${TMPDIR:-/tmp}/panack-oracles.XXXXXX")
phase=initialization
cleanup() {
    status=$?
    trap - 0
    if [ "$status" -ne 0 ]; then
        echo "oracle contracts failed during $phase" >&2
        if [ -s "$temporary/errors" ]; then cat "$temporary/errors" >&2; fi
    fi
    rm -rf "$temporary"
    exit "$status"
}
trap cleanup 0
trap 'exit 1' HUP INT TERM

fail() { echo "oracle contracts: $*" >&2; exit 1; }

if [ "$mode" = all ]; then
    phase="arithmetic and registry"
    rows() { test "$(wc -l < "$fixtures/$1")" -eq "$2" || fail "changed case count: $1"; }
    rows integer.stdin 1800
    rows integer.stdout 1800
    rows decimal.stdin 422
    rows decimal.stdout 422
    rows rational.stdout 96
    rows builtin.names 82
    rows builtin.stdout 82

    bounded() {
        sh tests/profile_command.sh "oracle/command/$2" "$vm" run "$seed" run tests/runner/oracle_command.panack "$@"
    }
    bounded 20 "$fixtures/integer.stdin" "$temporary/integer" "$modules" arithmetic
    cmp "$fixtures/integer.stdout" "$temporary/integer"
    bounded 20 "$fixtures/decimal.stdin" "$temporary/decimal" "$modules" arithmetic
    awk -f tests/canonical_decimal.awk "$temporary/decimal" > "$temporary/canonical"
    cmp "$fixtures/decimal.stdout" "$temporary/canonical"

    set --
    while IFS= read -r name; do set -- "$@" "$name"; done < "$fixtures/builtin.names"
    bounded 10 /dev/null "$temporary/registry" "$modules" registry "$@"
    cmp "$fixtures/builtin.stdout" "$temporary/registry"
fi

compile() {
    phase="compiling $1"
    sh tests/profile_command.sh "oracle/compile/$1" "$vm" run "$seed" compile "$1" -o "$temporary/program.bc" > "$temporary/compile" 2> "$temporary/errors"
    test ! -s "$temporary/errors" || fail "compile stderr: $1"
    printf 'wrote %s\n' "$temporary/program.bc" > "$temporary/expected-compile"
    cmp "$temporary/expected-compile" "$temporary/compile"
    "$vm" check "$temporary/program.bc" > "$temporary/check" 2> "$temporary/errors"
    test ! -s "$temporary/errors" || fail "artifact verification stderr: $1"
    printf 'ok\n' > "$temporary/ok"
    cmp "$temporary/ok" "$temporary/check"
}

check_golden() {
    od -An -v -tx1 "$temporary/program.bc" | tr -d ' \n' > "$temporary/actual.hex"
    printf '\n' >> "$temporary/actual.hex"
    cmp "$fixtures/$1.hex" "$temporary/actual.hex"
}

if [ "$mode" = all ]; then
    compile "$fixtures/rational.panack"
    "$vm" run "$temporary/program.bc" > "$temporary/rational" 2> "$temporary/errors"
    test ! -s "$temporary/errors" || fail "rational stderr"
    cmp "$fixtures/rational.stdout" "$temporary/rational"
fi

for entry in basic relative/main logical/main; do
    compile "tests/fixtures/compiler_contracts/driver/$entry.panack"
    case "$entry" in
        basic) golden=driver-basic ;;
        relative/main) golden=driver-relative ;;
        logical/main) golden=driver-logical ;;
    esac
    check_golden "$golden"
done
compile tests/functional/cases/stdlib/main.panack
check_golden stdlib
compile examples/euler001.panack
check_golden euler001
"$vm" check "$seed" > "$temporary/check" 2> "$temporary/errors"
test ! -s "$temporary/errors" || fail "compiler seed verification stderr"
cmp "$temporary/ok" "$temporary/check"
"$vm" run "$seed" check examples/euler001.panack > "$temporary/check" 2> "$temporary/errors"
test ! -s "$temporary/errors" || fail "compiler source check stderr"
cmp "$temporary/ok" "$temporary/check"
"$vm" run "$seed" run examples/euler001.panack > "$temporary/output" 2> "$temporary/errors"
test ! -s "$temporary/errors" || fail "compiler source run stderr"
cmp tests/functional/expected/examples/euler001.stdout "$temporary/output"

if [ "$mode" = artifacts ]; then
    echo "native oracle artifact contracts: ok"
    exit 0
fi

# The original instrumented oracle ran every functional main and example on the
# selected VM. Preserve that corpus here, including meta-runner programs.
for source in tests/functional/cases/*/main.panack examples/*.panack; do
    compile "$source"
    case "$source" in
        examples/*) name=${source##*/}; expected="tests/functional/expected/examples/${name%.panack}.stdout" ;;
        *) expected="${source%/*}/expected.stdout" ;;
    esac
    phase="running $source"
    sh tests/profile_command.sh "oracle/run/$source" "$vm" run "$temporary/program.bc" > "$temporary/output" 2> "$temporary/errors"
    test ! -s "$temporary/errors" || fail "execution stderr: $source"
    cmp "$expected" "$temporary/output"
done
echo "native oracle contracts: ok"
