#!/bin/sh
set -eu

work=$(mktemp -d tests/http-package.XXXXXX)
server_pid=
holder_pid=
cleanup() {
    if [ -n "$server_pid" ]; then kill "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
    if [ -n "$holder_pid" ]; then kill "$holder_pid" 2>/dev/null || :; wait "$holder_pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup EXIT HUP INT TERM
http_client=${PANACK_HTTP_CLIENT:?}
mkdir -p "$work/app/src"
version=$(sed -n '1p' VERSION)

cat > "$work/app/panack.json" <<MANIFEST
{"schema":1,"package":"tests.http_app","version":"0.1.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
MANIFEST

cat > "$work/app/src/main.panack" <<'SOURCE'
import http/server as http
import stdlib/text as text

async respond(request: http.Request): Result[http.Response,http.HttpError] {
  Ok(http.Response(200, [http.Header("Content-Type", "text/plain")],
    text.encode_utf8("ok")))
}

async main(): Unit {
  report = await http.serve("127.0.0.1", 9000, @respond,
    http.ServerLimits(1, 1, 4096, 4096, 1000, 1000, 100))
  ()
}
SOURCE

./panack run packages/http/src/parser_test.panack
./panack compile packages/http/src/parser_test.panack -o "$work/parser.bc" > /dev/null
./panack run "$work/parser.bc"
./panack check --project "$work/app/panack.json" > /dev/null
./panack compile --project "$work/app/panack.json" -o "$work/http-app.bc" > /dev/null
test -s "$work/http-app.bc"
./panack check --project examples/network/http_server/panack.json > /dev/null
./panack compile --project examples/network/http_server/panack.json \
    -o "$work/documented-example.bc" > /dev/null
port=$("$http_client" port)
sed "s/8080/$port/g" examples/network/http_server/src/main.panack > "$work/app/src/main.panack"
./panack check --project "$work/app/panack.json" > /dev/null
./panack compile --project "$work/app/panack.json" -o "$work/example.bc" > /dev/null
for mode in source bytecode; do
    if [ "$mode" = source ]; then
        ./panack run --project "$work/app/panack.json" > "$work/stdout" 2> "$work/stderr" &
    else
        ./panack run "$work/example.bc" > "$work/stdout" 2> "$work/stderr" &
    fi
    server_pid=$!
    "$http_client" "$port" example || { cat "$work/stderr"; exit 1; }
    wait "$server_pid" || { cat "$work/stderr"; exit 1; }
    server_pid=
    test ! -s "$work/stdout" && test ! -s "$work/stderr"
done

cat > "$work/server.panack" <<'SOURCE'
import http/server as http
import stdlib/text as text

async respond(request: http.Request): Result[http.Response,http.HttpError] {
  if request.target == "/fail" {
    Error(http.HttpError.InvalidRequest())
  } else if request.target == "/echo" {
    Ok(http.Response(200, [], request.body))
  } else {
    Ok(http.Response(200, [], text.encode_utf8("ok")))
  }
}

pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}

async main(): Unit {
  result = await http.serve("127.0.0.1", PORT, @respond,
    http.ServerLimits(12, 2, 131072, 4096, 3000, 10000, 1000))
  match result {
    Error(problem) => expect(false),
    Ok(reports) => expect(reports.len() == 12)
  }
  ()
}
SOURCE
port=$("$http_client" port)
sed "s/PORT/$port/g" "$work/server.panack" > "$work/app/src/main.panack"
./panack check --project "$work/app/panack.json" > /dev/null
./panack compile --project "$work/app/panack.json" -o "$work/run.bc" > /dev/null
for extension in panack bc; do
    if [ "$extension" = panack ]; then
        ./panack run --project "$work/app/panack.json" > "$work/stdout" 2> "$work/stderr" &
    else
        ./panack run "$work/run.bc" > "$work/stdout" 2> "$work/stderr" &
    fi
    server_pid=$!
    for mode in get post handler-error method malformed partial body-limit header-limit; do
        "$http_client" "$port" "$mode" || { cat "$work/stderr"; exit 1; }
    done
    "$http_client" "$port" concurrent || { cat "$work/stderr"; exit 1; }
    "$http_client" "$port" disconnect || { cat "$work/stderr"; exit 1; }
    "$http_client" "$port" get || { cat "$work/stderr"; exit 1; }
    wait "$server_pid" || { cat "$work/stderr"; exit 1; }
    server_pid=
    test ! -s "$work/stdout" && test ! -s "$work/stderr"
done

cat > "$work/conflict.panack" <<'SOURCE'
import http/server as http
async respond(request: http.Request): Result[http.Response,http.HttpError] {
  Ok(http.Response(200, [], request.body))
}
pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}
async main(): Unit {
  result = await http.serve("127.0.0.1", PORT, @respond,
    http.ServerLimits(1, 1, 4096, 4096, 1000, 1000, 100))
  match result {
    Error(problem) => (),
    Ok(reports) => expect(false)
  }
  ()
}
SOURCE
"$http_client" reserve > "$work/held-port" &
holder_pid=$!
attempt=0
while [ ! -s "$work/held-port" ] && [ "$attempt" -lt 100 ]; do
    sleep 0.05
    attempt=$((attempt + 1))
done
test -s "$work/held-port"
port=$(cat "$work/held-port")
sed "s/PORT/$port/g" "$work/conflict.panack" > "$work/app/src/main.panack"
./panack check --project "$work/app/panack.json" > /dev/null
./panack run --project "$work/app/panack.json" > "$work/stdout" 2> "$work/stderr"
kill "$holder_pid"
wait "$holder_pid" 2>/dev/null || :
holder_pid=
test ! -s "$work/stdout" && test ! -s "$work/stderr"

printf '%s\n' 'HTTP package parser/serializer and server interoperability: ok'
