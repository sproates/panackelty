# Source-coverage feasibility — GI#131, first slice

Run `make source-coverage-experiment` from the repository root with the native
toolchain and Node 24+. This isolated experiment runs in the compiler CI jobs;
it does not add Node to `make check`, instrument production execution, change
bytecode, or produce a suite-wide coverage percentage. All temporary artifacts
are removed by the runner. The JSON output contains exact fixture assertions,
artifact/map/counter sizes and five timing samples per mode.

## Decision

**Reuse the validated sidecar identity contract and prototype opt-in instruction
counters in the real dispatch loop in the next slice. Do not publish a source
line or branch percentage from the current maps.** The experiment proves bounded
expression-start-line reach and source-function entry counts. Existing maps
omit statement stores, loop machinery and implicit returns; enclosing spans can
cover untaken arms. A location map is not an executable-source inventory.

Before a line baseline, add a versioned compiler-produced inventory of eligible
source expressions/statements and source function declarations, including unused
ones. Associate explicit probe points with those identities. Keep generated
instructions distinct. A separate source-branch inventory must describe both
outcomes of conditionals, short-circuit decisions, loop exits and match arms.
VM branch counters alone cannot supply that source denominator.

| Option | Feasibility and cost | Decision |
| --- | --- | --- |
| Deterministic sidecar plus execution counters | Reuses exact local reproduction; no executable changes; retains full source closure; validation costs a compilation | Selected foundation; extend coverage inventory separately and version it |
| Embedded metadata in a new bytecode version | Could make transport a single file; still needs source/compiler identity validation; changes decoder, verifier, tools, seed and bootstrap compatibility | No demonstrated benefit sufficient to justify migration in this slice |
| Append metadata to v9 | Rejected by the existing verifier in the experiment | Not a compatible extension |

No new versioned embedded format is implemented or benchmarked here. Its
migration cost is a design comparison, not a measured runtime result. A new
format does not itself fix missing executable-source or branch identities.

## Independently specified fixture expectations

The native test driver includes the real VM dispatcher, decoder and verifier.
It advances one instruction at a time, recording attempts before execution,
function entries on frame creation and the actual Boolean operand of
`JUMP_FALSE`. The runner's literal expectations are specified from fixture
semantics, not generated from the counters under test.

`basic.panack` calls `choose` twice, once per arm; `main` and the imported erased
generic `identity` each enter once. `unused` and `unused_import` enter zero
times. Eight project expression-start lines are reached and two are eligible
but unexecuted. Each conditional outcome occurs once. The loop fixture calls
`multiply` exactly 30,000 times with 30,000 true tests and one false exit.
Recursive calls have distinct frame entries. Short-circuit and match fixtures
check skipped callees and selected callees; generic calls at `Nat` and `Str`
share one erased body and count twice, without runtime type specialisation.

The source map's function order must not be assumed to equal decoded bytecode
table order. The join uses the unique function name within the exact artifact;
names are hex encoded in the private probe stream. A later collector must bind
numeric IDs to the exact artifact and map manifest, not merge by name alone.

The experiment measures fake async I/O through a real suspension and completion;
the child enters once across the wait. Native source execution, saved artifact
execution and the counter driver retain checked stdout/trap behavior. Ordinary
and mapped compilation produce identical bytecode. The normal bootstrap seed
is not regenerated or instrumented.

## Denominators and count meanings

| Category | Meaning in this experiment | Production requirement |
| --- | --- | --- |
| Instruction attempt | Increment before executing a verified PC; a trapping instruction counts as attempted | Preserve attempted versus successfully completed distinction |
| Expression-start-line reach | OR of hits on direct, non-lowered mapped PCs starting on that line; include zero-hit entries from the full map | Label this narrow metric; never mark every line of an enclosing span hit |
| Function entry | Frame activation count, not count of instructions on its first line | Map declarations explicitly, including empty or wholly unmapped bodies |
| Lowered/unmapped instructions | Retained in raw PC counters, excluded from direct expression reach | Inventory and display exclusions separately; do not silently drop executable source |
| Unused loaded/imported code | Eligible direct entries remain in the denominator with zero hits | Build inventory before execution, including eligible files never loaded by any test |
| Generic function | One erased source body across type arguments | Do not claim per-instantiation coverage |
| Source branch | No percentage supported | Source decision/outcome identities, including short-circuit, loops and match |
| Tests/generated sources/stdlib | No blanket path exclusion in the experiment | Agree explicit scope and versioned reasoned exclusions before a baseline |

Summing instruction hits is not a line execution count: one expression emits
several instructions and expressions can share a line. Binary line reach is
deliberately used here. Do not interpret this fixture's denominator as all
executable lines in a project or as a compiler/library baseline.

## Identity and failure states

The harness recompiles the explicit local entry and compares both artifact and
sidecar exactly before decoding **the locally regenerated map**, using the same
trust model as `panack locate`. Foreign map fields never control file access,
allocation or attribution. Reads are bounded by the expected local length.
Truncated, extra, missing, coherently forged and stale imported-source metadata,
and missing or different artifacts, are tested. The production mapping suite
additionally checks Unicode, relocation, core changes, malformed coordinates and
other adversarial inputs (`make source-mapping-experiment`).

The private text probe stream is consumed only from a fresh local child process
and unique temporary path; it is not a general-purpose untrusted coverage format.
Missing completion footers and duplicate records reject collection. Invalid
bytecode produces no valid report. Timeouts/process failure fail the experiment.
The instruction budget bounds observed steps, and the counter allocation is
limited to one million instruction cells. These are experimental bounds, not a
CPU guarantee for a host call or nested execution; the harness also has a
90-second child-process timeout. Counters cannot overflow within this step cap.

| Collection state | Interpretation |
| --- | --- |
| Completed, no gap, identity valid | Zero eligible hits really mean unexecuted in this bounded run |
| Trap | Retain the attempted prefix and the trap; distinguish it from a successful complete suite |
| Step limit / unsupported host wait | Partial observation; never infer missing hits as zero |
| Nested execution | Outer prefix/counters exist, child execution is missing; whole-run collection is incomplete |
| Missing/invalid metadata or report | Unavailable/failed collection, not zero coverage |

Nested execution is exercised directly and through an indirect wrapper. A nested
compiler also successfully compiles a fixture to identical bytes while remaining
unobserved. `execute()` creates child VMs outside this driver's stepping loop.
TCP/server scheduling, independent task sessions, subprocesses, native host
implementation code and killed processes are not claimed as measured. The next
collector must put hooks in the shared dispatcher and register each execution,
including child/server/task contexts. Raw output needs version, toolchain/source
and artifact identities, run/process/execution IDs, expected counts, terminal
status and gap information. Aggregate unique completed execution records once;
reject mixed identities and duplicates rather than replaying cached test hits.
Expected collection manifests must expose absent processes/files. Checksums are
identity aids, not authentication of source claims.

Maps retain complete source snapshots: existing source disclosure and stable-tree
assumptions in [SOURCE_MAPS.md](../../../docs/SOURCE_MAPS.md) continue to apply.

## Bounded follow-up estimate

1. **Collection and inventory (medium–large, 1–2 PRs):** versioned executable
   expression/function inventory, optional dispatcher counters, disabled-path
   equivalence, bounded allocation/overflow handling, traps and async/child
   execution identities, strict raw-data validation and aggregation tests.
   Source branches require their own inventory; keep them explicitly unfinished
   if a line-first delivery is chosen.
2. **Suite baseline and separate publication (medium, 1 PR):** source manifest for
   compiler, tooling and stdlib; subprocess and bootstrap collection; cache rules;
   complete/partial classification; separate source report through the existing
   coverage host, preserving C reports and release artifacts.
3. **Gap closure/regression policy:** estimate only after the measured baseline.
   Execution is not assertion quality; no universal percentage target is chosen.

Thus collection/publication remains provisionally 2–3 PRs, conditional on the
inventory and nested-execution work. GI#131 stays open. This slice delivers a
supported direction plus specific reasons to revise a naive line-counter design.
There is no new released feature or website claim to publish.

## Performance evidence

The runner alternates bulk/count loop runs and prints five CPU-time samples,
five ordinary/mapped compile wall-time samples, and allocated counter bytes.
The bulk control uses the same driver and allocation but does not increment
counters; single-step overhead is included in the counted mode. This is a
feasibility cost, not a prediction of an in-dispatch production implementation.
Compilation samples include process startup and source loading. Small shared-host
samples are noisy; do not infer a production regression or speedup from them.
Both modes assert identical results; fixed per-artifact allocation does not grow
with run length. The [validation profile](../../VALIDATION_PROFILE.md) records
the measured experiment results; the delivery PR records final canonical and
hosted validation. Counter bytes describe the extra fixed tables, not total VM
RSS. CPU samples exclude artifact loading and report serialization; target wall
time includes the full experiment.
