#!/bin/sh
# Exercise the real dispatcher and cache with tiny assertion-bearing suites.
. tests/unit/harness/common.sh
checkout=$work/'shared compiler checkout'
mkdir -p "$checkout/tests/runner/compiler_suites" "$checkout/src" "$checkout/bootstrap"
cp tests/runner/compiler_shared_unit.panack "$checkout/tests/runner/"
cp tests/run_probe.sh tests/run_probes.sh tests/probe_dependencies.awk tests/profile_command.sh "$checkout/tests/"
cp -R src/stdlib "$checkout/src/"
cp bootstrap/compiler-v9.bc "$checkout/bootstrap/"
vm=$root/panack-vm
names='contracts checker module_bindings loader_confinement package_source_limits instruction_sources source_maps source_inventory coverage'
for name in $names; do
    cat > "$checkout/tests/runner/compiler_suites/$name.panack" <<SOURCE
import stdlib/testing as testing
import stdlib/environment as environment
import stdlib/host as host
import project/tests/runner/shared_dependency as dependency
pub run_suite(): Void {
  fail = match environment.get("SHARED_TEST_FAIL") { Some(value) => value, None() => "" }
  results = [testing.expect("$name", fail != "$name" && dependency.valid(), "injected failure")]
  if testing.report(results) > 0 { host.exit(1) }
}
SOURCE
done
printf 'pub pure valid(): Bool { true }\n' > "$checkout/tests/runner/shared_dependency.panack"
cd "$checkout"
export PANACK_PROBE_VM="$vm" PANACK_PROBE_SEED="$checkout/bootstrap/compiler-v9.bc"
export PANACKELTY_STDLIB_PATH="$checkout/src/stdlib" PANACK_PROBE_CACHE="$checkout/cache"
export VALIDATION_PROFILE_FILE="$checkout/profile"
source=tests/runner/compiler_shared_unit.panack
build() { sh tests/run_probe.sh --compile "$source" "$checkout/shared.bc" > build.out 2> build.err; }
build_count() { awk -F '\t' '$2 ~ /^probe-build\// { n++ } END { print n+0 }' profile; }
build
case_name=shared-compiler-each-suite-executes-separately
for name in $names; do
    for repeat in 1 2; do
        PANACK_PROBE_GROUP="compiler_${name}_unit" sh tests/run_probe.sh "$source" > actual 2> errors
        printf 'PASS %s\ntests: 1, failures: 0\n' "$name" > expected
        equal_files expected actual
        test ! -s errors || fail 'unexpected suite stderr'
    done
done
test "$(build_count)" = 1 || fail 'suite selection recompiled shared artifact'
pass

case_name=shared-compiler-worker-pool-executes-each-selection-once
set --
: > expected
for name in $names; do
    set -- "$@" "$source::compiler_${name}_unit"
    printf 'PASS %s\ntests: 1, failures: 0\n' "$name" >> expected
done
VALIDATION_JOBS=2 sh tests/run_probes.sh "$@" > actual 2> errors
equal_files expected actual
test ! -s errors || fail 'unexpected worker stderr'
test "$(build_count)" = 1 || fail 'workers recompiled prepared shared artifact'
pass

case_name=shared-compiler-rejects-missing-and-unknown-suite
for selection in '' unknown compiler_checker; do
    status=0
    PANACK_PROBE_GROUP="$selection" sh tests/run_probe.sh "$source" > actual 2> errors || status=$?
    test "$status" = 2 || fail 'invalid selection did not fail with status 2'
    test ! -s actual || fail 'invalid selection executed a suite'
    contains errors "unknown or missing compiler test suite: $selection"
done
status=0
env -u PANACK_PROBE_GROUP sh tests/run_probe.sh "$source" > actual 2> errors || status=$?
test "$status" = 2 || fail 'absent selection was accepted'
pass

case_name=shared-compiler-does-not-cache-assertion-success
status=0
SHARED_TEST_FAIL=checker PANACK_PROBE_GROUP=compiler_checker_unit sh tests/run_probe.sh "$source" > actual 2> errors || status=$?
test "$status" = 1 || fail 'assertion failure was hidden'
contains actual 'FAIL checker: injected failure'
contains actual 'tests: 1, failures: 1'
PANACK_PROBE_GROUP=compiler_checker_unit sh tests/run_probe.sh "$source" > actual 2> errors
contains actual 'tests: 1, failures: 0'
test "$(build_count)" = 1 || fail 'assertion failure changed compilation cache'
pass

case_name=shared-compiler-direct-and-transitive-invalidation
printf '\n// direct edit\n' >> tests/runner/compiler_suites/checker.panack
build
test "$(build_count)" = 2 || fail 'direct suite edit did not invalidate'
printf '\n// transitive edit\n' >> tests/runner/shared_dependency.panack
build
test "$(build_count)" = 3 || fail 'transitive suite edit did not invalidate'
pass

case_name=shared-compiler-corruption-and-incomplete-publication
for artifact in cache/*.bc; do printf 'corrupt\n' > "$artifact"; done
build
test "$(build_count)" = 4 || fail 'corrupt artifact was reused'
rm cache/*.sha256
build
test "$(build_count)" = 5 || fail 'incomplete publication was reused'
pass

case_name=shared-compiler-missing-suite-never-reuses-stale-artifact
mv tests/runner/compiler_suites/checker.panack checker.saved
if build; then fail 'missing suite compiled successfully'; fi
mv checker.saved tests/runner/compiler_suites/checker.panack
build
PANACK_PROBE_GROUP=compiler_checker_unit sh tests/run_probe.sh "$source" > actual 2> errors
contains actual 'tests: 1, failures: 0'
pass
