#!/bin/sh
# Run unit and bootstrap checks concurrently with isolated bootstrap artifacts.
set -eu
[ "$#" = 1 ] || { echo 'expected make command' >&2; exit 2; }
make_command=$1
jobs=${VALIDATION_JOBS:-4}
case "$jobs" in ''|*[!0-9]*) echo 'VALIDATION_JOBS must be an integer from 1 to 32' >&2; exit 2 ;; esac
while [ "${jobs#0}" != "$jobs" ] && [ "$jobs" != 0 ]; do jobs=${jobs#0}; done
[ "$jobs" -ge 1 ] && [ "$jobs" -le 32 ] || exit 2

if [ "$jobs" = 1 ]; then
    printf '%s\n' 'running unit then bootstrap sequentially with one worker'
    "$make_command" --no-print-directory unit
    "$make_command" --no-print-directory bootstrap-check
    exit 0
fi

unit_jobs=$((jobs - 1))
created_build=0
if [ ! -d build ]; then
    mkdir build
    created_build=1
fi
work=
work=$(mktemp -d 'build/check-phases.XXXXXX')
trap 'rm -rf "$work"; if [ "$created_build" = 1 ]; then rmdir build 2>/dev/null || :; fi' 0
trap 'wait; exit 1' HUP INT TERM

bootstrap_report=
if [ -n "${PANACK_CHECK_RUNNER_REPORT:-}" ]; then
    bootstrap_report=${PANACK_CHECK_RUNNER_REPORT}.bootstrap
fi

printf 'running unit with %s workers and bootstrap with 1 worker in parallel\n' "$unit_jobs"
VALIDATION_JOBS=$unit_jobs "$make_command" --no-print-directory unit \
    > "$work/unit.out" 2> "$work/unit.err" &
unit_pid=$!

if [ -n "$bootstrap_report" ]; then
    VALIDATION_JOBS=1 PANACK_CHECK_RUNNER_REPORT="$bootstrap_report" \
        "$make_command" --no-print-directory \
        "BOOTSTRAP_DIR=$work/bootstrap" \
        "PANACK_PROBE_CACHE=$work/probes" bootstrap-check \
        > "$work/bootstrap.out" 2> "$work/bootstrap.err" &
else
    env -u PANACK_CHECK_RUNNER_REPORT VALIDATION_JOBS=1 \
        "$make_command" --no-print-directory \
        "BOOTSTRAP_DIR=$work/bootstrap" \
        "PANACK_PROBE_CACHE=$work/probes" bootstrap-check \
        > "$work/bootstrap.out" 2> "$work/bootstrap.err" &
fi
bootstrap_pid=$!

unit_status=0
if wait "$unit_pid"; then :; else unit_status=$?; fi
bootstrap_status=0
if wait "$bootstrap_pid"; then :; else bootstrap_status=$?; fi

printf '%s\n' '== unit =='
cat "$work/unit.out"
cat "$work/unit.err" >&2
printf '%s\n' '== bootstrap =='
cat "$work/bootstrap.out"
cat "$work/bootstrap.err" >&2

if [ "$unit_status" -ne 0 ]; then exit "$unit_status"; fi
exit "$bootstrap_status"
