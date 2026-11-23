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

Parameters may omit types when an already-known expected callable signature
supplies them. For example, `state.update(requests, (current) => current + 1)`
gets Nat from State[Nat], and a route handler gets its parameter types from the
route method. An isolated `(current) => current + 1` still requires an annotation.
Resolve the callback parameter types from the receiver, other typed arguments or
an annotated target before checking an unannotated callback. Its locally inferred
result may then determine remaining generic result parameters. For example, with
`values: [Nat]`, `values.map((x) => x + 1)` obtains x: Nat from the receiver, then
infers the callback result and map result parameter U as Nat. Not every generic
argument must be known before the body is checked. Do not infer an unknown input
parameter type backwards from its body, solve circular constraints or guess among
overloads. Explicit annotations must match the expected parameter types. Context supplies types, not permission to perform effects.
A result annotation is optional when the body determines a single result type;
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

All closure environments reject resource-bearing captures, including closures
called immediately. There is no escaping/nonescaping distinction or escape-analysis
work in this batch. Apply this recursively through records, enums, collections,
generic instantiations and the environments of captured callables.

The prohibited native categories at this baseline are:

| Native category | Public values affected | Reason |
| --- | --- | --- |
| HTTP server state (`V_HTTP_STATE`) | Server, ContextualServer and raw HttpServerState | The server retains handlers; capturing it can create a cycle |
| HTTP writer state (`V_HTTP_WRITER`) | ResponseWriter / HttpResponseState | Lifetime and mutation belong to one response |
| HTTP connection (`V_HTTP_CONNECTION`) | Native connection capability and wrappers | Request/connection lifetime |
| Directory stage (`V_DIRECTORY_STAGE`) | Staged filesystem publication handle | Owned publication/cleanup lifecycle |
| HTTP stream (`V_HTTP_STREAM`) | Streaming transport handle and wrappers | Owned stream/cleanup lifecycle |
| SHA-256 context (`V_SHA256_CONTEXT`) | Digest resource handle | Mutable resource/cleanup lifecycle |
| gzip decoder (`V_GZIP_DECODER`) | Decoder resource handle | Mutable resource/cleanup lifecycle |

Tasks and suspended execution contexts are also ineligible. New resource kinds
must be classified before being made capture-eligible; generic erasure and raw
bytecode cannot bypass the check. This is a structural capability check, not a
ban on every type returned by a native API. Instant, Duration, Path, Bytes,
filesystem metadata/root identity, completed process results and ordinary request
metadata are data and can be captured when all their fields are eligible. An
iterator may be captured only with its existing value/snapshot semantics and an
eligible backing value; never by retaining a suspended execution context.

Other safe closures and State handles are permitted. The State payload restriction
below prevents a cell from retaining a closure or another cell and completing a
cycle. Restricted resources may still be passed as call parameters and used under
their existing lifetime rules; this proposal does not introduce ownership transfer.

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
serialised or ordered. Equality compares cell identity, never the current payload:
two aliases of one cell compare equal, and two separate cells containing zero do
not. Comparison is pure and does not acquire the cell reservation or read its
contents. Generic `same[T](a: T, b: T): Bool { a == b }` and recursive container
comparison use this same rule when T contains State. This covers arrays, record
fields and Map values. Current scalar-type restrictions on Map keys and Set
elements remain unchanged: neither State nor callable values become eligible
keys/elements merely because equality is defined for them.
Its lifetime ends when its last reachable reference is released. There is no
manual close or use-after-close state.

Callable equality also needs a defined rule. Preserve existing named-reference
equality. For anonymous functions, compare the lexical function-body identity and
the captured values recursively in that body's capture layout. Copies compare
equal; separate evaluations of the same body with equal captures also compare
equal. Different lexical bodies do not compare equal merely because their source
text matches. Captured State values compare by cell identity, without reading
payloads. No new allocation identity is observable for closure creation, so creating
a pure closure does not introduce a hidden identity-generating effect. Widening a
PureFn to Fn preserves its callable identity. Collection comparisons follow the
same rule for eligible container positions, including Map values. This proposal
does not add callable hashing or broaden Map-key/Set-element eligibility.

Payloads are immutable data values: primitives and recursively compatible records,
enums and collections, including guarded types. State handles, callable values,
tasks and native/resource handles are excluded recursively. The compiler must
check generic instantiations too. This deliberately avoids cycles through cells
and callbacks, hidden resources and unresolved ownership rules. A new public trait
system is not part of this proposal.

`read` returns a snapshot. `update` reserves the cell, reads its current value,
invokes the pure transformation once, and atomically replaces the payload on
successful completion. The return value is that committed new value. All other
reads and updates of that cell wait for the reservation to be released, including
ordinary and async callers. Unrelated cells and tasks can progress. Two successful
overlapping increments of zero return 1 and 2 in some order and leave 2 stored.
Task scheduling, waiter fairness and response delivery order are not promised.

The callback cannot await, read another cell, write a cell, perform I/O or call
ordinary effectful functions. This prohibits source-level suspension and reentrant
state access, not scheduler preemption. The VM continues its ordinary scheduling
and deadline checks while executing the callback. A waiting ordinary call must not
block the VM's scheduler or the reservation owner. This requires internal cell
reservation/waiting support, not a new public lock API or an OS worker thread.

Purity is not a termination or time bound. Other tasks can run while a long callback
holds this cell, but callers waiting for that cell cannot proceed until completion,
failure or cancellation. Existing timeouts remain applicable; no automatic new
state-update deadline is introduced. A nonterminating update without a cancellation
or timeout can therefore keep its cell reserved indefinitely.

Cancellation or a trap before commit discards any candidate replacement, retains
the old payload, releases the reservation and wakes waiters. Cancellation while
waiting removes the waiter without invoking its callback. A callback that has
started is never retried: successful updates execute it once, aborted updates
execute it at most once and may stop partway through. Commit is the indivisible
boundary: cancellation before it prevents replacement; cancellation after it cannot
undo replacement, even if the caller never receives the result. The verifier and
runtime must enforce these rules through ordinary and async entry points alike.

Allocation failure follows the existing VM failure policy and must release any
reservation during unwinding. These operations have no invented recoverable
host-error Result wrapper. No durable storage, multi-cell transactions, actors,
public locks or channels are introduced.

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
            response.send("Requests counted: ${total}; uptime unavailable: the clock could not be read.")
            Ok(())
          },
          Ok(now) => {
            elapsed = now.difference(started)
            seconds = elapsed.as_seconds()
            response.send("Requests counted: ${total}; uptime: ${seconds} seconds")
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
not captures. This counts committed increments for this server instance, including
clock failures and cancellations or failures after the increment commits. A request
cancelled after handler entry but before commit is not counted. It does not count
unrelated routes or requests rejected before invoking the handler. Uptime starts at the
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
- This state update awaits work. Its callback cannot await work while
  holding the cell reservation. Await that work before starting the update.
- This function captures `response`, a response writer whose lifetime belongs to
  the current request. Pass the writer to the function when it is called instead.
- State cannot store this payload: field `callback` contains a callable value.
  Store the data the operation needs and keep the callback outside the cell.

Acceptance includes:

1. Pure, ordinary and async anonymous functions; expression/block bodies; explicit
   result annotations, contextual parameter typing, annotation-required failures
   without context, and useful failures for incompatible results/effects. With
   `values: [Nat]`, both `values.map((x) => x + 1)` and
   `values.map((x: Nat) => x + 1)` produce [Nat]; a body returning Str determines
   [Str] without requiring the result parameter U to be known beforehand.
2. Existing named references and collection callbacks retain behaviour; `.call`
   checks arity, type and effect. Source and saved bytecode agree.
3. Returned, nested and copied closures retain captures after the creator returns.
   Snapshot, shadowing and per-call local mutation have unambiguous tests.
4. Mutable capture and restricted capabilities fail through nested containers,
   aliases and generic instantiations, including immediate closures. Test every
   listed native capability and allowed immutable native-produced data. A handler
   cannot capture its owning server and form a retained-reference cycle.
5. State aliases share updates; separate cells stay independent; ordinary/async
   interoperability works. Reads are snapshots and updates return committed values.
   Direct/generic/nested State equality, including arrays, records and Map values,
   uses stable cell identity. State and callable Map keys/Set elements remain
   rejected under the existing scalar-type restrictions. Callable equality distinguishes lexical bodies, compares captures,
   preserves named references and has no fresh allocation identity.
6. Overlapping successful updates never lose an increment. Test unrelated-task
   progress and request deadlines during a long callback, same-cell read/update
   waiting, ordinary/async contention, waiter cancellation, cancellation partway
   through a callback, failure before commit, cancellation after commit, reservation
   release and no retries. Use controlled scheduler tests, not timing-dependent
   sleeps. Verify the HTTP count at the commit boundary, not handler entry.
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

The recommended answers needing agreement are: effect inference and contextual
parameter typing for anonymous functions; rejection of implicit mutable capture;
module-level State operations with scheduler-preemptible atomic updates; cell
identity equality and structural closure equality; and data-only State payloads
with the explicit capture restrictions above. Approve or revise this contract before implementing it.
Website documentation and the earlier illustrative article need review against the
final design; no website or article changes are authorised by this document.


## Review corrections, 23 November 2026

The owner requested addressing Frank's review of PR #700. This revision separates
atomicity from scheduler preemption, specifies before/after-commit cancellation and
reservation cleanup, defines generic/container equality for State and closures,
lists prohibited capture capabilities without escape analysis, allows parameter
types from an already-known callable context, and counts committed increments in
the service. These are revised recommendations, not implementation or final owner
acceptance. The four slice weights and zero earned contribution are unchanged.

Second review clarification, 23 November: callback input types must be known
before body checking, but its local result can determine remaining generic output
parameters, including map's U. Existing scalar Map-key/Set-element eligibility is
unchanged; State/callable equality applies in allowed value positions. Acceptance
cases cover both rules. No scope, weight or implementation-status change.
