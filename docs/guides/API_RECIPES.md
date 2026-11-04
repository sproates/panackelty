# Standard-library API recipes

These complete programs demonstrate the APIs available on alpha.15 `next`.
Run commands from the checkout root after `make`, or from an extracted release
containing these examples. The host examples use POSIX tools including `/bin/sh` and `/bin/cat`;
they are not browser examples. Each program is checked from source and saved
bytecode against an exact transcript by the functional suite.

## Eight tasks to try

### Text and UTF-8

**[Shorten a display name](../../examples/text_preview.panack)**

Slice by Unicode scalar position. Compare a name that fits, one that needs shortening, and a zero-width preview.

```sh
./bin/panack run examples/text_preview.panack
```

```text
Ada
Aλ🙂…
0
```

**[Decode bytes received from another program](../../examples/decode_message.panack)**

Handle malformed UTF-8 at the input boundary. The second message contains an incomplete multibyte sequence.

```sh
./bin/panack run examples/decode_message.panack
```

```text
hello λ
Message contains invalid UTF-8
```

### Collections

**[Count votes by name](../../examples/count_votes.panack)**

Build a frequency table. Initialise a missing entry on its first vote, then update the count when the same name appears again.

```sh
./bin/panack run examples/count_votes.panack
```

```text
2
1
false
```

**[Remove duplicate names and preserve input order](../../examples/unique_names.panack)**

Use a set to track membership and an array for the ordered result. The first occurrence wins.

```sh
./bin/panack run examples/unique_names.panack
```

```text
Ian
Ada
Sam
```

### Test reports

**[Test a rule at its boundary](../../examples/test_booking_limits.panack)**

Use a table of inputs and independently specified expected results. The assertion name identifies the failing case and the failure count controls the exit status.

```sh
./bin/panack run examples/test_booking_limits.panack
```

```text
PASS one seat short
PASS exactly enough
PASS spare seat
tests: 3, failures: 0
```

**[Show only failed checks](../../examples/report_failures_only.panack)**

Inspect each test outcome to build a compact report. This demonstration deliberately includes a mismatch so you can see its name and reason.

```sh
./bin/panack run examples/report_failures_only.panack
```

```text
shipping label: expected Lisbon, got Paris
```

### Files and processes

**[Write and read a temporary note](../../examples/read_temporary_note.panack)**

Create a private temporary file in a writable directory, read it with a byte limit and decode the contents. Remove the file after either success or failure.

```sh
./bin/panack run examples/read_temporary_note.panack
```

```text
practice at six
```

**[Pass text to a child process](../../examples/pipe_input.panack)**

Send UTF-8 on standard input to /bin/cat and inspect its output. Repeat with a smaller capture limit to see the portable error code.

```sh
./bin/panack run examples/pipe_input.panack
```

```text
hello λ
output_limit
```

## Strings and bytes

Run `./bin/panack run examples/strings.panack`.
See the [complete program](../../examples/strings.panack).

`Str.len()`, indexing and slicing use Unicode scalar positions, not UTF-8 byte
positions or user-perceived characters. `"Aλ🙂"` has three scalars and encodes to
seven bytes. Iterate text with `for index in 0..text.len()` and `text[index]`.
For example, these four lines from the program print `3` and `7`:

```panack
text = "Aλ🙂"
encoded = text.encode_utf8()
print(text.len())
print(encoded.length())
```

Bind the indexed character to a local name before interpolation, as the example
does. Interpolation accepts local names.

Use `text.encode_utf8()` to obtain `Bytes`, `buffer.length()` for its byte count,
and `buffer.decode_utf8()` for known valid UTF-8. For untrusted bytes, use
`host.decode_utf8` after `import stdlib/host as host`. It returns
`Result[Str, HostError]`; handle
failure explicitly. The example's encoded text is valid by construction.

Use `name.ends_with(".panack")` for suffix checks. The collections example also
shows filtering sorted filenames without manually computing suffix positions.

## Collections and optional values

Run `./bin/panack run examples/collections_and_bytes.panack`.
See the [complete program](../../examples/collections_and_bytes.panack).

Array `append`, Map `put` and Set `add` return new collections. Save their return
values: appending `3` to `[1, 2]` leaves the original unchanged. Annotate empty
collections when their element or key/value types cannot be inferred.

`Map.get(key)` returns a value and traps when the key is absent. Check
`Map.has(key)` before reading a key that might be missing. The missing-key branch in the example prints `missing key`.

```panack
if answers.has("missing") {
  print(answers.get("missing"))
} else {
  print("missing key")
}
```

Array `first()` returns an `Option`; the
example explicitly chooses `99` as the fallback for an empty array. That fallback
is a demonstration policy, not a special value returned by the library.

Import the byte helper module with an alias such as `byte_buffer`:
`import stdlib/bytes as byte_buffer`. The name `bytes` is already a built-in.

## Test reporting

Run `./bin/panack run examples/testing_report.panack`.
See the [complete program](../../examples/testing_report.panack).

`testing.expect`, `equal_nat` and `equal_str` return test results. Collect them
and call `testing.report(results)`, which prints results in order and returns
the number of failures. It does not exit the process.

```text
PASS greeting
FAIL deliberate mismatch: expected 3, got 2
PASS empty collection
tests: 3, failures: 1
returned failures: 1
```

This teaching example deliberately includes one failing assertion and exits
successfully only when that expected count is returned. In a real test suite,
use `if failures > 0 { host.exit(1) }` after reporting. The host-failure recipe
uses that normal policy.

## Filesystem and process failures

Run `./bin/panack run examples/host_failures.panack`.
See the [complete program](../../examples/host_failures.panack).

A command can start successfully and still fail. For example, a shell that exits
with status 7 returns `Ok(output)`. Trying to start a missing executable returns
`Error(error)` with code `launch_failed`. This helper from the runnable example
shows which value to inspect in each case.

```panack
pure command_status(result: Result[ProcessOutput, HostError]): Str {
  match result {
    Error(error) => error.code,
    Ok(output) => {
      signal = output.signal
      status = output.exit_code
      if signal != 0 { "terminated by signal ${signal}" }
      else if status != 0 { "exited with status ${status}" }
      else { "completed successfully" }
    }
  }
}
```

The complete program creates a unique temporary directory under the current directory,
so run it from a writable location. It checks these behaviours and removes its
file and directory before reporting:

- Reading a missing file returns `Error` with `HostError.code == "not_found"`.
- Reading a five-byte file with a one-byte limit returns `"output_limit"`.
- Launching a missing executable returns `"launch_failed"`.
- A child that launches and exits with status 7 returns `Ok(ProcessOutput)`.
  Check `exit_code`, `signal`, `stdout` and `stderr`; `Ok` alone does not mean
  the child command succeeded. The example checks all four fields.

The expected report has ten passes and ends with `tests: 10, failures: 0`.
Host errors carry a portable `code`; avoid depending on platform-specific
`native_code` values. Process calls include a five-second timeout and a 128-byte
output limit. The only shell script is a fixed literal; do not substitute
untrusted text into shell commands. `process.run` inherits the environment and
applies supplied overrides; `run_isolated` instead supplies the complete child
environment. No environment isolation is needed for this fixed demonstration.

For the language and error contracts, see the
[specification](../reference/SPEC.md), the
[standard-library overview](../../src/stdlib/README.md), and the
[testing guide](../../tests/README.md).
