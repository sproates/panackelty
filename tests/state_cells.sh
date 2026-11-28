#!/bin/sh
set -eu
work=$(mktemp -d build/state-cells.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
compiler=${PANACK_TEST_COMPILER:-bootstrap/compiler-v9.bc}
export PANACKELTY_STDLIB_PATH="$PWD/src/stdlib"
fixtures=tests/fixtures/compiler_contracts/state_cells
for name in ordinary async captures recursive; do
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
for name in callable cell record resource pure impure-update hidden-generic transitive closure-generic map-key set-element; do
    if ./panack-vm run "$compiler" compile "$fixtures/$name.invalid.panack" -o "$work/rejected.bc" > "$work/rejected" 2>&1; then
        echo "State accepted invalid $name" >&2
        exit 1
    fi
    test ! -e "$work/rejected.bc"
    if [ "$name" = pure ]; then
        grep -q 'pure function cannot call impure function' "$work/rejected"
    elif [ "$name" = impure-update ]; then
        grep -q 'expected PureFn' "$work/rejected"
    elif [ "$name" = map-key ] || [ "$name" = set-element ]; then
        grep -q 'Use an immutable identifier' "$work/rejected"
    else
        grep -q 'Store immutable data' "$work/rejected"
    fi
done
printf '%s\n' 'State source/bytecode: ordinary, async, retained captures and payload/effect rejection: ok'

# Reuse the generic helper's cached code after changing only its caller.
mkdir "$work/project" "$work/cache"
chmod 700 "$work/cache"
cat > "$work/project/helper.panack" <<'SOURCE'
import stdlib/state as state
pub stash[T](value: T): Void { cell = state.create(value) }
SOURCE
cat > "$work/project/main.panack" <<'SOURCE'
import "helper.panack" as helper
main(): Void { helper.stash(1) }
SOURCE
case "$compiler" in
    /*) export PANACKELTY_COMPILER_PATH="$compiler" ;;
    *) export PANACKELTY_COMPILER_PATH="$PWD/$compiler" ;;
esac
export PANACKELTY_VM_PATH="$PWD/panack-vm"
export PANACKELTY_COMPILE_CACHE_DIR="$PWD/$work/cache" PANACKELTY_COMPILE_CACHE_TRACE=1
export PANACKELTY_COMPILE_CODE_CACHE=1
./panack-vm run "$compiler" --local-compile-cache compile "$work/project/main.panack" -o "$work/cold.bc" > "$work/cold.out" 2> "$work/cold.err"
./panack-vm run "$compiler" --local-compile-cache compile "$work/project/main.panack" -o "$work/warm.bc" > "$work/warm.out" 2> "$work/warm.err"
cmp "$work/cold.bc" "$work/warm.bc"
grep -q 'compile cache: hit' "$work/warm.err"
cat > "$work/project/main.panack" <<'SOURCE'
import "helper.panack" as helper
pure identity(value: Nat): Nat { value }
main(): Void { helper.stash(@identity) }
SOURCE
if ./panack-vm run "$compiler" --local-compile-cache compile "$work/project/main.panack" -o "$work/invalid.bc" > "$work/invalid.out" 2> "$work/invalid.err"; then
    echo 'State accepted invalid payload through cached generic helper' >&2
    exit 1
fi
grep -q 'as a State payload inside stash' "$work/invalid.err"
grep -q 'compile cache: code module hits ' "$work/invalid.err"
if grep -q 'compile cache: code module hits 0' "$work/invalid.err"; then
    echo 'State cache test did not reuse any code modules' >&2
    exit 1
fi
printf '%s\n' 'State generic requirements survive cold/warm code caching and caller edits: ok'
