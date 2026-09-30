#!/bin/sh
# Real source and saved-bytecode programs over independent loopback TCP peers.
set -eu
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-tcp.XXXXXX")
server_pid=
cleanup() {
    if [ -n "$server_pid" ]; then kill "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup 0
trap 'exit 1' HUP INT TERM
for mode in echo empty example limit stall refuse; do
    fixture=$mode
    [ "$mode" != empty ] || fixture=echo
    [ "$mode" != example ] || fixture=echo
    "${PANACK_TCP_SERVER:?}" "$fixture" > "$work/port" &
    server_pid=$!
    attempts=0
    while [ ! -s "$work/port" ]; do
        attempts=$((attempts + 1))
        [ "$attempts" -lt 100 ] || exit 1
        sleep 0.05
    done
    port=$(cat "$work/port")
    limit=100000
    timeout=2000
    case "$mode" in
        echo|example) limit=24576; expected='Ok(data) => expect(data == request), Error(problem) => expect(false)' ;;
        empty) limit=0; expected='Ok(data) => expect(data == request), Error(problem) => expect(false)' ;;
        limit) limit=0; expected='Ok(data) => expect(false), Error(problem) => expect(problem == "TCP response limit exceeded")' ;;
        stall) timeout=30; expected='Ok(data) => expect(false), Error(problem) => expect(problem == "TCP timed out")' ;;
        refuse) expected='Ok(data) => expect(false), Error(problem) => expect(problem == "TCP connect failed")' ;;
    esac
    cat > "$work/main.panack" <<SOURCE
pure expect(value: Bool): Unit { checked = [0][if value { 0 } else { 1 }]; () }
async exchange(request: Bytes): Result[Bytes,Str] {
  await tcp_exchange("127.0.0.1", $port, request, $limit, $timeout)
}
async main(): Unit {
  mut request: Bytes = byte_append(utf8_encode("hello"), 0)
  for i in 0..12 { request = bytes_concat(request, request) }
  operation: AsyncFn[Bytes,Result[Bytes,Str]] = @exchange
  match await operation.call(request) { $expected }
  ()
}
SOURCE
    if [ "$mode" = empty ]; then
        sed 's/byte_append(utf8_encode("hello"), 0)/bytes()/' "$work/main.panack" > "$work/empty.panack"
        mv "$work/empty.panack" "$work/main.panack"
    fi
    if [ "$mode" = example ]; then
        sed "s/9000/$port/g" examples/network/tcp_exchange.panack > "$work/main.panack"
    fi
    ./panack check "$work/main.panack" > "$work/check"
    printf 'ok\n' > "$work/expected"
    cmp "$work/expected" "$work/check"
    ./panack run "$work/main.panack" > "$work/stdout" 2> "$work/stderr" || { cat "$work/stderr"; exit 1; }
    test ! -s "$work/stdout" && test ! -s "$work/stderr"
    ./panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
    ./panack run "$work/main.bc" > "$work/stdout" 2> "$work/stderr" || { cat "$work/stderr"; exit 1; }
    test ! -s "$work/stdout" && test ! -s "$work/stderr"
    if [ "$mode" = echo ] || [ "$mode" = empty ] || [ "$mode" = example ]; then wait "$server_pid"; else kill "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi
    server_pid=
    rm "$work/port"
done
printf 'async TCP source and bytecode: ok\n'

# Invalid calls must be rejected without any network side effect.
for mode in bare ordinary pure type; do
    case "$mode" in
        bare) source='async main(): Unit { result = tcp_exchange("127.0.0.1", 80, bytes(), 10, 10); () }'; diagnostic='async call requires await' ;;
        ordinary) source='main(): Void { result = await tcp_exchange("127.0.0.1", 80, bytes(), 10, 10) }'; diagnostic='await requires an async function' ;;
        pure) source='pure main(): Unit { result = tcp_exchange("127.0.0.1", 80, bytes(), 10, 10); () }'; diagnostic='pure function cannot call impure function' ;;
        type) source='async main(): Unit { result = await tcp_exchange(42, 80, bytes(), 10, 10); () }'; diagnostic='expected Str' ;;
    esac
    printf '%s\n' "$source" > "$work/invalid.panack"
    if ./panack check "$work/invalid.panack" > "$work/stdout" 2> "$work/stderr"; then
        echo "TCP negative case unexpectedly passed: $mode" >&2; exit 1
    fi
    grep -F "$diagnostic" "$work/stderr" > /dev/null || { cat "$work/stderr"; exit 1; }
done
printf 'async TCP source rejection: ok\n'
