# Anonymous functions, closures and explicit state

Proposal for RM#162: Real-program fixes, 23 November 2026.
Based on next at `9e7e911`. This is a reviewable contract, not implemented syntax
or an amendment to the current language specification. The owner selected this
design task after completing the expanded async stdlib batch.

## Outcome

A developer can write a small handler inline, retain the immutable values it needs,
and explicitly share a changing value between calls. The acceptance program is an
HTTP status endpoint reporting uptime and its request count. The same facilities
must work outside HTTP, including returned functions and concurrent async tasks.

The existing [discussion](ASYNC_DESIGN_DISCUSSION.md#anonymous-functions-closures-and-explicit-state)
records preferences, including decisions that were deliberately left open. This
proposal supplies recommended answers; it does not retrospectively call them agreed.

## Recommended contract

### Anonymous functions use the existing callable types

```text
increment = (current: Nat) => current + 1
label = (name: Str): Str => { "Hello, ${name}" }
read_clock = async () => { await time.now() }
```

Parameters require explicit types, including when a surrounding API could supply
one. A result annotation is optional when the body determines a single result type;
otherwise the diagnostic asks for it. This is local callable checking, not a wider
change to binding or generic inference. Block bodies follow existing named-function
result rules. Zero parameters use `()`. Arrow bodies extend the expression grammar;
they do not change named-function declarations or match-arm syntax.

An unmarked anonymous function whose body is pure has type `PureFn`. An unmarked
body performing ordinary effects has type `Fn`. An explicit `pure` prefix is also
allowed and enforces purity. `async` is explicit and produces `AsyncFn`, even if
that particular body never suspends. The result annotation, when present, describes
the function's result, not an internal task type. Async result requirements remain
those of existing named async functions. Nested function creation does not execute
its body or acquire its effects.

Existing `@named_function`, `PureFn[A,R]`, `Fn[A,R]`, `AsyncFn[A,R]` and `.call(...)`
remain. PureFn can still widen to Fn. No automatic conversion between ordinary and
async callables. No new `callback(...)` sugar, generic anonymous-function syntax,
constructor references or self-recursive anonymous bindings in this batch. Named
recursion and returning a closure remain supported workflows.

### Capture immutable bindings, not mutable variables

A closure retains the values of referenced immutable lexical bindings at creation.
Only bindings actually referenced by its body are captured. Parameters can be
captured too. An uninitialised binding, including the closure's own binding, cannot
be captured. Shadowing follows normal lexical scope. Captured bindings are not
assignable inside the closure.

```text
prefix = "Hello"
greet = (name: Str) => "${prefix}, ${name}"
```

The retained values survive the creating function returning. Captures are values,
not pointers to stack locals. Ordinary value semantics still apply; sharing an
immutable representation internally does not expose mutable aliases.

Implicit capture of a `mut` binding is rejected, even if this closure only reads it.
For a snapshot, first bind its current value to an immutable local. For shared
updates, use explicit state. Local mutable variables declared inside the anonymous
body belong to each invocation and retain existing purity rules.

A captured State handle refers to the same cell in every invocation. Capturing the
handle does not read or update that cell; those operations are effects. Do not
infer that a closure is pure simply because its captured bindings are immutable.

Do not capture lifetime-restricted capabilities such as a response writer or an
owned native resource in an escaping closure. This batch conservatively rejects
such captures rather than adding borrow inference or an implicit ownership transfer.
Ordinary immutable data, other safe closures and State handles are permitted;
restrictions apply recursively to captured records, enums and collections.

### State is an explicit shared cell

Use matching ordinary and async modules:

```text
import stdlib/state as state
// Or, in async code:
import stdlib/state/async as state
```

Proposed ordinary signatures, with corresponding async signatures under `/async`:

```text
create[T](initial: T): State[T]
read[T](cell: State[T]): T
update[T](cell: State[T], transform: PureFn[T,T]): T
```

All three are effectful, including creation. Both modules expose the same State
identity. Generic argument syntax stays consistent with existing generic calls.
The handle is copyable and aliases one cell; it cannot be constructed by user code,
serialised, ordered or compared for value equality. Its lifetime ends when its last
reachable reference is released. There is no manual close or use-after-close state.

Payloads are immutable data values: primitives and recursively compatible records,
enums and collections, including guarded types. State handles, callable values,
tasks and native/resource handles are excluded recursively. The compiler must
check generic instantiations too. This deliberately avoids cycles through cells
and callbacks, hidden resources and unresolved ownership rules. A new public trait
system is not part of this proposal.

`read` returns a snapshot. `update` reads the current value, invokes the pure
transformation exactly once, stores the returned value, and returns that new value.
It is one indivisible operation relative to all other access to the same cell,
including ordinary and async callers. Two overlapping increments of zero return
1 and 2 in some order and leave 2 stored. Task scheduling and response delivery
order are not promised.

The callback cannot await, read another cell, write a cell, perform I/O or call
ordinary effectful functions. A long pure callback can still monopolise execution;
purity is not a time bound. Do not silently retry the callback or introduce a worker
thread. A trap before replacement leaves the old value stored and follows normal
runtime failure handling. Allocation failure follows the existing VM failure
policy; these operations have no invented recoverable host-error Result wrapper.

Cancellation observed before an update begins prevents it. Once begun, the update
runs to completion without a suspension point. Cancellation after commit does not
roll it back, even if the caller never receives the result. No fairness, durable
storage, multi-cell transactions, actors, locks or channels are introduced.

A read followed by other work and then an update based on the old snapshot can
still overwrite intervening changes. Read-modify-write logic belongs inside the
update callback. No separate `write` API is necessary for this first contract:
replacement is expressible by a pure callback returning the replacement value.

### Why module functions rather than handle methods?

Earlier sketches used `requests.update(...)`. A shared State type cannot give its
method a different effect simply because its import changed. Using
`state.update(requests, ...)` keeps one handle type, makes the selected interface
visible and follows the stdlib module convention without inventing import-dependent
method resolution. This is an explicit departure from the illustrative article,
for owner review; do not silently rewrite the published article.

## Complete target example

This is proposed source, not a runnable example today. The HTTP API shape comes
from the existing uptime program and server package; anonymous functions and state
are the new parts. Use the existing HTTP example's project manifest/dependency.

```text
import http/server as http
import stdlib/time/async as time
import stdlib/state/async as state
import stdlib/host/async as host

async main(): Unit {
  match await time.now() {
    Error(problem) => {
      reported = await host.stderr("Cannot start: the monotonic clock is unavailable.")
      await host.exit(1)
    },
    Ok(started) => {
      requests = await state.create[Nat](0)
      server = http.create_bound_server("127.0.0.1", 8082)
      server.get("/status", async (request: http.Request, response: http.ResponseWriter): Result[Unit,Str] => {
        total = await state.update(requests, (current: Nat) => current + 1)
        match await time.now() {
          Error(problem) => {
            response.status(503)
            response.send("Requests handled: ${total}; uptime unavailable: the clock could not be read.")
            Ok(())
          },
          Ok(now) => {
            elapsed = now.difference(started)
            seconds = elapsed.as_seconds()
            response.send("Requests handled: ${total}; uptime: ${seconds} seconds")
            Ok(())
          }
        }
      })
      match await server.listen() {
        Ok(done) => (),
        Error(problem) => {
          reported = await host.stderr(problem.message)
          await host.exit(1)
        }
      }
    }
  }
}
```

The handler captures `started` and `requests`; request/response are its parameters,
not captures. This counts entries to this handler for this server instance,
including clock failures and failures after increment. It does not count unrelated
routes or requests rejected before invoking the handler. Uptime starts at the
startup reading and is an exact duration, not a wall-clock date. `as_seconds()`
returns an exact rational; choosing decimal formatting is separate presentation.
A restarted process starts its count again.

An equivalent version using a named handler and the existing contextual server
must also be provided. Anonymous functions are an option, not a prerequisite for
state. Both forms must produce the same counter and failure behaviour.

## Diagnostics and acceptance cases

Diagnostics must identify the source expression, explain the violated rule and
show a relevant remedy. Avoid internal instruction numbers or vague effect errors.
These are proposed wording examples, with actual source excerpts/locations supplied
by the implementation:

- `count` is mutable and cannot be captured by this function. Bind a snapshot
  before creating the function, or use State when calls need to share updates.
- This state update calls `time.now()`, which reads the clock. An update callback
  must be pure. Read the clock before the update and pass the resulting value in.
- This state update awaits work. An update must finish without suspending so
  another task cannot change the cell partway through it.
- This function captures `response`, a response writer whose lifetime belongs to
  the current request. Pass the writer to the function when it is called instead.
- State cannot store this payload: field `callback` contains a callable value.
  Store the data the operation needs and keep the callback outside the cell.

Acceptance includes:

1. Pure, ordinary and async anonymous functions; expression/block bodies; explicit
   result annotations and useful failures for incompatible results/effects.
2. Existing named references and collection callbacks retain behaviour; `.call`
   checks arity, type and effect. Source and saved bytecode agree.
3. Returned, nested and copied closures retain captures after the creator returns.
   Snapshot, shadowing and per-call local mutation have unambiguous tests.
4. Mutable capture and restricted capabilities fail through nested containers,
   aliases and generic instantiations, not only the simplest direct spelling.
5. State aliases share updates; separate cells stay independent; ordinary/async
   interoperability works. Reads are snapshots and updates return committed values.
6. Overlapping updates never lose an increment; callbacks execute once. Scheduling,
   cancellation before/after commit, failure before commit and retained closures
   are covered with controlled tests rather than timing-dependent sleeps.
7. Bad bytecode cannot forge captures or State handles, violate their types/effects,
   escape host capabilities, or bypass update restrictions. Resource cleanup is
   bounded across creation/drop loops and cancellation.
8. Both forms of the real HTTP example run from a relocated installation; clock
   failure gives the stated 503/count behaviour. Pure callback examples work
   independently of HTTP. Explain output identifies effects, capture origins and
   rejected captures; documentation and website examples match the final contract.

## Delivery after contract approval

Provisional weights describe this bounded programme, not all RM#162 or alpha.16.
Implementation has not started and all earned contributions are zero.

| Slice | Deliverable | Weight | Status | Earned |
| --- | --- | --- | --- | --- |
| CS1 | Anonymous functions using existing callable types; effects and diagnostics | 20 | Proposed | 0 |
| CS2 | Safe retained captures, verifier/lifetime support and explanations | 30 | Proposed | 0 |
| CS3 | Ordinary/async State API, atomic update and cancellation semantics | 30 | Proposed | 0 |
| CS4 | Both service examples, installed acceptance, reference and website guidance | 20 | Proposed | 0 |
| Total | Working callable/state feature | 100 | Contract review | 0 |

Order: CS1, then CS2 and CS3, then CS4. CS3 can use named pure callbacks; it need
not depend on closure syntax. Implement related work in a development branch with
local focused checks and no CI, following the alpha.16 trial. Batch integration
where practical; provisionally two or three implementation PRs, not one PR per
small edit. The contract itself is one documentation PR.

Before runtime edits, confirm closure representation, value ownership and verifier
changes against the existing VM. Do not promise to preserve the bytecode version:
if closure/cell representations require a format change, document compatibility
and bootstrap migration before implementation. Existing ordinary function-call
performance is a regression workload; measure named calls, noncapturing/capturing
callbacks, repeated state updates and allocation cleanup. No performance claim is
made by this design-only change.

## Owner review checkpoint

The recommended answers needing agreement are: effect inference for unmarked
anonymous functions; rejection of implicit mutable capture; module-level State
operations with pure nonsuspending updates; and data-only State payloads/restricted
capability capture. Approve or revise this contract before implementing it.
Website documentation and the earlier illustrative article need review against the
final design; no website or article changes are authorised by this document.
