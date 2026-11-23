# RM#162: useful async programs

Investigation, 6 November 2026. Baseline: alpha.15 product tree, reconciled into
`next` in PR #676. No implementation or new language contract is delivered here.
The owner selected investigation of the general async limitation, using uptime
and request counting as concrete cases. HTTP exposes the issue but does not own it.

## Reproduced findings

Using the checkout's `bin/panack` (alpha.15), without a rebuild:

- `reading = time.now()` in an ordinary `main(): Unit` checks successfully.
  The same body in `async main(): Unit` fails with
  `async function cannot call blocking function time.panack:now`.
- A callback receiving `Context(0)`, copying its `requests` field into a mutable
  local and incrementing it returns `1` on both calls. Output: `1, 1`.
- An async caller retaining a mutable local and explicitly assigning each
  callback's returned count produces `1, 2`. Local mutation and preservation
  across await already work.
- `context.requests = context.requests + 1` is rejected by the parser:
  `expected semicolon or closing brace`. Record field mutation is not supported.

These are reduced reproductions, not a recovered copy of the earlier failed
HTTP example. The HTTP source confirms the relevant boundary: `ContextualServer`
passes a context value to a handler returning `Result[Unit,Str]`. There is no
context replacement operation or general mutable cell. The native implementation
retains the context value; it does not provide user-visible mutable record fields.

## Existing capability map

| Capability | Current support |
| --- | --- |
| Pure computation, local mutation, async helpers | Supported. Async helpers use await; local state survives suspension. |
| Clock read | Existing monotonic host clock, rejected in async code. |
| Shared application state across independently invoked callbacks | No general API. HTTP has specialised internal mutable resources, not an application state primitive. |
| Network I/O | Async TCP and HTTP transports already exist. |
| Terminal/arguments/exit | Explicit async host APIs exist. |
| Filesystem I/O | Narrow async bounded reads/root identity exist; ordinary synchronous filesystem operations are not thereby async-compatible. |
| Process execution and sleep | Ordinary host APIs; no corresponding public async wrappers were found in the current standard library. |
| Ordinary helper calls | Direct helpers using pure calls and recognised local HTTP operations can be classified as async-safe. This is not general effect inference or permission through arbitrary callable values. |

## Why this happens

The compiler distinguishes pure, ordinary effectful and async functions. It also
recognises a closed set of HTTP-local operations, propagating that permission
through direct helpers. The bytecode verifier independently derives the same
permission. Other ordinary effectful calls are rejected by async callers.

The clock implementation calls `clock_gettime(CLOCK_MONOTONIC)` and constructs an
Instant. It does not wait for an external event as synchronous I/O does. Calling
all rejected ordinary effects "blocking" therefore overstates what is known.
Do not mark a clock read pure: its result depends on external state.

The counter is a state-model gap, not a clock/effect diagnostic problem. Immutable
context values cannot become shared mutable state merely by relaxing call checks.

## Historical proposal — revised during owner discussion

The [discussion record](ASYNC_DESIGN_DISCUSSION.md) supersedes the direction below:
the owner prefers async standard-library counterparts selected through `/async`
imports. General ordinary-effect permission is not the selected clock fix.
State, closures and capture rules remain open design questions.

### Original proposal

1. Generalise the existing compiler/VM permission mechanism to represent a small,
   explicit set of synchronous effects permitted during async execution. Include
   the monotonic clock and existing HTTP-local operations. Keep pure callers
   restricted, derive helper permissions, and reject blocking I/O and forged
   bytecode. Do not add a new unrestricted escape hatch or promise that arbitrary
   synchronous computation is cheap or preemptible. Settle callable-value
   behaviour explicitly; do not imply that direct-call support covers it.
2. Provide an explicit general-purpose state handle, usable as callback context
   outside HTTP as well. Prefer an update operation that accepts a pure,
   non-suspending transformation and publishes the replacement as one indivisible
   operation relative to task execution. Specify visibility, failure, lifetime
   and ownership before implementation. Do not make separate read/await/write
   operations appear atomic. Pure functions cannot create, read or mutate the
   handle; payload admissibility must prevent hidden mutation through pure values.
3. Deliver an HTTP example with startup time and a count of admitted handler
   invocations since process start, including the current healthcheck request.
   This is not a durable lifetime count across restarts. Use a standalone async
   callback example to prove that the capability is not tied to HTTP.

The state API and effect details are proposals, not accepted syntax. A small
counter-specific primitive would be easier but would leave the general callback
state problem unsolved. Assess the generic handle's implementation cost before
committing to it; do not silently expand into actors, locks, channels, mutable
records or a complete new effect system.

## Acceptance boundary

- Uptime uses monotonic elapsed time, handles clock failure, and does not decrease.
- Repeated and overlapping requests observe increments without lost updates.
- State update semantics remain explicit around await, failures and cancellation.
- Pure-code restrictions, synchronous blocking-I/O restrictions and hostile
  bytecode checks remain enforced. Check both source and saved-bytecode execution.
- Existing HTTP resources and direct helper behaviour remain compatible.
- Diagnostics explain the actual unsupported effect; guides show both examples.
- Record focused startup/runtime costs for the new capabilities, without repeating
  the already accepted alpha.15 coverage/performance work.

Additional async filesystem/process/sleep capabilities need their own concrete
use cases and bounded implementation; they are gaps in the map, not automatically
selected deliverables. Native adapter/compiler decoupling remains separate.
No website change. No CI was run; local reduced programs and documentation checks
supply investigation evidence only, not acceptance of the proposed implementation.

## Source evidence

- [Async contract](reference/SPEC.md#bounded-asyncawait-interface)
- [Monotonic clock contract](reference/SPEC.md#monotonic-instants)
- [Compiler permission propagation](../src/compiler/module_effects.panack)
- [Bytecode effect verification](../src/vm/verify.c)
- [Clock implementation](../src/vm/host_types.c)
- [HTTP context and handler signatures](../packages/http/src/server.panack)
- [Internal HTTP runtime bridge](../src/stdlib/http_runtime.panack)

## Implementation follow-up, 8 November 2026

The [approved batch](ASYNC_DESIGN_DISCUSSION.md#implementation-batch-selected-8-november-2026)
implements explicit async clock and environment APIs plus consistent import paths
for existing async host/filesystem operations. The [uptime service](../examples/network/http_uptime/README.md)
uses a start instant carried in immutable context. The historical alpha.15
findings above remain evidence of the original restriction, not a description of
the development branch after this batch. The counter/shared-state gap remains
open; this implementation does not relax the ordinary-call effect boundary.
