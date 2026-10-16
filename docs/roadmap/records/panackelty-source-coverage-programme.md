[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Panackelty source coverage programme

<a id="rm-46"></a>

**RM#46: Panackelty source coverage** · [GI#131: Panackelty source coverage](https://github.com/sproates/panackelty/issues/131).

Work record: [.panack source coverage](https://github.com/sproates/panackelty/issues/131).
**Programme complete — all six outcomes accepted on 2026-10-06.** GI#131 is the
umbrella work record. The feasibility work was delivered to `next` in
[PR#299: Source coverage feasibility](https://github.com/sproates/panackelty/pull/299),
merged as `9fefbd6`. SC2 inventory and SC3 single-execution collection are now
delivered, as are complete aggregation, verified baseline publication and the agreed first gap-closure batch. Promotion to
`main` requires separate approval.

### Source coverage programme baseline

**Accepted overall completion after SC6 live verification: 100%.** Baseline SC-1, established 2026-10-05,
weights independently accepted outcomes by relative implementation, testing,
integration and acceptance effort. The earlier four PR boundaries were delivery
estimates, not a weighted programme; this is the first percentage baseline, not
a reduction of previously earned progress. The parent RM#46/GI#131 is not counted
again alongside its children.

| Work item | State | Share of programme | Item complete | Earned contribution |
| --- | --- | ---: | ---: | ---: |
| [SC1 / RM#136: Feasibility and design](#rm-136) | Done on `next`, PR#299 | 15% | 100% | 15 percentage points |
| [SC2 / RM#137: Executable-source inventory](#rm-137) | Done on `next`; [inventory contract](../../reference/SOURCE_INVENTORY.md) | 20% | 100% | 20 percentage points |
| [SC3 / RM#138: Runtime coverage collection](#rm-138) | Done on `next`; [runtime contract](../../reference/SOURCE_COVERAGE.md) | 20% | 100% | 20 percentage points |
| [SC4 / RM#139: Complete run aggregation](#rm-139) | Delivered by SC4 integration; [session contract](../../reference/SOURCE_COVERAGE.md) | 20% | 100% | 20 percentage points |
| [SC5 / RM#140: Suite baseline and publication](#rm-140) | Done; [baseline contract](../../history/SOURCE_COVERAGE_BASELINE.md), [verified live report](https://sproates.github.io/panackelty-coverage/source/html/index.html) | 15% | 100% | 15 percentage points |
| [SC6 / RM#141: Initial gap closure and regression policy](#rm-141) | Done; PR#307 integrated, clean report reproduced and verified live | 10% | 100% | 10 percentage points |
| **Total** | | **100%** | | **100 percentage points = 100% overall** |

SC5 added 15 percentage points to the prior 75%; SC6 adds the final 10 points
to 90%. SC-1 weights remain unchanged.

These are provisional scope estimates, not time estimates or equal shares per
PR. Inventory, collection and aggregation carry most weight because they require
compiler/VM implementation and substantial correctness and failure testing.
SC6 assumes one bounded, risk-ranked first batch of assertion-relevant gaps, not
closing every uncovered line forever. Its exact batch is selected from the
baseline before implementation; unexpected scope requires an explicit baseline
revision rather than silently changing weights or claiming completion.

Every programme delivery and status report must include **all six work items**,
their programme shares, evidenced item completion and earned contributions.
Contribution = programme share × item completion / 100. Retain SC-1 weights
between reports; explain scope/weight changes and their effect on the previous
total. Planning alone earns no implementation credit, and pending/deferred work
remains in the denominator. Partial credit requires explicit accepted sub-outcomes;
activity, elapsed time and PR counts do not establish completion. These programme
percentages are unrelated to the source-coverage percentages eventually measured.

<a id="rm-136"></a>

**SC1 / RM#136: Feasibility and design — 15%; Done.** Accept the reproducible
experiment, identity and denominator rules, comparison with an embedded format,
limitations, cost evidence and bounded follow-up design. PR#299 supplies 68
fixture checks, passing canonical validation and hosted Linux/macOS checks.
This accepts the investigation, not a production coverage collector.

<a id="rm-137"></a>

**SC2 / RM#137: Executable-source inventory — 20%; Done.** Define a versioned inventory
of eligible expressions/statements, functions and source decisions/outcomes,
including unused declarations/files. Distinguish generated/lowered instructions,
imports, erased generics, short-circuit decisions, loop exits and match arms.
Accept against literal inventory fixtures, explicit exclusions, source/compiler/
artifact identity, malformed/stale-input rejection and unchanged ordinary
bytecode/bootstrap behavior. The [inventory contract](../../reference/SOURCE_INVENTORY.md)
and literal/public-command acceptance tests deliver this outcome. Explicit extra
roots include libraries without `main`; ordinary compilation still requires it.
The seed is refreshed through the existing fixed-point procedure, without VM
instrumentation or executable-format changes. SC3 builds on this inventory;
SC5 defines the measured suite scope separately. Inventory generation alone does not collect hits.

<a id="rm-138"></a>

**SC3 / RM#138: Runtime coverage collection — 20%; Done.** Implement opt-in counters in
the shared dispatcher and a versioned raw format bound to SC2 identities. Prove
exact line/function/source-branch observations on small fixtures, attempted
versus completed behavior, traps, allocation limits, overflow handling and
enabled/disabled semantic equivalence. Measure runtime and memory overhead.
The [runtime contract](../../reference/SOURCE_COVERAGE.md) and production command fixtures
deliver source branches as well as expression/line/function observations.
Overflow, traps, bounded allocation, suspension and identity failures are tested.
Nested/child contexts deliberately mark a collection gap; complete manifests and
aggregation remain SC4. No suite percentage is established by SC3.

<a id="rm-139"></a>

**SC4 / RM#139: Complete run aggregation — 20%; delivered on integration.** Register async suspension,
nested VMs/compiler runs, task/server child contexts and subprocess executions.
Aggregate each run exactly once under a manifest of expected source/artifact and
execution identities. Reject mixed identities, duplicates, truncated/malformed
records and lost/killed collectors; preserve zero, excluded, unavailable and
partial states. Validate cache behavior so reused test transcripts cannot invent
new execution hits. Accept exact multi-execution fixtures and explicit supported
scope, with missing eligible collection preventing a complete report.

`coverage-session` registers exact artifact inventories and every admitted nested,
task, server or subprocess execution. `coverage-aggregate` checks fresh session
identities, exclusive claims, parent child counts, the shared admission/byte
budget, exact directory membership and terminal records before adding counters.
Missing, killed, cancelled, unregistered or nonparticipating collection blocks a
complete report; cached fixture-runner transcripts are rejected. Source states
and per-artifact roots remain explicit, with no suite percentage. The runtime
contract documents native-only scope and limits. Exact CLI/native/failure tests,
compiler fixed points and the real nested compiler acceptance command are
recorded in `tests/COVERAGE.md` and `tests/VALIDATION_PROFILE.md`.


<a id="rm-140"></a>

**SC5 / RM#140: Suite baseline and publication — 15%; Done.** Collect compiler,
bytecode-tooling and standard-library execution across the agreed test and
bootstrap paths, retaining eligible unexecuted files and reviewed exclusions.
Publish source line/function/branch results separately from native C through
the coverage host. Accept reproducible identities, scope, freshness, missing-data
handling and verified live output; preserve release/seed artifacts. Publication
and changes in another repository retain their own approval requirements.

Accepted 2026-10-05: core [PR#305](https://github.com/sproates/panackelty/pull/305)
merged to `next` as `7b35fbab2beb7066d6d045f14c8c9063e5187b8d`;
[next Check run 37373904441](https://github.com/sproates/panackelty/actions/runs/37373904441)
passed all 22 jobs on attempt 2. The separately approved publisher
[PR#3](https://github.com/sproates/panackelty-coverage/pull/3) merged as
`37c95cf29f26fe53c24a9bd436913d36f27ea786`;
[production run 37378463006](https://github.com/sproates/panackelty-coverage/actions/runs/37378463006)
passed preparation, deployment and byte verification of every live file.
The [HTML report](https://sproates.github.io/panackelty-coverage/source/html/index.html)
and [JSON summary](https://sproates.github.io/panackelty-coverage/source/summary.json)
cover all 38 tracked production files across 30 fresh executions: lines
5,804/6,732 (86.22%), functions 469/522 (89.85%), source outcomes
6,027/7,574 (79.57%), no unavailable measurements. This is the explicit initial
corpus, not every canonical test. Native C coverage remains independently pinned
to `main`; core `main` was not promoted. Bootstrap output is unchanged.

<a id="rm-141"></a>

**SC6 / RM#141: Initial gap closure and regression policy — 10%; Done.** Use SC5 evidence
to agree and close a bounded first batch of high-risk behavioral gaps with
assertions that detect wrong results, not merely execute lines. Define and test
per-component regression handling, exclusion review and failure reporting.
Accept the agreed batch plus the documented policy; no arbitrary universal
coverage target or promise to eliminate every uncovered path is introduced.

Owner-approved batch, 2026-10-06: directly assert UTF-8 boundary/rejection behavior,
both option/result helper arms, and exact/negative/zero duration ratios; add the
existing `host_types` functional fixture to the measured corpus. Tests uncovered
and fix signed-zero guards in `duration_ratio` and `duration_divide`. The
[SC6 policy contract](../../history/SOURCE_COVERAGE_BASELINE.md#report-integrity-and-release-gate)
requires complete current evidence, exact per-component floors, reviewed
denominator/manifest/exclusion changes and protected source outcomes. There is
no automatic baseline lowering. Accepted 2026-10-06: PR#307 merged to `next`
as `b382bd84ce65b64bdcd859f4decfd600d75663c4`; final-head and fresh next Check
passed all 22 jobs. [Next run 37384119034](https://github.com/sproates/panackelty/actions/runs/37384119034)
reproduced 31 fresh executions, zero unavailable measurements and all 68 protected
items. [Publisher run 37387725315](https://github.com/sproates/panackelty-coverage/actions/runs/37387725315)
passed preparation, deployment and byte verification of every live report file.
The live summary identifies the SC6 commit: lines 5,832/6,732 (86.63%), functions
480/522 (91.95%), source outcomes 6,064/7,574 (80.06%). Gains over SC5 are 28 lines,
11 functions and 37 outcomes with unchanged denominators/exclusions. PR#308 keeps
full collection on main promotion PRs and explicit next refreshes. Core main is
untouched. The accepted outcome is the bounded first batch, not all source gaps.

All six scoped outcomes meet acceptance; GI#131 is complete. Further assertion
quality and corpus expansion remain separate work. PR packaging can change without changing
programme weights: the earlier 2–3 collection/publication PR estimate is
provisional and must be reassessed as source-branch and aggregation scope is
implemented. The detailed inventory/collection/aggregation split makes ownership
and earned progress explicit without claiming new implementation.

The [reproducible experiment and design decision](../../../tests/experiments/source_coverage/README.md)
establish exact small-fixture instruction attempts, function entries and direct
expression-start-line reach, including unused/imported code. Tests distinguish
lowering, erased generics, short-circuit/match behavior, recursion, async
suspension, traps, stale/forged identity and incomplete collection. Nested
bytecode and nested compiler execution preserve behavior but explicitly report
missing child observation. Select sidecar identity plus future shared-dispatch
counters; add a versioned executable-source inventory before any source-line
percentage. Source-branch identities remain separate unfinished work. A bytecode
format migration is not justified by this slice. Collection and publication are
provisionally 2–3 further PRs; gap closure remains unestimated.

**Urgency: High — user decision, 2026-10-04.** As the language grows, missing
measured `.panack` compiler, tooling and library coverage makes source line,
function and branch reach harder to assess; existing tests and C coverage do
not establish it. Bring the bounded source-mapping and measurement feasibility
slice forward for consideration at the next work review. That urgency decision
alone started no implementation; the later explicit first-slice selection above
supersedes its unscheduled state without changing other programme weights.

The public LLVM report measures the native C VM only. Existing `.panack` tests
exercise the compiler and libraries, but there is no measured source-line or
branch baseline for those files. Publishing C coverage did not close this gap.

The delivered U2 sidecars carry validated instruction/source mappings; executable
v9 bytecode remains unchanged. Coverage requires additional compiler/VM design,
not just an HTML export. Reuse the
[delivered mapping foundation](current-status-and-grooming-2026-10-01.md#source-to-bytecode-mapping-foundation-u1-and-u2-done)
where its validated contract fits; coverage denominators, source branches and
counter correctness remain separate acceptance requirements.
Compare deterministic sidecar metadata with a versioned bytecode extension;
do not assume a format change or a particular instrumentation scheme in advance.

Original delivery outline (historical PR grouping; SC-1 above now owns programme
work items, weights and progress):

1. **Coverage design and feasibility.** Inventory source-position fidelity and
   lowering; define executable lines, functions and source branches, including
   short-circuit expressions, match arms, loops, generated instructions, imports
   and erased generics. Prove a small source-to-execution mapping. Decide metadata
   identity, compatibility, validation and bootstrap implications; document
   overhead and implementation scope before committing to an estimate.
2. **Collection and correctness.** Implement opt-in measurement and a versioned
   raw format, with exact expected results on small known programs. Preserve
   ordinary outputs and semantics; test malformed/mismatched maps, disabled
   instrumentation and traps. Aggregate across subprocesses and nested VM runs
   without collisions, lost counts or double-counting cached test observations.
3. **Suite integration and published baseline.** Measure the `.panack` compiler
   while it compiles programs, bytecode tooling and standard-library execution,
   including relevant compiler/functional/bootstrap paths. Account for unexecuted
   eligible files, test infrastructure, generated code and intentional exclusions.
   Publish separate source reports through the independent coverage host,
   with commit/source identity, suite scope, denominator and freshness visible.
4. **Risk-ranked gap closure and regression policy.** Turn uncovered behaviors
   into bounded test PRs; validate the assertions, not just execution counts.
   Select per-component change/regression policy after a credible baseline exists,
   with reviewed exclusions and no invented universal percentage requirement.

- [x] Complete the design/feasibility decision and bound the initial scope (PR#299, SC1)
- [ ] Establish independently checked source mapping and counter correctness
- [ ] Record source-line/function coverage for the agreed `.panack` scope
- [ ] Record source-branch coverage, or explicitly track it as unfinished if a
      line-first increment is chosen; never substitute VM branch counts silently
- [ ] Include subprocess/nested compiler execution and all eligible unexecuted
      files in the aggregate, with reproducible source identities and exclusions
- [ ] Publish separate native C and `.panack` baselines; never blend percentages
- [ ] Demonstrate instrumented/uninstrumented semantic equivalence and record
      runtime, memory and validation cost without weakening existing checks
- [ ] Establish a documented regression policy and close the first verified gaps

An instrumented compiler must not accidentally change release artifacts or the
ordinary bootstrap fixed-point contract. Decide source-map privacy/path handling,
artifact retention and source-snapshot validation as part of the measurement
format. A report must distinguish zero hits, excluded code, missing data and
failed collection; partial collection must not appear as a complete green report.
