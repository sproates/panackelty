#!/bin/sh
set -eu
work=$(mktemp -d build/state-cells.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
compiler=${PANACK_TEST_COMPILER:-bootstrap/compiler-v9.bc}
export PANACKELTY_STDLIB_PATH="$PWD/src/stdlib"
fixtures=tests/fixtures/compiler_contracts/state_cells
for name in ordinary async captures; do
    ./panack-vm run "$compiler" compile "$fixtures/$name.panack" -o "$work/$name.bc" > "$work/compile"
    ./panack-vm run "$work/$name.bc" > "$work/bytecode"
    ./panack-vm run "$compiler" run "$fixtures/$name.panack" > "$work/source"
    cmp "$work/source" "$work/bytecode"
    if [ "$name" = ordinary ]; then
        printf '0\n1\n2\n2\ntrue\nfalse\nhello!\n' > "$work/expected"
        cmp "$work/expected" "$work/source"
    else
        test ! -s "$work/source"
    fi
done
for name in callable cell record resource pure impure-update; do
    if ./panack-vm run "$compiler" compile "$fixtures/$name.invalid.panack" -o "$work/rejected.bc" > "$work/rejected" 2>&1; then
        echo "State accepted invalid $name" >&2
        exit 1
    fi
    test ! -e "$work/rejected.bc"
    if [ "$name" = pure ]; then
        grep -q 'pure function cannot call impure function' "$work/rejected"
    elif [ "$name" = impure-update ]; then
        grep -q 'expected PureFn' "$work/rejected"
    else
        grep -q 'Store immutable data' "$work/rejected"
    fi
done
printf '%s\n' 'State source/bytecode: ordinary, async, retained captures and payload/effect rejection: ok'
