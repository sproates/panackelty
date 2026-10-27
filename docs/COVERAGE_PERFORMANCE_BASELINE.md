# Faster coverage: baseline and cache contract

RM#154, alpha.15. Work started 27 October 2026. Slice A is complete; slice B local reuse is implemented and verified in PR#644. The capacity and stale coverage metadata repairs pass validation.
All four baseline workloads completed successfully on 27 October 2026. The owner agreed the numerical target below on 27 October 2026; no speed improvement
or overall task completion is claimed.

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
`stdlib/host.arguments`. Neither repair changes the intended coverage semantics.
PR#640 delivers these compatibility repairs and records the baseline blocker.

With both repairs, the parity comparison passes for both fixture paths: 388 exact
source items per path match the accepted source renderer. Failed pre-repair runs
are diagnostic evidence, not successful performance samples.

## Measurement method

Use the existing collector and manifest with the approved capacity adjustment
and unchanged selected assertions. A temporary external Node profiler wraps subprocess calls and exported
reader/aggregation/report functions, recording elapsed monotonic time. No timing
instrumentation is added to production source. Native build preparation is outside
the collection measurement. Runs execute sequentially on the same machine.
The profiler also records each subprocess's peak resident memory using macOS
`time -l`; these are individual-process peaks, not total machine memory usage.

Every run creates a fresh collection workspace. The current collector has no
persistent plan cache, so both the empty-cache run and unchanged repeat generate
plans from scratch. This is the before-implementation baseline, not a cache-hit
measurement. Reporting-only changes alter an output caption; the dependency
workload appends a comment to `src/stdlib/bytes.panack`, changing source identity
without changing executable behavior. Temporary edits are restored after each run.
The comment-only case does not establish behavior-changing dependency acceptance;
the slice B fixture below separately tests such changes and invalidation.

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
The owner approved inspecting the size breakdown, raising the inventory limit
to 64 MiB consistently, preserving unrelated bounds and testing diagnostics,
then resuming the baseline. The repair and complete measurements are recorded below;
the existing full corpus remains selected.

Publication is independently bounded. Historical successful publication
[run 38779817246](https://github.com/sproates/panackelty-coverage/actions/runs/38779817246)
had preparation/deploy/verification jobs of 28/9/29 seconds, including 21 seconds
verifying live files. The latest publication
[run 40313444348](https://github.com/sproates/panackelty-coverage/actions/runs/40313444348)
spent 23 seconds in preparation and skipped deployment. Neither is a controlled
before/after sample for this change; publication is not included in local
collection timings.

## Agreed cache contract


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

The owner approved the measured numerical target on 27 October 2026 before
slice B implementation. The expected saving and irreducible fresh-execution cost
are recorded below.

## Approved capacity adjustment, 27 October 2026

The failing inventory contains 44 distinct source files and 111,315 items.
Source text occupies 1,383,844 bytes; structural item IDs 10,272,445; item kinds
1,038,386; details 861,872; source positions 2,671,560. Names, length prefixes,
compiler bytes and program bytes make up the remainder. Structural paths repeat
prefixes by design. There are no duplicate source-file entries. This is a verbose
representation, but no accidental duplication was found; redesign is not selected.

The implementation allows 64 MiB inventories and 128 MiB enclosing plans,
registries and records. The enclosing bound accommodates the inventory plus
compiler/program bytes and probe/counter metadata. Synchronous filesystem I/O
also needs a 128 MiB ceiling to exchange those containers; callers retain their
explicit lower read limits. Compiler and bytecode identities remain 16 MiB, source
files 1 MiB, source closures 8 MiB/256 files, item counts 100,000 per file,
instruction cells 262,144, executions 4,096 and total-session data 256 MiB.
Rooted/async filesystem, HTTP and process-output bounds are unchanged. This is a
capacity change, not a coverage format or attribution change.

Exporter diagnostics distinguish inventory limits from changed source snapshots.
Native boundary tests cover exactly 64 MiB through session creation, inherited
execution and raw recording, plus one-byte-over rejection and unchanged bytecode
limits. JavaScript tests exercise the same inventory boundary, oversized
containers and retained bounded metadata. Self-hosted tests cover size-diagnostic
boundaries, missing compiler identity and decoding of valid larger containers.
Full local validation passed in 817 seconds, and both parity paths matched all
388 source items. Native and JavaScript capacity boundaries passed.

The first capacity-adjusted collection completed all 31 fresh sessions. The
formerly failing module-binding inventory was 19,977,611 bytes; its plan took
129.74 seconds and peaked at 1,104,052,224 resident bytes. Collection reached
aggregation after 1,006.86 seconds: exporter compilation 29.08 seconds, plan
generation 868.99 seconds and fresh execution 102.48 seconds. These remain
diagnostic timings because report validation did not complete.

Aggregation found that `stdlib/http_transport.panack`, already eligible in the
unchanged manifest, was absent from all source closures. The scope fixture now
imports that module explicitly. Replaying retained evidence with the repaired
scope for diagnosis then exposed a stale declaration-exclusion hash. The old
hash matches the published 15 October report (commit `061212655bf90cc9aca56003b46aac68255a288c`).
The current list was checked against source: 235 imports, 134 records and 26 enums,
with no executable functions or statements excluded. The policy records this
review and refreshed identities; eligible/unmeasured scope and assertion selection
are unchanged. Diagnostic replay is not a fresh baseline measurement. All four
workloads were then rerun with fresh sessions, with the results below.


## Completed four-workload baseline

Measured commit: `bee774413938483877713790a2db28fc93632c79` (PR#642).
All four collectors completed and passed report-integrity checks. Each ran 31
sessions/executions; all 124 session nonces are distinct. The temporary reporting
and source edits were restored. These remain local, non-publishable development
reports, not hosted publication evidence or a claim of cache performance.

Seconds, rounded to two decimal places:

| Phase | Empty cache | Unchanged repeat | Reporting-only change | Dependency source change |
| --- | ---: | ---: | ---: | ---: |
| Total | 955.43 | 939.13 | 958.23 | 946.91 |
| Exporter compilation | 29.71 | 30.24 | 30.09 | 30.10 |
| Plan generation | 820.73 | 804.60 | 822.64 | 815.18 |
| Plan validation | 2.32 | 2.38 | 2.41 | 2.41 |
| Fresh execution | 98.72 | 97.86 | 99.05 | 95.31 |
| Session validation | 0.52 | 0.51 | 0.60 | 0.55 |
| Observation mapping and aggregation | 2.94 | 3.02 | 2.92 | 2.92 |
| Report writing | 0.03 | 0.03 | 0.03 | 0.03 |
| Largest subprocess peak RSS (MiB) | 1040.91 | 1052.14 | 1039.92 | 1040.30 |

Totals also include orchestration, metadata and integrity checks. The memory
figures are the largest individual subprocess peaks, not whole-collection or
machine peaks. These are one sample per workload on one machine; the small timing
differences are not evidence that one variant is faster. The capacity adjustment
has not been credited with a performance improvement.

Unchanged and reporting-only runs match every measured file's lines, functions,
branches, exclusions and metrics exactly, including counters. Their artifact and
inventory identities also match. The dependency case changes only the measured
`src/stdlib/bytes.panack` source hash and 13 session inventory identities; all
artifact bytes, coverage states and metrics remain equivalent. Lexer counters
change because the compiler processes the additional comment, as expected.
Behavior-changing dependency invalidation is tested separately by the slice B fixture below.

Plan generation alone accounts for roughly 86% of collection time. Even with
reuse, approximately 95–99 seconds of fresh execution must remain, plus validated
plan reads, aggregation and reporting. Hosted publication is separately bounded
as described above and is not included in these local totals.

## Numerical target agreed on 27 October 2026

For unchanged-repeat and reporting-only workloads, reduce median collection time
by **at least 75%** against a matched uncached baseline. At these observed costs,
that means roughly **four minutes or less**, while executing all 31 sessions afresh.
For empty-cache and dependency-change workloads, allow no more than **10%** median
regression against matched uncached collection. Measure three sequential samples
per compared case on the same machine/toolchain, separate from other validation.
Do not trade assertions, source scope, provenance, corruption checks or fresh
execution for speed. Record memory peaks and explain any material increase.

The owner agreed these targets and authorised slice B implementation. The expected
saving comes from reusing validated preparation, not test results or counters.

## Slice B: local authenticated plan reuse

PR#644 delivers local preparation reuse. The collector caches preparation by default in
`build/source-plan-cache`; `--no-plan-cache` selects uncached measurement and
`--plan-cache PATH` selects a different local payload directory. Exporter compilation
is lazy: a run with only valid hits does not need to compile the exporter.

The first implementation deliberately uses a conservative input snapshot: all
Panack source contents and directory identities under `src/` and `tests/`, package
manifest/lock inputs there and at the checkout root, the launcher, version, cache
producer, compiler/VM bytes, canonical checkout/stdlib roots and fixed generation
settings. Source additions, removals, renames and symlink identities participate;
mtime alone never establishes freshness. This captures resolution changes as well
as dependency content. It may invalidate unaffected plans, but does not claim
fine-grained dependency reuse. A plan whose sources escape the captured roots
cannot be cached. Presentation-only report changes are not generation inputs.

Only locally generated plans that pass the existing bounded plan reader can be
sealed. A receipt authenticates the exact input context and every plan member's
name, size and SHA-256 with HMAC-SHA256. Its random local key is stored separately
in `build/source-plan-trust`, outside the payload cache. Both directories and
files must be private to the current POSIX user; linked and hard-linked members
are rejected. The trusted user/host is the security boundary; this is not a
sandbox against the same user deliberately replacing trusted state. Do not copy,
upload or restore the trust directory as part of a shared cache. Losing the key
causes preparation to be regenerated. Shared hosted-cache integration is slice C.

A hit authenticates the receipt, verifies every bounded member and copies the
verified bytes to the new run's private workspace, then runs the existing plan
reader against current source/compiler identities. Source and trust identities
are checked during collection and again before reporting. Every execution,
assertion, counter, session nonce, aggregation and report remains fresh. No
execution data is stored in the cache. Incomplete, corrupt, untrusted or stale
entries log a rebuild reason. Publication uses a private staging directory and
atomic rename; failure to save a valid fresh plan does not invalidate that run.
Concurrent complete publication never mixes members between generations.

Cache entries are disposable and remain until removed; `make clean` removes the
default payload and trust directories. An explicitly selected payload directory
is caller-managed. This slice changes coverage tooling only, not compiler or VM
interfaces, the selected corpus, or coverage attribution semantics.

### Slice B verification, 27 October 2026

PR#644 implements local reuse. Full `make check` passed in 819 seconds. The
changed `make source-coverage-report-tests` target passed, including both 388-item
parity paths and the real dependency fixture. All 55 focused cache, reader,
manifest, policy, routing and release-gate tests passed. Hosted Linux/macOS checks
also passed for the implementation commit.

Full-corpus local evidence at `f3e992b2d73cc4784cf0f0503c5f3936dc745b12`:

| Workload | Seconds | Cache hits / regenerated plans | Collector peak RSS (MiB) | Largest subprocess peak RSS (MiB) |
| --- | ---: | ---: | ---: | ---: |
| Cold population | 971.80 | 0 / 29 | 947.63 | 1040.44 |
| Unchanged repeat 1 | 108.48 | 29 / 0 | 1132.69 | 333.11 |
| Unchanged repeat 2 | 110.36 | 29 / 0 | 1126.33 | 330.66 |
| Unchanged repeat 3 | 107.56 | 29 / 0 | 1128.55 | 330.92 |
| Reporting-only change | 107.84 | 29 / 0 | 1134.05 | 333.28 |

All five collectors completed and passed integrity checks. Every run executed
31 sessions; all 155 nonces are distinct. Files' source hashes, line/function/
branch states, counters, exclusions and metrics match exactly across these runs,
as do all artifact and inventory identities. The compiler, VM and measured
source identities match the earlier uncached baseline. Temporary presentation
edits were restored after measurement.

The three-repeat median is 108.48 seconds, 88.45% below the earlier unchanged
939.13-second sample. The reporting-only sample is 88.75% below its earlier
958.23-second sample. The cold sample is 1.71% above the earlier 955.43-second
cold sample. Hits perform no exporter compilation or plan generation; roughly
99–102 seconds of fresh execution remain, and plan validation takes about two
seconds. These are strong local results, not the full matched-median acceptance
specified for slice C: the earlier uncached cases have only one sample each,
and cold/dependency/reporting acceptance still needs matched repeats.

The profiler now records the collector's own peak RSS as well as subprocess
peaks. Cached runs retain decoded plans and verification/copy buffers in Node;
the measured collector peak is about 1.10 GiB, versus 0.93 GiB in the new cold
sample. Subprocess peaks fall because native plan generation is absent. These
are separate process peaks, not simultaneous total memory, and the earlier
baseline did not measure collector RSS. No overall memory improvement or
regression against the old collector can therefore be established from these
numbers. Slice C owns the matched memory comparison alongside timing acceptance;
retain current file/count bounds and explicitly assess any material increase.

Slice B's local implementation is verified. Slice C remains: trusted hosted
persistence, three matched samples per comparison case across all four workloads,
cache invalidation/recovery replay, hosted provenance/publication evidence and
final operating acceptance. No release-completion credit or hosted speed claim
is assigned from these local measurements alone.

## Slice C: hosted preparation and matched acceptance

Implementation is in progress. Hosted reuse uses a separately named
`source-preparation-INPUT_IDENTITY` artifact retained for 30 days, so weekly runs
can reuse preparation without depending on the ordinary cache's one-week idle
retention. The name binds the full local context and hosted importer version.
Selection examines at most 20 matching artifacts and accepts only an unexpired,
bounded artifact from a completed successful `source-coverage.yml` dispatch on
this repository's `next`, with matching producer commit and repository identity.
The workflow downloads the selected immutable artifact ID from that exact run;
a transport digest mismatch fails the download. Selection or download failure
falls back to fresh preparation, without skipping any assertions.

GitHub's authenticated artifact service and the trusted core workflow establish
remote provenance. Downloaded receipts do not establish their own authority.
Only after successful transport verification does the hosted importer validate
bounded regular members, exact input context, names, sizes and hashes, and pass
copied bytes through the existing plan reader. The local cache then seals these
copies under a newly generated local key. No prior runner's key is uploaded or
restored, and no new repository secret is needed. An invalid import discards the
partially imported local set and regenerates. The trusted repository/workflow
maintainers and runner user remain the trust boundary; this does not defend
against a maintainer deliberately changing the trusted workflow.

The archive is limited to 64 preparation slots, at most 259 members per slot,
existing per-member reader bounds and 2 GiB of imported members. These are
transport limits, not increased compiler/inventory limits. The current corpus
needs 29 slots. Archive expiration, changed sources/toolchain/roots/importer,
failed provenance or corruption produces a cold run. Each successful collection
retains its current preparation separately from the static
`source-coverage-RUN_ID` report. The coverage publisher continues to select only
that exact static report name; executable preparation is never published as a
coverage website asset. Failure to retain preparation is visible but does not
invalidate a successful report.

Remaining acceptance: focused workflow and real import/execution replay;
three matched samples per workload with memory comparison; hosted cold and warm
runs with fresh nonces; publication boundary verification; final operating review.
The release summary retains partial/unquantified status until those checks pass.

Initial slice C verification: all 77 focused reader/cache/hosted/manifest/policy/
routing/release-gate tests pass, as do CI routing, workflow YAML/layout and docs
checks. `make source-coverage-report-tests` passes both 388-item parity paths and
the real dependency/import fixture with four distinct fresh sessions. These
changes are confined to hosted coverage tooling, its CI validation target and
tests; compiler/runtime behavior and the local collector are unchanged.
Existing baseline compiler, VM and all measured source hashes match the current
checkout exactly, and the local cache implementation is unchanged from PR#644.
Existing valid samples will be retained when completing the missing matched
repeats; the earlier profiler's missing collector RSS remains explicitly unknown.
No website changes are needed: static report format and publisher selection are
unchanged. Hosted execution and final performance acceptance remain pending.
