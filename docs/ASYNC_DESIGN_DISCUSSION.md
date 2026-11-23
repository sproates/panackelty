# Async usability: findings, preferences and open design questions

Discussion record, 6 November 2026. Related work: RM#162, real-program fixes.
This records the owner's preferences and the reasoning behind them. It is not a
final language specification or approval to implement every idea below.

## Why we started

Two small HTTP examples exposed limitations: a healthcheck reporting uptime and
an endpoint counting requests. The broader problem is what async functions can
do. HTTP routes requests to async handlers, but useful async programs elsewhere
need the same capabilities.

The [investigation](ASYNC_USEFUL_PROGRAMS_INVESTIGATION.md) records source evidence
and reduced reproductions against alpha.15. The findings below describe that
baseline; the implementation and expanded programme sections record later work.

- `time.now()` works in ordinary functions but is rejected in async functions.
- It reads a monotonic clock and returns `Result[Instant,ClockError]`. Subtracting
  instants gives a Duration. Uptime requires a startup instant and a later reading;
  the startup instant can be immutable, so uptime does not require shared mutation.
- The compiler's "blocking function" diagnostic conflates an ordinary effectful
  function with one that actually waits. Clock reads are impure but need not wait.
- Async functions can call pure functions, await async functions, and invoke a
  recognised set of HTTP-local operations and qualifying direct helpers. The
  bytecode verifier independently enforces the effect boundary.
- Local mutable variables work across await. Independent callbacks receiving an
  immutable context do not thereby share an updating counter: reduced calls
  returned `1, 1`; explicitly threading the updated count returned `1, 2`.
- HTTP contextual handlers receive a context value, with no general application
  state update API. Relaxing clock/effect checks alone will not solve that gap.

## Async API direction preferred by the owner

Keep the visible pure-or-async rule for general calls from async code. Provide
async standard-library counterparts, rather than making the clock a privileged
ordinary function that user-defined wrappers cannot compose with consistently.

An async function may suspend; it need not suspend on every call. An async clock
read may complete immediately. It remains impure. Do not make clock reads pure,
relax purity guarantees, or disguise synchronous blocking work in an async wrapper.

The earlier assistant recommendation to generalise permission for ordinary
non-blocking effects was challenged because it could make `time.now()` special.
The owner prefers an async clock API aligned with the existing language model.
A general new effect classification is not the selected direction for this fix.

### Select the interface through the module import

The owner strongly dislikes per-function suffixes such as `readFileSync` and
prefers a whole async module with matching operation names:

```panackelty
import stdlib/time as time

now = time.now()
```

```panackelty
import stdlib/time/async as time

now = await time.now()
```

These were interface sketches, not complete programs. The paths for time,
environment, host and filesystem are now implemented and locally validated in
PR #693; the expanded programme below covers the remaining APIs. The first call is
synchronous, not necessarily blocking. The module chooses the API; `await`
remains explicit and compiler checked. Import aliases stay convenient and stable.

Apply the same convention consistently where appropriate, for example
`stdlib/filesystem/async`, `stdlib/process/async` and `stdlib/host/async`.
Verify module layout/resolution before assuming this needs language sugar.
Existing `stdlib/async_host` and `stdlib/async_filesystem` names need an explicit
compatibility/migration decision. A wrong-interface diagnostic should explain
what happened and point to the appropriate async module when one exists.

### Standard-library coverage

The owner considers broad synchronous/async API coverage reasonable, rather than
an exceptional capability added separately for every program. Node's paired
filesystem/process interfaces are a precedent; its function naming is not the
preferred model for Panack.

| Capability | Current finding and prospective work |
| --- | --- |
| Clock | Synchronous only; async counterpart is the immediate preferred direction. |
| Sleep | Synchronous only; async sleep must let other tasks progress. |
| Environment | Synchronous lookup only; async counterpart could complete immediately. |
| Filesystem | Narrow async bounded reads and root identity already exist; broader operations remain synchronous. |
| Subprocesses | Ordinary run/run_isolated APIs; async execution needs execution and cancellation semantics. |
| Streaming HTTP | Separate streaming API is synchronous; ordinary HTTP exchange already supports async. |
| Arguments, stdout, stderr, exit, TCP | Async APIs already exist; reconcile naming rather than duplicate capabilities. |

This is a desired consistency direction, not an agreed implementation inventory,
release commitment or estimate. Decide bounded delivery and cancellation/resource
contracts before adding operations. Pure computation does not need an async twin.

## Anonymous functions, closures and explicit state

The owner is leaning towards supporting all three independently:

- Anonymous functions express small operations inline.
- Closures retain the context a function needs.
- Explicit state makes shared mutation visible and can define update guarantees.

These are separate choices, not mutually exclusive solutions. Existing Panack
supports named function references through `@name` and `.call(...)`; the anonymous
function syntax used in this discussion is illustrative, not implemented.

### Visibility of mutation matters

The owner prefers the explicit-state counter sketch over implicit mutable capture.
With `count = count + 1` inside a closure, a reader must remember that `count`
was captured, rather than being a per-invocation local. `count.update(...)` makes
mutation visible at the point of use. The advantage sought is understandable
behaviour, not difference from other languages for its own sake.

Proposed sketch:

```panackelty
count = state.create[Nat](0)
next_count = () => count.update((current: Nat) => current + 1)
```

The binding to the state handle is immutable; its contents can change through
explicit effectful operations. Capturing an immutable binding does not by itself
make the resulting closure pure. State reads and updates are effects.

Anonymous pure transformations could avoid the boilerplate of a separate named
increment function. Contextual HTTP handlers need not be the only way of passing
state: a closure could capture the explicit state handle instead.

### Mutable capture remains an open design choice

The owner found rejecting implicit capture of mutable bindings understandable,
but explicitly requested discussion without committing to anything yet. Do not
treat the assistant's subsequent "that's the rule" wording as a final decision.

The explored rule was: capture immutable bindings; reject implicit capture of a
`mut` binding. To retain a snapshot, bind the current value to an immutable local
and capture that. To observe shared updates, capture an explicit state handle.

"Ambiguous" here means difficult for readers to infer, not impossible for a
compiler to specify. For `mut count = 0; read = () => count; count = 10`, snapshot
and shared-variable capture would produce different answers. An explicit choice
avoids silently choosing either behaviour.

Local mutable variables declared inside a closure remain a separate matter:
each invocation can own a local accumulator without shared mutable capture.
Do not conflate local mutation, shared mutation and impurity.

### State semantics still need design

The proposed update operation reads, transforms and replaces a value as one
indivisible operation relative to other tasks. The transformation would be pure
and unable to suspend. This is not an implemented concurrency guarantee.

Reading a value, awaiting other work, then writing a replacement derived from
that stale value can still lose intervening updates. An atomic update API cannot
make every use of state correct automatically.

Open questions include handle ownership/lifetime, payload restrictions, failure,
cancellation, callable types and whether state operations use the same async
module convention. Earlier direct `State.update()` examples predate the preferred
async API direction; their call/await syntax must not be mistaken for agreement.
There is no commitment to actors, locks, channels or generally mutable records.

## Related language questions clarified

- Named function values currently require `@name`; functions can be stored in
  variables and invoked with `.call(...)`.
- Types are not currently ordinary values. `chosen_type = Nat` is rejected;
  type annotations and generic type arguments are supported.
- Recursion is supported through ordinary function calls. The checked factorial
  example used a zero base case; the compiler proved Nat subtraction safe in
  the other branch.
- Methods currently belong in `impl` blocks with an implicit immutable `self`.
  Putting methods inside record bodies was discussed as a possible design choice,
  not selected work. No verified historical rationale for choosing `impl` was
  established in this discussion.

## Historical next design step and workflow

Define the async clock API under the preferred module convention, checking the
resolver and native async mechanism. Keep counter/state/closure design open until
explicitly selected; neither a clock fix nor this record approves those features.

Work stays on `feature/rm162-real-program-fixes`, branched from `next`. Under the
alpha.16 workflow trial, no CI on branch/PR updates unless the owner explicitly
requests it; merging into `next` may trigger CI. Local checks and review still
apply. No website changes or implementation scope expansion follow from this note.

## Implementation batch selected, 8 November 2026

The owner approved batching the async module convention, immediate clock and
environment reads, and paths for existing host/filesystem async operations on
`feature/rm162-async-clock`, with local validation and no development CI.

- `time/async`: async `now`, shared error types and pure duration constructors.
- `environment/async`: async `get`, preserving the invocation snapshot and
  the distinction between missing and empty values.
- `host/async`: existing async arguments, stdout, stderr and exit.
- `filesystem/async`: existing async root identity and confined reads.

Old async import paths stay compatible through re-exports. New immediate reads
require a compiler/VM capability update, but no bytecode format change. Nested
modules must be installed and included in browser source bundles. The originating
uptime service is the integration example. Blocking sleep, subprocesses, new
filesystem operations, closures, anonymous functions and mutable shared state
remain outside this batch.

### Local validation and limits

The batch passed focused native async-effect and clock-failure contracts,
source/bytecode stdlib checks, and the real HTTP uptime and confined-filesystem
integrations. `make check` passed in 582 seconds, including relocated installs
with the nested modules, on 8 November 2026. Thirteen coverage manifest/policy
checks passed; the new adapters remain explicitly unmeasured by that bounded
coverage corpus. Browser source-bundle file identities were checked; this does
not claim that a new browser product has been built or published.

Seed stages 2/3/4 are identical:
`f2314de2de1c47bb23de74aa3bde16d66b2f55287089ad1fe96c3b8ba5890f52`.
Existing-program native artifact compatibility passed, and ordinary stdlib
conformance bytecode retained its prior SHA-256.

Performance: the new reads complete on the current task without worker creation
or pending-operation storage. The full-check timing is recorded as an observation,
not a controlled performance comparison or a new budget. Historical validation
budget warnings remain part of the existing performance review; this batch does
not resolve that programme. No throughput claim is made.

The batch is ready for review on its development branch, not integrated or
released. No development CI or website publication has been requested.

## Expanded async stdlib and documentation programme, 8 November 2026

The owner selected completing the relevant async stdlib work together before
returning to shared state, closures or anonymous functions. Documentation is part
of the delivery, including website reference pages for time, processes and I/O.
PR #693 is the first locally validated batch, not completion of this programme.

### Operation inventory and proposed delivery boundary

The inventory below comes from the current stdlib declarations. Existing APIs
are evidence; proposed async counterparts still need the runtime contracts below.
Do not interpret a proposed module path as an already implemented API.

| Area | Operations and disposition |
| --- | --- |
| Time and environment | `time.now`, `environment.get`: implemented in PR #693 under `/async`. Pure duration helpers remain shared. |
| Host I/O | Async `arguments`, `stdout`, `stderr`, `exit` already exist; PR #693 adds `host/async`. The approved API contract places canonical sleep under `time`/`time/async`, retaining host.sleep for compatibility; checked output uses `console`/`console/async`. Bounded ordinary/async console.read now implements explicit EOF, short-read and cancellation handling; it is not an alias for a nonexistent host.read. |
| General filesystem | Provide async `metadata`, `read`, `write`, `list_directory`, `remove_file`, `remove_directory`, `create_directory`, `temp_directory`, `temp_file`. Retain current result types, Path semantics and documented bounds. |
| Bounded/confined filesystem | Existing async `root_identity`, `read_beneath` remain. Include `read_chunk`, `atomic_write`, `read_link`, and `directories_overlap` in the capability review and define their matching module paths without silently moving existing public owners. |
| Processes | Ordinary/async `run(Command)` with explicit inherited/exact environment modes and Exited/Signalled results. The positional run/run_isolated API remains in process/legacy for compatibility. Bounded input/output, timeouts and cancellation/reaping are implemented; integration acceptance remains pending. |
| Streaming HTTP | Async `open`, `read`, `close` counterparts for `http_stream`; preserve framing responsibility in the HTTP package. Existing async HTTP exchanges and TCP services are reused. |
| Staged filesystem | Explicitly assess `begin`, `path`, `directory`, `write`, `symlink`, `publish`, `close`. Handle ownership and cancellation during publication must be settled before choosing implementations. Do not quietly omit these public I/O operations or equate cancellation with rollback. |
| Digest and gzip | Explicitly assess digest `sha256_open`, `update`, `finish`, `close` and gzip `open`, `step`, `close`. These are stateful native computation, not necessarily blocking I/O; determine how async callers use them safely and how large work yields. Do not automatically add worker threads or call them pure. |
| Nested execution | `host.run` executes bytecode, not an operating-system subprocess. Record whether this specialist operation belongs in the batch after reviewing embedding, exit propagation and cancellation; do not assume the process implementation covers it. |
| Pure and specialised support | No async twins for pure path/text/collection/numeric operations. Compiler bootstrap wrappers and testing helpers need an explicit classification as tooling support, not automatic duplication. Existing HTTP-local resource operations retain their established permission model. |

The first implementation step is a runtime-capability review of this finite list,
including an explicit disposition for each specialist row. General filesystem,
processes, sleep and streaming HTTP are the main missing user-facing groups.
Material API choices must be made concrete with signatures and small programs;
routine implementation decisions do not require repeated owner questions.

### Runtime contracts and acceptance

For each group, record and test:

- Whether the call completes immediately, waits on the scheduler, or uses a
  bounded worker. No blocking host call may hold up unrelated async tasks.
- Arguments, return/error types, data and concurrency bounds, and host support.
- Cancellation before start, while active and during completion. State what
  effects may already have happened; never promise undo of a write or process.
- Ownership and cleanup of handles, buffers, descriptors and child processes,
  including errors, task cancellation and service shutdown.
- For processes, timeout versus cancellation, child termination/reaping and
  descendants; for streaming, close/read races and completion after cancellation;
  for sleep, timer semantics and independent-task progress.
- Purity and await checks in source, verification and runtime checks for forged
  bytecode, and compatibility of existing ordinary/async APIs.
- Source execution, saved bytecode and relocated installation with the actual
  user examples. Browser/unsupported hosts must report their real capabilities.

Batch related implementation on the CI-free branch. Use focused tests while
iterating and full local validation at coherent delivery checkpoints. Development
pushes/PR updates must not start CI; integration into next may do so. This plan
does not authorise merging PR #693 or publishing the website now.

### Documentation deliverable

The website source currently has `/reference/http-client/`,
`/reference/http-server/` and `/playbook/http-server/`, without equivalent time,
process or general-I/O reference pages. Correct discoverability as well as prose:

1. A standard-library landing page linked from the existing API navigation,
   grouping modules by purpose and explaining ordinary versus `/async` imports.
2. Time and durations: monotonic instants, units, exact conversions, clock errors,
   measuring elapsed time and async sleep (in its actual owning host module).
3. Files and paths: directory traversal, bounded reads, writes, temporary files,
   atomic/confined operations, errors and cancellation side effects.
4. Processes: executable paths, arguments, stdin/stdout/stderr, environments,
   exit status versus signal, output limits, timeouts and cancellation.
5. Console and environment: arguments, input availability, output, exit and
   snapshot environment reads, including missing versus empty values.
6. Streaming/network links and specialist module entries matching the final
   inventory; reuse existing HTTP references rather than duplicating them.

Each page needs exact signatures, import examples, complete runnable success and
failure examples, expected output, limits, cleanup/cancellation and platform
support. Reuse tested core examples as the source of website examples; extend
existing website documentation tooling rather than inventing a second generator.
Core reference/recipes and website content must agree. Review preview pages for
navigation and readability, not just compilation of snippets.

Document existing ordinary APIs as well as async counterparts. Explicitly label
alpha.15 versus alpha.16-development availability. Native host examples must not
appear runnable in the browser playground unless supported there. Publication
follows review and the release checklist; live release claims stay accurate.
Published articles remain untouched unless their proposed edits are separately
shown to the owner. Preserve unrelated website drafts.

### Provisional tracking baseline

These weights apply only to this expanded programme, not the whole release.
They estimate relative implementation, validation and documentation effort and
are provisional until the specialist-operation review establishes its boundary.
Filesystem and process/stream lifetime rules account for most uncertainty.
Documentation is its own acceptance work, not a final optional polish step.

| ID / outcome | Weight | Current state |
| --- | ---: | --- |
| AS1: Imports, clock and environment | 10 | Accepted on next: PR #693, local full validation and green integration checks. Earned: 10 points. |
| AS2: Filesystem and resource-operation boundary | 30 | Accepted on next: ordinary/async general, confined and staged filesystem; typed digest/gzip resources and TCP adapters; domain errors and compatibility. Lifecycle/fault/source/bytecode/relocated evidence is recorded in STDLIB_API_CONTRACT.md. Earned: 30 points. |
| AS3: Timers, console and processes | 25 | Accepted on next: checked timers, bounded console input/output, Command subprocesses and runtime/testing adapters. Local full and focused acceptance plus green integration. Earned: 25 points. |
| AS4: Streaming HTTP | 15 | Accepted on next: typed streams, detailed errors, diagnostic HTTP client and disassembly parity. Local full, TLS, lifecycle and relocated checks plus green integration. Earned: 15 points. |
| AS5: Stdlib reference, guides and website | 20 | Accepted. Core guides/examples integrated; website PR #77 published after owner approval and passing checks. Deployment and live content/navigation verified. Earned contribution: 20/100 points. |
| Total | 100 | Accepted contribution: 100/100 points. All five slices complete; this is the expanded stdlib/documentation batch, not the whole release or broader RM#162. |

Do not count AS1 twice within RM#162 or equate this programme's weights with a
release percentage. Shared state, closures and anonymous functions remain
separate work after this topic, unless the owner changes the order.

## Whole-library review proposal, 17 November 2026

The owner requested a recommendation covering the entire stdlib, including names
and module ownership. [The redesign proposal](STDLIB_REDESIGN_PROPOSAL.md) records
that recommendation for review under RM#162. It does not approve the individual
API changes or replace the existing runtime acceptance requirements.

## Redesign direction approved, 18 November 2026

The owner approved the whole-library redesign direction and requested continuation.
The [API implementation contract](STDLIB_API_CONTRACT.md) records signatures,
compatibility, ownership and ordering. This approval supersedes the earlier
proposal-only status, while detailed semantic choices remain explicit. The subsequent
filesystem batch now passes local acceptance, recorded in the API contract. It is
committed on the development branch, not integrated or accepted as all of AS2.

23 November: PR #693 merged as `930f951`; [integration run](https://github.com/sproates/panackelty/actions/runs/45163842662) stopped at a stale coverage-inventory test; its explicit expectation is corrected and the exact policy suite passes locally. Hosted acceptance is pending a new integration run. Website PR #77 has owner preview approval but no publication permission. The implementation is integrated, not yet declared hosted-accepted.

23 November acceptance: [integration run 45164349102](https://github.com/sproates/panackelty/actions/runs/45164349102) passed on `12dccfb` after the explicit inventory expectation was corrected in PR #697. These are the configured next package/policy/integration checks, not the full hosted release matrix. Earlier full local validation remains the product evidence. Website publication is still separately authorised.

23 November publication acceptance: the owner authorised website checks and publication.
[Website PR #77](https://github.com/sproates/panackelty-website/pull/77) merged as
`e59e93e` after [checks 45165626275](https://github.com/sproates/panackelty-website/actions/runs/45165626275)
passed. [Deployment 45166480787](https://github.com/sproates/panackelty-website/actions/runs/45166480787)
passed, including verification of deployed bytes and provenance. Live browser review
confirmed the Time reference, signatures, errors, complete example and stdlib-before-HTTP
navigation. The [published reference](https://panackelty.com/reference/standard-library/)
contains nineteen module pages and 72 function entries, alongside four guides and nine
complete examples. AS5 is accepted: all five slices of the expanded async stdlib and
documentation batch are complete (100/100 weighted points). RM#162 remains open for
broader real-program work, including anonymous functions, closures and shared state.
These are alpha.16 development docs; release pins, the playground and published articles
are unchanged. This completion record changes no runtime or build behaviour and requires
no performance measurement or repeated product validation.


## Callable and state contract proposal, 23 November 2026

The owner selected the next design task after stdlib completion. The
[contract proposal](CALLABLES_AND_STATE_PROPOSAL.md) records syntax, effects, immutable capture,
explicit state, lifetime/cancellation rules, diagnostics and the complete target
uptime/counter service. Four provisional implementation slices carry weights
20/30/30/20; all are proposed with zero earned contribution. Owner review is
pending; this is not implementation approval or a current language specification.
No runtime/performance impact or CI is required for this informational record.
Website and published-article updates await agreement and separate review.

23 November review revision: PR #700 now defines scheduler-preemptible cell
reservations with cancellation before commit, State identity and closure capture
equality, explicit resource-capture restrictions, contextual callback parameter
types and committed-increment counting. Owner contract review remains pending;
implementation has not started and the proposed weights remain unchanged.
