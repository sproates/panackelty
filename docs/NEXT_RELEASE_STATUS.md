# Next release status

Updated: 6 November 2026. Release: **v0.1.0-alpha.16**. Integration branch: **next**.
Latest published release: **v0.1.0-alpha.15**, published 6 November 2026.

**Alpha.16: build useful programs. Ready to begin delivery; feature work has not
started.** Alpha.15 is complete, including native archives, installer, browser
playground and website. Its [final status snapshot](ALPHA15_FINAL_STATUS.md) and
[release acceptance](releases/ALPHA15_RELEASE_ACCEPTANCE.md) preserve the evidence.

Read this file first for release status. Detailed requirements remain in the
[agreed allocation](roadmap/records/next-release--agreed-scope-2026-10-06.md#agreed-release-direction--20-october-2026)
and linked work records. Update this summary whenever release work changes.

## Allocated work

| Item | State and remaining definition |
| --- | --- |
| [GI#166: Typed HTTP exchanges](https://github.com/sproates/panackelty/issues/166) | Planned. Runnable installed success/failure workflows; no general RPC framework. |
| [GI#167: Recursive file discovery](https://github.com/sproates/panackelty/issues/167) | Planned. Reconcile existing examples and APIs with the issue before defining the remaining delivery. |
| [GI#168: Subprocess examples](https://github.com/sproates/panackelty/issues/168) | Planned. Reuse existing examples and verify the remaining installed workflows. |
| [GI#169: Tiny build tool](https://github.com/sproates/panackelty/issues/169) | Planned. A useful small program, not a production build system. |
| [GI#138: Tested guides](https://github.com/sproates/panackelty/issues/138), with [GI#310: Examples](https://github.com/sproates/panackelty/issues/310) | Planned. Coordinate guides with runnable examples; do not count the same delivery twice. |
| [RM#162: Real-program fixes](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-162) | Planned. Scope and effort emerge from actual use. Fix selected obstacles and verify the originating programs; agree a release cutoff. This core planning placeholder requires no website update. |
| [GI#170: Inference/diagnostics assessment](https://github.com/sproates/panackelty/issues/170) | Planned assessment. No inference implementation or architecture is selected. |
| [RM#157: Performance review](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-157) | Planned review and bounded improvements. Workloads, targets and selected improvements still need agreement. |
| [GI#435: Guard alternatives](https://github.com/sproates/panackelty/issues/435) | Planned follow-on for string, collection and record-relationship guards: proof rules and diagnostics need design. Numeric alternatives already work; the alpha.15 contract remains complete. |
| Native-interface/compiler decoupling | Allocated to alpha.16 in the [boundary record](roadmap/records/next-release--agreed-scope-2026-10-06.md). Define the bounded change using the completed [GI#409 assessment](COMPILER_STDLIB_BOUNDARY_ASSESSMENT.md); do not repeat the assessment. |
| Coverage approach review | Allocated to alpha.16 in the [performance record](roadmap/records/standing-performance-engineering--ongoing.md). Review cold preparation and retained decoded plans; the alpha.15 optimisation is already accepted. |

No alpha.16 implementation completion is claimed. Delivery slices, ordering and
scope weights have not been established for this release; a weighted overall
percentage is therefore not yet available. Alpha.15's completed 90-point baseline
is preserved in its snapshot, not carried forward as alpha.16 progress.

GI#552: Full API reference and GI#553: Load profiling remain candidates, not
additional commitments. Work deferred from this release goes explicitly to
alpha.17 unless the owner chooses another destination.

## Branch readiness and next action

- Reconcile `main` into `next`, retaining the published alpha.15 product tree,
  installer defaults, release fixes and end-to-end release checklist. Preserve
  the later alpha.15 completion evidence from `next`.
- `VERSION` remains the released alpha.15 baseline until an actual alpha.16
  development/version change is needed. No alpha.16 artifacts are published.
- Select the first allocated item and define its bounded remaining outcome using
  existing code and evidence. This reconciliation starts no feature implementation.
- Follow the [release checklist](releases/RELEASE_CHECKLIST.md) for alpha.16:
  packages, installer and browser must be ready before the final website publication.
- Website scope remains unchanged by owner direction. Website PR #74 is closed.

Update status, remaining acceptance and evidence here alongside affected detailed
records. Preserve this snapshot at release completion before reusing the file.
