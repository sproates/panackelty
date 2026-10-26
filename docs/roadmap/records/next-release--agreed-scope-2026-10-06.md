<a id="next-release--agreed-scope-2026-10-06"></a>

[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

## Release plans and decisions

**Current status (21 October 2026): alpha.14 is released.**
Alpha.15 is the next planned release. The allocation table below supersedes
older release assignments in the retained decision history. Earlier planning
text explains the original decisions, rather than reopening completed gates.
See [alpha.14 acceptance](../../releases/ALPHA14_RELEASE_ACCEPTANCE.md).

## Agreed release direction — 20 October 2026

The owner rebalanced the releases on 20 October 2026: alpha.14 delivers completed
methods/HTTP work with a bounded documentation finish; incomplete alpha.14 work
moves to alpha.15, and the previous alpha.15–alpha.18 themes move to alpha.16–alpha.19.
The GI#106 validation-speed milestone stays in alpha.15 as an explicit exception.
No dates are set. This allocation supersedes earlier version assignments below
and in historical issue discussions; existing scope boundaries still apply.
Implementation and release publication remain separate decisions.

| Release | General scope | Boundaries |
| --- | --- | --- |
| alpha.14: methods and HTTP | Released: GI#525 type-owned methods/dot notation, GI#518 routed HTTP framework, and the HTTP reference and runnable examples slice of [GI#552](https://github.com/sproates/panackelty/issues/552). | Release validation passed and native artifacts are published. [GI#555](https://github.com/sproates/panackelty/issues/555) coloured diagnostics was not included. The wider GI#552 standard-library reference remains separate. |
| alpha.15: strengthen the foundations | Remaining compiler understanding GI#180 (GI#173 provenance and GI#134 explanations); GI#435 validated domain types; GI#170 inference assessment; GI#409 compiler-facing stdlib assessment; full package-management implementation GI#200, arranged as P01–P04 under RM#44 and tracked as RM#158–RM#161; RM#154 faster coverage, RM#155 selected friction improvements, RM#157 performance review/quick wins; GI#106 validation-speed milestone. | All previously agreed bounded outcomes remain. Package-management implementation follows the [P01–P04 plan](dependency-management--idea.md#alpha15-implementation-plan); GI#200 stays open through final acceptance. Test-speed acceptance stays here rather than moving with the old alpha.15 theme. |
| alpha.16: build useful programs | GI#166 typed HTTP exchanges, GI#167 recursive file discovery, GI#168 subprocess examples, GI#169 tiny build tool, GI#138 tested guides coordinated with GI#310. | Prove installed success/failure workflows; no general RPC framework or production build system. Full GI#552 API-reference completion and GI#553 load profiling were only proposed for old alpha.15; their corresponding candidate slot is now alpha.16, not an automatic commitment. |
| alpha.17: improve everyday development | GI#139 one-editor support, GI#136 source-aware runtime errors, GI#147 Homebrew installation. | No full language server/debugger commitment. GI#555's proposed fallback slot moves here if not selected for alpha.14. |
| alpha.18: interactive and connected applications | GI#165 browser event handlers; GI#142 bounded service/library contract testing. | Contract design follows concrete use cases; no general DOM/UI framework. Docker toolchain images (GI#146) and Linux ARM64 releases (GI#145) remain optional. |
| alpha.19: improve language ergonomics | GI#274 error propagation and further GI#435 guard families, subject to design/evidence. | Beyond alpha.15's strings, collection sizes and cross-field records. Targeted inference and remaining stdlib discoverability may support the release; they are not automatic gates. |

### Alpha.14 finishing scope

The HTTP slice of GI#552 must provide browsable reference pages for server types,
constructors and every public method, request/response types and callback/lifecycle
contracts, with signatures, errors, limits and version applicability. Include the
HTTP client/types reference and complete runnable examples for ordinary/contextual
servers, routing, request access, responses, errors, shutdown and static integration.
Cross-link reference entries and repository examples; execute examples against the
matching toolchain and verify site links and presentation. A reader must be able
to find and use the API without opening implementation source.

This finishes the HTTP slice, not the entire stdlib reference initiative.
Alpha.14 API documentation now applies to the published release. Methods and HTTP delivery evidence
remain in their existing acceptance records; no completed work is reopened.

For alpha.15, retain each selected workstream's acceptance, including implemented
validation-speed improvements and owner acceptance. For alpha.16, require runnable
service/developer-tool examples and checked guides. For alpha.17, require usable
editing, installation and source-aware failures. Alpha.18/19 designs remain bounded
by their existing records. Do not silently shift an optional candidate into a gate.

The homepage, roadmap and maintenance guidance must use the same release mapping.
The methods article still promises exactly methods and HTTP in alpha.14; add a dated
completion note while retaining its historical migration explanation. The owner subsequently authorised alpha.14 publication; the acceptance record
tracks the native, browser and website promotion evidence.

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
P8 acceptance requirement. At that point its product model remained under
discussion; do not draft a design document until the owner says the discussion
is ready to record. The 22 October 2026 owner decision supersedes this design-
only allocation and selects full Alpha.15 implementation, split into the
packets P01–P04 recorded under [RM#44](dependency-management--idea.md#rm-44)
and tracked as [RM#158–RM#161](../../../ROADMAP.md#rm-158).

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

**RM#28 / GI#106: Scope agreed, 26 October 2026. Baseline and selected
implementation remain to be started.** The alpha.15 commitment is a bounded
improvement to the validation workflow: make a normal change, get useful feedback
quickly and establish confidence without repeatedly doing unnecessary work.
This replaces the broader milestone description agreed on 20 October. Alpha.13's
accepted outcome remains accepted; the wider GI#106 issue remains an umbrella.

#### V1: Establish the baseline and agree the improvements

Measure these five workloads at the same revision on the same named machine:

| Workload | Question |
| --- | --- |
| Clean full validation | What does validation cost with no prepared build artifacts? |
| Unchanged repeat of full validation | What work repeats when the inputs have not changed? |
| Small compiler change with focused checks | How quickly does compiler development get useful feedback? |
| Small package-tool change with focused checks | Does package development pay unrelated compiler costs? |
| Documentation-only change | Does the lightweight route remain lightweight? |

Record elapsed time, actual compilation counts, fixture preparation and assertion
execution. Record the exact revision, commands, representative edits, host and
cache conditions. Use a few repeated samples where variation matters. For
representative hosted runs, separate runner queueing from execution and compare
before/after on comparable runner configurations; do not compare local and hosted
numbers as though they were the same environment.

The output is a short table explaining the dominant avoidable costs. Bring the
owner one consolidated proposal selecting **at most three improvements**, their
numeric latency targets and acceptance cases. Targets follow the evidence and
owner agreement, not historical timing thresholds. No telemetry platform or
exhaustive benchmark programme is selected.

#### V2: Implement the fixed selection

Evaluate these remedies in order of preference, without assuming the baseline
will establish any particular cause:

1. Remove duplicated preparation and compilation within a validation run.
2. Reuse valid build artifacts across unchanged repeat runs, with correct input
   invalidation and fresh assertion execution on every run.
3. Provide a dependable, documented focused development route for compiler and
   package-tool changes.

The agreed implementation list may substitute a better evidenced remedy; do not
silently add a fourth improvement. Further opportunities become follow-ups.
Dependency-aware caching across arbitrary source graphs is not automatically
included: if it requires substantial compiler work, bring a separate scope
proposal rather than expanding this milestone.

For each supported change category, document the development command, what it
checks and does not establish, the pre-merge requirements, and the nightly and
release requirements. Retain existing merge, nightly and release requirements.
Any cadence or selection change that moves required coverage needs a separate,
explicit owner decision supported by a coverage/risk map. Shared or unknown
impact must retain a conservative full-validation fallback.

#### V3: Demonstrate the result and accept the milestone

Repeat the five workloads under comparable conditions. Report what became faster,
by how much and why; which compilations or preparation steps disappeared; and
any unchanged or slower paths. Demonstrate that relevant changes invalidate the
right artifacts, assertions still execute, deliberate failures still surface and
restored code passes. Add meaningful regressions for changed validation machinery.
Preserve existing assertions, safety and failure coverage, bootstrap, platform,
package and release checks. Never cache passing test outcomes, weaken checks or
increase timeouts as a speed solution.

Completion requires the selected improvements, replayable before/after evidence,
passed applicable validation and explicit owner acceptance of the resulting
workflow. A report or timeout repair alone does not complete the milestone.
Record milestone acceptance separately from closure of the wider GI#106 issue;
other open concerns do not keep this accepted release outcome unfinished.
The delivery maintainer owns implementation, measurements and recording unresolved
costs under [RM#123](standing-performance-engineering--ongoing.md#rm-123).

#### Boundaries and delivery estimate

Outside this milestone: alpha.16 native-adapter/compiler coupling work; separate
compilation, a linker or major compiler redesign; RM#154 coverage-plan caching;
RM#157 general runtime/HTTP performance; and a broad overhaul of agent context
usage or reporting. Coordinate overlapping evidence without counting delivery
twice. Existing tool-preparation/reuse and watchdog repairs remain delivered but
do not substitute for the agreed measurements and acceptance.

Provisionally three to five PRs: baseline and agreed plan, one to three selected
improvements, and final acceptance documentation folded into the last delivery
where practical. Re-estimate after the baseline if necessary, without silently
expanding the fixed selection. This records scope only; measurements and
implementation have not been started by this update. Website impact: none;
release claims and published installation guidance are unchanged.

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
SP1 baseline remains 90/100 earned. P7's 10% remains unearned on that original
baseline; its Alpha.15 packet weights are a separate GI#200 planning baseline.
Installer/default and website promotion remain separate; their public-artifact
prerequisite is satisfied. No alpha.14 implementation is included.

### Alpha.15 explanation, provenance and domain-type clarification

Owner agreement, 26 October 2026. This defines acceptance requirements, not a
claim that current implementations already meet them, and starts no implementation.

- **GI#134:** complete coverage of every relevant type, effect and proof decision
  made while checking a function remains the target, with delivery prioritising
  completeness as soon as practicable. The inventory must make remaining work
  finite and visible, not select attractive examples and defer the rest. Each
  decision exposes what was checked, the requirement, actual outcome, supporting
  checker facts and relevant source locations. Unavailable means checking could
  not reach the decision, with a reason; missing implementation is a coverage gap.
- **GI#173:** every emitted instruction must have accurate attribution to its
  responsible source construct, including imported code and generated instructions
  implementing a source construct. Where no corresponding source location genuinely
  exists, identify the compiler-generated origin explicitly. No unexplained holes
  and no fabricated source locations. In the reverse direction, explain constructs
  producing several instructions, shared instruction mappings or no executable
  instructions. Connect checking and transformations to actual emitted bytecode;
  source mapping alone is not complete provenance. Inventory decisions and
  transformations, reconcile existing coverage and derive finite delivery slices.
- **GI#435:** the owner subsequently agreed the exact six-constraint semantic
  scope on 26 October, recorded in [RM#153](language-direction-and-differentiation--exploration.md#rm-153):
  non-empty/non-blank/bounded-length/control-free strings, bounded arrays/maps/sets,
  and ordered same-record numeric field relationships. Static proof, explicit
  runtime validation, invariant preservation and deterministic failure reporting
  are included. Explicit guard bindings, the `validate[Type](input)` result contract and failure
  ordering are now also agreed in RM#153. Remaining constraint syntax, error schema,
  Unicode version and examples need design before implementation.
- New GI#435 proof decisions shipped in alpha.15 must also have GI#134 explanations.
  Define that integration once, without counting its delivery twice. Agree example
  transcripts before implementing output, including failure and unavailable cases.
  Use realistic multi-module, generic, guard and effect examples plus maintained
  code for acceptance; preserve ordinary compilation and bootstrap correctness,
  and measure query latency and ordinary compilation overhead.

Runtime value tracing, change-impact prediction and automatically deriving fixes
remain outside the selected GI#180 alpha.15 workstreams. A debugger, interactive
compiler visualiser and stable public compiler-IR API are not selected. Detailed
inventories, acceptance examples and effort estimates remain planning work; if
scope is too large, seek an explicit release decision rather than redefine complete.

### Compiler understanding and correctness: now alpha.15

<a id="rm-151"></a>

**RM#151: Compiler understanding and correctness focus (alpha.15).**

The 20 October release rebalance moves the incomplete outcomes below to alpha.15.
The dated alpha.14 selections are retained as history, not current allocation.

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

For Alpha.15, document the compiler's direct and transitive standard-library
dependencies and distinguish source dependencies, compiler-seed/bootstrap
rebuilds, VM/runtime ABI coupling, and package versioning. Recommend whether an
explicit compiler-facing library boundary would help app-facing standard-library
work proceed independently. This is an assessment: any implementation or
reorganization requires a later scope decision based on the findings.


<a id="rm-154"></a>

**RM#154: Incremental coverage collection: alpha.15 requirement.**

Current allocation: alpha.15 after the 20 October rebalance.

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

**RM#155: Review the friction log and implement selected improvements: alpha.15.**

Current allocation: alpha.15 after the 20 October rebalance.

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

**RM#157: Performance: alpha.15.**

Current allocation: alpha.15 after the 20 October rebalance.

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

Planning update, 20 October 2026: GI#525 is complete on `next`; the sentence
above about it remaining open is historical release-scope context. The
[GI#518 staged plan](../../design/routed-http-server/implementation-plan.md)
records the separate HTTP implementation baseline, packet dependencies and
acceptance gates. The planning-only status is superseded by the implementation
and final acceptance delivery: P01–P15 are complete on its merge, with evidence
in the [acceptance register](../../design/routed-http-server/acceptance-register.md).
Both GI#518 and GI#525 are implementation-complete for alpha.14; native release
publication and website promotion remain separate.

Owner clarification, 20 October 2026: the simple constructors retain `0.0.0.0`;
explicit-address ordinary/contextual constructors allow IPv4 binding without
resource-tuning options. Preview migration preserves its `127.0.0.1` default
and `--host` through the explicit contextual constructor. GI#518 records the
address validation and error contract.
