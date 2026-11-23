# Standard-library redesign: implementation contract

18 November 2026. RM#162: Real-program fixes.

The owner approved the [redesign direction](STDLIB_REDESIGN_PROPOSAL.md) on
18 November. This document makes its implementation order and interface details
concrete. Signature choices below are implementation recommendations under that
direction, not claims about the released library. Changes with new semantics are
identified explicitly rather than hidden inside a rename.

Console, sleep and process domain-error migration is now implemented. The
[domain-error contract](design/STDLIB_DOMAIN_ERRORS.md) defines categories, context,
rendering and compatibility. Dated checkpoints below retain their historical
validation evidence; this contract supersedes their provisional HostError results.

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
| `sleep(duration: Duration): Result[Unit,SleepError]` | Ordinary wait / scheduler timer |
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
`host.sleep` and ordinary `time.sleep` share the native blocking implementation
during compatibility. Canonical sleep now returns SleepError, retaining the
original HostError in its native field. Clock and calendar semantics do not change.

## Filesystem

Ordinary `stdlib/filesystem`; counterpart `/async`. Keep FileKind/FileMetadata
identities. Canonical APIs now return FilesystemError; compatibility paths retain HostError.

| Signature | Preserved contract |
| --- | --- |
| `metadata(path: Path): Result[FileMetadata,FilesystemError]` | Existing metadata semantics |
| `read(path: Path, limit: Nat): Result[Bytes,FilesystemError]` | Bounded whole-file read |
| `write(path: Path, data: Bytes): Result[Unit,FilesystemError]` | Existing write semantics, not atomic publication |
| `read_chunk(path: Path, offset: Nat, limit: Nat): Result[Bytes,FilesystemError]` | Existing bounded/no-follow regular-file semantics |
| `write_atomic(path: Path, data: Bytes): Result[Unit,FilesystemError]` | Existing atomic replacement, not crash durability |
| `list_directory(path: Path): Result[[Path],FilesystemError]` | Existing deterministic ordering |
| `create_directory(path: Path): Result[Unit,FilesystemError]` | Existing single-directory creation |
| `remove_file(path: Path): Result[Unit,FilesystemError]` | Existing file removal |
| `remove_directory(path: Path): Result[Unit,FilesystemError]` | Empty directory only |
| `create_temp_file(parent: Path): Result[Path,FilesystemError]` | Creates an owned file, not a name reservation |
| `create_temp_directory(parent: Path): Result[Path,FilesystemError]` | Creates an owned directory |
| `directories_overlap(left: Path, right: Path): Result[Bool,FilesystemError]` | Existing physical overlap check |

Aliases retain temp_file/temp_directory/atomic_write. The old bounded_filesystem
and package_filesystem paths delegate without weakening confinement or bounds.
New async operations must have the same result/error semantics as ordinary ones;
cancellation abandons waiting, not an already-started filesystem side effect.

### Confined operations

`stdlib/filesystem/confined` and `/async` export:

- `root_identity(root: Path): Result[FilesystemRootIdentity,FilesystemError]`.
- `read(root: Path, relative: Path, limit: Nat): Result[FilesystemRootRead,FilesystemError]`.
- `read_link(root: Path, relative: Path): Result[Bytes,FilesystemError]`.

Preserve the existing identity/read record fields. Confined reads resolve bounded
relative symlinks inside the root and reject escape; read_link reads the final
link target without following it and rejects symlinked ancestors.
Keep read_beneath as a compatibility alias. A root identity is evidence about a
read, not a permanent capability or a promise the filesystem cannot change.

### Staging operations

`stdlib/filesystem/staging` and `/async` use an opaque DirectoryStage:

- `create(parent: Path): Result[DirectoryStage,FilesystemError]`.
- `path(stage: DirectoryStage): Result[Path,FilesystemError]`.
- `create_directory(stage: DirectoryStage, relative: Path): Result[Unit,FilesystemError]`.
- `write_chunk(stage: DirectoryStage, relative: Path, data: Bytes, offset: Nat): Result[Unit,FilesystemError]`.
- `create_symlink(stage: DirectoryStage, relative: Path, target: Path): Result[Unit,FilesystemError]`.
- `publish(stage: DirectoryStage, name: Path, mode: PublishMode): Result[Unit,FilesystemError]`.
- `discard(stage: DirectoryStage): Result[Unit,FilesystemError]`.

PublishMode is CreateOnly or ReplaceExisting. Retain existing relative child-name
and target-link policies. Successful publication consumes the stage; discard
removes unpublished content and consumes it. Specify failure/cancellation state
for each operation before implementing async publication; never promise rollback.
Typed resources and modes are semantic work, not merely wrapper renames.

### Staging lifetime and async implementation boundary (19 November)

The following is the implementation contract, not a claim that async staging is
already available. Existing Nat handles remain compatibility entry points.

| Operation | Ownership and failure outcome |
| --- | --- |
| create | A worker owns a new stage until successful result handoff. Cancellation before handoff discards it. Allocation failure must not strand an undisclosed stage. |
| path | Returns a snapshot of the current path after parent-identity validation; it does not transfer ownership or guarantee future path stability. |
| create_directory / write_chunk / create_symlink | The stage remains owned and usable after completion. Failure or cancellation may leave a partial change; no rollback is promised. |
| publish | Pre-commit failure leaves the stage usable. Successful rename/exchange consumes it, even if waiting was cancelled. Never return publication failure after the commit point. |
| discard | Successful removal consumes the stage. Cleanup failure must retain enough ownership to retry or report the remaining path; it must not silently forget remaining content. |
| invocation shutdown | Abandon pending waits, prevent new operations, and retain native ownership until workers finish. Remove unpublished content; never remove the published destination. |

One operation at a time may use a stage. A second concurrent operation returns a
structured busy error rather than racing mutation or cleanup. Aliases refer to
one resource state: consumption invalidates all of them. Opaque DirectoryStage
values must identify their owning invocation and cannot be constructed by wrapping
an arbitrary Nat in a public record.

Workers must own native resource state independently of VM/program/value lifetimes.
Do not borrow the current VM stage table without an independently retained lease
when dispatching a filesystem worker. Table destruction must not close descriptors still in use by a worker. Keep
admission bounded and perform recursive discard on workers, not the event loop.
Prepare the successful publication result before committing, so an allocation
failure cannot make a committed publication appear to have failed. Old-generation
cleanup after replacement is separate from publication success and must report
failure without deleting a substituted directory.

The existing synchronous implementation has verified pre-commit rejection,
continued stage use, consumed handles and shutdown cleanup. Failed close now retains ownership for retry. Post-publication cleanup failures
are reported without reversing publication success; retained old-generation
resources receive another cleanup attempt at invocation shutdown. If the old
generation cannot be opened or identified safely, Panack reports its location
and does not delete it. Successful publication results are allocated before
commit. Focused native tests cover retry after identity substitution and a
permission-denied old-generation cleanup (the latter requires a non-root user).
The native opaque DirectoryStage representation now retains an invocation identity
token independently of the VM table. Native regressions reject forged numbers,
foreign invocation handles, stale handles after table recreation, and consumed
aliases. Identity references are atomic so values and worker-held resource tables can
finish on different threads. A native worker lease retains the table independently
of the VM and exclusively reserves one stage. Ordinary operations and another
lease on that stage return busy until release. VM shutdown detaches its table;
the last lease performs remaining cleanup after its operation finishes.
The ordinary `filesystem/staging` module now exposes DirectoryStage and PublishMode
through the candidate compiler. Source/bytecode tests cover creation, writes,
create-only rejection, replacement, discard and alias consumption; negative cases
reject numeric handles, direct construction and pure side effects. A native-thread test performs a stage operation after VM destruction and source
value release, then verifies cleanup on lease release. The development async
module now dispatches all seven operations through bounded filesystem workers.
Creation reserves an invocation slot before dispatch and retains worker ownership
until result handoff; cancellation before handoff discards the stage. Existing
stages are leased exclusively, and publication may commit after cancellation.
Source/bytecode tests cover the matching API; native tests cover creation
cancellation, accepted handoff, publication after cancellation and allocation
failures during creation. Embedded teardown now transfers its table reference to a single cleanup worker
without allocation or filesystem I/O. Retained tables are bounded at 32 per
process, including queued cleanup; each invocation still has at most 64 slots.
Capacity exhaustion returns resource_limit. Normal CLI runtime shutdown drains
queued cleanup after stopping the runtime; embedded event loops never drain it.
A paused-fstat regression proves destruction returns while cleanup is blocked.
Admission/recovery tests cover the global table bound. This closes the recorded
owner-thread teardown gap; local batch and coverage-inventory checks passed on
19 November. Integration remains pending.

## Console, environment and process

`console`/`console/async`: write, write_line, write_error, write_error_line each
accept Str and return Result[Unit,ConsoleError]. A line operation adds exactly the
existing line terminator. Raw write does not. Ordinary writes gain checked failure
behaviour; preserve legacy host wrappers separately during compatibility.
Both modules also provide read(limit: Nat): Result[Option[Bytes],ConsoleError],
with the bounded input contract below.

`environment`/`environment/async`: `get(name: Str): Option[Str]`, preserving the
invocation snapshot, missing versus empty distinction and coverage-marker rules.

`process`/`process/async`: arguments(): [Str], exit(status: Nat): Unit, and
`run(command: Command): Result[ProcessOutput,ProcessError]`.

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

`digest/sha256`: create(): Result[Sha256Context,Sha256Error], update(context,data):
Result[Unit,Sha256Error], finish(context): Result[Bytes,Sha256Error], close(context):
Result[Unit,Sha256Error]. finish consumes the context. A pure one-shot hash(Bytes)
returns Bytes and is separately tested for bounds/fairness; context mutation is
not pure. Preserve old digest adapters during migration.

`compression/gzip`: create_decoder(output_limit: Nat): Result[GzipDecoder,GzipError],
decode_chunk(decoder,input: Bytes,finish: Bool): Result[GzipChunk,GzipError],
close(decoder): Result[Unit,GzipError]. Preserve consumed/data/finished, concatenated
member handling, trailer checks and budgets. The execution/lifetime contract below
defines bounded async CPU work.

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

Console, sleep and process errors follow the linked domain-error contract. For the
remaining domains, before replacing
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
exhaustion/release. Bootstrap seed refresh, artifact compatibility and inventory
reconciliation pass for the settled batch. Final acceptance is recorded below.

Temporary creation now keeps ownership on its bounded worker until the result is
successfully delivered to the execution. Cancellation before delivery removes the
new file or empty directory on that worker. Cleanup failure reports the path and
native error on stderr; it never recursively deletes content. These are ordinary
path operations, not a confinement or process-crash cleanup guarantee. Native
tests cover cancellation before/after completion, accepted ownership, allocation
failures and a non-empty directory preventing cleanup. No completion credit is
claimed for AS2 yet. Canonical ordinary and async confined modules now provide
root_identity, read,
read_beneath compatibility and read_link. Candidate source/bytecode tests cover
shared record identity, contained symlinks and escape rejection. Typed ordinary and
async staging now implement all seven operations, with opaque
resource identity, exclusive worker leases and cleanup after cancellation. Embedded
teardown queues cleanup without blocking the execution thread. Admission is bounded
at 32 retained invocation tables and 64 stage slots per invocation.

## Filesystem batch local acceptance, 19 November 2026

`make check` passed in 606 seconds, including unit, functional, bootstrap and
release-smoke checks. Separate ordinary and async staging programs passed source
execution and saved-bytecode execution after moving a fresh installation, from an
unrelated working directory with checkout library overrides removed. All seven
staging operations, named publication modes and consumed aliases were exercised.

The installation inventory includes every nested stdlib module. Thirteen coverage
inventory/policy tests pass; four new adapters are explicitly unmeasured by the
bounded source-coverage corpus. This does not claim new measured coverage.

Artifact preflight passes. The stdlib golden artifact adds 12 wrapper/import
functions; all 35 existing function bodies are unchanged after module renumbering,
and old/new execution output matches. Compiler stages 2–4 reach the same fixed
point. Bytecode remains v9.

The historical timing warnings remain informational under the existing performance
reassessment. This expanded run is not a controlled performance comparison and no
speed improvement is claimed. Integration and hosted validation remain pending;
development CI stays off. AS2 remains in progress, including remaining specialist
resource decisions and migrations; this does not complete the expanded programme.

## Timer development checkpoint, 20 November 2026

Ordinary `time.sleep` and `time/async.sleep` now have matching signatures; the
legacy host operation remains available. Async sleep retains only a monotonic
deadline in its suspended execution. HTTP handler polling and task-session polling
allow other executions to progress. Cancellation drops the timer immediately.
Zero duration completes immediately; negative/oversized durations and clock
failures return the existing HostError representation. The existing 365-day bound
and exact nanosecond representation remain unchanged.

Candidate source and saved-bytecode checks, native module contracts and native
fault contracts pass, including task-session resumption, independent execution,
cancellation, error types and completion allocation failure. No worker is created
per timer; polling costs one clock read per pending timer per host poll. No
throughput claim is made. The compiler seed and final batch validation are deferred
until the related console/process implementation settles. This is a development
checkpoint, not AS3 completion or integration acceptance.

## Console development checkpoint, 20 November 2026

Both console modules now implement write/write_line/write_error/write_error_line,
returning the shared HostError result. Raw writes add no bytes; line writes append
one LF. The existing 16 KiB message bound (excluding LF), four retained operations,
SIGPIPE-safe worker and cancellation ownership apply to both paths. Ordinary calls
wait for that worker; async calls suspend. Legacy host wrappers retain their return
types; legacy synchronous printing flushes before returning to preserve ordering
with descriptor-based checked output. Concurrent writes can interleave and failed
writes may have emitted a prefix.

Focused source/bytecode tests cover every operation, UTF-8, empty/raw/line output,
legacy ordering, type checking and effect restrictions. Native tests add exact
embedded-NUL bytes and closed descriptors; fault tests exercise legacy and raw
checked paths through allocation failures, blocked-write cancellation and admission
exhaustion. Candidate builds are used; final seed and combined batch validation
remain pending with subprocess work. Ordinary checked output now pays the bounded
worker setup/wait cost; no throughput improvement is claimed.

## Subprocess development checkpoint, 21 November 2026

Ordinary and async process modules implement Command, explicit environment modes
and Exited/Signalled results. Positional compatibility is isolated in
process/legacy; existing compiler/package/test callers import it explicitly.
The shared native collector retains existing deadlines, output bounds and
termination policy. Async calls copy their operands and environment, prepare
coverage tickets on the owner thread, and suspend through a four-operation
worker service. A 32 MiB copied-payload bound and 4096 inherited-entry bound limit
admission. Cancellation requests termination/reaping without blocking the owner.

Focused source/bytecode, native lifecycle and allocation/syscall-fault tests pass:
environment ownership, nonzero exits, signals, limits, deadlines, cancellation,
slot recovery and embedded grants. HTTP progress passes in source and bytecode modes. Final combined local
`make check` passed in 612 seconds on 21 November (unit 379s, bootstrap 82s);
packaging and release-smoke checks are included. The browser runtime bundle and
its unsupported-host process compilation path also pass. Ordinary execution retains its direct collector path; async
execution adds a bounded copy and worker setup cost. No throughput claim is made.
The timer import changes the stdlib oracle's module numbering and adds time.sleep;
reviewed disassembly and identical old/new output support that fixture update.

This is local acceptance of the timer/console/process implementation batch, not
completion of AS3 or RM#162. Input and nested-bytecode boundaries, remaining
specialist/resource work, streaming HTTP, domain errors and documentation delivery
remain tracked. Integration and hosted validation are pending; development CI is off.

Ordinary and async Command APIs also pass source and saved-bytecode tests from a
relocated installation with spaces in its path, without checkout library overrides.

## Runtime and testing support checkpoint, 21 November 2026

Canonical runtime/bytecode.run re-exports the ordinary host.run operation. It
executes verified bytecode synchronously, preserves arguments and exit propagation,
and cannot be called from async or pure code. There is no async embedding API in
this batch, as agreed above. This completes the nested-execution classification.
The later bounded console input delivery below supplies the distinct
EOF/partial-read/cancellation contract; legacy read_line remains ordinary.

Canonical testing/filesystem provides create_workspace, discover_fixtures and
remove_empty(workspace). Its /async module awaits filesystem operations and shares
the existing TestWorkspace identity. TestWorkspace.remove_empty remains ordinary;
async callers use the module function. Callers own workspace contents and cleanup;
discovery preserves sorted immediate directories and rejects partial results.

Canonical testing/process provides run, command_output, command_result,
capture_report and expect_command_error. Both variants share TestCommand and the
compatibility ProcessOutput types. Async execution adapts explicit termination
back to those existing test observations. Failed observations never overwrite a
capture destination; capture_report retains its existing mismatch-diagnostic UTF-8 decoding behavior
(invalid UTF-8 can trap). A destination-write failure may leave partial output.
The command-supervisor harness remains on internal compatibility support and is
not re-exported as a new public helper.

Testing/reporting and testing/reporting/async share pure assertions and TestResult/TestOutcome. Both report operations return
Result[Nat,HostError]: Ok carries the failure count; a checked console error stops
reporting and returns Error. Prior output is not rolled back. Lines and the final
summary preserve caller order and the existing transcript format. Ordinary report
continues returning Nat, and old testing_files/testing_commands imports remain.

Mixed process APIs exposed native-record schema aliasing during emission. Native
adapter record names are now reserved for their actual owning module; new stdlib
declarations with those names receive private emitted symbols. A second regression
showed that the Panack bytecode reader lacked recent async services. Source effect
checking and bytecode validation now share their closed async classification.
Final local make check passed in 583 seconds on 21 November (unit 371s,
bootstrap 81s, functional 146s), including packaging and release-smoke checks.
All 275 focused bytecode-reader checks and 13 coverage-inventory checks pass.
The seven support modules also pass ordinary/async source, saved-bytecode and
disassembly tests from a relocated installation without checkout overrides.
No bytecode format change. Integration remains pending; development CI stays off.

## Streaming HTTP implementation — 21 November 2026

`stdlib/network/http_stream` and `stdlib/network/http_stream/async` expose the
same signatures; the latter's operations are async:

- `open(host: Str, port: Nat, tls: Bool, request: Bytes, response_limit: Nat, timeout: Duration): Result[HttpStream,HttpStreamError]`
- `read(stream: HttpStream): Result[Bytes,HttpStreamError]`
- `close(stream: HttpStream): Result[Unit,HttpStreamError]`

These are raw transport support, not a second high-level HTTP client. The HTTP
package retains parsing and framing. `open` allocates a transport; it does not
confirm a connection or response. Reads drive connection, request transmission
and response receipt. Async reads yield through libcurl polling on the VM owner
thread, without occupying a worker. Ordinary reads block.

HttpStream is an opaque, invocation-owned value. Copies refer to the same stream;
close invalidates every copy. Numeric compatibility handles cannot address typed
streams. An overlapping read, or close during a read, returns
`HTTP stream read is already in progress`. Foreign and closed handles return
`invalid HTTP stream handle`. Cancelling a pending read consumes the stream,
including when bytes arrived before cancellation; a subsequent read cannot
silently skip those bytes. Teardown closes idle streams and pending reads retain
safe ownership until their cancellation/completion. Cancellation cannot undo a
request already sent to a server. Explicit close is required for early release;
invocation teardown provides final cleanup.

Each read returns at most 16 KiB; empty Bytes means EOF and remains repeatable
until close. Request size is at most 64 KiB, total raw response size at most
256 MiB, with at most 512 opens per invocation (slots are not reused). Timeout
must be positive and at most 60 seconds. Its exact nanosecond deadline starts at
open and includes time between reads; the curl millisecond backstop rounds up,
while VM deadline checks retain the original value. Transport failures retain
existing Str categories pending the separately tracked domain-error work.
The legacy stdlib/http_stream numeric/millisecond API remains compatible.
Native embedders must grant network access for the async operations. Browser/WASI
hosts retain the existing unsupported-transport result.

Final local make check passed in 647 seconds (unit 406s, bootstrap 86s,
functional 158s), including package/archive smoke and quick-start checks.
Ordinary/async source and saved-bytecode tests, same-server handler progress,
native lifecycle and 2,200 native allocation-failure cases pass. Native module
contracts also pass under address/undefined-behaviour sanitizers. All 284
bytecode-reader checks and 13 coverage-inventory checks pass. A relocated package
with spaces in its path passes the stream API and handler tests, plus imports,
source execution, saved bytecode and disassembly from an unrelated directory
without checkout overrides. Integration remains pending; development CI stays off.

### Digest/gzip execution and lifetime

Both ordinary and async modules are implemented. Async create/update/finish/close
and decoder operations execute one bounded atomic native step on the VM owner
thread, then yield before the caller resumes. They do not occupy an I/O worker.
Cancellation before the step makes no mutation; cancellation afterwards does not
roll back the step. Task/VM teardown closes owned resources. No operation suspends
halfway through a mutation, so there is no in-flight busy state.

Sha256Context and GzipDecoder are opaque invocation-owned handles. Copies alias
the same resource; finish/close invalidate all copies. Numeric and foreign handles
are rejected. Output/acknowledgement allocation failure after mutation closes the
resource rather than leaving an uncertain state reusable. Compatibility numeric
adapters remain separate and retain their existing callers.

SHA-256 updates accept at most 64 KiB per call. The pure hash(Bytes): Bytes helper
accepts at most 16 MiB; larger input traps with guidance to use a context. It does
not yield and is re-exported unchanged by the async module. Ten 16 MiB hashes took
about 5–7 ms each on the development Mac; this is an observation, not a portable
latency guarantee. Streaming contexts are appropriate for larger input or finer
cooperative scheduling. At most 65,536 contexts can be created per invocation.

Gzip takes at most 64 KiB input and emits at most 64 KiB per step, with a total
output budget of at most 256 MiB and at most 64 decoder slots per invocation.
Callers preserve unconsumed input using GzipChunk.consumed. Concatenated members,
trailer verification and truncated input remain checked. Slots are not reused.

Native builds reuse OpenSSL and zlib. Unsupported targets return HostError with
code unsupported for resource operations; pure hash reports an unsupported VM
error. Domain-specific error migration remains separate work.

## Checked UTF-8 and value methods (21 November 2026)

Implemented on the development branch: pure encoding/utf8.decode returns
Result[Str,Utf8Error]. Utf8Error contains offset: Nat and kind: Utf8ErrorKind.
Offsets count bytes from zero, not Unicode characters. InvalidContinuationByte
points to the offending byte; other categories point to the sequence start.
The remaining categories are InvalidLeadingByte, TruncatedSequence,
OverlongEncoding, SurrogateCodePoint and CodePointOutOfRange. The first invalid
sequence is reported. Within a sequence, an invalid leader takes priority, then
insufficient remaining bytes, then continuation validation, then scalar validity.
No replacement characters are inserted. Empty input and embedded NUL are valid.

Utf8Error.render() explains the offset convention and specific failure in one
sentence. The existing native validator supplies the successful fast path; only
invalid input is scanned in Panack to classify the problem. Internal HostError
values never escape this public API. Existing host.decode_utf8 and trapping
Bytes.decode_utf8 remain compatible. There is no async twin for pure conversion.

Bytes.len() aliases length(); slice(start,end) uses the existing half-open bounded
slice operation and traps when start > end or end > length. Empty and whole-value
slices are valid. Path.parent() preserves directory() exactly, including lexical
relative/root behaviour; it performs no filesystem access. Old names remain.

## Canonical TCP adapters (21 November 2026)

Implemented in network/tcp/async, with the existing TCP runtime:

- exchange(address,port,request,response_limit,timeout: Duration): Result[Bytes,TcpError].
- serve_bounded(address,port,handler,limits: TcpServerLimits): Result[[Result[Unit,TcpError]],TcpError].
- `serve_stream[T](address,port,worker,context,limits: TcpServerLimits): Result[[Result[Unit,TcpError]],TcpError]`. This remains a finite streaming server.
- `serve[T](address,port,worker,context,limits: TcpServiceLimits,ready): Result[TcpServiceSummary,TcpError]`. This is continuous serving, with existing readiness/signal/drain behaviour.
- read_chunk(limit: Nat): Result[Bytes,TcpError], usable only in the existing streaming-server child context.

TcpServerLimits fields are clients, concurrency, request_limit, response_limit
(all Nat), client_timeout, admission_timeout and drain (all Duration).
TcpServiceLimits retains concurrency/request_limit/response_limit and uses
client_timeout/drain Duration fields. TcpServiceReady and TcpServiceSummary are
re-exported from their existing owner, not duplicated. The new Duration records
have distinct nominal identities from legacy millisecond records; the compiler
reserves native compatibility names for tcp/tcp_service's original declarations.

Timeouts must be whole milliseconds from 1 ms through 60 seconds. Drain permits
zero through 60 seconds, also in whole milliseconds. Negative, over-limit or
fractional-millisecond values return a specific error before network admission;
zero does not mean unlimited. These adapters preserve the existing timer precision
and do not silently round. Supporting finer TCP timers requires separate runtime
work. Existing IPv4-only addressing, byte/concurrency/client limits, result shapes,
error strings and invocation ownership remain. No synchronous server is added.

Duration.as_nanoseconds() and is_less_than(right) now preserve ticks() and
is_before(right) respectively; compatibility methods remain. TCP error types now follow the domain-error contract below.

## Bounded console input (21 November 2026)

console.read(limit: Nat) returns Result[Option[Bytes],ConsoleError]. Import
stdlib/console for ordinary calls or stdlib/console/async for awaited calls.
A limit from 1 through 65,536 is required. This 64 KiB per-read bound matches the
streaming chunk scale and bounds retained input after cancellation. Invalid
limits return out_of_range without consuming input. Some contains a nonempty
chunk no longer than limit; short reads are normal. None means end of input.
Empty input and subsequent EOF reads return None. No text decoding, newline
stripping or carriage-return conversion occurs. Use encoding/utf8.decode for
checked text conversion, allowing for a UTF-8 sequence split across chunks.

Standard input is shared by the process. Exactly one read may be outstanding;
overlapping ordinary, async or legacy reads fail with busy. Async input uses one
worker, leaving the VM owner thread free. The worker owns a duplicated descriptor
and native buffer, never VM values. Terminal modes remain unchanged, so an
interactive terminal normally waits for a complete line. Ordinary calls wait for
the same worker. On unsupported hosts the result is unsupported_host. Other codes
are io_error and resource_limit in ConsoleError.native; the native operation is
console_read and native_code retains the host failure number where available.
ConsoleError adds the typed category, requested operation and render().

Cancellation abandons the async result without joining the worker. The worker
checks cancellation between readiness waits (currently 50 ms) and before reading.
A host read already in progress cannot be forcibly cancelled and keeps the sole
read reservation until it returns. Input already consumed is retained in a
process-owned buffer of at most 64 KiB until successfully delivered to a caller.
Result allocation or async stack-delivery failure also preserves those bytes.
A later read, including one in a later VM invocation, receives the retained bytes
first. Cancellation after delivery does not rewind input. Embedders must not
read or replace stdin concurrently outside this adapter.

Embedded async executions require vm_execution_enable_console_input. Output
permission does not imply input permission; HTTP request children do not inherit
stdin access. Legacy read_line shares the broker and its retained bytes, keeping
its existing carriage-return stripping and empty-string EOF behavior. It may
read ahead but consumes only through its newline. Its result remains unbounded
for compatibility; the new read API is the bounded alternative. On WASI, legacy
read_line retains its synchronous stdio implementation; the new bounded API
returns unsupported_host.

Focused native lifecycle, allocation-failure, sanitizer and ordinary/async
source/bytecode tests pass. Final local make check passed in 654 seconds (unit
407s, bootstrap 83s, functional 167s), including packaging and installed smoke.
A relocated installation with spaces in its path passes all input fixtures and
unrelated-directory source/bytecode/disassembly acceptance. All 308 bytecode
reader checks pass. Integration remains pending; development CI stays off.
No bytecode-format change is required. The previous full check took 626 seconds;
these runs are not a controlled performance comparison. Historical validation
budget warnings remain under the existing GI#106 reassessment.

## Console, sleep and process errors (21 November 2026)

The [domain-error contract](design/STDLIB_DOMAIN_ERRORS.md) defines the implemented
ConsoleError, SleepError and ProcessError records, stable categories, native detail,
rendered examples and migration. Ordinary/async APIs share identities. Compatibility
host and positional process APIs retain their old results. Focused tests pass;
final local make check passed in 621s (21 November), plus relocated source,
saved-bytecode and disassembly acceptance. Integration remains pending. Remaining filesystem,
digest/gzip and network error work is not complete.

## Filesystem errors and nominal identities (21 November 2026)

The [domain-error contract](design/STDLIB_DOMAIN_ERRORS.md#filesystem) defines the
shared FilesystemError, operation context, categories, native preservation and
compatibility paths. Both ordinary/async APIs and staging are covered. Compiler
emission separates ordinary stdlib declarations while preserving core/native ABI
identities. No VM or bytecode-format change is required. Final local make check passed in 628s (unit 393s, bootstrap 85s, functional 167s, 21 November), including packaging and installed smoke. Relocated ordinary/async source, saved-bytecode and unrelated-directory acceptance passes. Integration remains pending; development CI stays off.

## Digest/gzip domain errors (21 November 2026)

The [resource error contract](design/STDLIB_DOMAIN_ERRORS.md#sha-256-and-gzip-resources)
defines Sha256Error and GzipError, operation context, typed categories, honest
native limitations, retry/lifetime guidance and legacy migration. Focused ordinary/async source and saved-bytecode checks pass. Final local make check passed in 645s (21 November), including unit, functional, bootstrap and packaging checks. Relocated installation with a spaced path passed the ordinary/async resource and compatibility fixtures. Integration remains pending and development CI stays off.

## HTTP stream domain errors (21 November 2026)

Canonical network/http_stream and /async return HttpStreamError. Open errors retain host/port/TLS/limit/timeout context; first-read transport failures retain available bounded libcurl detail. Legacy numeric transport strings and high-level HTTP package signatures remain unchanged. See [domain errors](design/STDLIB_DOMAIN_ERRORS.md#http-streams).

HTTP stream domain errors pass focused real-TLS, ordinary/async and compatibility checks, final local make check (685s, 21 November), and relocated-install real TLS source/bytecode acceptance. Compiler seed and unchanged-program bytecode artifacts remain unchanged. Integration remains pending and development CI stays off.

## TCP and diagnostic HTTP client (21 November 2026)

network/tcp/async now returns TcpError, including typed finite-server client outcomes. The HTTP package exports client/async with get/get_with_limits returning HttpClientError, preserving the original HttpError category, optional detailed transport error and safe endpoint context. See [network error contracts](design/STDLIB_DOMAIN_ERRORS.md#tcp-operations). Legacy modules and high-level client/stream contracts remain available.

TCP domain errors and the diagnostic HTTP client pass focused source/bytecode checks, final local make check (676s, 21 November), and relocated source/saved-bytecode execution. The subsequent 22 November repair reconciles all 25 HTTP builtin arities and local-resource call permissions in the self-hosted decoder with the native verifier. Focused legacy/diagnostic-client and routed-server disassembly checks pass. The compiler seed is refreshed; unchanged-program native oracle contracts pass. Final local make check passed in 674s (22 November), including all 427 bytecode tests, public HTTP disassembly, packaging and installed smoke. Relocated TCP and diagnostic-client source/bytecode/disassembly acceptance also passed. Integration remains pending and development CI stays off.

## AS5 core documentation and examples (23 November 2026)

Added the [ordinary/async stdlib reference](reference/STANDARD_LIBRARY.md) and
[migration guide](guides/STDLIB_MIGRATION.md), with development-versus-release
boundaries and compatibility/error/lifecycle guidance. Three standalone async
examples cover timing, temporary-file cleanup and bounded subprocesses. All pass
the named functional runner source/compile/saved-bytecode checks. The inventory
now contains 41 examples and 598 transcript assertions; its mutation checks pass,
and the updated smoke fixture compiles. Documentation links pass.

This is a focused example/documentation follow-up to validated revision
`3d4daae15f5d983a210a50b0a6cf1fdd8ba08f83`; runner edits change only registration
counts and expected transcript data. Compiler, VM, stdlib implementation and seed
are unchanged, so the prior full validation is retained for those components.
No runtime performance impact is expected; the suite gains three short examples.
AS5 remains in progress: broader examples, website adaptation/review and integration
are pending. The existing website follow-up applies; no website is changed or
published by this batch.
