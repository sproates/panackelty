#!/bin/sh
set -eu
work=$(mktemp -d tests/static-http.XXXXXX)
server_pid=
holder_pid=
cleanup() {
    for pid in "$server_pid" "$holder_pid"; do
        if [ -n "$pid" ]; then kill -KILL "$pid" 2>/dev/null || :; wait "$pid" 2>/dev/null || :; fi
    done
    rm -rf "$work"
}
trap cleanup EXIT HUP INT TERM
client=${PANACK_STATIC_HTTP_CLIENT:?}
port_client=${PANACK_HTTP_CLIENT:?}
wait_for_exit() {
    remaining=100
    while kill -0 "$server_pid" 2>/dev/null; do
        if [ "$remaining" = 0 ]; then cat "$work/stderr" >&2; echo 'static server exit timeout' >&2; exit 1; fi
        remaining=$((remaining - 1))
        sleep 0.05
    done
    wait "$server_pid"
}
stop_server() {
    kill -TERM "$server_pid"
    wait_for_exit
    server_pid=
}
make_root() {
    mkdir -p "$work/root/sub" "$work/root/empty"
    printf 'root page' > "$work/root/index.html"
    printf 'sub page' > "$work/root/sub/index.html"
    printf 'hello\000world' > "$work/root/hello.txt"
    printf 'space' > "$work/root/a b.txt"
    printf 'unicode' > "$work/root/é.txt"
    printf 'literal percent' > "$work/root/%2e.txt"
    printf 'css' > "$work/root/style.CSS"
    printf 'private' > "$work/secret"
    dd if=/dev/zero of="$work/root/asset.bin" bs=1000 count=700 2>/dev/null
    dd if=/dev/zero of="$work/root/large.bin" bs=1048576 count=1 2>/dev/null
    cp "$work/root/large.bin" "$work/root/framing.bin"
    printf x >> "$work/root/large.bin"
    ln -s hello.txt "$work/root/inside"
    ln -s ../secret "$work/root/outside"
    ln -s "$PWD/$work/root/hello.txt" "$work/root/absolute"
    ln -s loop "$work/root/loop"
    mkfifo "$work/root/pipe"
}
./bin/panack run packages/static_http/src/paths_test.panack
./bin/panack compile packages/static_http/src/paths_test.panack -o "$work/paths.bc" > /dev/null
./bin/panack run "$work/paths.bc"
./bin/panack run --project packages/static_http/tests/panack.json
./bin/panack compile --project packages/static_http/tests/panack.json -o "$work/config.bc" > /dev/null
./bin/panack run "$work/config.bc"
./bin/panack compile --project tools/http-server/panack.json -o "$work/server.bc" > /dev/null
make --no-print-directory install "PREFIX=$PWD/$work/installation with spaces" > "$work/install.log" 2>&1
mv "$work/installation with spaces" "$work/relocated installation"
installed=$PWD/$work/relocated\ installation/bin/panack-http-server
"$installed" --version > "$work/version"
printf 'panackelty-http-server 1\n' > "$work/expected"
cmp "$work/version" "$work/expected"
for mode in source bytecode installed; do
    make_root
    port=$("$port_client" port)
    root_path=$PWD/$work/root
    if [ "$mode" = source ]; then
        ./bin/panack run --project tools/http-server/panack.json --root "$PWD/$work/root" --port "$port" --no-cache --noindex > "$work/stdout" 2> "$work/stderr" &
    elif [ "$mode" = bytecode ]; then
        ./bin/panack run "$work/server.bc" --root "$PWD/$work/root" --port "$port" --no-cache --noindex > "$work/stdout" 2> "$work/stderr" &
    else
        (cd /tmp && exec "$installed" --root "$root_path" --port "$port" --no-cache --noindex) > "$work/stdout" 2> "$work/stderr" &
    fi
    server_pid=$!
    "$client" "$port" files || { cat "$work/stderr" >&2; exit 1; }
    printf 'edited' > "$work/root/hello.txt"
    "$client" "$port" changed
    mv "$work/root" "$work/old-root"
    mkdir "$work/root"
    printf 'replacement' > "$work/root/index.html"
    "$client" "$port" root-changed
    stop_server
    printf 'Serving http://127.0.0.1:%s/\n' "$port" > "$work/expected"
    cmp "$work/stdout" "$work/expected"
    grep 'resource limit exceeded' "$work/stderr" > /dev/null
    grep 'configured root was replaced' "$work/stderr" > /dev/null
    rm -rf "$work/root" "$work/old-root"
done
# Empty roots still become ready, and SIGINT follows the normal stop path.
mkdir "$work/root"
port=$("$port_client" port)
./bin/panack run "$work/server.bc" --root "$PWD/$work/root" --port "$port" --no-cache --noindex > "$work/stdout" 2> "$work/stderr" &
server_pid=$!
"$client" "$port" empty
kill -INT "$server_pid"
wait_for_exit
server_pid=
test ! -s "$work/stderr"
for mode in missing-root invalid-host invalid-port missing-value unknown; do
    case "$mode" in
        missing-root) set -- --root "$PWD/$work/missing"; expected=1 ;;
        invalid-host) set -- --root "$PWD/$work/root" --host localhost; expected=1 ;;
        invalid-port) set -- --port 0; expected=2 ;;
        missing-value) set -- --root; expected=2 ;;
        unknown) set -- --unknown; expected=2 ;;
    esac
    ./bin/panack run "$work/server.bc" "$@" > "$work/stdout" 2> "$work/stderr" &
    server_pid=$!
    result=0
    wait_for_exit || result=$?
    server_pid=
    test "$result" = "$expected"
    test ! -s "$work/stdout"
    grep '^http-server:' "$work/stderr" > /dev/null
done
# Bind failure must never print readiness.
"$port_client" reserve > "$work/port" &
holder_pid=$!
remaining=100
while [ ! -s "$work/port" ]; do
    test "$remaining" != 0
    remaining=$((remaining - 1))
    sleep 0.05
done
port=$(cat "$work/port")
./bin/panack run "$work/server.bc" --root "$PWD/$work/root" --port "$port" > "$work/stdout" 2> "$work/stderr" &
server_pid=$!
result=0
wait_for_exit || result=$?
server_pid=
test "$result" = 1
test ! -s "$work/stdout"
grep 'bind failed' "$work/stderr" > /dev/null
printf '%s\n' 'static HTTP source, bytecode and relocated installation: ok'
