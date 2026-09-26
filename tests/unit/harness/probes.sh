#!/bin/sh
. tests/unit/harness/common.sh
checkout=$work/'probe checkout (spaces)'
mkdir -p "$checkout/src/stdlib" "$checkout/tests" "$checkout/examples" "$checkout/bootstrap"
cp tests/run_probe.sh tests/run_probes.sh tests/profile_command.sh tests/check.sh "$checkout/tests/"
printf 'seed\n' > "$checkout/bootstrap/compiler-v8.bc"
printf 'source output\n' > "$checkout/tests/probe.panack"
printf 'imported source\n' > "$checkout/src/stdlib/import.panack"
cat > "$checkout/vm" <<'VM'
#!/bin/sh
set -eu
if [ "$1" = check ]; then printf 'ok\n'; exit; fi
if [ "${3:-}" = compile ]; then
    printf 'compile\n' >> calls
    cp "$4" "$6"
    if [ -f reject-compile ]; then exit 7; fi
    if [ -f edit-input ]; then printf 'changed\n' >> src/stdlib/import.panack; fi
    printf 'wrote %s\n' "$6"
    exit
fi
printf 'run\n' >> runs
cat "$2"
shift 2
for argument do printf '%s\n' "$argument"; done
exit "${PROBE_TEST_EXIT:-0}"
VM
chmod +x "$checkout/vm"
cd "$checkout"
unset PANACKELTY_STDLIB_PATH PANACK_PROBE_SEED
PANACK_PROBE_VM=./vm
PANACK_PROBE_CACHE='cache with spaces'
export PANACK_PROBE_VM PANACK_PROBE_CACHE
probe() { sh tests/run_probe.sh tests/probe.panack 'argument with spaces' > actual; }
count() { test "$(wc -l < calls | tr -d ' ')" = "$1" || fail "expected $1 compilations"; }

case_name=probe-cache-reuses-compilation-not-results
probe
probe
count 1
printf 'source output\nargument with spaces\n' > expected
equal_files expected actual
test "$(wc -l < runs | tr -d ' ')" = 2 || fail 'cached probe was not executed'
if PROBE_TEST_EXIT=7 probe; then fail 'cached execution failure was hidden'; else test "$?" = 7; fi
unset PROBE_TEST_EXIT
count 1
sh tests/run_probe.sh --compile tests/probe.panack 'copied probe.bc' > actual
printf 'wrote copied probe.bc\n' > expected
equal_files expected actual
equal_files tests/probe.panack 'copied probe.bc'
test "$(wc -l < runs | tr -d ' ')" = 3 || fail 'compile-only mode executed the probe'
count 1
pass

case_name=check-session-is-fresh-and-cleaned-on-failure
printf 'stale report\n' > stale-report
export PANACK_CHECK_RUNNER_REPORT="$checkout/stale-report"
export PANACK_TEST_RUNNER_REPORT="$checkout/stale-report"
export PANACK_TEST_CAPTURE_RUNNER_REPORT="$checkout/stale-report"
for status in 0 7; do
    actual_status=0
    sh tests/check.sh sh -c '
        test ! -e "$PANACK_CHECK_RUNNER_REPORT" || exit 9
        test -z "${PANACK_TEST_RUNNER_REPORT+x}" || exit 9
        test -z "${PANACK_TEST_CAPTURE_RUNNER_REPORT+x}" || exit 9
        printf "%s\n" "$PANACK_CHECK_RUNNER_REPORT" > session-path
        printf "fresh report\n" > "$PANACK_CHECK_RUNNER_REPORT"
        exit "$1"
    ' sh "$status" || actual_status=$?
    test "$actual_status" = "$status" || fail 'check wrapper changed status or reused ambient report'
    test ! -e "$(cat session-path)" || fail 'check report survived session'
done
printf 'stale report\n' > expected
equal_files expected stale-report
unset PANACK_CHECK_RUNNER_REPORT PANACK_TEST_RUNNER_REPORT PANACK_TEST_CAPTURE_RUNNER_REPORT
pass

case_name=probe-cache-content-and-toolchain-invalidation
touch -r src/stdlib/import.panack original-time
printf 'edited import\n' >> src/stdlib/import.panack
touch -r original-time src/stdlib/import.panack
probe
count 2
printf 'new source\n' > src/added.panack
probe
count 3
rm src/added.panack
probe
# The previous content-addressed entry is valid again.
count 3
printf 'new seed\n' >> bootstrap/compiler-v8.bc
probe
count 4
printf '\n# different instrumented VM\n' >> vm
probe
count 5
printf 'new probe\n' >> tests/probe.panack
probe
count 6
pass

case_name=probe-cache-corruption-and-missing-digest
for artifact in "$PANACK_PROBE_CACHE"/*.bc; do printf 'corrupt\n' > "$artifact"; done
probe
count 7
rm "$PANACK_PROBE_CACHE"/*.sha256
probe
count 8
pass

case_name=probe-cache-failed-build-is-not-published
printf 'failed compile input\n' >> tests/probe.panack
touch reject-compile
if probe 2> errors; then fail 'failed compilation was accepted'; fi
count 9
rm reject-compile
probe
count 10
pass

case_name=probe-cache-rejects-input-edits-during-build
printf 'concurrent edit input\n' >> tests/probe.panack
touch edit-input
if probe 2> errors; then fail 'changed inputs were published'; fi
contains errors 'probe inputs changed during compilation'
rm edit-input
probe
count 12
test -z "$(find "$PANACK_PROBE_CACHE" -name '.compile.*' -print)" || fail 'temporary compilation directory leaked'
pass

case_name=probe-cache-missing-input-is-not-reused
rm bootstrap/compiler-v8.bc
if probe 2> errors; then fail 'missing compiler seed was accepted'; fi
count 12
pass

case_name=parallel-probes-preserve-order-streams-and-failures
cat > tests/run_probe.sh <<'PROBE'
#!/bin/sh
set -eu
case "$1" in
    first)
        attempts=0
        while [ ! -f second-started ] && [ "$attempts" -lt 5 ]; do
            sleep 1
            attempts=$((attempts + 1))
        done
        test -f second-started || exit 9 ;;
    second) touch second-started ;;
esac
printf '%s out\n' "$1"
printf '%s err\n' "$1" >&2
test "$1" != failed || exit 7
PROBE
VALIDATION_JOBS=2 sh tests/run_probes.sh first second > actual 2> errors
printf 'first out\nsecond out\n' > expected
equal_files expected actual
printf 'first err\nsecond err\n' > expected
equal_files expected errors
for jobs in 1 2; do
    status=0
    VALIDATION_JOBS=$jobs sh tests/run_probes.sh failed second last > actual 2> errors || status=$?
    test "$status" = 7 || fail 'batch lost failed probe status'
    printf 'failed out\nsecond out\nlast out\n' > expected
    equal_files expected actual
    printf 'failed err\nsecond err\nlast err\n' > expected
    equal_files expected errors
done
for jobs in 0 -1 invalid 33; do
    if VALIDATION_JOBS=$jobs sh tests/run_probes.sh first > actual 2> errors; then
        fail "accepted invalid concurrency: $jobs"
    fi
done
pass
