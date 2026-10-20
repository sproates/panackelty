#!/bin/sh
set -eu
work=$(mktemp -d tests/http-package.XXXXXX)
server_pid=
cleanup() {
    if [ -n "$server_pid" ]; then kill -KILL "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup EXIT HUP INT TERM
http_client=${PANACK_HTTP_CLIENT:?}
mkdir -p "$work/app/src"
version=$(sed -n '1p' VERSION)
cat > "$work/app/panack.json" <<MANIFEST
{"schema":1,"package":"tests.http_app","version":"0.1.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
MANIFEST
for probe in parser_test routed_test; do
    ./bin/panack run "packages/http/src/$probe.panack"
    ./bin/panack compile "packages/http/src/$probe.panack" -o "$work/$probe.bc" > /dev/null
    ./bin/panack run "$work/$probe.bc"
done
./bin/panack compile packages/http/src/writer_test.panack -o "$work/writer.bc" > /dev/null
"${PANACK_NATIVE_MODULE_TEST:?}" http-writer "$work/writer.bc"
port=$("$http_client" port)
./bin/panack run packages/http/src/routed_lifecycle_test.panack "$port"
./bin/panack compile packages/http/src/routed_lifecycle_test.panack -o "$work/lifecycle.bc" > /dev/null
./panack-vm run "$work/lifecycle.bc" "$port"
# Run the real routed fixture through the exported module and an external manifest.
sed 's/import project\/server as http/import http\/server as http/'     packages/http/src/routed_socket_test.panack > "$work/app/src/main.panack"
./bin/panack compile --project "$work/app/panack.json" -o "$work/routed.bc" > /dev/null
for mode in source bytecode; do
    port=$("$http_client" port)
    if [ "$mode" = source ]; then
        ./bin/panack run --project "$work/app/panack.json" "$port" > "$work/stdout" 2> "$work/stderr" &
    else
        ./panack-vm run "$work/routed.bc" "$port" > "$work/stdout" 2> "$work/stderr" &
    fi
    server_pid=$!
    "$http_client" "$port" routed
    "$http_client" "$port" routed-capacity
    if [ "$mode" = bytecode ]; then
        "$http_client" "$port" routed-limits
        "$http_client" "$port" routed-deadline
    fi
    kill -TERM "$server_pid"
    wait "$server_pid"
    server_pid=
    grep -q 'handler diagnostic only' "$work/stderr"
    grep -q 'index is out of bounds' "$work/stderr"
    grep -q 'division by zero' "$work/stderr"
done
# Exit from an active handler propagates the status and drains its first response.
port=$("$http_client" port)
./bin/panack run "$work/routed.bc" "$port" > "$work/stdout" 2> "$work/stderr" &
server_pid=$!
"$http_client" "$port" routed-exit
exit_status=0
wait "$server_pid" || exit_status=$?
server_pid=
test "$exit_status" = 7
# Run the checked-in ordinary and contextual examples, exactly as linked from
# their READMEs, from both source and saved bytecode. Only the port is supplied
# at runtime so these loopback checks do not depend on demonstration defaults.
for example in http_server http_contextual_server; do
    project="examples/network/$example/panack.json"
    ./bin/panack compile --project "$project" -o "$work/$example.bc" > /dev/null
    for mode in source bytecode; do
        port=$("$http_client" port)
        if [ "$mode" = source ]; then
            ./bin/panack run --project "$project" "$port" > "$work/stdout" 2> "$work/stderr" &
        else
            ./panack-vm run "$work/$example.bc" "$port" > "$work/stdout" 2> "$work/stderr" &
        fi
        server_pid=$!
        peer_mode=example-contextual
        if [ "$example" = http_server ]; then peer_mode=checked-in-ordinary; fi
        "$http_client" "$port" "$peer_mode"
        kill -TERM "$server_pid"
        wait "$server_pid"
        server_pid=
        test ! -s "$work/stderr"
    done
done
# Compile the exact agreed examples unchanged. Runtime copies change only the
# fixed demonstration port so tests do not depend on port 8080 being available.
for example in ordinary contextual; do
    number=1
    if [ "$example" = contextual ]; then number=3; fi
    awk -v heading="### $number." '
      index($0, heading) == 1 { section=1; next }
      section && /^```panack$/ { code=1; next }
      code && /^```$/ { exit }
      code { print }
    ' docs/design/routed-http-server/contract-snapshot.md > "$work/app/src/main.panack"
    ./bin/panack compile --project "$work/app/panack.json" -o "$work/exact-$example.bc" > /dev/null
    cp "$work/app/src/main.panack" "$work/exact.panack"
    port=$("$http_client" port)
    sed "s/(8080/($port/g" "$work/exact.panack" > "$work/app/src/main.panack"
    ./bin/panack compile --project "$work/app/panack.json" -o "$work/example.bc" > /dev/null
    for mode in source bytecode; do
        if [ "$mode" = source ]; then
            ./bin/panack run --project "$work/app/panack.json" > "$work/stdout" 2> "$work/stderr" &
        else
            ./bin/panack run "$work/example.bc" > "$work/stdout" 2> "$work/stderr" &
        fi
        server_pid=$!
        "$http_client" "$port" "example-$example"
        kill -TERM "$server_pid"
        wait "$server_pid"
        server_pid=
        test ! -s "$work/stderr"
    done
done
# Two independently retained contexts, two named handlers and all four methods.
cat > "$work/app/src/main.panack" <<'SOURCE'
import http/server as http
import stdlib/async_host as host
record Context { greeting: Str }
async home(context: Context, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send(context.greeting)
  Ok(())
}
async health(context: Context, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("health " + context.greeting)
  Ok(())
}
async start(port: Nat, greeting: Str): Unit {
  server = http.create_bound_contextual_server("127.0.0.1", port, Context(greeting))
  server.get("/", @home)
  server.post("/", @home)
  server.put("/", @home)
  server.delete("/", @home)
  server.get("/health", @health)
  match await server.listen() { Ok(done) => (), Error(problem) => { await host.exit(1) } }
}
async main(): Unit {
  arguments = await host.arguments()
  await start(arguments[0].parse_nat(), "one")
  await start(arguments[1].parse_nat(), "two")
}
SOURCE
./bin/panack compile --project "$work/app/panack.json" -o "$work/contexts.bc" > /dev/null
for mode in source bytecode; do
    first=$("$http_client" port)
    second=$("$http_client" port)
    while [ "$second" = "$first" ]; do second=$("$http_client" port); done
    if [ "$mode" = source ]; then
        ./bin/panack run --project "$work/app/panack.json" "$first" "$second" > "$work/stdout" 2> "$work/stderr" &
    else
        ./bin/panack run "$work/contexts.bc" "$first" "$second" > "$work/stdout" 2> "$work/stderr" &
    fi
    server_pid=$!
    "$http_client" "$first" contexts one
    "$http_client" "$second" contexts two
    "$http_client" "$first" contexts one
    kill -TERM "$server_pid"
    wait "$server_pid"
    server_pid=
    test ! -s "$work/stderr"
done
# A second listener controls shutdown of an admitted, body-waiting request.
# Its response is sent only after the awaited operation releases the first server.
cat > "$work/app/src/main.panack" <<'SOURCE'
import http/server as http
import stdlib/async_host as host
async hold(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("must not run without the complete body")
  Ok(())
}
async close(server: http.Server, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  match await server.close() { Ok(done) => {}, Error(problem) => {} }
  response.send("released")
  Ok(())
}
async begin_close(server: http.Server, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("closing")
  match await server.close() { Ok(done) => {}, Error(problem) => {} }
  Ok(())
}
async destroy(server: http.Server, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  match await server.destroy() { Ok(done) => {}, Error(problem) => {} }
  response.send("released")
  Ok(())
}
async main(): Unit {
  arguments = await host.arguments()
  server = http.create_bound_server("127.0.0.1", arguments[0].parse_nat())
  server.post("/hold", @hold)
  control = http.create_bound_contextual_server("127.0.0.1", arguments[1].parse_nat(), server)
  control.get("/close", @close)
  control.get("/begin-close", @begin_close)
  control.get("/destroy", @destroy)
  match await server.listen() { Ok(done) => (), Error(problem) => { await host.exit(1) } }
  match await control.listen() { Ok(done) => (), Error(problem) => { await host.exit(1) } }
}
SOURCE
./bin/panack compile --project "$work/app/panack.json" -o "$work/shutdown.bc" > /dev/null
for operation in close destroy escalate; do
    port=$("$http_client" port)
    control=$("$http_client" port)
    while [ "$control" = "$port" ]; do control=$("$http_client" port); done
    ./bin/panack run "$work/shutdown.bc" "$port" "$control" > "$work/stdout" 2> "$work/stderr" &
    server_pid=$!
    "$http_client" "$port" "$operation" "$control"
    kill -TERM "$server_pid"
    wait "$server_pid"
    server_pid=
done
cat > "$work/blocking-wrapper.panack" <<'SOURCE'
import stdlib/http_runtime as runtime
blocking(state: HttpResponseState): Unit {
  runtime.writer_status(state, 200)
  print("must remain blocking")
  ()
}
async rejected(state: HttpResponseState): Unit { blocking(state) }
main(): Unit { keep = @rejected; () }
SOURCE
if ./bin/panack check "$work/blocking-wrapper.panack" > "$work/rejected.stdout" 2> "$work/rejected.stderr"; then
    echo 'HTTP local mutation accidentally allowed blocking I/O from async' >&2
    exit 1
fi
grep -q 'async function cannot call blocking function' "$work/rejected.stderr"

# Concrete context and async effect mismatches must fail through public imports.
cat > "$work/app/src/main.panack" <<'SOURCE'
import http/server as http
async wrong(context: Str, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] { Ok(()) }
async main(): Unit {
  server = http.create_contextual_server(8000, 42)
  server.get("/", @wrong)
}
SOURCE
if ./bin/panack check --project "$work/app/panack.json" > "$work/rejected.stdout" 2> "$work/rejected.stderr"; then
    echo 'wrong HTTP context type was accepted' >&2; exit 1
fi
grep -q 'argument' "$work/rejected.stderr"
for signature in arity effect closure; do
    case "$signature" in
        arity) handler='async wrong(request: http.Request): Result[Unit,Str] { Ok(()) }'; reference='@wrong' ;;
        effect) handler='wrong(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] { Ok(()) }'; reference='@wrong' ;;
        closure) handler=''; reference='(request, response) => Ok(())' ;;
    esac
    cat > "$work/app/src/main.panack" <<SOURCE
import http/server as http
$handler
async main(): Unit {
  server = http.create_server(8000)
  server.get("/", $reference)
}
SOURCE
    if ./bin/panack check --project "$work/app/panack.json" > "$work/rejected.stdout" 2> "$work/rejected.stderr"; then
        echo "invalid HTTP callback accepted: $signature" >&2; exit 1
    fi
    test -s "$work/rejected.stderr"
done
for declaration in ServerLimits ConnectionContext ConnectionPolicy serialize_response serve; do
    printf 'import http/server::{%s}\nmain(): Unit {}\n' "$declaration" > "$work/app/src/main.panack"
    if ./bin/panack check --project "$work/app/panack.json" > "$work/rejected.stdout" 2> "$work/rejected.stderr"; then
        echo "obsolete HTTP server export accepted: $declaration" >&2; exit 1
    fi
done
printf 'import http/service as old\nmain(): Unit {}\n' > "$work/app/src/main.panack"
if ./bin/panack check --project "$work/app/panack.json" > "$work/rejected.stdout" 2> "$work/rejected.stderr"; then
    echo 'obsolete HTTP service module accepted' >&2; exit 1
fi
printf '%s\n' 'Routed HTTP public package, lifecycle, protocol and limit contracts: ok'
