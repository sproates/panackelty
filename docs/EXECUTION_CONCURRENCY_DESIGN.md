# Execution, concurrency and host integration — proposal

Date: 2026-09-29. Baseline: `859fd5ac4e3facc706d4a8198af3c4eee38610df`
(PR #90). Work record: [issue #91](https://github.com/sproates/panackelty/issues/91).
[ROADMAP.md](../ROADMAP.md) owns current priority and state.

This is a design investigation, not an implemented language contract. The user
authorised comparison and a recommendation, not an execution model or syntax.
Merging this report does not authorise the implementation sequence below.
The [specification](../SPEC.md), [bytecode contract](../src/bytecode/FORMAT.md)
and [host ABI](../src/runtime/ABI.md) remain unchanged.

## Recommendation for review

Build toward **structured cooperative tasks on one VM-owning thread**, with
host-driven event delivery and explicit suspension points. Prefer an eventual
`async/await`-style application interface over mandatory callback chains. Keep
named callbacks plus explicit state for event adapters, including GUIs. Separate
this direction from the exact spelling and type rules, which need another review.

First prove resumable execution, bounded dispatch and host-owned lifetime using
a fake host and no network. Then prove task ownership and timer cancellation.
Only after those gates select the first networking API and backend. Keep simple
synchronous scripts working; do not make every program manage an event loop.
Do not begin with shared-memory language threads, unrestricted FFI or a mandatory
global event loop. CPU parallelism should remain possible through later isolated
workers, without pretending it has already been designed or implemented.

Why: the pilot's two HTTP failures establish missing capability, not a preferred
scheduler. The architecture supports immutable data and explicit effects, but
has no suspendable call stack. A cooperative model could give readable sequential
I/O code while respecting existing single-threaded ownership. This is a design
judgment, not performance evidence or proof that developers prefer the syntax.

## Current source evidence

| Evidence | Consequence for a design |
| --- | --- |
| [execute.c](../src/vm/execute.c): `execute` owns a local `Frame`; `OP_CALL` and `OP_CALL_VALUE` recursively invoke it | Saving only the instruction pointer cannot suspend the whole call chain. Frames, arguments, operand stacks, iterators and returns need durable ownership. |
| [vm.h](../src/vm/vm.h): borrowed program/argument/environment pointers and one `error` field | A suspended execution needs explicit lifetime rules; tasks cannot share one mutable error slot. This is an internal struct, not a supported embedder ABI. |
| [value.c](../src/vm/value.c) and [value model](../src/vm/VALUE_MODEL.md): non-atomic counts and acyclic values | Keep retain/release on the VM-owning thread initially. Immutable source values do not make the implementation thread-safe. |
| [builtins.h](../src/vm/builtins.h) and [builtins.c](../src/vm/builtins.c): fixed name/arity/purity registry, immediate `Value *` result | A pending host call needs a distinct internal outcome and validated completion contract, not a null pointer masquerading as a result. |
| [host.c](../src/vm/host.c): `process_exit` calls C `exit`; [host_capabilities.c](../src/vm/host_capabilities.c): sleep uses `nanosleep`, subprocess handling owns a blocking poll loop | Existing bounded operations can still monopolise the VM thread. An embedded session must not terminate the containing GUI/browser process. Internal polling is not general async support. |
| [builtins_vm.c](../src/vm/builtins_vm.c): nested bytecode runs synchronously with borrowed snapshots | Nested invocations must inherit budgets/cancellation or be rejected in the cooperative mode; otherwise they bypass its guarantees. |
| [SPEC callable rules](../SPEC.md#effects-and-purity): `@name`, `PureFn`/`Fn`, no captured local state | Explicit state can support the first callback experiment. Closures are not a prerequisite, but asynchronous function types and state ownership cannot be hand-waved. |
| [bytecode format](../src/bytecode/FORMAT.md): version 8, reserved function flags and verified calls | Scheduler internals can be prototyped without changing source or bytecode. New async flags, instructions or executable semantics require coordinated versioning, verifier, compiler and bootstrap work. |

These observations extend [assessment E1/E2/E6](ADOPTION_ASSESSMENT.md).
No runtime prototype, platform port or throughput measurement was performed for
this report. Source inspection is high-confidence evidence for the constraints;
cost and usability estimates remain provisional.

## Alternatives and effort

Concurrency means overlapping activities; parallelism means simultaneous
execution. An event loop is a scheduling/I/O mechanism; callbacks and
`async/await` are programming interfaces. These choices can be combined.
Effort below includes testing, docs, bootstrap and continuing maintenance, not
just a minimal demo. Sizes are relative, not delivery dates.

| Model | Useful properties | Cost, risk and fit | Recommendation |
| --- | --- | --- | --- |
| Blocking calls, one activity | Straight-line code; smallest initial socket experiment | S–M for bounded transport only; waiting or computing delays everyone else. Simple scripts fit; responsive servers and GUI hosts do not. | Keep synchronous workflows; do not freeze this as the universal host model. |
| Host event loop, named callbacks and explicit state | Natural event adapters; can start from current callable syntax | M for bounded adapter, L for a general lifecycle. Chained operations require state machines; errors and cancellation can scatter. An unbounded callback still freezes the host. | Useful integration layer and comparison prototype, not the sole application API. |
| Cooperative tasks, explicit await, structured scopes | Sequential-looking I/O with visible interleaving; lexical task ownership | L across resumable frames, scheduler, effect/type rules, cleanup and diagnostics. CPU-heavy work and blocking services need containment. | Preferred direction, subject to feasibility and user review. |
| Stackful tasks with implicit suspension in ordinary calls | Fewer async annotations; sequential code | L; suspension less visible, portable native-stack management harder, host re-entry and bytecode debugging need care. Heap VM stacks could enable variants without native fibers. | Compare ergonomics, but prefer explicit application suspension boundaries initially. |
| Shared-memory language threads | Potential CPU parallelism and direct blocking integrations | XL; atomic counts alone do not solve shared backing stores, host affinity, error state, callbacks or data races. | Defer until a demonstrated workload warrants the language/runtime complexity. |
| Isolated VM workers with messages | Potential CPU parallelism without shared `Value *` ownership | M–L; value transfer, queues, shutdown, program lifetime and overhead still need design. Processes are another isolation option. | Preserve as a future path; do not promise a worker API yet. |

For GUI/game/browser/mobile hosts, the host must be able to request a bounded
amount of VM progress and regain control. A command-line server may own a loop;
a toolkit or browser often owns it already. Neither implies that the same
network or UI APIs exist on every platform. Games also need frame-time and
numeric-performance investigation; this scheduler proposal is not a game engine.
Mobile suspension can prevent timers running: resume must recheck deadlines,
and abrupt process termination cannot guarantee application cleanup.

## Proposed runtime contracts

### Ownership, execution and host integration

1. An execution session owns tasks, a resource registry, pending operations and
   capability configuration. Each task owns its frame stack and terminal outcome;
   the decoded program and snapshots outlive all their tasks. Exactly one thread
   executes or retains/releases VM values for that session.
2. Replace native recursive VM calls with explicitly owned frames before allowing
   host suspension. An internal advance operation returns `Completed`, `Trapped`,
   `Exited`, `Yielded` or `Waiting`, with no borrowed C stack pointers surviving
   the call. These names are conceptual, not proposed public C declarations.
3. A host operation either completes immediately or registers one pending result.
   Completion is queued, never delivered by re-entering the VM inline. An operation
   has an owner, a generation-qualified identity and one terminal transition.
   Validate completion type, ownership and state even when source was checked.
4. A host pump advances runnable work within an instruction budget, exposes the
   next deadline and accepts readiness/completion notifications. Native standalone
   and toolkit-driven adapters share this boundary. Do not run competing nested
   event loops. Re-entrant invocation on the same session returns a defined error.
5. Start with a fixed typed service set and fake host. Extensible service
   registration requires its own capability, signature, purity and verifier
   contract; this proposal does not add arbitrary native pointers or dynamic FFI.
6. Worker threads, if a backend needs them, receive owned native buffers/requests,
   not shared VM values. Results are copied or ownership-transferred into a queue,
   then converted to VM values on its owning thread. Destroying a session first
   quiesces pending producers; a late callback must never target freed memory.

Instruction budgets bound interpreter dispatch, not elapsed time. A single exact
numeric operation, release walk or host call can take substantial time. Measure
these cases; split, bound or offload them before claiming latency guarantees.
Internal budget yields can preserve responsiveness even inside pure computation,
but do not make that computation an effect or authorise hidden host I/O.

### Tasks, scopes, errors and cancellation

- Default task creation belongs to a lexical scope. No detached work in the
  first design. Scope success waits for children. An unhandled child failure
  requests sibling cancellation and joins them before reporting failure.
  Keep all non-cancellation failures in task-creation order for inspectable
  reporting; expected I/O failures remain explicit `Result` values to handle.
- Cancellation is a distinct terminal outcome, not arbitrary successful data
  or an ordinary user-catchable I/O error. Parent cancellation propagates to
  descendants. Observe it at awaits and dispatch budget boundaries. Explicit
  recovery/shielding, if needed later, needs a separate bounded contract.
- Use monotonic absolute deadlines, inheriting the earlier parent deadline.
  Cancellation requests do not imply that native I/O has stopped. Resolve
  timeout/completion races once on the owning thread; discard late delivery
  while retaining native request storage until the backend is actually done.
- A failed/cancelled task runs runtime-owned cleanup once. Scope-owned native
  resources are not dependent on user code reaching `close`. Initially exclude
  arbitrary asynchronous user finalizers; graceful application shutdown is
  explicit work with its own bounded deadline before mandatory native cleanup.
- Do not publish a hard shutdown bound for non-cancellable native calls. The
  first supported asynchronous services must have bounded teardown; arbitrary
  blocking foreign work remains unsupported. Forced process death cannot promise
  cleanup or transaction rollback. Native OOM remains fatal under today's contract;
  recoverable embedder allocation failure would be a separate deliberate change.
- A scheduler trace should identify task/parent, wait reason, deadline and
  terminal outcome, with bounded storage. Source stacks require source mappings
  not currently present; do not claim them for the first runtime experiment.

### Resources, backpressure and security boundaries

Opaque handles refer to session-owned registry entries, not exposed file
descriptors. Copies alias the same identity; they are not independent ownership.
The creating scope owns the resource. Close invalidates every alias and is
idempotent; subsequent use returns a typed closed-resource error. Runtime checks
reject stale generations, wrong-session identities and forged bytecode values.
Do not serialize live handles or use final reference release as the only cleanup
policy. Initial parent-to-child borrowing stays within the owning scope's
lifetime; reject use after escape. Static non-escape/linear typing is a later
possibility, not a prerequisite silently assumed by this design.

Put explicit limits on tasks, accepted connections, pending operations, queued
events and buffered bytes. Reads and writes are bounded and may be partial;
waiting for write capacity must not grow an unbounded queue. EOF is distinct
from error. One pending read and one pending write per stream is a reasonable
first contract; simultaneous conflicting operations return a defined error.
Acquire a connection permit before admitting another request task. Overload
behavior must be explicit, not accidental memory exhaustion.

Capabilities restrict host services per session, but bytecode verification plus
capability flags is not a security sandbox. A browser adapter cannot promise raw
TCP listening just because a native adapter supports it. TLS, DNS, HTTP framing,
request limits and authentication are separate concerns with separate tests.

### Blocking compatibility and effects

Retain existing synchronous CLI behavior. In a new cooperative/embedded session,
classify every host service as immediate/bounded, suspendable, or forbidden.
Do not call today's `host_sleep`, `process_run`, file reads or console operations
on the event thread and label the session nonblocking. Provide new asynchronous
adapters or reject unsupported blocking calls before side effects; async call
graphs need corresponding compiler assistance and runtime enforcement.

Keep `PureFn` purity intact. Creating tasks, awaiting external activity and
accessing resources are effects. If explicit async syntax is chosen, its callable
type and permitted calling contexts must be represented and checked; it cannot
silently reuse today's `Fn` contract. Calling an async function must have one
defined activation rule (cold operation versus immediately scheduled task).
Recommend scope-owned explicit activation; unawaited work must be diagnosed,
not silently dropped. Exact syntax/type names remain open.

In embedded mode, `process_exit` must produce an invocation outcome or be
rejected under a new documented profile, never call native `exit`. Preserve
existing CLI semantics via its outer adapter. This is a compatibility decision,
not an incidental refactor. New pending host outcomes and async instructions
need an explicit ABI/version plan; do not retrofit incompatible behavior into
version 8 without the required coordinated update.

## Application-facing sketches for review

**All code below is non-executable design pseudocode.** Functions, types,
`async`, `await`, scope/resource constructs and error propagation are hypothetical;
no example is claimed to compile. `try` below means propagate a typed failure,
not a decision to add exceptions. The examples expose semantic requirements;
they are not a specification or a selected GUI/HTTP library.

### Server: sequential I/O inside bounded concurrent handlers

```text
async handle(client, request_deadline): Result[Unit,ServerError] {
  request = try await read_request(client, max_bytes = 65536,
                                  deadline = request_deadline)
  response = make_response(request)       // pure application logic
  try await write_response(client, response, request_deadline)
  Ok(())
}

async serve(listener, shutdown): Result[Unit,ServerError] {
  task_scope children(max_tasks = 64) {
    while !shutdown.requested() {
      permit = try await children.reserve()
      client = try await listener.accept(shutdown)
      children.start_owned(permit, client, @handle, deadline_after_seconds(10))
    }
  }                                     // join/cancel children, close resources
  Ok(())
}
```

`start_owned` transfers the accepted resource's scope ownership to the child;
if activation fails it closes the resource and returns the permit. Cancelling
accept also returns its unused permit. The handler wrapper logs/handles expected
disconnects so they do not cancel the whole server; unhandled internal failures
do trigger the chosen scope policy. Shutdown stops admission, allows a bounded
drain interval, then cancels remaining work. The request parser, response model
and HTTP limits are separate work, not delivered by sockets alone.

The callback alternative expresses the same lifecycle as `Accepted`,
`ReadComplete`, `WriteComplete`, `TimedOut` and `Closed` events plus an explicit
per-client state record. This can work with named functions, but every branch
must coordinate permits, buffers and one terminal cleanup. Prototype a two-step
read/write handler in both styles before settling developer-facing syntax.

### GUI: named events and explicit state, without requiring closures

```text
record Model { generation: Nat, text: Str }

pure update(model: Model, event: UiEvent): (Model, [UiCommand]) {
  match event {
    Refresh => (Model(model.generation + 1, "Loading"),
                [FetchText(model.generation + 1)]),
    TextReady(generation, text) =>
      if generation == model.generation { (Model(generation, text), []) }
      else { (model, []) },
    Closed => (model, [CancelWindowWork])
  }
}

main(): Void {
  gui_run(Model(0, "Ready"), @update)
}
```

Tuple notation and commands are illustrative too. A host adapter serialises
events on the UI thread, owns the model and executes commands as window-scoped
tasks. A fetch failure becomes a typed UI event (omitted above for brevity), not
an orphaned task failure. Refresh cancels obsolete work; generation checks also
discard late results. Closing the window cancels children and unregisters
callbacks before releasing UI resources. No toolkit calls occur on worker
threads. A later async event handler may be more convenient; this example shows
that useful callback/state integration need not wait for capturing closures.

## Proposed PR sequence and acceptance gates

This sequence is a recommendation, not a scheduled multi-PR commitment. Reassess
after each gate; do not let a large runtime programme automatically crowd out
small compiler-assistance work. Every implementation retains canonical unit,
functional, bootstrap, sanitizer, fault-injection and packaging checks.

| PR / relative effort | Bounded outcome | Required evidence before continuing |
| --- | --- | --- |
| 1. Resumable execution feasibility, M–L | Explicit owned call frames, internal budgeted advance, host-controlled lifetime and termination; fake host only, no public async API | Direct/indirect calls, recursion, iterators and nested execution retain behavior across forced yields; independent sessions; re-entry rejected; exit never kills an embedder; allocation-failure cleanup; unchanged version-8 compiler/stdlib bootstrap artifacts; measured dispatch overhead. |
| 2. Task/lifecycle feasibility, M–L | Minimal scoped tasks, virtual clock and fake pending operations; bounded queues and deterministic completion tests | Parent/child join, cancellation at every wait transition, completion-before-cancel and cancel-before-completion, duplicate/late completion, stale handles, session destruction with pending work, no double release or orphaned tasks. |
| 3. Public syntax/effects decision, M design then L implementation | Compare callback and explicit-await workflows; define async callable typing, activation, scope/resource syntax and bytecode migration | Readable server and GUI examples with handled failure paths; negative checks for pure/async misuse, escaping resources and unobserved work; coordinated compiler/verifier/seed tests. Separate implementation PR if design remains disputed. |
| 4. First native transport, M–L after backend review | Loopback TCP and timers through the chosen lifecycle, Linux/macOS first; bounded echo service before HTTP | Slow client does not stall a fast one; partial writes/reads, EOF, refusal, timeout, overload, close/cancel races and graceful shutdown; sanitizer/fault coverage, no descriptor or task leaks. |
| 5. Useful service and host demonstrations, separately scoped | HTTP pilot plus one host-driven UI/event demonstration | Original HTTP acceptance plus adversarial framing/size cases; UI remains responsive, repeated refresh and close-in-flight work safely; independently review networking security before wider exposure. |

A platform backend decision should compare a small POSIX adapter, a maintained
event library and host-native integration: cancellation/teardown guarantees,
dependency/license cost, Linux/macOS packaging, later Windows support and GUI
loop integration. This report selects none. A browser adapter and real mobile
ports remain separate feasibility work; a fake UI driver is not a shipped GUI.

The first feasibility stage shipped in PR #94. On 2026-09-29 the user separately
selected the second, fake-host task/lifecycle stage after comparison with compiler
assistance and other candidates. [ROADMAP.md](../ROADMAP.md#now-task-and-lifecycle-feasibility)
owns its current scope and evidence. Later stages remain proposals.

## Decisions requested and unresolved questions

Request agreement on the **direction**, not syntax: one owning thread per VM,
resumable tasks, structured lifetime, host-driven pumping and explicit application
suspension. Recommend authorising only PR 1 after that review, with a stop/go
decision before adding a scheduler. If it proves too costly, retain the callback
adapter alternative and reassess, rather than quietly delivering blocking APIs.

Still unresolved: async declaration/call syntax and types; exact scope/resource
escape diagnostics; task-failure representation; host-service registration and
version negotiation; default queue/budget limits informed by measurement; backend
and toolkit choice; isolated-worker transfer format; source-aware task traces.
These have proposed constraints above, not falsely completed implementations.

## External design references

Primary documentation consulted on 2026-09-29. These are precedents and platform
constraints, not dependencies or proof that their complete models fit Panackelty.

- [libuv design](https://docs.libuv.org/en/v1.x/design.html): a thread-owned event
  loop, asynchronous network I/O, and explicit handle/request lifetime. This
  motivates separating backend completion from language task ownership.
- [Python asyncio tasks](https://docs.python.org/3/library/asyncio-task.html):
  task groups wait for children and use cancellation in structured control flow.
  Panackelty's `Result`-based error model needs its own contract; this is not a
  proposal to copy Python exception semantics or depend on its runtime.
- [GTK threading](https://docs.gtk.org/gtk4/section-threading.html): most GTK
  objects belong on the main thread. This is an example of host affinity, not a
  toolkit selection or a universal claim about every GUI framework.
- [Emscripten runtime](https://emscripten.org/docs/porting/emscripten-runtime-environment.html#browser-main-loop):
  browser execution must return control between turns. This supports a host-pump
  requirement, not a claim that a browser port or raw networking is available.
