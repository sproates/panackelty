#!/bin/sh
set -eu
work=$(mktemp -d tests/tcp-canonical.XXXXXX)
server_pid=
cleanup() { if [ -n "$server_pid" ]; then kill -KILL "$server_pid" 2>/dev/null || :; wait "$server_pid" 2>/dev/null || :; fi; rm -rf "$work"; }
trap cleanup 0
trap 'exit 1' HUP INT TERM
command=${PANACK_TEST_COMMAND:-./bin/panack}
client=${PANACK_TCP_CLIENT:?}
peer=${PANACK_TCP_SERVER:?}
wait_server() {
  attempts=0
  while kill -0 "$server_pid" 2>/dev/null; do
    attempts=$((attempts + 1)); [ "$attempts" -lt 100 ] || { cat "$work/stderr"; exit 1; }
    sleep 0.05
  done
  wait "$server_pid" || { cat "$work/stderr"; exit 1; }
  server_pid=
}
cat > "$work/limits.panack" <<'SOURCE'
import stdlib/network/tcp/async as tcp
import stdlib/time as time
import stdlib/bytes as byte_buffer
pure expect(ok: Bool): Unit { n = [0][if ok { 0 } else { 1 }]; () }
async reply(data: Bytes): Result[Bytes,Str] { Ok(data) }
async worker(limit: Nat): Result[Bytes,Str] { await tcp.read_chunk(limit) }
async ready(info: tcp.TcpServiceReady): Result[Unit,Str] { Ok(()) }
async main(): Unit {
  expect(time.milliseconds(2).as_nanoseconds() == time.milliseconds(2).ticks())
  expect(time.nanoseconds(-1).is_less_than(time.seconds(0)))
  expect(!time.seconds(1).is_less_than(time.seconds(1)))
  for invalid in [time.nanoseconds(-1), time.seconds(0), time.nanoseconds(1), time.nanoseconds(1000001), time.seconds(61)] {
    match await tcp.exchange("invalid", 0, byte_buffer.empty(), 1, invalid) {
      Error(problem) => expect(problem.starts_with_at("TCP timeout must be",0)), Ok(_) => expect(false)
    }
    limits = tcp.TcpServerLimits(1,1,1,1,invalid,time.seconds(1),time.seconds(0))
    match await tcp.serve_bounded("invalid",0,@reply,limits) {
      Error(problem) => expect(problem.starts_with_at("TCP client_timeout must be",0)), Ok(_) => expect(false)
    }
    limits2 = tcp.TcpServerLimits(1,1,1,1,time.seconds(1),invalid,time.seconds(0))
    match await tcp.serve_stream("invalid",0,@worker,1,limits2) {
      Error(problem) => expect(problem.starts_with_at("TCP admission_timeout must be",0)), Ok(_) => expect(false)
    }
  }
  for valid in [time.milliseconds(1), time.seconds(60)] {
    result = await tcp.exchange("invalid",1,byte_buffer.empty(),1,valid)
    expect(result == Error("invalid IPv4 address"))
    limits = tcp.TcpServerLimits(1,1,1,1,valid,valid,time.seconds(0))
    report = await tcp.serve_bounded("invalid",1,@reply,limits)
    expect(report == Error("invalid IPv4 address"))
  }
  for invalid in [time.nanoseconds(-1), time.nanoseconds(1), time.seconds(61)] {
    limits = tcp.TcpServiceLimits(1,1,1,time.seconds(1),invalid)
    match await tcp.serve("invalid",0,@worker,1,limits,@ready) {
      Error(problem) => expect(problem.starts_with_at("TCP drain must be",0)), Ok(_) => expect(false)
    }
  }
  ()
}
SOURCE
"$command" run "$work/limits.panack" > "$work/source.out"
: > "$work/expected"
cmp "$work/source.out" "$work/expected"
"$command" compile "$work/limits.panack" -o "$work/limits.bc" > /dev/null
"$command" run "$work/limits.bc" > "$work/bytecode.out"
cmp "$work/bytecode.out" "$work/expected"
# Existing independent echo peer serves one source and one saved-bytecode client.
"$peer" echo > "$work/port" &
server_pid=$!
attempts=0
while [ ! -s "$work/port" ]; do attempts=$((attempts + 1)); [ "$attempts" -lt 100 ]; sleep 0.05; done
port=$(cat "$work/port")
cat > "$work/exchange.panack" <<SOURCE
import stdlib/network/tcp/async as tcp
import stdlib/time as time
async main(): Unit {
  mut request = "hello".encode_utf8().push(0)
  for i in 0..12 { request = request.join(request) }
  result = await tcp.exchange("127.0.0.1",$port,request,24576,time.seconds(2))
  expect(result == Ok(request))
}
pure expect(ok: Bool): Unit { n = [0][if ok { 0 } else { 1 }]; () }
SOURCE
"$command" run "$work/exchange.panack"
"$command" compile "$work/exchange.panack" -o "$work/exchange.bc" > /dev/null
"$command" run "$work/exchange.bc"
wait_server
for mode in bounded stream continuous; do
  port=$("$client" port)
  call='result = await tcp.serve_bounded("127.0.0.1", PORT, @reply, tcp.TcpServerLimits(2,2,32768,32768,time.seconds(3),time.seconds(10),time.seconds(1))); match result { Ok(reports) => expect(reports == [Ok(()),Ok(())]), Error(_) => expect(false) }'
  client_mode=echo
  if [ "$mode" = stream ]; then
    call='result = await tcp.serve_stream("127.0.0.1",PORT,@worker,64,tcp.TcpServerLimits(1,1,256,256,time.seconds(3),time.seconds(10),time.seconds(1))); match result { Ok(reports) => expect(reports == [Ok(())]), Error(_) => expect(false) }'
    client_mode=stream
  elif [ "$mode" = continuous ]; then
    call='result = await tcp.serve("127.0.0.1",PORT,@worker,64,tcp.TcpServiceLimits(1,256,256,time.seconds(3),time.seconds(1)),@ready); match result { Ok(summary) => expect(summary.accepted == 1 && summary.succeeded == 1), Error(_) => expect(false) }'
    client_mode=stream
  fi
  cat > "$work/template.panack" <<SOURCE
import stdlib/network/tcp/async as tcp
import stdlib/time as time
pure expect(ok: Bool): Unit { n = [0][if ok { 0 } else { 1 }]; () }
async reply(data: Bytes): Result[Bytes,Str] { Ok(data) }
async worker(limit: Nat): Result[Bytes,Str] { await tcp.read_chunk(limit) }
async ready(info: tcp.TcpServiceReady): Result[Unit,Str] { expect(info.port == PORT); Ok(()) }
async main(): Unit { $call; () }
SOURCE
  sed "s/PORT/$port/g" "$work/template.panack" > "$work/server.panack"
  "$command" compile "$work/server.panack" -o "$work/server.bc" > /dev/null
  for extension in panack bc; do
    "$command" run "$work/server.$extension" > "$work/stdout" 2> "$work/stderr" &
    server_pid=$!
    "$client" "$port" "$client_mode" || { cat "$work/stderr"; exit 1; }
    if [ "$mode" = continuous ]; then kill -TERM "$server_pid"; fi
    wait_server
    test ! -s "$work/stdout" && test ! -s "$work/stderr"
  done
done
cat > "$work/wrong-timeout.panack" <<'SOURCE'
import stdlib/network/tcp/async as tcp
import stdlib/bytes as byte_buffer
async main(): Unit {
  result = await tcp.exchange("127.0.0.1",80,byte_buffer.empty(),16,1000)
  ()
}
SOURCE
if "$command" check "$work/wrong-timeout.panack" > "$work/type-error" 2>&1; then
  echo 'raw millisecond count accepted as Duration' >&2; exit 1
fi
grep -q Duration "$work/type-error"
printf '%s\n' 'Canonical TCP exchange, finite/streaming/continuous servers and duration validation passed in source/bytecode modes.'
