<a id="next-release--agreed-scope-2026-10-06"></a>

[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

## Release plans and decisions

**Current status (16 October 2026): alpha.12 and alpha.13 are released.**
Alpha.14 is the next planned release; its scope is recorded below under RM#152,
RM#154–RM#157. The alpha.12/13 planning text below is retained
to explain
original decisions, not to assert that their release gates are still pending.
See [alpha.13 acceptance](../../releases/ALPHA13_RELEASE_ACCEPTANCE.md).

## Agreed release direction — 20 October 2026

The owner selected the following general scope for alpha.14 through alpha.18.
These are planned, unreleased directions, with no delivery dates. This decision
supersedes the provisional alpha.15 candidate list in GI#410. Detailed designs,
acceptance criteria and implementation sequencing remain bounded by the linked
work records. Recording direction does not start implementation or claim delivery.
Optional items are not release gates. Release promotion still requires its usual
validation and explicit approval.

| Release | General scope | Boundaries |
| --- | --- | --- |
| alpha.14 — strengthen the foundations | Type-owned methods and dot notation (GI#525); routed HTTP server framework (GI#518); package management (GI#200); compiler understanding, provenance and explanations; faster coverage; selected authoring-friction and performance improvements. | Existing RM#151–RM#157 detail remains authoritative, including validated domain types (string constraints, collection sizes and cross-field record invariants), inference assessment and compiler-facing library assessment. Package management's product model remains to be settled. |
| alpha.15 — build useful programs | Typed HTTP exchanges between independent services ([GI#166](https://github.com/sproates/panackelty/issues/166)); recursive file discovery ([GI#167](https://github.com/sproates/panackelty/issues/167)); subprocess examples ([GI#168](https://github.com/sproates/panackelty/issues/168)); a tiny build tool ([GI#169](https://github.com/sproates/panackelty/issues/169)); tested guides ([GI#138](https://github.com/sproates/panackelty/issues/138)), coordinated with the examples inventory (GI#310); faster developer validation ([GI#106](https://github.com/sproates/panackelty/issues/106)), with the bounded milestone below. | Builds on alpha.14. Prove both success and failure cases using installed tools. No production build system, shell, general RPC framework or duplicated examples programme. GI#410 remains the programme discussion record. |
| alpha.16 — improve everyday development | Basic support for one editor ([GI#139](https://github.com/sproates/panackelty/issues/139)); source-aware runtime errors ([GI#136](https://github.com/sproates/panackelty/issues/136)); Homebrew installation ([GI#147](https://github.com/sproates/panackelty/issues/147)). | Choose the editor before implementation. No full language server or debugger commitment. Runtime mapping needs design and saved-bytecode acceptance; installation includes upgrades and removal. |
| alpha.17 — interactive and connected applications | Browser events invoking Panackelty handlers ([GI#165](https://github.com/sproates/panackelty/issues/165)); bounded service/library contract testing ([GI#142](https://github.com/sproates/panackelty/issues/142)). | Contract testing needs an agreed design based on concrete use cases. No general DOM/UI framework. Versioned Docker toolchain images (GI#146) and Linux ARM64 releases (GI#145) are optional; multi-architecture images depend on validated Linux targets. |
| alpha.18 — improve language ergonomics | Error-propagation shorthand ([GI#274](https://github.com/sproates/panackelty/issues/274)) and a bounded further expansion of guarded types ([GI#435](https://github.com/sproates/panackelty/issues/435)), subject to design and evidence. | Preserve explicit Result contracts. Select guard cases from real programs, beyond alpha.14's string, collection-size and cross-field record scope; candidates include formats, sorted/unique collections and cross-argument relationships, with no specific family yet selected. Targeted inference improvements (GI#170) and remaining standard-library discoverability work (GI#313) are possible supporting work, not automatic gates. Existing alpha.14 assessments remain there. |

For alpha.15, completion means runnable service and developer-tool examples with
checked guides and meaningful failure tests, plus the validation-speed acceptance
below. For alpha.16, it means a supported
installation route, basic editing support and actionable source-aware failures.
For alpha.17 and alpha.18, settle the bounded feature acceptance during design;
use earlier applications as evidence rather than expanding every linked proposal.
The release themes do not mark their broad umbrella issues complete or allocate
implementation credit. Future programme delivery registers must track their
selected scope and evidence separately.

The homepage and website roadmap will reflect this same direction. Historical
articles and released-version documentation retain their original context.

## Historical alpha.12 plan — 2026-10-06

<a id="rm-143"></a>

**RM#143: Next release — source coverage, namespaces and else if.**

The owner selected these three outcomes for the next core release after
alpha.11. The intended version is alpha.12; no date is selected. This is an alpha-era release plan,
distinct from [RM#126: First non-alpha release](first-non-alpha-release--milestone-definition.md#rm-126); it does not claim the
broader non-alpha milestone is complete.

| Included outcome | Current state | Release acceptance |
| --- | --- | --- |
| Panackelty source coverage — [RM#46](panackelty-source-coverage-programme.md#rm-46), [GI#131](https://github.com/sproates/panackelty/issues/131) | SC1–SC6 are accepted on `next`; the bounded report is verified live. Owner decision (15 October 2026): source collection and publication run weekly, with manual dispatch available. Report provenance and scope remain validated, but report freshness and coverage amounts do not gate releases. Fast policy checks and full product/package validation remain required. Coverage-speed work remains alpha.14 scope. The alpha.12 candidate archive passes local inventory, collection, session, aggregation and report smoke checks on macOS arm64. | Re-run package acceptance on the promoted Linux/macOS artifacts and verify public checksums. Preserve measured/unmeasured corpus scope and report identity; display collection age and use metrics as information, not release thresholds. |
| Namespaces — [RM#41](language-namespaces--idea.md#rm-41), [GI#198](https://github.com/sproates/panackelty/issues/198) | Executable checked-subset namespaces and coordinated migration completed on `next` in PR#358. | Verify the promoted native toolchain and package; retain the documented subset, identity-based checking/emission, import isolation, source and saved-bytecode execution, diagnostics and bootstrap limits. |
| Else-if chains — [RM#142](#rm-142), [GI#312](https://github.com/sproates/panackelty/issues/312) | Implemented in PR#316; compiler, bootstrap and examples are tested on `next`. | Verify chained syntax, evaluation order, type/effect rules, source locations, source/bytecode behavior and documentation in the release candidate. |

Release only after all three outcomes meet their acceptance gates, the
`next`-to-`main` promotion passes required checks and receives explicit owner
approval, and packaging/install/release validation passes. Release notes must
describe delivered behaviour and limitations rather than planned features.
The source coverage report remains distinct from native C coverage and does not
claim whole-suite coverage or assertion quality.

The website has its own reviewed publication. After alpha.12 artifacts pass
public availability and checksum verification, prepare a separate website PR for
release/download pins, coverage links, namespace and source-coverage claims, and
else-if examples. Retain the current published-version pins until that review and
preview pass; do not publish links to the candidate release. The later alpha.13
adoption must also document the P4 API names, `prelude.byte_buffer`, removed
legacy calls and supported examples after the native/browser artifacts are
available and verified; use a separate website PR and live preview. PR#404 is
only on `next`, so it requires no immediate change to the site pinned to alpha.12.

Packages, HTTP, the post-namespace stdlib refactor, broad example expansion and
the repository-wide readability/test programme are not additional gates for
this three-outcome release. Their existing scope and dependencies remain intact;
further scope requires an explicit owner decision.

### Alpha 13 — HTTP and expanded explanations

<a id="rm-144"></a>

**RM#144: Planned alpha.13 — HTTP and expanded explanations.**

Owner decision, 2026-10-06: plan a further alpha release after alpha.12 around
HTTP and expanded explanations, using existing backlog scope. **Planned, not
released; no date selected.** This records release intent, not feature delivery
or completion of the first non-alpha milestone.

| Planned outcome | Backlog scope and current state | Release acceptance |
| --- | --- | --- |
| Native HTTP client | P5 / RM#110 / [GI#236](https://github.com/sproates/panackelty/issues/236), implementation complete on `next` through PR #456; GI#236 closed; P8 release acceptance complete in alpha.13 | Accepted namespace/local-package prerequisites; certificate and hostname verification, limits, timeouts/cancellation, deterministic protocol/TLS tests and installed source/bytecode usage |
| Native HTTP server | P6 / RM#111 / [GI#237](https://github.com/sproates/panackelty/issues/237), implementation complete on `next` through PR #467; GI#237 closed; P8 release acceptance complete in alpha.13 | Typed handlers/responses, framing/limits, disconnect/shutdown/concurrency tests and independent interoperability; server TLS remains outside the initial contract |
| Panackelty-written local website preview server | [GI#162](https://github.com/sproates/panackelty/issues/162), generic package and tool merged into `next` in PR #498 after prerequisites #495–#497; website adoption is [website PR #42](https://github.com/sproates/panackelty-website/pull/42). All 81 assembled files match and 33 desktop/phone browser checks pass; owner inspected the local preview positively. Website PR #42 merged as `18b4cd5` after owner approval; its post-merge checks passed | Serve the built site over loopback with correct HTML/JavaScript/Wasm MIME types; support GET/HEAD, directory indexes, missing files, clear startup and port-conflict errors, clean shutdown, and reject traversal/symlink escapes. Exercise assembled-site browser tests against it and document launch/rebuild/stop |
| Expanded checker explanations | U3 / RM#48 / [GI#134](https://github.com/sproates/panackelty/issues/134): Alpha.13 outcome accepted complete by owner on 2026-10-15; implementation slices through PR #492 are on `next`; GI#134 remains open with its broader complete-decision target moved to Alpha.14 | Alpha.13's selected explanation outcome is complete. Remaining issue scope continues in Alpha.14; no further GI#134 implementation is required for Alpha.13 |
| Developer build and validation workflow — Alpha.13 gate accepted | RM#28 / [GI#106](https://github.com/sproates/panackelty/issues/106). On 2026-10-15, the user accepted the current workflow for Alpha.13 and said no further work is needed unless raised again. | **Accepted by the user.** GI#106 remains open as a broader tracker; the owner explicitly reopened validation-speed work on 2026-10-20 for the alpha.15 milestone below. The alpha.13 acceptance remains valid. Former timing values are historical, not current gates. |

Owner decision, 2026-10-15: GI#200 package management has been moved out of
Alpha.13 and into Alpha.14. It is no longer an Alpha.13 release outcome or a
P8 acceptance requirement. Its product model remains under discussion; do not
draft a design document until the owner says the discussion is ready to record.
This defers package-management work without choosing a mechanism or committing
to implementation.

Owner decision, 2026-10-15: GI#134's selected Alpha.13 outcome is accepted
complete. Its broader checker-decision scope is moved to Alpha.14; GI#134 remains
open for that remaining scope.

HTTP retains P1–P3 and transport/DNS/TLS prerequisites, the agreed P2 → P3 → P4
implementation sequence, and client-before-server ordering. Clean-installed and
relocated language/local-package checks remain final P8 acceptance evidence; no
intermediate release or promotion is required before HTTP.
Do not fold packages/HTTP into alpha.12's three gates. The planned alpha.13 HTTP
scope does not complete reproducible dependencies or the whole GI#233 programme.

<a id="alpha15-validation-speed"></a>

### Alpha.15: faster validation without weakening tests

**RM#28 / GI#106: Planned release milestone; implementation not started.** On
20 October 2026 the owner requested a concrete upcoming-release outcome because
validation takes too long, while explicitly wanting to harden and expand tests.
Alpha.15 carries this bounded milestone. This reopens the work after alpha.13's
accepted outcome; it does not reopen that release or replace alpha.14's separate
coverage-speed commitment.

The delivery must improve the time from an edit to trustworthy feedback, rather
than only producing another timing report. The implementation owner will:

- Establish a repeatable baseline for representative compiler, stdlib, runtime
  and package edits: first failing test, focused green rerun, warm full validation,
  clean full validation and hosted PR feedback. Separate compilation, fixture
  setup, assertion execution, queueing and repeated work; record cache state,
  host/resources, revisions, repetitions and variability.
- Identify and remove measured avoidable costs, evaluating dependency-correct
  build/probe reuse, fixture setup, bounded parallel execution and duplicate
  validation. Cached build artifacts must invalidate correctly; cached passing
  test outcomes are not evidence that assertions ran.
- Define and demonstrate a documented focused red/green workflow with deliberate
  representative faults. Show that dependency changes select the necessary
  tests and that restored code passes; changes spanning components and unknown
  impact must retain a full-validation fallback.
- Preserve existing assertions and required safety, failure, compiler/bootstrap,
  package and release coverage. Add meaningful regression tests for changed
  validation machinery and identified gaps. No speedup may depend on deleting
  difficult cases, weakening checks, reducing test scope or increasing timeouts.
  Any change to validation cadence requires an explicit coverage/risk map and
  owner agreement; it cannot silently move necessary checks out of PR acceptance.
- Demonstrate materially faster representative local and hosted feedback with
  comparable before/after measurements. Report all measured paths, including
  unchanged or slower ones, and explain how adding tests affects the critical
  path. Use the baseline to propose concrete latency targets for owner agreement;
  neither historical timings nor an arbitrary new target establish success.

Acceptance requires implemented improvements, executed regression evidence,
replayable before/after results and the owner's explicit acceptance of the
resulting workflow. A report alone does not complete the milestone. Keep GI#106
open until its owner acceptance is recorded; distinguish this release outcome
from other work still tracked by the broad issue. The delivery maintainer owns
measurement and implementation, with unresolved costs and next actions recorded
under [RM#123](standing-performance-engineering--ongoing.md#rm-123).

Scope is the developer build/test and CI feedback cycle. Application throughput,
HTTP load testing (GI#553), a testing-framework rewrite and numerical release
performance claims are separate. Effort is provisionally medium to large;
profile first and select bounded changes rather than promising a scheduler or
build architecture before evidence. No implementation begins through this
planning update.

For GI#106, inspect which builds and tests run during local red/green work,
focused and full checks, pull requests, post-merge pushes and releases; what each
protects; how changes cross compiler, standard-library, bytecode, VM/runtime,
package and CI boundaries; and how much elapsed time is execution, queueing or
other waiting. Use the evidence to offer replacement NFRs, test selection and
cadence, and possible build improvements for the user's decision. Do not set
numeric targets or a test-frequency policy in advance. Preserve regression-risk
coverage and execute selected tests; never reuse test outcomes as cache entries.
GI#106 is complete only when the user says they are happy with the result.

When alpha.13 was planned, GI#180's owner-requested pause remained in effect;
that decision did not restart explanation implementation. On 2026-10-14 the
owner included GI#180 in Alpha.14, with its release-specific scope recorded in
[RM#151](#rm-151). On 2026-10-15 the owner accepted GI#134's selected Alpha.13
outcome as complete and moved the remaining complete-decision target to
Alpha.14. Alpha.13's explanation outcome is therefore not waiting for further
implementation.

Release only after agreed outcomes and supporting prerequisites are accepted,
required promotion/package/install checks pass and the owner approves publication.
Update version-specific documentation and website claims through a separate
reviewed publication with an iPhone-accessible preview. Do not claim experimental
work is shipped or describe this release as production-ready.

### Alpha.13 release completion — 2026-10-15

Alpha.13 is published as `v0.1.0-alpha.13` at
`a2ac16dfb7f7667b8531ddb9e821a35b13644fd5`. Promotion PR #504 and
Release run 38806215200 passed; public checksums/provenance and byte equality
with both native CI-smoked archives are verified. The downloaded macOS archive
also passed installed/relocated acceptance locally. The
[acceptance record](../../releases/ALPHA13_RELEASE_ACCEPTANCE.md) completes P8 / GI#238.
Source coverage is weekly/manual reporting, not a release gate. The original
SP1 baseline remains 90/100 earned, with P7's 10% deferred to alpha.14.
Installer/default and website promotion remain separate; their public-artifact
prerequisite is satisfied. No alpha.14 implementation is included.

### Alpha 14 — compiler understanding and correctness

<a id="rm-151"></a>

**RM#151: Alpha.14 — compiler understanding and correctness focus.**

Owner direction, 2026-10-13: Alpha.14 should include some work on compiler
understanding and correctness, Panackelty's strongest novel area. On 2026-10-14
the owner explicitly included [RM#9: Compiler understanding programme](current-status-and-grooming-2026-10-01.md#rm-9) /
[GI#180](https://github.com/sproates/panackelty/issues/180) in Alpha.14. The
owner has now moved two bounded outcomes from Alpha.13 into this release:
[RM#91: Compilation provenance](current-status-and-grooming-2026-10-01.md#rm-91) / [GI#173](https://github.com/sproates/panackelty/issues/173),
and [RM#153: Expand guarded types beyond numeric bounds](language-direction-and-differentiation--exploration.md#rm-153) /
[GI#435](https://github.com/sproates/panackelty/issues/435), initially scoped
to a non-empty string guard and broadened on 2026-10-20 to string constraints,
non-empty/bounded collections and cross-field record invariants, with static
proof and explicit runtime validation. The updated [RM#153 scope](language-direction-and-differentiation--exploration.md#rm-153)
is authoritative; broader families remain for later selection. GI#173 is one selected workstream within GI#180;
on 2026-10-15, the owner also moved the remaining broad completion scope of
[RM#48: Compiler explanations](groomed-development-and-engineering-work.md#rm-48) / [GI#134](https://github.com/sproates/panackelty/issues/134)
to Alpha.14; its selected Alpha.13 outcome is complete, while GI#134 remains
open. The selected GI#180 workstreams are GI#173 and the remaining GI#134 scope;
no other GI#180 workstream is selected for Alpha.14. On 2026-10-15, the owner
also selected [RM#64: Inference and diagnostics](language-direction-and-differentiation--exploration.md#rm-64) /
[GI#170](https://github.com/sproates/panackelty/issues/170) as a separate
Alpha.14 investigation of current inference limits and diagnostic experience.
Its existing assessment scope remains authoritative; this does not select an
inference architecture or implementation. Detailed guard semantics and
acceptance remain for slice planning. These selections record release scope,
not implementation progress or authorization to begin. The final release scope,
amount of work and date remain undecided. [RM#152](#rm-152) separately selects
a bounded standard-library dependency assessment; it does not commit the release
to a redesign or refactor. On 2026-10-15, the owner also moved package
management ([RM#44](dependency-management--idea.md#rm-44) / [GI#200](https://github.com/sproates/panackelty/issues/200))
from Alpha.13 into Alpha.14. Its product model remains under discussion; this
selection does not choose a distribution mechanism or automatically commit to
implementation. Alpha.13's plan and gates remain separate. On 2026-10-15, the
owner also required faster code coverage in Alpha.14;
[RM#154: Incremental coverage collection](#rm-154) records that outcome. The
owner also selected [RM#155: Authoring-friction improvements](#rm-155) for
Alpha.14: address prioritised items from recorded development experience. On
2026-10-16, the owner also selected [RM#156: Routed HTTP server API](#rm-156) /
[GI#518](https://github.com/sproates/panackelty/issues/518) for Alpha.14.
The owner also selected [RM#157: Performance](#rm-157), with particular
attention to HTTP server performance and source coverage instrumentation time.

<a id="rm-152"></a>

**RM#152: Assess the compiler-facing standard-library boundary** ·
[GI#409: Assess the compiler-facing standard-library boundary](https://github.com/sproates/panackelty/issues/409).

For Alpha.14, document the compiler's direct and transitive standard-library
dependencies and distinguish source dependencies, compiler-seed/bootstrap
rebuilds, VM/runtime ABI coupling, and package versioning. Recommend whether an
explicit compiler-facing library boundary would help app-facing standard-library
work proceed independently. This is an assessment: any implementation or
reorganization requires a later scope decision based on the findings.


<a id="rm-154"></a>

**RM#154: Incremental coverage collection — Alpha.14 requirement.**

**Planned for Alpha.14 by owner direction, 2026-10-15.** Improve code-coverage
collection speed, particularly repeated release checks after changes that do not
alter the measured program. During Alpha.13 preparation, reporting-only repairs
repeated expensive plan generation. Successful source-coverage
[Check 38775373415](https://github.com/sproates/panackelty/actions/runs/38775373415)
took 29m48s for 31 fresh sessions and 45 measured files. This is an observed hosted
baseline, not a fixed performance target. Profile generation, execution,
aggregation and publication separately before selecting further optimisations.

The first implementation direction is incremental reuse of generated coverage
plans, keyed by exact compiler/exporter and VM identities, transitive source
inputs, format/schema and relevant settings. Validate cached plans and provenance
before use; missing, corrupt, partial, untrusted or invalidated entries must
rebuild safely. Keep every required test execution fresh. Reusing execution
results is a separate design decision and is not selected by this requirement.

Acceptance must demonstrate materially faster repeated collection with comparable
cold/warm measurements, explain cache hits and invalidations, and cover changes
to compiler, VM, exporter, dependencies and settings. Cached and freshly generated
plans must produce equivalent inventories and coverage semantics. Preserve the
full measured corpus, assertions, fresh session identities, explicit unmeasured
and unavailable results, source provenance and release freshness checks. Raising
timeouts alone does not satisfy the speed outcome; do not reduce test scope or
weaken validation to meet it. Set a concrete improvement target from the profiling
evidence during implementation planning rather than inventing a number now.

This is a bounded follow-up to [RM#28: Incremental and modular builds](independent-website-publishing.md#rm-28) /
[GI#106](https://github.com/sproates/panackelty/issues/106); the owner's accepted
Alpha.13 workflow remains accepted. Initial effort is medium, provisional on cache
trust and invalidation requirements. Implementation is not started by this roadmap
addition, and it does not add a new gate to the ongoing Alpha.13 release.


<a id="rm-155"></a>

**RM#155: Review the friction log and implement selected improvements — Alpha.14.**

**Planned by owner direction, 2026-10-15; clarified 2026-10-16.** Review the
recorded authoring-friction log, identify concrete improvements, and select a
bounded, prioritised set to implement during Alpha.14. Candidate improvements may
address language/compiler behaviour, diagnostics, API discoverability,
documentation, examples or development tooling. Distinguish confirmed defects
from confusing intended behaviour and unverified suggestions; reproduce each
selected case before choosing its remedy.

Acceptance requires an explicit list of selected items and their intended
outcomes, a demonstrated before/after improvement for each, appropriate regression
tests for behaviour changes, and updated guidance where discovery or explanation
is the problem. Record resolved, deferred and still-unverified cases with evidence.
Coordinate overlapping work with [RM#64: Inference and diagnostics](language-direction-and-differentiation--exploration.md#rm-64),
[RM#48: Compiler explanations](groomed-development-and-engineering-work.md#rm-48) and [RM#154: Incremental coverage
collection](#rm-154) rather than counting the same delivery twice.

The release commitment is to review the log, identify concrete improvements,
and implement the selected prioritised items; it is not to implement every logged
suggestion. Confirm the selected items and estimate their combined effort during
Alpha.14 planning; scope and effort remain provisional until that review. Preserve
existing correctness and compatibility contracts,
and document any deliberately selected changes. This roadmap addition starts no
implementation and adds no gate to the ongoing Alpha.13 release.

<a id="rm-156"></a>

**RM#156: HTTP server framework and type-owned dispatch — Alpha.14.**

**Selected by owner direction, 2026-10-16.** Implement the agreed routed HTTP
server API in `packages/http`, as specified in [GI#518](https://github.com/sproates/panackelty/issues/518).
Preserve the HTTP client while making the low-level server/service machinery an
internal implementation detail. The public server API includes the simple
`create_server(port)` form and the typed `create_contextual_server(port, context)`
form, with named handlers, route registration, request/response handling, fixed
first-version defaults, error isolation and lifecycle behavior as specified in
GI#518.

Owner clarification, 2026-10-19: Alpha.14 includes the type-owned method dispatch
migration in [GI#525](https://github.com/sproates/panackelty/issues/525), which
supports this HTTP server framework. Server methods such as `get` and `put` must
coexist with Map operations through checked owner identity. The migration includes
built-in and nominal methods, generic and effect checking, maintained API/caller
migration, removal of receiver-to-global fallback and collection-name exceptions,
and final dot-call syntax. Arrow calls are temporary migration syntax. Follow the
[implementation handoff on next](https://github.com/sproates/panackelty/blob/next/docs/design/type-owned-dispatch/dispatch-implementation-handoff.md)
for packet acceptance and bootstrap checkpoints. This records release scope;
GI#525 remains open until the full migration and release/consumer acceptance pass.

Migrate the static preview tool to compose the generic server with its own typed
static-serving context and fallback handler. Keep all static-file knowledge and
behavior in the static package/tool; `packages/http` must remain generic. Deliver a
complete user-facing `packages/http/README.md` with practical examples. Update
affected API/reference documentation and examples, and test the route API, context
dispatch, HTTP behavior, lifecycle, preview integration and existing client
regressions. GI#518 contains the detailed contract and acceptance cases.

This records Alpha.14 release scope and does not itself authorize implementation
or settle work sequencing, effort, or release timing. If implementation exposes
an unanticipated gap or inconsistency, stop and ask the issue owner rather than
guessing; update GI#518 before proceeding.


<a id="rm-157"></a>

**RM#157: Performance — Alpha.14.**

**Selected by owner direction, 2026-10-16.** Review Panackelty holistically to
identify material performance problems, areas that need closer investigation, and
well-supported quick improvements. The review should look across compilation,
runtime execution, libraries and packages, and development and test workflows,
then distinguish measured bottlenecks from hypotheses and areas where evidence is
still missing.

Give particular attention to the reported HTTP server performance relative to
Node.js and the time required to instrument Panackelty source for code coverage.
Use comparable, correctness-checked workloads and record the environment and
limitations; establish measurements before setting improvement targets. For
coverage, separate instrumentation and plan-generation time from test execution,
aggregation and publication, and coordinate with [RM#154: Incremental coverage
collection](#rm-154) so its validated plan-caching work is not duplicated or
counted twice.

Prioritise concrete findings. Implement a bounded set of safe, evidence-backed
quick wins during Alpha.14 where they fit the release; record larger or
uncertain opportunities with evidence and a recommendation for later selection.
Preserve correctness, fresh required test execution and existing validation
coverage. This review does not set a universal performance target or commit to
implementing every identified optimisation. Agree specific workloads, targets
and any larger implementation scope during planning, after the initial evidence
is available. Coordinate baseline work with [RM#56: Performance baselines](current-status-and-grooming-2026-10-01.md#rm-56)
and ongoing policy under [RM#123: Standing performance engineering](standing-performance-engineering--ongoing.md#rm-123).


### Else-if chains

<a id="rm-142"></a>

**RM#142: Else-if chains** ·
[GI#312: Else-if chains](https://github.com/sproates/panackelty/issues/312).

**Implemented in next through PR#316; planned for alpha.12, not released.** Accept
`if ... { ... } else if ... { ... }` chains with an optional final else block.
Preserve existing nested syntax and conditional result/Void rules, type/effect
checking, ordered short-circuit evaluation and useful malformed-input diagnostics.
Tests cover selected and skipped arms, omitted else where legal, rejected
conditions/results/effects, source spans and source/saved-bytecode parity.
Preserve source inventory/coverage meaning and compiler/stdlib fixed points.
Update SPEC.md and coding guidance when the syntax is implemented.
