#!/bin/sh
set -eu
work=$(mktemp -d tests/domain-errors.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else "${PANACK_TEST_COMMAND:-./bin/panack}" "$@"; fi
}
cat > "$work/template.panack" <<'SOURCE'
import stdlib/consoleSUFFIX as console
import stdlib/timeSUFFIX as time
import stdlib/processSUFFIX as process
import stdlib/console/errors as console_errors
import stdlib/host::{HostError}
import stdlib/console/errors::{ConsoleErrorKind.InvalidReadLimit, ConsoleErrorKind.OutputTooLarge,
  ConsoleErrorKind.OtherFailure, ConsoleOperation.Read}
import stdlib/time/errors::{SleepErrorKind.NegativeDuration, SleepErrorKind.DurationTooLarge}
import stdlib/process/errors::{ProcessErrorKind.InvalidTimeout, ProcessErrorKind.TimedOut,
  ProcessErrorKind.LaunchFailed, ProcessErrorKind.OutputLimitExceeded, ProcessErrorKind.InvalidEnvironment}
import stdlib/process::{Command, ProcessEnvironment.ExactEnvironment, ProcessTermination.Exited}
import stdlib/path as path
pure expect(ok: Bool): Unit { check = [0][if ok { 0 } else { 1 }]; () }
pure p(text: Str): Path { path.from_text(text).value_or(path.current()) }
pure console_error[T](result: Result[T,console.ConsoleError]): console.ConsoleError {
  match result { Error(problem) => problem, Ok(_) => { missing: [console.ConsoleError] = []; missing[0] } }
}
pure sleep_error(result: Result[Unit,time.SleepError]): time.SleepError {
  match result { Error(problem) => problem, Ok(_) => { missing: [time.SleepError] = []; missing[0] } }
}
pure process_error(result: Result[process.ProcessOutput,process.ProcessError]): process.ProcessError {
  match result { Error(problem) => problem, Ok(_) => { missing: [process.ProcessError] = []; missing[0] } }
}
pure command(script: Str, timeout: Duration, limit: Nat): Command {
  Command(p("/bin/sh"), ["-c",script], "sensitive stdin".encode_utf8(), p("/"), ExactEnvironment([]), timeout, limit)
}
EFFECT main(): Unit {
  read = console_error(WAIT console.read(0))
  expect(read.kind == InvalidReadLimit())
  expect(read.operation == Read(0))
  expect(read.native == HostError("console_read","out_of_range",0))
  expect(read.render() == "Cannot read standard input (requested limit: 0 bytes): the read limit must be from 1 through 65536 bytes. Choose a limit within that range.")
  negative = sleep_error(WAIT time.sleep(time.nanoseconds(-1)))
  expect(negative.kind == NegativeDuration())
  expect(negative.duration == time.nanoseconds(-1))
  expect(negative.native.code == "negative_duration")
  expect(negative.render() == "Cannot sleep for -1 nanoseconds: the duration is negative. Use zero or a positive duration.")
  large = sleep_error(WAIT time.sleep(time.seconds(31536001)))
  expect(large.kind == DurationTooLarge())
  expect(WAIT time.sleep(time.nanoseconds(0)) == Ok(()))
  mut text = "x"
  for unused in [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14] { text = text + text }
  output = console_error(WAIT console.write(text))
  expect(output.kind == OutputTooLarge())
  expect(output.render() == "Cannot write standard output: the message exceeds 16384 UTF-8 bytes before the optional newline. Write smaller chunks.")
  unknown = console_error(console_errors.convert[Unit](Error(HostError("future_native_operation","future_code",87)), Read(4)))
  expect(unknown.kind == OtherFailure())
  expect(unknown.native == HostError("future_native_operation","future_code",87))
  expect(unknown.render() == "Cannot read standard input (requested limit: 4 bytes): the host returned an unrecognised failure. The native field preserves its details. Host error number: 87.")
  zero = process_error(WAIT process.run(command("exit 0",time.seconds(0),100)))
  expect(zero.kind == InvalidTimeout())
  expect(zero.render() == "Cannot run process '/bin/sh' with working directory '/': the timeout must be positive and within the supported range. Choose a timeout greater than zero and no longer than 365 days.")
  expect(process_error(WAIT process.run(command("exit 0",time.seconds(-1),100))).kind == InvalidTimeout())
  expect(process_error(WAIT process.run(command("exit 0",time.seconds(31536001),100))).kind == InvalidTimeout())
  timed = process_error(WAIT process.run(command("while :; do :; done",time.milliseconds(10),100)))
  expect(timed.kind == TimedOut())
  expect(timed.timeout == time.milliseconds(10))
  expect(timed.render() == "Cannot run process '/bin/sh' with working directory '/': the process did not complete within the requested 10000000 nanoseconds. The process group was stopped and partial output is not returned.")
  limited = process_error(WAIT process.run(command("printf excessive",time.seconds(2),1)))
  expect(limited.kind == OutputLimitExceeded())
  expect(limited.output_limit == 1)
  expect(limited.render() == "Cannot run process '/bin/sh' with working directory '/': combined standard output and standard error exceeded the 1-byte limit. The process group was stopped and partial output is not returned.")
  missing = Command(p("/panack-no-such-executable"), ["secret-argument"], "secret-input".encode_utf8(),
    p("/"), ExactEnvironment(["SECRET=secret-environment"]), time.seconds(2),100)
  failed = process_error(WAIT process.run(missing))
  expect(failed.kind == LaunchFailed())
  expect(failed.executable == missing.executable && failed.working_directory == missing.working_directory)
  expect(failed.native.code == "launch_failed" && failed.native.native_code > 0)
  host_number = failed.native.native_code
  expect(failed.render() == "Cannot run process '/panack-no-such-executable' with working directory '/': the host could not start the process. Check the executable, working directory and permissions. Host error number: ${host_number}.")
  invalid_env = Command(p("/bin/sh"), [], "".encode_utf8(), p("/"), ExactEnvironment(["X=1","X=2"]),time.seconds(2),100)
  expect(process_error(WAIT process.run(invalid_env)).kind == InvalidEnvironment())
  match WAIT process.run(command("exit 7",time.seconds(2),100)) {
    Error(_) => expect(false), Ok(value) => expect(value.termination == Exited(7))
  }
  said = WAIT console.write_line("domain errors passed")
  expect(said == Ok(()))
}
SOURCE
printf 'domain errors passed\n' > "$work/expected"
for mode in ordinary async; do
  if [ "$mode" = async ]; then suffix=/async; effect=async; wait=await; else suffix=; effect=; wait=; fi
  sed -e "s@SUFFIX@$suffix@g" -e "s/EFFECT/$effect/g" -e "s/WAIT/$wait/g" "$work/template.panack" > "$work/main.panack"
  panack run "$work/main.panack" > "$work/output"
  cmp "$work/expected" "$work/output"
  panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
  panack run "$work/main.bc" > "$work/output"
  cmp "$work/expected" "$work/output"
  panack disasm "$work/main.bc" > /dev/null
done
printf '%s\n' 'Domain errors: typed categories, context, exact rendering, native detail, successful results and ordinary/async source/bytecode passed.'
