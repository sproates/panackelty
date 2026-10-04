#!/bin/sh
# Harness contracts use a small fake toolchain and the real probe-cache driver.
set -eu
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-baseline-test.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
fail() { printf 'build baseline regression: %s\n' "$*" >&2; exit 1; }
# Expected failures must not hide an earlier setup/timer failure behind a
# secondary missing-file error. Keep the original bounded evidence in CI logs.
show_failure_evidence() {
    evidence_report=$1
    evidence_invocation=$2
    for evidence in "$evidence_invocation" "$evidence_report/clone.log" \
        "$evidence_report"/clean-*.log "$evidence_report"/*.time \
        "$evidence_report"/*.stderr "$evidence_report"/*.status \
        "$evidence_report"/*.timer-status; do
        if [ -f "$evidence" ]; then
            printf '%s\n' "--- failure evidence: ${evidence##*/} ---" >&2
            tail -n 40 "$evidence" >&2
        fi
    done
}
require_failure_sample() {
    for artifact in samples.tsv clean-full-1.status clean-full-1.timer-status; do
        if [ ! -f "$1/$artifact" ]; then
            show_failure_evidence "$1" "$2"
            fail "missing failure artifact: $artifact; original evidence above"
        fi
    done
}
mkdir -p "$work/source/scripts" "$work/source/tests" "$work/source/src/stdlib" "$work/source/examples" "$work/source/bootstrap" "$work/bin"
cp scripts/build_baseline.sh "$work/source/scripts/"
cp tests/run_probe.sh tests/profile_command.sh tests/without_interpreter.sh "$work/source/tests/"
touch "$work/source/examples/.keep" "$work/source/src/stdlib/.keep"
printf 'seed\n' > "$work/source/bootstrap/compiler-v9.bc"
printf 'tracked\n' > "$work/source/deleted.txt"
printf 'build/\n' > "$work/source/.gitignore"
cat > "$work/source/panack-vm" <<'VM'
#!/bin/sh
case "$1" in
 check) printf 'ok\n' ;;
 run)
 if [ "${3:-}" = compile ]; then
   result=42
   if grep '+ 3' tests/build-baseline-fixture/direct.panack >/dev/null || grep '{ 41 }' tests/build-baseline-fixture/transitive.panack >/dev/null; then result=43; fi
   printf '%s\n' "$result" > "$6"
   printf 'wrote %s\n' "$6"
 else
   if [ "${BASELINE_TEST_BAD_OUTPUT:-0}" = 1 ]; then printf 'wrong\n'; else cat "$2"; fi
 fi ;;
 *) exit 2 ;;
esac
VM
chmod +x "$work/source/panack-vm"
cat > "$work/bin/make" <<'MAKE'
#!/bin/sh
case "$*" in
 clean|ci-compiler) ;;
 *) [ -z "${CHECK_BUDGET_SECONDS-}${INCREMENTAL_BUDGET_SECONDS-}${FUNCTIONAL_BUDGET_SECONDS-}${BOOTSTRAP_BUDGET_SECONDS-}" ] || exit 7 ;;
esac
case "$*" in
 --version) printf 'Fixture make\n' ;;
 ci-compiler) exec "${BASELINE_TEST_SHELL:-sh}" scripts/build_baseline.sh "$BASELINE_TEST_OUTPUT" fixture-host "${BASELINE_TEST_REPEATS:-2}" "${BASELINE_TEST_MODE:-all}" ;;
 clean) rm -rf build ;;
 '-j1 native') : ;;
 '-j1 check'|'-j1 check-compiler')
   [ "${BASELINE_TEST_FAIL:-0}" = 0 ] || exit 9
   printf 'fixture checks passed\n' ;;
 *) exit 3 ;;
esac
MAKE
chmod +x "$work/bin/make"
# A wrapper proves the external timer survives isolation even when the calling
# shell has a time keyword. Resolve the real utility before changing PATH.
BASELINE_TEST_REAL_TIME=$(which time)
[ -x "$BASELINE_TEST_REAL_TIME" ] || fail 'external POSIX time unavailable'
BASELINE_TEST_TIME_CALLS=$work/time-calls
export BASELINE_TEST_REAL_TIME BASELINE_TEST_TIME_CALLS
cat > "$work/bin/time" <<'TIME'
#!/bin/sh
[ "${LC_ALL:-}" = C ] || exit 8
printf 'external timer\n' >> "$BASELINE_TEST_TIME_CALLS"
timer_status=0
"$BASELINE_TEST_REAL_TIME" "$@" || timer_status=$?
case "${BASELINE_TEST_TIMER_MODE:-normal}" in
 normalize) [ "$timer_status" = 0 ] || exit 1 ;;
 fail) [ "$timer_status" != 0 ] || exit 7 ;;
esac
exit "$timer_status"
TIME
chmod +x "$work/bin/time"
export PATH="$work/bin:$PATH"
cd "$work/source"
git init -q
git add .
git -c user.name=fixture -c user.email=fixture@example.invalid commit -qm fixture
# Both additions and deletions must be represented in the archived candidate.
rm deleted.txt
printf 'untracked candidate\n' > addition.txt
before=$(git status --porcelain)
LC_ALL=POSIX CHECK_BUDGET_SECONDS=999 INCREMENTAL_BUDGET_SECONDS=999 FUNCTIONAL_BUDGET_SECONDS=999 BOOTSTRAP_BUDGET_SECONDS=999 \
BASELINE_TEST_OUTPUT="$work/report" sh tests/without_interpreter.sh compiler > "$work/output" 2>&1 || { cat "$work/output" >&2; exit 1; }
if grep 'sw_vers:.*not found' "$work/output" >/dev/null; then
    fail 'missing optional macOS metadata command leaked into output'
fi
if [ "$(uname -s)" = Darwin ]; then
    expected_os=$(sw_vers -productVersion 2>/dev/null || printf unavailable)
    grep -F -x "os_version=$expected_os" "$work/report/environment.txt" >/dev/null || fail 'isolated macOS metadata differs from available host evidence'
fi
[ "$(wc -l < "$work/time-calls" | tr -d ' ')" = 12 ] || fail 'isolated external timer not used for every sample'
[ "$(git status --porcelain)" = "$before" ] || fail 'source worktree changed'
[ -f "$work/report/COMPLETE" ] || fail 'completion marker missing'
[ "$(wc -l < "$work/report/samples.tsv" | tr -d ' ')" = 13 ] || fail 'sample count'
awk -F '\t' '
$1 == "probe-unchanged" && ($5 != 0 || $6 != 1) { exit 1 }
$1 ~ /^probe-(unrelated|direct|transitive)$/ && ($5 != 1 || $6 != 1) { exit 1 }
' "$work/report/samples.tsv" || fail 'actual compilation/execution counts'
[ "$(wc -l < "$work/report/summary.tsv" | tr -d ' ')" = 7 ] || fail 'summary groups'
tar -tf "$work/report/source.tar" > "$work/paths"
grep '^addition.txt$' "$work/paths" >/dev/null || fail 'untracked candidate omitted'
if grep '^deleted.txt$' "$work/paths" >/dev/null; then fail 'deleted candidate retained'; fi
if find "$work/report" -type d -name 'work.*' | grep . >/dev/null; then fail 'disposable checkout retained'; fi
if sh scripts/build_baseline.sh "$work/report" fixture-host 1 probes > "$work/reject" 2>&1; then fail 'overwrote existing report'; fi
if sh scripts/build_baseline.sh "$PWD/output" fixture-host 1 probes > "$work/reject" 2>&1; then fail 'accepted source-local output'; fi
if sh scripts/build_baseline.sh "$work/invalid" fixture-host 0 probes > "$work/reject" 2>&1; then fail 'accepted invalid repeat count'; fi
if BASELINE_TEST_TIMER_MODE=normalize BASELINE_TEST_FAIL=1 sh -x scripts/build_baseline.sh "$work/failed" fixture-host 1 all > "$work/failure" 2>&1; then fail 'ignored failed validation'; fi
[ ! -e "$work/failed/COMPLETE" ] || fail 'failed run claimed complete'
require_failure_sample "$work/failed" "$work/failure"
recorded_status=$(awk -F '\t' '$1 == "clean-full" { print $4 }' "$work/failed/samples.tsv")
recorded_timer=$(cat "$work/failed/clean-full-1.timer-status")
if [ "$recorded_status" != 9 ]; then
    show_failure_evidence "$work/failed" "$work/failure"
    fail "failed sample not recorded: expected=9 sample=$recorded_status timer=$recorded_timer"
fi
[ "$recorded_timer" = 1 ] || fail "normalizing timer fixture not exercised: sample=$recorded_status timer=$recorded_timer"
if BASELINE_TEST_TIMER_MODE=fail sh scripts/build_baseline.sh "$work/timer-failed" fixture-host 1 all > "$work/failure" 2>&1; then fail 'ignored timer failure'; fi
[ ! -e "$work/timer-failed/COMPLETE" ] || fail 'timer failure claimed complete'
require_failure_sample "$work/timer-failed" "$work/failure"
[ "$(cat "$work/timer-failed/clean-full-1.status")" = 0 ] || fail 'timer-failure workload did not succeed'
recorded_status=$(awk -F '\t' '$1 == "clean-full" { print $4 }' "$work/timer-failed/samples.tsv")
recorded_timer=$(cat "$work/timer-failed/clean-full-1.timer-status")
if [ "$recorded_status" != 7 ]; then
    show_failure_evidence "$work/timer-failed" "$work/failure"
    fail "timer failure not recorded: expected=7 sample=$recorded_status timer=$recorded_timer"
fi
if BASELINE_TEST_BAD_OUTPUT=1 sh scripts/build_baseline.sh "$work/wrong" fixture-host 1 probes > "$work/failure" 2>&1; then fail 'ignored incorrect probe output'; fi
[ ! -e "$work/wrong/COMPLETE" ] || fail 'incorrect output claimed complete'
if command -v dash >/dev/null 2>&1; then
    BASELINE_TEST_SHELL=$(command -v dash) BASELINE_TEST_OUTPUT="$work/dash-report" \
    BASELINE_TEST_REPEATS=1 BASELINE_TEST_MODE=probes \
    sh tests/without_interpreter.sh compiler > "$work/dash-output" 2>&1 || {
        cat "$work/dash-output" >&2; fail 'dash isolated timing failed'
    }
    [ -f "$work/dash-report/COMPLETE" ] || fail 'dash report incomplete'
fi
printf 'Build baseline isolation, snapshot, cache counts, output, external timing and failure contracts passed.\n'
