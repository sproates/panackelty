#!/bin/sh
set -eu

work=$(mktemp -d tests/http-package.XXXXXX)
trap 'rm -rf "$work"' EXIT HUP INT TERM
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
printf '%s\n' 'HTTP package parser/serializer and public server facade: ok'
