#!/bin/sh
set -eu
work=$(mktemp -d tests/continuous-service.XXXXXX)
server_pid=
cleanup() {
    if [ -n "$server_pid" ]; then kill -KILL "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup EXIT HUP INT TERM
# Bound successful shutdown and startup-error paths independently of the server.
wait_for_exit() {
    remaining=100
    while kill -0 "$server_pid" 2>/dev/null; do
        if [ "$remaining" = 0 ]; then
            printf '%s\n' 'continuous service failed to exit within five seconds' >&2
            cat "$work/stderr" >&2
            exit 1
        fi
        remaining=$((remaining - 1))
        sleep 0.05
    done
    wait "$server_pid"
}
http_client=${PANACK_HTTP_CLIENT:?}
mkdir -p "$work/app/src"
version=$(sed -n '1p' VERSION)
cat > "$work/app/panack.json" <<MANIFEST
{"schema":1,"package":"tests.continuous_service","version":"0.1.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
MANIFEST
cat > "$work/template.panack" <<'SOURCE'
import stdlib/async_host as host
import stdlib/text as text
import http/server as server
import http/service as service

async ready(endpoint: service.TcpServiceReady): Result[Unit,Str] {
  arguments = await host.arguments()
  if arguments[0] == "fail-ready" {
    Error("readiness rejected")
  } else {
    port = endpoint.port
    match await host.stdout("ready ${port}") {
      Ok(done) => Ok(()),
      Error(problem) => Error("readiness output failed")
    }
  }
}

async respond(configuration: Nat, request: server.Request): Result[server.Response,server.HttpError] {
  match await host.stderr("request handled") {
    Ok(done) => Ok(server.Response(200, [], (if configuration == 42 { "ok" } else { "bad" })->encode_utf8())),
    Error(problem) => Error(service.TransportFailure("diagnostic output failed"))
  }
}

async worker(context: server.ConnectionContext[Nat]): Result[Bytes,Str] {
  await server.serve_connection_with_context[Nat](context)
}

async main(): Unit {
  arguments = await host.arguments()
  mode = arguments[0]
  port: Nat = if mode == "invalid" { 0 } else { PORT }
  context: server.ConnectionContext[Nat] = server.ConnectionContext(42, @respond, 4096, 4096)
  report = await service.serve[Nat]("127.0.0.1", port, @worker, context,
    service.ServiceLimits(2, 4096, 4096, 3000, 1000), @ready)
  match report {
    Error(problem) => { ignored = await host.stderr("service failed"); await host.exit(7) },
    Ok(summary) => {
      accepted = summary.accepted
      succeeded = summary.succeeded
      ignored = await host.stdout("stopped ${accepted} ${succeeded}")
      ()
    }
  }
  ()
}
SOURCE
port=$("$http_client" port)
sed "s/PORT/$port/g" "$work/template.panack" > "$work/app/src/main.panack"
./bin/panack compile --project "$work/app/panack.json" -o "$work/service.bc" > /dev/null
for mode in source bytecode interrupt; do
    if [ "$mode" = source ]; then
        ./bin/panack run --project "$work/app/panack.json" serve > "$work/stdout" 2> "$work/stderr" &
    else
        ./bin/panack run "$work/service.bc" serve > "$work/stdout" 2> "$work/stderr" &
    fi
    server_pid=$!
    "$http_client" "$port" concurrent || { cat "$work/stderr"; exit 1; }
    if [ "$mode" = interrupt ]; then
        kill -INT "$server_pid"
    else
        kill -TERM "$server_pid"
    fi
    wait_for_exit || { cat "$work/stderr"; exit 1; }
    server_pid=
    printf 'ready %s\nstopped 2 2\n' "$port" > "$work/expected"
    cmp "$work/stdout" "$work/expected"
    printf 'request handled\nrequest handled\n' > "$work/expected"
    cmp "$work/stderr" "$work/expected"
done
for mode in invalid fail-ready; do
    status=0
    ./bin/panack run "$work/service.bc" "$mode" > "$work/stdout" 2> "$work/stderr" &
    server_pid=$!
    wait_for_exit || status=$?
    server_pid=
    test "$status" = 7
    test ! -s "$work/stdout"
    printf 'service failed\n' > "$work/expected"
    cmp "$work/stderr" "$work/expected"
done
printf '%s\n' 'continuous configured HTTP service source and bytecode: ok'
