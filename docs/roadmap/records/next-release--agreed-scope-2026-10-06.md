<a id="next-release--agreed-scope-2026-10-06"></a>

[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

## Release plans and decisions

**Current status (6 November 2026): alpha.15 is released.**
Alpha.16 is the next planned release. The allocation table below supersedes
older release assignments in the retained decision history. Earlier planning
text explains the original decisions, rather than reopening completed gates.
See [alpha.15 acceptance](../../releases/ALPHA15_RELEASE_ACCEPTANCE.md).

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
| alpha.15: strengthen the foundations | Remaining compiler understanding GI#180 (GI#173 provenance and GI#134 explanations); GI#435 validated domain types with multiline, parenthesised conjunctions; GI#409 compiler-facing stdlib assessment; full package-management implementation GI#200, arranged as P01–P04 under RM#44 and tracked as RM#158–RM#161; RM#154 faster coverage, RM#155 selected friction improvements; GI#106 validation-speed milestone. | All previously agreed bounded outcomes remain. Package-management implementation follows the [P01–P04 plan](dependency-management--idea.md#alpha15-implementation-plan); GI#200 stays open through final acceptance. Test-speed acceptance stays here rather than moving with the old alpha.15 theme. |
| alpha.16: build useful programs | [RM#162: Real-program fixes](#rm-162), an explicit implementation allocation with scope discovered during use; GI#170 inference/diagnostics assessment; GI#166 typed HTTP exchanges, GI#167 recursive file discovery, GI#168 subprocess examples, GI#169 tiny build tool, GI#138 tested guides coordinated with GI#310; RM#157 performance review and bounded quick wins; GI#435 alternatives (`&#124;&#124;`) for the new string, collection and record-relationship guards, including proof rules and diagnostics; existing numeric alternatives are preserved. | Prove installed success/failure workflows; no general RPC framework or production build system. Full GI#552 API-reference completion and GI#553 load profiling were only proposed for old alpha.15; their corresponding candidate slot is now alpha.16, not an automatic commitment. |
| alpha.17: improve everyday development | GI#139 one-editor support, GI#136 source-aware runtime errors, GI#147 Homebrew installation. | No full language server/debugger commitment. GI#555's proposed fallback slot moves here if not selected for alpha.14. |
| alpha.18: interactive and connected applications | GI#165 browser event handlers; GI#142 bounded service/library contract testing. | Contract design follows concrete use cases; no general DOM/UI framework. Docker toolchain images (GI#146) and Linux ARM64 releases (GI#145) remain optional. |
| alpha.19: improve language ergonomics | GI#274 error propagation and further GI#435 guard families, subject to design/evidence. | Beyond alpha.15's strings, collection sizes and cross-field records. Targeted inference and remaining stdlib discoverability may support the release; they are not automatic gates. |

### Real-program fixes — alpha.16 allocation, 6 November 2026

<a id="rm-162"></a>

**RM#162: Real-program fixes. Status: In progress for alpha.16. Overall scope and effort: unknown.**

The current [async-stdlib batch](../../ASYNC_DESIGN_DISCUSSION.md#implementation-batch-selected-8-november-2026)
adds clock/environment async reads and consistent paths for existing async host
and filesystem operations. The uptime HTTP service is its integration example.
Focused checks and the complete local `make check` passed (582s, 8 November);
integration and hosted validation remain pending. This does not commit the
separate state/closure design.

Reserve a real delivery workstream for issues encountered while the owner builds
useful programs in Panackelty. Its purpose is to fix the problems and return to
the program that exposed them, not merely to produce an issue list or defer the
work to a later release. This is part of alpha.16, not spare-time work after the
named features. No number of fixes, time budget or percentage weight is yet agreed.

The earlier attempt to read the system clock inside an HTTP response handler
illustrates the kind of obstacle this workstream covers. It is context, not a
claim that the same problem remains unfixed in the current release; reproduce
each case against the current toolchain first.

Eligible discoveries include correctness defects, missing capabilities needed by
the real program, API or effect restrictions, confusing diagnostics, and tooling
or documentation gaps that obstruct normal use. Preserve the intended language
and safety contracts; fixing friction does not mean bypassing a valid check.

For each discovery, record the concrete program/use case, reproduction, expected
behaviour and smallest useful fix. Define the individual outcome when it becomes
known, implement it, add proportionate regression coverage and affected guidance,
and verify that the original program can continue. Reuse existing work items
where they cover the same outcome; do not count one fix twice under this slot and
a named alpha.16 workstream. Creating an issue alone does not complete the fix.

The workstream runs alongside building the real programs. Routine fixes within
this remit can proceed through normal delivery; ask for a decision when a fix
requires a material design choice or threatens the release's size or schedule.
Do not silently substitute a workaround or put the problem back in the backlog.
If deferral is agreed, allocate it explicitly to alpha.17 under the release
deferral rule unless the owner chooses another destination.

Because discoveries are open-ended, the workstream has no invented fixed task
count or automatic 100% completion when its current list is empty. At the
alpha.16 scope cutoff agreed with the owner, account for every selected finding:
delivered and verified against its originating program, or explicitly deferred
with its release allocation. Do not promise to discover or fix every possible
issue before shipping.

Website: no update requested. By owner direction on 6 November 2026, this
placeholder is for planning in the core roadmap; leave the published website
scope unchanged. Website PR #74 was closed without merging.

### Inference assessment allocation, 26 October 2026

GI#170: Inference assessment is selected for **alpha.16**. Its bounded assessment
scope and acceptance criteria are unchanged; no inference architecture or
implementation is selected. GI#134's complete explanations of existing inference
decisions remain in alpha.15. This allocation changes no other release item.
The [release deferral rule](../ROADMAP_PROCESS.md#release-deferrals) requires an
explicit following-release allocation rather than returning deferred work to an
unscheduled backlog.

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

Owner decision, 2026-10-22: superseding the 15 October deferral, select full
package-management implementation for Alpha.15. GI#200 remains open through
final acceptance, with P01 lock/restore contract, P02 local dependencies, P03
pinned remote sources and P04 `panack` integration. See
[RM#44](dependency-management--idea.md#rm-44) for packet status and the accepted
source scope.

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


**GI#409: Compiler/stdlib boundary assessment — complete on merge, 27 October 2026.**
The [assessment](../../COMPILER_STDLIB_BOUNDARY_ASSESSMENT.md) verifies all 42 loaded
sources and the nine-module stdlib closure against Panack's own inventory. Five
change cases separate actual cache compilation, Make dry-run scheduling, source
semantics, native registry contracts and release metadata. Retain the existing
source boundary; investigate the demonstrated package-tool dependency overreach
within GI#106: Validation speed. No new compiler-support layer or refactor is
selected. Native coupling remains separate alpha.16 work. All assessment
acceptance is covered; optimisation is not required to close this assessment.

<a id="rm-154"></a>

**RM#154: Incremental coverage collection: alpha.15 requirement.**

Owner agreement, 26 October 2026: deliver faster repeat source-coverage collection
by safely reusing generated coverage plans while running every required test
afresh. This is working optimisation scope, not merely a measurement report.
Scope recording starts neither profiling nor implementation.

**Done, 28 October 2026:** local and hosted preparation reuse (PR#644/PR#645) meet the agreed four-workload targets, with equivalent results and fresh sessions. See [final acceptance](../../COVERAGE_PERFORMANCE_BASELINE.md#final-acceptance-28-october-2026). Cold preparation remains slow; the fundamental approach will be reviewed separately after alpha.15.

**Planned for Alpha.14 by owner direction, 2026-10-15.** Improve code-coverage
collection speed, particularly repeated release checks after changes that do not
alter the measured program. During Alpha.13 preparation, reporting-only repairs
repeated expensive plan generation. Successful source-coverage
[Check 38775373415](https://github.com/sproates/panackelty/actions/runs/38775373415)
took 29m48s for 31 fresh sessions and 45 measured files. This is an observed hosted
baseline, not a fixed performance target. Profile generation, execution,
aggregation and publication separately before selecting further optimisations.

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

**Selected Alpha.15 documentation improvements, 2026-10-22:** contributor
onboarding links to the existing first-program, language-tour and testing guides;
the language tour explains that interpolation takes a local name and shows
binding an indexed value before using it. These changes improve discovery and
clarify the existing syntax without changing language behavior. Website: no
impact; no live website claims or release information change. No issue was
created solely for this documentation work.

**RM#155 tested API recipes, 4 November 2026:** owner selected the independent
examples slice while GI#180 awaits input. Implementation covers strings/bytes,
collections, testing reports and filesystem/process failures through runnable
programs, an [API recipes guide](../../guides/API_RECIPES.md) and exact functional
transcripts. Baseline `7d60c45` had short strings/collection examples but no user-facing
testing-report or host-failure recipes. The new guide makes those existing
contracts discoverable without reading test implementations. [PR #652](https://github.com/sproates/panackelty/pull/652) merged fourteen
source/bytecode recipes and a local HTTP fragment example into `next`.

**RM#155 final acceptance (effective on merge).** All three improvements selected
in [the agreed scope](https://github.com/sproates/panackelty/pull/626) have
reproducible evidence:

| Observation | Before | After |
| --- | --- | --- |
| Unsupported string interpolation | Printing `"${name[0]}"` printed those characters unchanged. | The [rejected fixture](../../../tests/functional/failures/interpolation_expression/main.panack) points at the opening dollar and gives the local-binding remedy. The [working case](../../../tests/functional/cases/expression_contracts/main.panack) exercises both that remedy and deliberate literal text. |
| API examples were hard to find | Strings and collections had brief examples. Test reports and host failures required looking up test code. | The [recipe guide](../../guides/API_RECIPES.md) links complete programs, commands and expected results. |
| An imported type was usable inside a module but inaccessible to its callers | A [private import](../../../tests/functional/failures/private_signature_import/main.panack) fails when `weekend()` publicly returns `Trip`. | The [worked module guide](../../../examples/namespaces/README.md#returning-a-type-from-another-module) shows the public import and a caller naming and using the type. |

The first finding changes compiler behaviour. The other two resolve gaps in
guidance. General expression interpolation, broader API-reference coverage and
native-interface coupling were outside the selected scope. Log suggestions
without a reproduction remain unverified and earn no release credit.

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

## GI#180: definitive alpha.15 delivery checklist, 28 October 2026

This is the execution tracker for the five agreed compiler-understanding slices.
It supersedes the conversational stage proposals, without changing the completion
scope agreed in PR#619. See the current delivery decision below. Status values are not
started, in progress, blocked, and complete; each completed stage must link its
implementation and acceptance evidence here. Update NEXT_RELEASE_STATUS.md with
slice transitions. These slices share GI#180's existing weight; no extra credit.

### Review findings and reuse rules

- Existing checker evidence is substantial: SubtractionEvidence, EffectEvidence,
  GuardConversionEvidence, BindingTypeEvidence, AssignmentTypeEvidence,
  ReturnTypeEvidence, CallArgumentEvidence, TypeJoinEvidence,
  CollectionCallbackEvidence, OperatorTypeEvidence and BodyTypeCheckEvidence.
  Extend the existing producers in checker.panack, module_bodies.panack and
  module_effects.panack where a concrete gap is established. Do not introduce a
  parallel generic evidence system as an assumed prerequisite.
- The [explanation guide](../../guides/COMPILER_EXPLANATIONS.md#function-checking-decision-inventory)
  already contains a checker-decision inventory and known gaps. Refresh it against
  the current source; some prose still describes Void evidence as absent while
  later text records it as implemented. Documentation is evidence to reconcile,
  not proof that implementation is missing.
- The two scope inventories are checker decisions and emission/transformations.
  Neither is a replacement for the source inventory used by coverage. Forward
  and reverse source mappings belong to the second inventory, not two new ones.
- InstructionSource, FunctionSources, SourceMappedProgram, EmissionProbe and the
  existing validated source-map queries already provide attribution foundations.
  Complete and extend these where necessary; do not rebuild their functionality.
- Slice 1 records checker facts; slice 2 explains them. Slice 3 establishes
  instruction origins; slice 4 explains the transformations those origins support.
  Do not make every instruction carry a checker proof merely to link these views.
- Slice 4 is the existing command/query experience. Interactive keyboard
  navigation, expandable terminal trees and a REPL are outside alpha.15.
- Tests accompany implementation. Slice 5 integrates and reconciles their evidence;
  it does not repeat every earlier experiment or postpone validation to the end.
- GI#435 adds evidence, explanations and provenance with its own new decisions.
  GI#180 final acceptance includes those additions, but does not implement or
  count them a second time. Existing inference is explained; new inference
  algorithms remain outside this task.

### Shared preparation checkpoint — not a sixth implementation slice

- [x] P1 — Reconcile the existing checker inventory with actual active decision
  sites, evidence types, renderers and regressions. Enumerate reachable success,
  rejection and unavailable paths; give reasons for unreachable outcomes.
- [x] P2 — Reconcile the emission/transformation inventory with current emission
  sites, source mappings, query output and regressions. Include generated/shared
  instructions and constructs producing no executable code.
- [x] P3 — Freeze the resulting gap checklist for the existing language and map
  each gap to exactly one delivery owner below. Select representative transcripts
  and realistic programs for review, including the presentation subtraction case.
  Identify mechanism changes and revise the provisional PR estimate from evidence.

Output: finite, source-linked gaps and agreed example outputs. Reuse existing
inventories; maintain one authoritative entry per gap. This checkpoint precedes
implementation and earns no independent completion credit. It is a bounded audit
of the agreed scope, not a compiler redesign. Material architectural discoveries
must be reported before expanding the work; minor implementation choices remain
with the implementor.

### 1. Complete checker evidence — complete on merge

- [x] 1.1 — For each confirmed checker gap, specify the missing facts, actual
  decision site and existing evidence record to extend. Introduce a new record
  only where existing types cannot faithfully express the missing decision.
- [x] 1.2 — Retain facts at the point the checker makes the decision, including
  inference/substitution dependencies and specific causes of deferral. Preserve
  partial evidence after errors and distinguish a genuinely unreached decision
  from missing instrumentation. Implement by the finite decision-family checklist.
- [x] 1.3 — Reconcile evidence against active decision sites and reachable
  outcomes. Regressions must detect omitted evidence. Check retention enabled
  and disabled, preserving ordinary checking results and diagnostics.

Exit: all inventoried existing-language checker decisions have the required
truthful facts and source attribution. No prose redesign is required to declare
this evidence slice complete. New domain-type cases remain assigned to GI#435.

### 2. Complete explanations — complete on merge

- [x] 2.1 — Apply the owner-approved source-first interactive workflow to
  representative success, rejection, inference-chain, partial-check and
  unavailable cases. Lead with the conclusion and relevant supporting facts.
  Preserve plain output for scripts. The 4 November delivery decision supersedes
  repeated transcript-approval checkpoints.
- [x] 2.2 — Extend existing renderers to explain the slice 1 facts and their
  dependencies, with exact source locations and specific unavailable reasons.
  Reuse established wording across decision families; do not reconstruct proof
  by rerunning an independent checker inside the renderer.
- [x] 2.3 — Reconcile every checker-inventory entry with its output and reachable
  outcome regressions. Verify the agreed full transcripts and representative
  multi-module programs. Accepted sample wording is not proof of complete coverage.

Exit: every in-scope retained checker decision is explained accurately and
readably through the existing query surface. Slice 2 can proceed family by
family once the corresponding slice 1 evidence exists.

### 3. Complete instruction attribution — complete on merge

- [x] 3.1 — Classify confirmed mapping gaps against existing emission paths and
  source-map structures. Specify source origins or genuine generated origins,
  with reasons, preserving module/function identity and artifact validation.
- [x] 3.2 — Fill the missing origins through emission and transformations,
  including imported/generic code, methods and generated control flow. Preserve
  shared origins where required instead of selecting a misleading single source.
- [x] 3.3 — Check every emitted instruction against the emission inventory and
  reverse source mapping. Include deliberately omitted mappings and stale or
  mismatched artifact/source cases. Existing validated attribution must survive.

Exit: all emitted instructions have truthful, validated origins. Maintain the
source cases that produce no instructions in the inventory for slice 4; do not
invent instructions to give them something to point to.

### 4. Connect the compilation journey — complete on merge

- [x] 4.1 — Specify and verify queries showing how representative source constructs
  become instructions: direct, lowered, multiple, shared and no-output cases.
  Identify the actual transformation facts needed beyond slice 3's origin maps.
- [x] 4.2 — Retain missing transformation facts at their existing production sites
  and expose them through the existing source/instruction queries. Explain the
  relationship and reason, reusing checker evidence where actually relevant.
  Do not invent a chronological narrative or causal link the compiler did not record.
- [x] 4.3 — Reconcile each transformation-inventory entry with its query output
  and regressions. Confirm truthful explanations for shared and zero-output
  constructs, failed compilation, imports, generics and method/control-flow lowering.

Exit: source queries explain how and why the actual output arose, including
absence of executable output. This slice consumes slice 3 mappings; mapping
completeness is not implemented twice. The owner subsequently selected an
interactive exploration prototype; see the updated presentation decision below.

### 5. Acceptance and documentation — complete on merge

- [x] 5.1 — After GI#435 and other alpha.15 compiler changes, reconcile both
  maintained inventories against the resulting implementation. GI#435 supplies
  its new decision evidence in its own deliveries; resolve remaining integration
  gaps here. Do not declare whole-programme completeness before this checkpoint.
- [x] 5.2 — Exercise the agreed realistic programs and installed CLI: accepted
  and rejected programs, partial checking, imported/generic code, methods,
  callbacks, control flow, mutation-invalidated facts and new domain constraints.
  Reuse valid slice evidence and run the necessary combined acceptance checks.
- [x] 5.3 — Measure comparable retention-on/off time and memory for the agreed
  programs, assess material costs, and update user documentation, limitations,
  release summary and completion records. Claim only capabilities demonstrated
  by the final inventories and transcripts.

Exit: both inventories have no unexplained gaps, installed behavior matches the
agreed examples, costs are disclosed, and documentation matches the feature.

**Done on merge.** The [final integration results](../../COMPILER_UNDERSTANDING_ACCEPTANCE.md#final-alpha15-integration)
close GI#134 Compiler explanations, GI#173 Compilation provenance and the
agreed alpha.15 scope of GI#180 Compiler understanding. The two inventories
include static and runtime domain decisions, their sources across module
boundaries, and validation bytecode. The installed explorer and both
directions of the source-map query were exercised. Later research remains
allocated to its own issues.

### Earlier delivery checkpoint, 5 November 2026

The owner requested end-to-end delivery after reviewing the subtraction
explorer. PR #654 delivered that first interface. The delivery branch now
extends it to type, inference, callback, guard and effect decisions, assignment
mutability and generated instructions. The compiler retains statement and
control-flow origins, multiple origins and transformation purposes. Reverse
queries validate the artifact and source pair before selection.

Slices 1–4 are complete on merge, subject to the exact-head hosted checks. The [acceptance record](../../COMPILER_UNDERSTANDING_ACCEPTANCE.md)
maps C01–C10 and E01–E08 to implementation and regression evidence. The final
compiler seed reproduced exactly, full local `make check` passed, and checkout
and installed terminal navigation passed. Audits of the complete package manager
and compiler found zero missing or invalid instruction origins.

Comparable before/after time and memory measurements are recorded for the
package manager and compiler. Stage 5.1 and the new-domain-type
part of 5.2 still depend on GI#435. Existing-language realistic and installed
acceptance has passed; reuse that evidence and check the new integration when
GI#435 supplies its decisions. Whole-programme completion remains open.

### Ordering and tracking

P1–P3 precede implementation. Slice 1 feeds slice 2; slice 3 feeds slice 4.
Complete those foundations before GI#435, which integrates its new decisions as
it implements domain types. Slice 5 closes GI#180 after those additions and the
remaining alpha.15 compiler changes. Family-level implementation may connect
adjacent slices in one PR; update each affected checklist item without duplicating
work. A slice is complete only when all its stages meet their exit conditions.

The earlier five-to-seven-PR figure remains provisional pending P3. These stages
are tracking units, not promises of one PR each. No new implementation or test
execution is authorised merely by recording this plan. This documentation-only
change has no runtime, memory, build or website-capability impact.

### Preparation evidence register — source reconciliation, 28 October 2026

Baseline: next at 8e15658b1546cc7a96f4aeda854da77cda9bde1d.
P1 and P2 source reconciliation is **complete**. P3 is **in progress**: the
implementation packets are defined below; representative output remains proposed,
not agreed. Implementation slices are **not started**. This is a source/test
inventory checkpoint, not proof that the missing behavior has been implemented.
No compiler execution, rebuild or test run was needed to inspect these source
paths and existing test assertions. Referenced regressions below were read, not
rerun. Keep this register with the execution checklist; do not duplicate the
existing checker-family inventory in another document.

| ID | Source evidence and finding | Disposition / owner | Implementation acceptance / regression needs |
| --- | --- | --- | --- |
| C01 | `module_bodies.panack`: `bind_body_block` retains rejected non-Void storage requirements; `check_body_array` retains rejected non-Void array-element requirements as `BodyTypeCheckEvidence`. `compiler_suites/module_bindings.panack` and `cli_explanations/main.panack` already assert these facts and output. | Existing implementation; no slice 1 rebuild. Correct the stale guide claim during documentation reconciliation. | Check other reachable value-use paths separately; these cases do not establish universal coverage. |
| C02 | `body_check_typed_intrinsic` calls `body_require_value` for every typed argument but retains `CallArgumentEvidence` only for an explicit name list. Missing argument-bearing names are `run_bytecode_args`, `process_exit`, `host_exit_async`, `process_run`, `process_run_isolated`, `host_sleep`, `host_decode_utf8`, `read_file`, `file_exists`, `read_bytes`, `write_bytes`, `write_file`, `host_stdout_async` and `host_stderr_async`. Streaming HTTP, staging, digest, gzip and filesystem operations are already on the retention list. | Slice 1 owns missing argument evidence; slice 2 owns corresponding output coverage. Reuse `CallArgumentEvidence` and existing arity evidence. | All 14 names use the same accepted/rejected/deferred argument-checking loop. Zero-input calls (`read_line`, `command_args`, `host_arguments_async`) need arity coverage, not invented argument evidence. Check preservation of previously checked children on early exits under C09. |
| C03 | `BodySolution` and `BoundBody.constraints` retain solutions, while `body_apply_solutions` and `body_finish_constraints` turn unification failures into diagnostics and deferred results into spans. This is not a retained chain of the originating facts and unification decisions. | Slice 1 owns inference-cause retention; slice 2 explains the retained chain. Preserve existing inference semantics. | Map each inference/unification/solution path and its source origin; determine the smallest extension to existing records. |
| C04 | Existing argument/binding/assignment/join records retain actual/expected types and `Option[Bool]` outcomes. Several renderer reasons describe acceptance or unavailable types generically. | Slice 2 owns specific reasons where existing facts suffice. Slice 1 owns only causes confirmed absent from retained evidence. | For each generic renderer, distinguish a missing fact from wording that fails to use an existing fact. |
| C05 | `module_effect_call` already records known effect boundaries and unresolved indirect calls. `module_local_functions` derives permission for ordinary wrappers around local-resource operations; `module_effect_call` uses that permission to allow calls inside async functions, but its evidence records only the target effect and declaration identity, not that permission or its supporting calls. Invalid bodies skip effect checking; deferred bodies can retain boundaries. | Slice 1 extends existing effect evidence with the actual permission and dependency facts. Slice 2 explains them. No second effect checker or inferred-effect algorithm. | Valid function signatures always carry an effect; invalid signature sets prevent publication of effect evidence in the loader. Do not add a user-facing unknown-direct-effect case for an internally incomplete signature. Assert local-resource permission causes and keep skipped body checking distinct from an allowed call. |
| C06 | `body_check_collection_core_call` retains callback type, expected inputs/result and one combined outcome. It checks purity, then arity, then parameters and result. Later checks can be skipped after an earlier failure; the combined result does not retain those individual outcomes. Hole/parameter/non-callable branches already have records. | Slice 1 extends `CollectionCallbackEvidence` with the performed requirements and causes; slice 2 explains each relevant result without pretending skipped checks ran. | Regression matrix: map/reduce success, non-callable, unresolved callable, impure callback, wrong arity, each parameter mismatch, map Void result and reduce result mismatch. Existing callback regressions are a base, not a reason to repeat the implementation. |
| C07 | `body_guard_conversion` and `body_subtraction_evidence` retain proof results and bounds. `Bounds` has a lower-bound origin but no upper-bound origin. `body_forget_written_fact` clears bounds without retaining the assignment that invalidated them. `body_require_value` can perform a guarded compatibility check without retaining its proof; proof retention in `body_refine` only covers callers that also refine. | Slice 1 owns missing proof causes at the existing decision sites, including mutation invalidation and conversions checked by specialized calls. Slice 2 renders those causes. Do not strengthen the proof engine or infer a runtime failure from insufficient proof. | Trace conversion retention through each require/refine caller and both condition branches. Account separately for a guarded value widening to its base: this is compatibility, not a new proof obligation. |
| C08 | The five `body_check_http_*_core_call` helpers for buffer, lifecycle, connection, writer and state operations check typed arguments but construct `BodyCall` with empty argument evidence. State/handler-shape decisions have diagnostics but no retained requirements. Several wrong-arity/invalid-state branches return `body_failure` before adding arity or child evidence. TCP and streaming HTTP have different paths and must not be conflated with these helpers. | Slice 1 adds argument, shape and performed-arity evidence using existing record families. Slice 2 provides their output. | Cover all names selected by these five helpers, including accepted checks, wrong count, wrong state/handler shape, argument mismatch and unresolved input. Exercise compiler-owned intrinsic calls through their real declarations as well as helper fixtures. |
| C09 | `body_call_with_receiver` checks children before dispatch. Some special-call early returns construct an empty failure body; the outer rejected-type-argument helper adds count evidence but cannot recreate discarded child records. Partial checking therefore needs preservation as well as better unavailable wording. | Slice 1 preserves already-produced evidence through rejected/deferred parents; slice 2 reports the exact stopping point. This is one cross-cutting repair, not a second implementation in each renderer. | Binding/assignment wrappers preserve checked children; function results refine then solve and retain their return requirement. Constraint solving preserves records but substitutes types without a decision history (C03). Early special-call failures and failed owned-method selection discard checked child/receiver records. Assert preserved earlier checks and explicitly skipped later checks. |
| C10 | The `FunctionExpr` body branch constructs a callable type from the resolved signature or rejects a generic function reference, but retains no requirement/outcome explaining that distinction. A checked expression type alone does not explain the rejection. | Slice 1 adds the callable-reference requirement using `BodyTypeCheckEvidence`; slice 2 explains the result. Name resolution itself stays outside scope. | Assert a non-generic callable reference, a resolved generic reference requiring a wrapper, and an internally incomplete signature separately. Do not label a failed name lookup as a failed callable-type decision. |
| E01 | `emit_block` appends stores for bindings/assignments and pops for expression statements without adding `InstructionSource` entries for those instructions. | Slice 3: complete statement instruction origins. | Determine available statement spans and any loss of source identity before emission; verify origin truth rather than copying the nearest expression span blindly. |
| E02 | `emit_block` appends while jumps/pops and for iterator initialization/advance/jumps without corresponding instruction-source entries. Coverage probes exist for loop decisions but are not instruction-origin records. | Slice 3: generated control-flow attribution. | Account for every generated instruction and enclosing construct; preserve existing coverage probes independently. |
| E03 | `NoBlockValue` emits a Void constant; `compile_program_emission` appends an implicit return after the emitted function body. Those paths do not add origins. | Slice 3: implicit-output origins with genuine generated reasons. | Identify the responsible block/function source and reachable cases, including empty bodies. |
| E04 | `compile_program_emission` produces no executable output for import/type/record/enum declarations and filters unused core functions. | Slice 4: explain no-output and selection cases. These are not missing instructions to manufacture. | Reconcile earlier transformations: a declaration may contribute meaning even when this emitter branch emits nothing. |
| E05 | `InstructionSource` contains instruction index, span and a lowered flag. The documented locate behavior uses a generic unavailable result for both missing attribution and invalid/stale pairs. | Slice 3 owns valid origin/identity checks and explicit validation failure; slice 4 consumes them in transformation queries. | Trace each failure return and CLI exit behavior; agree a distinct failure transcript before changing it. |
| E06 | Source maps already expose direct/lowered origins and validated function selectors. A lowered flag alone does not explain which transformation occurred or why. | Slice 4 owns missing transformation reasons, without rebuilding slice 3's mappings. | Reconcile short-circuit, calls, interpolation, match, generic sharing, methods and imported module identity with actual transformations and available query selectors. |
| E07 | `module_emission_block` builds bare expressions for literal and local-use expression statements, although their bound nodes have spans. `module_emission_expression_with_parameters` normally wraps expressions in `LocatedExpr`. The bare statement paths can lose origins before the emitter; fixing only instruction append sites would miss them. | Slice 3 preserves existing source spans through the module-emission bridge, alongside E01–E03. | Compare identity-emitted and direct frontend fixtures, including discarded literal/local/function expressions. Never substitute an initializer span for a complete statement without identifying the generated operation. |
| E08 | `source_map_arguments`/`run_source_map_command` accept an instruction index and function for `locate`. `source_map_location` returns one location and overwrites its result if another matching entry exists. There is no public reverse source-to-instructions selector or transformation explanation in this path. | Slice 3 owns truthful multiple-origin representation. Slice 4 owns reverse-query selection and result presentation as part of the already-agreed reverse mapping requirement. | P3/4.1 must specify source-led navigation and truthful reverse-query results. Interactive exploration is now selected for prototyping; do not describe reverse queries as an already available capability. |

The following paths have now been reconciled at source level. The eleven-family
checker inventory remains in [Compiler explanations](../../guides/COMPILER_EXPLANATIONS.md#function-checking-decision-inventory);
the rows below refine its findings rather than create another family inventory.

- Calls: direct argument count and explicit type-argument count already retain
  outcomes. Indirect target/arity and argument records exist. The outer
  `body_retain_rejected_type_argument_arity` covers rejected explicit arguments
  in special calls; adding a second count recorder there would duplicate work.
  Count comparisons have accepted/rejected outcomes; an unavailable count is
  meaningful for an unresolved callable, not for a known fixed-arity builtin.
- Joins/operators: arrays, conditional branches, match arms, concatenation and
  repeated generic parameters retain join results. Equality can call `body_join`
  and produce solutions without retaining a join record; C03 owns that missing
  inference path. Operator records retain operands and result but not all rule
  selection/deferred causes (C04). `await` is checked by the effect pass, not a
  missing ordinary unary-operator proof.
- Expression requirements: index receiver/position, iteration, fields, Boolean
  conditions and interpolation have requirement records. Field rejection can
  mean non-record, missing field or failed substitution; a generic rejected
  record does not distinguish those causes (C04). Guarded index receivers can
  defer despite a known nominal type; do not say the type was absent.
- Match: subject, membership, uniqueness, payload count/type, exhaustiveness and
  arm joins already retain evidence. Exhaustiveness is unavailable if enum
  variants are unavailable; uniqueness is a definite comparison of resolved
  identities. Pattern name-resolution failures are outside the body-decision
  explanation scope. Substitution causes and unavailable dependencies belong to
  C03/C04, not a new match-evidence architecture.
- Emission: literals, names, arrays, unary/binary operations, ranges, fields,
  indices and direct/indirect/constructor calls already append source entries
  when located. Short-circuit, conditional, match, interpolation and collection
  map/reduce machinery already retain generated origins. Preserve those paths;
  E01–E03/E07 identify actual omissions. `resolve_pending_jumps` replaces each
  placeholder with one jump, so existing instruction indices remain valid.
- Await rewrites the terminal invocation without adding an instruction; its
  mapping remains that of the call. Slice 4 must explain the await relationship,
  not claim that no transformation occurred because no new instruction appeared.
- Module emission uses declaration identities for imports/methods and shares
  generic bodies after erasing call type arguments. Declarations can contribute
  types or constructor metadata without standalone instructions. Only unused
  compiler-owned core functions are pruned; ordinary unused functions are kept.
  Explain that actual policy under E04/E06, not a general dead-code-elimination
  policy the compiler does not implement.
- `locate` validates by reproducing the bytecode and canonical map. Failed
  source preparation, missing/mismatched files and an ordinary missing entry
  currently converge on `source-map: unavailable` with status 0. Preserve the
  validation mechanism; E05 requires distinguishing these outcomes.

Source anchors: [body decisions](../../../src/compiler/module_bodies.panack),
[effect decisions](../../../src/compiler/module_effects.panack),
[bound-to-emitted conversion](../../../src/compiler/module_emission.panack),
[instruction emission](../../../src/compiler/emitter.panack),
[source-map queries](../../../src/compiler/source_maps.panack) and
[CLI dispatch](../../../src/compiler/driver.panack).

Existing regression sources inspected are
[`module_bindings.panack`](../../../tests/runner/compiler_suites/module_bindings.panack),
[`cli_explanations`](../../../tests/functional/cases/cli_explanations/main.panack),
[`instruction_sources.panack`](../../../tests/runner/compiler_suites/instruction_sources.panack)
and [source-map CLI cases](../../../tests/runner/compiler_source_maps_cli.panack).
Instruction-source cases explicitly assert the current missing while/for/return
origins; those expectations must change with E01–E03. Existing map invariants
check ordering, identity and unchanged executable bytes, but do not require one
truthful origin for every instruction. Add that completeness assertion rather
than treating existing success as evidence that every instruction is covered.
The older `compiler_explanations_unit.panack` covers the legacy checker: its
passing cases alone cannot establish module-checker completeness.

### Final reconciliation and acceptance obligations

The three remaining source traces are resolved:

1. **Proof and partial-body retention.** Direct arguments, annotated initializers,
   assignments and function returns call `body_refine`; that path retains
   conversion evidence and its original children. Specialized calls commonly
   call `body_require_value` without refinement, so C07 covers their missing
   proof causes. `body_solve` preserves existing records and substitutes several
   recorded types while retaining earlier outcomes; C03 must keep the original
   decision and later solution distinguishable. C09 is concentrated in early
   call failures, including a checked receiver whose owned-method selection
   fails. The resolution failure itself remains outside scope; earlier body
   decisions must not disappear with it.
2. **Effect reachability.** `check_module_signatures` assigns an effect to every
   valid function and synthetic intrinsic signature. The loader only publishes
   effect evidence when signature/guard validation succeeds. A missing direct
   function effect is therefore an incomplete/internal signature case, not an
   extra ordinary user outcome. Invalid bodies skip the effect pass; deferred
   bodies can retain evidence. The existing unresolved-indirect-effect unit test
   constructs a deferred body explicitly: keep that internal coverage without
   claiming its exact malformed state is a runnable source example.
3. **Origin boundaries.** Parser `Statement` and `Block` do not carry complete
   construct spans. Binding/assignment bound spans currently follow the value;
   loops can inherit the enclosing span. Slice 3 must carry honest construct
   locations from parsing through binding/module emission, not relabel those
   inherited spans as exact statement ranges. Bare internal ASTs have no source:
   record an explicit generated/internal origin and reason, never a guessed
   location. Defensive `OwnedCallExpr`/`DotCallExpr` traps in the low-level
   emitter are internal bypass cases: the checked frontend resolves those forms
   before emission. Preserve defensive coverage without presenting those traps
   as the normal method-lowering result.

Additional anchors: [signature construction](../../../src/compiler/module_signatures.panack),
[loader gates](../../../src/compiler/loader.panack),
[statement/block representation](../../../src/compiler/parser.panack) and
[bound facts](../../../src/compiler/checker.panack).

For every implementation packet, accepted/rejected/unavailable means the actual
checker outcome. Known arity comparisons and match uniqueness have two outcomes;
no synthetic unavailable outcome is needed. Type compatibility, proof and
inference can lack required facts. A check skipped after another failure is
reported as not performed with its dependency, not as a rejection or success.
No source-level rejected outcome is required for an instruction that is only
emitted after successful checking; failed compilation instead has no validated
artifact. Internal-only cases remain labelled as such.

| Gaps | Existing regression evidence to preserve | Missing acceptance to add in the owning slice |
| --- | --- | --- |
| C01, C04, C10 | Module-binding requirement/return/type-at tests and CLI cases already cover Void storage, operators, indexing, fields, match, joins and interpolation. | Rule-specific reasons, generic-reference rejection, genuine deferral causes, full readable transcripts; no empty evidence family presented as a failed check. |
| C02, C08 | Typed fs/streaming and native TCP/HTTP transport fixtures; `network_core_body_tests` distinguishes transport argument results. | Every argument-bearing typed-helper name and each of the five HTTP-state helper families: performed count/shape/argument checks, accepted/rejected/unresolved outcomes and retention off. No network request is needed to check these compiler decisions. |
| C03 | Inferred locals, explicit type counts, repeated-generic joins and owned generic methods already have checking/output tests. | First inference source, equality/collection/empty-constructor constraints, nested substitution, conflicting/recursive constraints, deferred dependencies and final solutions. Assert the source of each constraint, not merely the final type. |
| C05, C06 | `namespace_effect_tests`, callback contract/receiver tests, `owned_callback_tests` and CLI partial-function cases. | Individual performed callback requirements and skipped successors; local-resource effect permission and its call dependencies; body/global gates with reasons. |
| C07 | Branch conversion evidence and `owned_mutation_fact_tests` already check fresh/unrelated facts and reject stale facts. | Upper/lower source origins, specialized-call proof retention, assignment invalidation cause and true/false branch facts. Assert truthful insufficient-proof output rather than claiming runtime failure. |
| C09 | Existing invalid/deferred body and valid-sibling recovery cases. | A previously checked argument/receiver remains explainable after its parent call fails; binding and return wrappers preserve it; final solving does not rewrite history into a different performed decision. |
| E01–E03, E07 | `instruction_sources` fixtures explicitly assert current missing stores, loops and returns; identity/order/byte-parity checks already exist. | Replace those missing-origin expectations, include discarded expression statements and empty/nested blocks, and require an origin for every emitted instruction. Include an internal generated-origin case with no invented file/range. |
| E05, E08 attribution | Source-map CLI tests already exercise stale, malformed, mismatched, missing and forged pairs; imported generic selector fixtures verify identity. | Distinct validation failure vs valid no-result, all origins for shared output, negative completeness test, and unchanged bytecode with retention on/off. Keep bounded local replay validation. |
| E04, E06, E08 queries | Existing exact short-circuit, map/reduce, match, await, interpolation and imported-generic mapping fixtures. | Source-to-instruction results, actual transformation reasons, shared generic body vs call sites, method target identity, meaningful declarations with no standalone instructions and unused core pruning. No claims of specialization or general dead-code elimination. |

Existing tests were inspected, not rerun. Rows describing missing acceptance are
implementation obligations, not passing tests. This finishes P1/P2's mapping of
source decisions, retained evidence, output and regression gaps; it does not
replace slice 1.3/2.3/3.3/4.3's executed completeness checks. A later discovered
omission within these agreed families is a defect to fix, not permission to
narrow the completion target or silently add a new programme.

### P3 implementation packets and output review

The five slices stay unchanged. The following packets are a delivery order
within them, not eight new slices or eight new weighted tasks:

| Packet | Concrete result | Slice ownership / gap IDs |
| --- | --- | --- |
| A | Complete ordinary/special-call requirements and preserve previously checked evidence through failures. | Slice 1: C01, C02, C08, C09, C10. |
| B | Retain where inference constraints came from, how they combine and why they resolve, fail or defer. | Slice 1: C03. |
| C | Retain proof fact origins and mutation invalidation without changing what the compiler can prove. | Slice 1: C07. |
| D | Retain individual callback requirements and local-resource effect permissions. | Slice 1: C05, C06. |
| E | Render all retained decisions with specific reasons, source locations and compact complete output. | Slice 2: C04 and output for A–D; no second checking implementation. |
| F | Complete instruction origins through parsing/emission; distinguish invalid artifacts and support truthful multiple/generated origins. | Slice 3: E01–E03, E05, E07 and attribution part of E08. |
| G | Explain transformations, reverse mappings, shared output and no-output cases through noninteractive queries. | Slice 4: E04, E06 and query part of E08. |
| H | Reconcile GI#435 additions, installed examples, documentation and comparable retention costs. | Slice 5, after GI#435 and the other alpha.15 compiler changes. |

Packets A–D can feed E incrementally. Packet F precedes G. Add regressions while
implementing each packet; H does not defer their testing. The mechanism changes
are bounded to retained decision dependencies, construct origins and query/map
representation. There is no new inference/proof engine, compiler/stdlib
reorganisation. The earlier exclusion of an interactive terminal application is
superseded by the owner decision below.

**Historical planning estimate: roughly 8–12 PRs**, replacing the unsupported
five-to-seven estimate. This estimate predates the interactive presentation
decision and must be reassessed after prototype review. Eight coherent delivery
packets were identified; the
inference and source-origin changes may need more than one reviewable PR. This
is an estimate, not a completion condition or a reason to create tiny PRs. The
five slice exit criteria determine completion. Release weights are unchanged.

**Output review — checker presentation and three example sections approved.**
Owner agreement: use `expression => Type` and spell out locations as “line N,
column M”. Bare `N:M` positions were mistaken for ratios and must not appear
in these human explanations, including headings and supporting references.
The owner subsequently approved the rejected-subtraction, generic-inference
and unavailable-return explanation sections below. Apply that format and tone
consistently; the source-mapping CLI extension remains proposed.
Keep the current
`panack explain SOURCE --function NAME` entry point. Group by source construct,
lead with the result and attach supporting facts to that result. Combine closely
related decisions in a paragraph without omitting their outcomes. Omit empty
family headings. Do not print “unavailable” simply because a function has no
calls, subtraction or generic inference. Keep the selected function's status
separate from other project failures. These earlier text candidates do not establish acceptance of the interaction
design; the following decision supersedes the noninteractive-only assumption.

First full successful explanation candidate, using the existing bounded
subtraction example from the explanation guide. This is a proposed transcript,
not captured output. `remaining.panack` contains:

```panackelty
pure remaining(n: Nat): Nat {
  if n >= 2 {
    n - 2
  } else {
    0
  }
}
main(): Void {}
```

Command: `panack explain remaining.panack --function remaining`.
Proposed complete stdout, empty stderr and exit status 0:

```text
remaining: accepted
Source: project/remaining.panack, line 1, column 1

n >= 2 => Bool
  Line 2, column 6.
  n has the declared type Nat; 2 is Nat. These numeric values can be compared.
  The result is Bool, as required by if.

n - 2 => Nat
  Line 3, column 5.
  Both operands are Nat. The condition n >= 2 on line 2, column 6 guarantees
  that n is at least 2 in this branch. Subtracting 2 therefore cannot produce
  a value below 0.

0 => Nat
  Line 5, column 5.
  The other branch produces the Nat literal 0.

if … else => Nat
  Line 2, column 3.
  Both branches produce Nat, so their combined result is Nat.
  This matches remaining's declared return type Nat on line 1, column 1.

Effects: no call or await boundaries in this pure function.
```

Use the approved presentation when completing the full transcripts for
rejected subtraction, generic inference, an HTTP/host requirement failure,
partially checked function and genuinely unavailable decision. Each must show
all performed relevant decisions and distinguish local acceptance from project
failure. Existing ordinary diagnostics remain unchanged; failure transcripts
must include their stderr and exit status as well as explanation stdout.

### Further explanation examples — approved wording

The owner approved these three individual explanation sections on 28 October
2026. They are not complete stdout/stderr transcripts.
The source fixtures below establish their locations; no captured execution or
new implementation is claimed. Failure diagnostics and other performed decisions
must still appear in the eventual full transcript.

**Rejected subtraction.** Source:

```panackelty
pure remaining(n: Nat): Nat {
  n - 2
}
main(): Void {}
```

```text
n - 2 => Nat — rejected
  Line 2, column 3.
  Both operands are Nat, but Nat includes 0 and 1. Subtracting 2 from either
  would produce a negative value, which Nat cannot represent.
  Panack cannot prove that n is at least 2 here.

  Check that n >= 2 before subtracting, or use Int if negative results are valid.
```

Here Nat is the arithmetic result type selected by the type rule, not a claim
that the expression passed its separate proof obligation. The rejected status
must remain on the heading.

**Generic inference.** Source:

```panackelty
pure identity[T](value: T): T { value }
pure greeting(): Str {
  identity("hello")
}
main(): Void {}
```

```text
identity("hello") => Str
  Line 3, column 3.
  identity expects one argument of type T and returns the same type T.
  "hello" has type Str, so Panack infers T as Str for this call.
  The argument matches Str, and the call therefore produces Str.
  This matches greeting's declared return type Str.
```

**A check that could not finish.** Source:

```panackelty
pure answer(): Nat {
  missing
}
main(): Void {}
```

```text
missing => type not determined
  Line 2, column 3.
  No value named missing is defined here, so Panack cannot determine its type.
  Panack therefore cannot check whether this result matches answer's
  declared return type Nat.
```

The unresolved name remains an ordinary diagnostic, not a newly selected
name-resolution explanation feature. The in-scope explanation identifies the
unavailable return-type check and the prior failure that prevented it.

### Complete wrappers for the approved sections

Keep the approved wording above. The complete rejected-subtraction transcript
for `panack explain /examples/remaining.panack --function remaining` is:

```text
remaining: rejected
Source: project/remaining.panack, line 1, column 1.

n - 2 => Nat — rejected
  Line 2, column 3.
  Both operands are Nat, but Nat includes 0 and 1. Subtracting 2 from either
  would produce a negative value, which Nat cannot represent.
  Panack cannot prove that n is at least 2 here.

  Check that n >= 2 before subtracting, or use Int if negative results are valid.

Return type: compatible
  The arithmetic result type Nat matches remaining's declared return type.
  The subtraction is still rejected because its safety requirement was not proved.

Effects: not checked because remaining's body failed type checking.
```

Stderr, with the existing diagnostic wording; exit status 1:

```text
error: /examples/remaining.panack:2:3: Nat subtraction may underflow; prove the left side is large enough or use Int
  2 |   n - 2
    |   ^
```

For `panack explain /examples/generic.panack --function greeting`, complete
stdout is below; stderr is empty and exit status is 0:

```text
greeting: accepted
Source: project/generic.panack, line 2, column 1.

identity("hello") => Str
  Line 3, column 3.
  identity expects one argument of type T and returns the same type T.
  One argument was supplied.
  "hello" has type Str, so Panack infers T as Str for this call.
  The argument matches Str, and the call therefore produces Str.
  This matches greeting's declared return type Str.

Effects: identity is pure, so this call is permitted inside pure greeting.
```

The full unavailable-return case is represented by the partial-check transcript
below: remove its earlier binding and move the missing expression to line 2 for
the simpler approved fixture. No invented result/return compatibility check may
be added when the result type is absent. These composed transcripts remain
specification candidates until exercised against the implementation.

### Complete host and partial-check candidates

The following full explanation candidates apply the approved convention. They
are specification examples, not output from an implemented renderer. Ordinary
failure diagnostics keep their existing stderr format, including its path/line/
column notation. The new plain-language position convention applies to the
explanations; this programme does not rewrite the ordinary diagnostic formatter.

**Host argument failure.** File `/examples/host-call.panack`:

```panackelty
import stdlib/host as host
main(): Void {
  host.exit("done")
}
```

Command: `panack explain /examples/host-call.panack --function main`.
Proposed complete stdout:

```text
main: rejected
Source: project/host-call.panack, line 2, column 1.

host.exit("done") => Void — rejected
  Line 3, column 3.
  exit expects one argument, and one was supplied.
  The status argument must be Nat, but "done" is Str.
  Panack rejected this argument because Str is not compatible with Nat.

Return type: compatible
  The call's declared result is Void, which matches main's declared return type.
  This type match does not make the rejected call valid.

Effects: not checked because main's body failed type checking.
```

Existing diagnostic stderr contract (exit status 1):

```text
error: /examples/host-call.panack:3:13: argument 1 to exit is Str, expected Nat
  3 |   host.exit("done")
    |             ^
```

This is a public stdlib call, not an execution of the host operation. Evidence
for the underlying intrinsic's own body remains part of C02 and uses its actual
stdlib source identity when that function is selected.

**Preserve useful results after a failure.** File `/examples/partial.panack`:

```panackelty
pure answer(): Nat {
  number = 1
  missing
}
main(): Void {}
```

Command: `panack explain /examples/partial.panack --function answer`.
Proposed complete stdout:

```text
answer: rejected; some checks completed
Source: project/partial.panack, line 1, column 1.

number => Nat
  Line 2, column 3.
  The initializer 1 has type Nat, so Panack infers Nat for number.
  Nat is a value type and can be stored in this binding.

missing => type not determined
  Line 3, column 3.
  No value named missing is defined here, so Panack cannot determine its type.
  Panack therefore cannot check whether this result matches answer's
  declared return type Nat.

Effects: not checked because answer's body failed type checking.
```

Existing diagnostic stderr contract (exit status 1):

```text
error: /examples/partial.panack:3:3: unknown local value missing
  3 |   missing
    |   ^
```

Unlike the failed parent-call cases in C09, a block already preserves its earlier
binding. Reuse that evidence. The location for the binding must identify the
binding construct, not silently reuse the initializer's position. If exact
binding spans depend on packet F, deliver the source-origin dependency first or
show the accurately labelled initializer location until F is available.

### Realistic usability review

The owner rejected instruction-number-led provenance examples: users cannot
relate those numbers to their code. Those candidates are superseded by the
[source-led realistic review](../../COMPILER_UNDERSTANDING_REALISTIC_REVIEW.md),
using the real package-tool function `hash_archive_file`, an observed failing
edit and verified disassembly. Its complete overview, focused queries and full
detailed transcript are proposals for review, not approved output. Existing
short-example approval covers notation and wording, not overall usability.
The baseline prints 1,025 lines for this 26-line function. Proposed source
selection and detail controls must be agreed before renderer implementation;
a second real compiler function must then test whether the design generalises.

For slice 4, extend `locate` without adding a separate application. Proposed
mutually exclusive query forms, both retaining the current artifact/source/map
validation:

- Instruction query: existing `--function NAME --instruction PC`.
- Source query: `--source-file MODULE --source-at LINE COLUMN`, where MODULE
  is a portable identifier from the validated source snapshots, such as
  `project/library.panack`. This selects across emitted functions, so a generic
  declaration or import does not need an invented function selector. It must
  not read an arbitrary path supplied through the map or this selector.

Choose the smallest recorded source construct containing the point and show its
exact range; never choose a nearby construct when the point has no match.
Report all corresponding function/instruction ranges and recorded transformation
reasons. Shared generic bodies must be described as shared, not specialised.
Declarations contributing metadata can have no standalone instructions: explain
that relationship explicitly. A point with no recorded construct, a recorded
construct with no executable output, and failed artifact validation are distinct
results. Valid query/no-output results exit 0; invalid arguments or unvalidated
artifacts exit 1. Preserve the source-map replay trust model and bounded reads.
This is a proposed command extension, not a currently available capability.

Review the source-led realistic transcripts against this interface before
slice 4 implementation, including a second case covering shared-generic and
no-output outcomes. No extra declaration selector,
interactive navigation or runtime debugger is required.

P3 remains open for realistic usability agreement, including proposed overview,
source selection, full-detail and provenance-query transcripts. The three short
checker sections establish approved wording and notation only. No compiler
implementation has started and no completion credit is earned by preparation.

### Interactive presentation prototype — owner decision, 28 October 2026

The owner selected interactive terminal exploration now: an interim interface
must be useful in its own right, and line-coordinate navigation has not met that
bar. Prototype direct expression selection, expandable supporting facts, origin
links and back navigation using the real package function. Keep honest plain-text
output for redirected use. Replace the proposed coordinate-driven human workflow;
do not build two competing navigation interfaces. Source positions remain useful
context. The evidence inventories and five-slice dependencies remain necessary.

The local prototype is written in Panack, with a small prototype-only POSIX adapter
for key/resize events and terminal restoration. Panack uses existing read_line;
no compiler or VM changes are required. Navigation, layout, evidence fixtures and
history are Panack code. It demonstrates accepted/rejected fixtures, source view,
origin/back and paging. It does not process live compiler evidence. The owner has
not yet accepted the interaction design. Prototype code is isolated from the core
checkout; this PR records the decision, not a shipped application.

Before implementation, reassess presentation packets E/G, terminal capability
ownership, supported platforms, accessibility/plain output and the delivery
estimate. Validate a second real function after the first interaction review.
The previous 8–12 PR estimate is no longer current; no replacement figure or
completion credit is claimed. Preserve provisional release weights pending that
assessment. No website capability claims change.

Phone review is available through a prototype-only Panack HTTP event bridge and
touch controls. It replays a bounded per-tab control history through the same
Panack navigation/rendering code, rather than exposing a general shell or
duplicating the model in browser code. Error/origin/back output was byte-compared
with the terminal interface; browser interaction was checked at 390×844. This
enables review, not live compiler evidence or production session acceptance.

#### Live presentation milestone — 28 October 2026

The owner requested a working Mac demonstration for 29 October at 15:00,
including a phone reveal. A separate local demo now loads real retained compiler
evidence, includes final preparation diagnostics and reloads edited source.
`--serve` shares one running terminal session: navigation on the Mac or phone
changes both screens. Panack owns the model and UI; isolated tmux/ttyd sessions
supply terminal transport. The earlier fixture bridge remains available.

Accepted/rejected subtraction, generic-call evidence, effect rejection, final
preparation rejection, origin/back navigation and same-process source reload
pass local checks. Two-way native-terminal/browser control was verified at a
phone-sized viewport. Physical-phone and presentation-network rehearsal,
accessibility, broader usability and production integration remain outstanding.
No compiler/VM implementation changed, no additional evidence completeness is
claimed and no release completion credit is added. This bounded milestone does
not replace the five slices or their acceptance criteria. Website claims remain
unchanged because the demo is local and unreleased.

The first physical iPhone review subsequently showed repeated/overwritten browser
text while the tmux pane remained intact. The local bridge now has a candidate
canvas-renderer and settled-sizing correction. Physical-phone verification is
still pending; the desktop viewport check did not establish phone readiness.

### Delivery decision, 4 November 2026

The owner reviewed [the live subtraction explorer](https://github.com/sproates/panackelty/pull/654),
accepted its interaction pattern, and authorised end-to-end delivery of GI#180.
Open in source order, show the project result and enclosing source, allow direct
selection of the first unproved decision, and preserve selection on return from
a supporting origin. Earlier proposals for mandatory prototype or transcript
approval at each packet are superseded. Exercise the realistic package function
and a second compiler function in engineering acceptance.

P3 uses the C01–C10 and E01–E08 register above for the existing language. A–G
cover the current compiler. H integrates these results and rechecks new decisions
when GI#435 supplies them. A passing subtraction demonstration earns no
completeness credit for other decision families.
