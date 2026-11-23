# Standard library: ordinary and async APIs

This page describes the **alpha.16 development branch**. These APIs are not all
available in the released alpha.15 toolchain. Use a build of the matching branch
when running the linked examples.

## Choose an import, keep the function names

```panack
import stdlib/time as time
```

An ordinary function calls `time.now()` directly. An async function instead uses:

```panack
import stdlib/time/async as time
```

and calls `await time.now()`. Both return `Result[Instant,ClockError]`. Pure helpers
such as `time.seconds(5)` remain ordinary pure calls, even through the async import.
An async import does not make every exported function async.

The module path selects the effect. Async functions may call pure code and await
async operations; importing an ordinary blocking function does not make calling
that function safe inside an async handler. A clock read is impure because its
result depends on when the call occurs.

## Module map

Paths below follow `stdlib/`. An `/async` counterpart exports the same value and
error identities where both interfaces exist. The source links provide complete
signatures, including parameter order.

| Need | Ordinary module | Async module | Main operations |
| --- | --- | --- | --- |
| Monotonic clock and timers | [time](../../src/stdlib/time.panack) | [time/async](../../src/stdlib/time/async.panack) | `now`, `sleep`; pure duration factories |
| Environment | [environment](../../src/stdlib/environment.panack) | [environment/async](../../src/stdlib/environment/async.panack) | `get` |
| Checked console I/O | [console](../../src/stdlib/console.panack) | [console/async](../../src/stdlib/console/async.panack) | `read`, `write`, `write_line`, `write_error`, `write_error_line` |
| Subprocesses | [process](../../src/stdlib/process.panack) | [process/async](../../src/stdlib/process/async.panack) | `run(Command)`, `arguments`, `exit` |
| Files and directories | [filesystem](../../src/stdlib/filesystem.panack) | [filesystem/async](../../src/stdlib/filesystem/async.panack) | metadata, bounded reads, writes, directory operations |
| Confined filesystem access | [filesystem/confined](../../src/stdlib/filesystem/confined.panack) | [filesystem/confined/async](../../src/stdlib/filesystem/confined/async.panack) | `read`, `read_link`, `root_identity` |
| Staged directory publication | [filesystem/staging](../../src/stdlib/filesystem/staging.panack) | [filesystem/staging/async](../../src/stdlib/filesystem/staging/async.panack) | `create`, `path`, `create_directory`, `write_chunk`, `create_symlink`, `publish`, `discard` |
| SHA-256 | [digest/sha256](../../src/stdlib/digest/sha256.panack) | [digest/sha256/async](../../src/stdlib/digest/sha256/async.panack) | `create`, `update`, `finish`, `close`; pure `hash` |
| Gzip decoding | [compression/gzip](../../src/stdlib/compression/gzip.panack) | [compression/gzip/async](../../src/stdlib/compression/gzip/async.panack) | `create_decoder`, `decode_chunk`, `close` |
| Raw HTTP transport | [network/http_stream](../../src/stdlib/network/http_stream.panack) | [network/http_stream/async](../../src/stdlib/network/http_stream/async.panack) | `open`, `read`, `close` |
| TCP clients and services | No canonical blocking counterpart | [network/tcp/async](../../src/stdlib/network/tcp/async.panack) | `exchange`, `serve_bounded`, `serve_stream`, `serve`, `read_chunk` |
| Nested bytecode execution | [runtime/bytecode](../../src/stdlib/runtime/bytecode.panack) | No async counterpart | Ordinary runtime support; not a handler API |

Pure operations need no async twin: [path construction](../../src/stdlib/path.panack),
[byte slicing](../../src/stdlib/bytes.panack),
[checked UTF-8 decoding](../../src/stdlib/encoding/utf8.panack), numeric operations
and persistent collections can be called from either kind of function.

The [testing modules](../../src/stdlib/testing.panack) provide assertions;
[testing/filesystem](../../src/stdlib/testing/filesystem.panack),
[testing/process](../../src/stdlib/testing/process.panack) and
[testing/reporting](../../src/stdlib/testing/reporting.panack) have `/async`
counterparts for effectful test support.

## Clock and duration values

`now()` returns a monotonic `Instant`, not a calendar date. `end.difference(start)`
returns a signed `Duration`. Instants obtained during one execution share a clock
domain; later readings may equal earlier readings. Do not persist an instant and
compare it with a reading from another invocation.

`seconds(Int)`, `milliseconds(Int)` and `nanoseconds(Int)` are pure exact factories.
`from_fractional_seconds(Rat)` returns a result because a fraction smaller than one
nanosecond cannot be represented. `duration.as_seconds()` returns an exact `Rat`;
`duration.as_nanoseconds()` returns an `Int`.

`sleep(Duration)` returns `Result[Unit,SleepError]`. The async form suspends the
calling task. Zero is an immediate wait; negative or unsupported durations fail.
See [the timing example](../../examples/async_timing.panack).

## Console and environment

`environment.get(name)` returns `Option[Str]`: `None()` means absent, while
`Some("")` means present with an empty value.

`console.read(limit)` returns `Result[Option[Bytes],ConsoleError]`. A successful
read contains one non-empty chunk of at most `limit` bytes; `None()` means EOF.
The allowed limit is 1 through 65536. Reads are chunks, not lines. UTF-8 characters
can cross chunk boundaries, so preserve incomplete bytes when building a text
reader. `encoding/utf8.decode(bytes)` is the checked pure conversion for a complete
buffer; invalid text returns an error with a byte offset and category.

Console writes accept at most 16384 UTF-8 bytes before the optional newline.
`write`/`write_line` target stdout; `write_error`/`write_error_line` target stderr.
All return `Result[Unit,ConsoleError]`. A failed diagnostic write cannot itself be
reported reliably to that same stream; the examples still exit unsuccessfully.

## Files and cleanup

Filesystem APIs accept `Path`, not text. `path.from_text` validates a path without
accessing the filesystem. `path.current()` names the current directory.

General operations include `metadata`, `read(path,limit)`, `read_chunk(path,offset,limit)`,
`write`, `write_atomic`, `list_directory`, `create_directory`, `remove_file`,
`remove_directory`, `create_temp_file`, `create_temp_directory` and
`directories_overlap`. Bounded reads fail rather than silently truncating data.
These operations return `FilesystemError` on failure.

Confined reads take a root and a relative path. Use the confined interface when
reading beneath a designated root is part of the security boundary; concatenating
paths is not a substitute. Directory staging is a separate opaque resource for
building and publishing a tree. Publication and cleanup rules are specified in
[the filesystem contract](SPEC.md).

The [temporary-note example](../../examples/async_temporary_note.panack) attempts
cleanup after both success and failure, and preserves both errors when the main
operation and cleanup fail. Ordinary control flow does not promise that cleanup
code runs after task cancellation or process termination. Use each resource's
specified cancellation behaviour rather than assuming automatic language scopes.

## Processes

`process.run(Command)` returns `Result[ProcessOutput,ProcessError]`. Construct a
command with executable `Path`, argument list, input `Bytes`, working-directory
`Path`, `ProcessEnvironment`, timeout `Duration` and output-byte limit.

`InheritEnvironment(entries)` applies entries over the inherited environment.
`ExactEnvironment(entries)` supplies only those entries. Arguments are passed
as arguments; Panack does not interpret them as shell syntax.

A completed process can still report `Exited(nonzero)` or `Signalled(signal)`.
Inspect `output.termination` as well as the outer result. `stdout` and `stderr`
are byte buffers. Timeouts and output limits are execution failures rather than
successful output with invented exit codes. Process errors preserve native detail
without retaining argument, environment or input/output contents.

The [async subprocess example](../../examples/async_pipe_input.panack) runs
`/bin/cat` with explicit input, environment, timeout and output limit, checks its
termination, and decodes stdout. It requires that POSIX executable.

## Streams and owned resources

SHA-256 contexts, gzip decoders, HTTP streams and directory stages are opaque
resources belonging to the current invocation. Copying a handle does not create
an independent resource. Explicit close/discard invalidates aliases; check the
returned result. Follow each API's lifecycle and cancellation contract.

`sha256.hash(Bytes)` is a pure one-shot operation bounded to 16 MiB. Larger inputs
use a context. Async context updates and gzip steps yield between bounded pieces
of work; they do not make arbitrarily large CPU computations instantaneous.
Gzip decoding has an explicit output limit, including protection against highly
compressed input expanding beyond that limit.

Raw HTTP streams accept request bytes and return response bytes; HTTP parsing
belongs to the HTTP package. An empty successful read means EOF. The timeout
includes time between reads, and cancelling a pending read closes the stream.

For complete GET requests, use the package export `http/client/async`: its
`get`/`get_with_limits` return `HttpClientError`, retaining the HTTP category,
optional transport detail and scheme/host/port context. This is a package import,
not `stdlib/http`. Existing `http/client` and `http/stream` remain compatible.
See [the package documentation](../../packages/http/README.md).

Canonical TCP APIs take `Duration` values. Current TCP deadlines must be whole
milliseconds, at most 60 seconds; client/admission deadlines must be positive and
drain may be zero. Callback failures and native failures cannot always be
separated by the existing ABI, so some error categories deliberately remain broad.

## Handle domain errors

Console, sleep, filesystem, process, HTTP stream, TCP, SHA-256 and gzip APIs expose
domain-specific errors with typed categories and preserved native information.
Use categories for control flow and `.render()` for a human explanation. Do not
parse rendered prose to identify a failure or automatically retry every error.
`ClockError`, path errors and checked UTF-8 errors have their own contracts;
there is no universal error record or universal `.render()` method.

The [domain-error contract](../design/STDLIB_DOMAIN_ERRORS.md) lists categories,
context, lifecycle restrictions and known limits. The
[migration guide](../guides/STDLIB_MIGRATION.md) explains compatibility imports.
