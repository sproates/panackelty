#!/bin/sh
set -eu
work=$(mktemp -d tests/stdlib-support.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else "${PANACK_TEST_COMMAND:-./bin/panack}" "$@"; fi
}
cat > "$work/async.panack" <<'SOURCE'
import stdlib/testing/reporting/async as testing
import stdlib/testing/filesystem/async as workspaces
import stdlib/testing/process/async as commands
import stdlib/testing::{TestResult, TestOutcome.TestPassed, TestOutcome.TestFailed}
import stdlib/filesystem/async as files
import stdlib/process/async as process
import stdlib/path as path
import stdlib/time as time
pure expect(value: Bool): Unit { check = [0][if value { 0 } else { 1 }]; () }
pure p(text: Str): Path { path.from_text(text).value_or(path.current()) }
pure passed(result: TestResult): Bool {
  match result.outcome { TestPassed() => true, TestFailed(_) => false }
}
pure count_result(result: Result[Nat,testing.ConsoleError]): Nat {
  match result { Ok(count) => count, Error(_) => { expect(false); 0 } }
}
async main(): Unit {
  args = await process.arguments()
  workspace = match await workspaces.create_workspace(p(args[0])) {
    Ok(value) => value, Error(_) => { expect(false); workspaces.TestWorkspace(path.current()) }
  }
  a = workspace.root.join(p("a")).value_or(path.current())
  b = workspace.root.join(p("b")).value_or(path.current())
  file = workspace.root.join(p("file")).value_or(path.current())
  made_b = await files.create_directory(b)
  made_a = await files.create_directory(a)
  wrote = await files.write(file, "sentinel".encode_utf8())
  fixtures = await workspaces.discover_fixtures(workspace.root)
  expect(fixtures == Ok([a,b]))
  refused = await workspaces.remove_empty(workspace)
  match refused { Ok(_) => expect(false), Error(problem) => expect(problem.code == "not_empty") }
  cmd = commands.TestCommand(p("/bin/sh"), ["-c", "printf hello; printf error >&2; exit 7"],
    "".encode_utf8(), path.current(), [], time.seconds(2), 100)
  expected = commands.ProcessOutput(7, 0, "hello".encode_utf8(), "error".encode_utf8())
  accepted = await commands.run("command", cmd, expected)
  expect(passed(accepted))
  wrong = commands.ProcessOutput(0, 0, "hello".encode_utf8(), "error".encode_utf8())
  rejected = await commands.run("mismatch", cmd, wrong)
  expect(!passed(rejected))
  captured = await commands.capture_report("capture", cmd, expected, Some(file))
  expect(passed(captured))
  contents = await files.read(file, 100)
  expect(contents == Ok("hello".encode_utf8()))
  reset = await files.write(file, "sentinel".encode_utf8())
  not_captured = await commands.capture_report("mismatch", cmd, wrong, Some(file))
  expect(!passed(not_captured))
  unchanged = await files.read(file, 100)
  expect(unchanged == Ok("sentinel".encode_utf8()))
  bad_destination = await commands.capture_report("write failure", cmd, expected, Some(a))
  expect(!passed(bad_destination))
  missing = commands.TestCommand(p("/panack-missing-command"), [], "".encode_utf8(), path.current(), [], time.seconds(2), 100)
  launch_error = await commands.expect_command_error("missing", missing, "launch_failed")
  expect(passed(launch_error))
  unexpected_success = await commands.expect_command_error("unexpected", cmd, "launch_failed")
  expect(!passed(unexpected_success))
  removed_file = await files.remove_file(file)
  removed_a = await files.remove_directory(a)
  removed_b = await files.remove_directory(b)
  removed_workspace = await workspaces.remove_empty(workspace)
  expect(removed_workspace == Ok(()))
  discovery_error = await workspaces.discover_fixtures(workspace.root)
  match discovery_error { Ok(_) => expect(false), Error(problem) => expect(problem.code == "not_found") }
  results = [testing.expect("first", true, ""), testing.expect("second", false, "expected failure")]
  failures = count_result(await testing.report(results))
  expect(failures == 1)
}
SOURCE
sed 's|stdlib/testing/reporting/async|stdlib/testing/reporting|;s|stdlib/testing/filesystem/async|stdlib/testing/filesystem|;s|stdlib/testing/process/async|stdlib/testing/process|;s|stdlib/filesystem/async|stdlib/filesystem|;s|stdlib/process/async|stdlib/process|;s/async main/main/;s/await //g' "$work/async.panack" > "$work/ordinary.panack"
printf 'PASS first\nFAIL second: expected failure\ntests: 2, failures: 1\n' > "$work/expected"
for mode in ordinary async; do
  panack run "$work/$mode.panack" "$work" > "$work/output"
  cmp "$work/expected" "$work/output"
  panack compile "$work/$mode.panack" -o "$work/$mode.bc" > /dev/null
  panack disasm "$work/$mode.bc" > "$work/disassembly"
  panack run "$work/$mode.bc" "$work" > "$work/output"
  cmp "$work/expected" "$work/output"
done
cat > "$work/closed.panack" <<'SOURCE'
import stdlib/testing/reporting/async as testing
import stdlib/process/async as process
async main(): Unit {
  result = await testing.report([testing.expect("write", true, "")])
  match result { Error(_) => (), Ok(_) => await process.exit(1) }
}
SOURCE
sed 's|stdlib/testing/reporting/async|stdlib/testing/reporting|;s|stdlib/process/async|stdlib/process|;s/async main/main/;s/await //g' "$work/closed.panack" > "$work/closed-ordinary.panack"
for closed in closed closed-ordinary; do
  panack compile "$work/$closed.panack" -o "$work/$closed.bc" > /dev/null
  panack run "$work/$closed.panack" >&-
  panack run "$work/$closed.bc" >&-
done
cat > "$work/child.panack" <<'SOURCE'
import stdlib/host as host
main(): Void { args = host.arguments(); print(args[0]) }
SOURCE
panack compile "$work/child.panack" -o "$work/child.bc" > /dev/null
cat > "$work/nested.panack" <<'SOURCE'
import stdlib/runtime/bytecode as runtime
import stdlib/host as host
import stdlib/filesystem as files
import stdlib/path as path
main(): Unit {
  args = host.arguments()
  file = path.from_text(args[0]).value_or(path.current())
  match files.read(file, 1048576) { Ok(data) => { runtime.run(data, ["nested: ok"]); () }, Error(_) => { host.exit(1); () } }
}
SOURCE
printf 'nested: ok\n' > "$work/expected-nested"
panack run "$work/nested.panack" "$work/child.bc" > "$work/output"
cmp "$work/expected-nested" "$work/output"
panack compile "$work/nested.panack" -o "$work/nested.bc" > /dev/null
panack run "$work/nested.bc" "$work/child.bc" > "$work/output"
cmp "$work/expected-nested" "$work/output"
reject() {
  printf '%s\n' "$1" > "$work/invalid.panack"
  if panack check "$work/invalid.panack" > "$work/stdout" 2> "$work/stderr"; then
    echo 'invalid support API use accepted' >&2; exit 1
  fi
  grep -F "$2" "$work/stderr" > /dev/null || { cat "$work/stderr" >&2; exit 1; }
}
reject 'import stdlib/runtime/bytecode as runtime
async main(): Unit { runtime.run("".encode_utf8(), []); () }' 'async function cannot call blocking function'
reject 'import stdlib/testing/filesystem/async as files
import stdlib/path as path
async main(): Unit { workspace = files.create_workspace(path.current()); () }' 'async call requires await'
printf '%s\n' 'Stdlib testing and nested-runtime modules: ordinary/async, source/bytecode, failures and effects passed.'
