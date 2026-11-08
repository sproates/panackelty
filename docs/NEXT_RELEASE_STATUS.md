# Next release status

Updated: 8 November 2026. Release: **v0.1.0-alpha.16**. Integration branch: **next**.
Latest published release: **v0.1.0-alpha.15**, published 6 November 2026.

**Alpha.16: build useful programs. Ten VM optimisation batches are delivered by
this integration; further VM experiments are paused. Compiler reuse has been
implemented and validated locally and is included in this integration ahead of
larger language changes; compiler integration and its coverage-manifest correction are green. RM#162 remains an investigation.** Alpha.15 is complete, including native archives, installer, browser
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
| [RM#157: Performance review](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-157) | Ten owner-requested VM optimisation batches delivered by this integration; details below. Final local `make check` passed in 593 seconds with unchanged compiler bytecode. VM integration CI passed ([run](https://github.com/sproates/panackelty/actions/runs/42349747971)). Broader performance-review workloads, targets and completion criteria still need agreement; RM#157 is not complete. Further VM experiments are paused; the compiler-reuse batch passed local validation and is included in this integration (see below). |
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
into next starts hosted CI. The VM integration passed in [run 42349747971](https://github.com/sproates/panackelty/actions/runs/42349747971). Compiler PR #678 merged; the missing coverage-manifest entries were corrected in PR #679. [Integration run 42422608096](https://github.com/sproates/panackelty/actions/runs/42422608096) passed.
No release, installer, global installation or website publication is part of this
integration. Further experiments remain on feature/rm162-real-program-fixes.

## Branch readiness and next action

- The owner selected compiler reuse before larger language changes. The
  [compiler reuse assessment](COMPILER_REUSE_ASSESSMENT.md) records the current
  path, existing caches and invalidation risks. The first product slice
  reuses successful project preparation within a command, eliminating duplicate
  module lowering. It is complete locally with a measured 3% self-compilation
  saving and a passing full local check (518 seconds); the change is included in this integration.
  The second slice, [unchanged-compilation reuse](COMPILER_UNCHANGED_REUSE_TRIAL.md),
  is complete locally as an opt-in trial. Warm self-compilation fell from 14.34s
  to 2.34s with identical bytecode; cold compilation took 15.04s. Full local
  validation passed in 572 seconds. It is included in this integration, and module-level
  body reuse after edits is validated in [retained in-memory snapshots](COMPILER_MODULE_REUSE_TRIAL.md):
  an entry-module edit reused 46 of 47 modules and took 8.27s versus 15.48s fresh,
  with identical bytecode. All 54 focused cases and full local validation passed
  (563 seconds). [Persistent module reuse](COMPILER_PERSISTENT_REUSE.md) now
  implements the selected one-shot `panack compile --incremental` interface in
  `next` via PR #678, with automatic private storage and fresh-process tests.
  All 117 focused checks, 15 CLI groups and full local validation pass (551s).
  Median self-compilation is 16.01s fresh, 20.67s cold incremental, 2.51s
  unchanged and 14.02s after an entry-file edit reusing 48 of 49 modules.
  The cache remains conservative and opt-in. The narrower declaration rule now
  passes 24 CLI groups and reuses 46/49 modules on an edited self-build (13.86s
  versus 15.91s fresh). Final local `make check` passed in 591 seconds (unit, functional, bootstrap,
  packaging and installed smoke checks). The separate
  [hosted cache trial](COMPILER_HOSTED_CACHE_TRIAL.md) passed: restored unchanged
  compilation 4.83s versus 32.73s fresh; edited compilation 21.11s versus 29.67s
  fresh on the same warm runner. Ordinary CI cache wiring is not enabled. Foreground watch work stays parked.
  This continues RM#28: Incremental and modular builds
  alongside RM#157: Performance review, without declaring either complete.

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
  When language work resumes: define an async clock counterpart under the preferred `/async` module
  convention, then verify a real uptime endpoint. State/closure design is not
  committed scope.
- Follow the [release checklist](releases/RELEASE_CHECKLIST.md) for alpha.16:
  packages, installer and browser must be ready before the final website publication.
- Website scope remains unchanged by owner direction. Website PR #74 is closed.

Update status, remaining acceptance and evidence here alongside affected detailed
records. Preserve this snapshot at release completion before reusing the file.

## Compiler reuse follow-up order

The owner selected integration of the validated batch, followed by narrower
invalidation for function edits in files containing type declarations, then a
separate hosted cache-reuse trial. Bootstrap proofs must remain fresh. Development
branch pushes and PR updates remain CI-free under the alpha.16 workflow trial.

The validated narrower-invalidation change and manual hosted trial are included
in this follow-up integration. Local acceptance and the explicit hosted trial
passed; ordinary next-branch CI runs on integration. The experiment branch is retained.

## Incremental build profile, 8 November

The [CI adoption assessment and edited-build profile](COMPILER_EDIT_PROFILE.md)
found that the current next-push route performs no compiler preparation to cache.
Cross-run nightly adoption remains separate; existing probe caches already reuse
complete artifacts within a build tree. No automatic CI cache was enabled.
The agreed profiling fallback completed: diagnostic edited-build time is dominated
by checked-body decoding (4.54s) and fresh signatures (4.24s), while changed-body
checking takes 0.17s. Normal edited compilation took 13.80s and matched fresh
output. The selected decoder trial reduced median decode time by 12.0% and complete
edited-build time by 6.3% across three alternating pairs. Artifacts match fresh
compilation; focused tests, bootstrap fixed points and final `make check` (617s)
passed. The bounded decoder improvement is included in this integration; it
does not complete RM#157 or GI#106. The previous product integration passed in
[run 42423346639](https://github.com/sproates/panackelty/actions/runs/42423346639).

## Local compiler workflow adoption

Included in this integration: a routine `make compiler` candidate build keeps the existing
whole-artifact probe cache and uses incremental module reuse after source edits.
An explicit `make compiler-fresh` comparison bypasses both caches. Bootstrap and
CI entry points stay unchanged. Focused checks pass. Real edited builds reuse 46/49 modules and match fresh
bytecode: median 13.81s versus 15.86s fresh; unchanged artifact reuse takes 0.14s.
The final checkout candidate matches the seed and checks a real example. Final
`make check` passed in 567s. This completes the local workflow slice, not the
broader RM#157/GI#106 work.

## Signature checking trial

Included in this integration: the detailed profile found repeated public re-export resolution
after accessibility was already proved. Two short-circuit conditions avoid that
work without caching signatures. Diagnostic phase time fell from 4.32s to 1.67s;
normal VM edited builds improved from median 13.51s to 11.02s (18.4%), with
matching fresh bytecode. All three fresh-build pairs improved too. The 1,448
module-binding assertions, six exact diagnostic comparisons and native artifact
checks pass. Bootstrap fixed points and final `make check` (565s) passed. This
completes the bounded search optimisation, not RM#157/GI#106.

## Name-lookup traversal trial

Completed, not adopted: stopping the import-selector walk reduced calls but
produced no meaningful ordinary-VM saving (13.155s original / 13.170s candidate
medians). Bytecode matched and the candidate passed 1,453 assertions. Trial code
and tests were removed; product code and seed remain unchanged. The profile
identifies repeated binding-conflict validation as the next candidate: 29,556
calls across 49 modules. The follow-up trial below implements a bounded reuse contract.

## Binding-validation reuse trial

Included in this integration: retain binding diagnostics with their exact
input declarations and imports within a compilation. Changed snapshots and raw
bindings still validate normally. The candidate passes 1,456 module-binding
assertions, including stale-input and diagnostic-position checks. Three paired
fresh self-builds improved by median 29.9%; three edited-build pairs
improved by 19.5%, retaining 46/49 module hits and matching bytecode. The candidate
passes exact diagnostic and native-artifact comparisons, three-stage bootstrap
fixed points and final `make check` (611s). This completes the bounded optimisation,
not RM#157/GI#106. The experiment branch is retained.

## Post-optimisation profile

Completed: [five paired measurements and a phase profile](COMPILER_EDIT_PROFILE.md#post-optimisation-profile-8-november-2026)
show fresh/edited self-builds effectively level (10.713s/10.827s medians), with
substantial timing variation. Unchanged reuse takes 2.834s. Decoding saved bodies
is the largest edited cost at 4.494s; checking changed bodies takes 0.146s.
The recommended next trial is cheaper checked-body decoding, preserving formats
and rejection checks. No implementation has started. This measurement-only
follow-up changes no compiler or cache behaviour and does not complete RM#157/GI#106.

## Checked-body decoder follow-up

Included in this integration: combined span reconstruction and balanced body-node tag
selection improve three paired edited builds by median 3.4% (10.620s to 10.260s),
with matching fresh outputs and 46/49 module reuse. Cache/bytecode formats are
unchanged; 1,471 focused assertions, native artifacts, bootstrap checks and final
`make check` (560s) passed. This completes the bounded batch, not RM#157/GI#106.
The [trial record](COMPILER_EDIT_PROFILE.md#span-and-body-node-decoder-trial-8-november-2026)
also captures the larger representation/reconstruction question raised by the
owner. No architectural redesign is selected or implemented.

## Retained-data assessment

Completed: [consumer audit and real-payload inventory](COMPILER_EDIT_PROFILE.md#retained-data-assessment-8-november-2026)
found 123,405 type descriptions but 1,705 distinct per-module values. A dictionary
size model reduces 9.08 MiB payloads by 14.4% for types alone and 31.5% with
identities/bindings; these are size estimates, not measured speedups. Straightforward
unused-field removal accounts for only 2.6%. The recommended bounded shared-type
trial is recorded below. A larger lowered-code/effect-summary redesign remains
unimplemented; RM#157/GI#106 remain open.

## Shared-type cache trial

Included in this integration: [per-module shared types](COMPILER_EDIT_PROFILE.md#shared-type-cache-trial-8-november-2026)
reduce median edited self-build time by 10.5%, peak RSS by 9.4%, and module-cache
files by 14.4%. All three pairs match fresh output and reuse 46/49 modules.
Cold/unchanged observations show no regression here. The private cache is version 2;
old entries fall back to fresh checking. All 1,487 focused assertions, cache CLI
checks, native artifacts, coverage-policy tests, bootstrap and final `make check`
(565s) pass. RM#157/GI#106 remain open.
No VM bytecode change or broader cache redesign is included.
