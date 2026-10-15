# Alpha.13 release acceptance

Status: preparation started on 15 October 2026 at the owner's request. No release
has been published. Current published core release: alpha.12.

## Accepted scope

| Outcome | Evidence / remaining delivery |
| --- | --- |
| Namespaces, local packages and stdlib API migration | P1–P4 implementation accepted; exact alpha.13 installed/relocated evidence remains in P8 |
| HTTP client | P5 complete in PR #456; typed HTTP/HTTPS, TLS, deadlines, cancellation and failure tests |
| HTTP server | P6 complete in PR #467; framing, lifecycle and independent interoperability |
| Checker explanations | Owner accepted the alpha.13 outcome through PR #492; broader GI#134 work moves to alpha.14 |
| Developer workflow | Owner accepted GI#106 for alpha.13; no additional optimisation selected |
| Website preview server | Core PRs #495–#498 merged; website PR #42 merged as `18b4cd5` with owner approval and passing post-merge checks |

GI#200 package management and GI#173 compilation provenance are alpha.14 work.
They are not alpha.13 gates. The broader SP1 original weight baseline is retained
in the roadmap; it is not an alpha.13 completion percentage.

## Release sequence and evidence

1. **Candidate preparation — complete on next.** PR #499 reconciled main's accepted scope with
   next's implementation and validation workflow. VERSION and all shipped
   package manifests identify alpha.13; notes and compatibility/prerequisites are prepared.
   Explicitly install libcurl development headers in Linux collection, full-check,
   nightly and release build jobs, matching the existing next smoke setup. The
   website CI exposed that the runner does not supply these headers by default.
   Use the string protocol restriction on libcurl 7.85+ and retain the older API
   on older supported systems; Ubuntu 24 CI rejected the deprecated option.
2. **Website adoption — complete.** Website PR #42 passed 30 local automation
   tests plus assembly checks, all 81 fresh preview files matched their served
   bytes (largest 706,607), and 24 execution/resource plus nine editorial/navigation
   tests passed across three engines including phone widths. The owner inspected
   the running preview positively and approved PR #42. It merged as `18b4cd5`;
   post-merge website Check run 38760731372 passed. The owner also approved the
   documentation completion correction in PR #43, merged as `b950a46`; its
   post-merge Check and publication run 38772659325 passed.
3. **P8 candidate acceptance — local checks passed; hosted/artifact acceptance pending.**
   The libcurl candidate passed `make check` in 555s; the coverage-capacity
   correction and refreshed seed passed the final full check in 576s on macOS arm64. This
   includes unit/functional suites, the bootstrap fixed point, HTTP/TLS failure
   and cleanup cases, installed/relocated static serving, archive smoke and the
   two independent applications sharing a relocated library in source and bytecode.
   The focused HTTP client suite and YAML/layout/routing contracts also passed.
   Hosted Linux/macOS and public-artifact evidence remain required; retain exact
   commit and archive identities with those results.
   Promotion Check 38780595525 exposed a Linux-only missing `sys/time.h`
   include in the independent HTTP test client and test-harness deadlines that
   were too short for hosted workloads. The complete 370-assertion fixture
   runner passed locally in 119.60s, while hosted executions reached the former
   180s aggregate bound. The CLI fixture passed locally in 17.10s against a
   former 20s per-execution bound; a cold driver probe took 34.16s locally and
   reached 90s on a hosted runner. The repair retains the corpus and assertions,
   gives these aggregate/compiler workloads separate bounded allowances, and
   passed final local `make check` in 402s, including archive and relocation
   smoke. All 41 focused coverage policy/reader tests passed. Fresh hosted
   validation and coverage evidence remain required before promotion.
4. **Source coverage refresh — prior report verified; repaired candidate refresh pending.**
   PR #506 changed test inputs, so its candidate requires a fresh trusted
   next-push report before publication. Request collection with the existing
   `[source-coverage]` commit marker; promotion-run artifacts do not substitute
   for the publisher’s trusted next-push provenance. The evidence below records
   the previous successful report, not freshness for the repaired candidate.
   Candidate `061212655bf90cc9aca56003b46aac68255a288c` passed
   [Check 38775373415](https://github.com/sproates/panackelty/actions/runs/38775373415).
   Collection took 29m48s and produced artifact `11995430565`: 31 fresh sessions
   and executions, all 45 measured files, and six explicitly unmeasured files.
   [Coverage Pages 38779817246](https://github.com/sproates/panackelty-coverage/actions/runs/38779817246)
   passed preparation, deployment and byte verification of every live report file.
   [Live source provenance](https://coverage.panackelty.com/source/provenance.json)
   and [summary](https://coverage.panackelty.com/source/summary.json) identify the
   exact candidate, successful Check and artifact. The release source-coverage
   gate passed against those downloaded live files on 15 October 2026.

   Repairs #500–#502 aligned containers at 32 MiB while retaining 16 MiB
   individual identities, corrected provenance hashing, reviewed stale policy
   hashes, moved manifest-policy validation before collection and allowed five
   minutes per plan export for observed runner variation. Execution sessions
   remain bounded at three minutes and the overall job at 35 minutes. Final
   product validation passed in 576s; 41 focused reporting/policy tests and
   renderer parity passed. All 410 compile-time exclusions were reviewed against
   source declarations; no measured files or execution roots were removed.

   Coverage amounts remain informational. The report explicitly retains 269
   unavailable line measurements, 70 function measurements and 84 branch
   measurements; it does not claim complete percentages or whole-language
   coverage. Native coverage remains a separate main-channel report. Promotion
   checks and release input-freshness checks remain required.
5. **Promotion — authorised, checks pending.** The owner instructed “Finish the
   release.” PR #504 promotes next to main after all required checks pass for
   its exact current head. Repair the observed promotion failures first.
6. **Release — authorised, publication pending.** After successful promotion,
   dispatch the Release workflow for version 0.1.0-alpha.13 and the exact merged
   main SHA. It checks coverage freshness, full validation, native archives,
   checksums and source provenance before publication.
7. **Public artifact acceptance — pending.** Verify published supported archives,
   checksums and clean installation before promoting installer defaults or website
   release/download claims. Website adoption is a separate reviewed publication.

## Performance and limitations

The owner accepted the current build/validation workflow for this release; old
printed budgets are historical comparisons. Retain all required checks. The preview
server's measured request latency is higher than Node (roughly 7–13 ms versus
sub-millisecond), accepted for development previews with successful responsive-page
checks. Compiler seed, VM and library must remain matched; v9 does not promise
cross-release bytecode compatibility. HTTP client uses replaceable platform libcurl;
server TLS, streaming large static responses and remote package management are absent.
