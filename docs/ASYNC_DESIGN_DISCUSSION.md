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

PR #700 second-review clarifications: infer callback input types before checking
the body, then infer generic output types from its result; preserve existing
scalar Map-key/Set-element restrictions. Acceptance cases updated. Contract
approval remains pending; implementation and proposed weights are unchanged.

23 November: the owner approved PR #700 for merge and requested a stop. The
callable/state contract is accepted on merge. CS1–CS4 are planned, unstarted
(0/100 earned implementation points); work pauses before CS1.

25 November: the owner resumed CS1 on `feature/rm162-anonymous-functions`.
Anonymous header parsing is under development; 224 parser assertions and the
candidate compiler build pass. Expression integration, contextual checking,
effects and lowering remain; no anonymous-function feature is accepted yet.
CS2–CS4 remain planned. No CI, seed refresh or final full validation at this
checkpoint; implementation weights and zero accepted contribution are unchanged.

27 November: CS1 now parses complete anonymous expressions, including block and
nested bodies. Parameter scopes, qualified annotations, source inventory and
cache encoding retain the new syntax. Focused checks cover 237 parser, 31 resolver,
38 source-span, 28 inventory and 2,042 module-binding/cache assertions; the
candidate compiler builds and verifies. Anonymous functions still fail closed at
type checking: contextual types, effects and noncapturing lowering remain. No CI,
bootstrap refresh or final full validation; no implementation slice is accepted.

27 November, working execution checkpoint: CS1 now checks and lowers noncapturing
anonymous functions using the existing bytecode format. Pure, ordinary and async
bodies have separate effect scopes. Direct annotated targets, direct return
values and callback arguments supply parameter types; map results infer from the
body. Generated functions survive body/code caches and generic type identities.
The focused module-binding/cache suite passes 2,151 assertions, including effect
rejection and fresh/cached equivalence. Source and saved-bytecode execution agree
for the new runtime fixture; the async fixture passes. A confirmed remaining gap
is contextual typing through conditional return branches. Full contextual
propagation, explanation/diagnostic acceptance and final compatibility/full
validation remain before CS1 acceptance. No CI or bootstrap refresh; CS2–CS4 and
the zero accepted implementation contribution are unchanged.


27 November, compound contextual typing checkpoint: expected callable inputs now
flow through if/match results, block tails, reassignment and compound callback
arguments. Nested anonymous bodies also receive an explicit result annotation.
Argument checking replays compound callbacks in source order after gathering
input types, preserving mutation-based invalidation of guarded facts. A paired
regression accepts guarded subtraction without a preceding mutation and rejects
it when an earlier argument writes the guarded variable. Runtime selection
effects occur once.

The module-binding/cache suite passes 2,180 assertions, explanations 266 and
purity 39. Candidate source execution and verified saved bytecode match the
expanded fixed transcript. Remaining work is the contextual-type acceptance
audit (including aggregate/constructor positions), explanation/diagnostic review,
final functional registration and compatibility/full validation. CS1 remains in
progress, with zero accepted implementation points; CS2–CS4 remain planned. No
CI or bootstrap refresh. The extra checking pass is limited to calls containing
compound callback expressions; ordinary calls retain their current path. No VM
or bytecode changes. Website claims remain unchanged until feature acceptance.


27 November, collection and evidence checkpoint: known array element types now
reach anonymous parameters, including nested arrays, returned arrays and callback
arrays supplied as arguments. Existing record constructor signatures also supply
callback inputs. Regressions reject incompatible parameter types, impure bodies
in pure callback arrays and unannotated inputs without context. Explanation
evidence retains the exact anonymous body expression and its checked result; the
missing-parameter diagnostic is asserted verbatim. The module-binding/cache suite
passes 2,205 assertions. Expanded source and saved-bytecode transcripts match.
The candidate passes native-oracle-artifacts for unchanged-program bytecode.

CS1 remains in progress: finish the constructor/generic contextual boundary audit,
end-to-end explanation review, functional registration and final bootstrap/full
validation. No CI or bootstrap refresh. CS2–CS4 remain planned and accepted
implementation contribution stays zero. The array pass reuses source-order child
checking; no VM, bytecode-format or runtime representation change. No website
publication; the proposal and test evidence describe development support.


27 November, CS1 acceptance checkpoint: constructor context now supplies callback
inputs through generic records and enum payloads, including nested arrays and
qualified constructors. Ordinary function calls retain input-only inference; a
regression rejects deriving an anonymous parameter from the receiving result.
Explanation traversal enters anonymous bodies for type-at-position, inferred
locals and guard evidence. The public CLI checks the original source locations.

The module/cache suite passes 2,219 assertions. Ordinary and async functional cases
pass source, compile and saved-bytecode checks; the CLI explanation fixture also
passes all three modes, with 121 internal assertions. The fixture inventory and
its mutation tests pass (616 outer transcript assertions). Candidate unchanged-
program bytecode compatibility passed. Bootstrap refresh produced identical
stage 2/3/4 compilers (88fa8a2ec9c850afb7e64bc714bca7eec194e9e8524e5821b8338e0be53c25ad)
and matching standard-library artifacts/output. Final make check is running.
CS1 remains unaccepted pending that result and integration; CS2–CS4 remain planned.
No development CI or website publication. Specification and architecture now
describe noncapturing support; closures and State are not claimed. The runtime
and bytecode version are unchanged. Constructor-context inference is restricted
to constructor calls with an expected type; ordinary calls keep their rules.


CS1 final local acceptance, 27 November: make check passed in 708 seconds with
host access (unit 421s, bootstrap 94s and functional 220s; bootstrap overlaps
unit). Relocated-package smoke and quick-start passed. The suite caught a stale
watchdog observation hook after the contextual block checker was introduced; the
test-only observer now recognises both block entry points, with explicit stall,
recovery and unchanged-budget assertions. No production VM or timeout policy
change was needed. Earlier sandbox failure and the watchdog failure were fixed
before this successful final run.

CS1 implementation and local acceptance are complete, pending next integration
and hosted acceptance. CS2–CS4 remain planned. Accepted implementation points
stay at zero until integration acceptance; RM#162 remains open. The measured
full-run duration is evidence, not a new performance target. No development CI
or website publication occurred.


CS1 merged into next through [PR #701](https://github.com/sproates/panackelty/pull/701) at `14b709d` on 27 November. [Integration run 46042786755](https://github.com/sproates/panackelty/actions/runs/46042786755) is running; hosted acceptance remains pending. Local make check passed in 708s. CS2–CS4 remain planned; accepted implementation contribution stays zero until that run is verified.


CS1 accepted, 27 November: [PR #701](https://github.com/sproates/panackelty/pull/701) is merged and [integration run 46042786755](https://github.com/sproates/panackelty/actions/runs/46042786755) passed for `14b709d`. Full local make check passed in 708s. Subsequent status changes contain informational Markdown only. CS1: Anonymous functions is complete (20/20 points); CS2: Retained captures (30), CS3: State (30) and CS4: Examples/documentation/installed acceptance (20) remain planned. Programme earned contribution is 20/100. RM#162 remains open; CS1 has no separate issue to close. No website publication or closure/State completion is claimed.


CS2 started, 27 November, on feature/rm162-retained-captures from next 4cc5d00.
The first runtime checkpoint adds an owned closure value with lexical target
identity and retained capture values. Equality compares target and captures, not
allocation identity. Capture eligibility follows actual ownership edges through
collections, records, variants and other closure environments, including hidden
shared-array suffixes. A non-recursive worklist with a visited-pointer set avoids
revisiting shared graphs. Native resources, Void, iterators and unclassified new
value kinds fail closed. Array append checks now follow closure ownership edges
to prevent introducing shared-storage reference cycles.

This is runtime groundwork, not executable source closure support. No instruction
or builtin constructs the new value yet. Source capture analysis, checked
construction/invocation, verifier/effect/arity enforcement, async/native handler
integration, iterator snapshots, cache/provenance evidence and final functional/
full validation remain. CS2 is in progress with zero earned points; CS1 remains
accepted and programme progress is 20/100. No CI, bootstrap rebuild or website
change accompanies this checkpoint.

CS2 checked invocation checkpoint, 27 November: CALL_VALUE and AWAIT_VALUE now
accept internal closure values. Captures precede explicit arguments in the
callee frame; target lookup, arity and the existing pure/async call-edge rules
are checked before entry. Named references keep their existing path. Small
combined argument lists use stack storage; larger lists use a temporary borrowed
pointer vector, with frame ownership handled by the existing retain/release path.
Native tests cover argument order, repeated invocation, the effect matrix,
missing/builtin targets, arity mismatch, suspension, cancellation and callee
traps. Source construction/lowering, host-retained callbacks, iterator snapshots,
cache/provenance and final slice acceptance remain. No bytecode format or compiler
seed change. CS2 remains in progress with zero accepted points (programme 20/100).

CS2 source-lowering checkpoint, 27 November: referenced immutable lexical values
now become closure captures, including captures used by nested closures. Generated
functions receive a hidden capture-parameter prefix; the public callable signature
retains only explicit arguments. Generic types are rebound to the anonymous body
while lexical declaration identities remain available for provenance. Capture
reads are children of the creation node; the nested function body remains a
separate effect scope. Known mutable/resource captures are diagnosed, and the
runtime validates target, layout and resource graphs again at construction.

The reserved pure `$intrinsic_closure` builtin is an additive v9 capability;
there is no opcode or constant-layout change. Older runtimes reject the unknown
intrinsic. Noncapturing and named callables remain unchanged. The compiler rejects
combined capture/explicit parameter counts beyond v9's 255-parameter limit.
Ordinary and async candidate fixtures pass source execution and saved bytecode,
including creator-return lifetime, nested/generic captures, captured callbacks,
collection callbacks, equality and repeated awaits. This is candidate evidence;
the public launcher still uses the unchanged bootstrap compiler. Host-retained
callbacks, iterator snapshots, dedicated capture explanations, bootstrap refresh
and final acceptance remain. CS2 stays in progress; programme earned contribution
is 20/100. No development CI or website publication.

CS2 retained-host checkpoint, 27 November: HTTP routes and fallbacks, contextual
HTTP handlers, TCP workers and service readiness callbacks now retain closure
environments. The shared checked callable descriptor preserves target identity,
explicit arity and async effects; child frames retain captures independently and
server destruction releases retained environments. Verified-program target lookup
uses the existing index. No throughput improvement is claimed.

Native lifetime and allocation-failure checks pass, including sanitizer checks.
The candidate HTTP fixture passes ordinary/contextual routes and fallback in
source and saved-bytecode modes after the creating function returns. The binding
and cache suite passes 2,265 assertions. Captured arrays and ranges can be iterated
from the start on every invocation; loop-created closures retain each iteration's
value. Source and saved-bytecode transcripts agree.

Iterator clarification supersedes earlier pending-snapshot notes: source iteration
exposes iterable values, not the VM's mutable V_ITER cursor. Each loop creates a
fresh cursor. Raw internal cursors remain capture-ineligible; no new public
iterator API or cursor snapshot feature is introduced.

The existing zero-grace TCP cancellation assertion failed once, then passed on
retry and under sanitizers. No timeout or assertion was weakened; watch this
case during final validation. Capture explanations, bootstrap refresh, public
launcher/full validation and integration remain. CS2 is not accepted; programme
progress remains 20/100. No development CI or website publication.

CS2 explanation checkpoint, 27 November: the checker now records capture eligibility
and reasons in explanation mode. Text and terminal views show accepted/rejected
captures and follow lexical identities to declaration context, including shadowed
parameters. Evidence is not inferred from diagnostic text. The ordinary body cache
retains lexical identities but omits explanation evidence; explain checks source
afresh. No cache-schema or bytecode-format change is needed. The terminal's generated
instruction description names closure creation rather than its reserved intrinsic.
Candidate CLI explanation checks pass (123 cases), and unchanged-program bytecode
contracts pass. Bootstrap refresh and final local validation are in progress.
CS2 remains unaccepted and programme progress stays 20/100. CS3 State and CS4 final
examples/installed/website acceptance remain planned; no website publication or
CI has been started by this development checkpoint.

CS2 local acceptance, 27 November: full `make check` passed in 725s, including
unit/functional suites, fixed-point bootstrap, packaging, relocation and quick
start. The existing zero-grace TCP assertion passed in this full run. Focused
capture checks passed 2,313 binding/cache assertions and 123 CLI cases; terminal
navigation, capture origin and TTY restoration passed. Bootstrap compiler and
stdlib conformance outputs matched across stages. Unchanged-program bytecode
compatibility and native closure lifetime/allocation/sanitizer checks passed.
The 725s broad run versus CS1's 708s is not a controlled performance comparison;
existing historical advisory budget warnings remain unchanged.

CS2 implementation and local acceptance are ready for integration. CS2 remains
unaccepted until next integration CI passes; no task points are earned early.
CS1 remains 20/20, CS2 0/30 pending integration, CS3 State 0/30 and CS4 examples/
installed/website acceptance 0/20. Programme accepted progress remains 20/100.
RM#162 stays open; CS2 has no separate issue to close. The existing CS4 website
follow-up covers closure examples/reference updates; nothing is published here.
