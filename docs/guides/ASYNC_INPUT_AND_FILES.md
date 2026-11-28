# Async input and safe file publication

These examples require the **alpha.16 development toolchain**. They use the
ordinary operation names through `/async` imports and explicitly await host work.

## Read bounded input until EOF

[async_console_input.panack](../../examples/async_console_input.panack) reads
standard input in chunks of at most 512 bytes, with a separate 4096-byte total
message limit. It converts the complete message with the checked pure UTF-8
API and prints the Unicode scalar count.

```sh
printf 'Aλ🙂' | ./bin/panack run examples/async_console_input.panack
```

```text
Received 3 characters.
```

An empty stream is valid and prints `Received 0 characters.`. When typing into
an interactive terminal, end input with EOF (usually Ctrl-D); this example does
not finish merely because a newline arrives. On a terminal with pending input,
EOF may first deliver that input and require another EOF to end the stream.

`console.read` returns `Ok(Some(bytes))`, `Ok(None())` for EOF, or an error.
A short read is not EOF. The example waits for `None()` and does not interpret
byte chunks as lines. It decodes once, after EOF, so a UTF-8 character split
between reads is preserved.

Oversized input prints `Input exceeds the 4096-byte message limit.` and exits
with status 1. Invalid text prints `Input is not valid UTF-8.` and also exits 1.
An I/O error uses the console error's explanation. A diagnostic-output failure
still leads to a failing exit status.

The 4096-byte limit is application policy, not the console API's maximum. Each
console read permits a limit of 1 through 65536 bytes. Repeated concatenation is
reasonable for this small bounded message; a larger stream should be processed
in chunks or use a suitable buffering strategy. Do not collect an unlimited
stream just to decode it at EOF.

## Publish a complete directory, then read within it

[async_publish_note.panack](../../examples/async_publish_note.panack) demonstrates
the staging and confined-read APIs together:

```sh
./bin/panack run examples/async_publish_note.panack
```

```text
Published note: hello
```

The example creates a unique temporary parent beneath the current directory,
so it needs write permission there. It builds a stage containing a file named
`note`, publishes the stage under the name `note`, reads the published file,
and removes its own file and directories before returning. The resulting path
is `<temporary parent>/note/note`; it never overwrites an existing application
file.

The sequence is:

1. `staging.create(parent)` creates an opaque `DirectoryStage`.
2. `staging.write_chunk(stage, relative, data, offset)` writes relative to the stage.
3. `staging.publish(stage, name, CreateOnly())` makes the prepared directory
   available at the chosen sibling name. An existing destination causes failure.
4. `confined.read(published_root, relative, limit)` performs the bounded read
   beneath that root and returns contents plus its physical root identity.

A successful publication consumes the stage. Do not discard it afterwards.
For an operation or pre-commit publication failure, the example attempts
`discard` and preserves any cleanup failure alongside the original error.
Publication is not implemented by exposing a partly written directory and
hoping readers wait for the last file.

The API also offers `ReplaceExisting`, but replacement has additional generation
and cleanup rules. This introductory example deliberately uses `CreateOnly`.
Read the [publication contract](../STDLIB_API_CONTRACT.md#staging-operations)
and [filesystem specification](../reference/SPEC.md) before implementing replacement.
An error after an unrelated operation is not proof that a previously successful
publication was rolled back.

## Confinement is an operation, not a string prefix

`confined.read(root, relative, limit)` enforces the filesystem boundary using the
native confined operation. Joining a root with user-provided text and calling an
ordinary read is not equivalent. The read result carries a physical root identity;
applications that need to bind multiple operations to an expected root must
check the relevant identity contract rather than assuming a pathname never changes.

The example's cleanup removes only paths it created beneath its unique parent.
It attempts cleanup after a read failure too, and reports cleanup errors instead
of claiming that temporary files were removed. It does not promise cleanup code
will execute after cancellation or process termination. The native staging
resource has its own cancellation and shutdown rules; arbitrary published files
do not acquire automatic lifetime management.

For a simpler file round trip without directory publication, see
[async_temporary_note.panack](../../examples/async_temporary_note.panack).
