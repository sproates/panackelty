#!/bin/sh
# Run two independent Make suites with bounded workers and ordered output.
set -eu
[ "$#" = 3 ] || { echo 'expected make command and two suite names' >&2; exit 2; }
make_command=$1
left=$2
right=$3
jobs=${VALIDATION_JOBS:-4}
case "$jobs" in ''|*[!0-9]*) echo 'VALIDATION_JOBS must be an integer from 1 to 32' >&2; exit 2 ;; esac
while [ "${jobs#0}" != "$jobs" ] && [ "$jobs" != 0 ]; do jobs=${jobs#0}; done
[ "$jobs" -ge 1 ] && [ "$jobs" -le 32 ] || exit 2
VALIDATION_JOBS=$jobs
export VALIDATION_JOBS
if [ "$jobs" = 1 ]; then
    "$make_command" --no-print-directory "$left"
    "$make_command" --no-print-directory "$right"
    exit 0
fi
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-suite-pair.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'wait; exit 1' HUP INT TERM
# Reserve one worker for the left suite and the remainder for the right suite.
VALIDATION_JOBS=1 "$make_command" --no-print-directory "$left" > "$work/native.out" 2> "$work/native.err" &
native=$!
VALIDATION_JOBS=$((jobs - 1)) "$make_command" --no-print-directory "$right" > "$work/probes.out" 2> "$work/probes.err" &
probes=$!
status=0
wait "$native" || status=$?
probe_status=0
wait "$probes" || probe_status=$?
for group in native probes; do
    cat "$work/$group.out"
    cat "$work/$group.err" >&2
done
if [ "$status" = 0 ]; then status=$probe_status; fi
exit "$status"
