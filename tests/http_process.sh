#!/bin/sh
set -eu
work=$(mktemp -d tests/http-process.XXXXXX)
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
version=$(sed -n '1p' VERSION)
cat > "$work/app/panack.json" <<MANIFEST
{"schema":1,"package":"tests.http_process","version":"0.1.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
MANIFEST
./bin/panack deps lock --project "$work/app/panack.json" > /dev/null
cat > "$work/template.panack" <<'SOURCE'
import http/server as http
import stdlib/process/async as process
import stdlib/process::{Command, ProcessEnvironment.ExactEnvironment, ProcessTermination.Exited}
import stdlib/filesystem/async as files
import stdlib/path as path
import stdlib/time as time
import stdlib/host/async as host
pure p(value: Str): Path { path.from_text(value).value_or(path.current()) }
async slow(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  command = Command(p("/bin/sh"), ["-c", "printf ready > 'ROOT/started'; while [ ! -f 'ROOT/release' ]; do /bin/sleep 0.01; done; printf ok"],
    "".encode_utf8(), path.current(), ExactEnvironment([]), time.seconds(10), 100)
  match await process.run(command) {
    Error(problem) => Error(problem.code),
    Ok(output) => { response.send_bytes(output.stdout); Ok(()) }
  }
}
async unblock(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  match await files.write(p("ROOT/release"), "go".encode_utf8()) {
    Error(problem) => Error(problem.code),
    Ok(_) => { response.send("ok"); Ok(()) }
  }
}
async main(): Unit {
  args = await host.arguments()
  server = http.create_bound_server("127.0.0.1", args[0].parse_nat())
  server.get("/slow", @slow)
  server.get("/release", @unblock)
  match await server.listen() { Ok(_) => (), Error(_) => await host.exit(1) }
}
SOURCE
sed "s|ROOT|$PWD/$work|g" "$work/template.panack" > "$work/app/src/main.panack"
./bin/panack compile --project "$work/app/panack.json" -o "$work/server.bc" > /dev/null
for mode in source bytecode; do
  rm -f "$work/started" "$work/release"
  port=$("$client" port)
  if [ "$mode" = source ]; then
    ./bin/panack run --project "$work/app/panack.json" "$port" > "$work/stdout" 2> "$work/stderr" &
  else
    ./bin/panack run "$work/server.bc" "$port" > "$work/stdout" 2> "$work/stderr" &
  fi
  server_pid=$!
  "$client" "$port" process-progress "$PWD/$work/started"
  kill -TERM "$server_pid"; wait "$server_pid"; server_pid=
  test ! -s "$work/stderr"
done
printf '%s\n' 'HTTP subprocess handler waits while another handler releases it: source and bytecode passed.'
