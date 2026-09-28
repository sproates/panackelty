# Validation profiling baseline

This report measures validation performance for the self-hosted toolchain.
No assertions, validation stages or timing budgets are removed or relaxed.
See [the reproduction procedure](README.md#detailed-validation-profiling).

## Pipeline critical-path improvements — 2026-09-28

The final implementation at `8934654` partitions sanitizer work
into VM contracts, ordinary oracle programs and the nested functional runner.
Compiler jobs run the harness alongside compiler probes, with a three-worker
budget (one plus two); long compiler probes start first. Runtime validation
uses two workers to overlap the native corpus with independent host/bytecode
probes. Native conformance runs complete programs through two isolated workers,
keeping each program's source/compile/bytecode assertions together. Bootstrap
overlaps the ordinary fixed-point check with the isolated seed-refresh proof,
retaining separate stages, hashes and comparisons. The macOS matrix retains
five jobs, avoiding an extra runner queue.
Distribution builds own their temporary executable and verify that the shared
VM inode and bytes remain unchanged, keeping compiler commands safe during
overlap. Every partition uses fresh native builds; no persistent cache or transferred
test result is used. The complete standalone commands retain all proofs.

The baseline sample comprises runs [36253029490](https://github.com/sproates/panackelty/actions/runs/36253029490),
[36252658731](https://github.com/sproates/panackelty/actions/runs/36252658731) and
[36252221963 attempt 2](https://github.com/sproates/panackelty/actions/runs/36252221963/attempts/2).
Their median required-check completion was **2m28s**, with a range of
2m27s–2m31s; median summed job duration was
19m45s. The table contains every attempt of the final source revision,
including any timing misses. All runs use the full PR merge-ref route against
`main`, with both packaging platforms, sanitizers, coverage and stable gates.

| Cold full run | All required checks complete | Summed job duration | Result |
| --- | ---: | ---: | --- |
| [Attempt 1](https://github.com/sproates/panackelty/actions/runs/36390450065/attempts/1) | 2m03s | 16m40s | passed |
| [Attempt 2](https://github.com/sproates/panackelty/actions/runs/36390450065/attempts/2) | 1m42s | 16m16s | passed |
| [Attempt 3](https://github.com/sproates/panackelty/actions/runs/36390450065/attempts/3) | 1m43s | 16m30s | passed |
| [Attempt 4](https://github.com/sproates/panackelty/actions/runs/36390450065/attempts/4) | 1m44s | 16m36s | passed |
| [Attempt 5](https://github.com/sproates/panackelty/actions/runs/36390450065/attempts/5) | 1m42s | 16m14s | passed |

Median completion is **1m43s**, a **30.4% reduction**. The
slowest run is 2m03s;
4/5 runs finished below two minutes.
Median summed job duration is **16m30s**, a 16.5% reduction.
These are elapsed runner durations, not CPU consumption or billing estimates;
platform multipliers and rounding are excluded.

Elapsed time runs from each attempt's `run_started_at` to the last job's
`completed_at`, including classification, runner queue/setup, uploads and final
required-result gates. Initial attempts start at workflow creation; reruns use
their new attempt start. Workflow `updated_at` can include later bookkeeping and
is not the endpoint. Hosted runner availability can still cause slower outliers;
the target is an observed operating result, not a guarantee of queue latency.

All 1,296 baseline PASS observations on each Linux validation path and 1,294 on
macOS remain, including multiplicity. Three additional harness observations cover
sanitizer partition equivalence, suite concurrency/failure propagation and
shared-executable isolation. Temporary workspace names are normalized before
comparing PASS observations; labels and multiplicity remain unchanged. The
sanitizer VM and coverage paths retain all 174 native PASS observations. The
oracle partition controls compare every selected compile, verify and execute
operation against the standalone sequence and inject each failure category.
The suite scheduler's FIFO controls verify overlap, bounded worker allocation,
serial behavior, invalid input rejection and failures in either branch.

The 2m03s outlier was held up by the Linux runtime packaging job: it started
52 seconds after the attempt began and then ran for 63 seconds, with the final
gate completing at 123 seconds. Four runs finished below two minutes. Keep
queue latency visible; this sample establishes a median, not an every-run cap.

The final native coverage summary is byte-for-byte identical to the baseline:
86.91% lines, 80.09% branches and 100% functions. Both platforms retain all
46 source conformance observations and 92 compile/bytecode observations,
including multiplicity. Negative fixtures, CLI contracts, archive checks and
both bootstrap proofs pass; no source, bytecode or instrumentation path was
substituted with a saved result.

The final clean local `make check` passed in 117 seconds (unit 77s,
functional 1s, bootstrap 24s, quick start 1s, with native setup included
in the total). The functional phase still verifies its session-local captured
runner report; the complete runner executes in the native corpus. Complete
native conformance, a fresh standalone bootstrap proof and the complete
standalone sanitizer sequence also passed during implementation. The unit-phase
warning remains; this work does not claim the separate focused-VM 15-second
target has been met.

### Earlier measurements and the shared-executable correction

An earlier [five-run sample](https://github.com/sproates/panackelty/actions/runs/36386916198)
before the conformance/bootstrap overlap took 112, 126, 129, 126 and 120 seconds
(median 126). Those misses motivated the remaining scheduling changes.
The next [sample at `88493f2`](https://github.com/sproates/panackelty/actions/runs/36388078564)
took 143, 105, 115, 103 and 113 seconds (median 113); its 143-second outlier
included a macOS compiler runner starting 32 seconds later than its peers.

The subsequent [documentation-head run](https://github.com/sproates/panackelty/actions/runs/36389380876)
failed five Linux compiler integration assertions. Distribution tests rebuilt
the root VM while compiler commands used it, making that overlap unsafe.
Distribution builds now use a copied executable in their temporary checkout;
a regression checks the shared VM inode and bytes remain unchanged. Reinstating
the root archive build makes that regression fail. Compiler integration failures
now include the actual exit status, signal and streams. The final five-run table
above measures the corrected source revision and replaces those earlier
samples as the merge evidence.

## Concurrent CI suites — 2026-09-26

The baseline main run `36249712860` at `9f0da42` completed in 408 seconds
(6m48s), including startup and result gates. Its summed job durations were
1,145 seconds (19m05s of runner time, before billing multipliers/rounding).
The Linux/macOS package commands took 377/345 seconds: their complete checks
took 202/189 seconds, followed by package validation taking 175/155 seconds.
That second pass repeated bootstrap for 46/38 seconds. Full Ubuntu validation
also sequenced project checks (194s), sanitizers (117s) and coverage (58s,
plus 10s tool installation).

CI now partitions shared canonical targets into compiler/harness,
runtime/functional and bootstrap suites. Both packaging platforms add
source and bytecode conformance suites, and Ubuntu runs sanitizers and coverage
independently. Bytecode conformance also builds and checks the archive.
Each suite starts in a fresh checkout; no persistent cache or transferred test
result is required. Bootstrap and seed-refresh independence remain intact.
Only the successful runner observation within runtime/functional is shared.
The standalone full commands and their warning budgets remain available.

The initial partitioned hosted runs passed in 166s and 185s, with 1,148s and
1,214s summed runner time. The second run exceeded the three-minute target
because macOS conformance took 157s. Source and bytecode conformance now run
in separate jobs, retaining both complete executions without sharing reports.

The final code at `71bf486` passed two complete cold runs, including both
platforms, sanitizers, coverage and stable gates. No Actions cache was restored.
Elapsed time includes classification, runner startup/queue delays and result
gates; summed job duration counts parallel runner time separately.

| Run | Elapsed | Summed runner time | macOS portion |
| --- | ---: | ---: | ---: |
| [Baseline](https://github.com/sproates/panackelty/actions/runs/36249712860) | 6m48s | 19m05s | 5m53s |
| [Final, attempt 1](https://github.com/sproates/panackelty/actions/runs/36252221963/attempts/1) | 2m20s | 20m41s | 7m10s |
| [Final, attempt 2](https://github.com/sproates/panackelty/actions/runs/36252221963/attempts/2) | 2m28s | 19m45s | 6m23s |

This is a 64–66% elapsed reduction with 3–8% more raw runner time on these
observations. Raw durations exclude billing rounding and platform multipliers;
they are not a billing estimate. Conformance jobs took 61–78s. Sanitizers took
100–127s and are the main remaining execution bottleneck; one Linux bootstrap
job also experienced 36s more startup delay than its peers in attempt 1.
At that point the two-minute stretch goal remained open; the September 28
measurements above supersede that pipeline baseline.

Both Linux paths preserve all 1,296 baseline `PASS` observations and macOS
preserves all 1,294, including multiplicity and normalizing temporary paths.
The complete hosted native coverage summary is byte-identical to the baseline:
86.91% lines, 80.09% branches and 100% functions. Dispatch/failure controls and
source/bytecode partition equivalence tests also pass.

The final clean local macOS `make check` passed in 131 seconds; an earlier
complete isolated conformance/archive run took 76 seconds. Serial local
validation still exceeds its 120-second budget. Native conformance now records
each source, compile and bytecode step to expose its remaining cost.

## Reuse and bounded workers — 2026-09-26

Same local arm64 host, macOS 26.5, Apple Clang 21.0.0, default `-O2`.
The before revision is `8cd16e2`; after measurements include this change. No
other builds ran concurrently with the timed checks. These are local wall-clock
observations, not a cross-platform speed claim. The two-worker setting remains
the default; four workers provided only a small further improvement.

| Check | Before | After | Budget |
| --- | ---: | ---: | ---: |
| Clean `make check`, two workers after | 176s | 127s | 120s |
| Clean `make check`, four workers after | 176s | 123s | 120s |
| Cached `make check-compiler` | 56s | 15s | 15s |
| Cached `make check-bytecode` | 11s | 2s | 15s |
| Cached `make check-vm` | 37s | 28s | 15s |

Focused measurements run compiler, bytecode and VM checks in that order after
the complete check, with no source edits between them. Before measurements
reuse native outputs; after measurements also reuse compiled probes. Earlier
after runs measured 14/2/27 seconds, so the compiler target has little headroom.

The two-worker clean run reduced time by about 28%. Unit time changed from
106s to 83s; functional time from 27s to 1s; bootstrap remained 31s. These phase
figures reflect work sharing: the oracle smoke executes the real full runner
once, then the functional phase checks that successful observation through its
source and bytecode smoke modes. Compiler stage 2 is also prepared before the
oracle corpus and shared. No test result survives the check session.

Compiled probes reuse bytecode only when all source/toolchain inputs match;
every invocation executes the tests. Invalidating a key causes recompilation,
so cached timings are not a promise that arbitrary source edits finish equally
quickly. The shell harness reuses its compiled runner across failure scenarios,
and the native oracle compiles its bounded command supervisor once per run.
Independent probes use a bounded worker pool with deterministic output.

All 1,276 baseline `PASS` observations remain (normalizing temporary workspace
names), with 18 additional regression observations. Native LLVM coverage was
run separately on both revisions: the entire per-file summary is identical,
including 86.97% line, 80.29% branch and 100% function coverage.
AddressSanitizer/UndefinedBehaviorSanitizer validation also passed. Coverage builds
retain separate selected-VM compilation and execution; no corpus was removed.

Remaining work: the clean total and full-unit warning budgets are still
exceeded. The standalone VM target must execute its own complete oracle runner;
its source compilation and subprocess work remain a priority. The independent
seed-refresh staging proof remains intact. Cross-platform initial and cached
profiles are collected by the profiling workflow; assess hosted results
separately from these local observations.

## Earlier profiling baseline: measurement boundaries

The base revision is `9975186` (alpha.9), plus this profiling change. Clean
checks include native compilation, unit and functional suites, fixed-point
bootstrap, seed refresh and quick-start gates. Warm checks start with ordinary
native prerequisites built. Package validation is recorded separately from the
clean check, and full CI duration also includes setup, sanitizers and coverage.
Parent observations are inclusive and cannot be added to their children.

The failure-injection seed-refresh check and native staging proof test different
contracts. Their separate labels are not evidence that either can be removed.
Source-probe measurements include compilation and execution; a slow probe needs
further measurement before attributing its cost solely to compilation.

## Local clean baseline — 2026-09-26

Linux x86-64 workspace, Ubuntu GCC 13.3.0, default `-O2`, serial `make check`
after `make clean`. The run passed all unit, functional, bootstrap and packaged
quick-start checks. This is one instrumented observation, not a speed claim or
a comparison with different CI hardware. The earlier 239-second baseline is
historical context. Whole-second values include profiling overhead.

| Phase | Seconds | Budget | Result |
| --- | ---: | ---: | --- |
| Complete check | 240 | 120 | Passed; timing warning |
| Unit phase | 146 | 15 | Passed; timing warning |
| Functional phase (including stage-2 build) | 37 | 75 | Passed |
| Bootstrap (including native seed refresh) | 52 | 60 | Passed |

The remaining approximately five seconds include native prerequisites, policy
checks and packaging/quick-start overhead. Rounding and nesting prevent exact
attribution by adding individual observations.

| Observation | Seconds | Included in |
| --- | ---: | --- |
| Native oracle contracts | 44 | Unit phase |
| Native seed-refresh staging proof | 38 | Bootstrap |
| Compiler integration probe | 26 | Unit phase |
| Functional fixture runner | 24 | Functional phase |
| Development harness | 22 | Unit phase |
| Harness fixture-runner contracts | 14 | Development harness |
| Compiler contracts probe | 14 | Unit phase |
| Bytecode probe | 13 | Unit phase |
| Stage-2 compiler build | 13 | Functional phase |
| Stage-3 compiler build | 13 | Bootstrap |

## Local warm component baselines

Same workspace and flags, immediately after the clean run, in compiler,
bytecode, VM order. All three targets passed; all exceeded the 15-second target.
These top-level measurements include Make prerequisite checks. Native outputs
were already built; source probes still compile their inputs as usual.

| Target | Seconds |
| --- | ---: |
| `make check-compiler` | 83 |
| `make check-bytecode` | 16 |
| `make check-vm` | 50 |

## Final instrumentation verification

The final full `make check` also passed with existing build outputs. Its finer
oracle observations measured **36 seconds in
`tests/functional/cases/runner_smoke/main.panack`**, within a 43-second oracle
suite. That program launches the full functional runner when no captured report
is supplied. This is evidence for investigating repeated runner work, not
permission to remove the corpus or its instrumented execution.

## Follow-up investigations

1. Break down native oracle compilation and bounded subprocess calls. Additional
   `oracle/compile/...`, `oracle/run/...` and `oracle/command/...` labels support this investigation;
   they were added after the initial clean observation above. Evaluate reuse of
   the command supervisor and compatible compiled fixtures, retaining every
   byte-exact observation and instrumented run.
2. Separate compilation from execution inside the compiler integration/contracts
   and bytecode probes before choosing caching or algorithm changes. Their
   current rows include both.
3. Inspect repeated compiler work in the 38-second native seed-refresh proof.
   Preserve its isolated staging, hashes, fixed-point comparisons, failure
   controls and publication guarantees; ordinary bootstrap artifacts cannot
   simply substitute for these checks.
4. Investigate fixture-runner subprocess and compilation costs in both the
   harness and functional suite, retaining independent failure-injection cases.

The warm compiler/bytecode/VM baseline workflow and the Check packaging matrix
provide independent Linux/macOS evidence. Their artifacts identify each run;
focused profiles include source and runner/compiler metadata. Hosted results
must be assessed separately from this local observation. Timing budget warnings
remain active until the optimization work demonstrates the existing targets.

Rows with nonzero status can be expected failure-injection commands nested in a
successful harness group; use the outer check result to determine suite success.
Do not interpret every negative-control observation as a CI failure.
