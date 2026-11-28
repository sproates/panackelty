#!/bin/sh
set -eu
work=$(mktemp -d tests/http-closures.XXXXXX)
server_pid=
cleanup() {
    status=$?
    if [ "$status" -ne 0 ] && [ -f "$work/stderr" ]; then cat "$work/stderr" >&2; fi
    if [ -n "$server_pid" ]; then kill -TERM "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup 0
trap 'exit 1' HUP INT TERM
client=${PANACK_HTTP_CLIENT:?}
mkdir -p "$work/app/src"
cp tests/fixtures/compiler_contracts/anonymous_functions/http.panack "$work/app/src/main.panack"
version=$(sed -n '1p' VERSION)
cat > "$work/app/panack.json" <<MANIFEST
{"schema":1,"package":"tests.http_closures","version":"0.1.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
MANIFEST
unset PANACKELTY_BOOTSTRAP_ROOT
./bin/panack deps lock --project "$work/app/panack.json" > /dev/null
export PANACKELTY_STDLIB_PATH="$PWD/src/stdlib"
export PANACKELTY_VM_PATH="$PWD/panack-vm"
export PANACKELTY_VERSION="$version"
if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    ./panack-vm run build/tools/panack-packages.bc prepare --project "$work/app/panack.json" > "$work/project"
    project_option=--resolved-project
    project_path="$work/project"
    set -- ./panack-vm run "$PANACK_TEST_COMPILER"
else
    project_option=--project
    project_path="$work/app/panack.json"
    set -- ./bin/panack
fi
"$@" compile "$project_option" "$project_path" -o "$work/http.bc" > /dev/null
for kind in ordinary contextual; do
    for mode in source bytecode; do
        port=$("$client" port)
        if [ "$mode" = source ]; then
            "$@" run "$project_option" "$project_path" "$port" "$kind" > "$work/stdout" 2> "$work/stderr" &
        else
            ./panack-vm run "$work/http.bc" "$port" "$kind" > "$work/stdout" 2> "$work/stderr" &
        fi
        server_pid=$!
        # Four registered methods and an unregistered path handled by fallback.
        "$client" "$port" contexts captured
        kill -TERM "$server_pid"
        wait "$server_pid"
        server_pid=
        test ! -s "$work/stdout" && test ! -s "$work/stderr"
    done
done
printf '%s\n' 'HTTP retained closures: ordinary/contextual routes and fallback in source/bytecode modes: ok'
