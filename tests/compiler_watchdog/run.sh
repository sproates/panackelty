#!/bin/sh
# Retain the compiler fixture's exact output contract, with bounded progress
# instead of an aggregate compilation deadline. Standalone use builds helpers.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"
evidence=${PANACK_COMPILER_WATCHDOG_DIR:-$root/build/validation/compiler-watchdog}
mkdir -p "$evidence"
if ! make --no-print-directory compiler-watchdog-build > "$evidence/build.log" 2>&1; then
    cat "$evidence/build.log" >&2
    exit 1
fi
supervisor=${PANACK_COMPILER_WATCHDOG_BINARY:-$root/build/vm/compiler-watchdog}
observer=${PANACK_COMPILER_WATCHDOG_VM:-$root/build/vm/compiler-observer-vm}
PANACKELTY_BOOTSTRAP_ROOT=$root
PANACKELTY_STDLIB_PATH=$root/src/stdlib
export PANACKELTY_BOOTSTRAP_ROOT PANACKELTY_STDLIB_PATH
exec "$supervisor" 90 "$evidence" "$observer" run "${SEED_COMPILER:-bootstrap/compiler-v9.bc}" "$@"
