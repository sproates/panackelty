#!/bin/sh
set -eu
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-server.XXXXXX")
server_pid=
cleanup() {
    if [ -n "$server_pid" ]; then kill "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup 0
trap 'exit 1' HUP INT TERM
client=${PANACK_TCP_CLIENT:?}
for mode in echo empty request-limit response-limit handler-error busy nested; do
    port=$("$client" port)
    request_limit=24576
    response_limit=24576
    handler='Ok(request)'
    client_timeout=3000
    first=true
    second=true
    case "$mode" in
        nested) handler='nested = await tcp_serve("127.0.0.1", 9000, @reply, TcpServerLimits(1, 1, 8, 8, 100, 100, 100)); Ok(request)' ;;
        empty) request_limit=0; response_limit=0 ;;
        busy) handler='if byte_len(request) > 0 { while true {} }; Ok(request)'; client_timeout=100; first=false ;;
        request-limit) request_limit=0; second=false ;;
        response-limit) response_limit=0; second=false ;;
        handler-error) handler='Error("expected handler error")'; first=false; second=false ;;
    esac
    cat > "$work/main.panack" <<SOURCE
import stdlib/tcp
pure expect(value: Bool): Unit { checked = [0][if value { 0 } else { 1 }]; () }
async reply(request: Bytes): Result[Bytes,Str] { $handler }
async main(): Unit {
  limits = TcpServerLimits(2, 2, $request_limit, $response_limit, $client_timeout, 10000, 3000)
  result = await tcp_serve("127.0.0.1", $port, @reply, limits)
  match result {
    Ok(reports) => {
      expect(reports.len() == 2)
      first_ok = match reports[0] { Ok(done) => true, Error(problem) => false }
      second_ok = match reports[1] { Ok(done) => true, Error(problem) => false }
      expect(first_ok == $first)
      expect(second_ok == $second)
    },
    Error(problem) => expect(false)
  }
  ()
}
SOURCE
    ./panack check "$work/main.panack" > /dev/null
    ./panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
    for extension in panack bc; do
        ./panack run "$work/main.$extension" > "$work/stdout" 2> "$work/stderr" &
        server_pid=$!
        "$client" "$port" "$mode"
        if [ "$mode" = nested ]; then
            if wait "$server_pid"; then echo 'nested listening unexpectedly succeeded' >&2; exit 1; fi
            grep -F 'TCP server is unavailable in this execution' "$work/stderr" > /dev/null
        else
            wait "$server_pid" || { cat "$work/stderr"; exit 1; }
            test ! -s "$work/stdout" && test ! -s "$work/stderr"
        fi
        server_pid=
    done
done
# Run the documented finite server itself against an independent peer.
port=$("$client" port)
sed "s/9000/$port/g" examples/network/tcp_serve.panack > "$work/example.panack"
./panack run "$work/example.panack" > "$work/stdout" 2> "$work/stderr" &
server_pid=$!
"$client" "$port" example
wait "$server_pid" || { cat "$work/stderr"; exit 1; }
server_pid=
test ! -s "$work/stdout" && test ! -s "$work/stderr"
# Source-level effect and handler typing failures, before any network operation.
for mode in bare ordinary handler limits; do
    call='await tcp_serve("127.0.0.1", 9000, @reply, TcpServerLimits(1, 1, 8, 8, 100, 100, 100))'
    main='async main(): Unit'
    handler='async reply(request: Bytes): Result[Bytes,Str] { Ok(request) }'
    case "$mode" in
        bare) call='tcp_serve("127.0.0.1", 9000, @reply, TcpServerLimits(1, 1, 8, 8, 100, 100, 100))'; diagnostic='async call requires await' ;;
        ordinary) main='main(): Unit'; diagnostic='await requires an async function' ;;
        handler) handler='pure reply(request: Bytes): Result[Bytes,Str] { Ok(request) }'; diagnostic='AsyncFn' ;;
        limits) call='await tcp_serve("127.0.0.1", 9000, @reply, 42)'; diagnostic='TcpServerLimits' ;;
    esac
    printf 'import stdlib/tcp\n%s\n%s { result = %s; () }\n' "$handler" "$main" "$call" > "$work/invalid.panack"
    if ./panack check "$work/invalid.panack" > "$work/stdout" 2> "$work/stderr"; then
        echo "TCP server negative unexpectedly passed: $mode" >&2; exit 1
    fi
    grep -F "$diagnostic" "$work/stderr" > /dev/null || { cat "$work/stderr"; exit 1; }
done
printf 'async TCP server source and bytecode: ok\n'
