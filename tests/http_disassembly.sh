#!/bin/sh
set -eu
work=$(mktemp -d tests/http-disassembly.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
command=${PANACK_TEST_COMMAND:-./bin/panack}
mkdir -p "$work/app/src"
version=$(cat VERSION)
printf '{"schema":1,"package":"tests.http_disassembly","version":"0.1.0","language":"modules-v1","toolchain":"%s","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}\n' "$version" > "$work/app/panack.json"
"$command" deps lock --project "$work/app/panack.json" > /dev/null
for client in client client/async; do
  cat > "$work/app/src/main.panack" <<SOURCE
import http/$client as http
pure expect(ok: Bool): Unit { check = [0][if ok { 0 } else { 1 }]; () }
async main(): Unit {
  result = await http.get("http://user@localhost/")
  expect(match result { Error(_) => true, Ok(_) => false })
}
SOURCE
  "$command" run --project "$work/app/panack.json"
  "$command" compile --project "$work/app/panack.json" -o "$work/client.bc" > /dev/null
  "$command" check "$work/client.bc" > /dev/null
  "$command" run "$work/client.bc"
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_BOOTSTRAP_ROOT="$PWD" ./panack-vm run "$PANACK_TEST_COMPILER" disasm "$work/client.bc" > "$work/disassembly"
  else "$command" disasm "$work/client.bc" > "$work/disassembly"; fi
  grep -q http_body "$work/disassembly"
done
cat > "$work/app/src/main.panack" <<'SOURCE'
import http/server as http
async handler(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("ready")
  Ok(())
}
async main(): Unit {
  server = http.create_server(8080)
  server.get("/",@handler)
  ()
}
SOURCE
"$command" compile --project "$work/app/panack.json" -o "$work/server.bc" > /dev/null
"$command" check "$work/server.bc" > /dev/null
if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
  PANACKELTY_BOOTSTRAP_ROOT="$PWD" ./panack-vm run "$PANACK_TEST_COMPILER" disasm "$work/server.bc" > "$work/disassembly"
else "$command" disasm "$work/server.bc" > "$work/disassembly"; fi
grep -q http_writer_commit "$work/disassembly"
printf '%s\n' 'HTTP legacy/diagnostic clients and routed server pass native verification and self-hosted disassembly.'
