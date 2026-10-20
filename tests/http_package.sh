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
    "ok"->encode_utf8()))
}

async main(): Unit {
  report = await http.serve("127.0.0.1", 9000, @respond,
    http.ServerLimits(1, 1, 4096, 4096, 1000, 1000, 100))
  ()
}
SOURCE

./bin/panack run packages/http/src/parser_test.panack
./bin/panack compile packages/http/src/parser_test.panack -o "$work/parser.bc" > /dev/null
./bin/panack run "$work/parser.bc"
./bin/panack check --project "$work/app/panack.json" > /dev/null
./bin/panack compile --project "$work/app/panack.json" -o "$work/http-app.bc" > /dev/null
test -s "$work/http-app.bc"
./bin/panack check --project examples/network/http_server/panack.json > /dev/null
./bin/panack compile --project examples/network/http_server/panack.json \
    -o "$work/documented-example.bc" > /dev/null
port=$("$http_client" port)
sed "s/8080/$port/g" examples/network/http_server/src/main.panack > "$work/app/src/main.panack"
./bin/panack check --project "$work/app/panack.json" > /dev/null
./bin/panack compile --project "$work/app/panack.json" -o "$work/example.bc" > /dev/null
for mode in source bytecode; do
    if [ "$mode" = source ]; then
        ./bin/panack run --project "$work/app/panack.json" > "$work/stdout" 2> "$work/stderr" &
    else
        ./bin/panack run "$work/example.bc" > "$work/stdout" 2> "$work/stderr" &
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
  } else if request.target == "/invalid" {
    Ok(http.Response(200, [http.Header("Content-Length", "3")], "ok"->encode_utf8()))
  } else if request.target == "/echo" {
    Ok(http.Response(200, [], request.body))
  } else {
    Ok(http.Response(200, [], "ok"->encode_utf8()))
  }
}

pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}

async main(): Unit {
  // Framing assertions need headroom for the 64 KiB header scan on hosted VMs.
  // Runtime deadline enforcement has separate native tests; this is not a
  // three-second performance assertion. All waits remain bounded.
  result = await http.serve("127.0.0.1", PORT, @respond,
    http.ServerLimits(18, 2, 131072, 4096, 10000, 30000, 1000))
  match result {
    Error(problem) => expect(false),
    Ok(reports) => expect(reports->len() == 18)
  }
  ()
}
SOURCE
port=$("$http_client" port)
sed "s/PORT/$port/g" "$work/server.panack" > "$work/app/src/main.panack"
./bin/panack check --project "$work/app/panack.json" > /dev/null
./bin/panack compile --project "$work/app/panack.json" -o "$work/run.bc" > /dev/null
for extension in panack bc; do
    if [ "$extension" = panack ]; then
        ./bin/panack run --project "$work/app/panack.json" > "$work/stdout" 2> "$work/stderr" &
    else
        ./bin/panack run "$work/run.bc" > "$work/stdout" 2> "$work/stderr" &
    fi
    server_pid=$!
    for mode in get post handler-error method malformed partial body-limit header-limit head head-body head-chunked head-error head-invalid-response head-malformed; do
        "$http_client" "$port" "$mode" || {
            printf 'HTTP package %s/%s failed\n' "$extension" "$mode" >&2
            cat "$work/stderr" >&2
            exit 1
        }
    done
    "$http_client" "$port" concurrent || { cat "$work/stderr"; exit 1; }
    "$http_client" "$port" disconnect || { cat "$work/stderr"; exit 1; }
    "$http_client" "$port" get || { cat "$work/stderr"; exit 1; }
    wait "$server_pid" || { cat "$work/stderr"; exit 1; }
    server_pid=
    test ! -s "$work/stdout" && test ! -s "$work/stderr"
done

cat > "$work/interop-server.panack" <<'SOURCE'
import http/server as http
import stdlib/text as text

pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}

async respond(request: http.Request): Result[http.Response,http.HttpError] {
  Ok(http.Response(200,
    [http.Header("Content-Type", "text/plain; charset=utf-8")],
    "hello from Panackelty HTTP server\n"->encode_utf8()))
}

async main(): Unit {
  result = await http.serve("127.0.0.1", PORT, @respond,
    http.ServerLimits(3, 1, 4096, 4096, 3000, 10000, 1000))
  match result {
    Error(problem) => expect(false),
    Ok(reports) => expect(reports->len() == 3)
  }
  ()
}
SOURCE

mkdir -p "$work/client/src"
cat > "$work/client/panack.json" <<MANIFEST
{"schema":1,"package":"tests.http_client_interop","version":"0.1.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
MANIFEST
cat > "$work/client/main.template" <<'SOURCE'
import http/client as http
import stdlib/text as text

pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}

async main(): Unit {
  result = await http.get("http://127.0.0.1:PORT/")
  match result {
    Error(problem) => expect(false),
    Ok(response) => {
      expect(response.status == 200)
      expect(response.body == "hello from Panackelty HTTP server\n"->encode_utf8())
      expect(response.headers[0].name == "Content-Type" &&
        response.headers[0].value == "text/plain; charset=utf-8")
    }
  }
  ()
}
SOURCE

for server_mode in source bytecode; do
    port=$("$http_client" port)
    sed "s/PORT/$port/g" "$work/interop-server.panack" > "$work/app/src/main.panack"
    sed "s/PORT/$port/g" "$work/client/main.template" > "$work/client/src/main.panack"
    ./bin/panack check --project "$work/app/panack.json" > /dev/null
    ./bin/panack compile --project "$work/app/panack.json" -o "$work/interop-server.bc" > /dev/null
    ./bin/panack check --project "$work/client/panack.json" > /dev/null
    ./bin/panack compile --project "$work/client/panack.json" -o "$work/interop-client.bc" > /dev/null
    if [ "$server_mode" = source ]; then
        ./bin/panack run --project "$work/app/panack.json" > "$work/server.stdout" 2> "$work/server.stderr" &
    else
        ./bin/panack run "$work/interop-server.bc" > "$work/server.stdout" 2> "$work/server.stderr" &
    fi
    server_pid=$!
    "$http_client" "$port" example || { cat "$work/server.stderr"; exit 1; }
    ./bin/panack run --project "$work/client/panack.json" > "$work/client.stdout" 2> "$work/client.stderr" || {
        cat "$work/client.stderr" "$work/server.stderr"
        exit 1
    }
    ./bin/panack run "$work/interop-client.bc" > "$work/client.stdout" 2> "$work/client.stderr" || {
        cat "$work/client.stderr" "$work/server.stderr"
        exit 1
    }
    wait "$server_pid" || { cat "$work/server.stderr"; exit 1; }
    server_pid=
    test ! -s "$work/server.stdout" && test ! -s "$work/server.stderr"
    test ! -s "$work/client.stdout" && test ! -s "$work/client.stderr"
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
./bin/panack check --project "$work/app/panack.json" > /dev/null
./bin/panack run --project "$work/app/panack.json" > "$work/stdout" 2> "$work/stderr"
kill "$holder_pid"
wait "$holder_pid" 2>/dev/null || :
holder_pid=
test ! -s "$work/stdout" && test ! -s "$work/stderr"

printf '%s\n' 'HTTP package parser/serializer and C/P5-client server interoperability: ok'
