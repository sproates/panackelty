#!/bin/sh
set -eu

[ "$#" = 3 ] || { echo 'expected compiler, toolchain version, and standard library root' >&2; exit 2; }
compiler=$1
version=$2
stdlib=$3
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-package-source.XXXXXX")
trap 'rm -rf "$work"' EXIT
trap 'exit 1' HUP INT TERM

app="$work/app"
mkdir -p "$app/src"
cat > "$app/panack.json" <<MANIFEST
{"schema":1,"package":"example.source_limits","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{}}
MANIFEST
printf 'import project/chunk as chunk\nmain(): Void { }\n' > "$app/src/main.panack"
awk 'BEGIN {
    block = sprintf("%4095s", "") "\n"
    for (i = 0; i < 256; i++) printf "%s", block
    printf " "
}' > "$app/src/chunk.panack"

status=0
PANACKELTY_VERSION="$version" PANACKELTY_STDLIB_PATH="$stdlib" \
    ./panack-vm run "$compiler" check --project "$app/panack.json" \
    > "$work/stdout" 2> "$work/stderr" || status=$?
if [ "$status" -eq 1 ] && [ ! -s "$work/stdout" ] &&
    grep -F 'package source read failed (output_limit)' "$work/stderr" >/dev/null; then
    printf 'PASS oversized package source fails closed through CLI\n'
else
    printf 'FAIL oversized package source CLI rejection (exit %s)\n' "$status"
    cat "$work/stdout" >&2
    cat "$work/stderr" >&2
    exit 1
fi
