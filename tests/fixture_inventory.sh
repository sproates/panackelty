#!/bin/sh
# Read-only bookkeeping check; no compiler, VM or generated transcript required.
set -eu
export LC_ALL=C
root=${1:-.}
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$root"
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-inventory.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
for directory in examples examples/namespaces tests/functional/expected/examples; do
    test -d "$directory" || { echo "fixture inventory: missing $directory" >&2; exit 1; }
done
{
    for source in examples/*.panack; do
        test -f "$source" || continue
        name=${source##*/}; printf 'source %s\n' "${name%.panack}"
    done
    for source in examples/namespaces/*/main.panack; do
        test -f "$source" || continue
        directory=${source%/main.panack}; printf 'source %s\n' "${directory##*/}"
    done
    for output in tests/functional/expected/examples/*.stdout; do
        test -f "$output" || continue
        name=${output##*/}; printf 'output %s\n' "${name%.stdout}"
    done
} > "$work/inventory"
awk -f "$script_dir/fixture_inventory.awk" tests/runner/main.panack \
    tests/functional/cases/runner_smoke/main.panack "$work/inventory"
