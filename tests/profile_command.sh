#!/bin/sh
# Optional inclusive wall-clock observations, separate from budget enforcement.
label=$1
shift
if [ -z "${VALIDATION_PROFILE_FILE-}" ]; then
    exec "$@"
fi
parent=${VALIDATION_PROFILE_PARENT:--}
VALIDATION_PROFILE_PARENT=$label
export VALIDATION_PROFILE_PARENT
started=$(date +%s)
"$@"
status=$?
finished=$(date +%s)
# One append per row; nested observations overlap and must not be summed.
if ! printf '%s\t%s\t%s\t%s\t%s\n' "${VALIDATION_PROFILE_RUN:-unspecified}" \
    "$label" "$parent" "$((finished - started))" "$status" >> "$VALIDATION_PROFILE_FILE"; then
    printf 'warning: could not record profile for %s\n' "$label" >&2
fi
exit "$status"
