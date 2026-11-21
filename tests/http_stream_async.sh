#!/bin/sh
set -eu
work=$(mktemp -d tests/http-stream-async.XXXXXX)
fixture_pid=
server_pid=
cleanup() {
  status=$?
  if [ "$status" -ne 0 ]; then
    for log in "$work/fixture.log" "$work/stderr"; do test ! -f "$log" || cat "$log" >&2; done
  fi
  if [ -n "$fixture_pid" ]; then kill "$fixture_pid" 2>/dev/null || :; wait "$fixture_pid" 2>/dev/null || :; fi
  if [ -n "$server_pid" ]; then kill -TERM "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
  rm -rf "$work"
}
trap cleanup 0
trap 'exit 1' HUP INT TERM
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_VERSION="$(cat VERSION)" PANACKELTY_BOOTSTRAP_ROOT="$PWD" PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else "${PANACK_TEST_COMMAND:-./bin/panack}" "$@"; fi
}
fixture=${PANACK_HTTP_FIXTURE:-build/vm/http_fixture}
"$fixture" "$work/port" > "$work/fixture.log" 2>&1 &
fixture_pid=$!
tries=0
while [ ! -s "$work/port" ]; do
  tries=$((tries + 1)); test "$tries" -lt 100; kill -0 "$fixture_pid"; sleep 0.05
done
port=$(cat "$work/port")
cat > "$work/template.panack" <<'SOURCE'
import stdlib/network/http_streamSUFFIX as stream
import stdlib/time as time
import stdlib/hostSUFFIX as host
ASYNC fail(status: Nat): Unit { AWAIT host.exit(status); () }
ASYNC expect(ok: Bool): Unit { if !ok { AWAIT fail(1) }; () }
ASYNC main(): Unit {
  request = "GET / HTTP/1.1\r\nHost: test\r\nConnection: close\r\n\r\n".encode_utf8()
  match AWAIT stream.open("127.0.0.1", PORT, false, request, 4096, time.seconds(2)) {
    Error(problem) => { AWAIT fail(2) },
    Ok(handle) => {
      mut total: Nat = 0
      mut ended = false
      while !ended {
        match AWAIT stream.read(handle) {
          Error(problem) => { AWAIT fail(3) },
          Ok(chunk) => { AWAIT expect(chunk.length() <= 16384); total = total + chunk.length(); ended = chunk.length() == 0; () }
        }
      }
      AWAIT expect(total > 50)
      match AWAIT stream.read(handle) { Ok(chunk) => AWAIT expect(chunk.length() == 0), Error(_) => AWAIT fail(4) }
      AWAIT expect(AWAIT stream.close(handle) == Ok(()))
      match AWAIT stream.read(handle) { Error(_) => (), Ok(_) => AWAIT fail(5) }
      match AWAIT stream.close(handle) { Error(_) => (), Ok(_) => AWAIT fail(6) }
    }
  }
  for timeout in [time.nanoseconds(0), time.seconds(61)] {
    match AWAIT stream.open("127.0.0.1", PORT, false, request, 4096, timeout) {
      Error(_) => (), Ok(handle) => { closed = AWAIT stream.close(handle); AWAIT fail(7) }
    }
  }
  match AWAIT stream.open("127.0.0.1", PORT, false, request, 1, time.seconds(2)) {
    Error(_) => AWAIT fail(8),
    Ok(handle) => {
      match AWAIT stream.read(handle) { Error(_) => (), Ok(_) => AWAIT fail(9) }
      closed = AWAIT stream.close(handle); ()
    }
  }
}
SOURCE
for effect in ordinary async; do
  if [ "$effect" = async ]; then suffix=/async; effect_keyword=async; await_keyword=await
  else suffix=; effect_keyword=; await_keyword=; fi
  sed -e "s|SUFFIX|$suffix|g" -e "s/ASYNC/$effect_keyword/g" -e "s/AWAIT/$await_keyword/g" -e "s/PORT/$port/g" "$work/template.panack" > "$work/$effect.panack"
  panack run "$work/$effect.panack"
  panack compile "$work/$effect.panack" -o "$work/$effect.bc" > /dev/null
  panack run "$work/$effect.bc"
  panack disasm "$work/$effect.bc" > "$work/$effect.disasm"
done
cat > "$work/forged.panack" <<'SOURCE'
import stdlib/network/http_stream as stream
main(): Unit { stream.read(1); () }
SOURCE
if panack check "$work/forged.panack" > "$work/stderr" 2>&1; then echo 'numeric handle was accepted' >&2; exit 1; fi
grep -Eq 'HttpStream' "$work/stderr"
cat > "$work/pure.panack" <<'SOURCE'
import stdlib/network/http_stream as stream
pure bad(handle: HttpStream): Result[Bytes,Str] { stream.read(handle) }
main(): Unit { () }
SOURCE
if panack check "$work/pure.panack" > "$work/stderr" 2>&1; then echo 'pure stream access was accepted' >&2; exit 1; fi
grep -Eq 'pure' "$work/stderr"
cat > "$work/ordinary-await.panack" <<'SOURCE'
import stdlib/network/http_stream/async as stream
bad(handle: HttpStream): Result[Bytes,Str] { await stream.read(handle) }
main(): Unit { () }
SOURCE
if panack check "$work/ordinary-await.panack" > "$work/stderr" 2>&1; then echo 'ordinary await was accepted' >&2; exit 1; fi
grep -Eq 'async|await' "$work/stderr"
printf '%s\n' 'Typed HTTP streams: ordinary/async, source/bytecode, bounds, EOF, close and effect checks passed.'
# A handler reads from another route on this same server. A blocking read would
# prevent /fast from ever running, so success proves scheduler/network progress.
: > "$work/stderr"
client=${PANACK_HTTP_CLIENT:-build/vm/http_client}
server_pid=
mkdir -p "$work/app/src"
version=$(sed -n '1p' VERSION)
cat > "$work/app/panack.json" <<MANIFEST
{"schema":1,"package":"tests.http_stream_async","version":"0.1.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
MANIFEST
# The package tool is unchanged; candidate selection applies to compilation.
"${PANACK_TEST_COMMAND:-./bin/panack}" deps lock --project "$work/app/panack.json" > /dev/null
server_port=$("$client" port)
cat > "$work/server-template.panack" <<'SOURCE'
import http/server as http
import stdlib/network/http_stream/async as stream
import stdlib/time as time
import stdlib/host/async as host
async fast(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("ready"); Ok(())
}
async loopback(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  raw = "GET /fast HTTP/1.1\r\nHost: local\r\nConnection: close\r\n\r\n".encode_utf8()
  match await stream.open("127.0.0.1", PORT, false, raw, 4096, time.seconds(3)) {
    Error(problem) => Error(problem),
    Ok(handle) => {
      mut total: Nat = 0
      mut ended = false
      mut failed = false
      while !ended {
        match await stream.read(handle) {
          Error(_) => { failed = true; ended = true; () },
          Ok(chunk) => { total = total + chunk.length(); ended = chunk.length() == 0; () }
        }
      }
      closed = await stream.close(handle)
      if failed || total < 20 { Error("stream read failed") }
      else { response.send("stream-ok"); Ok(()) }
    }
  }
}
async main(): Unit {
  server = http.create_bound_server("127.0.0.1", PORT)
  server.get("/stream", @loopback)
  server.get("/fast", @fast)
  match await server.listen() { Ok(_) => (), Error(_) => await host.exit(1) }
}
SOURCE
sed "s/PORT/$server_port/g" "$work/server-template.panack" > "$work/app/src/main.panack"
if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
  PANACKELTY_VERSION="$version" PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run build/tools/panack-packages.bc prepare --project "$work/app/panack.json" > "$work/resolved"
  set -- --resolved-project "$work/resolved"
else set -- --project "$work/app/panack.json"; fi
panack compile "$@" -o "$work/server.bc" > /dev/null
run_server() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    # Match the public launcher: saved programs run directly on the native VM.
    case "$1" in *.bc) exec ./panack-vm run "$@" ;; esac
    exec env PANACKELTY_VERSION="$version" PANACKELTY_BOOTSTRAP_ROOT="$PWD" PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" run "$@"
  else exec "${PANACK_TEST_COMMAND:-./bin/panack}" run "$@"; fi
}
for mode in source bytecode; do
  if [ "$mode" = source ]; then
    run_server "$@" > "$work/stdout" 2> "$work/stderr" &
  else
    run_server "$work/server.bc" > "$work/stdout" 2> "$work/stderr" &
  fi
  server_pid=$!
  "$client" "$server_port" stream-progress
  kill -TERM "$server_pid"; wait "$server_pid"; server_pid=
  test ! -s "$work/stderr"
done
printf '%s\n' 'HTTP stream handler yields to another handler: source and bytecode passed.'
