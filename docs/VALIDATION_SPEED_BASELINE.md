# GI#106: validation baseline and proposal

Measured 27 October 2026. **Owner approved the selection and targets on 27 October 2026. V2 implementation
is in progress; measured improvement and final acceptance remain outstanding.**
GI#106 remains open through implementation, comparable replay and explicit owner
acceptance. This is the bounded alpha.15 milestone, not the wider issue backlog.

## Five local workloads

Every measured command passed. All runs use two validation workers on the same
checkout and machine, sequentially in the order below. Times are enclosing
`/usr/bin/time -p` wall seconds, not sums of nested profile rows.

| Workload | Command | Seconds | Real cached-probe compilations |
| --- | --- | ---: | ---: |
| Clean full validation, after `make clean` | `make -j1 check` | 1245.71 | 42 |
| Unchanged full repeat | `make -j1 check` | 821.77 | 1 |
| Small compiler edit | `make -j1 check-compiler` | 534.43 | 27 |
| Same compiler edit, unchanged repeat | `make -j1 check-compiler` | 300.48 | 0 |
| Small package-tool edit | `make -j1 package-tool-check` | 53.10 | 5 |
| Same package edit, unchanged repeat | `make -j1 package-tool-check` | 31.30 | 0 |
| Documentation-only edit, three repetitions | `make docs` | 4.21 / 4.25 / 4.25 | 0 |

The compilation column counts actual `probe-build/` invocations, excluding the
synthetic compiler used by `harness/probes`. It is **not a census of every compiler
invocation**: direct CLI compilation, native C compilation, later bootstrap
stages and the package handoff test's private profile are outside that category.
The one warm-full probe compilation is the isolated bootstrap stage-2 compiler;
the ordinary compiler artifact is reused. Package-only checks build the package
tool and affected package probes, but never build the compiler or seed. Their
handoff test also deliberately builds an edited package-tool copy in isolation.

The unchanged full run saves 34.0% against clean. The existing cache works:
compiled artifacts are reused, while test assertions execute afresh. These are
single full-run samples, not statistical claims about every machine.

### Exact edits and cache sequence

1. Start from revision `1070ff775bf17c3cd826ff49a1529c5fa41dcf1d`. Its executable
   sources match `next` at `441229f09ce139130f620f782dad7b9ffa5e0c31`.
2. Run `make clean`, then the full check and unchanged repeat without clearing
   artifacts between them.
3. In `src/compiler/lexer.panack`, change the increment in `scan_digits` from
   `cursor = cursor + 1` to `cursor = 1 + cursor`. Run the two compiler checks.
   This equivalent function-body edit changes the real compiler dependency
   closure. Restore the exact original source afterwards.
4. In `src/packages/tar_header.panack`, change the increment in `tar_text` from
   `stop = stop + 1` to `stop = 1 + stop`. Run both package checks, then restore.
   Package checks use the seed, independently of the compiler artifact retained
   from the preceding workload.
5. Append `<!-- Documentation-only baseline measurement. -->` to
   `docs/NEXT_RELEASE_STATUS.md`, run `make docs` three times, then restore.

No temporary source edit is included in this delivery. Keep the same sequence
for the comparison; restoring a source's bytes does not reset the artifact cache.

For each run, use absolute paths outside the checkout for profile output:

```sh
/usr/bin/time -p env VALIDATION_JOBS=2 \
  VALIDATION_PROFILE_FILE="$output/profile.tsv" \
  VALIDATION_TIMINGS_FILE="$output/timings.tsv" \
  VALIDATION_PROFILE_RUN="$sample" \
  make -j1 check > "$output/$sample.log" 2> "$output/$sample.stderr"
```

Substitute the command from the table for the other workloads. Set `output` to
an existing directory and `sample` to a distinct label. The full local raw logs,
profiles, exact patches and environment are retained on the measurement host in
`.panackelty-ops/gi106-baseline-2026-10-27/`, outside the repository. The numerical
baseline and reproduction protocol are preserved here independently of that
local evidence directory.

## Where the time goes

| Observation, clean full run | Inclusive seconds | Interpretation |
| --- | ---: | --- |
| Unit phase | 887 | Includes preparation, harness, compiler and runtime checks |
| Functional phase | 240 | Fresh end-to-end assertions |
| Bootstrap phase | 254 | Overlaps unit; do not add it to unit duration |
| Native staging within bootstrap | 104 | Deliberate isolated rebuild proof |
| Real cached-probe compilation | 445 | Spread across probes and concurrent phases |
| Harness overall | 198 | Includes runner and distribution below |
| Harness runner checks | 118 | Fresh fixture-runner behavior and negative cases |
| Distribution checks | 60 | Temporary installation/package/relocation checks |
| Native oracle fixture-runner smoke | 151 | Executes the fixture corpus internally |
| HTTP client functional checks | 86 | Includes fixture preparation and source/bytecode tests |
| HTTP package functional checks | 49 | Fresh service/client tests |
| Final installed release smoke | 18 | Separate from the distribution harness invocation |

These rows overlap. Profile durations are whole seconds; zero means below the
clock's resolution, not no work. `probe-run` can itself launch child compilation,
so its entire duration is not necessarily pure assertion time. A
`bootstrap-build` recipe can visit an already-cached artifact; use the nested
`probe-build` event to identify actual compilation.

The compiler test programs provide the clearest bounded opportunity:

| Probe | Compile | Execute |
| --- | ---: | ---: |
| Checker | 29s | 1s |
| Module bindings | 44s | 4s |
| Instruction sources | 33s | 1s |
| Source inventory | 29s | <1s |

Several independently compiled programs include much of the same compiler.
After the small lexer edit, 27 real probe compilations took 425 inclusive seconds;
the unchanged compiler repeat compiled none. Their enclosing times were 534.43s
and 300.48s because compilation overlaps and both runs still execute tests.

The warm full unit phase took 501s, against 887s cold; functional took 237s,
against 240s cold. Compilation improvements alone cannot remove the substantial
execution, bootstrap and installation costs.

## Hosted corroboration

[Validation profile run 40418980260](https://github.com/sproates/panackelty/actions/runs/40418980260)
passed on the same source revision. The existing workflow measures representative
focused workloads; it is not a hosted replay of all five local workloads.
Native prerequisites are built before these measurements. Component order is
compiler, bytecode, VM; each initial check is immediately followed by its cached
repeat. Later components inherit earlier artifacts.

| Platform | Compiler initial / cached | Bytecode initial / cached | VM initial / cached |
| --- | ---: | ---: | ---: |
| macOS ARM64 | 975s / 493s | 70s / 52s | 327s / 326s |
| Linux x86_64 | 983s / 480s | 71s / 49s | 415s / 414s |

Dispatch was `2026-10-26T23:19:42Z`; both jobs started at `23:19:45Z` (3s delay).
The macOS job ran 37m43s, including 37m23s in its six-check measurement step.
Linux ran 41m04s, including 40m12s in that step. Queueing is separate from these
execution times. Artifacts contain `validation-profile.tsv`,
`validation-timings.tsv` and `profile-environment.txt` for each platform.

macOS used image `macos14`, version `20260831.0302.1`, Apple clang 15.0.0.
Linux used `ubuntu22`, version `20261023.322.3`, GCC 11.4.0. Both used
`VALIDATION_JOBS=2`. Hosted and local absolute durations are not interchangeable.

## Measurement limits

Local environment: macOS 26.5 (25F71), Darwin arm64, Apple clang 21.0.0,
GNU Make 3.81. Seed SHA-256:
`aab8b0557c9525ac1e917d093e1c44452ba746fde59d880b7ea3e5c2a24509df`.
The ordinary default is four validation workers; these measurements explicitly
use the existing two-worker profile configuration. Do not present them as timings
for the default setting. Repeat the default configuration as an additional
acceptance guard before recommending a development workflow change.

This was an active desktop, not a dedicated idle benchmark machine. One process
observation showed macOS fileproviderd using about 146% CPU. No competing local
validation was launched by this task. Before/after acceptance needs comparable
load and settings; repeat a pair if variance makes its conclusion uncertain.

The first sandboxed clean attempt failed after 300.64s when the HTTP test client
could not allocate a loopback port. The same command failed inside the sandbox
and succeeded outside it. The passing clean run started after another
`make clean` with loopback access. No product fix was needed; the failed attempt
is excluded from passing baseline comparisons.

Unrelated duplicate files already present in the workspace were preserved. They
make local whole-worktree change selection choose full validation. The doc sample
therefore directly measures `make docs` against an explicitly allowlisted file;
it does not claim that the dirty working tree routes as docs-only. This report's
committed documentation route is checked separately.

## Consolidated proposal: one bounded improvement

**Recommend shared preparation for the heavy compiler test programs.** Compile a
small number of test runners that import the common compiler modules once, then
select and execute each suite in a fresh process. Preserve suite names, every
assertion, exit status, failure output and focused selection. Keep lightweight
probes separate where sharing would make them slower. Unknown selection must
fail, and cache keys must include every suite and transitive source dependency.
Do not assume that one enormous runner is optimal: measure a small grouping and
retain it only if the targets below are met.

This changes test organisation, not production compiler modular compilation,
package policy or alpha.16 native coupling. No test-cadence change is proposed.
Keep full merge/nightly/release requirements, fresh bootstrap proofs and isolated
installation tests. Passing results must never be cached.

### Targets agreed by the owner, 27 October 2026

| Workload | Proposed acceptance |
| --- | --- |
| Local clean full, same two-worker protocol | At least 15% faster: at most 1058s against 1245.71s |
| Local compiler edit, same edit and warm starting state | At least 20% faster: at most 427s against 534.43s |
| Hosted initial compiler check, same workflow/settings | At least 10% faster on each platform: at most 877s macOS and 884s Linux |
| Unchanged full and compiler repeats | No material regression: investigate/repeat any increase over 5% |
| Package-only edit | Still zero compiler/seed rebuilds; investigate/repeat an increase over 5% or 2s, whichever is larger |
| Documentation-only edit | Zero compilation and median at most 5s on the same local host |

These are agreed criteria, not observed improvements.
Use comparable before/after samples, not rounding or a different worker count,
to claim the percentage improvements. If a hosted image changes, obtain a fresh
baseline and candidate pair on the same image. Preserve bytecode/VM checks as
no-regression observations even though they are not the improvement target.

Acceptance must demonstrate that each selected suite executes once, an injected
assertion failure still fails the command, unknown/missing suites fail, direct
and transitive edits invalidate shared artifacts, and corrupt or interrupted
artifacts rebuild safely. Run canonical `make check` after final harness edits,
then replay the five workloads and hosted profile. Add a four-worker comparison
to guard the ordinary default. Owner acceptance remains the final checkpoint.

The narrower package-tool prerequisites identified by GI#409 are not selected
for this first bounded change: the package edit already avoids compiler rebuilds
and takes 53s including its tests. That candidate remains recorded, rather than
being silently implemented or treated as essential to this proposal.

Expected remaining delivery: one or two implementation PRs depending on grouping
and regression coverage, then one evidence/acceptance PR. Stop and reassess if
safe grouping cannot meet the agreed targets; do not expand into a compiler
rewrite or weaken tests to make the figures pass.

## Delivery impact

Baseline PR#636 recorded measurements and a proposal only, with no product, test,
workflow, cache or timeout behavior changes. Public specifications, architecture and
installation instructions remain accurate. There is no website claim or release
behavior to update. GI#106 is not complete and alpha.15's earned scope does not
increase. The next action is implementation and comparable measurement against the agreed targets.
