# Next release status

Updated: 26 October 2026. Release: **v0.1.0-alpha.15**. Integration branch: **next**.
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
| [GI#200 Packages](https://github.com/sproates/panackelty/issues/200) | Accepted on next: local folders/archives, HTTPS archives, pinned Git, diagnostics and installed acceptance. [P04c evidence](design/PACKAGE_FINAL_ACCEPTANCE.md), PR#595. Not yet released. | 30% | 100% | 30 |
| [GI#180 Compiler understanding](https://github.com/sproates/panackelty/issues/180) | Acceptance clarified: complete relevant checker explanations and truthful attribution for every instruction, including generated origins and reverse mappings. Inventories, transcripts and slices remain. | 20% | Unassessed | Unknown |
| [GI#435 Validated domain types](https://github.com/sproates/panackelty/issues/435) | Six-constraint semantics agreed: four string constraints, bounded arrays/maps/sets and ordered record-field relationships. Syntax, validation API, pinned Unicode version and example output remain before implementation. | 20% | 0% | 0 |
| [GI#170 Inference assessment](https://github.com/sproates/panackelty/issues/170) | Pending assessment of inference limits and diagnostics; no inference implementation selected. | 5% | 0% | 0 |
| [GI#409 Compiler/stdlib boundary](https://github.com/sproates/panackelty/issues/409) | Pending dependency and coupling assessment with recommendation; no reorganisation selected. | 5% | 0% | 0 |
| [RM#154 Faster coverage](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-154) | Planned. Validated plan reuse, cold/warm measurements and invalidation/equivalence acceptance remain. Required test executions stay fresh. | 5% | 0% | 0 |
| [RM#155 Authoring friction](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-155) | Partial. Example inventory preflight and contributor/interpolation guidance delivered (PR#565 for guidance). Final selected-item reconciliation and acceptance remain. | 5% | Unquantified | Unknown |
| [RM#157 Performance review](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-157) | Open. Holistic measurements, prioritised findings and bounded quick wins remain to be assessed. | 5% | Unassessed | Unknown |
| [GI#106 Validation speed](https://github.com/sproates/panackelty/issues/106) | Scope agreed, 26 October: five baseline workloads, at most three selected improvements and measured owner acceptance. Baseline not started. Earlier repairs (PR#596–598) remain delivered; the bounded milestone is not complete. | 5% | Unquantified | Unknown |
| **Total** | **30 confirmed points; overall completion remains unquantified.** | **100%** | **Unknown** | **30 known** |

Weights preserve the provisional baseline reported on 26 October 2026; they are
not owner-approved estimates. Packages and the two large compiler workstreams
carry most of the weight. Unknown contributions are not zero. Do not describe
the known subtotal as an exact overall completion percentage or count child
programme deliveries again. Explain any future baseline change.

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
- Estimate: three to five PRs, subject to baseline evidence. Measurements and
  implementation have not started. The next GI#106 action is the baseline when
  authorised; no active work is implied by this scope record.

## Scope definition still required

[PR#605](https://github.com/sproates/panackelty/pull/605) records the 26 October
owner agreement on GI#134 completeness, GI#173 comprehensive truthful attribution,
and exact GI#435 constraints before implementation. It is merged into main. GI#180, GI#134, GI#173 and GI#435 issues carry the same clarification.
This is agreed direction, not implementation progress; weights are unchanged.

- GI#180: reconcile decision/transformation inventories; agree transcripts,
  realistic acceptance cases, delivery slices and effort. Missing implementation
  is not unavailable evidence. Instructions with no genuine source location need
  an explicit generated origin; source with no executable output needs explanation.
- GI#435: [six-constraint semantic agreement, PR#607](https://github.com/sproates/panackelty/pull/607).
  Non-empty/non-blank/bounded-length strings count Unicode scalars; non-blank uses
  pinned Unicode White_Space. Control exclusion is U+0000–001F and U+007F–009F.
  Array/Map/Set size uses inclusive constant bounds. Same-record numeric fields
  support < or ≤ and conjunctions. Static proof or explicit runtime validation
  establishes guarantees; updates must preserve them. Validation preserves inputs
  and reports all failed constraints deterministically. New proof decisions need
  GI#134 explanations. Syntax, API/error shape, Unicode version and transcripts
  remain for agreement. No implementation started; count integration once.
- GI#170: fix the assessment questions, representative inference/diagnostic cases
  and report acceptance; separate assessment from any later implementation.
- GI#409: define the assessment deliverable and representative dependency/change
  cases. The compiler/stdlib boundary assessment is already reasonably bounded;
  it must not absorb alpha.16 coupling implementation.
- RM#154: agree measured workloads and improvement target after profiling; retain
  the existing cache correctness, fresh execution and equivalence requirements.
- RM#155: settle the finite selected-item list and evidence for each, reconciling
  improvements already delivered with remaining work.
- RM#157: bound review workloads and selected quick wins, with explicit completion
  criteria and boundaries against GI#106 and RM#154.

## Release readiness and next action

- Remaining work is the open acceptance in the table. No release date is committed.
  Current planning work: define remaining alpha.15 commitments one at a time.
  Next proposed discussion: GI#435 syntax, validation API and example output. Implementation
  ordering follows scope definition; no implementation is started by this record.
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
