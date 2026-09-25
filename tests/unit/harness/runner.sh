#!/bin/sh
. tests/unit/harness/common.sh
runner=$root/tests/runner/main.panack
case_name=smoke-missing-and-mismatched-report
printf 'wrong runner output\n' > "$work/report"
capture 1 30 env PANACK_TEST_RUNNER_REPORT="$work/report" ./panack run tests/functional/cases/runner_smoke/main.panack
contains "$work/stdout" 'FAIL Panackelty fixture runner: runner report differed'
rm "$work/report"
capture 1 30 env PANACK_TEST_RUNNER_REPORT="$work/report" ./panack run tests/functional/cases/runner_smoke/main.panack
contains "$work/stdout" 'FAIL Panackelty fixture runner: runner report read failed'
pass
case_name=failure-commands
capture 0 30 ./panack run "$runner" --failure unknown_name
for command in check compile run disasm; do contains "$work/stdout" "PASS failure/unknown_name/$command"; done
contains "$work/stdout" 'PASS failure/unknown_name/no artifact'
contains "$work/stdout" 'tests: 7, failures: 0'
pass
case_name=incremental-failure-corpus
capture 0 45 ./panack run "$runner" --failures-only
contains "$work/stdout" 'PASS failure/while_condition_type/no artifact'
contains "$work/stdout" 'tests: 137, failures: 0'
pass
for fixture in cli_environment_files cli_commands cli_check_disasm stdlib; do
    case_name=runner-$fixture
    capture 0 45 env PANACKELTY_STDLIB_VALUE=ambient-test ./panack run "$runner" --case "$fixture"
    contains "$work/stdout" "PASS case/$fixture/source"
    contains "$work/stdout" "PASS case/$fixture/bytecode"
    contains "$work/stdout" 'tests: 4, failures: 0'
    pass
done
case_name=nonempty-workspace-recovery
mkdir "$work/cleanup"
capture 1 45 ./panack run "$runner" --test-cleanup-failure "$work/cleanup" --case hello_world
contains "$work/stdout" 'FAIL workspace cleanup'
contains "$work/stdout" 'tests: 5, failures: 1'
absent "$work/stdout" 'FAIL cleanup recovery'
absent "$work/stdout" 'FAIL workspace recovery'
test -z "$(find "$work/cleanup" -mindepth 1 -print)" || fail 'workspace was not emptied'
pass
# Isolated writable fixture checkout; never mutate repository fixtures.
checkout=$work/'checkout with spaces (test)'
mkdir -p "$checkout/tests/functional/cases"
ln -s "$root/panack" "$checkout/panack"
ln -s "$root/src" "$checkout/src"
cd "$checkout"
fixture=tests/functional/failures/imported_unknown_name
mkdir -p "$fixture"
for name in main.panack dependency.panack expected.stderr; do
    cp "$root/$fixture/$name" "$fixture/$name"
done
case_name=failure-diagnostic-and-artifact-cleanup
capture 0 30 ./panack run "$runner" --failure imported_unknown_name
for command in check compile 'no artifact'; do contains "$work/stdout" "PASS failure/imported_unknown_name/$command"; done
printf 'wrong diagnostic\n' > "$fixture/expected.stderr"
capture 1 30 ./panack run "$runner" --failure imported_unknown_name
for command in check compile; do contains "$work/stdout" "FAIL failure/imported_unknown_name/$command"; done
contains "$work/stdout" 'PASS failure/imported_unknown_name/no artifact'
absent "$work/stdout" 'FAIL workspace cleanup'
printf 'main(): Void { print(42) }\n' > "$fixture/main.panack"
capture 1 30 ./panack run "$runner" --failure imported_unknown_name
contains "$work/stdout" 'FAIL failure/imported_unknown_name/no artifact'
absent "$work/stdout" 'FAIL workspace cleanup'
rm "$fixture/expected.stderr"
capture 1 30 ./panack run "$runner" --failure imported_unknown_name
contains "$work/stdout" 'FAIL failure/imported_unknown_name/fixture'
pass
case_name=example-mismatch-and-missing-pairs
mkdir -p examples tests/functional/expected/examples
cp "$root/examples/vm_loop.panack" examples/
expected=tests/functional/expected/examples/vm_loop.stdout
printf 'wrong output\n' > "$expected"
capture 1 30 ./panack run "$runner" --example vm_loop
for mode in source bytecode; do contains "$work/stdout" "FAIL example/vm_loop/$mode"; done
contains "$work/stdout" 'PASS selected example count'
absent "$work/stdout" 'FAIL workspace cleanup'
rm "$expected"
capture 1 30 ./panack run "$runner" --example vm_loop
contains "$work/stdout" 'FAIL example/vm_loop/fixture: missing expected stdout'
printf 'stale\n' > tests/functional/expected/examples/orphan.stdout
capture 1 30 ./panack run "$runner" --example orphan
contains "$work/stdout" 'FAIL example/orphan/fixture: missing example source'
pass
case_name=case-output-mismatch
mkdir tests/functional/cases/hello_world
cp "$root/tests/functional/cases/hello_world/main.panack" tests/functional/cases/hello_world/
printf 'wrong output\n' > tests/functional/cases/hello_world/expected.stdout
capture 1 45 ./panack run "$runner" --case hello_world
for mode in source bytecode; do contains "$work/stdout" "FAIL case/hello_world/$mode"; done
contains "$work/stdout" 'PASS selected fixture count'
absent "$work/stdout" 'FAIL workspace cleanup'
pass
case_name=source-path-containment
fixture=tests/functional/cases/compiler_skeleton
mkdir "$fixture"
printf 'main(): Void { print(42) }\n' > "$work/outside.panack"
ln -s "$work/outside.panack" "$fixture/outside.panack"
for target in ../outside.panack "$work/outside.panack" "$fixture/outside.panack"; do
    printf '%s\n' "$target" > "$fixture/source.path"
    capture 1 20 ./panack run "$runner" --case compiler_skeleton
    contains "$work/stdout" 'FAIL case/compiler_skeleton/fixture'
    absent "$work/stdout" 'PASS case/compiler_skeleton/source'
done
pass
