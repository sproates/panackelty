#!/bin/sh
# The native corpus alone captures the session's functional runner observation.
# Other runtime probes use distinct artifacts and may run alongside that corpus.
set -eu
jobs=${VALIDATION_JOBS:-2}
case "$jobs" in ''|*[!0-9]*) echo 'VALIDATION_JOBS must be an integer from 1 to 32' >&2; exit 2 ;; esac
[ "$jobs" -ge 1 ] && [ "$jobs" -le 32 ] || exit 2
if [ "$jobs" = 1 ]; then
    "$@" --no-print-directory unit-runtime-native
    "$@" --no-print-directory unit-runtime-probes
    exit 0
fi
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-runtime-checks.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'wait; exit 1' HUP INT TERM
# Give each branch one worker: the combined concurrency stays bounded at two.
VALIDATION_JOBS=1 "$@" --no-print-directory unit-runtime-native > "$work/native.out" 2> "$work/native.err" &
native=$!
VALIDATION_JOBS=1 "$@" --no-print-directory unit-runtime-probes > "$work/probes.out" 2> "$work/probes.err" &
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
