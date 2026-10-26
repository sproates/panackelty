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
| [GI#180 Compiler understanding](https://github.com/sproates/panackelty/issues/180) | Remaining GI#173 provenance and GI#134 explanation coverage open. Earlier accepted outcomes remain accepted; the remaining release scope needs bounded delivery slices. | 20% | Unassessed | Unknown |
| [GI#435 Validated domain types](https://github.com/sproates/panackelty/issues/435) | Planned. Settle semantics and implement string, collection and cross-field guards with static proof and runtime validation. | 20% | 0% | 0 |
| [GI#170 Inference assessment](https://github.com/sproates/panackelty/issues/170) | Pending assessment of inference limits and diagnostics; no inference implementation selected. | 5% | 0% | 0 |
| [GI#409 Compiler/stdlib boundary](https://github.com/sproates/panackelty/issues/409) | Pending dependency and coupling assessment with recommendation; no reorganisation selected. | 5% | 0% | 0 |
| [RM#154 Faster coverage](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-154) | Planned. Validated plan reuse, cold/warm measurements and invalidation/equivalence acceptance remain. Required test executions stay fresh. | 5% | 0% | 0 |
| [RM#155 Authoring friction](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-155) | Partial. Example inventory preflight and contributor/interpolation guidance delivered (PR#565 for guidance). Final selected-item reconciliation and acceptance remain. | 5% | Unquantified | Unknown |
| [RM#157 Performance review](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-157) | Open. Holistic measurements, prioritised findings and bounded quick wins remain to be assessed. | 5% | Unassessed | Unknown |
| [GI#106 Validation speed](https://github.com/sproates/panackelty/issues/106) | Partial. Tool preparation/reuse and compiler watchdog repairs delivered (PR#596–598). Comparable speed evidence and explicit owner acceptance remain. CI repairs alone do not complete this milestone. | 5% | Unquantified | Unknown |
| **Total** | **30 confirmed points; overall completion remains unquantified.** | **100%** | **Unknown** | **30 known** |

Weights preserve the provisional baseline reported on 26 October 2026; they are
not owner-approved estimates. Packages and the two large compiler workstreams
carry most of the weight. Unknown contributions are not zero. Do not describe
the known subtotal as an exact overall completion percentage or count child
programme deliveries again. Explain any future baseline change.

## Release readiness and next action

- Remaining work is the open acceptance in the table. No release date is committed.
  Suggested next planning action: bound the remaining compiler-understanding
  work into delivery slices. This is a recommendation, not new authorisation.
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
