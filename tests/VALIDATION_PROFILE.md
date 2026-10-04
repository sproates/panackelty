> Current decision: coverage now has an independent GitHub Pages publisher in
> [`sproates/panackelty-coverage`](https://github.com/sproates/panackelty-coverage),
> verified live in run `36933404078`. The combined-site coverage attachment and
> refresh design below is historical and superseded. Website cutover is verified;
> remaining schedule/core-only and cold/warm timing acceptance are tracked in [#187's current roadmap state](../ROADMAP.md#next-fast-website-ci-and-prepared-browser-test-environments).

# Validation profiling baseline

## U3 local effect explanations — 2026-10-03

Baseline `46bccfa` and the candidate seed compiled the exact baseline compiler
source and library with the same native VM, serially, using forward/reverse
order. No validation or review probes overlapped these samples. Two compiler
runs per revision averaged **5.977s before / 6.127s after** (+2.5%); all outputs
were byte-identical to baseline seed SHA-256
`5d02c622bebc4cfeed28e8a212b2c0dec86551371d486a8665281839364f0ee7`.
Per-child Linux `wait4` peak RSS was 118,300–118,348 KiB before and
118,372–118,376 KiB after. These small samples do not establish a general bound.

A fixed small program compiled in a five-run median **40.7ms / 41.2ms**, with
identical bytecode and 10,664 KiB measured peak RSS for both seeds. On that same
program, five explanation invocations had median **42.2ms / 43.7ms**; the latter
includes the newly retained and rendered effect boundaries. Timings include
process startup. Query timings ran before then after, not interleaved, and are
observations rather than a statistical regression guarantee.

Ordinary mode retains no effect entries, but the shared traversal returns a
record containing diagnostics and an empty evidence array. It is not a zero-cost
implementation claim. Query mode retains local boundaries for loaded definitions;
large-query retention and U0's broader experiments remain open. Seed size grows
from 309,613 to 316,297 bytes (+6,684), without a bytecode version change.

Canonical clean `make check` passed in **150s** (unit 112s, functional 5s,
bootstrap 19s, plus native/package setup). The full 120s and unit/incremental 15s
budgets remain unmet under #106; no test was removed or skipped. The earlier
PR #221 run was 186s on its then-current tree, not a controlled same-input pair.
All 196 focused explanation assertions, 36 CLI assertions and 343 functional
cases passed. A baseline/candidate comparison over 227 existing compiler-contract
fixtures matched exit status, stdout, stderr and artifacts exactly (43 accepted,
184 rejected). Independent review added seven targeted parity probes and found
no actionable correctness issues.

## Compiler lookup and validation cost — 2026-10-02

The user selected bounded performance work under #106 after PR #219 and asked
whether recent compiler changes caused slower validation. Baseline: `f379c50`;
Linux x86_64, GCC 13.3.0, native `-O2`, default two validation workers. No other
build or measurement ran alongside the baseline or fixed-input timing samples.
The later independent review and seed refresh were not timed benchmarks.

### Isolating compiler revision cost

Five historical seeds compiled exactly the `de26483` compiler source tree and
standard library using the same native VM. Two runs used forward/reverse revision
order. Every output had SHA-256
`9888463f9310890f09c32439f9ca6ff66e6ba485cbdf150f69e7a3916a2af30f`.

| Seed revision | First run | Reverse-order run | Mean |
| --- | ---: | ---: | ---: |
| U1 baseline `de26483` | 15.737s | 16.515s | 16.126s |
| Frontend spans `6d10c1b` | 16.232s | 16.819s | 16.526s |
| Instruction sources `60010dc` | 16.876s | 17.279s | 17.078s |
| Source-map CLI `cbae41b` | 16.631s | 16.769s | 16.700s |
| Explanations `f379c50` | 17.552s | 17.009s | 17.280s |

The final mean is 7.2% above U1 and 3.5% above #218 on this workload. Two samples
are bounded observations, not a statistical general regression guarantee.
Compiler `.panack` source grew from 233,433 to 273,290 bytes (17.1%) over the same
interval. Self-compilation and compiler-importing tests therefore also process
larger inputs; comparing full checks alone conflates implementation overhead,
input growth, new tests and host variation.

### Where clean validation spends time

A fresh profiled baseline `make check` passed in **366s**: unit 274s, functional
6s, bootstrap 68s, quick-start 1s, plus native setup. The unchanged warm
`make check-compiler` passed in **33s**. Both exceed their 120s/15s budgets.
Profiles retain every executed test; no result caching or changed worker count.

Large unit costs include compiling driver and snapshot helpers (24s each),
compiler-contract tests (24s), instruction-source tests (23s), source-map tests
(21s), explanation tests (20s) and checker tests (15s). Most of these probes then
execute in under the profiler's one-second resolution. Native oracle coverage
includes a 50s fixture-runner execution, which itself compiles source and bytecode
fixtures. These observations are nested/overlapping and **must not be summed**.
Bootstrap's native seed-refresh transaction is also intentionally fresh.

A temporary native instruction counter attributed **265,754,650 of 342,311,954
VM instructions (77.6%)** to `record_type`, `guarded_type`, `variant_type`,
`function_type` and `enum_type` while the baseline seed compiled the fixed U1
source. This is dispatched-instruction count, not a CPU-time percentage. The
counter only incremented a per-function field at dispatch and printed totals on
successful exit; its binary was never used for elapsed comparisons or validation.
A separate `-pg` CPU sample indicated interpreter/value traffic, motivating the
language-function counter rather than a speculative native lookup change.

### Bounded improvement

`indexed_program` retains source-order declarations and builds immutable
per-kind declaration indexes plus variant-owner/type/payload metadata. Parsing
and module combination both construct fresh indexes. Checker and constructor
lookups use those indexes; malformed duplicate declarations preserve each old
scan's last-match result, while the resolver still rejects the duplicates.
Native Maps themselves use linear scans and persistent copying: this removes
repeated interpreted declaration traversal, not all linear cost. There is no
new CLI mode, skipped safety check, bytecode format, source-map or evidence loss,
dependency cache, test selection change or separate compilation.

The initial isolated prototype comparison on the fixed U1 source measured
6.131s and 6.042s around an unchanged 15.659s baseline run. All three produced the
same bytes above: about 61% lower compilation time in this comparison.

Final canonical `make check` after all implementation/test edits passed in
**186s**, versus the same-host 366s baseline (49.2% lower elapsed time).
Unit validation fell 274s to 138s; functional stayed 6s; bootstrap fell 68s to
23s. Native setup and package/quick-start also passed. Worker settings and test
coverage are unchanged, except the additional 19 lookup assertions and three
public-CLI checks. The full 120s and incremental/unit 15s budgets remain unmet;
this is a bounded improvement, not completion of #106.

The unchanged warm `make check-compiler` passed in **32s** versus 33s before:
there is no material warm-path improvement in these single samples, because
compiled-probe reuse already removes the expensive compilation work.

Final same-VM/source comparison using the refreshed seed:

| Workload / metric | Before | Indexed candidate |
| --- | ---: | ---: |
| Fixed U1 compiler source, elapsed | 16.7896s | 6.5460s |
| Same child process, peak RSS | 103,576 KiB | 103,692 KiB |
| Small program, five-run median | 58.7ms | 58.0ms |
| Small program, peak RSS range | 7,912 KiB | 7,844–7,912 KiB |

Elapsed observations include process launch; Linux `wait4` provides per-child
peak RSS, not a retained-AST measurement. The one compiler pair cannot establish
a general memory bound; the small timings show no material change beyond noise.
Every before/after artifact is identical. Small-program SHA-256:
`90570ceb9ef26fbc7e1355399fcf45027d19ee5018904180b4b91df8baba251d`.
The compiler seed itself grows from 308,735 to 309,613 bytes (+878 bytes), while
user-program artifacts stay unchanged. README/SPEC and source-map/explanation
contracts were reviewed: no public syntax, option, proof or attribution contract
changes require alterations there.

### Reproduction and scope

Use a detached `de26483` worktree and extract each revision's
`bootstrap/compiler-v9.bc` with `git show`. From the detached worktree, set
`PANACKELTY_STDLIB_PATH` to its absolute `src/stdlib`, then time the same current
native VM running each seed with `compile src/compiler/main.panack -o OUTPUT`.
Hash each output; alternate seed order and avoid concurrent builds. Time the
unmodified and candidate seeds with the same VM, source and environment.
For validation, set absolute `VALIDATION_PROFILE_FILE` and
`VALIDATION_PROFILE_RUN`, run `make clean && make check`, then
`make check-compiler` without cleaning. The profiling protocol above remains
unchanged; wall-clock one-second rows locate large costs, not tiny operations.

The previous seed produces the indexed compiler byte-for-byte; fresh stages
2/3/4 converge at
`5d02c622bebc4cfeed28e8a212b2c0dec86551371d486a8665281839364f0ee7`.
The standard-library fixed point remains
`614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`.
Expanded unit and imported public-CLI coverage preserve independently expected
results. Dave's independent review found no actionable findings: 1,540 direct
comparisons against the original five scan implementations covered duplicate and
cross-kind names, empty/Unicode keys, declaration rotations and module merging;
79 existing sources had exact checking/explanation stdout/stderr/status parity;
all 22 accepted sources compiled to identical bytes. A custom imported generic
fixture preserved explanation/source-map bytes and runtime output. All 220
compiler-contract assertions passed independently. Review probes completed before
final timed validation. Broad cache invalidation and general performance budgets remain #106
follow-ups; compiler mode design is recorded separately in #220.

## Separate coverage host acceptance, 2026-10-02

PR #203 merged as `5381bc5`. Main Check `36935711159` passed all required native,
platform and website gates. Pages `36935939432` restored the browser-certified
website, skipped duplicate browsers and passed deployment/live verification.

| Observation | Result |
| --- | --- |
| Check-through-packaging validation | 182s conservative wall bound |
| Merge-to-verified-live | 212s wall time |
| Initial Check queue | 3s |
| Publication-only validation | 41s; excludes originating Check, not an end-to-end result |
| Website preparation / packaging | 20s / 19s |
| Deployment / verification | 10s / 6s |

This changed publisher code and used full validation. It is not a routine
website-only cold/warm acceptance sample. The 120s/180s targets remain unchanged.
The homepage, old `/coverage/` and `/coverage/html/` entry points matched merged
bytes via independent HTTP checks; website provenance identified `5381bc5` and
Check `36935711159`. The landing layout was inspected in the live browser.

Coverage workflow `publish.yml` was enabled and its main cron was
`7,22,37,52 * * * *`, but no scheduled run had appeared during investigation.
Manual run `36938575919` passed preparation, deployment and every-file live
verification in 51s. Its report advanced from core `9ef402d` / Check `36926148148`
to `5381bc5` / Check `36935711159`, artifact `11198500306`. Live website identity
remained unchanged and no new core Pages run appeared. No code fix was needed
for report selection, download or publication.

This establishes independent report refresh, not automatic schedule delivery.
The schedule's root cause is not established. GitHub's
[documented delay/drop behavior](https://docs.github.com/en/actions/how-tos/troubleshoot-workflows)
is a possible explanation, not proof of a particular service incident.
#187 retains scheduled-run, core-only production and routine cold/warm acceptance.

## Website publication dependency assessment, 2026-10-01

Assessment for [#187](https://github.com/sproates/panackelty/issues/187), after
PR #192 (`58a07bd`). The user authorised investigation and a design proposal;
this section does not implement or approve changing publication policy.

### Evidence and the actual dependency

| Production Pages run | Validation excluding observed dispatch | Merge-to-live wall | Artifact lookup | Check selection and coverage download |
| --- | ---: | ---: | ---: | ---: |
| [36910124129](https://github.com/sproates/panackelty/actions/runs/36910124129), PR #191 | 132s | 174s | 9s | 21s |
| [36912354959](https://github.com/sproates/panackelty/actions/runs/36912354959), PR #192 | 155s | 191s | 5s | 50s |

Both passed deployment and live byte/provenance verification. Both subsequent
automatic runs skipped duplicate publication. The 50s step includes API lookup,
polling and download; it is not a measurement of 50s idle time alone. PR #192's
103s PR validation excludes this production step and cannot establish the
production budget. Browser execution was 45s and image initialization 27s in
both production runs. These are two observations, not a latency distribution.

The dependency chain is visible in `.github/workflows/pages.yml`:
`publish` calls `pages_ready.cjs`, which calls `pages_source.cjs`. The selector
first fetches current main, then requires a successful **whole Check workflow**
for exactly that SHA before selecting any coverage report. Thus an already
available coverage artifact cannot unlock publication while packaging runs.

In [Check 36912354646](https://github.com/sproates/panackelty/actions/runs/36912354646),
coverage completed at 19:12:36 UTC; the `test` gate completed at 19:12:45;
macOS runtime packaging completed at 19:12:53; the last package compatibility
gate completed at 19:13:03. Website browsers had finished at 19:12:18. The Pages
selection/download step ran approximately 19:12:26–19:13:16. This explains why
reducing artifact lookup and polling did not remove the main bottleneck.

There is also a merge-time coupling: `ci_scope.sh` only has `docs` and `full`
routes. `validation_component` recognises website files, but every component
other than documentation/process still selects `full`. `check.yml` then runs
all seven native suites and ten platform/package slices. Changing a website
file therefore still invokes core validation, even though Pages never uses the
resulting native packages. Moving files alone would not fix the selector contract.

| Published component | Real inputs and required evidence | Current extra dependency |
| --- | --- | --- |
| Website/playground | `site/`, checksum-pinned browser v0.1.1 archive, assembly/publisher code, fingerprint, link/assembly tests and all 24 browser scenarios | Current-main whole Check, native packages and newly generated coverage |
| Native coverage | Report artifact from a trusted successful main Check, report identity/date, safe attachment and live verification | Selection requires current-main Check even when an older eligible report is already available |
| Release promotion | Explicit pin/checksum change and tests against that released browser artifact | No native build output is consumed by site assembly |

This follows `assemble_site.sh`, `fetch_playground.cjs`, `site/playground.json`
and `attach_coverage.sh`. Shared build/routing/publisher changes still need core
tests when they can affect core behavior. The classification boundary must be
reviewed, not inferred just from a file's directory or component name.

### Alternatives and recommendation

| Alternative | Benefit | Limitation / decision |
| --- | --- | --- |
| More polling/lookup tuning | Small bounded overhead reduction | Cannot remove the whole-Check dependency; not the next remedy |
| Wait only for the coverage job | Earlier fresh coverage | Changes the existing successful-whole-Check trust rule; could publish coverage from a subsequently failed run. Reject for this slice |
| Independent validation decisions, one publisher in core | Removes website-only core builds and waits; preserves coverage trust, URLs and one production writer | Recommended first implementation; requires explicit route and source-selection policy changes |
| Full website repository split #178 | Strong ownership separation | Still needs the same artifact/coverage contracts, plus permissions, previews, domain and rollback migration. Keep unscheduled until the smaller boundary is proven |

The recommendation changes **which evidence applies to which output**. It does
not remove native checks from core, mixed, shared or unknown changes; it does
not accept reports from incomplete or failed Check runs. All 24 browser scenarios
remain required for changed website inputs in this slice. It needs no new host,
domain, repository or self-hosted runner.

### Proposed contract and event behavior

1. Add a narrowly reviewed `website` route alongside `docs` and `full`. Its
   initial allowlist covers static `site/` assets and the explicit browser pin;
   file modes, deletions, renames and mixed changes must be checked. Publisher,
   routing, workflows, unknown paths and shared scripts retain `full` initially.
   Website-only changes run documentation, website automation/assembly, release
   integrity and full browser validation without native builds. Existing required
   compatibility gates must depend on successful applicable checks; skipped,
   missing or failed website validation must never yield a green gate. Audit
   required check names/rules before implementing; no branch-protection bypass.
   Concretely, extract preparation/browser validation into a reusable unprivileged
   workflow called by Check when website inputs change. Website-route compatibility
   gates depend on that result instead of native matrices; full-route gates retain
   native dependencies and include website validation where applicable. The
   privileged publisher consumes the exact successful main validation artifact,
   rather than running a second browser suite or making validation depend on
   deployment. Check never waits for publication, so there is no circular wait.
   PR artifacts remain ineligible for production. This also makes the required
   gate contract explicit without assuming permission to alter repository rules.
2. Split source selection into independent website and coverage decisions.
   Website evidence records immutable source SHA, input fingerprint, browser
   pin/test-suite identity and successful website validation. For `website`
   changes this is sufficient without current-main native Check. `full` changes
   continue to wait for exact-source successful Check before website adoption.
   Bind routing evidence to exact base/head revisions, not mutable labels or a
   caller-supplied claim. Inherited website-only classification must not allow a
   later mixed commit to bypass its full gate.
3. On a website publication, attach the latest eligible coverage from a trusted
   successful main Check at or before the selected website source's ancestry.
   Do not wait for a new report merely because main advanced. Keep coverage SHA,
   report timestamp and Check ID visible and distinct from website SHA. This
   intentionally extends the existing documentation-only reuse policy to
   independently validated website edits. Missing, expired, corrupt or untrusted
   required artifacts fail closed; preserve the existing live site and require
   an explicit successful rebuild, rather than silently deleting coverage.
4. On successful core Check completion, update coverage using the latest
   successfully published/validated website artifact. Do not promote changed
   website inputs from current main while their validation is pending or failed.
   The coverage SHA may be newer than that website SHA; record both honestly.
   Preserve every website/playground byte and skip browser provisioning. Native
   Check failure leaves previous coverage and website intact.
5. Keep the single production concurrency group and a single deploy operation
   that publishes the assembled site plus coverage. Re-select eligible sources
   after acquiring the writer slot and recheck identities before deployment.
   Stale queued triggers must not roll either component back. If sources advance,
   fail/retry explicitly; do not combine an unchecked website with fresh coverage.
   Preserve selected byte hashes and provenance in the deploy artifact and verify
   them live. Exact duplicate pairs skip deployment. Retain existing URLs.
6. Store website validation provenance independently from whole-Check identity.
   Retain an immutable coverage-free website artifact even when a newer core
   commit exists; automatic coverage refresh restores it by its recorded identity.
   Separate publisher revision, website source and coverage source in provenance.
   Version the provenance schema and test transition from today's fields. An old
   artifact with insufficient evidence requires revalidation, not guessed trust.

| Event | Website adoption | Coverage adoption |
| --- | --- | --- |
| Static website/pin-only edit | After applicable website tests; no native wait | Most recent eligible existing report |
| Core-only change | Keep previously accepted website bytes | New report only after whole Check succeeds |
| Mixed/shared/publisher change | Exact-source whole Check plus website tests when inputs changed | Eligible report after whole Check succeeds |
| Documentation-only change | No unnecessary website adoption or browser run | No publication unless selected report/component identity changed |
| Failed validation, expired artifact, API failure | Preserve live site; explicit failure | Preserve live report; explicit failure |

For manual refresh, select the same eligible identities; explicit cold rebuild
forces website tests without changing pins. The initial bootstrap must have both
a validated website and a successful coverage report. Do not rely on copying
unverified public bytes as the trust source. Rollback uses a retained, verified
component pair via the same writer and live checks; it must be explicit and
distinguishable from automatic monotonic selection.

### Acceptance and bounded delivery estimate

Estimate: **medium, two implementation PRs after this assessment**, potentially
a third if hosted race/transition evidence exposes defects. This is an estimate
for the same-repository boundary, not the full #178 migration.

- PR A: exact route/evidence model, narrow website-only validation and independent
  coverage selection for website publication. Preserve full routing for shared
  and publisher edits. Regressions must exercise complete route-to-gate decisions,
  fake API histories, wrong SHA/base, malformed modes, missing evidence and failed
  website tests. Update architecture, testing and provenance contracts together.
- PR B: coverage-only reuse of accepted website artifacts, race-safe single-writer
  integration and production acceptance evidence; remove superseded current-main
  coupling code/tests only once replacement invariants are covered. Verify
  schema transition, retained URLs, rollback and expiry/error behavior.

Neither PR closes #187 on implementation alone. Required evidence includes:

- Website-only change invokes no native builds and cannot merge/publish with
  failed website checks; core/mixed/shared/unknown changes retain native gates.
- Successful core-only Check publishes genuinely changed coverage through hosted
  artifact restoration with zero website assembly/browser provisioning. Compare
  every website file before/after; preserve report and website source identities.
- A core failure cannot publish its report; a failing/newer website cannot be
  adopted by coverage refresh. Test overlapping pushes, delayed completions,
  reordered API history, retries, duplicate triggers, missing/expired artifacts,
  corrupt downloads and main advancing before deploy. Check identity immediately
  before publication and demonstrate no rollback under stale queued triggers.
- Measure at least two cold and two warm website-only publications and a real
  changed-coverage refresh. Retain all samples, including misses. Keep the agreed
  120s validation / 180s merge-to-live budgets; also report merge-to-live wall,
  initial queue, observed dispatch, image pull, tests, assembly and live checks.
  Report mixed/full pipeline timing separately without silently weakening its
  acceptance. Confirm applicability of the original budgets to mixed maintenance
  changes with the user before claiming complete acceptance.

Subtracting the latest 50s selection/download step would give 105s validation
and 141s merge-to-live wall, but this is only a counterfactual lower bound: old
coverage still needs selection/download, scheduling varies and real production
measurements are mandatory. No budget achievement or hosted changed-coverage
reuse is claimed by this assessment.

The assessment is an intermediate part of #187, not a separate completed outcome.
Ledger remains 2/3; programme #180 stays paused. #178 remains an independent,
unscheduled ownership/migration proposal. Implementation requires acceptance of
the policy above and each later PR still requires explicit merge permission.

## Website prepared environment assessment, 2026-10-01

Issue #187 / PR #189. Agreed budgets: routine website validation 120s and
merge-to-live 180s, for cold and warm runs, excluding separately reported queue.
The prior Pages build in run 36893783723 lasted 784s (13m04s).

At `d6ffac7`, run 36898308883 used the official digest-pinned Playwright 1.63.0
image and all 24 scenarios with three workers. Attempt 1 passed: changes 4s,
prepare 10s, browser job 80s (including image initialization; test execution
44.2s), packaging 8s. First job start to packaging completion was 109s.
Attempt 2 passed on another fresh hosted runner: prepare 8s, browser 80s,
packaging 8s. Raw validation wall time was 148s; the job API recorded the first
job starting at 17:20:49 UTC but its first runner step at 17:21:26 UTC. Excluding
that observed 37s startup dispatch gives 111s, before any further dispatch
adjustment. Do not label this repeat run as a cached-container or production
artifact-reuse measurement: both PR runs executed the full browser suite.

The initial image trial rejected all Firefox launches because the mounted home
was owned by a different user. Aligning `/github/home` ownership with the
container user fixed it without redefining HOME or disabling browser tests.
Both successful samples preserve all test cases. Package installation was small
relative to browser execution/image initialization, so a derived Node-dependency
image is not justified by these samples.

Production website pushes now validate alongside core Check; publication waits
for the exact pinned source's successful Check and eligible coverage. Missing,
failed, superseded and API-error paths cannot publish. Final hosted checks of
that scheduling change, production artifact reuse, cold/warm merge-to-live and
live byte/provenance verification remain acceptance work. The issue stays open
until those measurements meet the agreed budgets; local and PR success alone do
not establish publication latency. Native validation retains its separate timing
warning and coverage requirements.


## Guard-fact repair and test hardening, 2026-10-01

Core repair for [#182](https://github.com/sproates/panackelty/issues/182), following
programme #180's baseline below. The initial focused checker runner passed 160 assertions:
23 fixed fixtures and 48 generated stale/fresh pairs, with all 48 freshly guarded
programs executed by the VM against a hand-calculated result table. All 20 fixed
unsafe inputs were independently accepted by the pre-fix compiler. CLI fixtures
exercise check/compile/run/disasm rejection, no output artifact, and a valid
source/saved-bytecode workflow.

Mutation sensitivity was checked in a temporary copy of the sources and tests;
the production working tree was not changed. Compile and run
`tests/runner/compiler_checker_unit.panack` separately with each of these
intentional defects, preserving all other code:

| Intentional defect | Independent assertion failures |
| --- | ---: |
| Replace `forget_written_fact`'s body with `facts` | 68 |
| Use `incoming_facts` directly instead of `facts_after_expression` at entry to `check_expression_scoped` | 7 |
| Omit the two pre-loop `facts_after_statement` calls, retaining the post-statement call | 4 |

Each mutant compiled successfully and its test run exited 1 with the listed
assertion failures. The unmodified implementation exited 0. These checks measure
sensitivity to specific omissions, not exhaustive proof of soundness.

Independent PR review added two direct regressions for mutation during a while
condition and a for iterable, bringing the checker runner to 162 assertions and
25 fixed fixtures. Both require the loop body to reject the stale subtraction
proof. The mutation counts above describe the initial 160-assertion suite.

`make regenerate-seed` produced identical stage-2/3/4 compiler artifacts with
SHA-256 `9888463f9310890f09c32439f9ca6ff66e6ba485cbdf150f69e7a3916a2af30f`
and identical standard-library artifacts. Full `make check` passed on the local
Darwin arm64 environment: 139s overall, unit 86s, functional 5s, bootstrap 23s,
release smoke 1s and quick start 1s. Overall and unit budgets (120s/15s) warned;
the roadmap retains the validation-cost reminder. Earlier runs found stale
hardcoded fixture-report totals; both independent report expectations were
updated to include the added cases before this successful run.

Paired compiler-only observations, alternating the old seed and fixed seed on
the same updated `src/compiler/main.panack` with no other validation running:

| Compiler | Three wall-clock samples (seconds) | Median |
| --- | --- | ---: |
| Pre-fix seed | 6.353, 6.288, 6.248 | 6.288 |
| Fixed seed | 6.339, 6.177, 6.255 | 6.255 |

The difference is within the sample variation; this limited observation shows
no material checking slowdown on this workload and is not a general performance
guarantee. Reproduce with the baseline seed from core `9b94096`, the fixed seed,
and `panack run SEED check src/compiler/main.panack`. Both check the same source;
do not compare different revisions' input programs.

## Compiler understanding probes, 2026-10-01

Initial evidence for [programme #180](https://github.com/sproates/panackelty/issues/180)
and the [architecture investigation](../ARCHITECTURE.md#compiler-and-runtime-understanding-initial-investigation-2026-10-01).
Baseline: `2952dd27bd8f48eedb09f173a60eab9f023d6aa6`, Darwin arm64, native
`make -j2` build. These are exploratory probes, not feature acceptance or a new
regression suite. No compiler/VM implementation was changed in this investigation.

Build and verify the current compiler independently of its driver seed:

```sh
make -j2
mkdir -p /tmp/panackelty-180-probes
./panack compile src/compiler/main.panack -o /tmp/panackelty-180-probes/current-compiler.bc
shasum -a 256 bootstrap/compiler-v9.bc /tmp/panackelty-180-probes/current-compiler.bc
```

Both SHA-256 values were
`b8df915137e1e33355dd8baaa9219e2a3aa8dac28e0dc9d3442678dbacccc6b9`.
For each program below, run both `./panack check FILE` and
`./panack run /tmp/panackelty-180-probes/current-compiler.bc check FILE`.
For accepted programs, repeat with `run` instead of `check`.
Both compiler paths produced the same outcomes.

For the expression probes use this complete template, substituting the table's
body for `BODY`:

```panackelty
pure debit(balance: Nat): Nat { BODY }
main(): Void { print(debit(5)) }
```

| Case | BODY | Check exit | Execution / evidence |
| --- | --- | ---: | --- |
| Literal | `5 - 2` | 0 | Prints `3` |
| Lower bound | `if balance >= 2 { balance - 2 } else { 0 }` | 0 | Prints `3` |
| No fact | `balance - 2` | 1 | Nat subtraction may underflow |
| False-branch fact | `if balance < 2 { 0 } else { balance - 2 }` | 0 | Prints `3` |
| Conjunction | `if balance >= 2 && balance <= 9 { balance - 2 } else { 0 }` | 1 | Nat subtraction may underflow despite sufficient mathematical condition |
| Weakened fact | `if balance >= 1 { balance - 2 } else { 0 }` | 1 | Nat subtraction may underflow |

For the literal probe, omit the parameter and call `debit()` in `main`; this
matches the exact zero-argument program executed. Rejections emitted the unpositioned text
`error: Nat subtraction may underflow; prove the left side is large enough or use Int`.
Binary AST construction currently lacks the wrapper needed to attribute this
operation, even though positioned calls work.

The relational probe also rejects with that diagnostic:

```panackelty
pure debit(balance: Nat, amount: Nat): Nat {
  if balance >= amount { balance - amount } else { 0 }
}
main(): Void { print(debit(5, 2)) }
```

Two mutation probes expose [#182](https://github.com/sproates/panackelty/issues/182):

```panackelty
main(): Void {
  mut balance: Nat = 5
  if balance >= 2 {
    balance = 0
    print(balance - 2)
  }
}
```

Both `check` paths exit 0 with `ok`; both `run` paths exit 1 with
`error: VM trap: Nat underflow`. Replacing `balance = 0` with
`if true { balance = 0 }` has the same result. These observed acceptances are
known defects, not desired regression expectations. The fix must turn the
unsafe cases into checker rejections while retaining runtime protection.

The accepted lower-bound program's `disasm` includes:

```text
FUNCTION|debit|pure|balance
0|LOAD|balance
1|CONST|Nat:2
2|BINARY|>=
3|JUMP_FALSE|8
4|LOAD|balance
5|CONST|Nat:2
6|BINARY|-
7|JUMP|9
8|CONST|Nat:0
9|RETURN
```

There is no source/proof map in this output or the current `FunctionCode` schema.
It is a useful baseline for #173, not an explanation of the checking decision.

### Cross-module change probes

`debit.panack`:

```panackelty
pure debit(balance: Nat): Nat {
  if balance >= 2 { balance - 2 } else { 0 }
}
pure identity[T](value: T): T { value }
```

`order.panack` in the same directory:

```panackelty
import "debit.panack"
pure settle(balance: Nat): Nat { debit(identity(balance)) }
pure checkout(balance: Nat): Nat { settle(balance) }
pure unrelated(value: Nat): Nat { value + 1 }
main(): Void {
  print(checkout(5))
  print(unrelated(5))
}
```

The current-source compiler runs this program successfully, printing `3` and `6`.
Two independent edits to `debit.panack` (restore the original between them):

- Weaken the guard to `balance >= 1`: check exits 1 at the underflow obligation.
  It does not produce a transitive guarantee-impact report for the callers.
- Remove `pure` from `debit`: check exits 1 at `order.panack:2:34`, reporting
  `pure function cannot call impure function debit`, including a source excerpt.
  The declared pure signature of `settle` is still used at its call sites; the
  result is not a recursively inferred effect report.

These experiments establish useful boundaries for #174/#175. They do not prove
`unrelated` is unaffected, implement predictions or establish a runtime value
history. Those remain explicit prototype work.

### Timing baseline

Wall-clock observations on the existing local Darwin arm64 environment, including
process launch, with no claim of controlled hardware or cross-machine performance:

| Workload | Samples in seconds | Median |
| --- | --- | ---: |
| Fresh-source compiler checking `order.panack` | 0.0321, 0.0325, 0.0320, 0.0325, 0.0336 | 0.0325 |
| Fresh-source compiler checking `examples/euler001_iterative.panack` | 0.0335, 0.0297, 0.0300, 0.0308, 0.0305 | 0.0305 |
| `./panack check src/compiler/main.panack` | 6.073, 6.012, 6.064 | 6.064 |

The small programs are dominated by fixed costs and cannot justify an overhead
budget. Use repeated compiler-as-input and practical-program samples when adding
evidence collection. Proposed budgets and remaining evaluation decisions are
recorded in the architecture investigation. No #180 acceptance box is checked
by these timings.

## Browser ownership and provisioning — 2026-10-01

Core PR #158 and [browser PR #5](https://github.com/sproates/panackelty-browser/pull/5)
move the full suite rather than dropping coverage. The historical core baseline
is [run 36785380904](https://github.com/sproates/panackelty/actions/runs/36785380904),
job 110125464499. The downstream cold/warm samples use the exact same browser
commit `60d5d512fe13392650f9b0893376fda18d1245b5` and Ubuntu 24.04:
[run 36800287161](https://github.com/sproates/panackelty-browser/actions/runs/36800287161),
attempt 1/job 110172851594 and attempt 2/job 110173866101. Both passed all 20
Node tests (including 145 VM corpus cases) and 21 real-browser scenarios.
The first attempt missed both caches; the second restored their exact keys.
Durations below are whole seconds from Actions step timestamps.

| Phase | Previous core workflow | Downstream cold | Downstream warm |
| --- | ---: | ---: | ---: |
| SDK restore | — | 0 | 4 |
| SDK download/verification/extraction | 5 | 4 | 4 |
| npm install | 2 | 2 | 1 |
| Two deterministic builds | 4 | 3 | 4 |
| Native oracle and runtime contracts | 5 | 5 | 5 |
| Engine cache restore | — | 0 | 8 |
| Engine/OS provisioning | 278 | 81 | 40 |
| Complete browser tests | 101 | 98 | 104 |
| Archive packaging | — | 0 | 0 |
| Total browser job | 402 | 213 | 182 |

Warm engine restore plus installation took 48s versus 81s cold; the complete job
was 31s shorter. These are single hosted samples, not a guaranteed speedup. The
old-to-cold difference cannot be attributed to caching: download and runner
conditions varied. SDK caching showed no time saving in this sample (8s including
restore versus 4s cold); its small pinned archive is reverified before extraction.
Missing or checksum-invalid SDK archives are downloaded again. Engine installs
still run on hits; missing executables are installed, and unusable engines fail
the actual tests. No cache contains a test result. Keys include OS/architecture
and the installer or complete package lock; there are no broad restore keys.

The primary structural saving is removal of the duplicate core browser build
and duplicate downstream publisher. Core PRs with only native/example components
no longer provision engines for the independently pinned browser product. The
shared selector regressions cover independent components, mixed/unknown inputs,
renames and missing history; website/shared inputs keep browser integration.
Production Pages still runs the full suite, then deployed-byte/provenance checks.
This is not a claim that skipped tests run faster or that production avoids all
engine setup. Dependency upgrades require downstream compatibility validation.

Final local implementation `make check` passed in **132s** against the 120s
budget. The native full/unit budget backlog remains open; browser separation
does not claim to solve native validation performance. The older observations
below are historical snapshots of their then-current routing.

## Modular validation route — 2026-09-30

The bounded slice of [#106](https://github.com/sproates/panackelty/issues/106)
separates audited process prose from native validation. On the local macOS arm64
host, an isolated archive of baseline `574e41b` received the new selector/runner
and an `AGENTS.md` prose edit. Each sample ran
`bash scripts/validate_change.sh --run BASE_SHA` under the normal restricted
sandbox, with no prior build directory. Three sequential `/usr/bin/time -p`
samples measured **0.81, 0.81 and 0.80 seconds**. Each printed `route=docs`,
`components=process`, `checks=documents,links,whitespace`, passed local links and
whitespace, and left no `build` directory or `panack-vm` artifact. No compiler,
package download or socket permission was required.

| Representative change | Previous selection | Current selection |
| --- | --- | --- |
| Roadmap prose | Docs | Docs |
| AGENTS/contribution/roadmap-process/Next Item/PR-template prose | Full | Docs |
| Compiler, bytecode, VM/TCP, library or examples | Full native validation | Full native and browser integration envelope |
| Playground/package/shared/unknown inputs | Full Check; separate browser path filters | Full Check and both browser consumers via the shared selector |
| Mixed prose/code, executable or symlink documents | Full | Full |

Replaying the complete five-file diff of grooming PR #143 with the new selector
now selects `docs`, including its PR template; the previous selector chose full.
The earlier process-document change had taken 94 seconds with socket access
and failed native contracts under restriction. That is historical context,
not a controlled speedup ratio. The development full check for this slice
passed in **103 seconds**, including unit, functional, bootstrap and packaged
quick-start evidence. Its unit phase still exceeded the 15-second target;
full-validation optimization remains a recorded follow-up, not a claim made by
this change. Final-head validation is recorded in the delivery PR.

`make ci-check` tests independent expected components, conservative integrations,
local/committed parity, staged reversals, untracked changes, failure propagation,
renames/deletions and stable result gates. A fixture executes the actual local
runner and Makefile without compiler sources and with compiler/network commands
forbidden. Browser workflows use the shared selector for PR validation; existing
production Pages publication is separate and can still build after a docs merge.

No per-component code-test savings or incremental-cache benefit was measured.
The [dependency map](README.md#component-dependencies-and-retained-coupling)
records remaining compiler/VM, probe, bootstrap and packaging coupling. Full code
validation keeps all existing checks and now conservatively reaches browser
consumers for shared/unknown changes as well.

## Clean validation comparison — 2026-09-29

Investigation: [issue #104](https://github.com/sproates/panackelty/issues/104).
Two clean checks ran serially on the same Linux x86-64 workspace, with GCC
13.3.0, `CFLAGS=-O2` and `VALIDATION_JOBS=2`. The container exposed nine online
processors with an eight-CPU quota (`cpu.max` = `800000 100000`); no other builds
or validation ran alongside these samples. Baseline `30b584a` predates async;
current `e71449f` is merged PR #103. Each checkout used `make clean` first.

| Inclusive measurement | Before async (s) | Merged async (s) | Bounded overlap (s) |
| --- | ---: | ---: | ---: |
| Complete `make check` | 341 | 362 | 346 |
| Unit phase | 231 | 245 | 224 |
| Functional phase | 2 | 2 | 3 |
| Bootstrap phase | 54 | 60 | 61 |
| Final quick-start phase | 1 | 1 | 1 |
| Harness within unit | 73 | 75 | 80 |
| Native contracts within unit | 105 | 113 | 115 |
| Nested functional runner within native contracts | 48 | 53 | 54 |
| Compiler integration probe within unit | 39 | 43 | 48 |
| Isolated seed-refresh proof within bootstrap | 53 | 60 | 61 |

Every top-level phase passed. The complete check increased by 21s (6.2%);
most of the six-minute cost existed before async. This is one paired sample,
not a statistical estimate or proof that all 21s are caused by async machinery:
PR #103 also adds compiler/runtime tests and changes harness isolation. Earlier
macOS samples around 104s are a different environment and cannot establish a
regression of that size. The merged Linux checks at 358s and 360s corroborate
that the current wait is repeatable on this workspace, not a universal duration.

Rows are whole wall-clock seconds and include nested/overlapping work: do not
add them together. The functional phase reuses the successful complete runner
transcript produced inside native contracts; it is not a two-second standalone
functional suite. Policy, native prerequisites and packaging outside the named
top-level phases account for the remaining check time. Nonzero nested profile
rows deliberately injected by harness controls do not denote a failed check.

The bounded candidate reuses CI's existing harness/compiler overlap in canonical
`unit-impl`. With two workers each branch gets one worker; runtime follows only
after both succeed. With one worker execution stays serial. No tests, bootstrap
proofs, sanitizers or budgets are removed or relaxed. The existing isolation
checks protect the shared VM from harness relinks. A real-recipe regression
checks suite multiplicity, worker allocation, overlap, ordering and each failure
at one, two and three workers.

The candidate (merged async plus this PR's scheduling and regression changes)
passed a clean full check in 346s, 16s (4.4%) below the merged baseline. Unit time fell from 245s to 224s
(8.6%). All 1,390 reported PASS observations matched after normalising temporary
cleanup paths; the new recipe failure controls also passed. This is one clean
candidate sample, not a guaranteed saving. Concurrent compiler/harness work
increased some individual probe durations; total improvement is modest. The
bootstrap sample of 61s also exceeded its 60s warning budget, alongside the
existing full-check and unit warnings. No budget was widened. Subsequent edits
to this result report and roadmap are informational and use `make docs`.

Independent module builds are a follow-up candidate raised during review of
these results. Native C objects already rebuild separately and focused compiler,
bytecode and VM targets exist. Panackelty source modules, however, are loaded and
combined into one program before checking/emission; source organisation is not
separate compilation. First measure the payoff from dependency-scoped probe
cache keys and cached frontend work. Separately compiled module artifacts would
then need explicit interface/signature rules, dependency invalidation, linking,
type/effect preservation and deterministic bootstrap evidence. Neither approach
is implemented or authorised by this timing investigation.

Reproduce each revision in a separate Git checkout/worktree, serially, using an
absolute report directory outside either build tree:

```sh
make clean
export CFLAGS=-O2 VALIDATION_JOBS=2
export VALIDATION_PROFILE_FILE=/absolute/report-dir/variant-profile.tsv
export VALIDATION_TIMINGS_FILE=/absolute/report-dir/variant-budgets.tsv
export VALIDATION_PROFILE_RUN=variant-clean
sh tests/profile_command.sh variant/check make check
```

Use distinct empty report files for each variant. Preserve the host/toolchain,
revision, exit status and phase rows, and compare measurements with matching
parent labels. The 120s clean and 15s focused targets remain open; deeper
compiler/nested-runner or isolated seed-refresh optimisation needs its own
bounded investigation rather than weakening those proofs.

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

## Browser-playground feasibility — 2026-09-30

Issue [#110](https://github.com/sproates/panackelty/issues/110), native baseline
`a35e4e11da9412a7ede03d0c12e826d9428cb2ff`. See the
[architecture and reproduction record](../ARCHITECTURE.md#browser-playground-feasibility--2026-09-30)
for the pinned toolchain, private source snapshot, adapter and exact build flags.
This is an isolated research profile; the repository's build/test targets,
compiler seed, VM sources and fixed fixtures are unchanged.

### Execution evidence

Linux x86_64, Node 24.19.0, Emscripten 6.0.10, `-O2`, 256 MiB maximum linear
memory per instance and 2 MiB C stack. Node runs the actual generated Wasm;
the worker harness substitutes local-file reads for browser fetch/importScripts.

- Of 145 existing fixed bytecode cases, **131 match native exit status, stdout
  and stderr exactly**. Fourteen forged host-operand cases still exit 1 with
  empty stdout, but deliberately report unavailable host capability instead of
  the native path/duration/UTF-8/process operand diagnostic. Both groups of
  seven cases are preserved individually in the experiment's evidence file;
  original fixtures and their expected results were not changed.
- The unchanged v9 compiler seed compiles new source on the Wasm VM. One
  exact-arithmetic program produces byte-for-byte identical output bytecode to
  the native CLI compiler. This is one cross-target compiler sample, not a
  full Wasm bootstrap fixed-point proof.
- Actual worker tests pass for greeting output, rational/decimal/large-natural
  exactness, a logical stdlib import, a typed source diagnostic, unsupported
  host rejection, output overflow, overlong source and empty output.
- An infinite program is terminated after reaching execution; a fresh worker
  subsequently compiles and runs a greeting. The final local sample took
  7.19 ms to terminate after the request, excluding the prior 50 ms test delay.
  This is Node worker evidence, not an iPhone cancellation measurement.
- Memory growth beyond 256 MiB throws `RangeError`; a version-8 header mutation
  is rejected with status 1 and the unsupported-bytecode-version diagnostic.
  No total-browser-memory, allocation-exhaustion or full async-host proof is
  claimed.
- DOM-stub tests pass for start, duplicate-start prevention, stop, stale-response
  rejection, literal text output, the 15-second timeout, worker-load errors,
  source bounds and example selection. JavaScript syntax checks pass. Actual
  rendering, browser asset loading and browser-specific timer behavior remain
  unverified because browser automation was unavailable.

The initial output-limit design threw from the output callback. A real infinite
printing test timed out because that did not reliably abort the C runtime. The
corrected protocol posts one terminal response and has the owner terminate the
worker; the repeated overflow test passes. The first stdlib test also exposed
late environment initialization; setting its path in `preRun` fixes the test.
These are experiment findings, not native-toolchain regressions.

### Size and latency observations

| Asset | Uncompressed bytes | Local gzip level 9 bytes |
| --- | ---: | ---: |
| VM WebAssembly | 70,743 | 29,870 |
| Generated JavaScript runtime | 63,041 | 17,876 |
| Existing compiler seed | 261,561 | 50,149 |
| Standard-library source JSON | 12,420 | 3,533 |
| Initial page, scripts, styles and all runtime assets | 418,513 | 106,064 |

Totals exclude optional license/source downloads. Gzip sizes are measurements
of local files, **not** a verified CDN transfer policy or cold-download timing.
The native audit's executable size is not a like-for-like packaging comparison.

Final local worker sample, milliseconds; each case has a fresh worker and VM
instances, but the OS caches local files and no HTTP request occurs:

| Program | Compilation | Execution | Total worker request |
| --- | ---: | ---: | ---: |
| Greeting | 29.85 | 0.43 | 76.12 |
| Exact arithmetic | 31.21 | 0.74 | 57.77 |
| Standard-library suffix helper | 73.48 | 0.63 | 94.94 |
| Type error | 26.17 | Not executed | 40.13 |

These are individual feasibility samples, not medians, speedup comparisons or
browser latency promises. Phone performance, cold network loading, background
cancellation, peak total memory and cross-browser compatibility still need
measurement before public-site integration. The existing native validation
performance backlog and 120s/15s budgets remain unchanged by this experiment.

## Implicit core methods: 2026-09-30

Local macOS/Apple silicon comparison against the previous v9 seed at main
73e0e8f, using the same native VM and pinned WASI SDK 34.0 build. Seven warm
samples per variant followed one warm-up, alternating old/new order. Programs
print a greeting or test the same literal suffix; the older suffix source
imports stdlib/text, the new source calls `.ends_with()`. These are small local
compilation measurements, not end-to-end browser/network or phone timings.

| Workload | Old native median | New native median | Old WASI median | New WASI median |
| --- | ---: | ---: | ---: | ---: |
| hello | 4.67 ms | 21.67 ms | 2.05 ms | 15.00 ms |
| suffix | 15.60 ms | 24.66 ms | 9.02 ms | 14.50 ms |

Implicit loading parses and checks the source-defined core even for tiny
programs. The measured regression is roughly 17 ms native / 13 ms WASI for the
greeting, and 9 ms / 5 ms for suffix matching. This is an explicit latency cost,
not a speed improvement. Emitting only reachable core algorithms keeps greeting
bytecode unchanged at 46 bytes and reduces suffix bytecode from 886 to 350 bytes.
If larger programs or slower devices make the fixed core cost material, assess
cached/prevalidated core frontend work in the existing incremental-build backlog;
do not skip checking user code or silently expand implicit library loading.

| Asset | Old bytes | New bytes | Old gzip-9 bytes | New gzip-9 bytes |
| --- | ---: | ---: | ---: | ---: |
| compiler | 261561 | 265217 | 50078 | 50902 |
| stdlib_json | 12420 | 11349 | 3551 | 3358 |

VM/Wasm sources and the bytecode format are unchanged. Gzip values are local
compression measurements, not verified CDN transfer sizes. Browser runtime
contracts pass 13/13; real Chromium, Firefox and WebKit tests pass 18/18,
including import-free construction, chained methods and wrong-receiver errors.
The full functional runner passes 297 assertions. These results precede the
final clean canonical check; its result is recorded in the PR validation summary.


## U1 source-mapping experiment, 2026-10-02

Reproduce with `make source-mapping-experiment`. Measured on macOS arm64 with
Node 26.0.0, native release build and the existing v9 seed, based on `c6eadb0`.
Hosted experiment tooling pins Node 24. The final bounded suite reports 111
assertions. All five fixture artifacts are byte-identical to public CLI output.
The sidecar binds all six source files in the fixture corpus, including files
not loaded by a particular entry. Tiny-program size ratios are not representative
of real projects.

| Case | Executable bytes | Sidecar bytes | Embedded candidate total bytes |
| --- | ---: | ---: | ---: |
| local | 101 | 990 | 1099 |
| imported | 119 | 1002 | 1129 |
| generic | 161 | 1002 | 1171 |
| nested | 132 | 993 | 1133 |
| conditional | 143 | 1003 | 1154 |

Five warm samples in milliseconds, rounded to three decimal places:

| Operation | Raw samples | Median |
| --- | --- | ---: |
| compile_plain | 31.258, 28.940, 28.881, 28.612, 28.736 | 28.881 |
| compile_mapped | 31.788, 32.527, 29.888, 30.916, 29.424 | 30.916 |
| run_bulk | 2.678, 2.536, 2.550, 2.723, 2.501 | 2.550 |
| run_observed | 2.413, 2.334, 2.111, 2.247, 2.263 | 2.263 |
| serialize | 0.290, 0.333, 0.252, 0.269, 0.351 | 0.290 |
| lookup | 0.432, 0.379, 0.262, 0.332, 0.353 | 0.353 |

The compile median difference is +2.035ms; metadata serialisation and lookup are
separate costs. Runtime compares the same executable's bulk and single-step
modes over 1,000 successful calls and a final trap. The observed median difference
is -0.287ms, below startup/noise at this scale; it does not show an optimisation
or establish a production tracing budget. Probe build, disassembly and the whole
validation command are outside these samples. Production compiler/VM sources
and emitted executable bytes do not change. No production overhead is introduced.

The appended candidate is rejected by v9; a versioned embedded consumer was not
implemented. Optional sidecars are the recommended U2 direction. Source-span
retention, exact dependency closure, producer trust and larger realistic-program
budgets remain production work. The malicious recomputed-digest counterexample
is retained as evidence that integrity alone cannot authenticate attribution.
Independent review requested corruption protection, comparable timing and
variable-PC/nested-index regressions; those changes are included in this run.


## U2 frontend source spans, 2026-10-02

The first U2 slice retains complete expression spans and replaces the U1 range
recovery workaround. Reproduce frontend tests with `make check-compiler`, the
bounded attribution experiment with `make source-mapping-experiment`, and full
unit/functional/bootstrap validation with `make check`.

The focused compiler check passed in 47s against a 15s target; the existing #106
validation-cost reminder remains active. After the seed refresh, full validation exposed 15 old failure transcripts
(46 command assertions) with changed locations. Their rejection and error text
were preserved while precise locations/excerpts were updated. No tests were
removed or relaxed. The new span
runner has 33 literal range expectations; six public CLI integration assertions
check local and imported compound diagnostic locations. The 111 experiment
assertions pass with direct retained spans. Independent read-only review found
no actionable issue after ten extra range probes, five accepted byte-identical
programs and stale-guard/purity rejection probes.

`make regenerate-seed` verified fresh stage-2/3/4 compiler and standard-library
fixed points before updating the v9 seed and digest. The compiler digest is
`ad72e9e53f7431cc0acaadedc0dc6c8a230b6e01a09f387821fe27376b251d22`;
the conformance artifact digest is
`614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`.
The bytecode format and language syntax are unchanged.

Three warm, alternating baseline/candidate compile samples on macOS arm64, using
the `de26483` seed and refreshed seed with the same native VM and source files:

| Input | Baseline seconds | Candidate seconds | Executable bytes |
| --- | --- | --- | ---: |
| `src/compiler/main.panack` | 7.093, 7.045, 7.110 | 7.426, 7.195, 7.282 | 272798 |
| `tests/functional/cases/callables/main.panack` | 0.034, 0.033, 0.034 | 0.036, 0.035, 0.036 | 800 |

Every paired artifact was byte-identical. Each sample runs `./panack run SEED
compile INPUT -o OUTPUT` in a fresh process with stdout redirected. The compiler
input is the candidate compiler source in both cases. Its median rose by 0.189s
(about 2.7%); the tiny callable case differs by 2ms and includes process startup.
These observations do not establish a general overhead budget, memory cost or
realistic-program acceptance. Further U2 metadata collection must measure its
additional cost separately. Production instruction attribution remains unfinished.

## U2 instruction-source emission, 2026-10-02

The optional emitter path retains sparse original ranges at actual function-local
instruction indices. Ordinary emission disables entries; serialization and v9
instructions are unchanged. Tests distinguish direct expression operations,
lowered machinery and unavailable origins without borrowing nearby mappings.

Acceptance evidence:

- `make check` passed in 158s with native host operations available: unit 100s,
  functional 4s, bootstrap 25s, quick start 1s. All 36 new mapping assertions and
  the complete public-CLI nested lowering fixture pass. The 120s full/15s focused
  budgets remain exceeded and the #106 reminder is retained in ROADMAP.md.
- The initial sandboxed check failed native module/host-fault groups; both passed
  unchanged with host permissions available. No coverage was skipped or weakened.
- `make source-mapping-experiment` passed 121 assertions against real VM traps,
  including inner indexes, initializers, earlier expressions, imported/generic
  bodies and a collection callback. Missing/stale/malformed metadata checks remain.
- Fresh stage-2/3/4 compiler and standard-library artifacts converged exactly.
  Seed: `ca2f027ee8f1afce1660d1bd35c72532422a360bbcc495ae614ec805d3272dfb`
  (283151 bytes, previously 272798). Standard-library artifact SHA-256 remains
  `614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`.
- Independent read-only review found no actionable defect. A separate nested
  if/match/map/reduce probe checked 93 instructions at offset 13 and 21 manually
  specified exact source-range/kind checkpoints. Exact constructor-origin tests
  were added following the reviewer's optional coverage suggestion.

Three sequential warm samples compiled identical current sources using the
previous frontend-span seed and the refreshed seed. A third path used the
source-mapping experiment compiler with opt-in emission and INDEX_GET row output.
All three paths produced identical executable bytes in every sample. Commands:
`panack-vm run SEED compile SOURCE -o OUTPUT` and
`panack-vm run build/experiments/source-mapping-compiler.bc SOURCE OUTPUT map`.
These include loading, checking, emission, serialization and process startup;
the mapped probe also includes row output, so this is not an isolated emitter
benchmark. Samples were taken after full validation, without another test run.

| Source | Previous seed seconds | Ordinary new seed seconds | Mapped probe seconds |
| --- | --- | --- | --- |
| Compiler `src/compiler/main.panack` | 8.041, 8.111, 7.987 | 8.220, 8.216, 8.120 | 8.349, 8.176, 8.202 |
| Nested lowering fixture | 0.037, 0.037, 0.040 | 0.038, 0.040, 0.038 | 0.038, 0.047, 0.040 |

Compiler compilation medians were 8.041s before, 8.216s ordinary after (+2.2%)
and 8.202s with the mapped probe. The small difference between the two new paths
is measurement variation, not evidence that retaining maps is free or faster.
The fixture medians were 37/38/40ms. These bounded measurements establish neither
general overhead/memory budgets nor whole-programme realistic-program usefulness.

## U2 validated source-map CLI, 2026-10-02

The final U2 delivery replaces the experimental checksum sidecar with exact local
replay. Current reproduction is `make source-mapping-experiment`; it no longer
uses the historical `source-mapping-compiler.bc` adapter or JavaScript metadata
serializer. Earlier U1 bulk/single-step and serialization timings above describe
the retired experiment, not the current production contract.

Focused results: 51 self-hosted source-map unit assertions, 45 public-CLI
assertions, and 121 native observer/forgery assertions pass. Existing span and
emission suites remain registered. The independent reviewer found a directory
symlink output alias, fixed by checking the second destination after the first
write and verifying both resulting files. Its regression proves the bytecode
still executes after rejection. The reviewer also found that the initial timing
helper performed two lookups; the corrected samples below contain one each.
No further production correctness issue was reported.

Warm macOS development measurements, native optimised VM, process startup
included (milliseconds, five samples):

| Operation, local indexing fixture | Samples | Median |
| --- | --- | --- |
| Ordinary compilation | 28.36, 28.63, 28.43, 30.22, 29.40 | 28.63 |
| Compilation with source map | 31.47, 34.26, 30.72, 30.82, 31.61 | 31.47 |
| Reproduced lookup | 34.27, 30.89, 33.95, 31.31, 31.52 | 31.52 |

| Fixture | Executable bytes | Sidecar bytes |
| --- | ---: | ---: |
| Callback | 287 | 3,464 |
| Local | 101 | 2,746 |
| Imported | 119 | 2,853 |
| Generic/Unicode | 161 | 3,106 |
| Nested | 132 | 2,947 |
| Conditional | 143 | 3,048 |

A compiler-sized multi-module observation used `src/compiler/main.panack` as the
entry, `/usr/bin/time -p`, fresh output filenames, and `locate` for
`source_map_arguments` instruction 0. Ordinary compilation took 8.28 s; mapped
compilation took 10.47 s; validated lookup took 10.48 s. Ordinary/mapped artifacts
were byte-identical at 298,386 bytes; the sidecar was 1,493,879 bytes. This is one
warm sample each, not a stable performance guarantee. Lookup has compilation-scale
cost, and full source snapshots make maps larger than executables. These results
support the bounded local tool while leaving programme-wide usefulness and
later runtime retention budgets open. Serialisation uses balanced chunk joining.

Fresh stage-2/3/4 compiler SHA-256:
`164bb5899ce98dfd02d4b26be112c01a0756220e2c443b5de6cc8965174f81d9`.
All three standard-library artifacts match
`614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`
and execute with the expected output. Bytecode remains v9.

Final clean `make check` passed in 163 s, including unit, functional, bootstrap,
package smoke and quick-start checks. This exceeds the existing 120 s budget;
the #106 roadmap reminder includes the measurement. No coverage was skipped.
The original pre-U2c seed also reproduces the final 298,386-byte compiler exactly.
Documentation/link and diff checks pass.

## U3 first subtraction explanation query, 2026-10-02

Base: `cbae41b` (merged #218). Candidate branch:
`feat/guarded-subtraction-explanations`. The query retains the actual checker
subtraction decision and bound origin; normal checking omits evidence. The
[contract](../docs/COMPILER_EXPLANATIONS.md) defines scope and honest unavailable
results. This is the first U3 query, not acceptance of all #134 or #180.

Focused acceptance passed: 96 self-hosted unit assertions, including expected
proof outcomes, source origins, diagnostic parity and disabled retention; 26
public-CLI assertions in the new functional case, which passed both from source
and compiled bytecode. Existing checker mutation fixtures retain exact diagnostic
parity. The canonical `make check` remains the final implementation gate; its
final execution result and hosted gates are recorded in the delivery PR/#180.

Independent review reproduced seven mutation counterexamples against the base
seed, new ordinary checking and explanation mode; all kept the same diagnostics
and omitted stale guard origins. Additional probes covered return-type rejection,
shadowing, nested invalid operands and weaker replacement guards. Review found
mixed numeric-domain operands mislabelled as unproved/unsupported, fixed using
the ordinary compatibility predicate, and an unguarded environment read masking
missing stdlib configuration, fixed with the loader's guarded lookup convention.
Both gained regression tests and were independently rechecked on the refreshed
seed. No actionable findings remained.

The original base seed reproduces the final candidate byte-for-byte. Fresh
stage-2/3/4 compilers converge at
`5df1c9f258cd7607c5c076115ac8e025f14f0da645ed186e4d410736f84d8f56`;
the standard-library artifact remains
`614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`.
The candidate compiler is 308,735 bytes versus 298,386 at base. This is compiler
implementation size, not overhead added to user artifacts; bytecode remains v9.

### Same-workload measurements

Linux x86-64 development container, native VM built with `-O2`. Both seeds compile
an isolated `cbae41b` checkout's `src/compiler/main.panack`, using the same VM and
stdlib. The resulting baseline-program artifacts are byte-identical. Wall-clock
samples include process startup, loading, checking, emission and writing.

| Sample | Base seed | Candidate seed | Interpretation |
| --- | --- | --- | --- |
| Initial three alternating pairs, before review fixes | 22.669 / 21.672 / 21.106s | 22.257 / 19.795 / 20.862s | Medians 21.672 / 20.862s; limited noisy samples |
| Final reviewed-seed pair | 21.331s | 22.583s | +5.9% in this single observation; no general zero-overhead claim |

Final `explain` of the baseline compiler's `condition_facts` function took
17.296s and emitted 445 bytes, including the guarded `value - 1` proof. This
checks the whole compiler project before filtering presentation; it does not
compile or execute it. A small `remaining(n)` guard fixture took
50.7 / 50.0 / 58.8 / 51.3 / 53.0ms (median 51.3ms). No hard explanation latency or
memory guarantee is established by these examples. The existing #106 validation
budget concern remains; no tests were dropped or timing targets relaxed.

Reproduction: preserve the base seed and source checkout; run each seed with
`panack-vm run SEED compile BASE/src/compiler/main.panack -o OUTPUT.bc`, alternating
order and comparing bytes. Run the final CLI's `explain` against the same entry
with `--function condition_facts`. Use the complete example in the query contract
for small-fixture measurements. `make regenerate-seed` independently verifies
fresh compiler and stdlib fixed points; `make check` covers the canonical suites.

The first full U3 run caught a stale runner-smoke aggregate count: adding one
functional case adds its source/compile/bytecode assertions, so the independent
expected total changes from 340 to 343. The case list was already updated; the
footer is now corrected as well. That failed run took 300s (unit 281s) and did
not complete functional/bootstrap acceptance. The final canonical rerun is
reported separately in the delivery PR; no test was skipped or weakened.

## U0 runtime provenance retention — 2026-10-03

The [reproducible experiment](experiments/runtime_provenance/README.md#cost-and-decision)
records five process-CPU samples for bulk VM, single-step baseline, bounded prefix
and bounded ring observation. GCC 13.3.0 `-O2`, Linux x86_64; a 30,000-iteration
scalar/call fixture produces 570,010 events and 1,260,000. Ring medians are
54.612ms versus 27.676ms single-step and 24.764ms bulk. All observed modes produce
the same value. These short synthetic measurements are not production budgets.

At 1,024 events the complete requested provenance allocation is 238,648 bytes,
including fixed shadow frame/stack/local metadata; no retained runtime values
or dynamic rendering allocations. First-run RSS was 4,431,872 bytes in every
mode, a noisy whole-process high-water figure that does not isolate retention.
The linked report defines timing exclusions and bounds, retention trade-offs,
redaction limits and separately estimated production work. No compiler seed,
bytecode/runtime semantics, production source or published feature changes.

Final clean canonical `make check` passed in 151s (unit 114s, functional 4s),
including all 343 functional cases, native retention units, compiler/library
bootstrap fixed points and package/release/quick-start checks. The 120s total
and 15s unit budgets remain #106 concerns. The separate focused experiment passes
65 assertions. Independent review reproduced an out-of-bounds pre-instruction
inspection on a verified function-fallthrough path; the observer now delegates
the bounds check to the VM before dereferencing the instruction. Both scalar
and already-unsupported frontier regressions pass; independent ASan verification
confirms the repair. No production source or seed changed.

## Runtime performance assessment — 2026-10-03

This is a read-only research assessment requested after an external Gemini
performance critique. It records source inspection and existing measurements,
not a new benchmark run, performance guarantee, optimisation commitment or
priority change. The inspected baseline is
[`8b9c45c478b93d111807a0d6d7f6a20458b0aa5d`](https://github.com/sproates/panackelty/tree/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d).
Pinned source links below refer to that revision. The current work record remains
[#141](https://github.com/sproates/panackelty/issues/141), **Idea; unscheduled**,
as recorded in [the roadmap](../ROADMAP.md#performance-and-benchmarking).

### Assessment and limits of the criticism

The critique described performance as adequate for scripting, tool integration
and structural validation, but rejected raw number-crunching, high-throughput
backends and CPU-bound graphics as fundamental fits. Those negative claims are
too categorical, and the positive adequacy claims are also unmeasured. An interpreted, boxed, reference-counted VM with exact arithmetic
can have significant overhead; its importance depends on workload size, value
sizes, operation mix, I/O waits and latency requirements. Equally, the repository
does not yet establish that performance is adequate for representative production
applications. Neither a negative universal verdict nor a positive adequacy claim
is supported by the current small set of measurements.

Exact numerical algorithms are an explicit language design target. Panackelty
deliberately provides exact numeric semantics. Comparing its growing
integers, rational normalization or exact decimals directly with fixed-width
IEEE floating-point arithmetic measures different contracts. A floating-point
graphics kernel is therefore a poor sole acceptance test for the existing exact
numeric design. This does not make graphics performance irrelevant: an integer
image workload with a checked output is a useful controlled probe, while a real
floating-point rendering workload would require a separately selected numeric
and platform contract. Exactness is not a reason to avoid measuring costs or a
license to promise competitive throughput.

See the pinned [numeric specification](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/SPEC.md#values-and-numeric-semantics)
and [rational contract](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/SPEC.md#rational-arithmetic-and-exact-conversions).
These are semantic requirements, not evidence about achievable runtime speed.

### Costs visible in the current implementation

| Source evidence | Likely cost and what remains unmeasured |
| --- | --- |
| [`Value` representation and ownership](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/src/vm/value.h), [allocation/retain/release implementation](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/src/vm/value.c) | Boxed values, allocation and reference-count ownership can dominate tiny operations and produce cache traffic. Their share of real-program cost needs profiles; reference counting alone does not prove a prohibitive slowdown. |
| [Dispatcher, `local_get` and `local_put`](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/src/vm/execute.c) | Stack-machine dispatch adds work per instruction. Local names are found by linear scans with `strcmp`, so larger local sets can add lookup work. Source inspection identifies a hypothesis, not a measured ranking of bottlenecks. |
| [Map/Set builtins](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/src/vm/builtins_collections.c) | Map and Set membership scan sequences; updates can allocate/copy while preserving persistent semantics. Repeated growth or lookup at larger sizes can amplify cost. Small collections can behave differently, and equality cost depends on keys. |
| [`pn_big_mul` and `pn_big_divmod`](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/src/vm/bigint.c) | Multiplication has nested limb loops. Division performs per-limb quotient search and temporary big-integer operations. Numeric magnitude, not just source loop count, controls work and allocation. |
| [`big_gcd` and rational operations](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/src/vm/numeric.c) | Exact rational operations invoke multiplication/division and GCD normalization. Repeated rational work can be expensive, particularly with growing numerators/denominators; input distributions and reduction opportunities matter. |

Array append already has a bounded sharing optimization, described in the
pinned [value model](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/src/vm/VALUE_MODEL.md).
Immutable semantics therefore do not imply copying everything.

These observations justify targeted experiments. They do not demonstrate that a
JIT, new execution engine, tracing collector, numeric-semantic change or broad
rewrite is necessary. Several costs could be reduced within the existing VM.

### Concurrency and networking are bounded capabilities

The existing transport is a finite TCP experiment/development capability, not a
production HTTP stack. The pinned [exchange contract](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/SPEC.md#native-tcp-exchange-development-toolchain)
uses numeric IPv4, sends a bounded request, shuts down the write side and reads
a bounded response until peer EOF. It offers neither DNS nor IPv6, TLS, HTTP
framing or general connection reuse. Source async activation does not itself
spawn parallel work. Supported native development behavior must not be confused
with the older alpha.10 downloads or browser capabilities.

The [finite server contract](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/SPEC.md#native-tcp-server-development-toolchain)
allows **1–256 total admissions** and **1–32 concurrent clients**, with explicit
request/response limits and timeouts. Those admission and concurrency bounds
are different quantities; neither is measured requests per second. The
[server owner](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/src/vm/tcp_server.c)
cooperatively advances bounded child instruction budgets around socket polling.
That is useful for interleaving I/O, but it is not evidence of multicore CPU
parallelism, unlimited service lifetime or production tail-latency guarantees.

A current VM execution context is single-threaded and cooperatively advanced;
separate contexts can be interleaved by a host. The
[resumable dispatcher](https://github.com/sproates/panackelty/blob/8b9c45c478b93d111807a0d6d7f6a20458b0aa5d/src/vm/execute.c)
counts instructions, not elapsed time inside one instruction. A large exact
arithmetic operation, destructor or native callback cannot be preempted by that
instruction budget. Consequently, a CPU-heavy handler could delay other work on
the same pumping thread even when network operations are nonblocking. This is a
specific latency risk to measure, not proof that every I/O workload is slow.

### Existing evidence: one targeted compiler improvement

The [compiler lookup investigation](#compiler-lookup-and-validation-cost--2026-10-02)
already demonstrates that implementation choices can matter materially without
changing exact semantics or replacing the VM. On fixed U1 compiler source, the
recorded final pair fell from **16.7896s to 6.5460s**, producing byte-identical
output. It replaced repeated interpreted declaration scans with per-kind
indexes; native Maps still have linear lookup. This is a measured improvement
for that compiler workload, not proof that all collection access is fast.

Peak child RSS in that pair moved from **103,576 KiB to 103,692 KiB**. One pair
does not establish a memory regression, general allocation bound or broad
speedup distribution. Compiler implementation, source size, startup, test inputs,
cache state and repeated build work are distinct costs. The existing validation
and compiler measurements must not be extrapolated into claims about image
rendering, arbitrary user-program arithmetic, network throughput or service
latency. No new measurement was performed for this assessment.

### Proposed six-group baseline, still unscheduled

The next useful evidence would be a small correctness-checked baseline spanning
these groups, subject to separate scope and priority agreement:

| Group | Representative experiment | Evidence to retain |
| --- | --- | --- |
| Startup and execution | Tiny saved-bytecode program and a bounded pure loop; separate source compilation from execution | Cold/warm startup, wall/CPU time, peak memory, artifact size and exact output |
| Structural validation | Small, medium and large document or AST validation workloads with realistic key distributions; retain the compiler as an additional real workload | Known expected answers, dataset sizes, latency and peak memory; separate compiler compilation and build-validation costs |
| Exact numerics | Small and growing Nat/Int operations, multiplication/division, rational reduction and finite decimals | Magnitudes/bit sizes, operation counts, exact expected values, CPU/allocation or RSS evidence and growth behavior |
| Collections and text | Map/Set lookup and persistent updates across sizes; arrays and Unicode string processing | Correct output, collection/key sizes, hit/miss patterns, sharing behavior and scaling curves |
| Network throughput and tail latency | Finite loopback TCP run with controlled fast, slow and CPU-heavy handlers, within admitted/concurrent bounds | Completed/error counts, throughput, latency distribution including tails, timeouts, resource cleanup and fairness observations |
| Integer image computation | Fixed dimensions and deterministic integer-only pixel calculation, with output checksum | Correct checksum, compute-only versus encoding/I/O time, memory and operation/input dimensions |

Correctness must be checked outside the timed region where possible, while
retaining enough end-to-end checks to detect skipped or changed work. A checksum
validates the chosen image output; it does not establish visual quality or
floating-point rendering performance. Network measurements must disclose the
finite protocol, peer behavior and limits rather than label the result as an
HTTP-server benchmark.

Pin source/seed/runtime revisions, compiler flags, workload inputs and expected
outputs. Record hardware, OS, architecture, power settings where available and
measurement tools. Use repeated samples, alternating revision order where
appropriate, warmups and explicit cold/warm definitions; report variability and
unavailable metrics. Separate compile/startup/run and wall/CPU/peak-memory
quantities. Avoid concurrent benchmark or validation interference. Compare
identical workloads and equivalent semantics, including exactness, overflow,
normalization, Unicode and persistent-collection behavior; otherwise label the
semantic difference instead of presenting a language ranking. Cross-language
results should use comparable exact-number/data contracts and disclose libraries.
Define application-specific acceptance only after observing baseline noise.

### Optimisation hypotheses, not selected implementation

If profiles confirm the relevant costs, candidates include resolved/indexed
local slots in place of repeated string lookup; lower allocation/refcount churn
while preserving ownership; small-number representations with transparent
big-number promotion that preserve exactness; better collection representations with unchanged
persistent/equality behavior; and improved bigint division, multiplication or
normalization algorithms. Dispatch specialization or carefully scoped native
primitives might help a demonstrated hotspot, but need separate safety, code-size
and maintenance evaluation. Network fairness may require budgeting work inside
long operations or an explicitly designed scheduling boundary; instruction
counts alone cannot provide a wall-clock preemption guarantee.

Every candidate needs semantic-equivalence checks, targeted failure tests and
before/after measurements. Preserve exactness, proof rules, runtime verification,
purity/effect boundaries and stable bytecode/CLI contracts unless a separately
approved migration requires otherwise. Choosing silent floating-point
approximation to improve a score would answer a different question.

The resulting position is deliberately evidence-limited: there are concrete
optimisation opportunities and real capability limits; broad practical adequacy
remains unmeasured. This record adds research context to #141 without starting
the six-group suite, selecting an optimisation, closing an issue or changing
programme priority. It changes no release or website capability claim.

## U3 per-function effect recovery — 2026-10-03

The recovery feature retains function validity only in explanation mode; ordinary
compilation keeps its type-first diagnostic gate. Rejected explanation queries
may now run a filtered effect traversal; no extra type-analysis pass is added.
This improves availability, not proof power or transitive effect inference. The
semantic-impact report's original measurements are historical and do not measure
the newly recovered traversal.

Fresh seed stages 2/3/4 converge at compiler SHA-256
`75b4d7cd92596ef6c5be871dfdda243b7ee02ff404f5adba2a21e94b814a6224`
(318,005 bytes), with unchanged standard-library artifact
`614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`.
Focused explanation probes pass 266 unit assertions and 53 public-CLI assertions.
Exact baseline parity against the prior seed holds for all 227 compiler-contract
fixtures: 43 accepted bytecode artifacts and 184 rejections have identical exit
status, stdout and stderr (40.529s comparison). This is correctness evidence, not a
performance comparison.

Clean canonical `make check` passed in 154s: unit 116s, functional 5s (343 cases),
bootstrap 19s, with compiler/library fixed points, seed refresh, release-smoke and
quick-start validation. The updated semantic-impact experiment passes 25 assertions.
The 120s full and 15s unit budgets were exceeded; existing GI#106 remains the
performance reminder. These are environment-specific observations, not an
attributed recovery overhead benchmark. Final informational acceptance edits are
checked with `make docs`.


## Newcomer installation trial — 2026-10-04

GI#245: Installation and Hello World trial / RM#116.
A fresh-context coding agent, configured at low reasoning on the coordinator's
current model (exact runtime model identifier unavailable), followed public GitHub
README/release material unaided. No previous project conversation or solution was
supplied. The supplied entry point was the GitHub project page; marketing-site
navigation and independent discovery of the project were not tested.

Ubuntu 24.04.3 LTS, Linux x86_64, kernel 6.18.44. A fresh directory on a shared host
was used, not a freshly provisioned OS or security-isolated container. Existing
Python, curl, tar and sha256sum were available. Installation was local to the
workspace, using the documented executable path; persistent PATH setup was not
tested. Released alpha.10 was tested, not current development main.

Budget: 30 minutes elapsed / 12 failed install/build/run invocations. Start
2026-10-03 22:33:49 UTC; successful run at 22:36:31 UTC (4 October 00:36:31
Gibraltar), approximately 2m43s including startup and network time. Execution
stopped at success. Zero failed install/build/run commands and zero interventions.
One optional documentation-parser command failed because bs4 was absent; the
participant recovered through raw README retrieval. This is environment/tooling
friction, not a Panackelty defect.

Downloaded v0.1.0-alpha.10 Linux x86_64 from the public release assets and verified
its published SHA-256: `c6cf1f95f141a0de2809d63f0d99cd9cb7f48a47861e59fcb1b265ae6f9dd737`.
Version output: `panack 0.1.0-alpha.10 (bytecode 9)`. Check returned `ok` with exit
0. Run returned exactly `Hello, World!\n`, exit 0, empty stderr; recorded runtime
0.045 seconds. No saved-bytecode workflow was attempted.

```panack
main(): Void {
  print("Hello, World!")
}
```

### Findings and limits

- Minor documentation friction: the README lists Linux and macOS archives but
  shows a macOS-only extraction command. The Linux participant successfully
  substituted its archive filename. Candidate improvement: platform-specific
  copy-paste unpack commands.
- Minor convenience opportunity: add explicit platform download examples. The
  participant used public GitHub release API metadata to obtain the assets.
  This was successful, and does not establish that ordinary release-page
  downloading is difficult for a human.
- The missing optional bs4 dependency is not a product finding. No helper was
  installed and no coaching was needed.

This single AI trial demonstrates one successful local Linux release journey;
it does not establish human beginner usability, macOS support, clean-OS
prerequisite sufficiency, PATH integration or marketing-site usability. Findings
are proposals only; no fixes or further trials are authorised by this report.

### Evidence and reproduction

The following command record preserves timestamps, outputs and failures. README
was read from mutable main; the captured page OID and content hashes identify
observed inputs without claiming atomic consistency between separate requests.
Download the named release and checksum, verify with sha256sum, extract, then
run the version/check/run commands below using the source above. Network timings
are environment-specific, not product benchmarks.

- Captured `README.md` SHA-256: `440ad9053c92649ff5f60b17b957634d1216760dcf0b70f8c44330f80947375b`.
- Captured `project-page.html` SHA-256: `d83fb61757554a7260513692209631ae6d953f65b0cc16dd9a4af7d0afd6c5b1`.
- Captured `release.json` SHA-256: `101369290019fa1c119488d21f39b2a36759a28a59f2b554ba20e6dd899d2dcf`.
- Captured `hello.panack` SHA-256: `440339e999e14637a7d37f70c38e7665840fae031869bfb3b4b8be90db13368d`.
- Captured `commands.jsonl` SHA-256: `b16ebd945bc4f02d91349306a026ef3e98522acd50b2706292498b0f5714a552`.

<details>
<summary>Timestamped participant command evidence</summary>

```jsonl
{"timestamp": "2026-10-03T22:34:34.381656+00:00", "command": "curl -L --fail --max-time 30 https://github.com/sproates/panackelty -o project-page.html", "elapsed_seconds": 5.197386980056763, "exit_code": 0, "stdout": "", "stderr": "  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current\n                                 Dload  Upload   Total   Spent    Left  Speed\n\n  0     0    0     0    0     0      0      0 --:--:-- --:--:-- --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:01 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:02 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:03 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:04 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:05 --:--:--     0\n100  454k    0  454k    0     0  89613      0 --:--:--  0:00:05 --:--:--  108k\n"}
{"timestamp": "2026-10-03T22:34:50.953339+00:00", "command": "python -c 'from bs4 import BeautifulSoup; from pathlib import Path; s=BeautifulSoup(Path(\"project-page.html\").read_text(),\"html.parser\"); a=s.find(\"article\"); print(a.get_text(\"\\n\",strip=True) if a else \"NO ARTICLE\")'", "elapsed_seconds": 0.011940479278564453, "exit_code": 1, "stdout": "", "stderr": "Traceback (most recent call last):\n  File \"<string>\", line 1, in <module>\nModuleNotFoundError: No module named 'bs4'\n"}
{"timestamp": "2026-10-03T22:34:56.805652+00:00", "command": "curl -L --fail --max-time 30 https://raw.githubusercontent.com/sproates/panackelty/main/README.md -o README.md", "elapsed_seconds": 5.0760862827301025, "exit_code": 0, "stdout": "", "stderr": "  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current\n                                 Dload  Upload   Total   Spent    Left  Speed\n\n  0     0    0     0    0     0      0      0 --:--:-- --:--:-- --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:01 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:02 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:03 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:04 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:04 --:--:--     0\n100 33823  100 33823    0     0   6672      0  0:00:05  0:00:05 --:--:--  8316\n"}
{"timestamp": "2026-10-03T22:35:12.723433+00:00", "command": "sed -n '1,250p' README.md", "elapsed_seconds": 0.0026922225952148438, "exit_code": 0, "stdout": "# Panackelty\n\n**A small, expressive programming language for dependable terminal tools and\nexact numerical work.**\n\n[Website](https://panackelty.com) \u00b7 [Native VM coverage](https://sproates.github.io/panackelty-coverage/) \u00b7 [Specification](SPEC.md) \u00b7\n[Releases](../../releases) \u00b7 [Contributing](CONTRIBUTING.md)\n\nThe website consumes a pinned release from\n[Panackelty Browser](https://github.com/sproates/panackelty-browser), selected in\n`site/playground.json`. Browser sources, builds and tests live in that repository.\nPages runs its pinned integration suite against the complete website before\ndeployment. Native development requires neither WASI SDK nor JavaScript packages.\n\nPanackelty combines arbitrary-precision numbers, checked domain types, explicit\neffects, and a portable bytecode VM. Its syntax stays compact enough for a quick\nscript while its compiler catches the mistakes that become expensive when a\nprogram grows.\n\n<p align=\"center\">\n  <img src=\"docs/assets/panackelty.jpg\" width=\"800\" alt=\"A home-cooked plate of panackelty with beef, potatoes, carrots, and peas in gravy\">\n</p>\n<p align=\"center\">\n  <sub>\n    A home-cooked plate of panackelty, the North East English dish behind the\n    name. Photograph by the project author,\n    <a href=\"docs/assets/README.md\">licensed under CC BY 4.0</a>.\n  </sub>\n</p>\n\n## Exact fractions and Unit\n\n```panackelty\nmain(): Void {\n  third = 1/3\n  ten = third * 30\n  print(ten.nat())  // 10\n  print((1/8).dec()) // 0.125\n  success: Unit = ()\n  print(success)   // ()\n}\n```\n\nInteger `/` returns `Rat`;\n`quotient(a, b)` retains truncating natural division. Exact conversions trap if\nthe value cannot be represented: `(1/3).nat()` and `(1/3).dec()` fail rather than\nround. `Unit` is a first-class value for APIs such as `Result[Unit,Str]`.\nSee [the specification](SPEC.md#rational-arithmetic-and-exact-conversions).\n\n## Typed paths and elapsed time\n\nPanackelty includes opaque `Path`, exact signed nanosecond\n`Duration`, and monotonic `Instant` values. Import `stdlib/path` and\n`stdlib/time`. Path construction checks text or native bytes; lexical operations\npreserve spelling and do not access the filesystem. Durations support exact\narithmetic and checked fractional conversions. `instant_now()` is effectful\nand returns a `Result`; arithmetic on existing clock readings is pure.\n\nTyped filesystem and process APIs are available through `stdlib/filesystem`\nand `stdlib/process`. They accept `Path` values and return structured errors.\nProcess execution supports byte streams, working directories, environment\noverrides, output limits, and exact-duration timeouts; `host_sleep` provides\nchecked sleep. See the [host API contract](SPEC.md#typed-filesystem-process-and-sleep-apis)\nand [runnable example](tests/functional/cases/host_capabilities/main.panack).\n\nThe [API specification](SPEC.md#paths-and-monotonic-time) and\n[executable example](tests/functional/cases/host_types/main.panack) show their\ncontracts and use.\n\n## Experimental async source support\n\nThe alpha.10 toolchain supports `async` functions, explicit `await` and\n`AsyncFn` references with a fixed fake read service. The\n[executable example](tests/functional/cases/async_await/main.panack) checks typed\nsuccess/error results and nested suspension. This is a bounded language/runtime\nslice, not networking support; source spawning and resource scopes remain future\nwork. It uses bytecode v9, so saved v8 programs need recompilation.\n\nThe development checkout additionally supports a bounded native TCP client:\n\n```panack\nasync request(): Result[Bytes,Str] {\n  await tcp_exchange(\"127.0.0.1\", 9000, utf8_encode(\"hello\"), 4096, 2000)\n}\n```\n\nIt sends the request, closes the sending side, waits for the peer's response EOF,\nand returns bytes or an error. See the [runnable echo-client example](examples/network/tcp_exchange.panack)\nand [TCP contract](SPEC.md#native-tcp-exchange-development-toolchain).\nThis feature is not in the alpha.10 downloads. It supports numeric IPv4 on\nLinux/macOS, with response limits and timeouts. The development checkout also\nsupports [finite concurrent TCP servers](examples/network/tcp_serve.panack):\nimport `stdlib/tcp`, supply explicit `TcpServerLimits`, and await `tcp_serve`\nwith a named async byte-request handler. The runtime owns accepted connections,\nschedules handlers and drains or cancels clients on shutdown. See the\n[server contract](SPEC.md#native-tcp-server-development-toolchain).\nDNS, TLS, HTTP and general source spawning remain future work. Neither native\nnetworking feature is in alpha.10; the browser cannot open raw TCP sockets.\n\n## Start writing Panackelty\n\nThe developer preview is designed to be downloaded and run directly. The\ndownload contains everything needed to check, compile, and run programs: the\n`panack` command, native VM, self-hosted compiler, and standard library.\n\nDeveloper-preview archives for `0.1.0-alpha.10` are available from the\n[GitHub release](https://github.com/sproates/panackelty/releases/tag/v0.1.0-alpha.10).\nThe [release policy](RELEASE_POLICY.md) defines the preview's support and\ncompatibility boundaries.\n\n### Upgrading from alpha.9\n\nThis preview deliberately changes source APIs and saved bytecode. Recompile v8\nbytecode from source with alpha.10; the VM accepts bytecode v9 only.\n\n`Option`, `Result` and their constructors are now available without imports.\nRemove `stdlib/text` and `stdlib/collections` imports and use methods such as\n`text.ends_with(suffix)`, `values.first()` and `values.sort_by(@comparator)`.\nThe option/result modules remain available for explicitly imported value-or\nhelpers. See the [migration notes](CHANGELOG.md#010-alpha10--2026-09-30).\n\n### System requirements\n\nThe initial preview supports:\n\n- Linux on x86-64\n- macOS on Apple silicon (arm64)\n- A terminal and `tar` for unpacking the download\n- `sha256sum` on Linux or `shasum` on macOS for verifying the download\n\nDownload both the archive matching the operating system and processor and its\nadjacent `.sha256` file from the Releases page:\n\n| System | Archive |\n| --- | --- |\n| Linux x86-64 | `panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz` |\n| macOS arm64 | `panackelty-0.1.0-alpha.10-macos-arm64.tar.gz` |\n\nWindows and other architectures are not part of the initial preview.\n\n### Install a downloaded release\n\nIn the directory containing both downloaded files, verify the archive. On\nLinux, run:\n\n```sh\nsha256sum -c panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz.sha256\n```\n\nOn macOS, run:\n\n```sh\nshasum -a 256 -c panackelty-0.1.0-alpha.10-macos-arm64.tar.gz.sha256\n```\n\nThe command must report the archive as `OK`. Then unpack the matching archive;\nthe macOS name is shown here:\n\n```sh\ntar -xzf panackelty-0.1.0-alpha.10-macos-arm64.tar.gz\n./panackelty/bin/panack --version\n```\n\nThe version command prints `panack 0.1.0-alpha.10 (bytecode 9)`. The Linux\narchive follows the same layout and uses `linux-x86_64` in its name. To make\n`panack` available in future terminal sessions, keep the whole extracted\ndirectory together and link its command into a directory on `PATH`:\n\n```sh\nmkdir -p \"$HOME/.local/opt\" \"$HOME/.local/bin\"\nmv panackelty \"$HOME/.local/opt/panackelty\"\nln -s \"$HOME/.local/opt/panackelty/bin/panack\" \"$HOME/.local/bin/panack\"\nexport PATH=\"$HOME/.local/bin:$PATH\"\n```\n\nAdd the final `export` command to the shell's startup file if `~/.local/bin` is\nnot already on `PATH`. The complete toolchain remains under\n`~/.local/opt/panackelty`.\n\n### Write and run a first program\n\nSave this as `hello.panack`:\n\n<!-- quick-start-program-begin -->\n```panackelty\npure greeting(name: Str, answer: Nat): Str {\n  \"Hello, ${name}. The answer is ${answer}.\"\n}\n\nmain(): Void {\n  print(greeting(\"Ada\", 42))\n}\n```\n<!-- quick-start-program-end -->\n\nCheck the program, run its source, compile it to `hello.bc`, and run the saved\nbytecode:\n\n```sh\npanack --version\npanack check hello.panack\npanack run hello.panack\npanack compile hello.panack\npanack run hello.bc\n```\n\nPrimary lexer, parser, name, and type failures identify their owning source as\n`file:line:column`, including when the error is in an imported module.\nPositioned errors also show the source line and a caret. Tabs expand to\nfour-column stops; Unicode and control characters appear as `\\u{hex}` escapes\nto keep the caret aligned.\n\nThe alpha.10 toolchain produces the output below.\n\n<!-- quick-start-output-begin -->\n```text\npanack 0.1.0-alpha.10 (bytecode 9)\nok\nHello, Ada. The answer is 42.\nwrote hello.bc\nHello, Ada. The answer is 42.\n```\n<!-- quick-start-output-end -->\n\n`panack hello.panack` is shorthand for `panack run hello.panack`. Functions need\nno `fn` keyword, and the final expression in a non-`Void` function is its return\nvalue.\n\n### Upgrade or remove Panackelty\n\nTo upgrade, download and verify the new archive, extract it in a working\ndirectory, and replace the installed directory while keeping the command link:\n\n```sh\nmv panackelty \"$HOME/.local/opt/panackelty.new\"\nmv \"$HOME/.local/opt/panackelty\" \"$HOME/.local/opt/panackelty.old\"\nmv \"$HOME/.local/opt/panackelty.new\" \"$HOME/.local/opt/panackelty\"\npanack --version\nrm -rf \"$HOME/.local/opt/panackelty.old\"\n```\n\nDo not begin if either temporary path already exists. If the new version does\nnot run correctly, move `panackelty.old` back before deleting it. To remove\nPanackelty completely:\n\n```sh\nrm \"$HOME/.local/bin/panack\"\nrm -rf \"$HOME/.local/opt/panackelty\"\n```\n\n", "stderr": ""}
{"timestamp": "2026-10-03T22:35:18.671386+00:00", "command": "curl -L --fail --max-time 30 https://api.github.com/repos/sproates/panackelty/releases/tags/v0.1.0-alpha.10 -o release.json", "elapsed_seconds": 5.316051006317139, "exit_code": 0, "stdout": "", "stderr": "  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current\n                                 Dload  Upload   Total   Spent    Left  Speed\n\n  0     0    0     0    0     0      0      0 --:--:-- --:--:-- --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:01 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:02 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:03 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:04 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:04 --:--:--     0\n100 16725    0 16725    0     0   3151      0 --:--:--  0:00:05 --:--:--  3885\n"}
{"timestamp": "2026-10-03T22:35:43.055632+00:00", "command": "python -c 'import json; r=json.load(open(\"release.json\")); print(r[\"tag_name\"],r[\"target_commitish\"]); print(\"\\n\".join(a[\"browser_download_url\"] for a in r[\"assets\"])); print(r[\"body\"])'", "elapsed_seconds": 0.01822662353515625, "exit_code": 0, "stdout": "v0.1.0-alpha.10 main\nhttps://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.10/panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz\nhttps://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.10/panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz.provenance\nhttps://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.10/panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz.sha256\nhttps://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.10/panackelty-0.1.0-alpha.10-macos-arm64.tar.gz\nhttps://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.10/panackelty-0.1.0-alpha.10-macos-arm64.tar.gz.provenance\nhttps://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.10/panackelty-0.1.0-alpha.10-macos-arm64.tar.gz.sha256\nPanackelty 0.1.0-alpha.10\n\n- Saved bytecode moves from v8 to v9. Recompile existing `.bc` files from source;\n  the VM rejects v8 artifacts. Source checking, the compiler seed and runtime\n  are packaged together.\n- `Option[T]`, `Result[T,E]` and their constructors are implicit. Remove imports\n  used only for these types; their names can no longer be redeclared.\n- Remove `stdlib/text` and `stdlib/collections` imports. Replace imported\n  `text_*` helpers with methods on strings, `array_first(values)` with\n  `values.first()`, and `array_sort_by(values, @compare)` with\n  `values.sort_by(@compare)`. The option/result modules retain explicitly\n  imported `option_value_or` and `result_value_or` helpers.\n\n- Text and collection methods are available without imports, including\n  `.ends_with()`, `.first()` and stable `.sort_by()`. Standard method names\n  cannot be captured by unrelated global functions. Suffix matching is literal\n  and case-sensitive; sorting requires a pure comparator.\n- Add experimental `async`/`await`, `AsyncFn` references and typed fake-read\n  completions, with checked effects and nested suspension. This is a bounded\n  experiment: no real networking, source spawning or resource scopes.\n- Replace native-recursive calls with owned resumable VM frames and an internal\n  budgeted execution API. Add fake-host task/lifecycle experiments with scoped\n  joins, bounded completion queues, virtual deadlines and cancellation cleanup.\n- Amortise persistent array append while preserving immutable value semantics.\n\n- Add a browser playground using the existing compiler and VM through\n  WebAssembly, with nine editable examples and a visual explanation of execution.\n  Source compilation and execution stay on the user's device. Host capabilities\n  remain more limited than the downloadable native toolchain.\n- Version the complete playground asset set together to avoid mixing cached\n  examples, workers, compiler and standard library across deployments.\n- Expand the website's examples, installation guide and engineering information;\n  publish native C coverage reports alongside it.\n- Reduce duplicated validation work, reuse safe build artifacts, overlap suites\n  and partition hosted checks. Keep unit, functional, bootstrap and exact-package\n  gates, with additional browser checks in Chromium, Firefox and WebKit.\n", "stderr": ""}
{"timestamp": "2026-10-03T22:35:50.737420+00:00", "command": "curl -L --fail --max-time 60 https://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.10/panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz.sha256 -o panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz.sha256", "elapsed_seconds": 9.257946014404297, "exit_code": 0, "stdout": "", "stderr": "  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current\n                                 Dload  Upload   Total   Spent    Left  Speed\n\n  0     0    0     0    0     0      0      0 --:--:-- --:--:-- --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:01 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:02 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:03 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:04 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:04 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:04 --:--:--     0\n\n  0     0    0     0    0     0      0      0 --:--:--  0:00:05 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:06 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:07 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:08 --:--:--     0\n100   112  100   112    0     0     12      0  0:00:09  0:00:09 --:--:--    31\n"}
{"timestamp": "2026-10-03T22:35:50.737130+00:00", "command": "curl -L --fail --max-time 60 https://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.10/panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz -o panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz", "elapsed_seconds": 11.718363523483276, "exit_code": 0, "stdout": "", "stderr": "  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current\n                                 Dload  Upload   Total   Spent    Left  Speed\n\n  0     0    0     0    0     0      0      0 --:--:-- --:--:-- --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:01 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:02 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:03 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:04 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:05 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:05 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:05 --:--:--     0\n\n  0     0    0     0    0     0      0      0 --:--:--  0:00:06 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:07 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:08 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:09 --:--:--     0\n  0     0    0     0    0     0      0      0 --:--:--  0:00:10 --:--:--     0\n  0  141k    0     0    0     0      0      0 --:--:--  0:00:11 --:--:--     0\n100  141k  100  141k    0     0  12378      0  0:00:11  0:00:11 --:--:-- 38135\n"}
{"timestamp": "2026-10-03T22:36:13.394533+00:00", "command": "sha256sum -c panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz.sha256", "elapsed_seconds": 0.0027685165405273438, "exit_code": 0, "stdout": "panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz: OK\n", "stderr": ""}
{"timestamp": "2026-10-03T22:36:29.922122+00:00", "command": "tar -xzf panackelty-0.1.0-alpha.10-linux-x86_64.tar.gz", "elapsed_seconds": 0.008266210556030273, "exit_code": 0, "stdout": "", "stderr": ""}
{"timestamp": "2026-10-03T22:36:30.299451+00:00", "command": "./panackelty/bin/panack --version", "elapsed_seconds": 0.0034101009368896484, "exit_code": 0, "stdout": "panack 0.1.0-alpha.10 (bytecode 9)\n", "stderr": ""}
{"timestamp": "2026-10-03T22:36:30.702145+00:00", "command": "python -c 'from pathlib import Path; Path(\"hello.panack\").write_text(\"main(): Void {\\n  print(\\\"Hello, World!\\\")\\n}\\n\"); print(Path(\"hello.panack\").read_text())'", "elapsed_seconds": 0.018817663192749023, "exit_code": 0, "stdout": "main(): Void {\n  print(\"Hello, World!\")\n}\n\n", "stderr": ""}
{"timestamp": "2026-10-03T22:36:31.091310+00:00", "command": "./panackelty/bin/panack check hello.panack", "elapsed_seconds": 0.03743314743041992, "exit_code": 0, "stdout": "ok\n", "stderr": ""}
{"timestamp": "2026-10-03T22:36:31.496069+00:00", "command": "./panackelty/bin/panack run hello.panack", "elapsed_seconds": 0.04516744613647461, "exit_code": 0, "stdout": "Hello, World!\n", "stderr": ""}
{"timestamp": "2026-10-03T22:37:06.733414+00:00", "command": "rg -o \"currentOid.{0,90}\" project-page.html", "elapsed_seconds": 0.02014946937561035, "exit_code": 0, "stdout": "currentOid\":\"5cab3f19bddce5fa287159517b93c1b114e491b7\"},\"tree\":{\"items\":[{\"name\":\".agents/skills\",\"p\ncurrentOid\":\"5cab3f19bddce5fa287159517b93c1b114e491b7\"},\"helpUrl\":\"https://docs.github.com\",\"githubD\n", "stderr": ""}
```

</details>

Independent verification at 22:38:11 UTC reproduced version, check and run from a
separate directory against byte-identical source: all exits 0, empty stderr,
exact greeting bytes `48656c6c6f2c20576f726c64210a`. All 46 regular archive files
matched the installation; all 10 participant evidence-manifest hashes matched.
Archive hash also matched saved release metadata. Signed build provenance was
not verified. VM SHA-256:
`cd7a46282ae771f741db9cd473bc11809a469415c904a710251d70ef03464338`.
Independent verification record SHA-256:
`a16a91172b64681e1c1c4029bcddd80ba1cc15feb980dcf940b2b19952ab861f`.


## Platform installation instructions — 2026-10-03

[RM#117: Platform-specific installation instructions](../ROADMAP.md#rm-117) /
[GI#249: Platform-specific installation instructions](https://github.com/sproates/panackelty/issues/249)
implements the download/extraction follow-up from
[RM#116: Installation and Hello World trial](../ROADMAP.md#rm-116) /
[GI#245: Installation and Hello World trial](https://github.com/sproates/panackelty/issues/245).
The earlier trial record above is unchanged; this is author verification, not a
new independent newcomer trial.

Fresh release API inspection confirmed the published `v0.1.0-alpha.10` Linux
x86_64 and macOS arm64 archive/checksum names used by the README. On Ubuntu
24.04.3 x86_64, `sh tests/release_install_readme.sh` downloaded the real Linux
archive and checksum into a fresh temporary directory and executed the marked
README blocks without substituting their URLs or commands. SHA-256 reported
`OK`; the local executable produced this exact transcript:

```text
panack 0.1.0-alpha.10 (bytecode 9)
ok
Hello, Ada. The answer is 42.
wrote hello.bc
Hello, Ada. The answer is 42.
```

The optional home-directory move and command link also returned the expected
version using an isolated temporary home. No source build or global installation
was used for this released-archive check. The test extracts the current README
program, commands and expected output, and cleans its temporary files.

Six local failure probes executed the unchanged Linux and macOS installation
blocks with controlled curl failures on the first or second download, or a
mismatching checksum. All exited nonzero before invoking tar. The macOS shell
block and `shasum` failure probe on Linux do **not** establish macOS native support.
Hosted Check runs the real-release test once in each existing platform's
`conformance-bytecode` job. On 2026-10-03 UTC, both native hosted platform jobs
passed the exact README installation step at head `1607f577623ad02d4298ca73a258556a792d264e`:
[Linux x86_64](https://github.com/sproates/panackelty/actions/runs/37160525736/job/111312898639)
and [macOS arm64](https://github.com/sproates/panackelty/actions/runs/37160525736/job/111312898647).
This online check is deliberately separate
from the offline canonical `make check`.

README minimum OS versions agree with `RELEASE_POLICY.md`; alpha.10 migration,
archive layout and upgrade/removal guidance remain applicable. Website inspection
found truthful alpha.10 links, separate platform extraction and local check/run
paths. Optional direct-download/fail-fast parity is recorded as unscheduled
[RM#118: Website installation command parity](../ROADMAP.md#rm-118), without
changing the live site, release pins or original trial evidence.

Canonical `make check` passed after the README harness assertion was updated to
expect the direct local executable path: 119s total, unit 84s, functional 4s,
with bootstrap, release smoke and packaged quick start passing. The unit phase
exceeded its 15s budget; the existing RM#28 / GI#106 reminder records this sample.
The first clean attempt exposed the outdated PATH-only text assertion; no
coverage was removed to resolve it. The successful run reused that build.


## Website installation command parity — 2026-10-04

[RM#118: Website installation command parity](../ROADMAP.md#rm-118) /
[GI#250: Website installation command parity](https://github.com/sproates/panackelty/issues/250)
updates `site/index.html#start` to match the README delivered by
[PR#251: Platform-specific installation instructions](https://github.com/sproates/panackelty/pull/251).
Native alpha.10 and `site/playground.json` remain unchanged.

Decoded both HTML platform code blocks and compared them byte-for-byte with the
README's marked installation blocks: Linux x86_64 and macOS arm64 both match.
The website greeting matches the README program apart from surrounding blank
lines. Executed the website's Linux download/verification/extraction/version,
local check/run and compile/bytecode blocks against a freshly downloaded public
alpha.10 archive in a temporary directory on Ubuntu 24.04 x86_64. Every command
returned zero, checksum reported `OK`, version was
`panack 0.1.0-alpha.10 (bytecode 9)`, check printed `ok`, and both execution paths
printed `Hello, Ada. The answer is 42.`. The temporary installation was removed.
The identical macOS commands already passed native macOS arm64 CI in PR#251,
linked in the preceding installation record; no new local macOS execution is
claimed. That record also covers the six fail-before-extraction probes for these
same shell blocks.

`bash scripts/validate_change.sh --run origin/main` selected `route=website`:
document/link/whitespace checks, all 38 website automation tests and Pages
assembly/failure handling passed. The website-only route deliberately requires
no new native `make check`. Hosted release-integrity and browser checks, visual
preview acceptance before merge, and live verification after authorised deployment
remain required. A clean preview can be built with
`node scripts/preview.cjs build /absolute/fresh/output`; see
[the preview workflow](../docs/PR_PREVIEWS.md) for private phone-accessible review.
Neither a workspace HTTP check nor this local test report establishes iPhone
access or user visual approval. The delivery agent owns those outstanding checks
and must keep GI#250 open until live acceptance is recorded.

### Website installation live acceptance

PR#253 merged as `81b5a4d7a2b901bd27135afd96c1839d4c70c3c3` after
owner preview approval, independent source review and successful current-head
checks. Main Check 37163114763 and production Pages 37163232362 succeeded.
On 2026-10-04 (Europe/Gibraltar), a fresh fetch of `https://panackelty.com/`
with a verification query confirmed that the complete `#start` section matched
merged `site/index.html` exactly, including both release download/checksum links,
alpha.10 labels, fail-fast blocks and local first-program commands.
This completes GI#250 / RM#118 acceptance.


## Optional installer and website follow-up — 2026-10-04

[PR#258: Optional one-command installer](https://github.com/sproates/panackelty/pull/258)
merged as `28f36052034ab6cb94bba23553b1017f744ad62d`.
[Main Check 37169413177](https://github.com/sproates/panackelty/actions/runs/37169413177)
succeeded, including the actual published-release installer acceptance on
[Ubuntu x86_64](https://github.com/sproates/panackelty/actions/runs/37169413177/job/111339164694)
and [macOS arm64](https://github.com/sproates/panackelty/actions/runs/37169413177/job/111339164685).
These jobs execute the merged installer against alpha.10, verify the exact README
Hello World/check/source-run/compile/saved-bytecode output, repeat installation
and remove the installer-owned files.

After merge, the delivery maintainer fetched the public raw `main/scripts/install.sh`
and verified that its bytes match the merged reviewed script, then executed the
documented public one-command installation and complete greeting workflow on Linux.
The macOS evidence is the merged-script CI acceptance above; a separate public
shell-pipeline invocation on macOS is not claimed. The stale README availability
note is therefore removed. The installer remains optional; native release version,
platform support and all manual installation steps remain unchanged.

The RM#122 website follow-up adds the optional command, script inspection and
setup-guide links, explicit command-path substitution and opt-in PATH instructions.
The website's new installer command matches the README exactly; the text of all
manual command blocks is byte-for-byte unchanged from merged `28f3605`. Copy
controls wrap those blocks without changing their command text.
All 38 website automation tests and Pages assembly/failure handling pass locally.
Owner preview acceptance, successful current-head hosted checks and authorised
merge are required. The delivery maintainer must verify the live command, links
and preserved manual instructions after deployment before recording RM#121/RM#122
Done or completing GI#256. This record does not claim live website acceptance.

Canonical `make check` also passed for the follow-up in 152s: unit 115s,
functional 5s, bootstrap 18s, release smoke and packaged quick start passed.
The full-check and unit budget warnings remain under the existing GI#106
profiling reminder; no coverage was removed. Documentation/link checks passed.

The installer command also has a labelled, keyboard-focusable copy button with
visible live-region feedback. Its actual inline event handler passed focused
checks for exact README command text, disabled/pending state, resolved writes,
rejected writes and an unavailable clipboard API; these use simulated DOM and
clipboard objects, not browser acceptance. The incremental HTML/CSS change takes
the website validation route; all 38 automation tests and Pages assembly passed.
Local Playwright Chromium installation failed because its CDN download returned
a truncated/non-ZIP archive, so real clipboard permission, keyboard interaction
and narrow-screen acceptance remain for the browser preview. The button reports
success only after the write resolves and otherwise offers manual copying.


The shared copy control now covers all six installation-guide code boxes plus
the greeting program and its output. Each of the eight controls has its own
accessible name, target and live feedback region. All displayed `pre` text,
including multiline commands, indentation and trailing newlines, matches the
preceding revision byte-for-byte; only wrappers and controls change. Focused
checks execute the shared handler for every target and verify exact copied text,
pending/success/reset behavior and independent failure feedback. These remain
simulated DOM/clipboard checks; real phone/browser clipboard and layout acceptance
remain pending as described above. The incremental website gate again passes
all 38 automation tests and Pages assembly/failure handling.


### Installer and website final acceptance — 2026-10-04

This final record supersedes the pending acceptance statements above.
PR#259 merged as `3ee554fbb532120e996f332f1c9a07c3b2592f39` after
current-head checks passed or were intentionally skipped and independent review
of tree `db9918c643a8f06d31b1cffc5ede06d6b2354c56` found no blockers.
[Main Check](https://github.com/sproates/panackelty/actions/runs/37171515647)
and [production Pages](https://github.com/sproates/panackelty/actions/runs/37171626046)
completed successfully. A fresh HTTPS retrieval from `https://panackelty.com/`
matched merged `site/index.html` byte-for-byte, including all eight copy controls,
manual commands and optional installer links. The public raw installer likewise
matched merged `scripts/install.sh`. Earlier post-merge native release acceptance
passed on Linux and macOS; the isolated public-script Linux Hello World/check/
run/compile/bytecode/removal workflow also passed.

After receiving the live installer and multiline copy/paste plus phone layout
check instructions, the owner responded “Lgtm”. This is owner acceptance, not a
claim that the unavailable local browser automation was run. RM#121 and RM#122
are complete; this informational update closes GI#256 on merge. Existing manual
installation remains supported. No executable or website source changes here.
