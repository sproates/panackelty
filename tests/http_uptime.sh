#!/bin/sh
set -eu
work=$(mktemp -d tests/http-uptime.XXXXXX)
server_pid=
cleanup() {
    status=$?
    if [ "$status" -ne 0 ] && [ -f "$work/stderr" ]; then cat "$work/stderr" >&2; fi
    if [ -n "$server_pid" ]; then kill "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup 0
trap 'exit 1' HUP INT TERM
client=${PANACK_HTTP_CLIENT:?}
project=examples/network/http_uptime/panack.json
./bin/panack check --project "$project" > /dev/null
./bin/panack compile --project "$project" -o "$work/uptime.bc" > /dev/null
for mode in source bytecode; do
    port=$("$client" port)
    if [ "$mode" = source ]; then
        PANACK_SERVICE_NAME=uptime-test ./bin/panack run --project "$project" "$port" > "$work/stdout" 2> "$work/stderr" &
    else
        PANACK_SERVICE_NAME=uptime-test ./bin/panack run "$work/uptime.bc" "$port" > "$work/stdout" 2> "$work/stderr" &
    fi
    server_pid=$!
    "$client" "$port" uptime
    kill -TERM "$server_pid"
    wait "$server_pid"
    server_pid=
    test ! -s "$work/stdout" && test ! -s "$work/stderr"
done
printf '%s\n' 'HTTP uptime example: increasing monotonic uptime in source and bytecode modes: ok'
