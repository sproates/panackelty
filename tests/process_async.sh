#!/bin/sh
set -eu
work=$(mktemp -d tests/process-async.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else ./bin/panack "$@"; fi
}
cat > "$work/async.panack" <<'SOURCE'
import stdlib/process/async as process
import stdlib/process::{Command, ProcessEnvironment, ProcessEnvironment.ExactEnvironment,
  ProcessEnvironment.InheritEnvironment, ProcessTermination.Exited, ProcessTermination.Signalled}
import stdlib/console/async as console
import stdlib/path as path
import stdlib/time as time
pure expect(value: Bool): Unit { checked = [0][if value { 0 } else { 1 }]; () }
pure command(script: Str, environment: ProcessEnvironment, timeout: Duration, limit: Nat): Command {
  Command(path.from_text("/bin/sh").value_or(path.current()), ["-c", script], "input".encode_utf8(),
    path.current(), environment, timeout, limit)
}
pure output(result: Result[process.ProcessOutput,process.ProcessError]): process.ProcessOutput {
  match result { Ok(value) => value, Error(_) => { empty: [process.ProcessOutput] = []; empty[0] } }
}
pure failure(result: Result[process.ProcessOutput,process.ProcessError], code: Str): Unit {
  match result { Ok(_) => expect(false), Error(problem) => expect(problem.native.code == code) }
}
async main(): Unit {
  exact = ExactEnvironment([])
  first = output(await process.run(command("cat; printf error >&2; exit 7", exact, time.seconds(2), 100)))
  expect(first.termination == Exited(7))
  expect(first.stdout == "input".encode_utf8())
  expect(first.stderr == "error".encode_utf8())
  inherited = output(await process.run(command("printf '%s' \"$PANACK_PROCESS_VALUE\"", InheritEnvironment([]), time.seconds(2), 100)))
  expect(inherited.stdout == "snapshot".encode_utf8())
  isolated = output(await process.run(command("printf '%s' \"$PANACK_PROCESS_VALUE\"", exact, time.seconds(2), 100)))
  expect(isolated.stdout == "".encode_utf8())
  explicit = output(await process.run(command("printf '%s' \"$PANACK_PROCESS_VALUE\"", ExactEnvironment(["PANACK_PROCESS_VALUE=explicit"]), time.seconds(2), 100)))
  expect(explicit.stdout == "explicit".encode_utf8())
  signalled = output(await process.run(command("kill -TERM $$", exact, time.seconds(2), 100)))
  expect(signalled.termination == Signalled(15))
  failure(await process.run(command("exit 0", ExactEnvironment(["X=1", "X=2"]), time.seconds(2), 100)), "invalid_environment")
  failure(await process.run(command("exit 0", ExactEnvironment(["bad"]), time.seconds(2), 100)), "invalid_environment")
  failure(await process.run(command("exit 0", exact, time.seconds(-1), 100)), "negative_duration")
  failure(await process.run(command("exit 0", exact, time.seconds(0), 100)), "timeout")
  failure(await process.run(command("printf too-long", exact, time.seconds(2), 1)), "output_limit")
  failure(await process.run(command("while :; do :; done", exact, time.milliseconds(10), 100)), "timeout")
  missing = Command(path.from_text("/panack-no-such-program").value_or(path.current()), [], "".encode_utf8(),
    path.current(), exact, time.seconds(2), 100)
  failure(await process.run(missing), "launch_failed")
  finished = await console.write_line("process commands: ok")
  ()
}
SOURCE
sed 's|stdlib/process/async|stdlib/process|;s|stdlib/console/async|stdlib/console|;s/async main/main/;s/await //g' "$work/async.panack" > "$work/ordinary.panack"
export PANACK_PROCESS_VALUE=snapshot
printf 'process commands: ok\n' > "$work/expected"
for mode in ordinary async; do
  panack run "$work/$mode.panack" > "$work/output"
  cmp "$work/expected" "$work/output"
  panack compile "$work/$mode.panack" -o "$work/$mode.bc" > /dev/null
  ./bin/panack run "$work/$mode.bc" > "$work/output"
  cmp "$work/expected" "$work/output"
done
reject() {
  printf '%s\n' "$1" > "$work/invalid.panack"
  if panack check "$work/invalid.panack" > "$work/stdout" 2> "$work/stderr"; then
    echo 'unexpectedly accepted invalid process use' >&2; exit 1
  fi
  grep -F "$2" "$work/stderr" >/dev/null || { cat "$work/stderr" >&2; exit 1; }
}
reject 'import stdlib/process/async as process
async invoke(command: process.Command): Unit { result = process.run(command); () }
main(): Unit { () }' 'async call requires await'
reject 'import stdlib/process as process
pure invoke(command: process.Command): Unit { result = process.run(command); () }
main(): Unit { () }' 'pure function cannot call impure'
reject 'import stdlib/process/async as process
async main(): Unit { result = await process.run(1); () }' 'Command'
reject 'async main(): Unit { result = await process_run_async(); () }' 'removed global primitive'
printf '%s\n' 'Command process API: ordinary/async source and bytecode, environments, termination, bounds and effects passed.'
