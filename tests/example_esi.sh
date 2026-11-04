#!/bin/sh
# Run from the checkout root with a built toolchain and the HTTP test client.
set -eu
repo=$(pwd -P)
work=$(mktemp -d)
pid=
cleanup() {
    if [ -n "$pid" ]; then kill "$pid" 2>/dev/null || :; wait "$pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
example=examples/network/esi_includes
mkdir -p "$work/$example"
manifest="$repo/$example/panack.json"
"$repo/bin/panack" compile --project "$manifest" -o "$work/example.bc"
cd "$work"
for mode in source bytecode; do
    cp -R "$repo/$example/public" "$example/"
    port=$("$repo/build/vm/http_client" port)
    if [ "$mode" = source ]; then
        "$repo/bin/panack" run --project "$manifest" "$port" >server.log 2>&1 &
    else
        "$repo/bin/panack" run example.bc "$port" >server.log 2>&1 &
    fi
    pid=$!
    attempts=0
    until grep -q Listening server.log; do
        if ! kill -0 "$pid" || [ "$attempts" -ge 100 ]; then cat server.log >&2; exit 1; fi
        sleep 0.1
        attempts=$((attempts + 1))
    done
    base="http://127.0.0.1:$port"
    for route in / /about; do
        curl -fsS "$base$route" >body
        grep -F '<aside>Welcome to Panackelty.</aside>' body
        if grep -q '<esi:' body; then exit 1; fi
    done
    echo '<aside>Updated banner.</aside>' >"$example/public/fragments/banner.html"
    curl -fsS "$base/about" | grep -F 'Updated banner'
    rm "$example/public/fragments/banner.html"
    test "$(curl -sS -o body -w '%{http_code}' "$base/")" = 500
    kill "$pid"
    wait "$pid" || :
    pid=
done
