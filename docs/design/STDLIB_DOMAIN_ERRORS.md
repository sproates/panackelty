# Standard-library domain errors

21 November 2026. RM#162: Real-program fixes. Development branch contract;
not a claim about the released alpha.15 library.

## Shared rules

Ordinary and async operations return the same nominal error type. Domain modules
re-export their error type and kind; the corresponding errors module owns the
record, enum, pure conversion and render method. Callers can inspect kind without
parsing a message, and can use render() for a complete human-readable explanation.
For host-backed APIs, the native field retains the original HostError unchanged:
operation, code and native_code. HTTP streams and TCP retain their native transport Str,
including available libcurl detail, as specified below. Unknown codes map to OtherFailure; they are not silently relabelled
as a specific known cause. Native numeric errors remain available, not guessed
from platform-specific errno values. Render appends a nonzero host error number.

Console, sleep, process, filesystem, SHA-256 and gzip errors are implemented in Panack. Native ABI,
worker ownership, limits, cancellation and successful result values do not change.
Canonical HTTP stream errors preserve detailed TLS causes; TCP errors and the opt-in high-level HTTP async client are specified below.

## Console

console and console/async return ConsoleError from all five operations. Read
returns Result[Option[Bytes],ConsoleError]; writes return Result[Unit,ConsoleError].
ConsoleOperation is Read(Nat), Write, WriteLine, WriteError or WriteErrorLine.
ConsoleError stores operation, kind and native. Read carries the requested byte
limit. Output errors do not retain or print the message that failed to write.

| Native code | ConsoleErrorKind | Explanation |
| --- | --- | --- |
| out_of_range on read | InvalidReadLimit | Require 1 through 65,536 bytes |
| out_of_range on write | OutputTooLarge | Message exceeds 16,384 UTF-8 bytes before the optional newline |
| busy | ReadBusy | Another stdin read still owns the single reservation |
| io_error | IOFailure | Host could not complete I/O; native detail is retained |
| resource_limit | ResourceLimit | Worker admission/allocation/startup failed |
| unsupported_host | UnsupportedHost | Host lacks the operation |
| Any other code | OtherFailure | Unknown failure, preserved without speculation |

For console.read(0), render returns:

> Cannot read standard input (requested limit: 0 bytes): the read limit must be from 1 through 65536 bytes. Choose a limit within that range.

For an oversized standard-output message:

> Cannot write standard output: the message exceeds 16384 UTF-8 bytes before the optional newline. Write smaller chunks.

## Sleep

Both time.sleep and time/async.sleep return Result[Unit,SleepError]. SleepError
stores duration, kind and native. Clock reads retain their existing ClockError;
a failed sleep is not a failed clock read by definition.

SleepErrorKind is NegativeDuration, DurationTooLarge, TimerFailure,
UnsupportedHost or OtherFailure. negative_duration and out_of_range map to the
first two. io_error or clock_unavailable maps to TimerFailure, which does not
claim whether the failure occurred while reading a clock or waiting. The host's
365-day maximum remains; zero succeeds. Timer limits can also be constrained by
the host deadline representation.

For a negative duration:

> Cannot sleep for -1 nanoseconds: the duration is negative. Use zero or a positive duration.

## Processes

Both process.run and process/async.run return Result[ProcessOutput,ProcessError].
ProcessError retains executable, working_directory, timeout, output_limit, kind
and native. It does not retain arguments, environment values, stdin or partial
output. A normal nonzero exit or signal remains a successful ProcessOutput.

| Native failure | ProcessErrorKind |
| --- | --- |
| negative_duration; zero timeout; out_of_range with timeout above 365 days | InvalidTimeout |
| Other out_of_range | LimitExceeded |
| invalid_argument | InvalidArgument |
| invalid_environment | InvalidEnvironment |
| launch_failed | LaunchFailed |
| timeout with a positive timeout | TimedOut |
| output_limit | OutputLimitExceeded |
| clock_unavailable | ClockUnavailable |
| resource_limit or out_of_memory | ResourceLimit |
| cancelled | Cancelled |
| Known host I/O/path/permission failures | HostFailure |
| unsupported_host | UnsupportedHost |
| Other code | OtherFailure |

For a zero timeout:

> Cannot run process '/bin/sh' with working directory '/': the timeout must be positive and within the supported range. Choose a timeout greater than zero and no longer than 365 days.

For a 10 ms timeout that actually elapsed:

> Cannot run process '/bin/sh' with working directory '/': the process did not complete within the requested 10000000 nanoseconds. The process group was stopped and partial output is not returned.

A launch failure identifies both executable and working directory. The native
adapter cannot always distinguish which failed, so render suggests checking both
and their permissions. It does not assert that the executable is missing.
Limits that the native adapter groups together remain LimitExceeded; the wrapper
does not invent a more precise cause. Cancellation-before-launch may produce no
process, so Cancelled does not claim a child was started or killed.

## Migration and acceptance

The new canonical alpha.16 APIs now use their domain errors. Replace annotations
using console.HostError, time.HostError for sleep, or process.HostError for the
Command API with ConsoleError, SleepError or ProcessError. Replace direct native
code/operation/native_code access with error.native.code and the corresponding
native fields. Prefer error.kind for application decisions. A deliberately
compatible adapter can return error.native without losing any host detail.

Released host.sleep, legacy host output and process/legacy positional APIs retain
their existing error contracts. testing/reporting returns Result[Nat,ConsoleError].
The testing/process compatibility helper explicitly unwraps ProcessError.native
because its historical command_result/expect_command_error contract uses HostError.
No existing error identity is recreated under a new import spelling.

Focused tests exercise real console bounds, invalid and elapsed timers, process
launch/environment/output-limit failures, nonzero successful exits and unknown
native-detail preservation. They check exact representative rendering and shared
types in ordinary/async source and saved bytecode. Final local make check passed in 621 seconds (unit 385s, bootstrap 82s, functional
167s), including packaging and installed smoke. A relocated installation with
spaces in its path passes the complete error fixtures and unrelated-directory
source, saved-bytecode and disassembly checks. Ordinary and sanitizer artifact
checks pass. The first full run caught three omitted installation-inventory entries;
those entries were corrected and the complete packaged source set was checked
before the passing run. Integration remains pending. No compiler or VM source change is
required; a compiler candidate is byte-for-byte identical to the saved seed.

## Filesystem

Canonical filesystem, filesystem/async, filesystem/confined and its async twin,
and filesystem/staging and its async twin return FilesystemError. The shared
record contains operation: FilesystemOperation, kind: FilesystemErrorKind and
native: HostError. All error construction and rendering is Panack code.

Operations retain caller-supplied paths, without performing another filesystem
lookup. Metadata, writes, listing, removal and creation retain the path; temporary
creation retains the parent. Read retains path/limit; ReadChunk adds offset;
DirectoriesOverlap retains both paths. RootIdentity retains the root; ReadConfined
retains root, relative path and limit; ReadLink retains root and relative path.
CreateStage retains its parent. StagePath and StageDiscard have no path available.
StageCreateDirectory retains its relative path; StageWriteChunk retains relative
path and offset; StageCreateSymlink retains relative path and stored target;
StagePublish retains name and whether replacement was requested. Errors do not
retain file contents or the opaque stage handle. Relative paths are labelled as
relative; no diagnostic pretends to know an unavailable absolute path.

| Native code | FilesystemErrorKind |
| --- | --- |
| not_found | NotFound |
| permission_denied | PermissionDenied |
| already_exists | AlreadyExists |
| not_directory | NotDirectory |
| is_directory | IsDirectory |
| not_regular_file | NotRegularFile |
| not_empty | DirectoryNotEmpty |
| symlink_loop | SymlinkRejected |
| output_limit | OutputLimitExceeded |
| out_of_range | LimitExceeded |
| invalid_argument | InvalidArgument |
| invalid_path | InvalidPath |
| invalid_parent | InvalidParent |
| invalid_handle | InvalidStage |
| busy | StageBusy |
| parent_changed | ParentChanged |
| stage_changed | StageChanged |
| destination_conflict | DestinationConflict |
| publication_failed | PublicationFailed |
| cleanup_failed | CleanupFailed |
| create_failed | StageCreationFailed |
| open_failed | StageOpenFailed |
| filesystem_error | FilesystemFailure |
| io_error | IOFailure |
| resource_limit | ResourceLimit |
| out_of_memory | OutOfMemory |
| cancelled | Cancelled |
| unsupported_host / unsupported | UnsupportedHost |
| Any other code | OtherFailure |

Examples, with native numbers omitted where unavailable:

- Cannot read metadata for '/work/app/config.json': the path or a required path component could not be found. Check the spelling and that the required files and directories exist. Host error number: 2.
- Cannot get the staging directory's path: the staging handle is no longer usable. Use a live stage that has not been published or discarded.

NotFound does not claim that the final component is missing; an ancestor may be
missing. SymlinkRejected covers both refused traversal and a link-loop limit.
DestinationConflict covers an inspection failure as well as an ineligible target,
because the native adapter does not distinguish those cases. FilesystemFailure
is deliberately broad: bounded reads, atomic writes and link/staging operations
can return filesystem_error plus a native number without a portable category.
The renderer does not guess errno meanings. Ordinary and async host validation
can differ: an async request can be rejected as out_of_range before dispatch,
where an ordinary bounded operation reports filesystem_error. Native detail is
preserved in either case. No new promise of identical native failure ordering is
made. Unknown failures preserve all native fields.

Cancellation, confinement, ownership, limits, cleanup and publication semantics
are unchanged. A failed write does not promise no side effects; a failed cleanup
does not promise that nothing remains. A completed publication is not relabelled
as a failure if cleanup subsequently warns.

### Filesystem migration

Use filesystem/legacy and filesystem/legacy/async for the previous HostError
result contract. These thin adapters call the canonical implementation and unwrap
native. FileKind/FileMetadata retain their native identities. Existing bounded_filesystem,
package_filesystem, async_filesystem and numeric filesystem_stage APIs remain.
Compatibility read_beneath exports and the old root_identity export from
filesystem/async retain HostError; use filesystem/confined for the canonical
root_identity/read/read_link interface. Canonical temp_file/temp_directory aliases
now return FilesystemError, just like create_temp_file/create_temp_directory.
Compiler, package-tool and existing testing-helper callers explicitly retain the
compatibility contract. The prelude filesystem namespace uses the canonical type.

The compiler now gives ordinary stdlib declarations distinct module identities,
as it already does for project declarations. Core and explicitly owned native ABI
names remain unchanged. This prevents same-named enum constructors and records
in different stdlib modules from replacing one another during emission. The
bytecode format and VM are unchanged; the compiler seed is refreshed once for
this batch.

Final local make check passed in 628s (unit 393s, bootstrap 85s, functional 167s, 21 November), including packaging and installed smoke. Relocated ordinary/async source, saved-bytecode and unrelated-directory acceptance passes. Integration remains pending; development CI stays off.

The first final run passed unit/bootstrap but exposed a fixture setup error:
the public launcher deliberately selected its own stdlib instead of an environment
override. The regression now creates an isolated toolchain and passes through the
public launcher in both default and candidate modes. The full run was repeated
after that test-only correction; no product or seed change was needed.

## SHA-256 and gzip resources

Canonical digest/sha256 and /async return Sha256Error from create, update, finish
and close. compression/gzip and /async return GzipError from create_decoder,
decode_chunk and close. Both families re-export their error, kind and operation
types; their errors submodules own conversion and rendering. Each error stores
operation, kind and the original native HostError. No input bytes or resource
handles are retained in an error. The pure hash(Bytes) operation is unchanged:
it returns Bytes, accepts at most 16 MiB and traps above that existing bound.

Sha256Operation is Create, Update(Nat byte count), Finish or Close.
GzipOperation is CreateDecoder(Nat output limit), DecodeChunk(Nat input byte count,
Bool final input) or Close. A decode error cannot recover the configured total
budget from an opaque decoder, so it does not invent a limit value. All operation
context comes from the requested call, without another native operation.

| Native failure | SHA-256 category |
| --- | --- |
| invalid_handle | InvalidContext |
| invalid_argument on an update larger than 65,536 bytes | ChunkTooLarge |
| other invalid_argument | InvalidArgument |
| digest_unavailable | InitializationFailed |
| digest_failed | DigestFailed |
| resource_limit | ResourceLimit |
| unsupported / unsupported_host | UnsupportedHost |
| Other code | OtherFailure |

| Native failure | Gzip category |
| --- | --- |
| invalid_handle | InvalidDecoder |
| invalid_argument on a chunk larger than 65,536 bytes | ChunkTooLarge |
| invalid_argument or output_limit when creation requests more than 256 MiB | UnsupportedOutputLimit |
| output_limit during decoding | OutputLimitExceeded |
| decoder_finished | DecoderStopped |
| invalid_gzip | DecodeFailed |
| truncated_gzip | IncompleteInput |
| decoder_unavailable | DecoderUnavailable |
| other invalid_argument | InvalidArgument |
| resource_limit | ResourceLimit |
| unsupported / unsupported_host | UnsupportedHost |
| Other code | OtherFailure |

Native output_limit at creation denotes an unsupported configured limit. The
same code during decoding means the chosen budget was exhausted. An oversized
Nat rejected before native integer conversion is also UnsupportedOutputLimit.
A zero-byte output budget is allowed and can decode an empty gzip member.

Oversized chunks are rejected before modifying the resource: callers may retry
with smaller chunks. A digest library failure invalidates its context; finish
consumes the context even when finalisation fails. A gzip decode failure,
truncation or exhausted budget stops further decoding, but the decoder still
needs closing. Successful final input also stops decoding. DecoderStopped does
not claim which history occurred: the native decoder_finished code conflates
successful completion and a previous failed step. Closing an already closed
resource is a different InvalidContext/InvalidDecoder failure.

The native invalid_gzip code covers zlib decoding failures without preserving
the specific zlib status. DecodeFailed therefore does not claim corruption is
proven. Similarly, initialisation errors do not claim a missing library or an
allocation failure without evidence. Native detail remains unchanged, including
zero when no native error number is provided. Unknown codes remain available.

Resource quotas count creations within an invocation, including finished/closed
resources: 65,536 SHA-256 contexts and 64 gzip decoders, shared with their legacy
numeric APIs. Closing resources releases native state but does not replenish
these quotas. ResourceLimit can also represent allocation failure. Rendered
guidance does not incorrectly promise that closing a handle enables another
creation in the same invocation.

Examples:

- Cannot update a SHA-256 context with 65537 bytes: an update accepts at most 65536 bytes. Split the input into smaller chunks and update the context with each chunk.
- Cannot create a gzip decoder with an output limit of 268435457 bytes: the output limit must be at most 268435456 bytes (256 MiB). Choose a supported limit; zero permits no decompressed output.

Legacy stdlib/digest and stdlib/gzip retain numeric handles and HostError results.
Canonical typed-resource callers migrate error annotations and read native.code
where they previously used code, or branch on the new typed kind. Successful
values, bounds, invocation ownership, bounded async steps, cancellation and native
OpenSSL/zlib implementations are unchanged. No compiler/VM/bytecode-format change
is involved; the compiler candidate and checked stdlib oracle remain identical.

Focused ordinary/async source and saved-bytecode checks pass. Final local make check passed in 645s (21 November), including unit, functional, bootstrap and packaging checks. Relocated installation with a spaced path passed the ordinary/async resource and compatibility fixtures. Integration remains pending and development CI stays off.

## HTTP streams

Both network/http_stream modules return HttpStreamError with operation, kind and
native (the original transport Str). HttpStreamOperation is Open(host, port, tls,
response_limit, timeout), Read or Close. Open validates configuration but does not
complete a network connection: DNS, connection and TLS failures normally surface
on the first read. Read/Close do not invent endpoint information absent from the
opaque handle. No request bytes or handle are retained in the error.

Categories distinguish InvalidArguments, TimedOut, NameResolutionFailed,
TlsVerificationFailed, TlsFailed, ConnectionFailed, WriteFailed, ReadFailed,
ResponseLimitExceeded, InvalidStream, ReadBusy, ResourceLimit, Unavailable,
ResolverUnavailable, ClockFailed, InitializationFailed, PollingFailed,
InvocationEnded and OtherFailure. Classification matches an exact native category
or that category followed by a newline and native detail; arbitrary shared prefixes
are not classified as known errors. Unknown native text is retained unchanged.

The native typed transport now retains libcurl's bounded error buffer and numeric
CURLcode for connection, write and read failures, falling back to libcurl's error
text when the buffer is empty. The original category precedes the detail. Details
are copied before the easy handle is cleaned up; the buffer remains alive for the
handle's lifetime. Error text can vary by TLS backend and libcurl version and is
not an application matching API. This is a VM implementation change, without a
new intrinsic signature, compiler seed or bytecode version. Numeric compatibility
streams and the legacy exchange transport retain their previous strings.

For example, a hostname mismatch is rendered as:

> Cannot read the HTTP stream: the server’s TLS identity or certificate could not be verified. Use the native details to identify whether the hostname, certificate or trust configuration needs correcting.
> Native details: HTTP TLS verification failed
> libcurl error 60: [the TLS backend's specific hostname-mismatch explanation]

The bracketed line describes variable native output, not literal emitted text.
Verification remains enabled. No speculative clock advice or unsafe retry promise
is added. WriteFailed warns that some request bytes may already have been sent.
Errors do not close a stream automatically; close it when finished, except that
cancellation invalidates the stream. ResourceLimit documents the existing 512
cumulative stream slots; closing a stream does not replenish that quota.

Migration: canonical callers replace Result[...,Str] with HttpStreamError and use
problem.render() when deliberately converting to a textual callback error. Prefer
kind for decisions and preserve native for diagnostics. The HTTP package still
uses compatibility transports; its public error model has not changed in this
batch. TCP domain errors and high-level HTTP migration remain outstanding.

Real certificate fixtures exercise trusted success, hostname mismatch and an
untrusted CA in ordinary/async source and saved-bytecode modes. They verify that
the two failure causes produce distinct native details without pinning backend
wording. Pure tests check unknown-prefix safety, native preservation and success.
HTTP stream domain errors pass focused real-TLS, ordinary/async and compatibility checks, final local make check (685s, 21 November), and relocated-install real TLS source/bytecode acceptance. Compiler seed and unchanged-program bytecode artifacts remain unchanged. Integration remains pending and development CI stays off.

## TCP operations

network/tcp/async returns TcpError with operation, kind and native (the original
runtime Str). Operations retain exchange address/port/deadline/response limit,
server address/port, or the chunk-read limit. Invalid Duration configuration is
reported as InvalidDuration before network admission; the native text names the
field and its supported whole-millisecond range. Legacy tcp/tcp_service contracts
remain unchanged. Handler and readiness callbacks still return Result[...,Str].

Exchange and bounded-server setup distinguish invalid arguments/address, timeout,
connection/socket/bind/listen/accept/read/write/shutdown/poll/clock failures,
peer closure, byte limits, unavailable host and unknown failure. Unknown text
remains OtherFailure. Native errno is not provided by this runtime; the adapter
does not claim otherwise or infer a missing port/permission cause.

Finite-server client outcomes are Result[Unit,TcpError] with ClientFailure on
failure. The native runtime mixes handler-returned strings with client transport
strings, so these are not classified by their text. Continuous-service failures
similarly use ServiceOrCallbackFailure because readiness callbacks can supply the
same text as a native failure. Tests deliberately return “TCP server bind failed”
from a readiness callback and “TCP timed out” as a client result to verify no
false native classification. Improving that distinction requires native provenance
and is not secretly supplied by a string-matching heuristic.

When forwarding read_chunk errors from a callback whose contract is Str, use
problem.render() for explanation or problem.native for exact legacy text. Domain
errors retain no request/response contents. Server summaries and successful values
remain unchanged. No synchronous TCP server is introduced.

## High-level HTTP diagnostic client

The HTTP package additionally exports http/client/async, with get and
get_with_limits and the existing ClientLimits/Response types. Results carry
HttpClientError: kind is the existing HttpError, transport is an optional
HttpStreamError, and endpoint is an optional scheme/host/port string. The endpoint
excludes path, query, userinfo and request headers. Invalid URLs/configuration do
not cause network I/O. render() includes the endpoint when known and the detailed
transport explanation, or the parser/configuration explanation when no transport
failure occurred. Applications can inspect kind without parsing native text.

This client uses typed async streams, closes its handle on success or error, and
inherits cancellation invalidation while a read is pending. Response accumulation
remains bounded to 16 MiB. Chunks use binary-carry assembly with at most 25 slots, avoiding both a full-prefix
copy on every read and an unbounded list of one-byte chunks. Existing response framing/parsing and ClientLimits semantics
are reused. The typed transport also limits request bytes to 65536; an oversized
request is reported with its original InvalidArguments transport category.

The released http/client, http/stream and HttpError contracts are retained. The
blocking streaming client and package manager remain compatibility consumers;
they do not acquire detailed TLS errors simply because the new module exists.
This is an opt-in migration path, not a claim that every legacy consumer has
migrated. Migration guidance and the public examples/reference remain explicit
follow-up work. Replacing every legacy consumer is not an alpha.16 acceptance
requirement; compatibility APIs deliberately remain available. New code can import http/client/async as http without introducing
getAsync-style names.

TCP domain errors and the diagnostic HTTP client pass focused source/bytecode checks, final local make check (676s, 21 November), and relocated source/saved-bytecode execution. An additional disassembly check remains blocked by a pre-existing http_body arity-table mismatch, reproduced with the legacy client and previous package manifest. Compiler seed and unchanged-program oracle artifacts remain unchanged. Integration remains pending and development CI stays off.

### Known disassembler parity gap

The relocated diagnostic client runs as source and saved bytecode, but the extra
`panack disasm` check reports `bytecode call to http_body has invalid arity`.
The same failure was reproduced with the unchanged legacy client, the previous
HTTP manifest and the unchanged compiler seed. The native VM accepts and runs
both artifacts. `src/bytecode/decoder.panack` uses the compiler's builtin arity
table, which does not match the native HTTP builtin table for this case. This is
an existing inspection-tool limitation, not a claimed passing acceptance check.
The next bounded repair is to reconcile the HTTP builtin arities and add a
package-bytecode disassembly regression before integration acceptance.
