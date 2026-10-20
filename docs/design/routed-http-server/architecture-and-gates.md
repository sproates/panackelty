# Implementation boundaries and decision gates

Read the [contract](contract-snapshot.md) before this document. Internal designs
here are proposals to verify in P02, not evidence that the runtime already supports
them. A less experienced implementer should stop at an unmet gate, not substitute
an easier API.

## Source map

| Area | Inspect first | Why |
| --- | --- | --- |
| Public package | `packages/http/panack.json`, `src/server.panack`, `src/service.panack`, `src/types.panack`, `src/parser.panack` beneath that package | Current exports, shared client types, limits and framing |
| Existing consumers | `packages/static_http/src/service.panack`, `tools/http-server/src/main.panack`, their manifests | Context, filesystem grants, host option and shutdown |
| Native service | `src/vm/tcp_server.c`, `tcp_server.h`, `tcp.c`, `host.c`, `execute.c`, `vm.h` | Connection ownership, scheduling, traps and completion |
| Value lifetime | `src/vm/value.h`, `value.c`, `program.c`, `verify.c` | Retention, copied values, forged bytecode and program lifetime |
| Async model | `src/vm/tasks.h`, `tasks.c`, `src/stdlib/tcp_service.panack`, `src/stdlib/tcp.panack` | Cancellation and wait boundaries; fake-host tasks are not a production socket loop |
| Checked APIs | `src/compiler/owned_intrinsics.panack`, `module_signatures.panack`, `module_bodies.panack`, `module_emission.panack` | Owner identity, generic signatures, effects and native lowering |
| Existing tests | `tests/unit/vm/tcp_server.c`, `tests/unit/vm/tasks.c`, `tests/http_package.sh`, `tests/continuous_service.sh`, `tests/static_http.sh`, `tests/http_client_package.sh` | Existing guarantees and fixture prerequisites |

Paths in this table are starting points. P01 must record actual declarations and
callers with searches; a file name alone is not a design. No new declaration named
in this pack should be assumed to exist before its packet creates it.

## Layer boundaries

Keep route URI validation, routing decisions, header access and response semantics
in the HTTP package where possible. Native code owns sockets, opaque identity,
lifetime, task admission/cancellation and the safe recovery boundary. The compiler
checks concrete handle and callback types and selects operations by checked owner
identity. It must never special-case arbitrary source spellings `get`, `put` or
`send`. HTTP code must not know static paths, MIME tables or filesystem roots.

Proposed handle representation: a runtime-owned registry with non-forgeable typed
handles, explicit retention and stale-generation detection. Server references
alias the same server state; they must not behave like persistent route records.
Response writers identify one request, and become invalid outside that request's
lifetime. This requires an explicit ownership design, not global raw pointers or
serialisable numeric IDs. Native registry storage must preserve the checker's
concrete context/callback type association and retain values under existing value
semantics. Do not cast away `T`, use a public `Any` container or introduce closures.

P02 must demonstrate how these handles are declared and exported through a package
while preserving canonical owner identity. If the current language cannot express
that representation safely, document the smallest runtime/compiler extension and
ask the owner before expanding language scope. The simple direct registration
syntax is already agreed; do not ask to replace it with reassignment or Results.

## State tables

| Server state | Registration | listen | close | destroy |
| --- | --- | --- | --- | --- |
| Created | Record valid routes or sticky setup error | Validate setup/address/port, bind, resolve readiness | Close without binding | Destroy without binding |
| Listening | Trap; no route change | Lifecycle error | Stop admission, bounded drain | Stop admission, immediate cancellation |
| Closing | Trap; no route change | Lifecycle error | Idempotent completion | Escalate to immediate cancellation |
| Closed | Trap; no route change | Lifecycle error | Success | Idempotent resource release |
| Destroyed | Trap; no route change | Lifecycle error | Success | Success |

The contract requires resource release and idempotence; whether a destroyed shell
remains allocated while referenced is an internal ownership choice. Do not free
memory still reachable through aliases. Test repeated operations and stale handles.

| Writer phase | Allowed action | Failure result |
| --- | --- | --- |
| Open | Valid status/header changes, one valid send | Invalid input traps; request becomes generic 500 |
| Completed, buffered | Server flush only | Later mutation/send/error/trap is logged; retain first valid response |
| Flushed or request ended | No application access | Reject stale use safely; never write again |

`send` validates and snapshots the complete response before marking it completed.
It normally buffers, rather than immediately writing to the socket. No partial
application response may escape when size/status/header validation fails.
Generated framing participates in the 16 KiB header-block count.

## Fixed resource ledger

- Bind: simple constructors `0.0.0.0`; explicit forms use validated IPv4 address.
- Four admitted requests, separately from sixteen open connections.
- No request queue when the four slots are occupied. Reject with 503, no handler.
- 1,048,576 bytes each for request and response bodies.
- 16,384 bytes each for the complete request head and response header block.
- Ten seconds from accept through response completion, including head/body reads,
  interim response, handler and final write. One monotonic deadline, not a fresh
  timeout at each phase.
- Close drain: at most one second; destroy has no drain.

P02 must account for buffers, connection slots, handler slots, queued host
operations and ownership at every phase. Rejection must not allocate an unbounded
queue of sockets or tasks. Derive an upper bound including sixteen head buffers,
four body/writer buffers and bounded rejection work; measure allocation and cleanup
under saturation rather than claiming that four handlers alone bounds memory.

## Decisions that require evidence or owner clarification

| Gate | Question | Required disposition |
| --- | --- | --- |
| G1 | How do package-owned generic opaque handles retain typed callbacks/context? | P02 implementation design and compiling prototype; ask before language expansion |
| G2 | Which runtime faults are safely request-recoverable? | Enumerate structured categories and cleanup obligations; never classify by diagnostic substring or clear every VM error |
| G3 | A response completes send before the deadline, but the client cannot drain it by the deadline | Issue requires flushing while also bounding total completion; ask owner to define write allowance/cutoff before deadline implementation |
| G4 | A seventeenth connection must receive 503 while no more than sixteen remain open | Specify whether a transient rejection socket counts; ask owner rather than quietly raising the cap |
| G5 | No framing header, but bytes arrive after an apparently complete empty request | Define the observation boundary and one-request-close behaviour with the owner; do not wait indefinitely or claim future bytes can be detected |
| G6 | Listening keeps the process alive after main returns, while normal exit gracefully closes it | Define explicit exit versus main completion and signal semantics; preserve resource liveness and ask about any uncovered termination path |
| G7 | Existing preview bind options versus fixed-address constructor | Resolved in GI#518 on 20 October: explicit-address constructors; preserve preview loopback and --host |
| G8 | Which preview size/header options can coexist with fixed transport limits? | Inventory options in P01; preserve compatible application limits, ask before dropping or reinterpreting conflicting options |

These gates do not authorise weakening the issue. P01 collects one concise decision
request for unresolved observable behaviour. Only the affected packet is blocked;
continue independent inventory, pure tests and documentation. Record the owner's
answer in GI#518 and update this pack before implementing it. Do not mark a gate
resolved because a prototype chose one interpretation.

## Recovery and cancellation rules

Recovery is at a request execution boundary. Retain the server and sibling requests;
release only the failed request's stack, waits and owned resources. A completed
writer response survives a recoverable failure. Errors never leak into the client
body. OOM, integrity faults, unsafe host failures and process termination stay fatal.
Cancelled operations must not deliver into recycled slots; use identity/generation
checks and quiesce producers before release. Test cancellation before and after
host completion, repeated cancellation and allocation failure during cleanup.

A broad catch-all in `tcp_server_advance` is not an acceptable implementation.
Existing low-level TCP behaviour must remain unchanged unless a separately agreed
compatibility decision says otherwise. Do not change saved-bytecode ABI by accident;
P02 must decide whether new operations require new verification/seed work and show
how existing artifacts remain handled.

## Implementation evidence (20 October 2026)

The internal runtime trial now uses reference-counted opaque shells rather than a
numeric registry. No shell can be reconstructed from a language integer. Every
request gets a fresh connection token and writer; client-slot reuse cannot revive
an expired token. The package entry point remains private until consumer migration
and public acceptance are complete.

| Object | Owns | Release boundary |
| --- | --- | --- |
| Server shell | Immutable context, named callbacks and route keys | Last language/resource reference |
| Active listener | Server shell, native listener and bounded client slots | close drain, destroy, termination or fatal failure |
| Connection token | Writer and bounded request-body buffer | Last reference; borrowed client/execution pointers invalidated before slot reuse |
| Writer shell | Header draft and first committed wire snapshot | Last reference; mutation rejected after completion or request end |
| Handler execution | Selected callback frame and retained arguments | Normal completion, request failure, disconnect, deadline or shutdown |

`HttpServerState[T,H]` preserves the context/callback relationship in the checker.
`HttpConnectionState[Request,ResponseWriter]` lets one non-generic package worker
invoke the selected ordinary or contextual callback without closures, generic
function references, or an erased public context. Listen checks the worker and
callback signatures together. Native invocation rechecks function kind and arity.

Direct registration and writer methods require a narrow local-resource effect
permission. The compiler infers this only for direct wrappers around a closed set
of HTTP state operations and pure calls. Blocking I/O and indirect calls disqualify
a wrapper. Pure functions still cannot mutate resources. Saved-bytecode verification
independently recomputes this permission; it is not a trusted serialized flag.
There is no bytecode-version change or general mutable-cell feature.

The implementation follows these concrete boundary interpretations:

- **G3:** A valid send commits one immutable response and permits flushing while the
  handler continues. The ten-second accept-to-completion deadline still cancels
  the handler and closes the socket. At cutoff only a nonblocking best-effort flush
  is possible; a slow reader cannot extend the deadline.
- **G4:** Sixteen retained service connections are allowed. An additional accepted
  socket is used only for one immediate best-effort 503 and close, with no handler
  and no queue.
- **G5:** Bytes already buffered after an unframed request head cause 400. An empty
  buffered remainder means an empty body and one final response followed by close;
  future bytes cannot retroactively change that decision.
- **G6:** Main returning leaves active listeners alive. Explicit host exit and
  SIGINT/SIGTERM request bounded drain; fatal VM errors take the fatal cleanup path.
- **G8:** Preview migration retains explicit IPv4 binding, its loopback default and
  stricter application policies. The generic server owns transport limits; preview
  does not introduce public tuning knobs for them.

Focused evidence is in `tests/unit/vm/http_state.c`, the internal routed package
fixtures, and the `routed`, `routed-capacity`, and `routed-deadline` raw-peer modes
in `tests/http_client.c`. These cover aliasing, immutable first send, typed callback
invocation, local-effect rejection, readiness after main returns, real framing and
Expect exchanges, request recovery, separate admission caps, and the real deadline.
This is implementation evidence, not final acceptance of all packet rows. Public
imports, installed consumers, static migration and complete validation remain gates.

### Maintained caller migration inventory

| Current surface or caller | Required migration | Evidence location |
| --- | --- | --- |
| `http/server`: `serve`, `serve_with_context`, `ConnectionContext`, connection policies and serializer | Replace exported server module with routed constructors, types and lifecycle; move any shared response policy behind package internals | `packages/http/src/server.panack`, package manifest |
| `http/service`: transport-oriented limits, readiness and summary | Remove obsolete public export after callers migrate | `packages/http/src/service.panack`, package manifest |
| Shared legacy `Request(target)` and `Response` | Keep HTTP-client nominal types intact; explicit adapters for routed `Request(path,query)` | `types.panack`, `request.panack`, `client.panack` |
| Static-preview service | Contextual fallback, GET-only application policy, retained filesystem identity/confinement and stricter content limits | `packages/static_http/src/service.panack`, package tests |
| Preview command | Preserve loopback/`--host`, root validation and readiness; replace transport tuning with application limits | `tools/http-server/src/main.panack`, `tests/static_http.sh` |
| Network example | Ordinary routed API and executable documentation | `examples/network/http_server/` |
| Generated package consumers | Replace low-level HTTP server/service fixtures while retaining separate native TCP tests | `tests/http_package.sh`, `tests/continuous_service.sh`, `tests/async_filesystem.sh`, `tests/namespace_seed.sh` |
| Installed artifacts and guides | Update file inventory, package README, async/HTTP guide and relocated consumer checks | `tests/unit/harness/installed-files.txt`, package/archive tests, `docs/guides/ASYNC_AND_HTTP.md` |

Historical completed plan records are not maintained runtime consumers. They may
retain historical API names; current public examples and exported APIs may not.

### Runtime-slice validation

On 20 October, `make regenerate-seed` verified compiler and standard-library fixed
points. `PANACK_HTTP_CLIENT=... PANACK_NATIVE_MODULE_TEST=... sh tests/http_package.sh`
passed after the nested-program ownership and separate body-limit fixes. The
unchanged product tree then passed full `make check` in 629 seconds, including
packaging and relocated consumers of the existing public API. The timing-budget
warnings were advisory; the command exited successfully. Exact-head CI remains
required before merge. This does not accept the unfinished public migration rows.
