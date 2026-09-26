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
# The canonical unit path and CI must call the same suite implementations.
for contract in \
    '$(MAKE) --no-print-directory unit-harness' \
    '$(MAKE) --no-print-directory unit-runtime' \
    '$(MAKE) --no-print-directory unit-compiler' \
    'ci-compiler: policy unit-harness unit-compiler' \
    'tests/check.sh $(MAKE) --no-print-directory ci-runtime-phases' \
    'ci-runtime-phases: unit-runtime functional' \
    'ci-bootstrap: bootstrap-check quick-start' \
    'native-conformance/source sh tests/native_conformance.sh source' \
    'native-conformance/bytecode sh tests/native_conformance.sh bytecode'; do
    grep -F "$contract" Makefile >/dev/null || fail "missing shared coverage: $contract"
done
grep -F 'suite: [compiler, runtime, bootstrap, conformance-source, conformance-bytecode]' .github/workflows/check.yml >/dev/null || fail 'missing package partition'
grep -F 'suite: [compiler, runtime, bootstrap, sanitize, coverage]' .github/workflows/check.yml >/dev/null || fail 'missing validation partition'
grep -F 'run: make check-no-interpreter CI_SUITE=${{ matrix.suite }}' .github/workflows/check.yml >/dev/null || fail 'missing isolated suite dispatch'
grep -F 'run: sh tests/profile_command.sh ci/${{ matrix.suite }} make ci-${{ matrix.suite }}' .github/workflows/check.yml >/dev/null || fail 'missing project suite dispatch'
echo 'CI partition dispatch, failure propagation and shared coverage passed.'
