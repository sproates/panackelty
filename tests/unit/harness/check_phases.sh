#!/bin/sh
. tests/unit/harness/common.sh

case_name=check-phase-parallel-isolation-and-failure-propagation
mkdir "$work/phases"
cat > "$work/phases/make" <<'MAKE'
#!/bin/sh
set -eu
goal=
bootstrap_dir=false
probe_cache=false
for argument in "$@"; do
    goal=$argument
    case "$argument" in
        BOOTSTRAP_DIR=build/check-phases.*/bootstrap) bootstrap_dir=true ;;
        BOOTSTRAP_DIR=*) exit 9 ;;
        PANACK_PROBE_CACHE=build/check-phases.*/probes) probe_cache=true ;;
        PANACK_PROBE_CACHE=*) exit 9 ;;
    esac
done
case "$goal" in
    unit)
        [ "$VALIDATION_JOBS" = "$PHASE_UNIT_JOBS" ] || exit 9
        [ "${PANACK_CHECK_RUNNER_REPORT:-}" = "$PHASE_REPORT" ] || exit 9
        phase=unit ;;
    bootstrap-check)
        [ "$VALIDATION_JOBS" = 1 ] || exit 9
        if [ "$PHASE_PARALLEL" = yes ]; then
            [ "$bootstrap_dir" = true ] && [ "$probe_cache" = true ] || exit 9
            [ "${PANACK_CHECK_RUNNER_REPORT:-}" = "$PHASE_REPORT.bootstrap" ] || exit 9
        else
            [ "$bootstrap_dir" = false ] && [ "$probe_cache" = false ] || exit 9
            [ "${PANACK_CHECK_RUNNER_REPORT:-}" = "$PHASE_REPORT" ] || exit 9
        fi
        phase=bootstrap ;;
    *) exit 9 ;;
esac
printf '%s\n' "$phase" >> "$PHASE_CALLS"
: > "$PHASE_CONTROL/$phase.started"
if [ "$PHASE_PARALLEL" = yes ]; then
    peer=unit
    [ "$phase" = unit ] && peer=bootstrap
    attempts=0
    while [ ! -f "$PHASE_CONTROL/$peer.started" ]; do
        attempts=$((attempts + 1))
        [ "$attempts" -lt 50 ] || exit 10
        sleep 0.1
    done
fi
printf '%s stdout\n' "$phase"
printf '%s stderr\n' "$phase" >&2
if [ "${PHASE_FAIL:-}" = "$phase" ]; then exit 7; fi
MAKE
chmod +x "$work/phases/make"
export PHASE_CONTROL="$work/phases"
PHASE_REPORT="$work/runner.report" PHASE_UNIT_JOBS=3 PHASE_PARALLEL=yes
PHASE_CALLS="$work/phases/calls"
export PHASE_REPORT PHASE_UNIT_JOBS PHASE_PARALLEL PHASE_CALLS
for fault in none unit bootstrap; do
    rm -f "$work/phases/"*.started
    : > "$PHASE_CALLS"
    PHASE_FAIL=$fault
    export PHASE_FAIL
    expected=0
    if [ "$fault" != none ]; then expected=7; fi
    capture "$expected" 8 env VALIDATION_JOBS=4 PANACK_CHECK_RUNNER_REPORT="$PHASE_REPORT" \
        sh tests/run_check_phases.sh "$work/phases/make"
    contains "$work/stdout" 'running unit with 3 workers and bootstrap with 1 worker in parallel'
    contains "$work/stdout" 'unit stdout'
    contains "$work/stdout" 'bootstrap stdout'
    contains "$work/stderr" 'unit stderr'
    contains "$work/stderr" 'bootstrap stderr'
    sort "$PHASE_CALLS" > "$work/phases/actual"
    printf 'bootstrap\nunit\n' > "$work/phases/expected"
    cmp "$work/phases/expected" "$work/phases/actual" || fail "lost check phase after $fault failure"
done

rm -f "$work/phases/"*.started
: > "$PHASE_CALLS"
PHASE_PARALLEL=no
PHASE_UNIT_JOBS=1
PHASE_FAIL=none
export PHASE_PARALLEL PHASE_UNIT_JOBS PHASE_FAIL
capture 0 5 env VALIDATION_JOBS=1 PANACK_CHECK_RUNNER_REPORT="$PHASE_REPORT" \
    sh tests/run_check_phases.sh "$work/phases/make"
contains "$work/stdout" 'running unit then bootstrap sequentially with one worker'
printf 'unit\nbootstrap\n' > "$work/phases/expected"
cmp "$work/phases/expected" "$PHASE_CALLS" || fail 'single worker did not retain serial ordering'
: > "$PHASE_CALLS"
capture 2 5 env VALIDATION_JOBS=33 PANACK_CHECK_RUNNER_REPORT="$PHASE_REPORT" \
    sh tests/run_check_phases.sh "$work/phases/make"
test ! -s "$PHASE_CALLS" || fail 'invalid worker count performed work'
unset PHASE_CONTROL PHASE_REPORT PHASE_UNIT_JOBS PHASE_PARALLEL PHASE_CALLS PHASE_FAIL
pass
