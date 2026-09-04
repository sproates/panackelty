#!/bin/sh

label=$1
budget=$2
shift 2

started=$(date +%s)
"$@"
status=$?
finished=$(date +%s)
elapsed=$((finished - started))

printf 'timing: %s %ss (budget %ss)\n' "$label" "$elapsed" "$budget"
if [ "$elapsed" -gt "$budget" ]; then
    printf 'warning: %s exceeded its %ss validation budget (%ss)\n' \
        "$label" "$budget" "$elapsed" >&2
fi

if [ -n "${VALIDATION_TIMINGS_FILE-}" ]; then
    if ! printf '%s\t%s\t%s\t%s\n' "$label" "$elapsed" "$budget" "$status" \
        >> "$VALIDATION_TIMINGS_FILE"; then
        printf 'warning: could not record %s timing in %s\n' \
            "$label" "$VALIDATION_TIMINGS_FILE" >&2
    fi
fi

exit "$status"
