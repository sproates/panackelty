# Standard-library organisation and async API proposal

17 November 2026. Related work: **RM#162: Real-program fixes**.
Delivery branch: `feature/rm162-async-clock`.

This records the recommendation presented to the owner for review. It is not
approval of every rename or an assertion that the proposed APIs are implemented.
The owner has selected broad async stdlib coverage and documentation; the detailed
API choices below remain proposals. See [the discussion and delivery programme](ASYNC_DESIGN_DISCUSSION.md).

## Direction

Organise modules around programmer tasks, rather than the feature that originally
needed a capability. Provide matching `/async` modules for appropriate effectful
operations, with consistent names, arguments, return types and error identities.
Pure helpers remain ordinary pure calls even when re-exported by an async module.
Do not invent synchronous equivalents merely to make the module tree symmetrical.

The principal problems are the mixed responsibilities in `host`, fragmented
filesystem APIs, misleading names such as `run_isolated`, different ordinary/async
error behaviour, untyped resource handles and exposed compiler/runtime adapters.

## Proposed module map

| Module | Responsibility | Async policy |
| --- | --- | --- |
| `stdlib/path` | Pure lexical paths | None |
| `stdlib/time` | Duration construction, monotonic clock, sleep | `/async`; constructors stay pure |
| `stdlib/environment` | Environment variables | `/async` |
| `stdlib/console` | Standard input/output/error | `/async` |
| `stdlib/process` | Invocation arguments, exit, subprocesses | `/async` |
| `stdlib/filesystem` | General file/directory operations | `/async` |
| `stdlib/filesystem/confined` | Root-constrained operations | `/async` |
| `stdlib/filesystem/staging` | Staged directory construction/publication | `/async` |
| `stdlib/bytes` | Byte construction helpers | None; receiver operations stay methods |
| `stdlib/encoding/utf8` | Checked text encoding/decoding | None |
| `stdlib/digest/sha256` | SHA-256 | Pure one-shot API; separately designed stateful streaming |
| `stdlib/compression/gzip` | Gzip decoding | Stateful streaming effect/fairness review |
| `stdlib/network/tcp` | TCP | Existing async capabilities under `/async` |
| `stdlib/validation` | Structured validation errors/rendering | None |
| `stdlib/testing` | Assertions/results | Async reporting where needed |
| `stdlib/testing/filesystem` | Workspaces/fixtures | `/async` for effects |
| `stdlib/testing/process` | Command execution/assertions | `/async` for effects |

Keep user-facing HTTP at `http/client` and `http/server`. Treat native HTTP
transport/runtime modules as implementation support, not competing public clients.
A specialist `runtime/bytecode` module should own nested bytecode execution.
Compiler-specific and fake async test helpers should be internal support.

## Split host responsibilities

| Existing | Proposed |
| --- | --- |
| `host.arguments` | `process.arguments` |
| `host.exit` | `process.exit` |
| `host.stdout` | `console.write_line`, preserving existing print semantics |
| `host.stderr` | `console.write_error_line` |
| `host.sleep` | `time.sleep` |
| `host.decode_utf8` | `encoding/utf8.decode` |
| `host.run` | `runtime/bytecode.run` |

Provide explicit `write`, `write_line`, `write_error`, `write_error_line` console
operations. Define bounded input and EOF semantics before adding input functions.
Ordinary writes should return checked results like async writes; currently ordinary
wrappers return Void while async output reports Result. Retire `async_host` and
`async_filesystem` public spellings through a deliberate compatibility migration.

## Filesystem

General filesystem names: `metadata`, `read`, `write`, `read_chunk`, `write_atomic`,
`list_directory`, `create_directory`, `remove_file`, `remove_directory`,
`create_temp_file`, `create_temp_directory`, `directories_overlap`.

Rename `temp_file`/`temp_directory` to `create_temp_file`/`create_temp_directory`,
and `atomic_write` to `write_atomic`. Move generic overlap checking out of
`package_filesystem`. Remove `bounded_filesystem` as a public organisational
boundary: bounds are important across the library, not unique to that module.
Preserve the stricter existing path-handling contracts of chunk reads and atomic
writes; relocation must not silently make them behave like ordinary operations.

Put root identity, confined reads and confined link reads under
`filesystem/confined`, with `root_identity`, `read`, `read_link`. Keep explicit
root/relative arguments and physical identity evidence where required.

| Existing staging function | Proposed |
| --- | --- |
| `begin` | `create` |
| `path` | `path` |
| `directory` | `create_directory` |
| `write` | `write_chunk` |
| `symlink` | `create_symlink` |
| `publish` | `publish` |
| `close` | `discard` |

Replace the publication `replace: Bool` with an explicit mode such as CreateOnly
or ReplaceExisting. Discard means deleting an unpublished stage. Preserve visibility
atomicity without implying crash durability or cancellation rollback.

## Time

Keep `now`, `seconds(Int)`, `milliseconds(Int)` and `Duration.as_seconds()`.
Rename `from_nanoseconds` to `nanoseconds`, and checked `from_seconds(Rat)` to
`from_fractional_seconds`. Recommend `Duration.as_nanoseconds()` instead of
`ticks()` in application code, and `Duration.is_less_than` instead of `is_before`.
Move sleep here. Keep `now` monotonic for compatibility and document that clearly;
a future calendar clock needs a distinct type/API.

```panack
import stdlib/time/async as time

interval = time.seconds(5)
await time.sleep(interval)
```

This illustrates the proposed sleep interface, not current released availability.

## Processes

Replace seven positional run arguments with a command/options value containing
executable, arguments, working directory, environment, stdin, timeout and output
limit. One `run(command)` should distinguish InheritEnvironment(overrides) from
ExactEnvironment(values). Retire `run_isolated`: exact environment is not a sandbox.
Replace the exit_code/signal sentinel convention with Exited(code) or
Signalled(signal). Preserve explicit executable paths, bounds and cleanup contracts.

## Typed resources, digest and compression

Replace Nat handles with opaque GzipDecoder, Sha256Context, HttpStream and
DirectoryStage types. Keep runtime stale/consumed-handle checks: wrappers alone do
not establish lifetime safety. Initially prefer module operations such as
`await staging.publish(stage, destination, mode)` so the import chooses ordinary
or async behaviour without requiring a new resource-method effect design.

Move `digest.sha256_open` to `digest/sha256.create`, with update/finish/close.
Add a separately scoped pure one-shot hash API. Incremental context mutation remains
effectful. Move gzip to `compression/gzip`, with create_decoder/decode_chunk/close;
retain consumed/data/finished results and explicit limits.

Checked UTF-8 decoding is pure and belongs in encoding, with its own error type.
Make checked decoding the recommended interface and clearly distinguish any trapping
alternative. It should not require host privileges or an async twin.

## Networking and errors

Distinguish finite TCP serving (`serve_bounded`) from continuous serving (`serve`)
rather than near-duplicate tcp/tcp_service modules. Use Duration for timeout/drain
arguments instead of unit-suffixed Nat values. Keep low-level raw HTTP transport
separate from URL/framing policy in the HTTP package.

Introduce structured network errors preserving native detail, including HTTPS
causes. Domain errors should expose stable typed categories, operation/context,
native details where available and readable render(). A common host error may stay
underneath; callers should not need a miscellaneous host import to name file errors.
Ordinary/async equivalents must share error identities. Cancellation documentation
must state which effects may already have occurred.

## Core methods and testing

Keep receiver operations on their types rather than recreating wrapper modules.
Standardise Bytes.length to len, prefer data.slice over bytes.slice, and consider
Path.parent instead of directory while retaining lexical semantics. Keep display
text distinct from reversible native path bytes. Document compiler-provided methods
alongside source-defined methods: source declarations alone are not the whole API.

Move testing_files to testing/filesystem and testing_commands to testing/process;
rename workspace_create to create_workspace and run_command to run. Keep typed
assertion names until a sound generic assertion API exists. Move testing_async's
fake read primitive into internal test support. Move compiler_files/compiler_paths
to compiler support with coordinated builtin capability and bootstrap migration.

Keep validation's structured failures and rendering; no rename of validate is
proposed. Keep prelude curated; direct domain imports remain the recommended way
to make effect choices visible. Do not duplicate pure text/collection/numeric APIs.

## Implementation order and compatibility

1. Agree the public inventory and canonical module/type ownership.
2. Move/rename existing APIs with temporary re-exports and one type identity.
3. Complete async counterparts against the agreed interface.
4. Deliver substantive process-options, typed-resource and error improvements.
5. Migrate examples, tools, compiler callers and documentation together.

Give compatibility aliases a defined removal point. Do not combine cheap renames
with semantic changes without calling out their separate cost and acceptance.
The existing async programme's cancellation, resource ownership, unsupported-host,
source/bytecode and relocation checks still apply. Documentation covers both
ordinary and async APIs and their availability. No release completion is implied.
