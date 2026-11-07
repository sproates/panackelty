# Next release status

Updated: 7 November 2026. Release: **v0.1.0-alpha.16**. Integration branch: **next**.
Latest published release: **v0.1.0-alpha.15**, published 6 November 2026.

**Alpha.16: build useful programs. Ten VM optimisation batches are delivered by
this integration; further experiments are paused. RM#162 remains an investigation.** Alpha.15 is complete, including native archives, installer, browser
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
| [RM#162: Real-program fixes](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-162) | In progress: [async capability investigation](ASYNC_USEFUL_PROGRAMS_INVESTIGATION.md) reproduces the clock restriction and separate callback-state gap. The [discussion record](ASYNC_DESIGN_DISCUSSION.md) captures the preferred async-module API direction; state and closures remain exploratory. No fix implemented yet. Scope and effort emerge from actual use; verify each originating program and agree a release cutoff. No website update. |
| [GI#170: Inference/diagnostics assessment](https://github.com/sproates/panackelty/issues/170) | Planned assessment. No inference implementation or architecture is selected. |
| [RM#157: Performance review](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-157) | Ten owner-requested VM optimisation batches delivered by this integration; details below. Final local `make check` passed in 593 seconds with unchanged compiler bytecode. Hosted integration CI is pending. Broader performance-review workloads, targets and completion criteria still need agreement; RM#157 is not complete. Further optimisation experiments are paused. |
| [GI#435: Guard alternatives](https://github.com/sproates/panackelty/issues/435) | Planned follow-on for string, collection and record-relationship guards: proof rules and diagnostics need design. Numeric alternatives already work; the alpha.15 contract remains complete. |
| Native-interface/compiler decoupling | Allocated to alpha.16 in the [boundary record](roadmap/records/next-release--agreed-scope-2026-10-06.md). Define the bounded change using the completed [GI#409 assessment](COMPILER_STDLIB_BOUNDARY_ASSESSMENT.md); do not repeat the assessment. |
| Coverage approach review | Allocated to alpha.16 in the [performance record](roadmap/records/standing-performance-engineering--ongoing.md). Review cold preparation and retained decoded plans; the alpha.15 optimisation is already accepted. |

The VM optimisation batch is complete locally and included in this integration;
this does not complete RM#157 or RM#162. Delivery slices, ordering and
scope weights have not been established for this release; a weighted overall
percentage is therefore not yet available. Alpha.15's completed 90-point baseline
is preserved in its snapshot, not carried forward as alpha.16 progress.

GI#552: Full API reference and GI#553: Load profiling remain candidates, not
additional commitments. Work deferred from this release goes explicitly to
alpha.17 unless the owner chooses another destination.

## VM optimisation integration checkpoint

The owner requested integration into next on 7 November, retaining the development
branch for future experiments. The ten adopted batches are:

1. [Direct-call lookup](VM_CALL_LOOKUP_TRIAL.md).
2. [Direct concatenation allocation](VM_CONCAT_TRIAL.md).
3. [Indexed local variables](VM_LOCAL_SLOTS_TRIAL.md).
4. [String metadata reuse](VM_STRING_METADATA_TRIAL.md).
5. [Map/Set lookup indexes](VM_COLLECTION_INDEX_TRIAL.md).
6. [Small call-argument buffers](VM_CALL_ARGUMENTS_TRIAL.md).
7. [Builtin execution classification](VM_BUILTIN_DISPATCH_TRIAL.md).
8. [Shared Booleans](VM_SHARED_BOOLEANS_TRIAL.md).
9. [Shared Unit and Void](VM_SHARED_UNIT_VOID_TRIAL.md).
10. [Execution-scoped literal reuse](VM_LITERAL_REUSE_TRIAL.md).

Each was adopted after local validation; the final product tree passed full
make check (593 seconds), including unit, functional, bootstrap, packaging and
release smoke. Compiler bytecode remains unchanged. Individual trial percentages
are not additive or a controlled cumulative speedup claim. Memory trade-offs,
including literal retention, are recorded in the linked evidence.

Under the alpha.16 workflow trial, development/PR CI is suppressed and integration
into next starts hosted CI. Its result is pending; no hosted pass is claimed.
No release, installer, global installation or website publication is part of this
integration. Further experiments remain on feature/rm162-real-program-fixes.

## Branch readiness and next action

- PR #676 reconciled `main` into `next`, retaining the published alpha.15 product tree,
  installer defaults, release fixes and end-to-end release checklist, together
  with the later alpha.15 completion evidence from `next`.
- `VERSION` remains the released alpha.15 baseline until an actual alpha.16
  development/version change is needed. No alpha.16 artifacts are published.
- RM#162 first finding: enable a monotonic clock read in an async HTTP handler
  without permitting blocking I/O or marking clock reads pure. The existing
  native implementation calls `clock_gettime(CLOCK_MONOTONIC)`; the restriction
  is the current async effect contract, not a missing clock implementation.
  On 6 November, the checkout alpha.15 compiler accepted an ordinary `main`
  binding `reading = time.now()` and rejected the identical async body with
  `async function cannot call blocking function time.panack:now`.
  Next: define an async clock counterpart under the preferred `/async` module
  convention, then verify a real uptime endpoint. State/closure design is not
  committed scope.
- Follow the [release checklist](releases/RELEASE_CHECKLIST.md) for alpha.16:
  packages, installer and browser must be ready before the final website publication.
- Website scope remains unchanged by owner direction. Website PR #74 is closed.

Update status, remaining acceptance and evidence here alongside affected detailed
records. Preserve this snapshot at release completion before reusing the file.
