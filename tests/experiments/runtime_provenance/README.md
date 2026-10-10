# Bounded runtime provenance experiment (U0, #172 / #180)

Run `make runtime-provenance-experiment` with the native toolchain and Node 24+.
The observer reports emitted module-qualified identities for non-entry functions;
source-map `locate` queries continue to use source-level function names.
The target is also run by Linux/macOS compiler CI. `make check` includes the
retention unit checks; the separate experiment passes 65 assertions over complete programs,
checks derivations and source attribution, and prints five CPU samples per mode.

## Question and observed answer

For `multiply(6, 7)`, why is the returned value 42? The observer follows the
actual VM's main return to the multiply return, the multiplication event, its
operand loads, and the literal inputs connected through the actual call. The
multiplication's function-local PC is independently checked using production
`compile --source-map` and `locate`: its source expression is `a * b`. Mapped and
ordinary artifacts are identical. This is a value-origin graph, not a claim
that the immediately preceding instruction caused the result. No alternate
interpreter, replay, reversible execution or runtime feature is introduced.

`probe.c` includes the existing dispatcher just like the source-mapping trap
probe and uses `vm_execution_advance(..., 1)`. It observes metadata before/after
real instruction dispatch. It never computes the application's arithmetic or
retains a `Value *`. The sole VM implementation still owns execution, safety,
values and effects. The unmodified production compiler, seed and bytecode
format remain in use. Function/PC references belong only to this decoded
artifact during this process; exported traces have no cross-run identity or
trusted persistence format. Only the harness's exact mapped artifact/source
pair is passed to the validated production attribution command.

## Identity, limits and unavailable evidence

Events have monotonic per-execution IDs; neither memory addresses, frame depth,
PCs nor buffer indexes identify occurrences. Repeated calls at one PC receive
distinct call IDs; recursive activations do too. Operand edges name observed
origins. Stores replace local origins; loads follow the latest store. Return
edges connect the callee value to the caller's stack. Calls retain up to eight
argument edges and call context. No value-based matching is used.

Conditional events record the consumed Bool when payload capture is enabled.
Each frame records its latest conditional event, linked to earlier context and
inherited across a call. This is **chronological execution context**, not a
proven control-dependence graph: after joins it can include conditions irrelevant
to the value. The loop fixture observes true/true/true/false and distinct call
contexts for the three products. It makes no minimal causal-influence claim.

Two equal-capacity retention policies are executable:

| Policy | Useful evidence | Failure mode |
| --- | --- | --- |
| Last-N ring | Recent result and immediate derivation | Earlier operand, call and context edges say `evicted` |
| First-N prefix | Startup/input history | Later IDs and final root say `discarded` |

IDs remain stable when ring slots are overwritten. No missing edge reconnects to
a reused slot. Zero/unknown IDs say `missing`; unsupported observation has an
explicit frontier reason and no claimed final root. There is no fallback that
fills gaps by guessing. A retained root does **not** mean complete ancestry.

Capacity is 1–65,536 events. All shadow state is fixed: 32 frames, 128 operand
slots and 128 local slots per frame. Capacity checks are conservative (observation
stops before operating on a full stack/local array). More than eight call
arguments or a frame/local/stack cap disables further observation, while the VM
continues. Collections, iteration machinery, indirect calls, builtins, async
and host boundaries likewise stop observation with `unsupported-opcode`.
Traps/waits cannot produce a successful root. Verified bytecode that falls off
a function is delegated to the VM bounds check before inspecting the PC, including
when observation was already disabled; both paths have forged-artifact regressions. The embedding API intentionally
has no host adapter here: a print probe reaches the unsupported boundary and
traps, whereas the public CLI print program succeeds. This does not implement
host resumption or replay effects. Collection element identity and async task
identity are specifically unresolved; future support needs explicit task and
transformation edges, not shared object pointers.

The harness limits execution to ten million instructions and reports a
`step-limit` gap for an unfinished observed run. At most one event is allocated
per step, so 64-bit event IDs cannot overflow. This instruction bound does not
bound a single exact-arithmetic operation, allocation or host operation in time.

## Sensitive-value policy

Payload capture is off by default. Opt-in `values` stores only Bool and Nat
values that fit `size_t`, with a fixed-width payload. Larger numbers, strings,
collections and other values say `unsupported-value`; they are never rendered,
truncated, copied, hashed or retained by reference. With capture disabled,
every payload and final result is `redacted` and its numeric field is zero.
Even metadata reveals function names, occurrence counts and branch structure;
redaction is not a confidentiality guarantee. The experiment uses synthetic
inputs only, keeps history in process memory, and the runner removes temporary
source/artifact/map files. Manual `dump` writes metadata and opted-in values to
stdout; do not run it on secrets. Production would need explicit session consent,
retention/export controls and an honest warning that opted-in numeric inputs
can themselves be sensitive. Production source maps separately contain sources.

## Cost and decision

On Linux x86_64, GCC 13.3.0 `-O2`, a 30,000-iteration scalar/call loop returns
1,260,000 and creates 570,010 events. Five process-CPU samples (seconds):

| Mode | Samples | Median |
| --- | --- | --- |
| Bulk real VM | .024967, .025573, .024385, .024764, .024647 | .024764 |
| Single step, no observer | .032022, .027841, .027676, .026186, .026375 | .027676 |
| First 1,024 events | .056499, .067985, .060188, .055382, .056704 | .056704 |
| Last 1,024 events | .056818, .054178, .061033, .054612, .054374 | .054612 |

Single stepping is about 1.12× bulk; ring observation is about 1.97× single
stepping and 2.21× bulk in this small workload. Prefix avoids later buffer
writes but still tracks origins, explaining its similar cost. These are noisy
short synthetic runs, not application-wide budgets or evidence of zero overhead.
CPU timing excludes decoding, allocation of the observer, output, compilation,
and source-map lookup. No extra retention exists in ordinary production runs.

At 1,024 slots each policy allocates **238,648 bytes**: 99,384 bytes for the
whole Observer (including all shadow stacks/locals/names/frame context and
bookkeeping) plus 139,264 bytes for 1,024 136-byte Events. This requested-memory
bound is independent of event count and payload capture. At 65,536 slots it is
9,012,280 bytes. Allocator overhead, transient C call-stack locals and existing
VM/decoded Program storage are outside that retained-provenance total. Events
borrow function pointers and name pointers from the existing Program, whose
lifetime covers the observer; they do not allocate strings or pin runtime values.
There are no hidden value-rendering allocations. Stdout buffering and the
external compiler/locate process are not provenance retention.

First-run process peak RSS was 4,431,872 bytes in all four modes, measured with
`getrusage` (platform units normalized). RSS includes code, runtime, decoded
bytecode, existing values and allocator effects and is too coarse/noisy here to
isolate the small retention allocation. Use the explicit allocation bound for
retention; the equal RSS samples do not establish zero memory overhead.

**Go for a separately scoped U7 scalar production slice; no-go for shipping this
observer or promising general explanations.** Prefer a bounded recent-event
ring for a recent result, with visible evicted ancestry. A prefix is useful for
startup diagnosis but fails the final-result question. A third option, pinning
a selected value's ancestry, could preserve a chosen explanation but cannot
bound arbitrary ancestry without its own eviction/frontier policy; it is not
implemented or measured here. Full instruction history and replay remain out
of scope and would add costs/semantic obligations this question does not need.

First production slice estimate: **large, 2–3 focused PRs**, separately authorised.
(1) A disabled-by-default VM event hook and bounded origin storage for synchronous
Nat arithmetic/direct calls, explicit caps and privacy controls, with off/on
semantic and performance parity evidence. (2) One explicit selected-result query
that traverses the retained graph and binds it to validated U2 source identity,
rendering every gap. (3, if needed) adversarial/cross-module/generic/recursive
acceptance, packaging/docs and performance hardening. Agree representative
workloads and limits before production acceptance; target zero observer
allocation when off, a configurable hard retention cap (initial proposal 1 MiB),
and measure on-mode slowdown before approving its budget. This experiment's
roughly 2× cost is not an accepted production budget. Sampling/selective hooks
need measurement and cannot silently imply complete derivations.

This completes the bounded U0 retention experiment on merge, not U7, issue #172,
or programme #180. Imported/generic provenance, collection transformations,
precise branch dependency, host/async boundaries, real-program usefulness and
production source/session/export integrity remain. Positive non-impact and other
U0 investigations remain separate. No new release, CLI feature or website claim
is delivered, so the website has no impact from this test-only investigation.
