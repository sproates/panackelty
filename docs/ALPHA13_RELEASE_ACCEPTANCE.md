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

1. **Candidate preparation — in progress.** Reconcile main's accepted scope with
   next's implementation and validation workflow. Set VERSION and all shipped
   package manifests to alpha.13; prepare notes and compatibility/prerequisites.
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
   post-merge website Check run 38760731372 passed.
3. **P8 candidate acceptance — local checks passed; hosted/artifact acceptance pending.**
   The libcurl candidate passed `make check` in 555s; the coverage-capacity
   correction and refreshed seed passed the final full check in 576s on macOS arm64. This
   includes unit/functional suites, the bootstrap fixed point, HTTP/TLS failure
   and cleanup cases, installed/relocated static serving, archive smoke and the
   two independent applications sharing a relocated library in source and bytecode.
   The focused HTTP client suite and YAML/layout/routing contracts also passed.
   Hosted Linux/macOS and public-artifact evidence remain required; retain exact
   commit and archive identities with those results.
4. **Source coverage refresh — capacity correction in progress.** The requested
   refresh of candidate `2ca92cb` failed in run 38760436718: the module-bindings
   unit plan was 19,201,113 bytes, its registry 17,569,591 and wrapped raw record
   20,810,250, exceeding the former 16 MiB container bound. Align container and
   synchronous file-I/O limits at 32 MiB while preserving 16 MiB identity blobs
   and all instruction/session budgets; rerun exact-candidate collection after
   validation. Final full validation passed in 576s; 37 coverage policy/reader tests
   and renderer parity (257 items for each branch outcome) passed. No source roots
   or assertions are removed. The live report still
   measures `e5e353e261761e3eab14b7f040d9ce1fd2d0a8fa` from 11 October and predates
   the new HTTP/service code. Request a next push with `[source-coverage]`, wait
   for a successful exact-commit source artifact, dispatch Coverage Pages on its
   main branch, and verify live source provenance and summary. Do not add percentage
   thresholds. Retain measured/unmeasured scope and input freshness. Native coverage
   is a separate main-channel report and refreshes with full main validation.
5. **Promotion — pending.** Prepare next-to-main PR with reconciled scope and P8
   evidence, pass all required exact-head checks, obtain explicit owner approval.
6. **Release — pending.** Obtain owner publication approval for the exact main SHA
   and version; the Release workflow checks coverage freshness, full validation,
   native archives/checksums and source provenance before publication. No tag or
   release dispatch is authorised merely by completing preparation.
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
