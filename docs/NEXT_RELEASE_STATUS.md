# Next release status

Updated: 27 October 2026. Release: **v0.1.0-alpha.15**. Integration branch: **next**.
Latest published release: **v0.1.0-alpha.14**. Publication date: not agreed.

**Packages are accepted on next. The release still has substantial foundation
work remaining and is not ready for publication.**

This is the current release-status source of truth. Read this file for a quick
status answer; follow evidence links when investigating a particular item.
[Agreed scope](roadmap/records/next-release--agreed-scope-2026-10-06.md)
and linked issues retain the detailed requirements and decisions.

## Delivery

| Item | Current state and remaining acceptance | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| [GI#200 Packages](https://github.com/sproates/panackelty/issues/200) | Accepted on next: local folders/archives, HTTPS archives, pinned Git, diagnostics and installed acceptance. [P04c evidence](design/PACKAGE_FINAL_ACCEPTANCE.md), PR#595. Not yet released. | 30/90 | 100% | 30/90 |
| [GI#180 Compiler understanding](https://github.com/sproates/panackelty/issues/180) | Completion scope agreed: complete checker explanations and truthful instruction/source provenance, two checked inventories, five slices and realistic acceptance. PR#619 is merged into main; implementation has not started. | 20/90 | Unassessed | Unknown |
| [GI#435 Validated domain types](https://github.com/sproates/panackelty/issues/435) | Six-constraint semantics agreed: four string constraints, bounded arrays/maps/sets and ordered record-field relationships. Explicit guard binding, validate call/result, failure order and multiline/parenthesised conjunctions agreed. Final consolidated contract approved and merged into main in PR#616: constraint APIs, error schema/rendering, Unicode pin, examples and delivery slices. Existing numeric alternatives are preserved; implementation has not started. | 20/90 | 0% | 0 |
| [GI#409 Compiler/stdlib boundary](https://github.com/sproates/panackelty/issues/409) | Assessment complete on next (PR#635): 42-file closure verified, five change cases evidenced, recommendation to retain source organisation and assess narrower build dependencies in GI#106. [Evidence](COMPILER_STDLIB_BOUNDARY_ASSESSMENT.md). No refactor. | 5/90 | 100% | 5/90 |
| [RM#154 Faster coverage](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-154) | Scope agreed and merged into main in PR#624: four measured workloads, target approval before cache implementation, validated persistent plan reuse and fresh executions. Slice A [baseline profiling](COVERAGE_PERFORMANCE_BASELINE.md) started on 27 October 2026. Exporter/fixture compatibility repaired in PR#640; focused reader and parity checks pass. The first collection then exceeded the 16 MiB inventory limit; the owner approved a bounded 64 MiB inventory prerequisite on 27 October; PR#642 passes full local validation; the capacity and stale scope/exclusion repairs enabled all four complete workloads (939–958 seconds each, 124 fresh sessions). The owner agreed at least 75% faster repeat/reporting runs and at most 10% cold/dependency regression on 27 October; slice B safe plan reuse is in progress. | 5/90 | 0% | 0 |
| [RM#155 Authoring friction](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-155) | Partial. Inventory preflight and contributor/interpolation guidance delivered (PR#545 and PR#565). Remaining scope agreed: unsupported-interpolation diagnostics, tested API examples and public-signature visibility guidance. Roadmap PR#626 is merged into main; no additional implementation started. | 5/90 | Unquantified | Unknown |
| [GI#106 Validation speed](https://github.com/sproates/panackelty/issues/106) | Shared compiler-test preparation merged into next (PR#637). [Replay evidence](VALIDATION_SPEED_ACCEPTANCE.md): clean full 16.1% faster, compiler edit 20.05% faster, hosted initial compiler 24.9%/36.8% faster. All guards passed, including four workers. Owner accepted the alpha.15 milestone on 27 October 2026. Wider issue remains open. | 5/90 | 100% | 5/90 |
| **Total** | **40/90 known contribution (about 44%); overall completion unquantified.** | **100%** | **Unknown** | **40/90 known** |

Weights retain the original provisional relative sizes, excluding the five points
for GI#170: Inference assessment and the five for RM#157: Performance review,
now allocated to alpha.16. The remaining denominator is 90; the Weight column
sums to 100%. No remaining task was re-estimated. Known contribution is 40/90
(about 44%), up from 35/90 following owner acceptance of GI#106's five-point
alpha.15 milestone. Scope weights and the denominator are unchanged. Earlier denominator reductions reflected deferrals only. Unknown contributions are not zero; overall completion remains
unquantified. These are provisional weights, not owner-approved effort estimates.

## Deferred to the following release

Owner decision, 26 October: [GI#170 inference/diagnostics assessment](https://github.com/sproates/panackelty/issues/170)
moves explicitly from alpha.15 to **alpha.16**, retaining its assessment scope.
It is not unscheduled. [PR#611](https://github.com/sproates/panackelty/pull/611)
records both release allocations. GI#134 explanations of existing inference
remain in alpha.15. The later RM#157 decision is recorded below. Future deferrals go to the following
release unless the owner explicitly chooses a different destination; update both
release allocations, the issue and this summary together.

Owner decision, 26 October: **RM#157: Performance review** is allocated explicitly
to **alpha.16**, retaining its holistic review and bounded-improvement remit.
[PR#629: Performance review allocation](https://github.com/sproates/panackelty/pull/629)
records the roadmap change and is merged into main after owner approval. Detailed workloads, targets and
selected improvements still need agreement; no review or implementation has
started. GI#106: Validation speed and RM#154: Faster coverage remain in alpha.15.
Website release descriptions make no alpha.15 performance-review commitment and
remain accurate without an edit.

## GI#106: agreed alpha.15 boundary

[Roadmap scope PR#603](https://github.com/sproates/panackelty/pull/603) records the
owner-agreed boundary and is merged into main. GI#106 remains
open. This planning update earns no additional completion credit.

- Baseline five workloads: clean full validation, unchanged repeat, a small
  compiler edit with focused checks, a small package-tool edit with focused checks,
  and a documentation-only edit. Separate compilation, fixture setup, assertions
  and CI queueing; use comparable local and hosted before/after measurements.
- Bring one consolidated proposal for at most three improvements and numerical
  targets after the baseline. Candidates: remove duplicate preparation, reuse
  correctly invalidated build artifacts, and dependable focused development checks.
- Preserve assertions and existing merge/nightly/release requirements. Any cadence
  change needs explicit owner agreement with a coverage/risk map. Tests execute
  afresh; passing outcomes cannot be cached.
- Finish with replayable measurements, invalidation/failure regressions and owner
  acceptance. Keep wider GI#106 concerns separate. No alpha.16 native coupling,
  separate compilation, major redesign, RM#154 caching or RM#157 runtime review.
- Estimate: three to five PRs, subject to baseline evidence. Implementation is merged in PR#637 and the
  [five-workload replay](VALIDATION_SPEED_ACCEPTANCE.md) meets the targets approved
  on 27 October. The owner accepted the alpha.15 milestone on 27 October 2026; it is complete.
  This does not close the wider umbrella issue.

## Agreed alpha.15 scope boundaries

[PR#605](https://github.com/sproates/panackelty/pull/605) records the 26 October
owner agreement on GI#134 completeness, GI#173 comprehensive truthful attribution,
and exact GI#435 constraints before implementation. It is merged into main. GI#180, GI#134, GI#173 and GI#435 issues carry the same clarification.
This is agreed direction, not implementation progress; weights are unchanged.

- **GI#180: Compiler understanding:** [completion scope agreed, PR#619](https://github.com/sproates/panackelty/pull/619),
  merged into main after owner approval. GI#134: Compiler explanations must expose every
  relevant function-checking requirement, outcome, supporting facts and source,
  with specific reasons for genuinely unreached decisions. GI#173: Compilation
  provenance must connect actual checking/transformations to every instruction
  and reverse source mapping, including generated origins, shared instructions
  and no executable output. Reconcile two checked inventories; no unexplained
  gaps or fabricated attribution. Five delivery slices: checker evidence,
  explanations, instruction attribution, compilation journey, integrated acceptance.
  Large; provisionally 5–7 implementation PRs, with tests in every slice, realistic
  examples and retention on/off cost evidence. Existing inference explanations
  remain included; new inference, runtime tracing and debugger/IR tooling do not.
  Inventories and representative transcripts precede implementation; this is an
  agreed completion boundary, not completed delivery.
- GI#435: [six-constraint semantic agreement, PR#607](https://github.com/sproates/panackelty/pull/607).
  Non-empty/non-blank/bounded-length strings count Unicode scalars; non-blank uses
  pinned Unicode White_Space. Control exclusion is U+0000–001F and U+007F–009F.
  Array/Map/Set size uses inclusive constant bounds. Same-record numeric fields
  support < or ≤ and conjunctions. Static proof or explicit runtime validation
  establishes guarantees; updates must preserve them. Validation preserves inputs
  and reports all failed constraints deterministically. New proof decisions need
  GI#134 explanations. [Binding/API agreement, PR#609](https://github.com/sproates/panackelty/pull/609):
  `type Name = Base where binding => condition`, no implicit value; migrate numeric
  guards. `validate[Type](input)` returns `Result[Type, ValidationError]` without
  printing or retaining original input automatically. Structured failures identify
  type, location, constraint and useful facts. Order by declaration, conjunctions
  left to right; record fields precede cross-field constraints. Remaining constraint
  APIs, concrete errors, rendering, Unicode 17.0.0 and transcripts are consolidated
  in the [approved contract, PR#616](https://github.com/sproates/panackelty/pull/616),
  merged into main after owner approval and passing lightweight checks. It specifies `.len()` for Str/Array,
  new Map/Set count methods, two named text predicates, complete-record replacement
  and five delivery slices. Existing numeric `||` is already supported and must
  be preserved. Only extension to new string, collection and record-relationship
  guards is scheduled for alpha.16, including proof and diagnostics. Earlier
  blanket deferral wording was incorrect. Alpha.15 new constraints support
  multiline and parenthesised `&&`. No implementation or extra completion credit.

- **GI#409: Compiler/stdlib boundary assessment:** complete on next (PR#635). [Assessment and reproducible evidence](COMPILER_STDLIB_BOUNDARY_ASSESSMENT.md)
  verify the compiler's 42-file closure, including nine stdlib modules and implicit
  core, and account for all five change cases. Real cache probes distinguish a
  repeated recipe from actual recompilation. Retain source organisation; consider
  narrower package-tool prerequisites at GI#106's measured-selection checkpoint.
  Native-operation registry coupling stays with alpha.16. No refactor delivered.
- **RM#154: Faster coverage:** [scope agreed, PR#624](https://github.com/sproates/panackelty/pull/624),
  merged into main after owner approval. Measure empty cache, unchanged repeat, reporting-only
  change and measured-source/dependency change, separating preparation, execution,
  aggregation and publication. Agree a numerical improvement target after profiling
  and before cache implementation. Reuse only validated plans matching compiler,
  VM, exporter, transitive sources, schema and settings; explain hits/rebuilds and
  safely regenerate invalid entries. Every test/assertion executes afresh with new
  sessions and counters. Require cached/fresh equivalence, complete invalidation
  and corruption tests, and material measured repeat-run improvement. Medium,
  about three implementation PRs. Schedule, corpus and report-integrity policy
  stay unchanged; GI#106: Validation speed remains separate. Slice A profiling has started;
  all four workloads pass after capacity and scope/exclusion metadata repairs.
  The numerical target is agreed; slice B safe plan reuse is in progress.
- **RM#155: Authoring friction:** [scope agreed, PR#626](https://github.com/sproates/panackelty/pull/626),
  merged into main after owner approval. Three improvements remain: diagnose unsupported interpolation
  with source location and a local-binding remedy, add tested examples for strings/
  bytes, collections, test reporting and filesystem/process failures, and explain
  public-signature type visibility with a checked `pub import` example. Preserve
  supported interpolation and visibility rules; document deliberate literal text
  and the interpolation compatibility change. Require before/after reproductions,
  regression tests and checked documentation examples. Inventory preflight and
  onboarding guidance remain completed work. Fixed list, medium, about three
  implementation PRs; no additional implementation has started.

## Agreed delivery order

Owner agreement, 26 October. [PR#632: Alpha.15 delivery order](https://github.com/sproates/panackelty/pull/632)
records the detailed sequencing and is merged into main after owner approval.

1. **GI#409: Compiler/stdlib boundary assessment** — dependency map, five change
   cases and recommendation; no refactor.
2. **GI#106: Validation speed** — baseline, agreed targets and bounded improvements,
   then owner acceptance.
3. **RM#154: Faster coverage** — baseline, agreed target, safe plan reuse and acceptance.
4. **GI#180: Compiler understanding foundations** — two inventories and missing
   evidence, explanation and instruction/source-attribution mechanisms.
5. **GI#435: Validated domain types** — approved contract with explanations and
   provenance included as new checking/proof decisions are introduced.
6. **RM#155: Authoring friction** — three remaining improvements and reconciliation;
   independent documentation may happen earlier.
7. **Integrated acceptance** — finish compiler-understanding completeness against
   the new domain-type decisions and complete combined feature/release acceptance.

The first three are practical sequencing, not hard technical prerequisites. A
substantial coverage obstacle need not block feature work; make any adjustment
explicit with evidence. GI#180 foundations precede GI#435, with final acceptance
covering both. Keep tests and required validation in every slice. Integrated
acceptance adds no independently weighted task and does not duplicate credit.
The order and scope weights remain unchanged. GI#409 assessment delivery is recorded above; GI#106 implementation and replay are delivered and the owner accepted the alpha.15 milestone on 27 October 2026.

## Release readiness and next action

- Remaining work is the open acceptance in the table. No release date is committed.
  The remaining alpha.15 items now have agreed scope boundaries.
  GI#435's consolidated contract is approved and merged in PR#616; its design
  baseline is settled. Do not restart those decisions. GI#180: Compiler understanding now also has an agreed completion boundary
  in PR#619. GI#409: Compiler/stdlib boundary assessment is also scoped in PR#621.
  RM#154: Faster coverage is scoped in PR#624 and RM#155: Authoring friction in
  PR#626. RM#157: Performance review belongs to alpha.16. Delivery order is agreed
  above. GI#409 assessment is complete on next (PR#635).
  GI#106: Validation speed is accepted for alpha.15. Next is RM#154: Faster
  coverage: implement safe plan reuse against the agreed target, then complete measured acceptance.
- Promotion, package/install release checks and owner publication approval remain.
  Complete product acceptance on next does not mean a feature has shipped.
- Latest supporting repair: [PR#601](https://github.com/sproates/panackelty/pull/601)
  merged into main adds upload retries and failure-time network evidence.
  The original DNS cause is unconfirmed; a successful scheduled nightly with the
  repairs has not yet been recorded here. This does not add earned scope credit.
- Alpha.16 native coupling work remains outside this release. The complete release
  allocation, including later-release work, stays in the agreed scope record.

## Keeping this record current

Update this file with every release-related task addition, start, scope change,
review, completion, reopening, removal, deferral or acceptance result, alongside
any affected roadmap, issue and programme records. Include the evidence and the
remaining action. Unmerged delivery must say it is awaiting merge; a completion
statement in a delivery PR takes effect on merge. Confirm that transition after
merge and correct any mismatch before handing over.

Keep one row per release workstream, with child detail in linked records. Date
the update; record removals or deferrals briefly with their decision link so scope
does not disappear silently. Preserve the weight baseline unless an explicit
scope change or explained re-estimate changes it. Do not append a running diary.

Use the copy on the integration branch named above, rather than an older main
copy or an unmerged feature branch. Routine status answers need only this file;
check another source when the requested detail is absent, evidence conflicts, or
a known subsequent change has not been recorded. State and repair that gap rather
than silently presenting stale information as current.

After publication, preserve the final snapshot in the release acceptance record,
then reuse this path for the next agreed release. Update the release, integration
branch, scope, baseline and readiness together. If no next release is selected,
say so instead of inventing one.
