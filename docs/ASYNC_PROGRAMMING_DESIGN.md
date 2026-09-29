# Writing asynchronous programs in Panackelty

Proposal for review, 2026-09-29. Baseline: `a7577eb` (merged PR #96).
Work record: [issue #97](https://github.com/sproates/panackelty/issues/97).
[ROADMAP.md](../ROADMAP.md) owns priority and agreed decisions.

## The decision in plain language

A server needs to wait for one person without making everyone else wait too.
We've proved that the VM can pause work and manage related tasks using fake
operations. We now need to decide how someone asks for that behaviour in code.

Recommend `async` functions with explicit `await`, plus `spawn` inside an owning
scope when work should run alongside the caller. Read a message, prepare a reply,
then send it. The waiting points stay visible in that sequence. Keep named event
callbacks for hosts such as a UI toolkit, where receiving events is already the
natural interface.

The proposed words mean:

| Form | Meaning |
| --- | --- |
| `async reply(...)` | This function may wait for something outside the program. |
| `await read_message(...)` | Start this operation and wait for its result; other ready tasks can progress. |
| `spawn clients reply(...)` | Start a child owned by `clients` and continue without waiting for its result yet. |
| `scope clients(...) { ... }` | Keep these children together; leaving the scope waits for cleanup and accounts for every child. |

These are recommendations, not accepted language features. This PR changes no
compiler, VM, language specification, bytecode or host ABI. Approval to publish
this proposal is separate from accepting its design or implementing it.

## One small server, expressed two ways

**Every code block below is non-executable design pseudocode.** `async`, `await`,
`spawn`, `scope`, `move`, resource types and service names are proposed. The server
sketch also uses shorthand `break`, `return`, named arguments and time literals;
these aren't additions to today's grammar. Only the core forms above are being
recommended for a first spelling review. No closures or general exception syntax
are assumed.

Use a loopback request/reply demonstration: accept at most 16 clients in one run,
read one message of at most 4 KiB from each, and send one bounded reply. Sixteen is
a review/example limit, not a production default. A stalled client mustn't stop
another client's reply. Each client has a five-second absolute deadline. A stop
request ends admission, allows two seconds to drain, then cancels remaining work.
The listener backlog, buffers and pending requests also have explicit bounds.
HTTP, TLS, DNS and indefinite service operation are outside this example.

Both versions use the same proposed backend contract. `read_message` distinguishes
EOF/disconnect, oversized input and I/O failure; `write_all` handles partial writes
and reports failure after any partial side effect. Per-client deadline expiry is
task cancellation. Expected I/O errors become a `ClientReport` value. Runtime
traps fail the owning scope. Reports are retained for at most 16 admissions.

### Explicit await

```text
async reply(client: Client): ClientReport {
  match await read_message(client, max_bytes = 4096) {
    Error(problem) => ReadFailed(problem),
    Ok(message) => {
      response = make_reply(message)             // pure, bounded application work
      match await write_all(client, response) {
        Error(problem) => WriteFailed(problem),
        Ok(done) => Replied()
      }
    }
  }
}                                               // child-owned client closes here

async serve(listener, stop): ServerReport {
  scope clients(max_tasks = 16, cancel_policy = collect) {
    repeat at most 16 times {
      match await accept(listener, stop) {
        Stopping() => break,
        Failed(problem) => {
          cancel clients
          outcomes = await drain(clients, grace = 0s)
          return ServerFailed(problem, outcomes)
        },
        Connected(client) => {
          match spawn clients reply(move client) with deadline = now() + 5s {
            Error(problem) => {
              cancel clients                  // rejected transfer already closed client
              outcomes = await drain(clients, grace = 0s)
              return ServerFailed(problem, outcomes)
            },
            Ok(task) => {}                      // scope owns and observes this task
          }
        }
      }
    }
    outcomes = await drain(clients, grace = 2s)
    ServerFinished(outcomes)
  }
}
```

The enclosing root owns the listener and closes it on exit. `accept` can be
interrupted by the stop signal while no client is arriving. There is capacity
for each of the 16 admissions before acceptance; a general reusable server would
need an explicit reserve-before-accept operation. The example does not queue
unlimited sockets or accumulate reports forever. A stop during drain never opens
another admission window.

`move client` transfers the accepted resource into the child. On activation
failure the spawn operation consumes and closes it rather than leaving ambiguous
ownership. If cancellation happens between acceptance and spawn, the accepting
scope still owns and closes it. The task can return a report, but never its live
client handle. Expected read/write errors are recorded in that report, rather
than silently losing the operation's outcome or cancelling unrelated clients.

### Named callback and state-machine alternative

The same application can keep an explicit state record and pass named functions
to an adapter. It still needs the same bounded owner and shutdown rules.

```text
ClientState = Reading(client) | Writing(client) | Finished(report)

pure on_client_event(state, event): Transition {
  match (state, event) {
    (Reading(client), ReadOk(message)) =>
      Next(Writing(client), WriteAll(client, make_reply(message))),
    (Reading(client), ReadError(problem)) =>
      Finish(ReadFailed(problem)),
    (Writing(client), WriteOk()) => Finish(Replied()),
    (Writing(client), WriteError(problem)) => Finish(WriteFailed(problem)),
    (_, Deadline()) => Cancel(TimedOut()),
    (_, ParentCancelled(reason)) => Cancel(reason),
    otherwise => FailScope(ProtocolFault())
  }
}

on_server_event(state, event): ServerTransition {
  match event {
    Accepted(client) => StartOwnedClient(client, Reading(client), @on_client_event),
    ClientFinished(id, outcome) => RecordOutcome(id, outcome),
    StopRequested() => StopAdmissionAndDrain(2s),
    AcceptFailed(problem) => CancelChildrenAndFail(problem),
    DrainExpired() => CancelRemainingAndJoin(),
    AllChildrenJoined() => FinishServer()
  }
}
```

`Transition` and commands here are descriptive notation, not existing tuples or
copyable resource records. The adapter owns the client and executes one validated
command at a time. Admission/start failure closes the client. `Finish` releases
it once; `Cancel` first waits for backend quiescence. The adapter filters stale
and duplicate operation ids before dispatch, closes a client delivered after
admission stops, and invokes callbacks through a bounded VM pump. Callback return
values cannot smuggle a handle outside its owner. `CancelChildrenAndFail` joins cancellation and includes already-observed child
outcomes in the server failure report. Queue overflow stops admission or reports
an explicit failure, rather than discarding a completion needed for cleanup. These obligations apply equally to the await implementation.

| Question | Explicit await | Callbacks and state |
| --- | --- | --- |
| Where is the read-then-write sequence? | Together in `reply`. | Split between state/event cases. |
| Where is unfinished state kept? | Owned VM frames. | Adapter-owned state records. |
| What changes when adding another I/O step? | Add an awaited operation and its error branch. | Add state, completion events and transitions. |
| Where do late results and cleanup belong? | Runtime/backend, with source-visible ownership. | The same runtime/backend obligations; callbacks don't remove them. |
| Main implementation cost | New syntax, effects, verifier rules and bootstrap migration. | A command/state protocol and ownership rules; existing named callables alone aren't enough. |

Prefer await for sequential application work. Callback state machines remain a
useful host boundary. This is a design judgement from the worked example, not a
measured usability result. A small programmer/agent trial should challenge it
before we describe the syntax as easy to use.

## Activation and types

Recommend a restricted first version with **no first-class dormant operation
value**. A bare `reply(client)` is a compile error when `reply` is async. It neither
starts eagerly nor creates an object that can be forgotten. `await reply(client)`
starts it in the current task and produces `ClientReport`; `spawn clients
reply(move client)` creates a separately scheduled child. Spawn evaluates arguments
once, left to right, commits ownership on admission, and never runs child bytecode
inline. Allocation/admission failure produces a typed error with the transfer
cleanup described above. Cancellation after successful admission belongs to the
new child. Awaiting immediate completion is still a cancellation checkpoint; it
needn't force another task to run.

A named reference `@reply` has proposed type `AsyncFn[Client,ClientReport]`, following
today's `PureFn[A,R]` / `Fn[A,R]` notation. Indirect async invocation requires the
same await/spawn forms and runtime checks as a direct call. There is no implicit
conversion between `AsyncFn` and either existing callable type. Pure functions
remain ordinarily callable from async code, without becoming async themselves.
For the first version reject `pure async`. Use `Unit` for no-data task results;
`Void` remains today's non-storable return marker, never a `Task[Void]` type argument.
An `async main(): Unit` would let the CLI drive one root scope; ordinary synchronous
`main(): Void` keeps its behaviour. Hosts pump that root through their own loop.
Neither path runs a nested blocking event loop from an ordinary function.

Inside async functions permit pure calls, awaited `AsyncFn` calls and a small,
explicit set of cooperative task/resource primitives. Reject arbitrary ordinary
`Fn` calls, including today's blocking print, file, sleep and process services.
This restriction also applies indirectly and through higher-order helpers; making
an impure helper async does not make its blocking internals legal. Future effect
inference could relax it, but is not assumed here. Pure computation may still take
a long time; instruction budgets aren't wall-clock latency guarantees.

## Ownership, results and stopping

A `scope` binds a compiler-known owner, not an ordinary copyable record. Task and
resource handles are opaque and scoped. In the first version they may be local
bindings and explicitly borrowed parameters of awaited calls; reject returning
them, capturing them, placing them in user records/collections, serialising them,
or passing them through an unconstrained generic or ordinary `Fn`. Spawn requires
an explicit transfer for resources; the source binding becomes unavailable on
every outcome, including failure. This is a restricted capability discipline,
not a claim to have designed a general borrow checker. Runtime ownership,
generation and service-type checks remain necessary for forged bytecode.

| Situation | Proposed observable behaviour |
| --- | --- |
| Child returns a value | Scope retains it until observed by join/drain. Normal scope exit joins all children; non-`Unit` outcomes must be consumed explicitly, not silently discarded. An unhandled trap/cancellation instead transfers reporting to the root host. |
| Child returns `Error(e)` inside a normal `Result` | Ordinary data, not automatically a task failure. The caller must handle or propagate it; the scope cannot infer intent from a variant name. |
| Read/write fails in the example | `reply` returns `ReadFailed` or `WriteFailed`; the server report includes it. A partially written response cannot be rolled back by cancellation. |
| Child traps or violates a runtime contract | Cancel siblings and join cleanup before failing the scope. Preserve available non-cancellation failures in creation order for the root host/CLI report. No catchable language exception facility is proposed. |
| Child is individually cancelled or times out | Default scope policy fails/cancels siblings. Explicit `cancel_policy = collect` isolates expected per-client cancellation and includes `Cancelled(reason)` in joined outcomes. It never converts traps into ordinary results. |
| Parent/root is cancelled | Cancel all descendants and await quiescence, even under `collect`. Cancellation is a terminal control outcome; user code cannot turn it into ordinary success. |
| Scope returns early | Still join and release owned resources; explicit cancellation requests stop unfinished work first. No child outlives the scope. |
| Stop requested in the server | Stop admission, then drain until the earlier of the two-second grace limit or an inherited deadline; cancel and join remaining children. This graceful stop is distinct from immediate parent cancellation. |
| Completion races cancellation | Delivery is serialised on the owning thread. Cancellation before queued delivery wins. A delivered operation may have had effects even if the task is subsequently cancelled; no double release. |

`await drain` returns bounded outcomes in task-creation order, including cancellations
allowed by `collect`; it cannot return normally through an unhandled trap.
For the first source version, consuming a non-`Unit` await/join result means binding,
matching or returning it; a discarded expression is an error. This catches lost
work/results without pretending to prove that an application meaningfully uses
every bound value. It doesn't retrospectively change all synchronous `Result` uses.
Scope exit may therefore suspend even without an explicit final await. That suspension
is part of the visible scope construct and is permitted only in async functions.
Runtime-owned cleanup runs without user code. Asynchronous user finalisers,
shielding and forced termination of arbitrary blocking native calls are deferred.
A real backend must retain request storage until its producer has stopped; a
cancellation request or an invalidated identity alone doesn't prove that.
The two-second grace limit bounds voluntary draining, not total teardown time;
a hard shutdown bound needs separate evidence from the selected backend.

## UI cross-check

A toolkit's named event handler can return a command to start window-owned async
work, leaving its callback synchronous and short:

```text
Refresh => CancelPreviousAndStart(window, generation + 1, @load_text)
Loaded(generation, text) => ApplyOnlyIfCurrent(generation, text)
LoadFailed(generation, problem) => ShowErrorOnlyIfCurrent(generation, problem)
Close => StopEventsAndCancelWindowWork()
```

The window adapter owns tasks with an explicit collect-cancellation policy. It
marshals bounded results back to the UI thread; async workers don't call widgets.
Refresh may cancel the old task without failing the new one. A late result must
still pass the generation check. Closing stops new callbacks, cancels work and
joins producer cleanup before releasing host state. The toolkit retains its event
loop; no GUI library, capture syntax, mobile lifecycle or toolkit API is selected.

## What the existing experiment does and doesn't establish

| Repository evidence | Consequence for this proposal |
| --- | --- |
| [Resumable frames](../src/vm/vm.h) and [task session](../src/vm/tasks.h) preserve waits, joins, budgets and generation-qualified completions. | Reuse the ownership model, but don't mistake an internal C handle for a language feature. |
| [Current task failure propagation](../src/vm/tasks.c) turns independent child cancellation into parent failure. | The default above follows it. The explicit collect policy, structured cancellation reasons and drain grace period are new design work, not tested capabilities. |
| Task slots/results stay reserved until session destruction. | The finite demonstration fits that bound. An indefinitely running server needs safely reusable slots and observed-result reclamation before it can ship. |
| The fake service returns only a print acknowledgement or static error. | Typed read/write completions, resource transfer and native producer quiescence still need proof. |
| [Callable checking](../src/compiler/checker.panack), [purity](../src/compiler/purity.panack) and [function parsing](../src/compiler/parser.panack) currently distinguish pure/ordinary functions. | Async calling contexts, callable kinds, resource-flow diagnostics and cooperative service restrictions require coordinated compiler work. |
| [Version 8](../src/bytecode/FORMAT.md) reserves every function-flag bit except purity and defines immediate builtins. | A public async contract needs a new bytecode version. Do not repurpose reserved bits or silently reinterpret v8 calls. |

## Smallest implementation to consider after review

Recommend **one source-to-VM await slice using a fixed fake service**, estimated
M–L including compiler, verifier, bootstrap, tests and documentation. Prove an
`async main(): Unit` awaiting a named async helper and a typed fake completion,
with cancellation from the host. Include direct and indirect async-call checks;
reject spawn, scopes and resource values in that first slice. It demonstrates
source-level suspension, not the complete server above. Full scoped-source and
resource delivery is a later L-sized step and is not authorised here.

Before coding, reserve the next unused bytecode version and write its flags,
instructions, call legality, completion-result validation and terminal contract.
Ordinary calls must not enter async functions; pure functions must not reach
host effects through direct or indirect calls. Unsupported flags/opcodes and
wrong-kind completions must fail closed. Add the required async/result metadata
rather than assuming v8 encodes a source signature. Keep the historical v8 compiler
runnable during migration, use it to build a compiler that emits the new format,
keep the migrating compiler implementation within the old accepted source subset,
then prove repeated compiler/stdlib builds converge and deliberately refresh the
seed, launcher and package contracts. Remove temporary dual-version paths once
that documented bridge is no longer needed. Record v8 support or deliberate
retirement in the release policy and test clear version-mismatch diagnostics;
there is no promise that old saved artifacts will run forever. This PR assigns
no opcode numbers.

The first slice's acceptance examples are concrete:

| Probe | Expected result |
| --- | --- |
| `await helper()` reaches a pending fake read, then receives bytes | Caller resumes once with the independently expected result; an interleaved task/host pump can progress while it waits. |
| Fake read returns a typed I/O error | The function's explicit error branch runs. Wrong runtime result kinds trap instead. |
| Host cancels before registration, while waiting, or after enqueue | One terminal cancellation and complete cleanup; late delivery cannot resume freed work. |
| Bare async call; async call in pure or ordinary `Fn` code | Compile-time error explaining the required async calling context. |
| Async body calls blocking `print`, or hides it behind a helper/callback | Reject before effects; forged direct/indirect bytecode must also fail. |
| `AsyncFn` coerced to `Fn`/`PureFn`; await on an ordinary call | Reject the incompatible callable use. |
| Existing synchronous source and saved artifacts under their supported version | Preserve declared behaviour through the migration; check installed CLI and bootstrap identity. |

For the later scope/resource slice also require negative tests for a moved client's
reuse, escaping handles through returns/containers/generics, forgotten non-Unit
outcomes, foreign-owner tasks and spawn outside a scope. Positive scenarios must
cover two clients with one stalled, expected I/O errors, unexpected child traps,
stop during accept/read/write, deadline versus completion, and shutdown with late
native callbacks. Keep full `make check`, sanitizer, fault and packaging gates;
review the hypothetical examples here separately rather than counting them as tests.

A real loopback TCP/timer spike in a C harness remains a credible alternative to
the source slice if native cleanup is the uncertainty we want to retire first.
Its M–L effort buys backend evidence sooner, but doesn't test language usability.
Before freezing resource or cancellation semantics, require that spike to prove
partial I/O, stopped producers and bounded ownership. It must also compare backend
dependency/packaging costs. Compiler-assistance work remains a smaller alternative
if we decide immediate improvements to existing programs should come first.

## Precedents and review limits

Python distinguishes calling a coroutine from scheduling a task, and task groups
join their children. This illustrates why activation and ownership need separate
rules; the proposed bare-call error avoids adding a dormant operation value in
our first version. We aren't adopting Python exceptions or cancellation handling.
[Python asyncio documentation](https://docs.python.org/3/library/asyncio-task.html).

Tokio documents that spawning does not synchronously poll the task. That is a
useful precedent for the proposed no-inline-child guarantee, not a selection of
Rust's type system or runtime. [Tokio spawn documentation](https://docs.rs/tokio/latest/tokio/task/fn.spawn.html).

libuv documents that even a successfully cancelled request must not be freed
before its callback. This is why the fake-host tests cannot establish native
cleanup. [libuv request documentation](https://docs.libuv.org/en/v1.x/request.html).
Primary sources reviewed on 2026-09-29; none is a selected dependency.

The remaining review decisions are the restricted await/spawn activation model,
the initial ban on ordinary impure calls in async bodies, and the explicit
collect-cancellation scope policy. Exact resource grammar and backend choices
remain provisional. No user/agent usability study or real-network test was run
for this proposal. We should review those trade-offs before authorising the first
implementation slice.
