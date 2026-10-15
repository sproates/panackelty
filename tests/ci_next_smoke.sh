#!/bin/sh
# Exercise the stage-2 compiler and native VM with one source/bytecode program.
set -eu
[ "$#" = 1 ] || { echo 'expected the stage-2 compiler path' >&2; exit 2; }
compiler=$1
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-next-smoke.XXXXXX")
trap 'rm -rf "$work"' EXIT
trap 'exit 1' HUP INT TERM
export PANACKELTY_STDLIB_PATH="$root/src/stdlib"
source=tests/functional/cases/hello_world/main.panack

PANACKELTY_BOOTSTRAP_ROOT="$root" ./panack-vm run "$compiler" run "$source" \
    > "$work/source.stdout" 2> "$work/source.stderr"
printf 'hello world\n' > "$work/expected"
cmp "$work/expected" "$work/source.stdout"
test ! -s "$work/source.stderr"

PANACKELTY_BOOTSTRAP_ROOT="$root" ./panack-vm run "$compiler" compile "$source" \
    -o "$work/hello.bc" > "$work/compile.stdout" 2> "$work/compile.stderr"
printf 'wrote %s\n' "$work/hello.bc" > "$work/expected"
cmp "$work/expected" "$work/compile.stdout"
test ! -s "$work/compile.stderr"
./panack-vm check "$work/hello.bc" > "$work/check.stdout"
printf 'ok\n' > "$work/expected"
cmp "$work/expected" "$work/check.stdout"
./panack-vm run "$work/hello.bc" > "$work/bytecode.stdout" 2> "$work/bytecode.stderr"
printf 'hello world\n' > "$work/expected"
cmp "$work/expected" "$work/bytecode.stdout"
test ! -s "$work/bytecode.stderr"
PANACK_BIN="$root/bin/panack" PANACK_HTTP_FIXTURE="$root/build/vm/http_fixture" \
    sh tests/http_client_package.sh dns-late
PANACK_BIN="$root/bin/panack" PANACK_HTTP_FIXTURE="$root/build/vm/http_fixture" \
    sh tests/http_client_package.sh dns-failure
PANACK_BIN="$root/bin/panack" PANACK_HTTP_FIXTURE="$root/build/vm/http_fixture" \
    sh tests/http_client_package.sh connect-failure
echo 'next compiler and runtime smoke passed.'
