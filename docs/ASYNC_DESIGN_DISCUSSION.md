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
and reduced reproductions against alpha.15. No fix has been implemented.

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

These are interface sketches, not complete programs. The proposed async path
has not been implemented or validated with the resolver. The first call is
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

## Next design step and workflow

Define the async clock API under the preferred module convention, checking the
resolver and native async mechanism. Keep counter/state/closure design open until
explicitly selected; neither a clock fix nor this record approves those features.

Work stays on `feature/rm162-real-program-fixes`, branched from `next`. Under the
alpha.16 workflow trial, no CI on branch/PR updates unless the owner explicitly
requests it; merging into `next` may trigger CI. Local checks and review still
apply. No website changes or implementation scope expansion follow from this note.
