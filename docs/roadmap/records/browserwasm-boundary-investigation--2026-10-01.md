[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Browser/WASM boundary investigation — 2026-10-01

<a id="rm-5"></a>

**RM#5: Browser ownership** · [GI#151: Browser ownership](https://github.com/sproates/panackelty/issues/151).

Issue #151 records new evidence that browser-specific CI is imposing material
feedback latency on unrelated core changes. The bounded investigation recommends
a separate browser/playground repository **after one explicit core dependency
boundary is prepared**, rather than copying the current source-tree coupling
across repositories. See [the investigation](../../internals/BROWSER_WASM_BOUNDARY.md).

This is an evidence-triggered early grooming decision. It does not itself count
as an accepted implementation deliverable or reset the three-deliverable
checkpoint. The proposed implementation sequence remains subject to review and
merge approval.

### Browser website publication connection — verified

<a id="rm-6"></a>

**RM#6: Browser release integration**.

The independent browser repository's architecture explanation from
`sproates/panackelty-browser#3` is now live at
[the public playground](https://panackelty.com/playground/#build-architecture-title).
Browser PR #4 publishes a tested, checksummed `v0.1.0` release; core PR #156
consumes it through `site/playground.json`, preserving
website/coverage ownership and assembled-site browser tests in this repository.
It removes the browser compilation step from Pages, not the remaining legacy
compatibility profile or its tests.

Post-merge acceptance passed on 2026-10-01. The published browser archive matches
the reviewed SHA-256 and the CI candidate byte-for-byte. Core commit
`64421a19afb7279a668367b62bfc3a67eda841fc` passed Check; production
[Pages run 36798813540](https://github.com/sproates/panackelty/actions/runs/36798813540)
passed website browser integration, deployment and post-deployment verification
of the entry points, coverage provenance, every playground asset and Wasm MIME
type. A separate public HTTP check confirmed the architecture section, browser
source link and exact `playground/release.json` pin. No publication acceptance
remains for this slice.

### Browser repository separation — Done

Roadmap item: [RM#5: Browser ownership](#rm-5).

The user selected completion of #151's browser ownership and CI cleanup.
Browser PR #5 moves the complete legacy suite: 15 runtime tests (145 VM corpus
cases), asset identity coverage and seven scenarios in each of three browser
engines. Its only publisher remains the tested immutable release workflow;
checksum-verified SDK archives and exact-lockfile engine downloads are cached.
The product artifact remains byte-for-byte identical to released v0.1.0.

Core removes the duplicate application, npm dependencies, SDK installer and
Playground preparation workflow. Pages downloads the reviewed release and runs
the full integration suite from an exact browser commit against the assembled
website. The common component selector omits browser provisioning on isolated
native/example PRs while preserving it for website, package, shared, unknown and
mixed changes. Production browser and deployed-asset/provenance gates remain.
Core still owns native validation and the versioned runtime-bundle contract.

Work record: [#151](https://github.com/sproates/panackelty/issues/151).
Browser PR #5 and core PR #158 merged on 2026-10-01. Post-merge acceptance passed:
browser [Check run 36813656641](https://github.com/sproates/panackelty-browser/actions/runs/36813656641)
validated and published the unchanged release; core
[Check run 36813659789](https://github.com/sproates/panackelty/actions/runs/36813659789)
passed on `c1d5339d668d6cd6f5c6bd3b844f414197328f81`.
Production [Pages run 36813831380](https://github.com/sproates/panackelty/actions/runs/36813831380)
passed the complete assembled-site browser suite, deployment and verification of
all published playground bytes, Wasm MIME type and website/coverage provenance.
A separate public HTTP check confirmed the exact release pin, architecture
section, merged website commit and current coverage run. No acceptance remains
for #151; this completion record closes it and counts the migration once.
Author-local previews were subsequently completed under #160, as recorded below.
Local acceptance: 20 browser Node tests and 17 publisher tests pass, as do site
assembly and adversarial routing checks. Final implementation `make check` passes
in 132s, still above its 120s budget; the existing native full/unit
validation-budget backlog remains open. Both cold and warm browser CI runs passed all 21 scenarios; the measured jobs
were 213s and 182s. See [phase timings and limitations](../../../tests/VALIDATION_PROFILE.md#browser-ownership-and-provisioning--2026-10-01).
Core hosted and post-merge acceptance are complete as recorded above.
