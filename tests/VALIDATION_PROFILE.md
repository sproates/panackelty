# Validation profiling baseline

## Async source-to-VM slice — 2026-09-29

Baseline: `30b584a`, preserved before implementation. Candidate: issue #102's
v9 compiler/VM with direct and indirect await and typed fake-read completions.
Both VMs use the same C compiler and `-O2`; measurements ran serially without
other validation jobs. One warm-up per variant preceded five alternating runtime
pairs and three alternating compiler pairs. These are local samples, not a
throughput or wall-clock responsiveness guarantee.

| Workload | Baseline median ms | Candidate median ms | Change |
| --- | ---: | ---: | ---: |
| calls | 155.406 | 163.327 | 5.10% |
| iteration | 109.534 | 106.031 | -3.20% |
| compiler | 17887.878 | 19064.506 | 6.58% |


Runtime sources are `tests/fixtures/execution/calls.panack` (Fibonacci 24,
expected stdout 46368) and `iteration.panack` (100,000 indirect calls, stdout
100000). Each compiler emitted its own supported-format artifact from the same
source; their execution semantics are unchanged. Time `VM run ARTIFACT` with a
monotonic clock around each child process and check stdout on every sample.
The compiler workload compiles the baseline checkout's complete compiler source
with each seed, keeping source and library inputs identical. Run with that
checkout as cwd and its absolute src/stdlib as PANACKELTY_STDLIB_PATH:
`VM run SEED compile src/compiler/main.panack -o OUTPUT`.

All three medians remain below the 10% investigation threshold. Extra effect
checks and metadata add work; these samples do not establish a significant
speedup for the small negative iteration difference. Validation's existing
15-second unit budget remains a separate non-blocking concern.

Raw measured milliseconds (warm-ups excluded):

```text
calls baseline: 173.992, 149.093, 155.406, 147.782, 161.293
calls candidate: 165.552, 156.086, 169.487, 163.327, 152.702
iteration baseline: 95.747, 95.601, 114.303, 128.285, 109.534
iteration candidate: 97.790, 106.031, 128.923, 104.847, 124.247
compiler baseline: 17928.145, 17887.878, 17859.829
compiler candidate: 19064.506, 19126.477, 19019.967
```

Correctness evidence is in the compiler effect contracts, public CLI async case,
fixed v9 malformed vectors, native async call matrix/completion tests and typed
allocation-failure sweeps. Real OS I/O, resource scopes and producer quiescence
remain outside this evidence. Full validation and hosted platform/sanitizer gates
remain required before merge; local command outcomes are reported in the PR.
The local full sanitizer invocation is blocked by LeakSanitizer failing to read
`/proc`. A supplemental ASan/UBSan VM run with `detect_leaks=0` passed 176 of
177 contracts; the native-module command exceeded its 20-second harness limit.
Running that instrumented module executable directly passed, including the new
async lifecycle tests. This is partial local evidence, not a substitute for the
unmodified hosted sanitizer gates.


This report measures validation performance for the self-hosted toolchain.
No assertions, validation stages or timing budgets are removed or relaxed.
See [the reproduction procedure](README.md#detailed-validation-profiling).

## Task/lifecycle feasibility — 2026-09-29

Baseline: merged resumable-execution PR #94 (`d6988b6`). This change adds an
internal fake-host session; ordinary CLI execution still uses the same synchronous
dispatcher, source syntax and bytecode v8. It is a lifecycle correctness experiment,
not a throughput or OS-I/O benchmark. Environment: macOS arm64, Apple Clang,
default `-O2`; sanitizer builds use `-O1 -g` with AddressSanitizer/UBSan.

The independent contracts cover nested scope joins, retained results, failure and
sibling cancellation, cancellation around wait/enqueue/delivery/join transitions,
virtual deadline inheritance, CPU-task fairness, bounded queues with retry, stale
operation generations, wrong-session ids and destruction while pending. A nested
bytecode call and an indirect builtin call both suspend while retaining frames.
Three verified corpus programs replay through queued acknowledgements and retain
the original independently expected stdout. The VM probe now has 177 assertions.

Allocation sweeps cover session/task creation, repeated completion, host failure,
queued cancellation and pending destruction, asserting balanced live allocations
and input references. The complete native fault suite reports 1,844 injected
allocation failures; this total includes existing VM/numeric/host contracts, not
1,844 new scheduler cases. Native unit, full sanitizer VM/oracle/runner suites and
final instrumented module contracts passed locally. Compiler and stdlib bootstrap
identity, ordinary source/bytecode functional behavior and packaged quick-start
remain part of canonical validation.

A clean development `make check` sample completed in 103s (unit 69s), below the
120s full-check budget but above the unit target of 15s. This sample preceded the
last focused admission/nested-call test refinements; final canonical validation is
required after those edits and recorded in the PR. The unit warning remains in
[the non-blocking backlog](../ROADMAP.md#keep-validation-within-development-budgets--non-blocking-backlog).
No earlier Linux measurement is directly comparable to this macOS sample.

Reproduce with `make clean && make check`, `make native-fault` and
`make native-sanitize`. `make native-unit` runs the direct lifecycle contracts;
`make native-vm-contracts` additionally runs the three queued-host corpus replays.

Limits: task slots/results remain reserved until session destruction; task and
ancestor scans are intentionally simple, with no production-scale performance
claim. Budgets bound bytecode dispatch only. The fixed service returns a typed
Void acknowledgement or static error and has no external producer. Real sockets,
producer quiescence, thread handoff, reusable task slots, arbitrary resources,
async finalisers and public syntax/ABI remain separate work. No successor stage
is authorised by these results. See the
[precise lifecycle contract](../src/vm/README.md#internal-task-lifecycle-experiment).

## Resumable VM feasibility — 2026-09-29

Baseline: `a66ef9a89c21eb39f1ed2268a068d6c87a6d6656` (merged design PR #92).
Candidate: this PR's owned-frame dispatcher and internal execution API, with
unchanged bytecode v8 and synchronous CLI. Measurements preceded PR review.
Environment: Linux x86-64, GCC 13.3.0, default `-O2`, shared development host.
These are small feasibility workloads, not a claim about all applications.

Each CLI workload received one warm-up per binary, then five baseline/candidate
pairs, alternating which binary ran first. An external monotonic subprocess
timer included launch, decoding, verification and execution. No other project
benchmark/build ran concurrently. Every sample checked successful exit, exact
stdout and empty stderr. Fixed bytecode files were shared between binaries.

| CLI workload | Baseline median ms (range) | Candidate median ms (range) | Change |
| --- | ---: | ---: | ---: |
| Recursive Fibonacci(24) | 88.930 (85.454–89.354) | 88.716 (86.675–90.399) | -0.24% |
| 100,000 indirect calls in a range loop | 61.783 (58.906–65.692) | 62.859 (57.151–65.337) | +1.74% |
| Compile the compiler using the v8 seed | 11902.091 (11811.118–12286.262) | 11961.911 (11804.149–12217.326) | +0.50% |

All three CLI medians are below the predeclared 10% investigation threshold;
the small differences do not establish a speedup or a statistically significant
regression. The same unit-test executable also ran the two runtime workloads
through its fake immediate host, with one warm-up and five samples per budget.
These budget samples were collected sequentially after the paired CLI trials.

| Workload | Budget 1 median ms | Budget 1,000 median ms | Budget 1 versus 1,000 |
| --- | ---: | ---: | ---: |
| Recursive Fibonacci(24) | 99.357 | 96.254 | +3.22% |
| 100,000 indirect calls in a range loop | 68.579 | 64.371 | +6.54% |

Budget 1 is about 11–12% slower than the baseline CLI. This crosses the
investigation threshold, but is not an isolated measurement of yield overhead:
the driver uses a different executable and captures printing in memory. Within
that same driver, returning after every instruction is 3–7% slower than after
1,000 instructions. Each return repeats the caller/advance/status checks; this
stress mode deliberately maximizes those transitions. Use larger budgets for
throughput experiments. No wall-clock responsiveness guarantee follows from an
instruction count: a pure builtin or trusted host callback may take arbitrarily
long. Pending I/O and scheduling remain future work.

### Reproduction and raw observations

Build baseline and candidate with the same `CC`/`CFLAGS` in separate checkouts.
Run `make native native-module-build` in the candidate. Compile the two sources
under `tests/fixtures/execution/` once using the preserved baseline VM:
`panack-vm run bootstrap/compiler-v8.bc compile SOURCE -o ARTIFACT`.
Set `PANACKELTY_STDLIB_PATH` to the absolute `src/stdlib` directory for compiler
commands. Time each binary's `run ARTIFACT` command in alternating pairs as
above; expected output is `46368` for calls and `100000` for iteration, each with
a final newline. The compiler workload is
`panack-vm run bootstrap/compiler-v8.bc compile src/compiler/main.panack -o OUTPUT`.
For budget trials use `build/vm/test_modules resume ARTIFACT 1` and then `1000`.
An external monotonic timer or `/usr/bin/time -p` suffices; the latter reports
coarser precision. The timing harness is not a project runtime/build dependency.

All recorded samples below are milliseconds, in observation order within each
series. They include slower samples; no outliers were removed.

| Workload | Mode | Five samples (ms) |
| --- | --- | --- |
| calls | before | 89.120 / 86.964 / 85.454 / 89.354 / 88.930 |
| calls | after | 90.399 / 89.132 / 88.716 / 86.675 / 87.553 |
| calls | budget1 | 98.179 / 99.357 / 104.798 / 99.055 / 103.443 |
| calls | budget1000 | 94.807 / 96.254 / 94.638 / 101.261 / 98.411 |
| iteration | before | 60.500 / 58.906 / 65.692 / 63.264 / 61.783 |
| iteration | after | 57.151 / 63.581 / 62.859 / 65.337 / 59.350 |
| iteration | budget1 | 65.871 / 69.522 / 68.579 / 66.937 / 73.256 |
| iteration | budget1000 | 62.555 / 64.371 / 64.772 / 67.353 / 62.624 |
| compiler | before | 11902.091 / 11892.323 / 11811.118 / 12286.262 / 12212.327 |
| compiler | after | 11804.149 / 11877.342 / 12131.195 / 12217.326 / 11961.911 |

Correctness evidence includes forced yields through direct/indirect calls and
array/bytes/range iteration, independent interleaved sessions, frame growth,
suspended destruction, sticky terminal states, fake-host re-entry rejection,
exit interception, unsupported-service traps, and allocation-failure cleanup.
The public CLI fixture also exercises 20,000 recursive calls. Allocation-failure
sweeps check 1,776 failures across the native suite.

Canonical `make check` passed in 150s (unit 105s, functional 1s, bootstrap 37s),
including release smoke and quick-start validation. Native prerequisites were
prepared immediately after `make clean`; their build time is outside that
150s. Existing full/unit budget warnings remain tracked in the roadmap.
Two earlier full attempts stopped with launch failures because the generated
`test_modules` executable had mode 0644; a clean rebuild produced mode 0755,
passed its direct contracts and passed the complete suite without source changes
or assertion overrides. The permission change's cause was not established;
local syscall tracing is also blocked by ptrace restrictions.

Local GCC LeakSanitizer cannot inspect `/proc/.../task` in this environment and
terminates with its ptrace limitation; Clang is not installed locally. This is
not a sanitizer pass. The existing unmodified hosted sanitizer, coverage and
cross-platform gates must pass before merging.

## Focused VM check investigation — 2026-09-28

Measured revision: `8e13d54` (merged PR #76). This investigation changes no
validation behavior or budget. It identifies persistent collection copying in
the compiler workload as the first optimisation candidate; it does not claim
that a focused check already meets 15 seconds.

### Repeated local measurements

Environment: macOS 26.5 arm64, Apple Clang 21.0.0, default `-O2`, two validation
workers. Commands ran sequentially with no other project benchmark running.
Native prerequisites were built once after `make clean` (2.52s). The initial
check had no probe/compiler cache. Subsequent unchanged checks kept that cache.
Each edited run appended a different comment to
`tests/runner/host_runtime_unit.panack`, then ran the complete check. Original
source bytes were restored afterwards. This measures a test-source edit, not a
native C edit or a compiler semantics change.

| Scenario | Full `make check-vm` elapsed seconds | Result |
| --- | ---: | --- |
| Native prepared, initially empty probe cache | 36.156 | passed |
| Unchanged rerun 1 | 26.219 | passed |
| Unchanged rerun 2 | 26.137 | passed |
| Unchanged rerun 3 | 26.325 | passed |
| Test-source edit 1 | 35.588 | passed |
| Test-source edit 2 | 37.051 | passed |
| Test-source edit 3 | 36.338 | passed |

The unchanged median is **26.219s**; the edited median is **36.338s**.
All seven runs preserve the same 275 visible PASS observations, including
multiplicity after normalizing temporary workspace names. The nested runner
also checks its complete expected 273-test report internally. Every run warns
about the unchanged 15-second focused-check budget.

Totals above use an external monotonic subprocess timer around `make check-vm`,
including prerequisite checks. Existing profile rows use whole wall-clock
seconds; these inclusive observations overlap and must not be added together.

| Profile label | Cached runs (seconds) | Edited runs (seconds) |
| --- | --- | --- |
| Native contracts, including bootstrap preparation | 24 / 23 / 24 | 32 / 33 / 32 |
| Native oracle, within native contracts | 22 / 22 / 23 | 23 / 23 / 22 |
| Nested functional runner, within native oracle | 17 / 17 / 17 | 17 / 18 / 18 |
| Stage-2 compiler preparation, within native contracts | 0 / 0 / 0 | 7 / 8 / 8 |

The probe fingerprint includes all Panackelty source files. Even this comment
edit invalidates the compiler probe cache; the 7–8-second compiler rebuild
explains most of the edited-run penalty. Narrowing that fingerprint would need
proof of complete dependencies and is not part of this measurement change.

### Compiler work versus runner overhead

Separate unprofiled component runs kept the prepared stage-2 compiler available
through `PANACK_TEST_COMPILER`. Each compiler-source command checked the exact
expected usage output; each compiled runner execution passed all 273 tests.

| Component | Elapsed seconds, three runs | Median user / system CPU seconds |
| --- | --- | --- |
| `./panack run src/compiler/main.panack` | 7.627 / 7.621 / 7.651 | 7.471 / 0.142 |
| Compile `tests/runner/main.panack` | 0.305 / 0.314 / 0.317 | 0.297 / 0.008 |
| Execute that runner bytecode | 17.139 / 16.950 / 17.580 | 13.373 / 1.288 |

CPU figures include reaped child processes. They separate user execution and
system work, but do not isolate fork/exec overhead from other system calls or
attribute the entire elapsed-minus-CPU difference to process management.
Compiler-source execution is already a fixture inside the runner, so those
rows cannot be summed. Caching the runner's own compilation would save only
about 0.31 seconds in this experiment. Skipping the compiler-source fixture
would remove a required public-CLI observation and is not an optimisation.

A separate three-second macOS `sample` capture, starting approximately 0.2s
into native execution of the compiler-source command, collected 2,541 samples.
Its largest exclusive stack leaves were `release` (595), `value_sequence`
(591), and the platform `strcmp` implementation (462). The first two comprise
46.7% of this short sample. It is an early execution window, not a whole-program
allocation count or a promise of a 46.7% speedup.

Source inspection explains a plausible avoidable cost: array `append` allocates
and copies a temporary pointer array, `value_sequence` allocates another array
and retains every element, and replacement later releases the previous array.
Repeated growth therefore performs linear work per append. The next PR should
first investigate reducing that copying while preserving persistent value
semantics, aliases, iteration, failure handling and bytecode compatibility.
Name lookup is a secondary candidate; the sample alone does not identify every
`strcmp` caller or justify changing dispatch yet.

### Hosted platform cross-check

The existing [focused profiling workflow](https://github.com/sproates/panackelty/actions/runs/36395124883)
passed on both platforms at the same revision. These are one initial/cached
pair per platform, not repeated medians. The workflow measures compiler and
bytecode checks before VM checks, so its cache state differs from the isolated
local initial run. Do not compare absolute times across different hardware.

| Hosted platform | Initial VM check | Cached VM check | Cached nested runner |
| --- | ---: | ---: | ---: |
| Ubuntu 22.04 x86-64 | 61s | 45s | 30s |
| macOS 14 arm64 | 64s | 52s | 30s |

The run's `focused-profile-*` artifacts contain the raw TSV observations,
budget records, source revision, compiler version and runner image identity.
Both confirm that the nested runner dominates. A single scheduling change
cannot credibly promise a 15-second check when that block alone takes 30s.

### Reproduction and next PR acceptance

Use an otherwise idle checkout at the measured revision. Unset inherited
`PANACK_CHECK_RUNNER_REPORT`, `PANACK_TEST_RUNNER_REPORT`, and
`PANACK_TEST_CAPTURE_RUNNER_REPORT`. Set `VALIDATION_JOBS=2` and an absolute
`VALIDATION_PROFILE_FILE` outside `build/`, then:

1. Run `make clean` and `make native native-module-build native-fault-build`.
2. Time `make check-vm` once as initial and three times unchanged, setting a
   distinct `VALIDATION_PROFILE_RUN` for each. An external monotonic timer or
   `/usr/bin/time -p` includes the complete command; the built-in phase timer
   begins after the native prerequisites.
3. Back up `tests/runner/host_runtime_unit.panack`. Before each of three further
   runs, append a distinct comment to the original bytes. Restore the backup
   even if a check fails. Retain every timing and failure rather than retrying
   away slower observations.
4. Time `./panack run src/compiler/main.panack` separately. Compile
   `tests/runner/main.panack` to a temporary `.bc` path, then time its execution
   with `PANACK_TEST_COMPILER` set to the absolute prepared stage-2 compiler.
   Keep report-reuse variables unset and check status, stderr and expected output.
5. For a CPU sample on macOS, start `./panack-vm run bootstrap/compiler-v8.bc run
   src/compiler/main.panack` with an absolute `PANACKELTY_STDLIB_PATH`, then use
   `sample <that-process-id> 3 1 -file <temporary-report>`. Keep this separate
   from benchmark trials; sampling changes execution cost.
6. Run `make clean` and confirm source restoration and a clean working tree.

The optimisation PR must preserve every existing assertion, add aliasing and
allocation-failure regressions for changed ownership paths, and pass full
`make check`, sanitizers, coverage and both platform gates. Repeat unchanged
and edited-source measurements against this baseline, retaining standalone
compiler execution and all oracle compilation/verification/execution. Report
remaining distance from 15 seconds rather than relaxing the target or claiming
that the local result applies to hosted runners. Full-pipeline timing must also
be checked for a regression.

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

## Persistent array append experiment — 2026-09-28

The implementation reduces repeated array copying using at most two views of
one backing store. A prefix and one extension retain separate visible lengths;
branches, possible ownership cycles and exhausted inspection budgets copy.
The [value model](../src/vm/VALUE_MODEL.md#persistent-array-append-storage)
defines allocation, bounded ownership inspection and prompt suffix reclamation.
No assertion, standalone validation stage, bytecode contract or timing budget
is removed or relaxed.

The comparison uses the same macOS 26.5 arm64 development environment, Apple
Clang 21.0.0, `-O2`, and two workers as the focused baseline at merged `8e13d54`.
Native prerequisites are prepared before the initial check; unchanged trials
reuse valid artifacts. Each edited trial appends a distinct comment to the
original host-runtime probe, then runs the complete check. Source bytes are
restored after the experiment. Totals include the whole Make invocation and
were measured by an external monotonic subprocess timer. Runs are sequential;
these are local observations, not cross-platform speed guarantees.

| Scenario | Before (seconds) | After (seconds) |
| --- | --- | --- |
| Initial, empty probe cache | 36.156 | 33.904 |
| Unchanged run 1 | 26.219 | 25.598 |
| Unchanged run 2 | 26.137 | 25.613 |
| Unchanged run 3 | 26.325 | 25.595 |
| Test-source edit 1 | 35.588 | 33.529 |
| Test-source edit 2 | 37.051 | 32.993 |
| Test-source edit 3 | 36.338 | 32.043 |

The cached median changes from **26.219s to 25.598s** (2.4% lower); the edited
median changes from **36.338s to 32.993s** (9.2% lower). All seven after runs
pass and retain the same visible PASS observations. The cached improvement is
small and should not be presented as a large general VM speedup. **The 15-second
focused target remains unmet.** Compiler execution and the complete nested
functional runner still need further measured work.

A controlled append workload isolates the changed operation: compile a program
that appends integers `0..20000` into an initially empty array, then prints its
length and final element. Run the same bytecode on old and new `-O2` VMs,
alternating version order across three trials. Both produce exactly `20000`
and `19999`, with empty stderr and success status.

| Append-only bytecode execution | Trial 1 | Trial 2 | Trial 3 | Median |
| --- | ---: | ---: | ---: | ---: |
| Before | 0.3667s | 0.3468s | 0.3473s | 0.3473s |
| After | 0.0076s | 0.0077s | 0.0072s | 0.0076s |

This approximately 46x result applies to repeated unbranched append, not the
entire compiler or validation pipeline. Shared snapshots and large child graphs
can deliberately take the copying path. The optimisation trades spare buffer
capacity and a bounded ownership inspection for reduced repeated allocation
and retention work; it does not promise constant-time append for every value.

Validation before publication:

- Clean `make check` passes in 107s (unit 72s, functional 1s, bootstrap 19s).
  All 1,297 baseline PASS observations remain, including multiplicity after
  normalizing temporary paths. The unit-phase warning remains recorded.
- Complete standalone AddressSanitizer/UndefinedBehaviorSanitizer validation
  passes, including the original 174 native observations and full oracle corpus.
- Native regressions compare 256 deterministic branching updates with independent
  copied arrays, retain snapshots in different release orders, check both
  prefix/extension lifetimes and prompt hidden-child reclamation, reject direct
  and indirect storage cycles (including a hidden suffix), exercise growth,
  bounded inspection and overflow, and sweep every append allocation failure.
- The public collections fixture covers aliases, nested arrays/records and append
  during iteration in both source and bytecode modes. Its existing assertions
  remain and the persistent-append check is added to its expected output.

The initial conservative experiment allowed sharing only for sequence-free
children and measured a 24.551s cached median. The final version admits small
independent nested collections through bounded ownership inspection and measured
25.598s. These experiments ran at different times, so the difference is not an
isolated estimate of inspection overhead. The final measurements above are the
reported result; the faster preliminary sample is not substituted for them.

### Hosted validation of the append implementation

The [focused profiling workflow](https://github.com/sproates/panackelty/actions/runs/36398315858)
passes on Linux and macOS at implementation `1a545b4`. Compared with the
[baseline workflow](https://github.com/sproates/panackelty/actions/runs/36395124883):

| Platform | Initial VM check before / after | Cached VM check before / after |
| --- | --- | --- |
| Ubuntu 22.04 x86-64 | 61s / 55s | 45s / 41s |
| macOS 14 arm64 | 64s / 41s | 52s / 31s |

These are single initial/cached pairs on hosted runners, not repeated medians
or isolated hardware comparisons. They support the direction of the local
result but do not establish those percentages as repeatable speedups. Both
hosted focused targets still exceed 15 seconds.

Full hosted validation preserves every baseline PASS observation on both Linux
paths and macOS. Native coverage changes from 86.91% to 87.43% of lines and
80.09% to 80.65% of branches, with 100% function coverage retained. The changed
`value.c` has 95.98% line and 92.04% branch coverage. The ownership tests check
retains per distinct backing store and verify every visible retained version;
they no longer assume every snapshot must own a separate buffer.

The first full CI run passed in 153s while the additional profiling workflow
was active. Its macOS runtime job started at 79s, versus 20–21s for the other
macOS package jobs. That observation is retained here; it is not the comparison
for an otherwise idle pipeline. The following cold runs occur after profiling
finished, with the same full validation and platform gates.

| Cold full pipeline run | Required gates complete | Summed runner duration | Result |
| --- | ---: | ---: | --- |
| [Attempt 2](https://github.com/sproates/panackelty/actions/runs/36398315862/attempts/2) | 1m48s | 16m16s | passed |
| [Attempt 3](https://github.com/sproates/panackelty/actions/runs/36398315862/attempts/3) | 1m44s | 15m07s | passed |

Both observations retain full pipeline completion below two minutes. This pair
is a regression check, not a replacement five-run median or a guarantee about
hosted queue latency. Timing uses attempt start through the last completed job,
including classification, setup, uploads and aggregate gates, as in the earlier
pipeline report. Summed job durations are not CPU use or billing estimates.


## Pipeline scheduling and coverage setup — 2026-09-28

The follow-up starts from merged `d24b8f2`. GitHub job timestamps distinguish
three quantities: time from a dependency completing to a dependent job starting,
step execution, and time from the last step completing to the job completing.
The first includes scheduling/dispatch overhead and is not a direct measurement
of runner queue time. The last is completion/reporting overhead, not test work.
Overlapping jobs must not have these quantities added to obtain wall time.

The earlier five-run sample at `8934654` remains 123/102/103/104/102 seconds.
Its 123-second attempt had a Linux runtime job starting 39 seconds after the
classifier completed. During the append profiling workflow, a macOS runtime job
started 67 seconds after classification and the full run took 153 seconds.
That overlap is evidence of contention, not proof of a specific account limit.
The later idle attempts took 108 and 104 seconds.

The [post-merge main run](https://github.com/sproates/panackelty/actions/runs/36416131575)
took 125 seconds. Its package result step finished at 11:32:38 UTC, but the Linux
package job completed at 11:33:17 UTC: a 39-second completion tail. Its latest
heavy job started only nine seconds after classification. Treating this miss as
slow tests or changing test parallelism would not address the observed delay.
Required checks must still finish successfully before a merge.

The same run spent ten seconds installing the unversioned `clang`, `llvm` and
`llvm-runtime` metapackages. Its Ubuntu 24.04 image already contained LLVM 18;
coverage compilation and reporting used that toolchain after installation.
The follow-up pins the existing validation image to Ubuntu 24.04 and directly
uses `/usr/bin/clang-18`, `/usr/bin/llvm-cov-18` and `/usr/bin/llvm-profdata-18`.
All three version commands must succeed before coverage. There is no fallback
installation, cache restoration, reduced corpus or changed result gate.
The [runner image manifest](https://github.com/actions/runner-images/blob/main/images/ubuntu/Ubuntu2404-Readme.md)
documents the preinstalled compiler family; the hosted runs verify the actual
binaries and retain the coverage summary for comparison.

Five sequential cold full runs measure candidate `c681014` without deliberately
overlapping profiling workflows. Every attempt, including a timing miss, belongs
in the sample. This change removes avoidable package-network setup; it cannot
guarantee a bound on hosted scheduling or completion reporting.

Local clean `make check` passes in 104 seconds (unit 70s, functional 0s at the
whole-second timer resolution, bootstrap 18s). The unit warning remains visible;
this change does not address the separate 15-second focused-check target.
The first hosted coverage summary is byte-for-byte identical to the main
baseline: 87.43% lines, 80.65% branches and 100% functions. The workflow contract
checks retain the complete native coverage command, pin the image/tool family,
and reject a return to package installation in this validation job.

All five [cold attempts](https://github.com/sproates/panackelty/actions/runs/36418279295)
passed every required check on the same candidate commit:

| Attempt | Full completion | Summed job time | Coverage job | Latest heavy-job start after classification | Final gate completion tail |
| --- | ---: | ---: | ---: | ---: | ---: |
| [1](https://github.com/sproates/panackelty/actions/runs/36418279295/attempts/1) | 88s | 934s | 66s | 9s | 3s |
| [2](https://github.com/sproates/panackelty/actions/runs/36418279295/attempts/2) | 106s | 880s | 44s | 10s | 2s |
| [3](https://github.com/sproates/panackelty/actions/runs/36418279295/attempts/3) | 107s | 916s | 43s | 9s | 3s |
| [4](https://github.com/sproates/panackelty/actions/runs/36418279295/attempts/4) | 103s | 955s | 65s | 10s | 2s |
| [5](https://github.com/sproates/panackelty/actions/runs/36418279295/attempts/5) | 97s | 885s | 73s | 9s | 2s |

Full completion is attempt start to the last job completion, including setup,
uploads and required result gates. Every attempt used fresh hosted runners;
there is no Actions cache or transferred build artifact. Summed job time is
neither CPU time nor a billing estimate. Timing uses the same API timestamps as
the earlier samples; `created_at` is not reused as the start of later attempts.

The median is **103 seconds**, with all five below 120 seconds and a worst
observation of 107 seconds. The median matches the earlier five-run sample;
there is no demonstrated median pipeline speedup. The absence of a slow tail in
five runs does not prove that queueing or reporting tails are fixed. The observed
benefit is removal of a roughly ten-second package installation/network step;
version verification took 0/1/0/1/1 seconds. Coverage execution itself varied
from 36 to 64 seconds, so its entire time difference cannot be attributed to
setup removal. macOS packaging determined completion in attempts 2–4.

The first attempt reports Clang, llvm-cov and llvm-profdata **18.1.3**. All 174
coverage-corpus PASS observations match the main baseline, as does the complete
coverage summary. Canonical validation, all sanitizer partitions, both package
platforms and all three stable required gates remain enabled. Future timing
misses must still be recorded; do not increase job count or serialize independent
PRs solely on the basis of these small samples.

## Archived roadmap performance history — 2026-09-28

The following preserves earlier roadmap observations, milestones and proposed
follow-ups. It is historical evidence across different revisions/environments,
not the active priority order or a claim that all present budgets are met.
See `ROADMAP.md` for current priorities and the condensed timing baseline.


**Non-blocking backlog:** CI now has a 103-second median and the latest clean
local check takes 107 seconds. Defer further focused-check optimisation while
publishing public coverage reports, then progressing the REPL. Keep the
15-second focused target, existing timing warnings and measured misses visible;
revisit optimisation when feedback delays become disruptive.
The public-coverage publication checkout passed a clean local `make check` in
145 seconds (unit 99s, functional 1s, bootstrap 37s) on September 28. This
environment still exceeds the clean and unit budgets; retain those warnings
as backlog evidence, not a blocker for publishing existing coverage.
Strong coverage remains more important than speed;
do not drop assertions, failure cases, sanitizer checks or platform gates,
move required coverage out of canonical validation, or widen timing budgets.

The current pipeline optimisation separates the sanitizer corpus into VM,
oracle, and nested-runner jobs, overlaps harness/compiler validation in one
three-worker job, schedules long compiler probes first, and overlaps independent
runtime probes. Native conformance runs independent programs through two
isolated workers; ordinary fixed-point and isolated seed-refresh proofs overlap
without sharing their stages. The macOS matrix retains five jobs to avoid runner queueing.
Five cold full runs now have a 103-second median, down from 148 seconds,
including classification, queue/setup and required result gates. Four finished
below two minutes; the 123-second outlier included a Linux runtime job
starting 52 seconds into the run. Median summed runner time fell 16.5%, with every baseline test
observation retained and an identical native coverage summary. See
[the complete profiling evidence](VALIDATION_PROFILE.md).

- [x] Bring the median full cold pipeline below two minutes with all validation
      and platform gates retained; record all five runs, including the outlier
- [ ] Investigate hosted-runner queue tails before treating 120 seconds as an
      upper bound: one of five runs still took 123 seconds. Keep that timing
      miss visible rather than presenting only the four faster runs.
      The follow-up also separates job completion delays from queue and step
      time, and removes redundant LLVM metapackage installation from coverage.
      Five follow-up cold runs pass in 88/106/107/103/97 seconds (103s median),
      with identical native coverage. This sample does not establish a hard
      upper bound on hosted-runner scheduling or completion delays. See
      [the reliability follow-up](VALIDATION_PROFILE.md#pipeline-scheduling-and-coverage-setup--2026-09-28).

The focused VM investigation at merged PR #76 records a 26.219-second local
cached median and 36.338-second median after a test-source edit. Hosted cached
checks took 45s on Linux and 52s on macOS in one profile pair per platform.
The nested functional runner dominates, and a short compiler CPU sample points
to collection copying and release work. Prioritize persistent array append
allocation/copying while preserving aliases and failure behavior; keep the
broad probe fingerprint intact until dependency completeness can be proved.
See [the focused investigation](VALIDATION_PROFILE.md#focused-vm-check-investigation--2026-09-28)
for all trials, measurement limits and the optimisation PR acceptance criteria.

- [x] Refresh the focused VM baseline with repeated unchanged and source-edit
      runs, component CPU measurements and a hosted platform cross-check
- [ ] Reduce measured collection-copying costs in compiler execution; preserve
      persistent values and add ownership/failure regressions before claiming
      a focused-check improvement

The historical CI baseline from PR #64 is 275 seconds for `make check`:
169 seconds for units, 42 for functional tests and 59 for bootstrap. Use fresh
measurements of the current toolchain to guide validation improvements.

The initial isolated local check passed in 239 seconds: units took
146 seconds, functional tests 37 seconds and bootstrap 50 seconds. This is a
local baseline, not a comparison with CI hardware. Later work records a clean
117-second local check; the full-unit warning and focused-VM timing follow-up
remain open. Preserve every assertion when investigating the remaining cost.

- [x] Profile clean and incremental validation on Linux and macOS, separating
      native builds, probe compilation, subprocess overhead and bootstrap stages
- [x] Make CI validation proportional to the change. Roadmap and other purely
      informational documentation edits should run lightweight document/link
      checks without rebuilding the compiler or running the full test,
      packaging, sanitizer and coverage suites. Classify changes conservatively:
      documents used as executable fixtures or packaged inputs (including the
      README quick start), specification changes, mixed code/document changes,
      and validation/workflow changes must retain the relevant behavioral and
      release gates; unknown impact must fall back to full validation
- [x] Keep a stable required CI result for both lightweight and full validation,
      so documentation-only PRs can merge promptly without bypassing protection
      or waiting for checks that do not apply. Test change classification,
      including additions, deletions, renames and mixed changes, and document
      which gates each class requires; retain full release validation
- [x] Reuse compiled probes using complete source/toolchain content keys, verify
      cached artifact digests, share one successful runner observation within
      each canonical check, and run independent probes with bounded workers
      while preserving assertions and instrumented corpus execution
- [x] Partition CI across compiler/harness, runtime/functional, bootstrap and
      native conformance jobs on both packaging platforms; run sanitizers and
      coverage independently, retaining stable aggregate gates and every proof
- [x] Demonstrate full cold CI below three minutes across repeated hosted runs,
      tracking queue/setup overhead and total runner time as well as elapsed
      duration: final runs took 2m20s and 2m28s versus 6m48s, with 3–8% more
      raw runner time. Later September 28 work reaches a 103-second median
      with lower runner time; the recorded queue-delay outlier remains a follow-up
- [ ] Reduce the remaining standalone VM-runner and compiler-build costs. The
      September 2026 macOS comparison reduced clean checks from 176s to 127s
      with two workers (123s with four), still above the 120s target. The full
      unit phase also retains its warning. Prioritize the measured remaining
      work without weakening standalone targets or isolated bootstrap proofs;
      see `tests/VALIDATION_PROFILE.md` for warm timings and coverage comparison
- [ ] Demonstrate clean `make check` within 120 seconds and focused incremental
      checks within 15 seconds on the reference environments; retain visible
      per-phase timing, warnings and CI reports to catch future regressions

Informational-only changes now use lightweight documentation/local-file-link
checks through an explicit conservative allowlist. Existing named check results
remain present and fail if routing or documentation checking fails. Full code,
packaging, sanitizer, coverage and release gates remain for relevant changes.
See [change-aware CI](README.md#change-aware-ci) for the exact boundary.

Detailed opt-in profiling now separates native builds, harness groups, source
probes and bootstrap stages. Both packaging platforms retain clean suite
profiles, while a separate targeted workflow records focused warm checks.
See [the profiling report](VALIDATION_PROFILE.md) for evidence and the
next measured investigations. This instrumentation does not claim a speed fix.

The persistent-array append optimisation reduces the measured local cached VM
median from 26.219s to 25.598s and the test-source-edit median from 36.338s to
32.993s. A clean full check passes in 107s. The focused 15-second target remains
open; these measurements do not establish a hosted-runner target. See
[the append experiment](VALIDATION_PROFILE.md#persistent-array-append-experiment--2026-09-28)
for every trial, the ownership tradeoff and the unchanged validation corpus.

Prioritize unit and bootstrap costs during this follow-up.
The native harness preserves process bounds, archive/installation checks and
runner fault injection; profile repeated compilation without dropping evidence.
The initial isolated native harness passed in 33 seconds against its 15-second
warning budget.
The seed-refresh gate adds isolated compiler stages to the bootstrap phase;
profile that cost separately and preserve its digest, fixed-point and failure
evidence when reducing repeated compilation. Keep the existing phase and total
budgets and record refresh-migration timings in its PR. The
new native oracle target retains the full program corpus under sanitizers and
coverage; profile its compilation work and reuse verified artifacts without
removing observations. The 15-second unit and 120-second clean-check budgets
remain unchanged. The first oracle-retirement check passed in 129 seconds
(unit 94, functional 26, bootstrap 7); unit and total warnings remain. Record
final ordinary and instrumented measurements in the migration PR.

The resolver migration exposed the compiler fixture's 20-second subprocess
limit on this environment: unchanged compiler source execution succeeded in
21.8 seconds when measured separately, while the first full run failed its
source and compile commands. These two compiler-building commands now use the
existing compiler-driver build allowance of 90 seconds; ordinary fixture
commands and phase warning budgets are unchanged. The failed run took 183
seconds overall (unit 171). Profile compiler self-compilation as part of the
prioritized timing work; increasing a command allowance is not a speed fix.

The September 2026 clean local check after the lexer unit migration took 125
seconds (120-second budget); its unit phase took 71 seconds (15-second budget).
Profile the native unit harness and build/bootstrap on this
environment while retaining all compiler unit assertions. CI timings remain
the reference for the cross-platform validation budget.
The host milestone adds process and filesystem boundary cases without
changing the 15/120-second budgets. Profile native oracle runs and
byte-exact host process assertions if the warning persists; keep all failure
cases and sanitizer coverage.
The VM milestone adds portable execution/loader and native-wrapper probes.
Its local focused `make check-vm` passed in 48 seconds against the 15-second
budget in the historical measurement, which included the then-active
differential tests. Keep process-launch and fixture-decoding costs in the same prioritized
unit-budget investigation; the 120/15-second targets are unchanged.
The bytecode milestone adds two portable codec/native command probes; a local
focused check took 19 seconds against its 15-second warning budget. Profile
fixture decoding, redundant process launches and native oracle work
without dropping malformed inputs or changing the timing budgets.

The unit timer now includes all seven Panackelty compiler probes as well as
the native unit tests. Keep their compilation and execution cost visible
when profiling the existing unit-budget warning. The final compiler migration
adds a direct driver build, source/bytecode commands and snapshot checks; profile
these separately from the native oracle before increasing allowances.

Current environment follow-up: the September 2026 testing-library branch
reported a 21-second unit phase against its 15-second warning threshold, also
observed on the unmodified checkout in this environment. With the expanded
fixture runner, clean checks observed 32–36 seconds for units and 79–84 seconds
overall. Profile the unit phase here and address its dominant cost without
reducing coverage; the earlier full check remained within its 120-second budget.
The compiler `source.path` fixture raised one clean check to 177 seconds (unit
97 seconds, functional 63 seconds). The cleanup failure unit test now selects
one fixture, reducing the next clean unit phase to 68 seconds. Next, reuse the
compiler result across the remaining integration checks
without dropping the source, bytecode, or path containment assertions.
The example migration adds twenty source and bytecode checks to each full runner
invocation. Profile that added work as part of the same prioritized timing fix.
The failure migration adds forty-one check and compile diagnostic pairs plus
artifact assertions; measure the full runner and remove redundant invocations
without weakening the new negative coverage.
The final functional migration's first clean run took 251 seconds (unit 147,
functional 86), with warnings at all three budgets. Its checkout-with-spaces
unit regression redundantly reran the complete functional suite; that test now
checks the stage-two compiler path and byte-identical output directly. Continue
profiling the remaining sequential runner work and reuse verified artifacts
to recover the 15/75/120-second budgets without removing assertions.
The functional runner took 24.6 seconds and each smoke invocation repeated its
full work (23.8 seconds for source). The functional recipe now captures one
successful report and checks it byte-for-byte from the smoke source and saved
bytecode; a focused run fell from about 73 to 25 seconds. Keep the standalone
smoke path and the remaining unit/full-check timing follow-up.

Validation speed is an internal nonfunctional requirement because slow feedback
discourages frequent checking and compounds the cost of every implementation
change. On the reference CI or development environment, a clean `make check`
should finish within 120 seconds and a focused incremental check with an already
built native toolchain should finish within 15 seconds. Exceeding a budget must
produce a visible warning and a tracked follow-up rather than silently becoming
the new baseline. Coverage must not be weakened to meet either budget.

The suite reports stable per-phase and total wall-clock timings. A September
2026 clean run after adding source locations completed its unit phase in 13
seconds, functional phase in 80 seconds, and complete `make check` in 132
seconds. The functional and complete phases therefore exceed their 75- and
120-second budgets; the warnings remain visible until the regression is
removed.
A September 2026 macOS checkout baseline after fixing paths containing spaces
passed 227 unit tests and 17 functional tests, but reported 322 seconds overall:
12 seconds for unit tests, 71 for the functional phase, and 238 for bootstrap.
Rebuilding stage 3 immediately afterwards took 37.31 seconds elapsed,
36.55 seconds of user CPU time, and 0.60 seconds of system CPU time. The
238-second result was not reproduced; the validation timer measures wall-clock
time and can include host interruptions.

Component-focused compiler, bytecode, and VM checks retain representative
public-CLI coverage. CI publishes and archives each timing row.

- [x] Add stable wall-clock timing for the complete suite and its unit,
      functional, and bootstrap phases
- [x] Emit a warning when a clean `make check` exceeds 120 seconds or a focused
      incremental check exceeds 15 seconds
- [x] Define fast, component-focused incremental targets that preserve the
      relevant internal and end-to-end evidence for a change
- [x] Run the fixed-point bootstrap proof exactly once per complete validation
- [x] Compile the self-hosted compiler once per validation and safely reuse its
      checked artifact across compatible functional cases
- [x] Remove redundant semantic compilation while retaining representative
      coverage of every public CLI workflow and failure behavior
- [x] Record timing trends in CI so regressions are visible before they compound
- [x] Reach both budgets without skipping, weakening, or relocating required
      coverage outside the canonical validation workflow
- [x] Restore the functional phase below its 75-second budget by running each
      program's source and compiled forms in one balanced worker task,
      parallelizing independent invalid cases, and reusing the already-verified
      stage-2 artifact for the compiler program's compiled execution
- [x] Recover the clean validation budgets after file-aware token and expression
      positions increased self-hosted compiler build time, without reducing
      fixed-point, functional, or diagnostic coverage; use repeated elapsed and
      CPU measurements to distinguish compiler cost from host interruptions

- [x] Recover the remaining Linux CI budgets after optimisation: the full
      check fell from 134 to 38 seconds (budget 120), unit tests from 29 to 10
      seconds (budget 15), and package bootstrap from 75 to 19 seconds (budget
      60), retaining all coverage and cross-platform bootstrap evidence.

### CI feedback improvements

The Check workflow runs once per pull-request update, with pushes limited to
`main`, and cancels superseded runs for the same PR. CI partitions the canonical
check into shared suites without repeating focused developer checks.
Both required platform packaging checks and their complete validation gates remain.

The native VM now defaults to `-O2` with standard overridable build flags. A
macOS compiler benchmark took 38.77 seconds without optimisation and 13.16
seconds with `-O2`; the generated compiler artifacts were byte-identical. An
optimised clean `make check` passed 229 unit tests and 17 functional tests in
54 seconds, compared with the preceding 140-second local baseline. Full
validation still includes the stage-2/stage-3 fixed-point proof. Cross-platform
CI timings remain the measure of PR feedback speed; compiler-only benchmarks
must not be presented as full-workflow savings.

### Linux validation follow-up

A unit-test profile found repeated compilation of self-hosted probes. The lexer,
resolver, checker, purity, emitter, driver, and codec tests now compile each
parameterised probe once per class and run every input in a fresh VM. Local unit
validation fell from 13.50 seconds to 6.65 seconds, with all prior assertions
retained and new regression coverage included.

The native VM now records string code-point counts and ASCII metadata once,
avoiding repeated scans for length and ASCII offsets. A paired compiler build
measured 12.79 seconds before and 5.85 seconds after, producing byte-identical
compiler artifacts. Hosted CI passed all three required jobs: tests in 43
seconds, Linux packaging in 49 seconds, and macOS packaging in 63 seconds.
The complete workflow finished in 73 seconds. Linux phase timings were 10
seconds for units, 18 for functional validation, 9 for the remaining bootstrap
phase, and 38 for the complete check; separate package bootstrap took 19
seconds. Every measured phase met its budget, with no platform checks or
fixed-point evidence skipped.
