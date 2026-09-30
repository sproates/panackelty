# Panackelty virtual machine

The Panackelty VM is a stack machine. Every call frame owns a program counter, local
variables, and an operand stack. Its instruction set covers constants, local
access, arithmetic, collection and algebraic-data construction, iteration,
control flow, calls, and returns.

Start with the [VM execution guide](../../docs/VM_GUIDE.md) for instruction
listings and worked programs showing the operand stack, locals, call frames,
and output at each step.

The portable C11 VM is the execution target. Tests check its behavior against
independent fixed expectations. Seed regeneration
uses verified self-hosted stages on this VM.

The VM trusts neither source compilation nor bytecode files. Serialized
artifacts are verified before execution, and safety checks such as bounds
checking and `Nat` underflow remain enforced at runtime.

The frozen version-9 execution semantics and instruction stack effects live in
[`../bytecode/FORMAT.md`](../bytecode/FORMAT.md). The VM converts invalid dynamic
bytecode state into a Panackelty trap so host-language indexing, lookup, type,
and arithmetic exceptions do not cross the runtime boundary. A shared forged
runtime corpus exercises indirect-call validation, `Nat` underflow, zero
division, invalid byte and UTF-8 values, missing map keys, and single- and
multi-value stack underflow against both VM implementations.

The native representation, ownership, allocation, and reclamation rules are
specified in [`VALUE_MODEL.md`](VALUE_MODEL.md).

## Internal resumable execution

`vm.h` exposes an experimental C execution handle, not a stable embedding ABI or
source-language concurrency feature. `vm_execution_create` retains arguments
and borrows the VM, verified program, snapshots and host context until destruction.
One handle owns that VM context until `vm_execution_destroy`; independent VM
contexts can be interleaved on the same thread. Neither context nor values may
be shared concurrently across threads.

`vm_execution_advance(handle, budget)` dispatches at most that many instructions,
including calls and returns. Zero does no work. It returns `VM_YIELDED`,
`VM_COMPLETED`, `VM_TRAPPED` or `VM_EXITED`; terminal states are sticky. A
re-entrant advance returns `VM_BUSY`, a second create fails without changing the
first invocation, and destruction refuses while the handle is running.
`vm_execution_result` borrows the completed result until destruction; retain it
to keep it longer. `vm_execution_exit_status` is meaningful only after `VM_EXITED`.
Destroying a yielded execution releases all frames, locals, iterators and stack
references. Destruction invalidates the handle and allows reuse of its VM.

This mode intercepts `process_exit` as an outcome, rejects both nested-bytecode
services, and rejects other effectful services unless a trusted immediate host
adapter supplies them. The adapter borrows arguments, returns an owned result
(or a static error), and must remain bounded and nonblocking. It is not a
security boundary: native adapter code must not bypass these rules. There are
no pending host requests in this immediate-adapter API. The separate fixed-service
experiment below adds waiting and internal task lifetimes. Native TCP is a
separate explicit opt-in described below; fake task sessions do not enable it.
Source async awaits use the typed service below.
An instruction budget does not bound a long numeric operation, destructor or
native callback in wall-clock time.

The synchronous `execute` adapter runs the same dispatcher to completion with
legacy host behavior, including CLI process exit and synchronous nested bytecode.
It rejects re-entry on an occupied VM without modifying that invocation's error.
The source async slice uses bytecode version 9 and a refreshed compiler seed.
See the [design proposal](../../docs/EXECUTION_CONCURRENCY_DESIGN.md) for later
stages and [measured overhead](../../tests/VALIDATION_PROFILE.md) for this step.

`main.c` is the entry point for the portable C11 seed executable. Its `check` command performs
bounded version-9 decoding and independent semantic verification; `run`
executes verified artifacts with the reference-counted value model, exact
numerics, persistent collections, UTF-8 operations, and stable host ABI. It
accepts and runs the complete compiler and standard-library artifacts and
consumes the same malformed vectors as the self-hosted decoder. Build it with
`make native`. The build defaults to `CFLAGS=-O2`, retaining strict C11 and
warning checks. `CC`, `CPPFLAGS`, `CFLAGS`, `LDFLAGS`, and `LDLIBS` are
configurable. Run `make clean` before changing flags, for example before
`make native CFLAGS="-O0 -g"` for debugging. Optimisation does not change the
bytecode contract or disable runtime verification.

String construction records code-point count and whether every byte is ASCII.
Length is constant time, and ASCII indexing, slicing, and prefix-offset lookup
avoid rescanning the string. Non-ASCII offsets retain UTF-8 traversal. Every
string-producing operation uses the same constructor, including concatenation,
interpolation, slicing, reversal, decoding, and host inputs; bounds and UTF-8
validation remain in place.

Array append uses bounded sharing between an immutable prefix and its next
version, amortizing eligible repeated growth while preserving retained snapshots.
A bounded ownership check sends possible cycles and large object graphs to the
independent-copy path.
The [value model](VALUE_MODEL.md#persistent-array-append-storage) defines ownership,
release order and allocation-failure behavior.

Rational arithmetic uses normalized arbitrary-precision numerator/denominator
pairs. `Unit` has its own runtime tag; `.nat()` and `.dec()` perform exact checked
conversions and trap rather than discard precision.

Opaque paths and time values are implemented in `host_types.c`. Lexical paths
preserve native bytes; durations and instants use exact integer storage. Typed filesystem, process, and sleep operations are implemented separately
in `host_capabilities.c`, including descriptor ownership, concurrent stream
collection, resource limits, and structured host failures. See `VALUE_MODEL.md` and
`../../SPEC.md` for ownership, signatures, error behavior, and clock scope.

## Internal task lifecycle experiment

`tasks.h` / `tasks.c` build a bounded session around the same dispatcher. Each
task owns a VM context and execution. The session borrows a verified program and
argument/environment snapshots; these must outlive it. All session APIs, including
creation/destruction across sessions, run on one thread. This is experimental
internal C infrastructure, with no public ABI or language task syntax.

A host creates roots and children explicitly. Children can be added while a
parent is ready or waiting, never after its body finishes. Successful bodies join
all children before exposing their result. A child trap, exit or independent
cancellation fails its parent and cancels siblings, propagating through ancestors.
Parent cancellation cancels unfinished descendants. Completed children and their
results remain inspectable. Task records retain static errors and exit outcomes
in creation order; cancellation cannot overwrite a completed terminal outcome.
There is no detached work or user finalizer execution.

`vm_execution_create_pending` uses a deliberately narrow fake service: `print`
registers a borrowed input and suspends with `VM_WAITING`; the session retains the
input until acknowledgement or cancellation. Direct and indirect calls share this
path. Completion produces only `Void` or a static error, so arbitrary values cannot
violate the service's return type. `vm_execution_complete_print` changes readiness
without running bytecode inline. Other host effects remain unavailable, with
`process_exit` intercepted and nested bytecode rejected. Ordinary CLI printing
and the immediate host API retain their existing behavior.

Operations carry a session-qualified task id and per-task generation. Duplicate,
late, forged and cross-session completions are rejected. A full FIFO queue rejects
enqueue without consuming the wait; the host can pump and retry. Cancellation
before delivery invalidates even an already queued completion. Cancellation after
delivery can still cancel the unfinished task, but releases no request twice.

Pumping drains queued events, expires deadlines, then dispatches ready tasks in
round-robin order, one instruction per turn, within the supplied total budget.
Zero budget processes events/timeouts only. The host supplies virtual monotonic
ticks; backwards time is rejected. Children inherit the earlier ancestor deadline;
`UINT64_MAX` means no deadline. Deadlines include joining scopes: a queued result
delivered at the deadline does not make an unfinished task immune to cancellation.
The next deadline is exposed for a future host adapter; no OS clock is read.

Limits bound total admitted tasks and queued completions, with one pending request
per task. Task slots/results remain reserved until session destruction; this
prototype is unsuitable for an indefinitely running service. Task/ancestor scans
and joins are linear or quadratic in the configured task bound, and a pump's
instruction budget excludes event delivery/cleanup. Numeric operations, allocation
and reference destruction also have unbounded wall-clock cost. No latency claim,
production default limits, general resource registry or task trace is supplied.

Destroying a session releases frames, results, queued metadata and retained fake
requests. The fake service has no external producer. A real backend must prove
producer quiescence before freeing its session; identity validation does not make
calls through a freed session pointer safe. The experiment proves synchronous
runtime-owned cleanup, not cancellation of native OS operations.

## Finding the implementation

Read `main.c` for the complete load → decode → verify → execute path. Each C file
is a separate translation unit; headers declare shared types and contracts, and
private helpers remain static. `Reader` belongs to `decode.c`; `Frame` and `Local`
belong to `execute.c`. No implementation is included through a header.

| Component | Files | Responsibility |
| --- | --- | --- |
| Program model | `program.h`, `program.c` | Decoded constants, named functions, instructions, cleanup |
| Loader | `decode.h`, `decode.c` | Bounded version-9 decoding and format checks |
| Verifier | `verify.h`, `verify.c` | Canonical order, jumps, call arity and purity |
| Values | `value.h`, `value.c` | Tagged values, constructors, equality, reference counting |
| Numerics | `numeric.h`, `numeric.c`, `bigint.h`, `bigint.c` | Exact arithmetic and checked conversions |
| Text support | `buffer.*`, `render.*`, `utf8.*` | Byte buffers, value formatting, UTF-8 traversal |
| Execution | `vm.h`, `execute.c` | Invocation context, frames, stack, locals, opcode dispatch |
| Native TCP | `tcp.h`, `tcp.c` | Owned nonblocking request/response, bounded buffers, monotonic timeout |
| Task experiment | `tasks.h`, `tasks.c` | Scoped task ownership, fake waits, virtual deadlines, bounded host pumping |
| Builtins | `builtins.h`, `builtins.c`, `builtins_internal.h` | Shared arity/purity/handler registry |
| Builtin domains | `builtins_text.c`, `builtins_collections.c`, `builtins_numeric.c`, `builtins_vm.c` | Operations and nested bytecode execution |
| Host boundary | `host.*`, `host_types.*`, `host_capabilities.*` | Invocation snapshots, legacy I/O, paths, clocks and typed services |

`program.h` names every opcode with an explicit version-9 wire value. The decoder
and executor share those names. Numeric operations report through an error pointer
and do not depend on VM state. The verifier and executor use one builtin registry,
so effects, arities and implementation routing are kept together.

## Ownership and editing conventions

Constructors normally return one owned reference. Collection constructors retain
children; stack pop transfers a reference; local lookup borrows one. Header
comments identify exceptions such as the moving decimal constructor and consuming
host-result wrapper. See `VALUE_MODEL.md` for the full contract.

Use four spaces, braces for control statements, descriptive names, and blank lines
between validation, work, and cleanup. Comments should explain ownership or an
invariant rather than narrate obvious assignments. `src/vm/.clang-format` records
these conventions; run `clang-format -i src/vm/*.c src/vm/*.h` when editing C.
The formatter is a development convenience, not a build dependency.

`make native` compiles components separately and tracks header dependencies.
`make check-vm` includes direct C module contracts, independent header compilation,
registry parity with fixed independent signatures, and the existing native and public CLI corpus.
`make native-unit` runs only the C contracts. `make native-sanitize` builds isolated
AddressSanitizer/UndefinedBehaviorSanitizer binaries, runs those contracts, and
runs the native loader and execution suites against the instrumented runner.
It requires a compiler/runtime supporting those sanitizers and does not replace
`panack-vm`. Leak detection depends on host sanitizer support; reference-count
assertions check the tested ownership paths on every host.

`make native-fault` sweeps allocation failures and selected host syscall failures
in a separate test-only build. The normal VM suite additionally sweeps richer
compiled programs, including nested execution and traps with live caller values.
`make native-coverage` requires Clang and matching `llvm-cov`/`llvm-profdata`
(on macOS, Xcode command-line tools work) and writes `build/coverage/summary.txt`
and `build/coverage/html/`. CI runs sanitizers and uploads the coverage report.
These targets include seeded arithmetic properties, persistent-value lifetimes,
and deterministic mutation of every operand and constant form. They supplement,
but do not prove, memory safety; exhaustive host failures and coverage-guided
fuzzing remain follow-up work.


Direct VM execution and loader contracts run in `tests/runner/vm_unit.panack`
against the portable corpus in `tests/fixtures/vm_contracts`. Its 177 assertions
include native module, bigint and allocation-failure wrappers; header isolation
runs in `tests/native_headers.sh`. `make native-vm-contracts` runs this group,
and `make unit`, `make check-vm`, sanitizer and coverage gates include it.
The VM corpus checks 61 fixed execution contracts, including 21
per-artifact C return-kind assertions. Fixed independent arithmetic expectations
and builtin signatures run in `make native-oracle-contracts`.

Direct host, runtime and standard-library assertions run in
`tests/runner/host_runtime_unit.panack` with reviewed source and malformed
bytecode fixtures in `tests/fixtures/host_runtime`. The probe asserts native
process, file, path, environment and timing contracts and exact testing-library
reports. Direct C host checks and forced failures run under instrumentation.
The [migration inventory](../../tests/fixtures/host_runtime/README.md) maps all 37
former methods: 31 migrated to direct native evidence and the final six replaced
by fixed oracle fixtures and native/bootstrap cross-checks. Functional source and bytecode cases still verify
public behaviour on both supported platforms.

## Typed async fake read

The v9 async flag and AWAIT_CALL/AWAIT_VALUE distinguish activation from ordinary
calls. Verification and dispatch share the effect-edge rule, and indirect calls
check it against the resolved target. Async code cannot enter ordinary impure
helpers or blocking builtins. Async main's final value is checked as Unit.

`vm_execution_create_async` registers the fixed pending Bool read input;
`vm_execution_complete_read` accepts only Ok(Bytes) or Error(Str). It borrows the
completion and retains an accepted result. Print acknowledgements cannot complete
a read, or vice versa. The CLI supplies deterministic fake completions outside
advance. Task sessions retain queued values, discard stale/cancelled delivery,
and release values even when destroyed with completions queued. Cancellation and
operation identities retain the task experiment's bounds and ownership rules.
These APIs remain internal and single-threaded; no external producer exists.

## Bounded native TCP

`tcp.h`/`tcp.c` own the connection state for `tcp_exchange`; limits and source
semantics are in [SPEC.md](../../SPEC.md#native-tcp-exchange-development-toolchain).
The implementation uses POSIX [nonblocking connect](https://pubs.opengroup.org/onlinepubs/009695399/functions/connect.html)
and [poll readiness](https://pubs.opengroup.org/onlinepubs/9799919799/functions/poll.html),
with per-socket/per-send SIGPIPE suppression on macOS/Linux. No external runtime
library, background thread or callback queue is required.

`vm_execution_enable_tcp` grants an embedded execution the native service while
it is yielded. Existing executions deny it by default. On `VM_WAITING`, call
`vm_execution_poll_tcp(handle, max_wait_ms)` on the owning thread; zero performs
bounded nonblocking work, allowing another execution to progress. A poll performs
at most one 16 KiB send and receive, checks the total monotonic deadline, and may
install a typed completion; it never executes source instructions inline. Call
`vm_execution_advance` to resume. A false poll result means no TCP operation is
waiting or the execution is busy. Read/print completion APIs cannot inject a
result into a TCP wait. `vm_execution_destroy` cancels the operation and closes
its descriptor before invalidating the execution, including before first poll.
The synchronous CLI drives this same state machine, waiting only in its adapter.

The host owns the number of simultaneously enabled executions; the per-operation
memory limit does not impose a process-wide connection limit. This internal API
is not a security sandbox or a stable embedding ABI. Native tests interleave a
fast and stalled connection, exercise fragmented binary completion and repeat
cancel/destruction; the source harness runs against a separate loopback peer.
