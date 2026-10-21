[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Standing performance engineering — ongoing

<a id="rm-123"></a>

**RM#123: Standing performance engineering** ·
[GI#263: Standing performance engineering](https://github.com/sproates/panackelty/issues/263).

**User decision, 2026-10-04:** performance is a continuing programme and a
cross-cutting requirement of substantive deliveries, not an occasional backlog
reminder. Maintain the scorecard below through deliveries, releases and planning
reports under the [performance review process](../ROADMAP_PROCESS.md#performance-impact-and-regression-decisions).
The standing programme has no lifetime completion percentage. The finite core
establishment tranche below has its own stable denominator.

GI#518 delivery observation (20 October 2026): the public API migration passed
canonical `make check` in 509 seconds on the macOS developer checkout, with warm
build artifacts and the configured three unit workers/one bootstrap worker.
The earlier runtime slice took 629 seconds, but these are different workloads,
not a paired speedup measurement. New acceptance exercises 1 MiB byte boundaries,
16 connections/four requests and real ten-second/one-second deadlines. The final
acceptance timing is retained in its
[register](../../design/routed-http-server/acceptance-register.md). These checks
establish bounded behavior, not throughput, resident-memory or latency baselines.
No new numeric performance gate is introduced. GI#106 reassessment and RM#56
representative runtime measurements retain their existing ownership and next
actions; this feature delivery does not close them.

The modules programme remains the principal feature initiative;
its checked declaration/signature identity slice merged in PR#275 as `0bcf092`;
bounded body identities merged in PR#284 as `9f8f8a0`. Ordinary expressions and
control flow merged in PR#297 as `1ffd023`; aggregate, pattern and generic
inference complete slice 2. [PR#321](https://github.com/sproates/panackelty/pull/321)
and [PR#326](https://github.com/sproates/panackelty/pull/326) complete slice 3 on
`next`, and [PR#327](https://github.com/sproates/panackelty/pull/327) completes
slice 4. [PR#329](https://github.com/sproates/panackelty/pull/329) completes
slice 5 effects/await on `next`; PR#349 now integrates and completes all seven
slice-6 identity-emission and standalone-loading gates on `next`. PR#351–PR#354
deliver Slice 7's retained pattern, declaration, local-annotation and explicit
call-type spans. This delivery closes Slice 7 acceptance with direct/receiver
under-arity diagnostic checks and the existing source-map identity/staleness
matrix. Slice 8 now has a coordinated compiler and standard-library namespace
migration, refreshed seed fixed points, and passing native, browser-WASI and
installed-package acceptance. Compiler APIs and selected imports cover the
compiler/tooling graph; standard-library modules publish documented APIs. The
public namespace subset passes source and saved-v9 execution. Project
compilation always uses checked identity emission; the flat compatibility path
has been removed. The previously deferred Map/Set receiver inference case
passes identity checking. An exhaustive identity-only check passes all 59
functional, compiler-contract, and top-level example roots. PR#358 merged this
acceptance to `next`; the 59-root audit now runs in the functional phase of every
canonical `make check`. Slice 8 receives its full scope credit on that merge. See
the current Slice 8 assessment and validation profile.
The matched clean canonical checks took
202s on base `d5dd68a` and 204s on the candidate; a single pair does not establish
causation. Both exceed the 120s budget. The slice 6 acceptance disposition is recorded in the
[slice 6 validation profile](../../../tests/VALIDATION_PROFILE.md#namespace-identity-emission-and-loading--2026-10-08).
Compiler understanding
remains paused. The selected build/validation baseline is established below;
no optimisation, runtime/resource benchmark execution, backend investigation or
automation starts with it. Remaining baseline scope requires separately selected
delivery. Website performance work remains
deferred, and ordinary website content updates wait for the next supporting
release; existing publication correctness defects retain their separate treatment.

### Continuous scorecard

| Area / existing work | Established target and current evidence | State, responsible role and next action |
| --- | --- | --- |
| Developer/build feedback · [RM#28: Incremental and modular builds](independent-website-publishing.md#rm-28) / [GI#106](https://github.com/sproates/panackelty/issues/106) | 2026-10-12 macOS arm64: clean-check median 608.81s at two workers; warm `check-compiler` median 114.44s at two workers and 105s at four; one fresh-cache probe 0.30s. PR #389 and same-tree post-merge Check took 8m11s and 10m14s; both passed. Earlier 120s/15s/75s/60s figures remain historical. A reused-tree `make check` for the diagnostics slice passed in 413s; this is not a matched performance comparison. | The [local/CI inventory](../../../tests/VALIDATION_PROFILE.md#gi106-workflow-inventory) and [representative change-impact map](../../../tests/VALIDATION_PROFILE.md#gi106-change-impact-map) are recorded. All three earlier `next` push failures followed green PR Checks on identical product trees. Paired jobs also used identical runner and base-image versions within each OS; Linux regions differed. This rules out image-version drift as an evident explanation, but the historical failures remain unexplained. PR #392's first hosted run exposed a test-ordering mistake: the checkpoint check ran in the compiler harness before the VM test binary was built. Moving it to the VM contract runner fixed the order; the corrected local VM suite and compiler harness passed. Corrected head `0609d09` passed all checks in [PR run #38224322156](https://github.com/sproates/panackelty/actions/runs/38224322156) (8m45s); merged as `6597c16`, whose [post-merge run #38225333320](https://github.com/sproates/panackelty/actions/runs/38225333320) passed (9m24s). That historical disposition is superseded by the owner-directed cadence change below. PR #394 kept all four compiler experiments on every PR in both OS lanes and skipped their exact-tree repeat on `push`; the [first post-merge run](https://github.com/sproates/panackelty/actions/runs/38228244756) confirmed they were skipped in both compiler lanes while the product gates passed. The PR run took 8m34 and the post-merge run 8m30, so this pair does not demonstrate a total wall-time improvement. It does remove measured experiment work from the repeated post-merge lanes without reducing product gates. PR #389 corrected the isolated measurement fixture and documented a direct single-probe command; PR #390 records paired CI evidence. Runtime/compiler behavior and required product coverage are unchanged. The owner accepted the current workflow for Alpha.13 on 2026-10-15; That alpha.13 gate remains accepted. On 2026-10-20 the owner reopened the work; the [alpha.15 milestone](next-release--agreed-scope-2026-10-06.md#alpha15-validation-speed) now requires measured local/CI improvements with preserved assertions and owner acceptance. Implementation has not started; the delivery maintainer owns the baseline, bounded fixes and acceptance evidence. |
| Compiler/runtime/resources · [RM#56: Performance baselines](groomed-development-and-engineering-work.md#rm-56) / [GI#141](https://github.com/sproates/panackelty/issues/141) | Prior profiles and source assessment exist; no maintained representative compile/run, startup, throughput/latency, memory or artifact-size baseline is accepted. No general numeric runtime budget is established. | Planned baseline; execution not started. Baseline delivery owner to be assigned when selected. Next: agree representative correctness-checked workloads, measurements, repeats and noise calibration before thresholds or optimisation. |
| Website preview service · [RM#101](current-status-and-grooming-2026-10-01.md#rm-101) / [GI#162](https://github.com/sproates/panackelty/issues/162) | Local existing 52-file assembly matched byte-for-byte. Saved-bytecode startup 16–18 ms; small requests 7.0–7.9 ms versus Node 0.15–0.21 ms; largest 706,607-byte requests 11.1–12.9 ms versus 0.27–0.34 ms. Two observations per mode, no general throughput or memory-growth claim. | Material relative latency regression. Accepted for bounded development previews after the owner inspected the responsive site positively on 2026-10-15; installation compiles once to avoid 1.34s source startup. GI#162 delivery maintainer will revisit if latency obstructs use. See [evidence and limits](../../design/GI162_STATIC_HTTP_PLAN.md#performance-evidence-and-proposed-disposition). |
| Website delivery · [RM#8: Website CI follow-ups](current-status-and-grooming-2026-10-01.md#rm-8) / [GI#187](https://github.com/sproates/panackelty-website/issues/3) | Existing website validation 120s / merge-to-live 180s targets remain as historically scoped; timing acceptance is incomplete. They are not native/runtime targets. | Deferred by user decision; website maintainer retains the record. No new trials or scheduling. Revisit on the existing user-request/correctness/staleness triggers; release preparation reviews the deferred record without automatically restarting it. |

**GI#106 CI configuration route (PR #414):** workflow and explicitly listed CI
routing/gate/contract-only changes take focused syntax and contract checks;
product, mixed, unknown and other shared changes retain full validation. The
first PR run completed in 40s and its post-merge `next` push in 36s. Both skipped
the product suites, package matrix, macOS compiler lane and source-coverage
collection while passing workflow validation and stable result gates. This
removes the unrelated 17m38 source-coverage job observed on workflow-only PR
#413. The 17m38 figure is one job, not a matched whole-run baseline, so these
observations demonstrate the new route's duration rather than a controlled
end-to-end speedup. GI#106 remains open for owner acceptance.

**GI#106 performance disposition update (2026-10-10):** after the clean
two/four-worker comparison (530.32s/379s), the exact merged alpha.12 candidate
passed a clean canonical `make check` on the macOS arm64 development host in
375s (unit 200s, functional 12s, bootstrap 133s, release smoke 1s, quick start
1s). The regular Linux/macOS hosted Check workflow passed for the same commit;
the separate clean-hosted focused profile measured cached `check-compiler` at
127s on Linux and 110s on macOS, against 15s, with cached `check-vm` at 145s and
114s. These component timings are not a canonical full-check measurement; all
raw observations and comparability limits are in the
[validation profile](../../../tests/VALIDATION_PROFILE.md#alpha12-performance-close-out--2026-10-10).

**Owner decision, 2026-10-10:** accept the known developer-validation latency as
a disclosed alpha.12 limitation following the evidence-backed performance
improvements in PR #363. Keep the 120s/15s/60s targets and all coverage
unchanged; no further optimization is an alpha.12 prerequisite. This decision
accepts only the validation-time trade-off; all three alpha.12 feature gates,
promotion, package/release validation and explicit release approval still
apply. GI#106 remains open. Revisit before the next compiler-heavy delivery or
if comparable evidence shows a material timing change.

**P3 nested graph safety test slice (2026-10-11):** canonical `make check`
passed in 402s on the reused macOS arm64 build tree (unit 225s, functional 13s,
bootstrap 135s, release smoke 4s, quick start under 1s). Functional, full-check,
and bootstrap budgets were respectively 13/75s, 402/120s, and 135/60s. The
aggregate unit phase is not the focused 15s workload. Compared with the prior
397s P3 overlap run, this single unmatched run is 5s longer; it does not show
that the added fixture caused a regression. The compiler/runtime source is
unchanged, and no application runtime benchmark was run. See the
[P3 graph safety test profile](../../../tests/VALIDATION_PROFILE.md#p3-nested-graph-safety-tests--2026-10-11).

**Owner direction, 2026-10-20:** validation latency is again an explicit delivery
concern. The alpha.13 acceptance remains valid; the new
[alpha.15 GI#106 milestone](next-release--agreed-scope-2026-10-06.md#alpha15-validation-speed)
requires implemented local/CI feedback improvements, comparable measurements and
owner acceptance while preserving existing assertions and supporting stronger
coverage. Status: planned, implementation not started. The delivery maintainer's
next action when selected is a representative baseline and critical-path profile,
then bounded fixes and regression evidence. Review unresolved costs at milestone
acceptance; no replacement numeric budget is selected by this roadmap update.

**Owner direction, 2026-10-12:** the former 120s clean-check, 15s focused-check,
75s functional and 60s bootstrap figures are historical comparisons, not current
acceptance gates. Reassess the full developer build/test workflow, what runs and when, how
tests map to possible regressions across component boundaries, and how much time
is active execution versus queueing or other waiting. Offer replacement NFRs and
improvements for the user's decision. The only acceptance criterion for GI#106
is that the user says they are happy with the result; the issue remains open
until then.

**Selected candidates and stdlib direction, 2026-10-12:** the user selected
dependency-aware compiled-probe reuse and a separate-compilation design, while
keeping every test assertion and the full validation gates. The local reuse
pilot's synthetic unrelated-edit comparison is recorded in the
[GI#106 findings](../../../tests/VALIDATION_PROFILE.md#gi106-findings-and-options): the
compiler-contract probe was reused in 0.04–0.05s instead of recompiling in
18.32–18.48s. This is not a representative bug-fix result or clean-check speedup.
The host-enabled local `make check` passed in 421s on a reused build tree; its
historical budget warnings are not current acceptance gates. A temporary-copy
red/green probe of the 42-assertion stdlib runner was also recorded:
deliberately breaking `path.display` failed in 0.598s, restoring it passed in
0.276s, and an unchanged repeat passed in 0.037s; changing the imported stdlib
dependency invalidated the bytecode cache. This confirms a focused stdlib edit
reruns the affected probe, but it is a synthetic fault cycle, not timing from
the merged migration or a broader-check result. The owner-directed stdlib API
replay is complete in merged PR#404. On the recorded Mac M5 Pro, the restored
baseline and candidate clean checks passed in 602s and 619s respectively; this
single pair cannot attribute the 17s difference or establish a general
regression or speedup. The exact-head hosted PR Check took 10m05s and the
post-merge Check 7m17s. The [replay evidence](../../../tests/VALIDATION_PROFILE.md#owner-approved-p4-replay-evidence)
records byte-identical focused probes, migration size, timings and limits. Use
the boundary evidence to review the separate-compilation design. No replacement
NFRs are set and GI#106 remains open until the user is happy.

The 2026-10-08 text-intrinsic bootstrap prerequisite passed final-tree canonical
`make check` in 205s (functional 4s, bootstrap 20s), above the 120s target.
This local observation establishes neither a regression nor a cause;
the existing GI#106/RM#123 disposition and next action remain unchanged. See the
[prerequisite validation record](../../../tests/VALIDATION_PROFILE.md#slice-8-text-intrinsic-bootstrap-prerequisite--2026-10-08).

The controlled baseline now establishes applicable full/focused observations; both
targets remain missed. Earlier PR#262 timings remain historical evidence, not a
retroactive rejection of its already authorised merge or proof of regression's cause.
The next critical-path/candidate comparison must retain the pinned workload evidence. Future affected
deliveries must present a mitigation, justified trade-off or owned remediation
proposal for explicit acceptance with merge approval; copying a warning is not acceptance. See
[the recorded measurements and limits](../../../tests/VALIDATION_PROFILE.md#namespace-raw-references-and-binding-resolution--2026-10-04).

The GI#131 feasibility delivery adds an isolated experiment, not production
instrumentation or a new canonical-check workload. Its
[cost evidence and validation limits](../../../tests/VALIDATION_PROFILE.md#source-coverage-feasibility--2026-10-05)
retain the existing GI#106 owner, targets and next action. The local two-worker
attempt failed at runner-smoke; the supported single-worker canonical retry
passed in 343s, still above the full budget. Hosted correctness evidence is
required in the delivery PR before integration.
Accept only the bounded experiment cost; no existing performance concern closes.

### Finite core establishment tranche

Provisional scope-based estimates established on 2026-10-04 total 100%. The larger shares
cover reproducible harness/workload design, repeated samples and resource evidence;
policy and independent acceptance are smaller. This denominator includes only the
initial core establishment, not endless maintenance or the explicitly deferred
website work retained above. Existing child outcomes are counted once, not again
under the standing parent. Later scope/weight changes require an explained revision.

| Task | Scope and status | Weight | Completion / earned contribution |
| --- | --- | ---: | ---: |
| <a id="rm-124"></a>**RM#124: Performance governance** | Done through PR#264: ownership, mandatory proportionate delivery assessment, scorecard and explicit regression disposition. | 20% | 100% / 20 pp |
| [RM#28: Incremental and modular builds](independent-website-publishing.md#rm-28), baseline slice · GI#106 | Done on this baseline delivery's merge: bounded isolated harness, three repeats of clean/full, warm/focused and unrelated/direct/transitive edit observations, pinned inputs and explicit budget disposition. | 30% | 100% on merge / 30 pp on merge |
| [RM#56: Performance baselines](groomed-development-and-engineering-work.md#rm-56) · GI#141 | Planned; execution not started. Correctness-checked compile/startup/run workloads, meaningful throughput/latency, memory and artifact-size evidence; calibrate noise before general thresholds. | 40% | 0% / 0 pp |
| <a id="rm-125"></a>**RM#125: Initial performance scorecard acceptance** | Planned; execution not started. Independent reproduction/review of both core baselines, qualified target applicability and initial scorecard acceptance. Requires RM#28/RM#56 baseline evidence. | 10% | 0% / 0 pp |
| **Total** | **Core establishment only** | **100%** | **50% on baseline merge; 20% accepted before it** |

RM#124 and RM#125 are scoped under GI#263; no duplicate implementation issues are
created. Governance earns 20 pp; this bounded build baseline adds 30 pp on merge.
This accepts measurement, not resolution of the measured performance concerns. Delivery owners maintain evidence
and unresolved actions until a named maintainer accepts handover; independent
reviewers assess measurements and dispositions, and the user selects priorities
and accepts trade-offs. The build baseline's owned remediation action is recorded
in the scorecard; runtime/resource measurements and combined initial acceptance
remain planned and require separately selected execution. No fixed-count review or scheduled automation is introduced.

Governance acceptance was recorded in PR#264. The build baseline adds repeated
correctness-checked evidence and independent review of scope, budget applicability
and the outstanding remediation disposition. Website impact: no public feature,
artifact, page or release pin changes; this is contributor tooling and policy.
