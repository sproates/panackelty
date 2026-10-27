# Faster coverage: baseline and cache contract

RM#154, alpha.15. Work started 27 October 2026. This is slice A, before cache
implementation. The first repaired collection is blocked by the inventory-size
limit. Complete measurements and the numerical target remain pending; no speed
improvement or task completion is claimed.

## Delivery slices

- **A: Baseline and target.** Measure empty-cache collection, unchanged repeat,
  reporting-only change and measured-source/dependency change. Separate exporter
  compilation, plan generation/validation, fresh execution, aggregation, report
  writing and hosted publication. Agree the numerical target before implementation.
- **B: Safe plan reuse.** Implement validated persistent preparation reuse with
  invalidation, corruption/recovery and fresh-assertion tests in the same delivery.
- **C: Acceptance and integration.** Replay all four workloads, verify equivalent
  coverage semantics and provenance, record hosted evidence and operating guidance.

The selected corpus, weekly/manual schedule and report-integrity policy remain
unchanged. No prior test results, sessions or counters may be reused. The wider
compiler-coupling work remains outside this task.

## Baseline readiness

The initial unmodified local run failed while compiling
`tests/source_coverage/export.panack`: it called the removed global `eprint`.
The scheduled collection on 26 October failed for the same reason:
[run 40296515861](https://github.com/sproates/panackelty/actions/runs/40296515861).
After replacing those calls with the existing `stdlib/host.stderr`, the parity
fixture exposed a remaining removed `command_args` call. The fixture now calls
`stdlib/host.arguments`. Neither repair changes the intended coverage semantics. PR#640 delivers these\ncompatibility repairs and records the baseline blocker.

With both repairs, the parity comparison passes for both fixture paths: 388 exact
source items per path match the accepted source renderer. Failed pre-repair runs
are diagnostic evidence, not successful performance samples.

## Measurement method

Use the existing collector and manifest without changing their limits or selected
assertions. A temporary external Node profiler wraps subprocess calls and exported
reader/aggregation/report functions, recording elapsed monotonic time. No timing
instrumentation is added to production source. Native build preparation is outside
the collection measurement. Runs execute sequentially on the same machine.

Every run creates a fresh collection workspace. The current collector has no
persistent plan cache, so both the empty-cache run and unchanged repeat generate
plans from scratch. This is the before-implementation baseline, not a cache-hit
measurement. Reporting-only changes alter an output caption; the dependency
workload appends a comment to `src/stdlib/bytes.panack`, changing source identity
without changing executable behavior. Temporary edits are restored after each run.
The comment-only case does not establish behavior-changing dependency acceptance;
slice B must also test such changes and the resulting invalidation.

The local environment is macOS/Darwin 25.5.0, arm64, 18 logical CPUs, Node 26.0.0.
Existing unrelated untracked files require `--allow-dirty`; all local reports are
explicitly non-publishable development evidence. Hosted collection uses Node 24;
local and hosted absolute timings must not be treated as directly comparable.

## First collection attempt and blocker

The repaired cold attempt stopped after 385.68 seconds at
`tests/runner/compiler_module_bindings_unit.panack`. Exporter compilation took
29.29 seconds; completed and failed plan-generation calls took 354.73 seconds.
Six fresh sessions completed before the failure, taking 0.65 seconds in total.
These are partial timings, not the cost of a complete successful collection.
The unchanged, reporting-only and dependency workloads were not run: each would
reach the same unsupported inventory before a comparable report could complete.

The module-binding export failed after 128.63 seconds, within its existing
300-second deadline. This was not a timeout. The exporter combines two distinct
failures into `coverage plan: source changed`: changed snapshots and an empty
serialized inventory. Its saved plan contains an empty inventory payload. The
44 source shards imply approximately 19,962,707 bytes of serialized inventory,
above the serializer's 16,777,216-byte limit. Its compiler and bytecode artifact
are only 1,012,714 and 1,383,683 bytes respectively.

The compiler serializer, VM coverage identity reader and JavaScript report
reader all enforce the current bound. Changing only the exporter would not
provide supported larger inventories. A prerequisite needs explicit inventory
bounds, compatible consumers, boundary/rejection tests and distinct diagnostics.
The owner has been asked whether to include this prerequisite before resuming
slice A. No limit or format change has been implemented or approved by this
baseline document. The existing full corpus must remain selected.

Publication is independently bounded. Historical successful publication
[run 38779817246](https://github.com/sproates/panackelty-coverage/actions/runs/38779817246)
had preparation/deploy/verification jobs of 28/9/29 seconds, including 21 seconds
verifying live files. The latest publication
[run 40313444348](https://github.com/sproates/panackelty-coverage/actions/runs/40313444348)
spent 23 seconds in preparation and skipped deployment. Neither is a controlled
before/after sample for this change; publication is not included in local
collection timings.

## Proposed cache contract


Core collection owns executable preparation. The separate coverage publisher
continues to receive static reports only.

A reusable entry must bind the entry-point identity, compiler and VM bytes,
exporter and its transitive inputs, measured source closure, resolution inputs,
plan/inventory schema and relevant settings. Preserve source-root identity and
account for changes that can alter module resolution, not only file contents in
an old closure. Reporting-only presentation changes are not plan inputs.

Store the artifact, inventory, plan header and all source-plan shards together
with their identities and provenance. A matching key does not establish valid
contents: apply the existing bounded readers, exact artifact/inventory/source
checks and provenance validation before execution. Only trusted generation may
populate the cache. Publish complete entries atomically; missing, partial,
corrupt, untrusted or stale entries cause a clear rebuild decision.

Each collection still executes all selected tests with new sessions and counters,
checks assertions and current source/toolchain identity, aggregates fresh results
and writes a newly identified report. Log the reason for reuse or rebuilding.
The implementation must retain changes-during-collection detection.

## Acceptance checkpoint

The target will be proposed from completed measurements, with the expected saving
and irreducible fresh-execution cost stated explicitly. Owner agreement precedes
cache implementation. This document does not approve a numerical target.
