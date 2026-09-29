#!/bin/sh
# Exercise isolated dispatch without running the compiler; real suites run in CI.
set -eu
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-ci-partition.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
mkdir -p "$work/bin" "$work/tree/tests"
cp tests/without_interpreter.sh tests/profile_command.sh "$work/tree/tests/"
cat > "$work/bin/make" <<'STUB'
#!/bin/sh
printf '%s\n' "$*" >> "$CI_TEST_CALLS"
if [ "$*" = "${CI_TEST_FAIL:-}" ]; then exit 7; fi
STUB
chmod +x "$work/bin/make"
export CI_TEST_CALLS="$work/calls"
unset VALIDATION_PROFILE_FILE VALIDATION_TIMINGS_FILE
fail() { echo "CI partition: $*" >&2; exit 1; }
run() {
    : > "$CI_TEST_CALLS"
    result=0
    (cd "$work/tree"; PATH="$work/bin:$PATH" sh tests/without_interpreter.sh "$@") > "$work/output" 2>&1 || result=$?
}
for suite in compiler runtime bootstrap conformance-source conformance-bytecode; do
    run "$suite"
    [ "$result" = 0 ] || fail "dispatch failed: $suite"
    printf 'clean\nci-%s\n' "$suite" > "$work/expected"
    cmp "$work/expected" "$CI_TEST_CALLS" || fail "wrong suite: $suite"
    CI_TEST_FAIL=ci-$suite; export CI_TEST_FAIL
    run "$suite"
    [ "$result" = 7 ] || fail "lost failure: $suite"
    unset CI_TEST_FAIL
done
run
[ "$result" = 0 ] || fail 'default dispatch failed'
printf 'clean\ncheck\npackage\n' > "$work/expected"
cmp "$work/expected" "$CI_TEST_CALLS" || fail 'default no longer validates everything'
CI_TEST_FAIL=check; export CI_TEST_FAIL
run
[ "$result" = 7 ] || fail 'lost full-check failure'
printf 'clean\ncheck\n' > "$work/expected"
cmp "$work/expected" "$CI_TEST_CALLS" || fail 'packaged after failed check'
unset CI_TEST_FAIL
for phase in clean package; do
    CI_TEST_FAIL=$phase; export CI_TEST_FAIL
    run
    [ "$result" = 7 ] || fail "lost $phase failure"
    if [ "$phase" = clean ]; then
        printf 'clean\n' > "$work/expected"
        cmp "$work/expected" "$CI_TEST_CALLS" || fail 'validated after failed cleanup'
    fi
done
unset CI_TEST_FAIL
run unknown
[ "$result" = 2 ] && [ ! -s "$CI_TEST_CALLS" ] || fail 'invalid suite performed work'
run compiler runtime
[ "$result" = 2 ] && [ ! -s "$CI_TEST_CALLS" ] || fail 'extra suite was ignored'
cd "$root"
# Exercise the actual unit recipe: preserve all suites, bounded workers,
# successful-pair-before-runtime ordering and failure propagation.
mkdir "$work/unit"
cat > "$work/unit/make" <<'UNIT_MAKE'
#!/bin/sh
set -eu
group=$2
printf '%s\n' "$group" >> "$UNIT_CALLS"
: > "$UNIT_CONTROL/$group.started"
await_peer() {
    attempts=0
    while [ ! -f "$UNIT_CONTROL/$1.started" ]; do
        attempts=$((attempts + 1))
        [ "$attempts" -lt 50 ] || exit 9
        sleep 0.1
    done
}
case "$group" in
    unit-harness)
        [ "$VALIDATION_JOBS" = 1 ] || exit 9
        if [ "$UNIT_WORKERS" -gt 1 ]; then await_peer unit-compiler; fi ;;
    unit-compiler)
        expected=1
        if [ "$UNIT_WORKERS" -gt 1 ]; then expected=$((UNIT_WORKERS - 1)); fi
        [ "$VALIDATION_JOBS" = "$expected" ] || exit 9
        if [ "$UNIT_WORKERS" -gt 1 ]; then
            await_peer unit-harness
        fi ;;
    unit-runtime)
        [ "$VALIDATION_JOBS" = "$UNIT_WORKERS" ] || exit 9
        test -f "$UNIT_CONTROL/unit-harness.done"
        test -f "$UNIT_CONTROL/unit-compiler.done" ;;
    *) exit 9 ;;
esac
if [ "$UNIT_FAIL" = "$group" ]; then exit 7; fi
: > "$UNIT_CONTROL/$group.done"
UNIT_MAKE
chmod +x "$work/unit/make"
export UNIT_CONTROL="$work/unit" UNIT_CALLS="$work/unit/calls"
for workers in 1 2 3; do
    for fault in none unit-harness unit-compiler unit-runtime; do
        rm -f "$work/unit/"*.done "$work/unit/"*.started
        : > "$UNIT_CALLS"
        UNIT_WORKERS=$workers UNIT_FAIL=$fault
        export UNIT_WORKERS UNIT_FAIL
        result=0
        (unset MAKEFLAGS MFLAGS MAKELEVEL
         make --no-print-directory -f "$root/Makefile" unit-impl \
             MAKE="$work/unit/make" VALIDATION_JOBS="$workers") > "$work/unit/output" 2>&1 || result=$?
        if [ "$fault" = none ]; then
            [ "$result" = 0 ] || { cat "$work/unit/output"; fail 'unit recipe rejected success'; }
        else
            [ "$result" != 0 ] || fail "unit recipe lost failure: $fault"
        fi
        printf 'unit-harness\n' > "$work/unit/expected"
        if [ "$workers" -gt 1 ] || [ "$fault" != unit-harness ]; then
            printf 'unit-compiler\n' >> "$work/unit/expected"
        fi
        if [ "$fault" = none ] || [ "$fault" = unit-runtime ]; then
            printf 'unit-runtime\n' >> "$work/unit/expected"
            [ "$(tail -n 1 "$UNIT_CALLS")" = unit-runtime ] || fail 'runtime did not follow successful pair'
        fi
        sort "$work/unit/expected" > "$work/unit/expected.sorted"
        sort "$UNIT_CALLS" > "$work/unit/actual.sorted"
        cmp "$work/unit/expected.sorted" "$work/unit/actual.sorted" || fail "unit coverage changed: $workers/$fault"
    done
done
unset UNIT_CONTROL UNIT_CALLS UNIT_WORKERS UNIT_FAIL

# The canonical unit path and CI must call the same suite implementations.
for contract in \
    '$(MAKE) --no-print-directory unit-runtime' \
    'ci-compiler: policy native' \
    'sh tests/run_suites.sh $(MAKE) unit-harness unit-compiler' \
    'sh tests/run_suites.sh $(MAKE) unit-runtime-native unit-runtime-probes' \
    'tests/check.sh $(MAKE) --no-print-directory ci-runtime-phases' \
    'ci-runtime-phases: unit-runtime functional' \
    'ci-bootstrap: bootstrap-check quick-start' \
    'sh tests/run_suites.sh $(MAKE) bootstrap-fixed-point bootstrap-seed-refresh' \
    'bootstrap-fixed-point: $(STAGE3_COMPILER) $(STAGE1_STDLIB) $(STAGE2_STDLIB) $(STAGE3_STDLIB)' \
    'seed-refresh/native-staging sh tests/seed_refresh.sh --native' \
    'native-conformance/source sh tests/native_conformance.sh source' \
    'native-conformance/bytecode sh tests/native_conformance.sh bytecode'; do
    grep -F "$contract" Makefile >/dev/null || fail "missing shared coverage: $contract"
done
grep -F 'suite: [compiler, runtime, bootstrap, conformance-source, conformance-bytecode]' .github/workflows/check.yml >/dev/null || fail 'missing package partition'
grep -F 'suite: [compiler, runtime, bootstrap, sanitize-vm, sanitize-oracle, sanitize-runner, coverage]' .github/workflows/check.yml >/dev/null || fail 'missing validation partition'
grep -F 'run: make check-no-interpreter CI_SUITE=${{ matrix.suite }}' .github/workflows/check.yml >/dev/null || fail 'missing isolated suite dispatch'
grep -F 'run: sh tests/profile_command.sh ci/${{ matrix.suite }} make ci-${{ matrix.suite }}' .github/workflows/check.yml >/dev/null || fail 'missing project suite dispatch'
echo 'CI partition dispatch, failure propagation and shared coverage passed.'
