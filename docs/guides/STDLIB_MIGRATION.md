# Moving to the alpha.16 standard-library APIs

This guide targets the **alpha.16 development branch**, not the released alpha.15
installation. Existing compatibility APIs remain available during alpha.16;
there is no requirement to rewrite every working caller before using a new API.

## Select effects at the import

An ordinary caller uses `import stdlib/time as time` and `time.now()`.
An async caller uses `import stdlib/time/async as time` and `await time.now()`.
Keep the same alias and operation names. Apply the same pattern to filesystem,
console, process, environment, digest, gzip and raw HTTP-stream modules listed in
[the reference](../reference/STANDARD_LIBRARY.md).

Changing an import alone is insufficient: mark the effectful caller `async`,
await its async operations, and update the callers of that function. Pure helpers
remain callable without `await`. Do not make an impure function `pure` merely to
allow a call from an async handler.

## Existing names and replacements

| Existing interface | Preferred development interface | What else changes |
| --- | --- | --- |
| `stdlib/async_host` | `stdlib/host/async` for compatibility; `console/async` for checked console I/O | Console error type changes; choose raw writes or newline writes explicitly |
| `stdlib/async_filesystem` | `stdlib/filesystem/confined/async` for confined operations | Domain errors replace `HostError`; `read_beneath` becomes `read` with the same root/relative distinction |
| `stdlib/filesystem/legacy` and `/legacy/async` | `stdlib/filesystem` and `/async` | Inspect `FilesystemError.kind` and operation context rather than only native error codes |
| `stdlib/process/legacy` | `stdlib/process` or `/async` | Construct `Command`; inspect `Exited`/`Signalled`, not separate exit-code/signal fields |
| `stdlib/tcp`, `stdlib/tcp_service` | `stdlib/network/tcp/async` | Use `Duration` deadlines and `TcpError`; finite `serve` becomes `serve_bounded`, continuous service uses `serve` |
| `stdlib/digest`, `stdlib/gzip` | `stdlib/digest/sha256`, `stdlib/compression/gzip`, or their `/async` modules | Opaque resources and domain errors; follow explicit lifecycle rules |
| Raw compatibility HTTP streaming | `stdlib/network/http_stream` or `/async` | Typed stream handle and `HttpStreamError`, with preserved transport detail |
| Package `http/client` | Package `http/client/async` | Same `get` names; `HttpClientError` wraps HTTP category and optional detailed transport failure |
| `time.from_nanoseconds`, `time.from_seconds` | `time.nanoseconds`, `time.from_fractional_seconds` | Old factories remain aliases; fractional construction still returns a result |
| `duration.ticks`, `duration.is_before` | `duration.as_nanoseconds`, `duration.is_less_than` | Existing methods remain compatible |
| Trapping `bytes.decode_utf8()` | `stdlib/encoding/utf8.decode(bytes)` | Handle `Result[Str,Utf8Error]` instead of assuming valid text |

The compatibility host `stdout` operation adds a newline. Its closest console
replacement is `write_line`, not `write`. Console `read` has explicit EOF and byte
limits; it is not a line-reading replacement.

## Preserve failure handling and bounds

Do not discard limits when moving to a newer interface. Keep explicit subprocess
output limits and deadlines, filesystem read limits, stream response limits and
gzip expansion limits. Convert units using duration factories instead of bare
millisecond numbers. The TCP adapter rejects fractional milliseconds rather than
rounding them.

A process can complete successfully at the API level with a nonzero exit status.
A resource can require cleanup even after an operation fails. Preserve the first
failure and report cleanup failures as well; see the
[temporary-file example](../../examples/async_temporary_note.panack) and
[subprocess example](../../examples/async_pipe_input.panack).

Use typed categories for decisions. Unknown native failures remain represented;
the new APIs do not invent causes when the host provides insufficient detail.
The diagnostic HTTP client preserves detailed TLS failures, but importing it does
not upgrade legacy consumers elsewhere in the program.

## Verify the migrated caller

Run the program from source, compile it, and run the saved bytecode. Test the
failure path relevant to the application: missing files, rejected input,
subprocess failure or an unavailable service. For resource-owning code, check
cleanup and cancellation separately. Existing test helpers have ordinary/async
counterparts; use the interface matching the function under test.

No bytecode-format change is needed for these APIs. A suitable development
compiler, VM and bundled stdlib must still be installed together; copying newer
stdlib files into an alpha.15 installation is not a supported upgrade procedure.
