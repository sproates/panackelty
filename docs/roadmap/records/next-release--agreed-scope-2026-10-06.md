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
| alpha.15: strengthen the foundations | Remaining compiler understanding GI#180 (GI#173 provenance and GI#134 explanations); GI#435 validated domain types with multiline, parenthesised conjunctions; GI#170 inference assessment; GI#409 compiler-facing stdlib assessment; full package-management implementation GI#200, arranged as P01–P04 under RM#44 and tracked as RM#158–RM#161; RM#154 faster coverage, RM#155 selected friction improvements; GI#106 validation-speed milestone. | All previously agreed bounded outcomes remain. Package-management implementation follows the [P01–P04 plan](dependency-management--idea.md#alpha15-implementation-plan); GI#200 stays open through final acceptance. Test-speed acceptance stays here rather than moving with the old alpha.15 theme. |
| alpha.16: build useful programs | GI#166 typed HTTP exchanges, GI#167 recursive file discovery, GI#168 subprocess examples, GI#169 tiny build tool, GI#138 tested guides coordinated with GI#310; RM#157 performance review and bounded quick wins; GI#435 alternatives (`&#124;&#124;`) for the new string, collection and record-relationship guards, including proof rules and diagnostics; existing numeric alternatives are preserved. | Prove installed success/failure workflows; no general RPC framework or production build system. Full GI#552 API-reference completion and GI#553 load profiling were only proposed for old alpha.15; their corresponding candidate slot is now alpha.16, not an automatic commitment. |
| alpha.17: improve everyday development | GI#139 one-editor support, GI#136 source-aware runtime errors, GI#147 Homebrew installation. | No full language server/debugger commitment. GI#555's proposed fallback slot moves here if not selected for alpha.14. |
| alpha.18: interactive and connected applications | GI#165 browser event handlers; GI#142 bounded service/library contract testing. | Contract design follows concrete use cases; no general DOM/UI framework. Docker toolchain images (GI#146) and Linux ARM64 releases (GI#145) remain optional. |
| alpha.19: improve language ergonomics | GI#274 error propagation and further GI#435 guard families, subject to design/evidence. | Beyond alpha.15's strings, collection sizes and cross-field records. Targeted inference and remaining stdlib discoverability may support the release; they are not automatic gates. |

### Agreed alpha.15 delivery order — 26 October 2026

The owner has agreed the following order within the existing scope boundaries.
This fixes delivery sequencing; it starts no implementation, changes no acceptance
criteria and earns no additional completion credit. GI#200: Packages remains
accepted on next, subject to release integration and publication checks.

| Order | Work | Purpose and handoff |
| --- | --- | --- |
| 1 | GI#409: Compiler/stdlib boundary assessment | Complete the dependency map, five change cases and recommendation. Inform subsequent build/validation decisions; no refactor is authorised. |
| 2 | GI#106: Validation speed | Measure the agreed baseline, agree targets and the bounded improvement selection, then deliver and obtain owner acceptance. Faster trustworthy checks benefit later work. |
| 3 | RM#154: Faster coverage | Measure and agree the target, implement validated plan reuse with fresh executions, and complete its acceptance. Later compiler changes must exercise the invalidation rules. |
| 4 | GI#180: Compiler understanding — foundations | Complete the two inventories and establish missing checker evidence, explanation and instruction/source-attribution mechanisms. Keep final completeness acceptance open for the new domain-type decisions. |
| 5 | GI#435: Validated domain types | Deliver the approved contract using those mechanisms. Include explanations and provenance as new checking/proof decisions are introduced; do not add them as a later retrofit. |
| 6 | RM#155: Authoring friction | Deliver the three agreed remaining improvements against the resulting compiler and reconcile their evidence. Its independent documentation work may happen earlier. |
| 7 | Integrated acceptance | Finish GI#180 completeness, verify the combined features and complete release acceptance, including package/install checks. This is acceptance within existing scopes, not another independently weighted feature. |

The first three steps are a practical delivery order, not newly invented hard
technical prerequisites. In particular, a substantial obstacle in RM#154: Faster
coverage need not block feature work: report the evidence and make any sequencing
adjustment explicit. Do not silently change release scope or substitute alpha.16
native coupling or RM#157: Performance review for these bounded tasks.

The integration dependency is between GI#180: Compiler understanding and GI#435:
Validated domain types. The mechanisms precede the new feature, and final
understanding acceptance includes all new checking/proof decisions and source
attribution. Existing GI#180 delivery slices remain the implementation structure;
the foundations/final-acceptance split does not replace their requirements or
count their work twice. Every delivery includes its own tests and required
validation; the final acceptance step does not defer those checks.

Next delivery: GI#409: Compiler/stdlib boundary assessment. Its agreed scope is
small to medium, provisionally one to two assessment PRs. No assessment starts
with this planning record. The other existing estimates and release weights are
unchanged. This documentation-only sequencing decision has no runtime/performance
impact and changes no website capability or release claims.

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
  ordering are now also agreed in RM#153. The [approved consolidated contract](../../design/VALIDATED_DOMAIN_TYPES.md)
  fixes the remaining APIs, error schema, Unicode version and acceptance examples
  following owner approval of PR#616. Implementation remains unstarted.
- New GI#435 proof decisions shipped in alpha.15 must also have GI#134 explanations.
  Define that integration once, without counting its delivery twice. Agree example
  transcripts before implementing output, including failure and unavailable cases.
  Use realistic multi-module, generic, guard and effect examples plus maintained
  code for acceptance; preserve ordinary compilation and bootstrap correctness,
  and measure query latency and ordinary compilation overhead.

Runtime value tracing, change-impact prediction and automatically deriving fixes
remain outside the selected GI#180 alpha.15 workstreams. A debugger, interactive
compiler visualiser and stable public compiler-IR API are not selected. The agreed completion scope below defines inventories, acceptance and delivery
boundaries. Reconcile the detailed inventories before implementation; if scope
is too large, seek an explicit release decision rather than redefine complete.

### GI#180: Compiler understanding, agreed alpha.15 completion scope

Owner agreement, 26 October 2026: finish the two selected workstreams below.
This is completion of existing facilities, not a new compiler subsystem or a
restart of the wider programme. Existing accepted deliveries remain accepted.
This decision authorises scope definition, not implementation.

**GI#134: Compiler explanations.** Explain every relevant type, effect and proof
decision made while checking a function. State the requirement, the actual
outcome, the types/constraints/facts used, their source locations and why those
facts justify the result. If checking cannot reach a decision, name the missing
information or earlier error. A decision the checker made but Explain omitted
is an implementation gap, never an acceptable unavailable result. Reporting
that the checker accepted or rejected something is insufficient without the
supporting reason.

**GI#173: Compilation provenance.** Connect source constructs through actual
checking and transformation to emitted bytecode in both directions. Every
instruction identifies its responsible source construct, or a genuine
compiler-generated origin with a reason. Source queries explain multiple or
shared instructions and constructs with no executable output. Imports, generic
bodies, method calls and generated branch/loop machinery retain accurate
identities. A stale or mismatched artifact produces an explicit validation
failure rather than plausible attribution. Mapping alone is not a complete
explanation of the compilation journey.

#### Existing evidence and finite inventories

Inspection of `next` at `8a5cc01` confirms substantial explanation coverage and
the validated source-map foundation. The explanation guide already lists eleven
checker-decision families; use it as the starting inventory, not eleven complete
tasks or a complete count of missing decisions. Generic inference reasoning and
some host-operation argument checks still lack complete retained evidence.
Some renderers give a generic rejection/deferral reason despite more specific
checker facts. The current source-map model is sparse and permits missing
instruction entries, including stores, loop mechanics and implicit returns.
These are observed gaps, not a completed audit or new implementation results.

Maintain two checked inventories:

- **Checker decisions:** reconcile every relevant active function-checking
  decision site with its retained evidence, human-readable output, source
  attribution and tests. Include generic inference/substitution, calls and
  builtins, operators, locals/results, joins, patterns, effects and proofs.
- **Emission and transformations:** reconcile emission paths and transformation
  cases with instruction origins, reverse mappings, explanations and tests.
  Include paths producing no instruction, multiple instructions or shared code.

Each entry identifies the source site, reachable accepted/rejected/unavailable
outcomes, output and regression evidence. Mark gaps explicitly. Tests must detect
missing evidence or mappings, not only compare the entries that happen to exist.
Adding an alpha.15 language decision updates its inventory and evidence in the
same delivery. New **GI#435: Validated domain types** decisions are included;
count their integration once across the two workstreams.

Completeness means no unexplained inventory gaps, not a collection of successful
demonstrations. A genuinely unreachable outcome needs a reason, not a fabricated
test or invented explanation. Inventory reconciliation precedes implementation
slicing; it does not permit silently narrowing the agreed completeness target.

#### Delivery and acceptance

| Slice | Required result |
| --- | --- |
| 1. Complete checker evidence | Retain missing decisions and their actual supporting facts. |
| 2. Complete explanations | Explain reasons, inference chains and specific unavailable states. Agree representative complete transcripts before changing rendered output. |
| 3. Complete instruction attribution | Every emitted instruction has a truthful origin; preserve validated artifact/source identity. |
| 4. Connect the compilation journey | Source-to-instruction queries explain transformations, shared mappings and constructs producing no instructions. |
| 5. Acceptance and documentation | Realistic examples, complete regression evidence, installed-tool checks and comparable cost measurements. |

Each slice includes its own tests and applicable canonical validation. The final
slice is integration acceptance, not permission to defer tests until the end.
The work is provisionally large, approximately five to seven implementation PRs,
including tests, documentation and integration; reconcile inventories before
turning that range into a committed packet count.

Acceptance covers accepted and rejected programs, partial checking after errors,
imported/generic code, method calls, control flow, callbacks, mutation-invalidated
facts and the new domain constraints. Use small boundary fixtures and realistic
multi-module/maintained programs. Compare explanations with actual checker
outcomes and provenance with emitted instructions, including deliberate missing
mapping/evidence regressions. Do not accept plausible prose as proof of coverage.

Evidence retention must preserve ordinary checking outcomes and executable
behaviour, including bootstrap determinism. Reuse the current opt-in evidence and
validated-sidecar mechanisms; do not make runtime execution depend on a sidecar.
Any required metadata format change needs explicit versioning and compatibility
coverage, not an unannounced executable-bytecode or public IR contract change.
Measure compilation time, query time, memory and metadata size with collection
enabled and disabled, recording workload, revisions and platform. Report material
regressions with their disposition; no new arbitrary timing target is selected.

Runtime value tracing, new inference capabilities, change prediction, automatic
fixes, a debugger/visualiser and a stable public compiler-internals API remain
outside this alpha.15 scope. Explanations of existing inference are included;
**GI#170: Inference assessment** does not become a dependency for that work.

Scope recording itself changes no runtime/performance behaviour. The website's
broad planned compiler-understanding claim remains accurate; no immediate website
edit is needed. Detailed examples and capability claims follow verified delivery
and the release/website publication process.

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

Owner agreement, 26 October 2026: the alpha.15 outcome is an evidence-backed
assessment and recommendation, not a refactor. Establish which library changes
genuinely require rebuilding the compiler and which rebuilds result from source
organisation or broad build rules. Distinguish that question from needing to
edit the compiler/VM interface when adding a native host capability.

The assessment has three deliverables:

1. **Dependency map.** Account for the compiler's direct, transitive and implicit
   standard-library dependencies. Distinguish Panack source dependencies,
   compiler-seed/bootstrap requirements, native interfaces and release/version
   coupling. Identify dependency paths and why the compiler consumes them;
   an import list alone is insufficient.
2. **Change-impact table.** Cover each of the five cases below. Identify what
   currently rebuilds, why, what must rebuild for correctness and what validation
   remains necessary. Support conclusions with source/build-rule evidence and
   small reproducible probes, recording the revision and commands. Separate
   observed behaviour from inferred necessity; do not treat a broad validation
   run as proof that every component needs rebuilding.
3. **Recommendation.** Compare retaining the current arrangement, tightening
   dependency/build rules and introducing an explicit compiler-support library
   boundary. Explain benefits, costs, bootstrap implications and concrete
   follow-up work. Recommend the smallest justified next step; moving modules
   or adding an abstraction layer is not a predetermined outcome.

| Change case | Question the evidence must answer |
| --- | --- |
| Application-facing library code the compiler does not use | Does this rebuild the compiler despite having no source dependency, and which rule causes it? |
| A library helper used by the compiler | Which dependency path requires rebuilding, and which artifacts are affected? |
| An implicit core type or method | Which compiler, bootstrap and runtime contracts depend on it? |
| A new host capability, such as a filesystem or process operation | Which source dependencies and native/compiler interfaces change, and why? |
| Packaging or version metadata without changed behaviour | Which rebuilds arise from artifact identity or release policy rather than changed compiler behaviour? |

**Completion:** all five cases are accounted for, the dependency map covers the
compiler's complete library closure, and the recommendation identifies the
smallest justified next step with supporting evidence. A recommendation to make
no organisational change is valid if supported. Unverified dependencies or
unexplained rebuilds must remain visible and prevent claiming the assessment
complete. Small probes are evidence gathering, not production refactoring.

Keep **GI#106: Validation speed** separate: that task delivers faster developer
feedback. The alpha.16 native-coupling work may change the architecture; this
assessment can inform it but does not implement it. Do not add compiler, VM,
standard-library or packaging redesign, separate compilation, or independent
component releases to alpha.15 through this assessment.

Effort is provisionally small to medium, approximately one to two assessment PRs
including evidence and documentation. Scope agreement does not start the
assessment or claim findings. This planning update has no runtime/performance
impact. The website already describes a library-boundary assessment in alpha.15;
that remains accurate and needs no immediate edit.


<a id="rm-154"></a>

**RM#154: Incremental coverage collection: alpha.15 requirement.**

Owner agreement, 26 October 2026: deliver faster repeat source-coverage collection
by safely reusing generated coverage plans while running every required test
afresh. This is working optimisation scope, not merely a measurement report.
Scope recording starts neither profiling nor implementation.

Inspection of `scripts/source_coverage_baseline.cjs` on `next` confirms that the
collector reuses plans within one run but creates a new temporary workspace and
regenerates them in a subsequent run. Persistent reuse is the bounded first
implementation direction. The historical 29m48s run for 31 sessions and 45 files
([Check 38775373415](https://github.com/sproates/panackelty/actions/runs/38775373415))
is context, not a current baseline or fixed target.

#### Workloads and target checkpoint

Measure four workloads against the same declared corpus and comparable toolchain,
platform and settings:

1. Empty plan cache: full cold generation and fresh execution.
2. Unchanged repeat: matching inputs with reusable plans and fresh execution.
3. Reporting-only change: unchanged measured programs, with fresh execution and
   regenerated reporting/publication output.
4. Measured-source or dependency change: affected plans invalidate; demonstrate
   reuse only where dependencies and identities still justify it.

Separate plan generation, test execution, aggregation and publication costs.
Record revisions, cache state, input sizes, repetitions and variation, with both
stage timings and end-to-end results. Measure local and hosted collection where
persistent reuse is intended to operate. Do not compare a reduced corpus with the
old full corpus and call it a speed improvement.

After profiling, bring a concrete numerical improvement target for owner agreement
before proceeding beyond the baseline into cache implementation. Preserve cold-run
costs and unchanged/slower workloads in the report. If plan generation is not a
material cost, return with that evidence and a scope decision rather than building
a cache and claiming success from cache-hit counts alone.

#### Reuse and fresh execution

A reusable plan must match the exact compiler, VM and exporter identities,
transitive measured-source inputs, plan format/schema and relevant settings.
Specify the key, stored files, provenance validation and trusted cache sources
before implementation. A matching key alone does not establish valid contents.
Missing, corrupt, partial, untrusted or invalidated entries safely regenerate;
no stale plan may be executed. Log why each plan is reused or rebuilt. Changes to
reporting alone must not invalidate plans unless they alter a genuine plan input.

Execute every required test and its assertions afresh. Create new session
identities, execution counters and reports; never cache passing results, prior
sessions or execution counters. Preserve source/toolchain identity checks and
changes-during-collection detection. Reports identify the current collection and
revision even when preparation was reused from compatible earlier inputs.

#### Acceptance and boundaries

- Demonstrate materially faster repeat collection against the agreed target,
  with replayable before/after measurements for all four workloads.
- Verify cached/fresh plan identity and equivalent inventories, probe definitions,
  attribution and coverage semantics. For deterministic acceptance fixtures,
  compare resulting reports allowing only explicitly listed per-run differences,
  such as timestamps and session IDs. Do not mask differences in required test
  execution, assertions, measured scope or provenance.
- Test invalidation for compiler, VM, exporter, direct/transitive sources,
  format/schema and settings. Test missing, corrupt, incomplete and untrusted
  cache entries and recovery. Retain evidence that assertion failures still fail
  a warm collection and that stale or duplicate sessions are rejected.
- Preserve the complete selected corpus, explicit unmeasured/unavailable entries,
  policy checks and source provenance. No higher timeout or weaker test assertion
  can establish completion.

The weekly/manual collection schedule remains unchanged. Coverage report freshness
and percentages are informational under the existing release policy, not new
release gates. Preserve existing integrity requirements without reinstating a
coverage freshness gate from historical wording.

Do not expand the measured corpus, cache execution results, rewrite the coverage
system or weaken validation. **GI#106: Validation speed** remains the separate
developer-feedback task; do not count this implementation there as well. The
coverage publisher continues to receive static reports rather than executable
plans; core collection owns plan generation and reuse.

Effort is provisionally medium, approximately three implementation PRs: baseline
and cache contract; reuse implementation with correctness tests; hosted acceptance
and documentation. Each delivery includes its applicable tests; the final PR is
integration evidence, not deferred correctness testing. Completion requires the
implemented improvement and acceptance evidence, not scope approval alone.

This planning change has no runtime/performance impact. The future optimisation
must quantify collection cost and preserve coverage integrity. The website's
existing broad faster-coverage plan remains accurate; no immediate website edit
is required. There is no dedicated issue for this scoped record; RM#154 remains
the task identifier.


<a id="rm-155"></a>

**RM#155: Authoring friction: alpha.15.**

**Completion scope agreed by the owner, 26 October 2026.** The selected list is
fixed. Distinguish intended behaviour that needs better explanation from a
behaviour change; reproduce each remaining case before implementing its remedy.

Already delivered on next:

- [PR#545: Example-inventory preflight](https://github.com/sproates/panackelty/pull/545)
  detects inventory drift before expensive validation.
- [PR#565: Contributor and interpolation guidance](https://github.com/sproates/panackelty/pull/565)
  links existing onboarding guides and explains binding an expression to a local
  name before interpolation. This remains completed work, not a new deliverable.

Three remaining improvements:

1. **Diagnose unsupported interpolation.** Unsupported expressions such as
   `${package.name}` must produce a source-located, actionable compiler error
   instead of silently appearing unchanged in output. Explain how to bind the
   expression to a local name first. Preserve supported local-name interpolation;
   full expression interpolation is outside scope. This is a deliberate
   compatibility change for programs relying on literal unsupported forms:
   document and test how to write literal `${…}` text deliberately, and migrate
   affected maintained examples. Do not imply that currently unsupported escapes
   already provide that mechanism or expand this into a string-syntax redesign.
2. **Provide a bounded set of tested API examples.** Cover strings and bytes,
   collection access, reporting test results, and filesystem/process failure
   handling. Show the actual API names and relevant limitations. Link the examples
   from existing guides and reuse the authoritative reference rather than adding
   a competing API manual. These four groups are the selected documentation scope.
3. **Explain public-signature type visibility.** Provide a worked example showing
   why a privately imported type cannot appear in a public function signature and
   how `pub import` makes the exposed type accessible. Check the rejected and
   corrected examples. Preserve the existing visibility rules; this item requires
   guidance, not a change to type accessibility.

Acceptance requires a reproducible before/after example for each remaining
improvement, regression tests for changed compiler behaviour, and checked
executable documentation examples. Verify useful diagnostics, continued supported
interpolation and deliberate literal text. Reconcile the recorded findings with
this selected list and its evidence; distinguish resolved, unselected and
unverified observations. New observations do not automatically expand alpha.15.
An actual deferral of selected work requires an explicit following-release
allocation rather than silently dropping it into the backlog.

Keep GI#134: Compiler explanations, GI#170: Inference assessment, RM#154: Faster
coverage and GI#106: Validation speed separately accounted for. Native coupling
and a comprehensive API-reference project are outside this selected list. Do not
count the same delivery twice or treat authoring mistakes as compiler defects.

The remaining work is provisionally medium, about three implementation PRs:
interpolation diagnostics and compatibility; API examples; visibility guidance
and final evidence reconciliation. Each slice includes its own relevant tests.
No additional implementation starts with this scope record. No dedicated GitHub
issue was found for this roadmap item; none is created solely for bookkeeping.

This scope-only update has no runtime or performance impact. Implementation must
record proportionate performance evidence for compiler changes. Existing broad
website claims remain accurate; no immediate website change is needed. Publish
changed syntax guidance with the corresponding verified implementation/release.

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

**RM#157: Performance review: alpha.16.**

Current allocation: **alpha.16**, by owner decision on 26 October 2026. This is
an explicit next-release commitment, not an unscheduled backlog item. The review
and bounded improvement scope remains; detailed workloads, targets and selected
improvements still need agreement. No review or implementation has started.

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
quick wins during alpha.16 where they fit the release; record larger or
uncertain opportunities with evidence and a recommendation for later selection.
Preserve correctness, fresh required test execution and existing validation
coverage. This review does not set a universal performance target or commit to
implementing every identified optimisation. Agree specific workloads, targets
and any larger implementation scope during planning, after the initial evidence
is available. Coordinate baseline work with [RM#56: Performance baselines](current-status-and-grooming-2026-10-01.md#rm-56)
and ongoing policy under [RM#123: Standing performance engineering](standing-performance-engineering--ongoing.md#rm-123).


Website: the release descriptions make no alpha.15 performance-review commitment;
the broad foundation and useful-program themes remain accurate, so no immediate
website edit is required. This allocation change has no runtime or performance impact.

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
