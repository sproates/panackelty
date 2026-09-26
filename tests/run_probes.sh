#!/bin/sh
# Independent probes share a bounded worker pool; report in requested order.
set -eu
jobs=${VALIDATION_JOBS:-2}
case "$jobs" in ''|*[!0-9]*) echo 'VALIDATION_JOBS must be an integer from 1 to 32' >&2; exit 2 ;; esac
if [ "$jobs" -lt 1 ] || [ "$jobs" -gt 32 ]; then
    echo 'VALIDATION_JOBS must be an integer from 1 to 32' >&2
    exit 2
fi
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-probes.XXXXXX")
trap 'rm -rf "$work"' 0
# Let outstanding probes finish before removing their output files.
trap 'wait; exit 1' HUP INT TERM
worker() {
    index=0
    for source do
        index=$((index + 1))
        # Claims remain until cleanup, so every probe executes exactly once.
        if mkdir "$work/$index.claim" 2>/dev/null; then
            name=${source##*/}
            result=0
            sh tests/profile_command.sh "probe/${name%.panack}" sh tests/run_probe.sh "$source" \
                > "$work/$index.out" 2> "$work/$index.err" || result=$?
            printf '%s\n' "$result" > "$work/$index.status"
        fi
    done
}
count=0
pids=
while [ "$count" -lt "$jobs" ] && [ "$count" -lt "$#" ]; do
    worker "$@" &
    pids="$pids $!"
    count=$((count + 1))
done
status=0
for pid in $pids; do
    if ! wait "$pid"; then status=1; fi
done
index=0
for source do
    index=$((index + 1))
    if [ ! -f "$work/$index.status" ]; then
        echo "probe did not complete: $source" >&2
        status=1
        continue
    fi
    IFS= read -r result < "$work/$index.status"
    cat "$work/$index.out"
    cat "$work/$index.err" >&2
    if [ "$status" = 0 ] && [ "$result" != 0 ]; then status=$result; fi
done
exit "$status"
