# Standard-library redesign: implementation contract

18 November 2026. RM#162: Real-program fixes.

The owner approved the [redesign direction](STDLIB_REDESIGN_PROPOSAL.md) on
18 November. This document makes its implementation order and interface details
concrete. Signature choices below are implementation recommendations under that
direction, not claims about the released library. Changes with new semantics are
identified explicitly rather than hidden inside a rename.

## Common rules

- Ordinary and `/async` modules use identical operation names, arguments and
  result types. Pure helpers stay pure in both. Async reads may complete immediately.
- Type declarations have one canonical owner. Re-export types and variants; never
  create duplicate nominal identities for ordinary, async or compatibility paths.
- Factories/services are module functions; pure receiver operations remain methods.
- Preserve checked errors, byte limits, path confinement and exact arithmetic.
- Timeout values are Duration. Counts and byte bounds are Nat. Do not silently
  round durations to a native timer unit or interpret zero as unlimited.
- Native resources remain invocation-owned. Sharing an opaque handle does not
  permit simultaneous operations: overlapping use is rejected as busy. Operations
  already in flight retain resources until completion even after a waiter cancels.
- A read result, state update or resource handle must not be silently discarded on
  cancellation if doing so would leak resources or obscure an operation's effects.
- No async twin is needed for pure paths, encoding, text, numerics or collections.
- Builtin capability grants stay tied to trusted module/function identities.
  Relocation must update source checking, bytecode verification and runtime gates.

## Time

Ordinary owner `stdlib/time`; counterpart `stdlib/time/async`.

| Signature | Effect/meaning |
| --- | --- |
| `now(): Result[Instant,ClockError]` | Monotonic clock, ordinary/async |
| `sleep(duration: Duration): Result[Unit,HostError]` | Ordinary wait / scheduler timer |
| `seconds(value: Int): Duration` | Pure exact whole seconds |
| `milliseconds(value: Int): Duration` | Pure exact whole milliseconds |
| `nanoseconds(value: Int): Duration` | Pure exact nanoseconds |
| `from_fractional_seconds(value: Rat): Result[Duration,DurationError]` | Reject fractional nanoseconds |

Retain ClockError and DurationError identities and variants. Rename duration
methods to `as_nanoseconds(): Int` and `is_less_than(right: Duration): Bool`;
keep `as_seconds(): Rat`, add/subtract/scale/divide/ratio unchanged. Instant
arithmetic is unchanged. Signed durations remain valid values; sleep rejects
negative or unsupported durations. Zero sleep is immediate, not an infinite wait.

Aliases: from_nanoseconds -> nanoseconds; from_seconds ->
from_fractional_seconds; ticks -> as_nanoseconds; is_before -> is_less_than.
`host.sleep` delegates to the ordinary time API. Clock/calendar semantics do not
change. Sleep initially retains HostError; domain-error migration is a separate
semantic delivery below, not an accidental new nominal identity.

## Filesystem

Ordinary `stdlib/filesystem`; counterpart `/async`. Keep FileKind/FileMetadata
identities, and retain HostError until the explicit domain-error delivery.

| Signature | Preserved contract |
| --- | --- |
| `metadata(path: Path): Result[FileMetadata,HostError]` | Existing metadata semantics |
| `read(path: Path, limit: Nat): Result[Bytes,HostError]` | Bounded whole-file read |
| `write(path: Path, data: Bytes): Result[Unit,HostError]` | Existing write semantics, not atomic publication |
| `read_chunk(path: Path, offset: Nat, limit: Nat): Result[Bytes,HostError]` | Existing bounded/no-follow regular-file semantics |
| `write_atomic(path: Path, data: Bytes): Result[Unit,HostError]` | Existing atomic replacement, not crash durability |
| `list_directory(path: Path): Result[[Path],HostError]` | Existing deterministic ordering |
| `create_directory(path: Path): Result[Unit,HostError]` | Existing single-directory creation |
| `remove_file(path: Path): Result[Unit,HostError]` | Existing file removal |
| `remove_directory(path: Path): Result[Unit,HostError]` | Empty directory only |
| `create_temp_file(parent: Path): Result[Path,HostError]` | Creates an owned file, not a name reservation |
| `create_temp_directory(parent: Path): Result[Path,HostError]` | Creates an owned directory |
| `directories_overlap(left: Path, right: Path): Result[Bool,HostError]` | Existing physical overlap check |

Aliases retain temp_file/temp_directory/atomic_write. The old bounded_filesystem
and package_filesystem paths delegate without weakening confinement or bounds.
New async operations must have the same result/error semantics as ordinary ones;
cancellation abandons waiting, not an already-started filesystem side effect.

### Confined operations

`stdlib/filesystem/confined` and `/async` export:

- `root_identity(root: Path): Result[FilesystemRootIdentity,HostError]`.
- `read(root: Path, relative: Path, limit: Nat): Result[FilesystemRootRead,HostError]`.
- `read_link(root: Path, relative: Path): Result[Bytes,HostError]`.

Preserve the existing identity/read record fields. Confined reads resolve bounded
relative symlinks inside the root and reject escape; read_link reads the final
link target without following it and rejects symlinked ancestors.
Keep read_beneath as a compatibility alias. A root identity is evidence about a
read, not a permanent capability or a promise the filesystem cannot change.

### Staging operations

`stdlib/filesystem/staging` and `/async` use an opaque DirectoryStage:

- `create(parent: Path): Result[DirectoryStage,HostError]`.
- `path(stage: DirectoryStage): Result[Path,HostError]`.
- `create_directory(stage: DirectoryStage, relative: Path): Result[Unit,HostError]`.
- `write_chunk(stage: DirectoryStage, relative: Path, data: Bytes, offset: Nat): Result[Unit,HostError]`.
- `create_symlink(stage: DirectoryStage, relative: Path, target: Path): Result[Unit,HostError]`.
- `publish(stage: DirectoryStage, name: Path, mode: PublishMode): Result[Unit,HostError]`.
- `discard(stage: DirectoryStage): Result[Unit,HostError]`.

PublishMode is CreateOnly or ReplaceExisting. Retain existing relative child-name
and target-link policies. Successful publication consumes the stage; discard
removes unpublished content and consumes it. Specify failure/cancellation state
for each operation before implementing async publication; never promise rollback.
Typed resources and modes are semantic work, not merely wrapper renames.

## Console, environment and process

`console`/`console/async`: write, write_line, write_error, write_error_line each
accept Str and return Result[Unit,HostError]. A line operation adds exactly the
existing line terminator. Raw write does not. Ordinary writes gain checked failure
behaviour; preserve legacy host wrappers separately during compatibility.
A bounded stdin API still needs an explicit EOF/partial-read contract before
adding it. Do not invent one as a documentation-only addition.

`environment`/`environment/async`: `get(name: Str): Option[Str]`, preserving the
invocation snapshot, missing versus empty distinction and coverage-marker rules.

`process`/`process/async`: arguments(): [Str], exit(status: Nat): Unit, and
`run(command: Command): Result[ProcessOutput,HostError]`.

Command fields: executable: Path, arguments: [Str], stdin: Bytes,
working_directory: Path, environment: ProcessEnvironment, timeout: Duration,
output_limit: Nat. Use an explicit record; no language support for default arguments
or builder sugar is assumed. ProcessEnvironment has InheritEnvironment([Str])
and ExactEnvironment([Str]); preserve existing environment entry validation,
including duplicate-key errors. Do not silently replace lists with maps.

ProcessOutput contains termination: ProcessTermination, stdout: Bytes,
stderr: Bytes. ProcessTermination is Exited(Nat) or Signalled(Nat). Preserve
nonzero exit as a completed process result rather than a launch error. Preserve
explicit executable paths and no shell/PATH search. ExactEnvironment is not a
sandbox. Compatibility run/run_isolated signatures must remain on a legacy
adapter path because the language must not be assumed to overload run by arity.
Timeout and cancellation terminate/reap children under the existing process-group
policy; document descendant limitations rather than claiming general containment.

## Encoding, bytes and paths

`encoding/utf8.decode(data: Bytes): Result[Str,Utf8Error]` is pure and checked.
Utf8Error should identify the invalid byte offset and a stable decoding category;
it must not leak a host error into pure text conversion. Encoding can remain the
existing `Str.encode_utf8(): Bytes` method. Preserve explicitly named trapping
compatibility operations; no silently changing a method's return type.

Bytes gains len(): Nat and slice(start: Nat,end: Nat): Bytes with the existing
slice bounds contract. Preserve length and module slice as compatibility entry
points. Path gains parent(): Path preserving directory's exact lexical behaviour.
Path from_text/from_native/current and remaining owned methods stay unchanged.
Do not conflate lexical `current()` with a host call resolving the working directory.

## Digest, compression and networking

`digest/sha256`: create(): Result[Sha256Context,HostError], update(context,data):
Result[Unit,HostError], finish(context): Result[Bytes,HostError], close(context):
Result[Unit,HostError]. finish consumes the context. A pure one-shot hash(Bytes)
returns Bytes and is separately tested for bounds/fairness; context mutation is
not pure. Preserve old digest adapters during migration.

`compression/gzip`: create_decoder(output_limit: Nat): Result[GzipDecoder,HostError],
decode_chunk(decoder,input: Bytes,finish: Bool): Result[GzipChunk,HostError],
close(decoder): Result[Unit,HostError]. Preserve consumed/data/finished, concatenated
member handling, trailer checks and budgets. Define async admission/cancellation
for stateful CPU work before adding counterparts; do not disguise blocking work.

`network/tcp/async` retains exchange, finite serving as serve_bounded, continuous
serving as serve, and streaming/read operations. Replace timeout/drain Nat units
with Duration in the corresponding argument/limit records, retaining all byte and
concurrency bounds. The finite and continuous return types remain distinct.
Keep compatibility tcp/tcp_service modules. No synchronous server is invented.

HTTP's high-level client/server API stays in the HTTP package. Raw exchange,
stream open/read/close and server runtime hooks become documented support modules,
not competing URL clients. Preserve framing responsibility in Panack. HttpStream
is opaque; read/close ownership and cancellation are part of implementation acceptance.

## Testing, core and support

Move testing_files -> testing/filesystem; workspace_create -> create_workspace;
retain TestWorkspace.root and remove_empty. discover_fixtures keeps its contract.
Move testing_commands -> testing/process; run_command -> run. Preserve
command_output, command_result, capture_report and expect_command_error until a
specific clearer replacement exists. harness_command belongs to internal test
support, as does testing_async.read. Preserve testing.expect/equal_str/equal_nat,
TestResult/TestOutcome and report's ordered output/failure count.

Keep validation types/variants/rendering and validate unchanged. Keep Option,
Result and existing pure text/collection/numeric methods. Compiler-bound methods
must be included in the generated/reference inventory, not just source declarations.
Move compiler_files/compiler_paths into compiler-owned support with their existing
signatures first; changing compiler path representations is outside this redesign.
Move host.run(Bytes,[Str]) into runtime/bytecode support without promising a new
async embedding API. Keep prelude curated; do not flatten effectful APIs into globals.

## Errors and migration acceptance

Domain errors are a later semantic step in this same redesign. Before replacing
HostError, specify and test each operation's stable categories, native detail,
context and render output. Ordinary/async modules must share these types. Existing
callers get adapters; do not create new nominal types just by renaming imports.
Network errors must preserve TLS detail rather than just adding a new wrapper name.

Compatibility aliases stay throughout alpha.16. Proposed removal checkpoint:
alpha.17 after all repository consumers and published examples have migrated;
removal requires a documented migration guide and an explicit release decision.
No automatic deletion based only on a date.

Delivery order:

1. Names/imports and canonical type ownership; compatibility and relocated installs.
2. Broader filesystem plus confined operations; complete resource-boundary review.
3. Timers, console and subprocesses with bounded cancellation/cleanup.
4. Streaming HTTP and typed resource lifecycle integration.
5. Domain errors, remaining core-method migrations, reference and website examples.

Keep the existing programme weights provisional while these semantic changes are
sized. Track operation-level acceptance; do not infer completion from module files
existing. Use candidate builds and focused tests while batching, then one final
full check for a settled delivery. Development pushes remain CI-free. Website
preview/publication is separate and never implied by a library merge.

## Development checkpoint, 18 November 2026

Implemented on the development branch, not yet accepted for integration:

- time.nanoseconds and from_fractional_seconds, shared by ordinary/async imports,
  retaining the old factories and error identities.
- filesystem.create_temp_file/create_temp_directory, read_chunk, write_atomic
  and directories_overlap; existing paths and names remain available.
- General async metadata/read/write/list/create/remove/temp operations and overlap
  checking through the bounded filesystem worker, with canonical creation names.
- Async read_chunk and write_atomic reuse the ordinary bounded implementation: a
  64 KiB chunk limit, 256 MiB offset limit and 32 MiB atomic-write payload limit.
  Cancellation abandons waiting; an atomic replacement may still finish.
- Native admission checks reserve at most 512 MiB of pending payload budgets across
  at most 32 worker operations. This is a payload reservation, not total heap usage.

Candidate source/bytecode tests cover ordinary canonical operations, every general
async operation, shared FileMetadata identity, missing files and read limits.
Time tests cover exact fractional seconds, fractional-nanosecond rejection and
legacy compatibility. Native module and fault suites pass, including reservation
exhaustion/release. Bootstrap seed refresh, final compatibility/full checks and
relocated installation acceptance are pending the settled batch.

Temporary creation now keeps ownership on its bounded worker until the result is
successfully delivered to the execution. Cancellation before delivery removes the
new file or empty directory on that worker. Cleanup failure reports the path and
native error on stderr; it never recursively deletes content. These are ordinary
path operations, not a confinement or process-crash cleanup guarantee. Native
tests cover cancellation before/after completion, accepted ownership, allocation
failures and a non-empty directory preventing cleanup. No completion credit is
claimed for AS2 yet. Canonical ordinary and async confined modules now provide root_identity, read,
read_beneath compatibility and read_link. Candidate source/bytecode tests cover
shared record identity, contained symlinks and escape rejection. Staging ownership,
coverage inventory/hash reconciliation and final batch validation remain.
