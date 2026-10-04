# Panackelty roadmap

This roadmap tracks post-bootstrap language and engineering initiatives. The
completed compiler bootstrap and reproducibility guarantees
are recorded in [SELF_HOSTING.md](SELF_HOSTING.md).

Use the [roadmap decision process](docs/ROADMAP_PROCESS.md) for item states,
assessment criteria, priorities, document ownership and review. Implementation
items require their agreed scope, meaningful tests, affected documentation and
canonical validation; documentation and design items use their applicable checks.

## Roadmap identifiers

Refer to work as **RM#n: short name**, issues as **GI#n: short name**, and
pull requests as **PR#n: short name** in conversation and newly written records.
These are separate number spaces; an RM number is not a GitHub issue number.
Every independent work entry has an RM identity, including unissued ideas,
programme workstreams and milestones, website follow-ups and completed work.
Repeated summaries link to the same identity. Checklists describe their parent
item's scope; policy, review history and navigation sections are not work items.

The initial allocation contains **105 identities**. **Next available: RM#134.**
Allocate the next unused number above the largest allocated number, updating this
pointer in the same change. Never renumber or reuse IDs on reordering, completion,
deferral or retirement. Retain a linked tombstone for a removed or merged item.
Keep short names concise; an intentional rename preserves the ID and anchor.
Existing section headings and legacy anchors remain available; `#rm-N` is the
stable link for each item. No priority, scope or acceptance changes follow from
this numbering. See the [reference convention](docs/ROADMAP_PROCESS.md#work-identifiers-and-references).

## Priority review policy — 2026-10-03

At the user's request, the three-deliverable grooming rule and its completion
counter are retired. Review priorities on user request or when material evidence
changes assumptions, dependencies, scope, effort or value. Continue agreed work
without task-count gates; maintain delivery status and acceptance evidence.
Earlier review notes and outcome counts below are historical records only and
must not be used to schedule a review or block work. This supersedes count-based
wording in older issue histories as well. See the
[review process](docs/ROADMAP_PROCESS.md#review-and-completion).

## First non-alpha release — milestone definition

<a id="rm-126"></a>

**RM#126: First non-alpha release** ·
[GI#265: First non-alpha release](https://github.com/sproates/panackelty/issues/265).

User decision, 2026-10-04: this is an overarching release-readiness milestone
bringing programmes and workstreams together, not another programme. Version,
date and final scope remain undecided; further alpha releases may continue.
The following minimum outcomes are required, with detailed acceptance still to
be defined where stated. Open scope prevents a credible overall completion
percentage; do not double-count contributing programmes or call planning delivery.

| Required outcome | Existing work and remaining definition |
| --- | --- |
| Compiler work | Complete the agreed compiler-understanding scope in GI#180; confirm detailed release mapping and acceptance evidence. Its current pause remains in effect. |
| Namespaces, packages and HTTP | Complete GI#233, including migration, reusable packages, HTTP client/server, reproducibility and integrated acceptance. |
| Performance of a good standard | GI#263 supplies maintained measurements and explicit regression decisions. Agree representative compile/startup/run, resource and developer-feedback targets from repeatable evidence; no unmeasured performance claim. |
| Expanded standard library | Define practical application coverage, consistent APIs, tests and examples. This is broader than GI#235's namespace migration; detailed scope is open. |
| Expanded tooling and developer helpers | Define supported creation/edit/build/test/inspect/debug/maintenance workflows; reuse GI#139 editor work and relevant existing tool proposals. The full tooling/helper set remains open. |
| Code tidying | GI#132 / RM#62 component cleanup, maintainability and removal of obsolete transitional code with behavior protection. |
| Documentation | Coherent installation, learning path, language/library references, tooling guides, worked applications and migration notes; reuse GI#137/GI#138 and verify documented workflows. |
| Website outside the main repository | GI#178 / RM#14: source ownership/extraction and independent development/publication. The existing browser repository alone does not meet this requirement. |
| Faster local and collaboration workflow | GI#133/GI#106 cover developer feedback and build costs. Explicitly assess task-tracking/edit/review/publication latency too: elapsed time, active work versus waiting, repeated tool/review/approval overhead and resource/token use where observable. Administrative tracking overhead is a remaining scope gap, not a claimed existing acceptance criterion or measured cause. |
| Release readiness | Proposed supporting gates: reliable installation, packaging/upgrade, supported-platform evidence, compatibility/migration and support expectations; refine these with the owner. |
| Further requirements | Maintain an explicit open-scope list. Additional release requirements may be agreed; omission is not acceptance or a waiver. |

Release acceptance requires agreed bounded criteria for each area, evidence-linked
gate status and an independently verified clean-install walkthrough building a
useful multi-file application from public documentation. Resolve open scope before
choosing the release version; record limitations and accepted trade-offs. Individual
PR checks alone do not establish this milestone. Track gates as scope-to-define,
planned, in progress or accepted rather than inventing a total percentage.

Immediate workflow response: batch related bookkeeping into existing delivery or
planning PRs, use the established informational-document validation route, reuse
verified evidence and keep review proportionate to changed scope. Avoid repeated
status-only polling and duplicate approval rounds. This records the excessive
task-tracking delay as a concern; it does not claim the bottlenecks are measured or
fixed. No runtime/build measurements are required for this informational update.

Namespaces remains the principal feature initiative; this record starts no new
implementation or release. Ordinary website content updates remain deferred until
a supporting release, while repository extraction must be complete by this
milestone. Detailed decisions and future gate evidence belong in GI#265 and this
register, reusing existing child work rather than duplicating it.

## Standing performance engineering — ongoing

<a id="rm-123"></a>

**RM#123: Standing performance engineering** ·
[GI#263: Standing performance engineering](https://github.com/sproates/panackelty/issues/263).

**User decision, 2026-10-04:** performance is a continuing programme and a
cross-cutting requirement of substantive deliveries, not an occasional backlog
reminder. Maintain the scorecard below through deliveries, releases and planning
reports under the [performance review process](docs/ROADMAP_PROCESS.md#performance-impact-and-regression-decisions).
The standing programme has no lifetime completion percentage. The finite core
establishment tranche below has its own stable denominator.

The modules/packages/HTTP programme remains the principal feature initiative;
its checked declaration/signature identity slice merged in PR#275 as `0bcf092`;
body/effect/emission integration remains pending. Compiler understanding
remains paused. The selected build/validation baseline is established below;
no optimisation, runtime/resource benchmark execution, backend investigation or
automation starts with it. Remaining baseline scope requires separately selected
delivery. Website performance work remains
deferred, and ordinary website content updates wait for the next supporting
release; existing publication correctness defects retain their separate treatment.

### Continuous scorecard

| Area / existing work | Established target and current evidence | State, responsible role and next action |
| --- | --- | --- |
| Developer/build feedback · [RM#28: Incremental and modular builds](#rm-28) / [GI#106](https://github.com/sproates/panackelty/issues/106) | Controlled macOS arm64 baseline: clean full 133.98/134.42/136.10s against 120s; warm focused compiler 27.77/26.40/26.46s against 15s. All validation passes; both timing targets remain missed. [Exact inputs, counts and limitations](tests/VALIDATION_PROFILE.md#reproducible-build-baseline). | Open performance concern. Proposed disposition: accept the measurement baseline, retaining unchanged targets and coverage. Delivery owner retains GI#106 until a baseline maintainer accepts handover. Next: attribute the representative critical path, then compare a separately selected reuse candidate against this pinned matrix; review remediation scope before the next affected compiler delivery. No optimisation or causal regression claim is made. |
| Compiler/runtime/resources · [RM#56: Performance baselines](#rm-56) / [GI#141](https://github.com/sproates/panackelty/issues/141) | Prior profiles and source assessment exist; no maintained representative compile/run, startup, throughput/latency, memory or artifact-size baseline is accepted. No general numeric runtime budget is established. | Planned baseline; execution not started. Baseline delivery owner to be assigned when selected. Next: agree representative correctness-checked workloads, measurements, repeats and noise calibration before thresholds or optimisation. |
| Website delivery · [RM#8: Website CI follow-ups](#rm-8) / [GI#187](https://github.com/sproates/panackelty/issues/187) | Existing website validation 120s / merge-to-live 180s targets remain as historically scoped; timing acceptance is incomplete. They are not native/runtime targets. | Deferred by user decision; website maintainer retains the record. No new trials or scheduling. Revisit on the existing user-request/correctness/staleness triggers; release preparation reviews the deferred record without automatically restarting it. |

The controlled baseline now establishes applicable full/focused observations; both
targets remain missed. Earlier PR#262 timings remain historical evidence, not a
retroactive rejection of its already authorised merge or proof of regression's cause.
The next critical-path/candidate comparison must retain the pinned workload evidence. Future affected
deliveries must present a mitigation, justified trade-off or owned remediation
proposal for explicit acceptance with merge approval; copying a warning is not acceptance. See
[the recorded measurements and limits](tests/VALIDATION_PROFILE.md#namespace-raw-references-and-binding-resolution--2026-10-04).

### Finite core establishment tranche

Provisional scope-based estimates established on 2026-10-04 total 100%. The larger shares
cover reproducible harness/workload design, repeated samples and resource evidence;
policy and independent acceptance are smaller. This denominator includes only the
initial core establishment, not endless maintenance or the explicitly deferred
website work retained above. Existing child outcomes are counted once, not again
under the standing parent. Later scope/weight changes require an explained revision.

| Task | Scope and status | Weight | Completion / earned contribution |
| --- | --- | ---: | ---: |
| <a id="rm-124"></a>**RM#124: Performance governance** | Done through PR#264: ownership, mandatory proportionate delivery assessment, scorecard and explicit regression disposition. | 20% | 100% / 20 pp |
| [RM#28: Incremental and modular builds](#rm-28), baseline slice · GI#106 | Done on this baseline delivery's merge: bounded isolated harness, three repeats of clean/full, warm/focused and unrelated/direct/transitive edit observations, pinned inputs and explicit budget disposition. | 30% | 100% on merge / 30 pp on merge |
| [RM#56: Performance baselines](#rm-56) · GI#141 | Planned; execution not started. Correctness-checked compile/startup/run workloads, meaningful throughput/latency, memory and artifact-size evidence; calibrate noise before general thresholds. | 40% | 0% / 0 pp |
| <a id="rm-125"></a>**RM#125: Initial performance scorecard acceptance** | Planned; execution not started. Independent reproduction/review of both core baselines, qualified target applicability and initial scorecard acceptance. Requires RM#28/RM#56 baseline evidence. | 10% | 0% / 0 pp |
| **Total** | **Core establishment only** | **100%** | **50% on baseline merge; 20% accepted before it** |

RM#124 and RM#125 are scoped under GI#263; no duplicate implementation issues are
created. Governance earns 20 pp; this bounded build baseline adds 30 pp on merge.
This accepts measurement, not resolution of the measured performance concerns. Delivery owners maintain evidence
and unresolved actions until a named maintainer accepts handover; independent
reviewers assess measurements and dispositions, and the user selects priorities
and accepts trade-offs. The build baseline's owned remediation action is recorded
in the scorecard; runtime/resource measurements and combined initial acceptance
remain planned and require separately selected execution. No fixed-count review or scheduled automation is introduced.

Governance acceptance was recorded in PR#264. The build baseline adds repeated
correctness-checked evidence and independent review of scope, budget applicability
and the outstanding remediation disposition. Website impact: no public feature,
artifact, page or release pin changes; this is contributor tooling and policy.

## Ambition and adoption focus

Panackelty aims to be a broadly useful general-purpose language that developers
and AI coding agents choose to build real applications, with AI-assisted
development as an explicit adoption focus. The ambition is to become a better
choice for real development than Python or JavaScript/TypeScript on Node.js;
this is a goal to demonstrate, not a claim of current superiority.

Intended applications include scripts, daemons, web and application servers,
games, desktop GUIs (including the approachable experience of Tcl/Tk), browser
applications and mobile apps. This breadth guides architectural assessment;
it is not a claim of current support or a requirement to build every subsystem
before anyone can use the language. Interoperability with existing libraries,
runtimes and platform toolkits is a candidate enabling strategy.

Success means independent developers and coding agents can finish useful
applications, maintain and change them, and choose Panackelty again. Prioritise
reliability, useful compiler assistance, human-readable code, enjoyable tooling,
installation, deployment and discoverable libraries alongside language features.
Whether these strengths can overcome unfamiliarity and ecosystem gaps for AI
agents remains a hypothesis to test.

## Newcomer developer feedback — unscheduled backlog

Recorded on 2026-10-03; expanded on 2026-10-04. The remaining basic and advanced trials are **Idea / unscheduled**, outside the
modules/packages/HTTP programme. Recording them starts no setup, trial or coding.
Each trial requires explicit user selection as a dedicated task; completing setup
or another trial never triggers it automatically. No ad hoc implementation,
periodic runs or automatic fixes are authorised. Private participant configuration
stays outside repository records.

<a id="rm-113"></a>

**RM#113: Newcomer trial setup** ·
[GI#241: Newcomer trial setup](https://github.com/sproates/panackelty/issues/241).
Prepare a reusable fresh-context protocol, isolated workspace, pinned toolchain,
public documentation, bounded task/attempt budget and independent success checks.
Capture observable actions, diagnostics, recovery and outcomes; do not manufacture
mistakes or infer human beginner performance from an AI trial. Setup acceptance
is a ready-to-run protocol and task proposal, not an executed trial. Provisional
size: small–medium / one setup delivery. **Done — 2026-10-04:** reusable
private protocol and participant configuration saved and independently reviewed.
The protocol defines fresh-context public inputs, isolated workspace, explicit
selection, a default 30-minute / 12-failed-attempt budget, observable evidence,
independent outcome verification and limitations. A ready-to-run installation
and Hello World brief is prepared under [RM#116](#rm-116). Setup acceptance is
complete; no participant session, pilot or trial was started. Private configuration
remains outside the repository; trials require separate explicit selection.

<a id="rm-116"></a>

**RM#116: Installation and Hello World trial** ·
[GI#245: Installation and Hello World trial](https://github.com/sproates/panackelty/issues/245).
**Done — 2026-10-04:** explicitly selected installation trial completed after
[RM#113: Newcomer trial setup](#rm-113).
Suggested first dedicated trial before basic language feedback: start in a clean
supported native environment, follow public getting-started material, find and
install a published release, then write and run Hello World. Record the platform,
release and instructions used, commands, confusion, missing prerequisites,
diagnostics, recovery attempts and time to first successful run. Use a bounded
attempt budget without coaching; report any assistance separately. Independently
verify output and deliver reproducible findings with prioritised improvements,
including failure or budget exhaustion. The usability target is an unaided first
program using published instructions alone; a failed attempt remains useful trial
evidence. Small / one trial and report after setup; cross-platform coverage and
fix implementation are outside scope. Unaided Ubuntu x86_64 local installation
of published alpha.10 reached verified Hello World in about 2m43s, with zero
failed installation/check/run commands. Independent verification confirmed exact
output and archive/installed-file consistency. See the
[trial evidence and limitations](tests/VALIDATION_PROFILE.md#newcomer-installation-trial--2026-10-04).
Follow-up [RM#117: Platform-specific installation instructions](#rm-117) tracks
the unpack/download improvements; no blocking
product defect surfaced. This tested a fresh directory on a shared host starting
at GitHub, not a clean OS, marketing-site navigation or human beginner usability.
No fixes or further trials were started; each requires separate selection.

<a id="rm-117"></a>

**RM#117: Platform-specific installation instructions** ·
[GI#249: Platform-specific installation instructions](https://github.com/sproates/panackelty/issues/249).
**Done on merge of PR#251: Platform-specific installation instructions.** Selected follow-up
to [RM#116: Installation and Hello World trial](#rm-116) and
[GI#245: Installation and Hello World trial](https://github.com/sproates/panackelty/issues/245).
The README now gives separate Linux x86_64 and macOS arm64 alpha.10 download,
checksum, extraction and version blocks that stop on failures, followed by direct
local check/run/compile commands and optional home-directory PATH setup. The
published Linux archive passed the exact README commands on Ubuntu 24.04 x86_64;
all six injected download/checksum failures stopped before extraction.
[Installation verification](tests/VALIDATION_PROFILE.md#platform-installation-instructions--2026-10-03)
records evidence and limitations. The existing two-platform packaging CI now
executes the current README against real published assets. Both native hosted
platforms passed those exact commands in [the acceptance run](https://github.com/sproates/panackelty/actions/runs/37160525736);
independent source review also passed. Final-head checks and merge authorization
remain prerequisites to merging. No local macOS execution is claimed. This is an
installation documentation change, not another newcomer trial or a packaging,
installer or new-platform change. Website parity remains separately unscheduled
under [RM#118: Website installation command parity](#rm-118); release pins remain
unchanged and the original trial record is preserved.

<a id="rm-114"></a>

**RM#114: Basic language feedback** ·
[GI#242: Basic language feedback](https://github.com/sproates/panackelty/issues/242).
Depends on [RM#113: Newcomer trial setup](#rm-113). Use a bounded practical exercise
to assess types, bindings, functions, control flow, collections, records/enums,
pattern matching, errors and the basic purity distinction. Deliver reproducible
attempt evidence, independently checked programs and ranked usability findings.
Provisional size: small–medium / one assessment delivery; not started.

<a id="rm-115"></a>

**RM#115: Advanced language feedback** ·
[GI#243: Advanced language feedback](https://github.com/sproates/panackelty/issues/243).
Depends on [RM#113: Newcomer trial setup](#rm-113). Select a small application and
maintenance change combining supported generics, guarded types, higher-order
functions, effects, async and multi-file organisation. Include namespaces,
packages or HTTP only once supported by the selected toolchain. Deliver checked
outcomes and evidence of feature-interaction and recovery friction. Use fresh
context for independent trials; label retained learning as progression.
Provisional size: medium / one assessment delivery; not started.

Detailed acceptance and explicit-invocation rules live in the linked issues.
These trials complement [GI#133: Development workflow assessment](https://github.com/sproates/panackelty/issues/133),
which retains its broader installation-to-maintenance and human-walkthrough scope.
Findings may propose follow-up work; they do not authorise fixes. Website impact:
none from these backlog records; shipped capabilities are unchanged.

## Modules, packages and HTTP programme — 2026-10-03

<a id="rm-108"></a>

**RM#108: Modules, packages and HTTP programme** ·
[GI#233: Modules, packages and HTTP](https://github.com/sproates/panackelty/issues/233).

**P2 implementation in progress; accepted contribution 10 pp; 1 of 8 tasks accepted.**
P1 was completed by [PR#240: Module and package design](https://github.com/sproates/panackelty/pull/240),
merged on 2026-10-03 as `80cd50f`. The user selected P2 on 2026-10-04;
its first module/binding foundation merged in
[PR#261: Establish module binding identities](https://github.com/sproates/panackelty/pull/261)
as `a5cdb1a`, followed by qualified-reference syntax and cross-module binding
resolution in PR#262. [PR#275: Checked namespace signatures](https://github.com/sproates/panackelty/pull/275)
merged checked declaration/signature identities as `0bcf092`. P3–P8 remain planned and unstarted. P2 selection is separate from design acceptance.
The compiler-understanding programme remains paused; its scope is retained.

User decision, 2026-10-04: **P2 → P3 → P4 → language/local-package release,
then HTTP client work (P5)**. The first release checkpoint demonstrates complete
namespace and local-package use before adding HTTP. Its acceptance is the first
of [P8's three gates](#rm-112); it does not claim whole-programme completion.
P7 can proceed independently once P1/P3 are accepted, but is not a prerequisite
for this release or the client milestone. This changes sequencing and partitions
acceptance, without adding scope, changing programme weights or allocating a
version. It does not imply that the release must leave alpha.

The following milestone is **a working independently consumed HTTP client package**.
A clean native application imports its public client API, requests HTTP and HTTPS
resources by hostname, obtains typed status/headers/body and handles explicit
failures. HTTPS includes certificate and hostname verification. Deterministic local
HTTP/TLS fixtures prove successes, rejected certificates, timeout, malformed
responses and cleanup; a public-site smoke test is optional, not a CI dependency.
Local packages suffice: an HTTP server implementation and remote package retrieval
are not prerequisites for this milestone.

The desired shape is illustrative proposed syntax, not a shipped contract:

```panackelty
import http/client as http

async main(): Unit {
  response = await http.get("https://example.com")
  // Handle success or failure.
  ()
}
```

The [P1 design](docs/MODULE_PACKAGE_DESIGN.md) selects syntax, public types/errors,
ownership, transport/DNS/TLS prerequisites, compatibility and migration. HTTP framing is distinct from
today's finite EOF-framed TCP. Each implementation slice retains canonical tests,
bootstrap, docs, independent review and explicit merge approval.

| Stage | Task | State | Dependencies | Estimate |
| --- | --- | --- | --- | --- |
| P1 | [RM#109: Module and package design](#rm-109) · [GI#234: Module and package design](https://github.com/sproates/panackelty/issues/234) | Done | None | Medium / 1 design PR |
| P2 | [RM#41: Language namespaces](#rm-41) · [GI#198: Language namespaces](https://github.com/sproates/panackelty/issues/198) | In progress — checked declaration/signature identities | P1 | Large / 3–5 PRs |
| P3 | [RM#43: Local reusable packages](#rm-43) · [GI#199: Local reusable packages](https://github.com/sproates/panackelty/issues/199) | Planned; not started | P1; P2 boundaries | Large / 2–4 PRs |
| P4 | [RM#42: Standard library namespaces](#rm-42) · [GI#235: Standard library namespaces](https://github.com/sproates/panackelty/issues/235) | Planned; not started | P1–P3 as needed | Medium / 1–2 PRs |
| P5 | [RM#110: HTTP client package](#rm-110) · [GI#236: HTTP client package](https://github.com/sproates/panackelty/issues/236) | Planned; not started | P1–P3; transport/DNS/TLS | Large, uncertain / 4–7 PRs |
| P6 | [RM#111: HTTP server package](#rm-111) · [GI#237: HTTP server package](https://github.com/sproates/panackelty/issues/237) | Planned; not started | P1–P3; transport/lifecycle | Large / 2–4 PRs |
| P7 | [RM#44: Reproducible dependencies](#rm-44) · [GI#200: Reproducible dependencies](https://github.com/sproates/panackelty/issues/200) | Planned; not started | P1, P3 | Large, uncertain / 3–5 PRs |
| P8 | [RM#112: Package and HTTP acceptance](#rm-112) · [GI#238: Package and HTTP acceptance](https://github.com/sproates/panackelty/issues/238) | Planned; not started | Language/local-package gate P1–P4; client gate after P5; final all including P6/P7 | Medium / 1–2 PRs, provisional; reassess across three gates |

<a id="rm-109"></a>

**RM#109: Module and package design** — **Done**;
[GI#234: Module and package design](https://github.com/sproates/panackelty/issues/234).
Delivered [source-grounded design](docs/MODULE_PACKAGE_DESIGN.md): module identity,
private/public APIs, aliases/re-exports, explicit local-package manifests and root
confinement, two consuming applications, coordinated breaking migration, bounded HTTP
client/server contracts, DNS/TLS ownership prerequisites, acceptance matrix and
P2–P8 delivery estimates. Independent revision-pinned review and repository
validation are recorded in the delivery PR. No post-merge live acceptance is
needed for this design; no production syntax or package/HTTP capability is shipped.
The existing GI#198/GI#199 investigations feed this shared design; their tracked
production outcomes remain P2/P3 rather than being closed by a design document.

<a id="rm-110"></a>

**RM#110: HTTP client package** — P5 in GI#236: independently importable
client, HTTP/HTTPS request/response and failure handling under an explicit bounded
protocol/API contract. Native first; browser support is not implied.

<a id="rm-111"></a>

**RM#111: HTTP server package** — P6 in GI#237: importable handler/response
API, bounded protocol/resource handling, tested lifecycle and graceful shutdown.
Agree server TLS scope during design; first client acceptance need not wait.

<a id="rm-112"></a>

**RM#112: Package and HTTP acceptance** — P8 in GI#238 has three gates:

1. **Language/local-package release after P1–P4.** A clean installed toolchain
   builds and runs two independent applications consuming the same local library;
   relocated installations and package trees work. Namespace/stdlib migration,
   executed documentation, fresh bootstrap and the required native/browser
   baseline pass. Publish the supporting release through the normal reviewed
   release process. HTTP, server support and P7 remote/offline dependency
   reproduction are not prerequisites for this gate.
2. **HTTP-client acceptance after P5.** Verify the independently consumed client
   with local packages, including the bounded hostname HTTP/HTTPS, TLS rejection,
   failure and cleanup evidence above. P6/P7 do not block this gate.
3. **Final programme acceptance.** Verify all outcomes, including P6 server
   lifecycle/walkthroughs and P7 pinned remote restore plus verified offline
   reproduction, with clean/relocated workflows, executed documentation and
   supporting release availability. All eight outcomes must be accepted before
   closing the programme; neither earlier gate closes P8 as a whole.

The gates partition P8's existing scope and weight; they do not create three new
tasks or award completion merely for announcing a checkpoint. GI#166 typed service exchange and GI#162 preview-server
replacement remain separate followups, not silently completed by raw HTTP.

P2/P3/P4/P7 reuse RM#41/RM#43/RM#42/RM#44 respectively. This register supersedes
their earlier Idea/unscheduled and assessment-only status: P2 is In progress;
P3–P8 are Planned, not started. Their shared design is recorded in P1, with provisional
implementation estimates above and detailed slices in the design. No public
registry, separate compilation, web framework or new engine is required.

Provisional scope/effort baseline recorded on 2026-10-03 in
[GI#233: Modules, packages and HTTP](https://github.com/sproates/panackelty/issues/233):

| Stage / task | Programme weight | Task completion | Earned contribution |
| --- | ---: | ---: | ---: |
| P1 / [RM#109: Module and package design](#rm-109) | 10% | 100% | 10 pp |
| P2 / [RM#41: Language namespaces](#rm-41) | 20% | Partial; unquantified | Unquantified |
| P3 / [RM#43: Local reusable packages](#rm-43) | 15% | 0% | 0 pp |
| P4 / [RM#42: Standard library namespaces](#rm-42) | 5% | 0% | 0 pp |
| P5 / [RM#110: HTTP client package](#rm-110) | 20% | 0% | 0 pp |
| P6 / [RM#111: HTTP server package](#rm-111) | 15% | 0% | 0 pp |
| P7 / [RM#44: Reproducible dependencies](#rm-44) | 10% | 0% | 0 pp |
| P8 / [RM#112: Package and HTTP acceptance](#rm-112) | 5% | 0% | 0 pp |
| **Total** | **100%** | Partial implementation unquantified | **10 pp accepted; estimated total unquantified** |

P1 has earned its design-task credit through merged PR#240, supported by the deliverable above;
P2 is In progress: PR#261 merged its metadata/identity foundation and PR#262
merged raw qualified syntax, cross-module binding/re-export resolution and
diagnostics. PR#275 merged checked declaration/signature identities as `0bcf092`;
body/effect/emission identity integration and migration remain. These provide
partial implementation evidence without executable namespace acceptance. No stable sub-outcome
allocation exists within P2, so its partial completion and estimated contribution
remain unquantified rather than assigning credit by PR count. P3–P8 remain Planned.
The baseline weights and accepted subtotal are unchanged at 10 pp; the overall
estimated completion is therefore unquantified, with a known 10 pp accepted
contribution. Namespace execution remains unaccepted. Namespace/compiler
integration and HTTPS client correctness carry the largest shares; local packages and server lifecycle
follow, with smaller shares for design, reproducibility, migration and final
cross-task acceptance. These coarse estimates include each task's own tests/docs;
P8 covers the separate integration/release acceptance, without counting them twice.
P1 records bounded protocol scope; transport/backend feasibility remains a P5
prerequisite and may change its estimate. Keep this baseline stable until explicitly revised with its reason and effect on the total;
implementation sizes remain uncertain. Follow the shared
[programme tracking rules](docs/ROADMAP_PROCESS.md#programme-tracking).

Track each task as Planned, In progress, In review, Verification pending or
Accepted. On selection record owner, PR/revision, evidence, blockers and next action
in its issue and update this register at delivery. A design or experiment does not
complete a production task. Website impact: planning only; record supporting
release/adoption followups before feature promotion, and verify live claims before
closing such followups. P1 design acceptance is complete. The next action is
body/effect/emission identity integration following the merged checked-signature
delivery, then the remaining P2 acceptance and the selected P3/P4 sequence. P3–P8 have not started.
Website impact: no adoption update for this internal P2 slice; namespaces remain
unavailable for execution and version-pinned examples remain accurate. The later
namespace release checkpoint requires its own adoption follow-up.

### Namespace release checkpoint

The 2026-10-04 sequencing decision refines this checkpoint to the
[language/local-package release gate in P8](#rm-112): complete P2, P3 and P4,
then release before starting HTTP client work. A namespace-only internal slice
is insufficient; clean installed two-application local-package use, relocation,
coordinated stdlib/source migration and required native/browser baseline evidence
must pass. P7 remains independent after P1/P3 and is not a release prerequisite.

The 2026-10-03 decision originally selected coherent namespace migration as a
release checkpoint. The intervening [RM#128: Alpha.11 release](#rm-128), selected
on 2026-10-04, shipped already delivered features without waiting for namespaces.
The current checkpoint follows that release and retains the namespace migration
requirements; it makes local-package readiness and standard-library migration
explicit before HTTP. Record before/after import, visibility and name-resolution
migration notes and website adoption follow-ups, preserving truthful pinned claims.

Choose the version under the release policy when this gate is ready. No version
is allocated, no move beyond alpha is implied, and no future merge or publication
is approved by this planning record. Scope, weights and completion credit remain
unchanged; P2 is partial and the accepted programme contribution remains 10 pp.

## Programme pause and resumption checkpoint — 2026-10-03

**GI#180: Compiler understanding programme is temporarily paused at the user's request.**
Scope and acceptance criteria are retained; this is neither cancellation nor completion.
Resume only when the user selects programme work again; there is no automatic restart date.
This checkpoint supersedes earlier active-priority and next-action wording below.

Accepted foundations: RM#98 guard-fact repair (C0), RM#99 source attribution
experiment (U1), and RM#100 production source maps (U2).
U0 remains partial: initial investigation, RM#2 runtime retention experiment
(PR#227) and RM#106 semantic-impact experiment (PR#230) are delivered.
U3 remains partial: subtraction explanations (PR#219), RM#1 local effect
explanations (PR#225), and RM#107 per-function effect recovery (PR#231) are delivered.
Latest accepted main revision: `253ae966cebfd71848a976986eccbb70a34801ea`.
PR#231's merged tree matches the independently reviewed tree; hosted Check and
Pages passed before merge. No feature acceptance remains for that bounded slice.

All five capability workstreams remain open. U4–U7 production, U8 realistic
evaluation/release acceptance and U9 website demonstrations remain unfinished.
Supporting release and website adoption remain separately tracked, including
RM#104; this pause starts no release or website work.

On resumption, verify current main, open PRs and child-issue evidence first.
Compare a bounded U3 type/proof explanation, U0 declaration-effect dependency
research for U6, and the separately scoped U7 scalar provenance delivery.
U5 depends on sufficient retained checker evidence; U4 builds on accepted mapping.
Broader dependency/trust questions and representative performance budgets remain
explicit investigation work. Re-estimate and select one slice with the user;
none of these candidates is currently authorised for implementation.

## U3 per-function effect recovery — 2026-10-03

<a id="rm-107"></a>

**RM#107: Per-function effect recovery** — bounded outcome **Done on this PR's
merge**, following [RM#106: Semantic-impact experiment](#rm-106). The user selected
one medium feature PR: body type errors no longer suppress trustworthy local
effect explanations for separately type-valid functions. The checker retains
per-function validity and a conservative global gate for loading, resolution,
type errors in signatures and type declarations. Invalid functions explicitly remain
unavailable; no expression-level or transitive recovery is introduced.

Recovery is explanation-only to preserve ordinary acceptance, rejection, ordered
diagnostics and emitted artifacts. Local recovered violations appear in evidence;
original type diagnostics remain on stderr. Imported/generic source origins and
allowed versus whole-program-rejected status remain explicit. Unit and public-CLI
regressions cover unsafe callable/await/iterable inference, final return mismatch,
late global errors, diagnostic parity and fresh evidence after repair. The retained
semantic-impact experiment now positively checks the recovered caller boundary;
its original measurements remain labelled historical.

Acceptance: 266 focused unit assertions, 53 public-CLI assertions and all 343
functional cases pass. Clean canonical `make check` passed in **154s** (unit 116s,
functional 5s, bootstrap 19s), including fresh compiler/library fixed points and
package/quick-start checks. All 227 compiler-contract fixtures preserve exact
baseline diagnostics/status; all 43 accepted artifacts remain identical. The
updated semantic-impact experiment passes 25 assertions. The full 120s and unit
15s budgets remain exceeded; retain the existing
[GI#106: Validation performance](https://github.com/sproates/panackelty/issues/106)
reminder rather than relaxing them. See the
[validation profile](tests/VALIDATION_PROFILE.md#u3-per-function-effect-recovery--2026-10-03).

This completes only the bounded recovery feature; no separate issue exists.
[GI#134: Compiler explanations](https://github.com/sproates/panackelty/issues/134)
and [GI#180: Understandable-programming programme](https://github.com/sproates/panackelty/issues/180)
remain open. No post-merge feature acceptance is required. Supporting native
release and website adoption remain [RM#104: Explanation website adoption](#rm-104);
this PR starts no website implementation or promotion.

## U0 semantic-impact experiment — 2026-10-03

<a id="rm-106"></a>

**RM#106: Semantic-impact experiment** — bounded outcome **Done on this PR's
merge**, under [RM#97: Shared programme investigation](#rm-97) and
[GI#175: Semantic change prediction](https://github.com/sproates/panackelty/issues/175).
The user selected one research PR; no separate issue exists for this bounded
outcome. The [reproducible experiment and report](tests/experiments/semantic_impact/README.md)
predict guard and declared-effect consequences from actual baseline compiler
records before applying edits, then validate through changed checking and the
public CLI. It positively substantiates one caller-local Nat obligation and an
ordinary caller's permitted effect boundary. Nested guards, mutation and missing
effect evidence challenge unsupported independence claims.

The conditional two-edit effect cascade is verified; a single leaf edit does not
infer transitive effects or export return-bound guarantees. That negative finding
leaves GI#175's transitive acceptance unmet. The report measures metadata and
latency and recommends bounded declaration-effect dependencies before any public
prediction query. Production U6 is unstarted, broader GI#175/GI#180 remain open,
and further production scope requires separate selection. No post-merge acceptance
for this experiment. Website: no impact because no released behavior, command or
feature claim changes.

Acceptance evidence: 25 focused experiment assertions passed. Clean canonical
`make check` passed in 150s (unit 113s, functional 4s / 343 cases, bootstrap 19s),
including compiler/library fixed points and package/quick-start validation.
The 120s full and 15s unit budgets were exceeded; retain the existing
[GI#106: Validation performance](https://github.com/sproates/panackelty/issues/106)
reminder for these measured overruns rather than relaxing budgets. Independent
review found no blocking findings; its minor architecture wording correction
was included. Final informational completion text is checked with `make docs`.

## U3 effect explanation slice — 2026-10-03

<a id="rm-1"></a>

**RM#1: Local effect explanations**.

Selected by the user after performance PR #221 and backlog PR #223: extend the
existing explanation command with retained local purity/ordinary/async call and
await decisions, including callable types and imported/generic definitions.
Medium / 1–2 PRs, targeting one end-to-end delivery. This follows the agreed
return to programme #180; #134 remains open for broader type/proof explanations.
U0 retention and non-impact experiments remain required before dependent work.
No new release or website work is started. Owner: delivery agent.

**Bounded outcome Done on this PR's merge:** shared effect checking retains actual
local decisions without changing checking rules or user bytecode. Positive,
rejected and unavailable output covers callable types, imported/generic definitions,
nesting and source ranges. Local boundary status is separate from whole-program
validity; no inferred transitive/runtime effects are claimed. Independent
review found no actionable findings. All 196 focused explanation
assertions, 36 CLI assertions and 343 functional cases passed. Canonical clean
`make check` passed in **150s** (unit 112s, functional 5s, bootstrap 19s), with
fresh compiler/library fixed points and package checks. Exact baseline parity
holds for 227 compiler-contract fixtures (43 accepted artifacts, 184 rejected).

The [validation profile](tests/VALIDATION_PROFILE.md#u3-local-effect-explanations--2026-10-03)
records same-input timing and limitations: two-run compiler averages 5.977s before /
6.127s after (+2.5%); small-program compilation and query medians differ by under
2ms. This does not establish zero overhead. The 120s full and 15s incremental/unit
budgets remain explicit #106 reminders; broader U0 retention experiments remain.

This is an intermediate programme slice, not completion of #134 or #180. Broader
type/proof explanations remain. No post-merge acceptance remains for this local
query; release/website adoption is a separate follow-up. Next principal work
returns to programme prioritisation; no U4–U7 implementation is implied.

## U0 bounded runtime-retention experiment — 2026-10-03

<a id="rm-2"></a>

**RM#2: Runtime retention experiment**.

Selected by the user after the U3 local-effect delivery: investigate one actual
arithmetic/function-call value derivation, repeated call/loop identity, bounded
retention, explicit missing/evicted/unsupported evidence, CPU/memory costs and
sensitive-value policy. Medium / one experiment PR under
[#172](https://github.com/sproates/panackelty/issues/172) and
[#180](https://github.com/sproates/panackelty/issues/180). This completed experiment
preceded [RM#106: Semantic-impact experiment](#rm-106); broader production U7 is
not selected.

**Bounded outcome Done on this PR's merge:** the test-only observer follows the
actual VM's `multiply(6, 7)` return to its arithmetic/operand/call origins and
validates the actual PC against production source attribution. Repeated calls,
loops and recursion have distinct occurrence IDs. Ring and prefix policies
retain fixed memory and report evicted/discarded ancestry; metadata limits and
unsupported collection/host/async/indirect boundaries give explicit unavailable
roots without inventing a derivation. Payloads are opt-in fixed-width Nat/Bool;
metadata itself is not confidential. Chronological branch context is explicitly
not a proven control-dependence graph.

[Experiment, acceptance and measurements](tests/experiments/runtime_provenance/README.md):
570,010 events in the scalar loop, 238,648 bytes total retained metadata at
1,024 slots (including all shadow state), approximately 2.21× bulk CPU and
1.97× single-step CPU for ring observation in the recorded five-run sample.
A retained final root can still have evicted ancestry. Equal observed process
RSS does not establish zero allocation overhead. Recommend a separately scoped
U7 scalar slice, large / 2–3 PRs, with disabled-by-default hooks, validated
source/session identity, explicit queries and gaps, privacy controls and new
representative performance acceptance. Do not ship the experiment as a feature.

This finishes only the bounded U0 retention question. U7/#172 and all five
programme workstreams remain open; other U0 investigations, including positive
non-impact evidence, remain. No post-merge execution acceptance is required for
this experiment. Acceptance: 65 focused real-VM/CLI assertions, native retention
unit checks, and clean canonical `make check` pass (151s total, unit 114s,
functional 4s, all 343 functional cases, bootstrap and package gates). The
120s full and 15s unit budgets remain explicit #106 reminders. Independent
review found and verified a fix for unsafe observer PC inspection on a verified
fallthrough path; scalar and unsupported-frontier regressions now pass. No
remaining correctness findings. No release or website feature claim changes.

## Standard I/O, streaming and logging

<a id="rm-3"></a>

**RM#3: Streams and logging** · [GI#222: Streams and logging](https://github.com/sproates/panackelty/issues/222).

**Idea; awaiting planning, implementation unscheduled:**
[#222](https://github.com/sproates/panackelty/issues/222), requested 2026-10-03.
Existing `read_line`, `print` and `eprint` cover basic stdin/stdout/stderr;
whole-file text/binary I/O and subprocess capture also exist. Assess the gaps
and discoverability before proposing replacements. Plan coherent line/chunk and
text/byte stream APIs, EOF/error contracts, buffering/flush, pipelines and host
support, plus logging levels, filtering, sinks and structured context. Preserve
existing APIs and effect checks; keep machine-readable stdout separate from logs.
The first outcome is a capability matrix, examples, API/lifecycle recommendation
and prioritised implementation slices, not a delivered logging framework.
Medium / provisionally 1–2 investigation/design PRs; implementation estimated
after assessment. Coordinate with subprocess POC #168 and build-tool POC #169.
This records future planning without changing programme #180 priority. No release or website impact from this record.

## Compiler options and optimisation modes

<a id="rm-4"></a>

**RM#4: Compiler options** · [GI#220: Compiler options](https://github.com/sproates/panackelty/issues/220).

**Idea; unscheduled:** [#220](https://github.com/sproates/panackelty/issues/220),
requested during the 2026-10-02 performance discussion. Revisit compile-time speed,
generated-program optimisation and optional diagnostic/explanation support as
separate goals. Compare coherent profiles and orthogonal flags, with measured
trade-offs, explicit defaults, compatibility/cache/source-attribution rules and
preserved type/proof checking, exact semantics and runtime safety. Ordinary builds
already omit explanation evidence and instruction-source mappings; do not assume
another flag removes current costs. An `-O2`-style interface is a proposal, not a
selected design. Medium / 1–2 investigation PRs for an options matrix, reproducible
evidence and a separately estimated implementation recommendation. Relates to
#106, #141, #134 and #173; does not replace current performance work or expand #180.

## Performance priority review — 2026-10-02

Historical bounded investigation; ongoing obligations now belong to
[RM#123: Standing performance engineering](#rm-123).

**Agreed with the user after PR #219:** take a bounded validation-performance
investigation under [#106](https://github.com/sproates/panackelty/issues/106)
before further compiler features. Determine whether recent compiler changes
caused the observed 378s full check (290s unit, 71s bootstrap), separating compiler
implementation cost, growing compiler/test inputs, test execution and repeated
build work. Compare historical seeds on identical inputs with the same VM and
host; profile clean and incremental validation before choosing an optimisation.
Preserve every test, proof rule, diagnostic and bootstrap/release gate. No claim
of reaching the 120s budget or delivering separate compilation is made in advance.

Selected scope: performance evidence and one justified bounded improvement.
Delivered in one PR; acceptance takes effect on merge. Next: return to programme #180 and
reassess the next U3 type/effect query using the measured cost. U0 runtime-retention
and positive non-impact experiments remain required before their dependent work.
The file-discovery POC #167 and general benchmark suite #141 remain unscheduled.
Website CI #187 stays deferred; this task does not reopen it. The #182 corrective
release/browser adoption obligation remains pending and must be revisited on
publication correctness evidence or release preparation; no release is started.

This review considered U1 (#208), U2 (#209/#214/#218 counted once), and U3's first
query (#219, `f379c50`) as three accepted outcomes. The new baseline is **0 of 3
accepted outcomes** since this review; grooming itself does not add an outcome.
The performance delivery agent owns measurements, validation, the PR and tracker
handover. The bounded implementation replaces repeated interpreted declaration
scans with per-kind indexes, preserving declaration order and lookup semantics.
The [validation profile](tests/VALIDATION_PROFILE.md#compiler-lookup-and-validation-cost--2026-10-02)
records fixed-input history, clean/warm phase costs, paired results and limits.
**Bounded outcome Done on this PR's merge:** indexed lookup, regression coverage,
seed reproduction and independent correctness review are complete. Final clean
`make check` passed in **186s**, versus the same-host 366s baseline: unit 138s
(previously 274s), functional 6s, bootstrap 23s (previously 68s). The 120s full
and 15s incremental/unit budgets remain open #106 concerns; warm compiler checks
measured 33s before / 32s after, so no material warm-path improvement is claimed.
No tests were removed.
Broader #106 cache/design work remains open. No website correction or release promotion is required for this internal
speedup; emitted user-program bytes and advertised features remain unchanged.
The earlier #182 corrective release/adoption obligation is still separate. Further PR merges still require explicit user approval.

## Browser/WASM boundary investigation — 2026-10-01

<a id="rm-5"></a>

**RM#5: Browser ownership** · [GI#151: Browser ownership](https://github.com/sproates/panackelty/issues/151).

Issue #151 records new evidence that browser-specific CI is imposing material
feedback latency on unrelated core changes. The bounded investigation recommends
a separate browser/playground repository **after one explicit core dependency
boundary is prepared**, rather than copying the current source-tree coupling
across repositories. See [the investigation](docs/BROWSER_WASM_BOUNDARY.md).

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
were 213s and 182s. See [phase timings and limitations](tests/VALIDATION_PROFILE.md#browser-ownership-and-provisioning--2026-10-01).
Core hosted and post-merge acceptance are complete as recorded above.

## Current status and grooming, 2026-10-01

### Completed: playground footer layout

<a id="rm-7"></a>

**RM#7: Playground footer** · [GI#163: Playground footer](https://github.com/sproates/panackelty/issues/163).

**Done:** [#163](https://github.com/sproates/panackelty/issues/163), effective when
this completion record merges. Browser PR #6 and core PR #177 are merged; the
user accepted the homepage-style resource list preview. Browser v0.1.1 is pinned.
Production [Pages run 36875191639](https://github.com/sproates/panackelty/actions/runs/36875191639)
passed build, deployment and verification at core `2952dd2`. On 2026-10-01 the
published site was independently compared with that run's downloaded artifact
using `scripts/check_pages.cjs`: all playground bytes, Wasm MIME type and
website/coverage provenance matched. The published release pin is v0.1.1 and
all six resource rows retain their destinations. Browser regression/preview
acceptance and live publication are complete; no acceptance remains.

<a id="next-fast-website-ci-and-prepared-browser-test-environments"></a>

### Accepted website delivery; remaining CI follow-ups deferred

<a id="rm-8"></a>

**RM#8: Website CI follow-ups** · [GI#187: Website CI follow-ups](https://github.com/sproates/panackelty/issues/187).

**User decision, 2026-10-02:** the delivered website and separate browsable
coverage site are good enough for now. Accept the delivered outcome and shelve
further automatic-scheduling investigation, core-only production acceptance
trials and cold/warm performance work. #187 remains open as a deferred follow-up,
not an active blocker. No claim is made that the original 120s/180s targets or
automatic refresh acceptance have been met. No further trial or monitoring is
scheduled. Revisit on user request, a publication correctness failure, or report
staleness becoming a practical problem.

Programme #180 is no longer blocked by #187. The accepted website outcome was
the third deliverable since the previous review baseline. The proposed
[2026-10-02 priority review](#programme-priority-review-2026-10-02) records the
return to compiler work, effective when its documentation PR is approved and
merged. The deferral itself did not start compiler implementation.

The following history records the earlier scope, measurements and acceptance
requirements; its active-priority and ledger statements are historical and are
superseded by this decision.

**In progress; high priority.** Work record:
[#187](https://github.com/sproates/panackelty/issues/187). On 2026-10-01 the user
selected this maintenance item before the next substantial programme #180
feature. Repeated website setup delays affect every relevant edit and have also
held up compiler delivery. Finish existing delivery and correctness obligations;
then take this bounded improvement before resuming substantial programme features.
On 2026-10-01 the user explicitly selected #187 before any further programme
work. Programme #180 is temporarily paused at its existing scope and progress;
resume with the mapping foundation after this maintenance outcome. The grooming
ledger remains 2 of 3; completing #187 will make the next review due.

Agreed budgets: routine website validation **120 seconds** and merge-to-live
**180 seconds**, for cold and warm runs, excluding queue time reported separately.
Implementation uses the digest-pinned official Playwright 1.63.0 environment and
reuses browser-certified website artifacts for publication. Coverage now has an
independent Pages host, as recorded below. Hosted cold/warm
measurements and post-merge live verification remain acceptance requirements.
PR #189 is merged and live verification passed, but its first production sample
took 147s validation and 208s merge-to-live wall time. Coverage refresh also
repeated assembly/browser work despite matching inputs. The user selected repair
of artifact reuse and redundant publication next, after Node 24 action maintenance
(PR #190). The repair uses per-run artifact discovery and exact live provenance
comparison; hosted reuse, duplicate-skip and changed-coverage publication remain
acceptance requirements owned by the delivery agent. #187 stays open; neither
this repair nor the action upgrade establishes the agreed timing budgets.
PR #191 is merged as `67fa8f0`. Production run `36910124129` passed live byte
and provenance verification: validation was 132s excluding observed dispatch,
and merge-to-live wall time was 174s. Automatic follow-up `36910376804` found
the trusted artifact and skipped browsers, packaging and deployment. This proves
duplicate suppression, not warm restoration with changed coverage. The next
repair overlaps bounded artifact-history reads and shortens exact-source polling,
preserving all validation gates. Hosted timing and changed-coverage publication
remain post-merge acceptance owned by the delivery agent; #187 and ledger 2/3
remain unchanged until the full outcome is accepted.

PR #192 is merged as `58a07bd`; production run `36912354959` passed live
verification but measured 155s validation / 191s merge-to-live wall. Artifact
lookup fell to 5s; exact-source Check selection and coverage download grew to
50s. Coverage completed before platform packaging, but publication requires
the whole current-main Check. See the
[publication dependency assessment](tests/VALIDATION_PROFILE.md#website-publication-dependency-assessment-2026-10-01)
for timestamps, the source-code dependency chain, alternatives and acceptance.
The user authorised this assessment on 2026-10-01. Its recommendation is a narrow
website-only validation route plus independent website/coverage source decisions
under one production writer, initially within core. Estimate: medium, two
implementation PRs, with a possible third for hosted findings. This proposes a
publication-policy change; implementation and budget applicability to mixed
maintenance changes need agreement. It does not claim performance acceptance,
select the #178 repository migration or advance ledger 2/3. #187 remains active;
programme #180 remains paused pending its accepted outcome.

The user approved starting implementation on 2026-10-01. The first slice adds
the four-file website-only route, shared reusable browser validation and a
successful-main Check artifact consumed by Pages with existing trusted coverage.
Required compatibility gates depend on website success; shared, mixed, publisher
and unknown edits retain full native validation. Combined website-only timing
includes the originating Check, not just the restoration job. Production
acceptance remains pending after merge, including actual website-only publication
and old-coverage provenance. The next slice must still establish independent
coverage refresh against accepted website bytes under newer failing/pending site
changes, race/rollback handling and full cold/warm acceptance. #187 remains open,
ledger stays 2/3, and the original 120s/180s budgets have not been relaxed.
PR #194's review found that mixed changes could omit website results from the
required gates. The repair exports explicit website applicability, runs the
validator for applicable full routes, and requires both native and website
success. Pages defers that work to Check to avoid a duplicate browser run.
Native-only full routes retain their explicit website skip. Hosted mixed-route
negative acceptance must accompany the existing website-only fixture evidence;
this repair does not claim post-merge production acceptance.

PR #194 merged as `e913081`; main Check `36922481426` passed. Publication
`36922746934` restored the certified artifact and packaged it without browsers,
but GitHub's implicit job success condition propagated the browser skip into
deployment and live verification. No deployment occurred. The follow-up gives
those jobs explicit cancellation and direct-prerequisite success guards, with
regressions for skipped, failed and cancelled prerequisites. Actual deployment
and live verification remain post-merge acceptance owned by the delivery agent;
#187 stays open and ledger remains 2/3. This is a correctness repair within the
active website CI task, not completion or performance acceptance.

PR #197 merged as `9ef402d`. Main Check `36926148148` and production Pages
`36926424307` passed: certified website restoration, packaging, deployment and
live byte/MIME/provenance verification succeeded with duplicate browsers skipped.
The full-route change measured 178s combined validation and 210s merge-to-live
wall time, with 4s initial Check queue. This repairs production publication but
does not establish routine website-only budget acceptance.

PR #202 merged as `4ea9575`. Website-only Check `36930417262` skipped native
matrices and retained all 24 browser scenarios. Pages `36930649776` restored the
certificate, skipped duplicate browsers, deployed and passed live verification.
The then-combined report stayed at `9ef402d`. Combined Check-through-packaging
was 173s and merge-to-live wall time 204s (3s initial queue): route/publication
correctness is proven, but timing acceptance is not. Prepared browser time was
78s (28s initialization, 43s tests); preparation and packaging were 20s each.

**Current agreed publication boundary:** the user selected a separate GitHub
Pages coverage site after reviewing the coupling. This supersedes the historical
combined-site refresh/recombination and website/coverage race design above;
it does not select the full website repository migration #178. Coverage remains
browsable at <https://sproates.github.io/panackelty-coverage/>. Publisher PR #1 in
[`sproates/panackelty-coverage`](https://github.com/sproates/panackelty-coverage)
is merged; run `36933404078` deployed and verified every report file. Its built-in
GitHub token successfully reads public core artifacts without another credential.
Coverage generation and native validation remain unchanged in core. The publisher
polls about every 15 minutes and supports manual dispatch; GitHub schedule delays
and inactive-repository suspension remain operational limitations.

**Website cutover verified:** PR #203 merged as `5381bc5`. Main Check
`36935711159` and Pages `36935939432` passed, including all 24 browser scenarios,
certified artifact restoration without duplicate browsers, deployment and live
website/playground byte, Wasm MIME and independent website-provenance checks.
The homepage and both old coverage entry points return the exact merged bytes;
the compatibility landing was visually inspected live. Coverage attachment and
its obsolete wrappers are removed. Website publication records its own identity
in `publication.json`; the report retains independent `provenance.json`.

**Independent report refresh verified, automatic scheduling still pending:**
manual coverage run `36938575919` selected core Check `36935711159`, advanced the
live report from `9ef402d` to `5381bc5`, and verified every published report file.
It took 51s and did not trigger another core website publication; live website
identity stayed unchanged. No additional credential or publisher-code change was
needed. This proves report advancement independently of website deployment.
It does not yet prove a core-only main change leaves website publication idle.

At the 2026-10-02 investigation, the coverage workflow was enabled, with the
intended cron on main, but no `schedule` run had appeared since bootstrap.
Manual dispatch worked. The observable gap is scheduled-event delivery; the
underlying cause is unconfirmed. GitHub documents that scheduled events may be
[delayed or dropped](https://docs.github.com/en/actions/how-tos/troubleshoot-workflows).
Do not treat a manual refresh as automatic-refresh acceptance, or change working
publication code without evidence. If cron remains absent, investigate schedule
activation/delivery and assess a reliable trigger before claiming unattended use.

The #203 mixed/full-route publication measured 182s combined validation and 212s
merge-to-verified-live wall time (3s initial Check queue); publication alone was
41s. This is not routine website-only performance acceptance. Remaining work:
observe a successful scheduled refresh, establish core-only no-publication
production evidence, and obtain two cold/two warm routine publication measurements
against the original 120s/180s budgets. No #187 completion is claimed: issue stays
open, programme #180 paused, ledger 2/3; completing it makes grooming due.

Before PR #189, Pages cached browser binaries but still invoked
`playwright install --with-deps` on each relevant build. During PR #185 the observed job was still preparing tests
after 4m32s, with apt reporting 364 MB of additional disk use. That observation
establishes setup friction, not a complete publication baseline. This follow-up
to completed #151 can be delivered independently of the unscheduled repository
split [#178](#independent-website-publishing).

Acceptance:

- Compiler-only changes trigger no website assembly or browser provisioning,
  including through coverage-publication triggers. Preserve core checks and a
  correct coverage publication path; classify mixed and unknown inputs safely.
- Browser tests use a pinned prepared environment matching the test suite.
  Ordinary runs do not reinstall browsers or system packages. Assess the official
  Playwright image first; a derived image may include Node test dependencies.
- Dependency changes and maintenance updates explicitly refresh the environment.
  Document ownership and reproducible rebuilds; ordinary site edits do not rebuild it.
- Preserve existing test coverage and failure gates. Test relevant website,
  runtime-pin and integration changes, plus routing and provisioning failures.
- Measure cold and warm routine validation and merge-to-live times, including
  image pulls, dependency restoration, assembly, tests and deployment. Report
  queue time separately. Agree and record a numeric budget during assessment,
  before final evaluation, and meet it without weakening coverage. Replacing apt
  setup with a slow image download does not by itself satisfy acceptance.

Effort: S–M, provisionally 1–2 implementation PRs including meaningful routing
regressions and hosted publication verification. Existing pinned browser releases
and test ownership remain. Risks include mismatched versions, stale environments,
missed relevant changes and download latency. No self-hosted runner or hosting
change is preselected. Compiler changes, automatic release promotion and the
website repository migration are outside this item; website correctness remains
mandatory. Detailed implementation evidence belongs in #187.

### Compiler and runtime understanding programme: delivery and resumption

<a id="rm-9"></a>

**RM#9: Compiler understanding programme** · [GI#180: Compiler understanding programme](https://github.com/sproates/panackelty/issues/180).

**In progress:** the user authorised starting all five workstreams as one
coordinated programme on 2026-10-01; #163 live acceptance is now verified. This
expands the earlier #134-only selection. Work together on investigation, evidence design and delivery;
retain independently testable stages and child acceptance, rather than five
unrelated initiatives or one large implementation PR.

<a id="compiler-and-runtime-understanding-programme"></a>

Programme tracker: [#180](https://github.com/sproates/panackelty/issues/180).
**Overall: In progress; 0 of 5 workstreams accepted.** This counts accepted
workstreams, not an effort percentage. The initial shared investigation is
recorded in [ARCHITECTURE.md](ARCHITECTURE.md#compiler-and-runtime-understanding-initial-investigation-2026-10-01)
with [reproducible probes](tests/VALIDATION_PROFILE.md#compiler-understanding-probes-2026-10-01).
The first U3 subtraction query is accepted through #219 (`f379c50`); the local effect query is delivered on this PR's merge as recorded above. Broader
type/proof and transitive-effect explanations remain open. U2 provides production source maps; U3
retains subtraction decisions and guard origins. No production retained static
dependency or dynamic derivation graph exists.
The bounded U1 experiment establishes sidecar identity/fallback evidence;
producer authenticity and positive non-impact experiments remain open. The
bounded runtime-retention experiment is recorded above; production U7 remains open.

**Correctness prerequisite #182: accepted and closed.** PR #185 merged as
`81ba7b1`; its checker and public-CLI regression evidence is recorded in #180. The checker invalidates bounds across
direct/nested writes, expression children and loop iterations, retaining fresh
guards and unrelated facts. The public v9 seed is refreshed. Regression coverage
includes 20 formerly accepted unsafe cases, three positive fixtures, a 48-case
semantic matrix with actual VM execution, and public
CLI failure/success fixtures. The VM's runtime protection is unchanged. This
finishes the bounded core repair, not #134 or programme #180. The next programme
feature is source attribution and retained checker evidence. Remaining #187
work is deferred and does not block this programme. Validation passes; the measured 139s
full check and 86s unit phase exceed the 120s/15s targets. The U2 frontend focused compiler check also exceeded its 15s target (47s).
The instruction-source slice passed full validation in 158s; the final U2 sidecar/CLI slice passed in 163s, both above the 120s target. Keep validation-cost
profiling under #106 as an explicit prioritisation reminder; retain all coverage.
U3 final paired compiler-corpus compilation measured 21.331s before / 22.583s
after (+5.9% in one sample); earlier paired samples were noisy. Small explanation
median was 51.3ms; a whole-compiler query took 17.296s. These are observations, not
a general overhead guarantee. The first U3 full run exceeded the budget at 300s (unit 281s) and caught a
runner-smoke expected-count update; the corrected canonical rerun passed in
378s (unit 290s, functional 5s, bootstrap 71s). Retain #106 profiling as a priority-review candidate.
The earlier repair compiler-only checks show no material slowdown on their measured workload;
see the [repair measurements](tests/VALIDATION_PROFILE.md#guard-fact-repair-and-test-hardening-2026-10-01). Browser/native release adoption and
live verification remain separately recorded in the website follow-up register.

| Workstream | State | Acceptance focus / progress |
| --- | --- | --- |
| [RM#48: Compiler explanations](#rm-48) · [GI#134: Compiler explanations](https://github.com/sproates/panackelty/issues/134) | Investigation in progress | First guarded-subtraction query accepted through #219; full types/effects/proof scope remains open. |
| <a id="rm-91"></a>**RM#91: Compilation provenance** · [GI#173: Compilation provenance](https://github.com/sproates/panackelty/issues/173) | Investigation in progress | U1 attribution and U2 production mapping accepted; U4 connections through checking/lowering remain open. No accepted workstream delivery yet. |
| <a id="rm-92"></a>**RM#92: Counterfactual compilation** · [GI#174: Counterfactual compilation](https://github.com/sproates/panackelty/issues/174) | Investigation in progress | Derive sufficient requirements and validate them by actual compilation. No accepted delivery yet. |
| <a id="rm-93"></a>**RM#93: Semantic change prediction** · [GI#175: Semantic change prediction](https://github.com/sproates/panackelty/issues/175) | Investigation in progress | [RM#106: Semantic-impact experiment](#rm-106) establishes bounded direct predictions and positive local non-impact; one-change transitive acceptance and production remain open. |
| [RM#68: Runtime value provenance](#rm-68) · [GI#172: Runtime value provenance](https://github.com/sproates/panackelty/issues/172) | Investigation in progress | Explain opt-in computation/value derivations with bounded runtime overhead and retention. No accepted delivery yet. |

### Programme delivery register

This register is the implementation plan for #180. It gives an agent enough
context to resume without the originating conversation. ROADMAP.md owns scope,
priority, milestone state and the next action. Issue #180 links PRs and detailed
findings; the five child issues retain their full acceptance criteria.
ARCHITECTURE.md records accepted design decisions, tests/COVERAGE.md records
behavioural evidence, and tests/VALIDATION_PROFILE.md records reproducible probes
and performance measurements. Update these records together when delivery
changes their claims.

**Snapshot, 2026-10-02:** initial investigation #183 and correctness repair #185
are merged. U1 was accepted through #208; U2 frontend spans (#209), instruction
mappings (#214), and public sidecar/lookup (#218, `cbae41b`) are accepted. PR #219 (`f379c50`)
delivers U3's first working guarded-subtraction explanation.
It does not complete #134's full scope. All five workstreams remain open, with
**0 of 5 accepted**. This is an acceptance count, not an effort percentage.
This historical snapshot has no weighted baseline. The programme remains paused;
this tracking update does not estimate its weights or restart it. On a requested
progress report or resumption, establish a provisional scope-based baseline from
the complete register and current evidence using the
[programme tracking rules](docs/ROADMAP_PROCESS.md#programme-tracking). Do not
interpret its acceptance count as 0% estimated completion.

| ID | Outcome and acceptance | Dependencies | Current state / evidence | Initial size and PR estimate |
| --- | --- | --- | --- | --- |
| <a id="rm-97"></a>**RM#97: Shared programme investigation** (U0) | Complete shared investigation across all five: evidence inventory, source identity, runtime retention, positive non-impact probes, budgets and revised scope estimates | Existing #183 findings | Partial: #183 establishes the inventory; U1 supplies bounded identity/fallback evidence; local replay resolves U2 attribution trust; bounded runtime retention and [positive local non-impact](#rm-106) are investigated; authenticated external producers and broader semantic dependencies remain | Remaining investigation still required before U5–U7 scope commitments |
| <a id="rm-98"></a>**RM#98: Guard-fact correctness repair** (C0) · [GI#182: Guard-fact correctness repair](https://github.com/sproates/panackelty/issues/182) | Invalidate stale guard facts and preserve valid refreshed guards | None | Accepted: #182 closed by #185; 25 fixed fixtures and 48 generated pairs with VM execution | Delivered |
| <a id="rm-99"></a>**RM#99: Source attribution experiment** (U1) | Prove exact source attribution for a bounded runtime trap; local, imported and generic cases; reject stale/malformed/mismatched maps; compare representations and measure overhead | C0 and existing investigation | Accepted: PR #208 (`de26483`); [experiment, decision and limits](tests/experiments/source_mapping/README.md); no production ABI | Delivered in 1 feasibility PR |
| <a id="rm-100"></a>**RM#100: Production source maps** (U2) | Deliver the production source-map contract chosen from U1: deterministic identity, validation, compatibility and safe missing-map behaviour, with public-CLI tests | U1 design decision | Accepted: frontend #209, emission #214, exact local-replay sidecar and public CLI #218 (`cbae41b`); [contract](docs/SOURCE_MAPS.md) and [evidence](tests/VALIDATION_PROFILE.md) | Delivered in 3 PRs |
| [RM#48: Compiler explanations](#rm-48) (U3) | Retain checker evidence and deliver the first #134 explanation, then cover its agreed types/effects/proof scope; explain accepted and rejected obligations with source facts and honest unknowns | U1; production attribution from U2 before feature acceptance | First subtraction query accepted through #219; actual proof decisions, guard sources, rejected/unavailable results and CLI acceptance. Local effect query delivered on this PR's merge; broader type/proof scope remains | First slice delivered in 1 PR; remaining scope re-estimated at grooming |
| [RM#91: Compilation provenance](#rm-91) (U4) | Complete #173 compilation provenance, connecting source, checking, lowering and actual emitted bytecode; include generated instructions with unavailable attribution | U2 and relevant U0 design evidence | Unstarted; mapping alone does not complete provenance | Estimate after U1/U3 evidence |
| [RM#92: Counterfactual compilation](#rm-92) (U5) | Complete #174 inferred requirements; recompile proposed requirements and distinguish sufficient conditions from unsupported/minimality claims | Retained checker evidence from U3 and U0 constraint experiments | Unstarted | Estimate after U3 |
| [RM#93: Semantic change prediction](#rm-93) (U6) | Complete #175 semantic change prediction; apply changes and verify direct/transitive effects and claimed non-impact | U3 evidence and U0 dependency experiments; reuse U4/U5 where justified | Production unstarted; [RM#106: Semantic-impact experiment](#rm-106) supports direct/local claims, not automatic transitive guarantees | Smallest evidence slice: medium / 1 PR, separately selected; broader scope unknown |
| [RM#68: Runtime value provenance](#rm-68) (U7) | Complete #172 opt-in runtime value derivations, including retention, privacy and overhead controls | U0 runtime feasibility and U2 source identity; does not need to wait for U5/U6 | Production unstarted; bounded retention experiment and go/no-go recommendation recorded above | First scalar production slice: large / 2–3 PRs, separately authorised |
| [RM#10: Programme realistic evaluation](#rm-10) (U8) | Pass the realistic-program gate below across all five workstreams and complete release/docs acceptance | Accepted child scope from U3–U7 | Unstarted; corpus and budgets must be agreed before final evaluation | Estimate after investigation and representative corpus selection |
| [RM#11: Programme website demonstrations](#rm-11) (U9) | Final task: refresh website positioning and demonstrate the accepted compiler/runtime capabilities; deliberately adopt supporting releases and verify the live site | U8 accepted; published artifacts supporting advertised features | Planned, unstarted; tagline to be decided later with the user; acceptance below | Estimate after accepted capabilities and website scope are known |

U0 continues alongside production mapping and the later dependent workstreams. This is one coordinated
programme with independently testable deliveries. It does not require every
workstream to use one data structure or wait for a universal framework. The
order after U1 is provisional: revise dependencies and estimates from evidence,
recording the reason in this register and #180. No milestone silently narrows a
child issue's scope. Runtime explanations remain a separate engineering concern
where retention and execution cost differ from static evidence.

Each milestone moves through Planned, In progress, In review, Verification
pending and Accepted. A blocked milestone records the blocker and next action;
other genuinely independent work may be proposed. A merged experiment does not
close a workstream. Acceptance requires linked tests and review evidence, and
release or live checks where its scope requires them.

### Next action and agent handover

**U3 first query: accepted in PR #219 (`f379c50`).** The user selected
this bounded slice after U2 (#218) merged. Delivered command:
`panack explain SOURCE.panack --function NAME` reports the actual Nat subtraction
proof decision, constants/lower bound and guard branch/source from the checker.
It preserves checker diagnostics and explicitly distinguishes whole-project
validity from local proof status. See the [contract](docs/COMPILER_EXPLANATIONS.md)
and [validation evidence](tests/VALIDATION_PROFILE.md).

The query has no remaining acceptance; full local validation and all hosted
checks passed, and independent review found no actionable findings.
There is no release/live-site acceptance prerequisite for this core slice.
No child issue closes. The three-outcome grooming review selected bounded
performance work under #106 before the next principal programme feature; see
[the current priority decision](#performance-priority-review--2026-10-02).
U0 investigations remain open and U2 mapping alone does not complete #173.

An agent resuming this work should:

1. Read local and repository instructions, this programme section, #180, the
   relevant child issue and the linked investigation/acceptance evidence.
2. Check the actual checkout, current main, open PRs and worktrees. A different
   chat may have advanced the programme. Preserve unrelated work and never infer
   progress from this conversation or a stale branch.
3. Reconcile the register with merged evidence. Check current priorities
   and any unresolved release correctness obligation. If a milestone already has
   an owner or active PR, continue that delivery or agree a separate bounded
   scope rather than duplicating it.
4. Record the selected milestone, branch/PR, responsible agent or maintainer,
   acceptance cases, dependencies and next concrete action in #180. The agent
   doing the work owns validation, documentation and post-merge handover until
   explicitly handed over. Do not record personal identity details.
5. Deliver through a dedicated branch and PR, with unit/public-CLI regressions
   as appropriate and canonical validation under the repository instructions.
   Obtain independent review for compiler correctness and counterexamples before
   milestone acceptance. Reviewer findings and their resolution belong in the
   evidence record. A review does not replace tests or explicit merge approval.
6. Before stopping or handing over, update the milestone state here and the
   detailed record in #180. Name the exact remaining action, including any
   required approval or post-merge check. Keep failing probes and limitations.

Use this compact handover record in #180 for each active delivery:

- Milestone and child issue; state; responsible agent/maintainer.
- Branch, PR, tested commit and merged commit if applicable.
- Accepted criteria and linked commands/results; independent review and findings.
- Decisions, unresolved questions, blockers and remaining acceptance.
- Next concrete action, with enough context for another agent to execute it.
- Website/release impact, including explicit no-impact reasoning where applicable.

U1 was accepted through merged [PR #208](https://github.com/sproates/panackelty/pull/208)
at `de26483`. U2's frontend and emitter slices did not independently add grooming
outcomes. The final validated-sidecar delivery #218 completed U2 and advanced
the ledger to 2 of 3. U3's first-query outcome (#219) advanced it to 3 of 3;
the subsequent performance priority review above resets the baseline. It closes neither #173 nor #180. Existing #182 release/site
correctness work and programme demonstrations remain separate.

### Source-to-bytecode mapping foundation: U1 and U2 done

Roadmap item: [RM#100: Production source maps](#rm-100).

U2 connects retained expression ranges to actual emitted instructions and
provides validated local lookup with deterministic optional sidecars. It
preserves v9 bytes and returns unavailable for missing, stale, forged or
incompatible evidence. Exact snapshots include imports and core; sidecars contain
source text and lookup recompiles. These limits and privacy costs are part of
the [accepted contract](docs/SOURCE_MAPS.md), not deferred implementation details.

Acceptance evidence includes self-hosted unit/public-CLI tests, independent
actual-VM trap locations and coherent-forgery probes, ordinary/mapped byte
identity, fresh bootstrap fixed points, canonical validation and a compiler-sized
measurement. The [validation profile](tests/VALIDATION_PROFILE.md) records the
results. Independent review found an output directory-alias bug, fixed with a
post-artifact destination check and preservation regression, and a doubled
lookup timing sample, fixed by reusing the single captured output.

U2 has no separate issue; track it through #180/#173. This completes the shared
mapping milestone, while checker evidence, compilation reasons, runtime
provenance, source-aware errors (#136) and coverage (#131) retain their scope.
No live website claim needs correction for these commands. Alpha.11 is their
published native supporting release; explicit website/browser adoption remains
pending in the follow-up register.

The following U1 experiment scope is retained as historical decision context.
The smallest useful outcome is a bounded experiment mapping one runtime trap
back to its source expression, with independently checked local, imported and
generic-code cases. Compare deterministic sidecar metadata with a versioned
bytecode extension; decide the representation from evidence rather than assuming
a bytecode format change. Define how lowering and generated instructions map to
source ranges, including explicit cases where no source attribution is available.

- [x] Prove exact instruction-to-file/range attribution for the bounded trap,
  including imported and generic code.
- [x] Specify artifact/source identity, compatibility, validation, relative-path
  and privacy handling; test missing, malformed, stale and mismatched maps with
  safe fallback rather than incorrect source attribution.
- [x] Preserve execution semantics, bytecode verification, deterministic release
  artifacts and the ordinary bootstrap fixed-point contract; measure metadata
  size and compile/runtime overhead for the experiment.
- [x] Record the metadata decision, limitations, acceptance evidence and revised
  estimates for production mapping and each consumer.

Acceptance is bounded by the [experiment report](tests/experiments/source_mapping/README.md):
identity and integrity checks do not authenticate a malicious producer. The
production contract above addresses that boundary through exact local replay.
Canonical validation and independent review are recorded in #180/the delivery PR.
Website impact: none for U1, which changes no published syntax, runtime, release
artifact or website claim. The #182 corrective release follow-up remains open.

Estimate: small-to-medium investigation, provisionally one feasibility PR.
Production mapping, call stacks, coverage collection/reporting and provenance
integration require separately scoped delivery slices; a full debugger and async
history are outside this milestone. Mapping alone does not establish coverage
counter correctness or retain runtime value derivations. Keep consumer-specific
acceptance in #131, #136, #172 and #173.

This experiment changes no shipped capability or published version claim.
Consumer delivery must assess its own website follow-up.

### Programme acceptance: trustworthy and useful on realistic programs

<a id="rm-10"></a>

**RM#10: Programme realistic evaluation**.

**The explanations and predictions must be trustworthy and useful on realistic
programs.** Completing five isolated demonstrations is insufficient to close the
programme. Agree and record the representative programs and success criteria
before final evaluation; retain unsuccessful cases and limitations in the report.

- Use complete, reproducible application or tool workflows, including multi-module
  code, calls across module boundaries, generics and effects where supported.
  Select cases from practical development needs, not solely fixtures constructed
  to showcase the feature. Cover all five workstreams across the evaluation set;
  document unsupported categories rather than implying universal coverage.
- Establish trustworthiness against actual checker decisions, emitted bytecode
  and observed execution as appropriate. Apply predicted changes and compare
  predicted direct/transitive consequences with the real results. Check derived
  requirements by recompilation, and runtime provenance against execution.
  Include accepted, rejected, unsupported and deliberately misleading cases;
  unknown results must remain explicit. Silence is not evidence of non-impact.
- Demonstrate usefulness through recorded developer tasks: identify the cause of
  a rejection, make and verify a valid correction, assess a change's consequences,
  or explain an unexpected value. A reviewer who did not implement the feature
  must be able to use its output to reach a verifiable answer without reading
  compiler internals. Record the answer, supporting evidence, unresolved questions
  and confusing output; attractive text alone does not satisfy acceptance.
- Measure explanation latency, ordinary compilation overhead and runtime tracing
  cost on these programs against budgets agreed during investigation. Record
  reproducible commands, exact revisions, expected/actual results and reviewer
  findings. Resolve correctness failures within the agreed scope and review
  usefulness failures before claiming acceptance; any scope reduction needs an
  explicit user decision.

Keep this gate pending until its evidence is linked from #180 and the roadmap.
It supplements each child issue's acceptance and the release/website checks.

### Final task U9: website positioning and demonstrations

<a id="rm-11"></a>

**RM#11: Programme website demonstrations**.

Agreed on 2026-10-02: finish the programme with a website marketing refresh that
makes its demonstrated compiler and runtime understanding prominent alongside
exact values and the language's other foundations. Update the homepage blurb,
capability presentation and relevant learning/playground examples to show what
developers can achieve with the five accepted workstreams. Choose the tagline
later with the user; no proposed slogan is approved by this decision.

- Begin after U8 acceptance. Base claims on the realistic-program evidence and
  describe limitations and unknown results honestly. Distinguish native and
  browser support; do not advertise an unreleased or unsupported capability.
- Include tested examples of explanation, compilation provenance, inferred
  requirements, change prediction and runtime value provenance as supported by
  the adopted releases, including a rejected/explained/corrected example.
- Deliberately adopt published releases, verify download artifacts and run each
  example against its advertised version. Preserve truthful claims and working
  links throughout; recording this task does not publish or automatically update
  the site.
- Provide a working preview for user review before requesting merge approval.
  After publication, verify live examples, download links, version labels and
  feature claims, and link the evidence from #180 and the website follow-up
  register before accepting U9 or closing the programme.

Website repository separation #178 is not a prerequisite. This is the final
programme deliverable, not a sixth compiler/runtime workstream; the five-workstream
acceptance count remains unchanged.

At every accepted delivery, update this table and #180 with child issue/PR links,
acceptance evidence, blockers and remaining work. Keep child issues open until
their own agreed acceptance is met. A narrow first experiment does not complete
an entire workstream unless it fulfils that scope. Any deferral or scope reduction
requires an explicit user decision. Close the programme only after all five
workstreams, U8 evaluation/release acceptance and U9 website acceptance, or an explicitly
approved scope revision. Preserve completion evidence for independently accepted
outcomes without treating a bounded slice as completion of its umbrella issue.

Overall effort and PR count await the shared investigation. The previous
M / 1–2 PR estimate covers only the narrow #134 slice. Normal-build and runtime
costs must be measured, and explanations must use actual evidence rather than
invented narratives. Broader type inference #170 is adjacent, outside these five.
Source coverage #131, file discovery #167 and website separation #178 remain
independent candidates; review priorities at the usual checkpoints rather than
making the entire programme an unconditional prerequisite for unrelated work.

<a id="now-portable-automatic-pr-previews"></a>

### Completed: portable author-local previews

<a id="rm-12"></a>

**RM#12: Local website previews** · [GI#160: Local website previews](https://github.com/sproates/panackelty/issues/160).

The user selected [#160](https://github.com/sproates/panackelty/issues/160) on
2026-10-01. Previews must work independently of ChatGPT and contributor tooling.
The user clarified the scope on the same date: a temporary local preview for
the author, not a public URL or permanent staging site. The hosted-publication
proposal is superseded; no hosting provider or contributor account is required.
PR #161 delivers shared assembly, a checksummed portable build, loopback serving
and a tested CI artifact. The follow-up adds one-command build/start with fresh
temporary output, stop/cleanup and explicit rebuild instructions. See
[the local workflow](docs/PR_PREVIEWS.md).

**Done: PR #171 merged on 2026-10-01; #160 is closed.** Local validation passes: 24 website tests, complete
site assembly, a real default-command HTTP/Wasm/start-stop check, and canonical
`make check` (137s; existing non-blocking timing warning). The local browser route
was policy-blocked, so the user explicitly requested the existing private Sites
review route. The Sites skill published the exact clean PR build at
`c843571d07a7fb78fcfc34338372af29f4c8383b` to the owner-only
[review site](https://panackelty-staging.sproates846529.chatgpt.site).
The user confirmed it was working on their iPhone on 2026-10-01.

This verifies an optional private snapshot for this author's cloud workflow;
it does not make Sites a dependency of the portable local command or introduce
automatic hosted PR publication. Production is unchanged. No acceptance remains
for the agreed #160 scope. It counted once as the third accepted outcome
reviewed at the 2026-10-01 grooming checkpoint below.

<a id="rm-101"></a>

**RM#101: Panackelty preview server** · [GI#162: Panackelty preview server](https://github.com/sproates/panackelty/issues/162).

Backlog **Idea, unscheduled**: [#162](https://github.com/sproates/panackelty/issues/162)
would replace the serving component with a Panackelty-written local HTTP server.
Assess HTTP, file-I/O and long-running lifecycle needs first; this dogfooding
follow-up does not block the current Node-based workflow.

The finite TCP client/server stage is implemented on `main`: client PR #127,
server contract PR #129 and server implementation PR #130. Both operations are included in the published alpha.11 native release and were
absent from alpha.10 downloads. The server completion evidence
is recorded [below](#bounded-async-tcp-server).

The user selected backlog grooming after this milestone. The agreed scope covers
source coverage, component refactoring, development workflows, compiler
explanations, invariant testing, runtime diagnostics, documentation, editor
support, runnable demonstrations, resource baselines and a bounded independent
contract exercise. These are candidates for assessment, not blanket feature
authorisation. The user selected modular validation and component boundaries
under #106; its bounded validation slice is now complete as recorded below.
Programme #180 is the proposed return priority in the review below. Remaining
#187 work stays deferred. Other candidates remain unscheduled; this planning
change starts no compiler implementation.

The sections below retain earlier decisions and completion evidence. Current
candidate state and the agreed next task are recorded in the grooming table;
historical recommendations do not create competing implementation queues.

### Programme priority review, 2026-10-02

**Proposed decision, effective on approval and merge of this documentation:**
resume #180 with U1 and maintain the resumable programme register requested by
the user. Remaining #187 performance/scheduling work stays deferred: the delivered
website and independent report were accepted, so it no longer blocks compiler
work. Publishing the #182 corrective compiler/browser release remains a separate
correctness follow-up; revisit its urgency if current live claims or examples
are shown to be wrong. Other newly recorded language/tooling/performance ideas
remain candidates rather than silently expanding this programme.

This review considers the three accepted outcomes in the previous ledger below:
#163 playground layout, #182 guard-fact repair, and the accepted #187 website/
independent-coverage delivery. Source mapping is proposed first because it enables
trustworthy explanations and runtime locations; the next slice has a bounded
experiment and explicit failure cases. Further CI tuning is deferred by the user,
while a corrective release needs coordinated release/adoption work. Preserve its
recorded obligation without treating it as an explanation feature.

The new baseline takes effect on this planning PR's merge: **0 of 3 accepted
outcomes since this review**. The documentation and review do not themselves add
an outcome. Review again after three accepted outcomes or earlier if new evidence
changes priorities. This records direction and handover requirements; it neither
accepts a compiler milestone nor authorises future PR merges.

Previous ledger: **3 of 3 accepted outcomes**: U1 feasibility (#208), U2
production mapping (#218), and U3 first query (#219). Frontend/emitter slices
count within U2, not again. The subsequent performance priority review above
considers these outcomes and starts the current 0/3 baseline. No workstream or
programme closes from these bounded outcomes.

### Historical delivery record

Previous baseline and completion history (superseded by the review above on merge): the 2026-10-01 review, agreed with the user: fix #163 first, then
the compiler and runtime understanding programme #180, expanded from #134
on 2026-10-01. This supersedes the 2026-09-30 review in
[PR #143](https://github.com/sproates/panackelty/pull/143).

The review considered these three completed outcomes once each:

1. Bounded modular validation under #106, delivered in [PR #144](https://github.com/sproates/panackelty/pull/144); the broader
   cache/design work remains unscheduled.
2. Browser ownership and CI cleanup under #151, completed through browser #5,
   core #158 and the [verified completion record](#browser-repository-separation--done) in #159.
3. Author-local website previews under #160, delivered through #161 and #171,
   including the [user-accepted private iPhone review route](#completed-portable-author-local-previews).

Accepted deliverables since that previous baseline: **3 of 3**, considered by
the 2026-10-02 review above.

1. Playground layout #163: accepted preview, published browser v0.1.1, merged
   core pin and independently verified live assets/provenance. See the
   [completion record](#completed-playground-footer-layout).

2. Guard-fact correctness repair #182 within #180: core invalidation, regression
   coverage and refreshed v9 compiler seed, delivered in merged PR #185.
   Website release adoption remains separate and is not counted again as this repair.

3. Website CI and independent coverage delivery #187: user accepted the delivered
   state as good enough on 2026-10-02, after verified website publication and
   independent report refresh. Remaining scheduling and performance acceptance
   are explicitly deferred, not represented as passing. Count this delivered
   outcome once; documentation PRs do not add outcomes.

Programme #180 has started shared investigation; no workstream is accepted yet.
This initial investigation is an intermediate programme slice, not another
completed outcome. Grooming and bookkeeping do not add an outcome. Review again
after three accepted outcomes, or earlier if material evidence changes priorities.

### Groomed candidates

Except for **In progress** compiler explanations #134 within programme #180 above, the existing entries
below remain **Idea**, with implementation unscheduled. #106
has completed its bounded validation slice as recorded below; its later work
is unscheduled. The user agreed
the assessment scope; the estimates describe each first useful outcome, including
tests, documentation and integration. They are not estimates for completing every
extension. Linked issues hold acceptance detail; this table owns current state.
The debugging sequence agreed on 2026-10-01 is recorded below; it establishes
dependencies without replacing the current principal programme.

| Candidate and work record | First useful outcome | Size / estimated PRs |
| --- | --- | --- |
| [RM#46: Panackelty source coverage](#rm-46) ([GI#131: Panackelty source coverage](https://github.com/sproates/panackelty/issues/131)) | Prove exact source/execution attribution before collecting and publishing compiler/library baselines. Source branches, denominator correctness and collection failure remain explicit. | M / 1 feasibility; then provisionally 2–3 |
| [RM#62: Component readability](#rm-62) ([GI#132: Component readability](https://github.com/sproates/panackelty/issues/132)) | Inspect one component and fix a concrete readability or responsibility problem, with behaviour protection and a short follow-up list. | S–M / 1 for first component |
| [RM#47: Development workflow assessment](#rm-47) ([GI#133: Development workflow assessment](https://github.com/sproates/panackelty/issues/133)) | Observe installation through maintenance, including whether developers can diagnose and fix traps, external failures and incorrect results. | S–M / 1 assessment |
| [RM#48: Compiler explanations](#rm-48) ([GI#134: Compiler explanations](https://github.com/sproates/panackelty/issues/134)) | Expose one useful compiler-backed explanation of checked types, effects or guard facts, including why a case is rejected or unresolved. | M / 1–2 for one query |
| [RM#49: Invariant testing](#rm-49) ([GI#135: Invariant testing](https://github.com/sproates/panackelty/issues/135)) | Extend existing equivalence tests with one bounded, reproducible generated-input or semantics-preserving transformation family. | M / 1–2 for one family |
| [RM#50: Source-aware runtime errors](#rm-50) ([GI#136: Source-aware runtime errors](https://github.com/sproates/panackelty/issues/136)) | Explain a bounded set of runtime failures with source expressions and useful call context; validate through the debugging guide and preserve safe metadata fallback. | M–L / 1–2 after mapping design |
| [RM#51: Learning and debugging guides](#rm-51) ([GI#137: Learning and debugging guides](https://github.com/sproates/panackelty/issues/137)) | Deliver the installation-to-maintenance tutorial and an explicit debugging guide using available tools, with later source-aware updates. | M / 1–2 for first tutorial |
| [RM#52: Executable documentation](#rm-52) ([GI#138: Executable documentation](https://github.com/sproates/panackelty/issues/138)) | Verify documentation commands and outputs, explicitly including the debugging guide’s failing examples, fixes and regression tests for its declared release. | M / 1–2 for one surface |
| [RM#53: Interactive debugger](#rm-53) (no issue yet) | Assess a bounded synchronous CLI experiment for breakpoints, stepping, locals and call frames against observed debugging gaps. | M / 1 assessment; delivery estimate follows evidence |
| [RM#54: Editor support](#rm-54) ([GI#139: Editor support](https://github.com/sproates/panackelty/issues/139)) | Provide highlighting and basic editing in one selected editor; assess compiler-backed features separately. | S–M / 1–2 for one editor |
| [RM#55: Showcase programs](#rm-55) ([GI#140: Showcase programs](https://github.com/sproates/panackelty/issues/140)) | Deliver one complete, tested demonstration combining existing language capabilities and explicit failure boundaries. | M / 1–2 for one demonstration |
| [RM#56: Performance baselines](#rm-56) ([GI#141: Performance baselines](https://github.com/sproates/panackelty/issues/141)) | Planned core baseline under [RM#123](#rm-123); execution not started. Reproducible compile/run, memory and artifact-size evidence guides separately selected profiling/optimisation. | M / 1–2 for initial baseline; later optimisation separately scoped |
| [RM#57: Independent contract assessment](#rm-57) ([GI#142: Independent contract assessment](https://github.com/sproates/panackelty/issues/142)) | Attempt a narrowly scoped independent implementation from the written bytecode contract and record ambiguities. | M / 1 bounded assessment |

<a id="proposed-first-step"></a>

### Completed: modular validation and component boundaries

<a id="rm-13"></a>

**RM#13: Modular validation route**.

**Done: the bounded validation slice of [#106](https://github.com/sproates/panackelty/issues/106)**,
effective when its delivery PR merges. The broader issue remains open for
separately selected cache and compilation investigations.

The local `scripts/validate_change.sh --plan|--run BASE_REF` entry point and CI
share one selector. The reviewed process documents now run document/link and
whitespace checks without compiler builds or sockets. Other components retain
full canonical and hosted integration evidence; explicit ownership and the
[dependency map](tests/README.md#component-dependencies-and-retained-coupling)
explain that conservative boundary. Playground and Pages PR consumers use the
same selection, including unknown/shared inputs. Production publication and
release gates remain unchanged.

Acceptance evidence: independent component, shared/mixed/unknown, rename,
deletion, mode and local-index regression cases; real local execution with
compiler/network commands forbidden; failure propagation and stable CI gates;
canonical unit/functional/bootstrap validation. Three process-only runs in an
isolated real repository passed inside the restricted sandbox in 0.81, 0.81 and
0.80 seconds without creating native artifacts. The full development check
passed in 103 seconds with socket access; the existing unit budget warning
remains recorded in the [measurement report](tests/VALIDATION_PROFILE.md#modular-validation-route--2026-09-30).

This delivers the first useful slice in one PR. It does not narrow individual
code-component test envelopes or implement cached outcomes, dependency-aware
artifact reuse or separate source-module compilation. Those require independent
evidence; the launcher, probes, bootstrap and packaging retain coupling.

The prior selection favoured this task over source coverage because unnecessary
socket/build dependencies were directly observed during process editing.
Source-coverage feasibility (#131), editor basics (#139) and invariant testing
(#135) should be reassessed next; none is automatically scheduled by completion.

### Additional ideas recorded on 2026-10-01

Entries #164–#170 remain **Idea, unscheduled**. Entries #172–#175 are now
**In progress (shared investigation)** within [programme #180](#compiler-and-runtime-understanding-programme),
whose table above owns current progress. The summaries below retain their scope;
issue detail defines acceptance. Preserve independently useful outcomes and
justify shared evidence rather than requiring a large common framework.

| Work record | Bounded intent / relationship |
| --- | --- |
| <a id="rm-85"></a>**RM#85: Extensible numerics** · [GI#164: Extensible numerics](https://github.com/sproates/panackelty/issues/164) | Assess real/complex representations, operations and exactness guarantees. |
| <a id="rm-86"></a>**RM#86: Browser event experiment** · [GI#165: Browser event experiment](https://github.com/sproates/panackelty/issues/165) | Repeated browser events invoke a bytecode handler through an explicit host boundary. |
| <a id="rm-87"></a>**RM#87: Typed HTTP messages** · [GI#166: Typed HTTP messages](https://github.com/sproates/panackelty/issues/166) | Two independent Panackelty services exchange typed messages; assess HTTP and lifecycle gaps. |
| <a id="rm-88"></a>**RM#88: File discovery** · [GI#167: File discovery](https://github.com/sproates/panackelty/issues/167) | Deterministic recursive include/exclude matching with filesystem errors and symlink policy. |
| <a id="rm-89"></a>**RM#89: Subprocess experiment** · [GI#168: Subprocess experiment](https://github.com/sproates/panackelty/issues/168) | Exercise process execution and standard streams, including failure boundaries. |
| <a id="rm-90"></a>**RM#90: Tiny build tool** · [GI#169: Tiny build tool](https://github.com/sproates/panackelty/issues/169) | Dogfood discovery and processes in a bounded build workflow; establish prerequisites. |
| [RM#64: Inference and diagnostics](#rm-64) · [GI#170: Inference and diagnostics](https://github.com/sproates/panackelty/issues/170) | Demonstrate current inference limits and assess one predictable, sound expansion. |
| [RM#68: Runtime value provenance](#rm-68) · [GI#172: Runtime value provenance](https://github.com/sproates/panackelty/issues/172) | Investigate opt-in runtime value derivations; distinguish provenance from instruction tracing. |
| [RM#91: Compilation provenance](#rm-91) · [GI#173: Compilation provenance](https://github.com/sproates/panackelty/issues/173) | Connect one source construct through checking/lowering to actual emitted bytecode. |
| [RM#92: Counterfactual compilation](#rm-92) · [GI#174: Counterfactual compilation](https://github.com/sproates/panackelty/issues/174) | Derive a sufficient requirement from checker evidence and verify it by recompilation. |
| [RM#93: Semantic change prediction](#rm-93) · [GI#175: Semantic change prediction](https://github.com/sproates/panackelty/issues/175) | Predict direct/transitive proof consequences and verify against an actual change; unaffected claims require evidence. |

Durable resumable execution remains in the existing exploration below: persisted
checkpoints and crash recovery are not delivered by the completed in-memory VM
suspension work. It is distinct from value provenance and remains unscheduled.
#162, the Panackelty-written preview server, remains separate from completed #160.

## Independent website publishing

<a id="rm-14"></a>

**RM#14: Independent website publishing** · [GI#178: Independent website publishing](https://github.com/sproates/panackelty/issues/178).

Work record: [#178](https://github.com/sproates/panackelty/issues/178).

State: **Idea; unscheduled**. Recorded at the user's request on 2026-10-01.
Scope: maintenance. This does not select the repository migration. The separate [planned #187 CI improvement](#next-fast-website-ci-and-prepared-browser-test-environments)
can proceed before this split.

Website edits still share core validation and coverage publication through
`.github/workflows/pages.yml`, despite consuming a pinned browser release.
The user reports that simple website publication can take half an hour; current
hosted timings need measuring. This slows content corrections and site work.

Move pages, assets, public documentation and their publishing workflow into
`sproates/panackelty-website`. Keep the compiler, VM, specification and runtime
bundle contract in core, and browser adaptation/playground releases in
`sproates/panackelty-browser`. The site explicitly pins tested browser artifacts
and advertised native releases. A new core or browser release must not change
the live site until a separately reviewed website update is published.

Acceptance for the eventual implementation:

- Website-only changes validate and publish without core builds or waiting for
  core CI. Use focused content/link/assembly checks and a browser smoke test;
  retain full browser integration checks when runtime pins or integration change.
- Measure routine validation and merge-to-live latency, including cold and warm
  runs. Proposed budgets are under three minutes for validation and under five
  minutes from merge to live, reporting runner queue time separately. Confirm
  these budgets during assessment; they are not measured guarantees.
- Give core coverage an independent publication path, preserve existing public
  URLs through migration or redirects, and verify domain ownership, previews,
  rollback and deployed assets before retiring the existing publisher.
- Publish only existing, checksum-verified release artifacts. Validate download
  targets and execute advertised examples against their declared released
  version, including examples that run in the pinned playground. Review prose
  and syntax claims as well as machine checks; label unreleased features and
  differing native/browser versions explicitly. Do not call an older pin
  "latest" without verifying that claim.
- Record core-to-website follow-ups through the
  [website impact process](docs/ROADMAP_PROCESS.md#website-impact-and-follow-ups).
  Recording a follow-up never schedules or starts the update. Version lag is
  acceptable only while every published claim remains true for its stated version.

Effort: M, provisionally 2–3 PRs across core and the new website repository,
including migration and acceptance. Dependencies include release availability,
Pages/domain configuration, preview portability and a separate coverage
publisher. Risks include broken URLs, overlapping deployment writers and stale
version claims. The artifact boundary already exists; hosting cutover and timing
remain to be assessed. No compiler changes or automatic upgrade bot are included.

### Website follow-up register

- <a id="rm-127"></a>**RM#127: Website release history** — Done
  (2026-10-04, PR #269). Added changelog-generated release history with dated
  notes, migration guidance, release links, labelled current-source changes and
  independently pinned native/browser availability. Homepage, release history
  and playground share navigation and footer markup; playground resource links
  and versioned assets are preserved. Prepared notes never imply published
  downloads. No release was created and native/browser pins are unchanged.
  Owner preview acceptance, independent review and required checks passed.
  Production deployment and live byte verification passed for merged revision
  `5bbbd29d9c718764bd4a2056431963fea16f5152`
  ([Pages evidence](https://github.com/sproates/panackelty/actions/runs/37211779050)).
  No dedicated issue. Performance impact is limited to static website assembly;
  no compiler/runtime path changes or validation-budget improvement are claimed.
  Existing RM#123 budget disposition and GI#106 follow-up remain unchanged.


User decision, 2026-10-04: ordinary website content/promotion updates wait for
the next supporting release. Retain these entries and release prerequisites;
this does not schedule website implementation or reopen deferred GI#187 trials.
Existing broken links, failing advertised examples or false claims remain
correctness defects under the separate [website process](docs/ROADMAP_PROCESS.md#website-impact-and-follow-ups).

- <a id="rm-122"></a>**RM#122: Optional one-command installation guidance** ·
  Follow-up to [RM#121 / GI#256](#rm-121) — **Done (2026-10-04)**.
  [PR#259: Optional installation guidance](https://github.com/sproates/panackelty/pull/259)
  merged as `3ee554f`. The optional route preserves all manual alpha.10 commands
  and adds eight independently labelled copy controls. Main checks and
  [Pages publication](https://github.com/sproates/panackelty/actions/runs/37171626046)
  passed. The live page exactly matches merged HTML, including command/link
  targets; the public installer matches reviewed bytes. The owner accepted the
  live phone clipboard/layout verification on 2026-10-04. No platform or release
  support changes. See [acceptance evidence](tests/VALIDATION_PROFILE.md).

- <a id="rm-119"></a>**RM#119: Cookie-free website analytics** ·
  [GI#252: Cookie-free website analytics](https://github.com/sproates/panackelty/issues/252) — Idea / unscheduled.
  Add basic aggregate visits, page views, popular pages and referrers, provisionally
  using free Cloudflare Web Analytics with existing GitHub Pages hosting and DNS.
  Acceptance: verify a configuration requiring no cookies or consent banner under
  applicable rules; exclude fingerprinting, advertising, cross-site tracking,
  playground contents and sensitive URL data. Provide an accurate footer privacy
  notice, production-only collection, reviewed website preview and live dashboard
  verification after authorised deployment. Account setup or the public snippet
  is a prerequisite; no account API key belongs in the repository. Small: one
  website PR plus setup and verification. Recording this starts no implementation.

- <a id="rm-118"></a>**RM#118: Website installation command parity** ·
  [GI#250: Website installation command parity](https://github.com/sproates/panackelty/issues/250) — **Done**.
  Delivered in [PR#253: Website installation instructions](https://github.com/sproates/panackelty/pull/253).
  Website Linux x86_64 and macOS arm64 alpha.10 download, checksum, extraction
  and version commands match the tested README blocks and stop on failures.
  Prerequisites, supported OS versions, local first-program commands and optional
  PATH guidance are explicit. Release and browser pins remain unchanged.
  Local validation, independent review, hosted browser/release checks and owner
  preview acceptance passed. On 2026-10-04, production Pages deployment
  [37163232362](https://github.com/sproates/panackelty/actions/runs/37163232362)
  succeeded; the live installation section matched the merged source exactly.
  See [verification evidence](tests/VALIDATION_PROFILE.md#website-installation-command-parity--2026-10-04).

- <a id="rm-102"></a>**RM#102: Source-map website adoption** — U2 public source-map commands under [GI#180: Compiler understanding programme](https://github.com/sproates/panackelty/issues/180): dedicated examples/documentation pending; owner: programme
  delivery agent. The browser still advertises v0.1.1; alpha.11 is the published native
  supporting release and is adopted by native downloads/installer through PR #272.
  Dedicated source-map documentation and example acceptance remain pending. Next,
  consider CLI documentation and a validated lookup example, stating exact-source
  reproduction, unavailable fallback, lookup cost and full-source sidecar privacy.
  Browser examples remain unchanged. Do not advertise
  automatic runtime explanations. Acceptance requires checking the example against
  the adopted artifacts and verifying the published pages. Recording this entry
  does not start website work; U9's broader positioning remains separate.

- [RM#8: Website CI follow-ups](#rm-8) — **Separate coverage host:** website cutover and independent manual report
  refresh verified; owner: delivery agent. PR #203 / Pages `36935939432` passed
  live homepage, playground, both old entry points and website identity checks;
  landing layout inspected. Coverage run `36938575919` advanced the report to
  core `5381bc5` without another website deployment. No runtime version change.
  Automatic scheduling, core-only no-publication and timing acceptance remain
  open but deferred under #187 by user decision; they no longer block other work.

- <a id="rm-103"></a>**RM#103: Corrective compiler release adoption** — follow-up to [GI#182: Guard-fact correctness repair](https://github.com/sproates/panackelty/issues/182): owner is the #180 delivery agent until
  handed over. The published v0.1.1 playground compiler has the same SHA-256 as
  the affected core seed; plan a corrective compiler/browser release and explicit
  website pin update after the core repair is merged and released. Alpha.11
  is the published native corrective release; browser adoption remains unassigned. Review static-safety and guarded-type claims against the actual
  published runtime; the homepage's literal guarded-type example does not exercise
  the reproduced mutation defect. Keep this follow-up open through public
  verification that the unsafe examples are rejected and valid examples still run.
  Recording it does not start browser implementation or imply browser adoption.

Record concrete follow-ups here until ownership moves explicitly to the website
repository. Each entry needs the source issue/PR, affected pages and claims,
currently advertised version, target version or release prerequisite, required
update, correctness assessment, owner, state and acceptance evidence. Use a
linked issue for detail; retain enough information here if GitHub is unavailable.
Never mark an update complete merely because its PR merged: verify the live site.

- <a id="rm-104"></a>**RM#104: Explanation website adoption** — **U3 subtraction and local effect explanation CLI:** examples/documentation pending; owner is the core
  delivery agent until release/adoption handover. After a supporting native
  release, update command examples and feature claims with the bounded
  Nat-subtraction and local call/await effect scope, whole-program/local-boundary
  distinction and unavailable cases. After a release supporting
  [RM#107: Per-function effect recovery](#rm-107), explain valid sibling recovery,
  global declaration gating, invalid-function unavailability and unchanged original
  diagnostics. Alpha.11 is published and adopted by native downloads/installer
  through PR #272; dedicated explanation examples remain pending. Explain declared/callable effects without
  implying a transitive effect graph or runtime execution. Browser support must be verified separately before advertising it.
  The native version promotion does not complete those examples or browser
  adoption. Broader demonstrations remain in U9.
- [RM#11: Programme website demonstrations](#rm-11) — **Final website refresh (U9):** promotion pending; owner is the
  agent or maintainer delivering #180 until explicitly handed over. Affected
  surfaces: homepage capabilities, learning examples and playground where supported.
  Refresh the marketing blurb and capability presentation using U9 acceptance
  above, with the tagline left undecided. Add tested demonstrations, including a
  rejected/explained/corrected example, and describe each delivered capability
  accurately. Current baseline: browser pin v0.1.1 and native download
  alpha.11, verified after PR #272; recheck actual advertised versions when implementing.
  Target release is not assigned. Prerequisites: feature acceptance, published
  native/browser artifacts supporting each example, and explicit website adoption.
  Keep existing claims unchanged until supported; no current-site defect is
  asserted by this entry. No automatic update is scheduled. Close after live
  verification of examples, download links and version/feature claims. Website
  repository separation #178 is not a prerequisite.

### Additional platform and distribution ideas

Recorded at the user's request on 2026-09-30 for future grooming. All four
items are **Idea**, with implementation unscheduled; this adds no priority,
release commitment or completed implementation. Relate
installation friction to the [development workflow assessment](#development-workflow-assessment)
(#133) when comparing these with other candidates.

Linux arm64 enables native arm64 containers and packages. Docker can first ship
amd64 independently; Homebrew on the existing macOS arm64 target is independent.
A downloadable Debian package and a hosted apt repository are separate stages.
These proposals do not change the current release support policy.

<a id="linux-arm64-support"></a>

#### Support Linux arm64 release artifacts

<a id="rm-15"></a>

**RM#15: Linux arm64 releases** · [GI#145: Linux arm64 releases](https://github.com/sproates/panackelty/issues/145).

Work record: [#145](https://github.com/sproates/panackelty/issues/145).

Linux x86-64 and macOS arm64 releases exist; Linux arm64 is missing. Native ARM containers, ARM cloud hosts and compatible Raspberry Pi systems would benefit. macOS arm64 binaries cannot run in Linux containers.

Add a native Linux arm64 CI runner, validate bootstrap, VM, networking and exact packaged installation, then publish a checksummed release archive with an explicit OS baseline. Run meaningful compiler/unit/functional and release gates on that target. Exclude Windows, other architectures and a promise to support every ARM board.

M / provisionally 1–2 PRs including tests, release integration and documentation. Portability failures, runner availability and ongoing CI cost remain uncertain. Enables native arm64 Docker images; amd64-only images can proceed independently.

<a id="container-distribution"></a>

#### Publish versioned Panackelty Docker images

<a id="rm-16"></a>

**RM#16: Docker distribution** · [GI#146: Docker distribution](https://github.com/sproates/panackelty/issues/146).

Work record: [#146](https://github.com/sproates/panackelty/issues/146).

The self-contained release archives are suitable inputs for repeatable CI and agent environments, but no maintained container image is currently provided.

Publish a toolchain image to GHCR from validated release artifacts. Verify version/checksum provenance, check/compile/run workflows, bind-mounted projects and output ownership. Use explicit version tags and document digest pinning and base-image maintenance. Publish amd64/arm64 manifests only after both Linux targets pass release gates. Exclude application hosting and runtime-only images.

M / provisionally 1–2 PRs. Depends on existing release gates; native multi-architecture delivery depends on the Linux arm64 proposal. Registry permissions, base-image updates and partial publication recovery need assessment. No urgency or implementation order is agreed.

<a id="homebrew-distribution"></a>

#### Provide Panackelty installation through a Homebrew tap

<a id="rm-17"></a>

**RM#17: Homebrew distribution** · [GI#147: Homebrew distribution](https://github.com/sproates/panackelty/issues/147).

Work record: [#147](https://github.com/sproates/panackelty/issues/147).

Manual archive installation is supported; a maintained Homebrew tap would simplify installation and upgrades for developers and coding agents.

Create a project-owned tap with pinned release/checksum inputs and an explicit supported-platform policy. Assess source builds versus binary packaging, then verify clean installation, check/compile/run, upgrades and removal. Automate or document release updates. Exclude homebrew/core admission while the project remains an alpha.

S–M / provisionally 1–2 PRs including integration and documentation. Independent of Docker and Linux arm64 for the existing macOS arm64 target. Formula layout/relocation, Homebrew policy and ongoing release maintenance require validation.

<a id="debian-package-distribution"></a>

#### Assess Debian packages and apt distribution

<a id="rm-18"></a>

**RM#18: Debian distribution** · [GI#148: Debian distribution](https://github.com/sproates/panackelty/issues/148).

Work record: [#148](https://github.com/sproates/panackelty/issues/148).

Linux users currently unpack archives manually. Native package installation could simplify removal and upgrades; demand for a hosted apt repository remains unmeasured. Existing PREFIX/DESTDIR installation provides a starting point.

First assess and deliver a downloadable .deb for an explicitly supported Debian/Ubuntu baseline. Verify dependencies, installation, check/compile/run, upgrade and removal on that baseline. Consider a signed apt repository separately when demand justifies hosting, scoped signing keys and key rotation. Exclude official Debian/Ubuntu archive inclusion and blanket distro support.

M / provisionally 1–2 PRs for a downloadable package; hosted apt repository effort remains unknown. Validate ABI compatibility rather than assuming an Ubuntu-built archive works on every Debian release. arm64 packages depend on Linux arm64 support; amd64 packaging is independent. Ongoing repository/security maintenance is a material cost.

### Additional installation proposals — 2026-10-04

RM#120 remains **Idea / unscheduled**. RM#121 was selected on 2026-10-04 as
an optional alternative to the existing manual installation, which stays supported.
Neither route replaces the other. These proposals complement
[RM#17: Homebrew distribution](#rm-17) and
[RM#18: Debian distribution](#rm-18).

<a id="rm-120"></a>

#### RM#120: Graphical installer

[GI#255: Graphical installer](https://github.com/sproates/panackelty/issues/255).

Casual users should be able to download a guided installer without manually
extracting archives or entering checksum commands. Assess an initial macOS arm64
installer, including format, signing/notarisation, Gatekeeper, permissions,
command discovery, upgrades and removal. Reuse validated release artifacts;
this does not add Windows or other architecture support. Verify clean install
through Hello World and check/compile/run, failure handling, upgrades and removal.
Update release delivery and website/README instructions once verified.

Provisional M–L / 2–3 PRs; signing credentials, costs and release maintenance
require assessment before commitment. Independent of Homebrew and the terminal
installer. Priority remains unset.

<a id="rm-121"></a>

#### RM#121: One-command installer

[GI#256: One-command installer](https://github.com/sproates/panackelty/issues/256).

Offer a short, copyable terminal installation command for supported macOS/Linux
platforms without requiring a package manager. Detect OS/architecture, select a
versioned release, verify downloads before installation and fail safely on errors.
Prefer user-owned locations; define command discovery, explicit opt-in PATH
changes, repeat installation, upgrades, removal and interruption recovery.
Make the script inspectable and assess its transport/integrity trust. Verify
clean install through Hello World and check/compile/run plus failure cases on
each supported target. Retain manual instructions as a fallback and update
website/README guidance only when the installer is available and verified.

Provisional M / 1–2 PRs including platform tests and release/documentation work.
Independent of Homebrew and the graphical installer; existing supported release
artifacts are prerequisites. Portability, file ownership and ongoing maintenance
have been assessed for the bounded installer implementation.

State: **Done (2026-10-04)**. Selected with owner approval on 2026-10-04;
[PR#258: Optional one-command installer](https://github.com/sproates/panackelty/pull/258)
merged as `28f3605`. It adds `scripts/install.sh`, offline regressions and
published-release acceptance in the Ubuntu 22.04/macOS 14 CI matrix. The installer
uses dedicated user-owned storage, HTTPS/checksum verification, archive checks,
native compiler smoke tests, immutable releases, atomic activation, conflict
refusal, retained versions, owned removal and opt-in PATH guidance. Manual
installation remains supported. The public raw script matches the reviewed
bytes; the documented Linux one-command install and complete Hello World workflow
passed after merge. Platform and public-availability evidence is recorded in
[the validation profile](tests/VALIDATION_PROFILE.md).

The optional website route is tracked as RM#122 in the website follow-up register.
PR#259 delivered that route; post-merge checks and production publication passed.
Live HTML and installer bytes match the merged versions, preserving manual command
text. The owner accepted the live clipboard/layout check on 2026-10-04. Linux and
macOS published-release acceptance passed; this completion update closes GI#256.
The clean local check still exceeds the 120s full-check budget; keep
[GI#106: Validation performance](https://github.com/sproates/panackelty/issues/106)
as the existing prioritized profiling reminder, without reducing coverage.

### Scope retained outside this grooming batch

Existing namespace and standard-library organisation ideas, broader inference,
JSON/library work, host integration and repository tooling remain unscheduled.
The finite server does not provide DNS, TLS, HTTP, indefinite services, general
source spawning or public resource scopes. Revisit those against a concrete
application and its ownership requirements; no automatic next networking phase
is selected. Alpha.10 remains the downloadable release; a networking release
requires its own release scope and gates.

Full separate compilation remains conditional on design evidence under #106;
bounded probe reuse is independently assessable. The stateful REPL remains
unscheduled now that the complete-program playground is available. Revisit it
when a concrete session workflow justifies the additional semantics. Broader
platform and language explorations retain their existing proposals. Known
correctness or safety defects are considered promptly on their actual risk.

Website contributor links from PR #125 were delivered in the squash merge of
PR #127. The original PR #125 is now closed as redundant; its two website files
match main. No website appearance changes are included in this validation slice.

For each proposed design, assess the concrete guarantee or workflow it improves,
its fit with existing semantics, the evidence required and maintenance cost.
Ask what can be simplified or removed while preserving the capability. These
questions should produce bounded outcomes rather than more prerequisites.

<a id="now-expanded-holistic-and-architectural-gap-assessment"></a>

### Completed: expanded holistic and architectural gap assessment

<a id="rm-19"></a>

**RM#19: Architecture and adoption assessment** · [GI#85: Architecture and adoption assessment](https://github.com/sproates/panackelty/issues/85).

Work record: [issue #85](https://github.com/sproates/panackelty/issues/85).
The [assessment report](docs/ADOPTION_ASSESSMENT.md) was merged in PR #87; issue
#85 is closed. Its implementation ordering remained provisional pending the pilot. Evidence covers host integration,
execution lifecycle, composition, developer/agent assistance, platform/library
gaps, resources, performance, distribution, security and maintainability.

- [x] Assess execution targets, embedding/interoperability, concurrency and
      cancellation, memory/resource management, modules/namespaces/packages,
      networking, GUI/platform integration and development tooling against the
      intended application range; identify decisions that could constrain it
- [x] Review correctness, security, testing/measurement, performance, libraries,
      installation/releases, maintainability, documentation and adoption;
      include repository settings/tooling and human-readable code refactoring
- [x] Distinguish observed defects, verified omissions, deliberate limitations,
      stale documentation and hypotheses, citing current repository evidence
- [x] For each finding, record value, affected users/agents, effort including
      tests/docs/maintenance, risk, dependencies, confidence and a smallest
      useful PR or investigation; compare useful assistance with infrastructure
- [x] Reconcile stale roadmap states, record remaining documentation drift
      for scoped follow-up, and apply the repository-led
      [Issues workflow](docs/ROADMAP_PROCESS.md#github-issues-workflow)
- [x] Present concise findings and a provisional Now / Next / Later comparison
      in chat; identify prerequisite decisions and deliberate deferrals

Acceptance: evidence-backed assessment across the intended application range,
with effort and architectural uncertainty explicit. It does not promise all
platforms or authorise implementation of every identified gap.

<a id="next-bounded-ai-assisted-development-experiment"></a>

### Completed: AI-assisted delivery pilot

<a id="rm-20"></a>

**RM#20: AI delivery pilot** · [GI#86: AI delivery pilot](https://github.com/sproates/panackelty/issues/86).

Work record: [issue #86](https://github.com/sproates/panackelty/issues/86).
The [pilot report and evidence](docs/AGENT_DELIVERY_PILOT.md) record 12 fresh-context
trials: Panackelty passed four of six initial and maintained tasks, with HTTP
blocked twice; Python passed six of six. Token usage/cost were unavailable.
The bounded delivery arm and report review are complete; PR #88 is merged.
The free-choice arm, TypeScript comparison and human onboarding were not run.

- [x] Select a few small representative application/change tasks and define
      independent acceptance tests before running agents; include a later
      maintenance change and human readability review
- [x] Compare Panackelty with Python or JavaScript/TypeScript on Node.js using
      recorded model/tool versions, equivalent resource budgets, task briefs
      and access to documentation; record language-specific setup differences
- [x] Measure correctness, elapsed time, cost where observable, repair attempts,
      human intervention, setup/library failures and subsequent-change success
- [x] Preserve prompts, revisions, commands and results sufficiently to reproduce
      the experiment; disclose model familiarity, run variation and sample limits
- [x] Include unsupported tasks and failures; separate missing capabilities from
      language/tooling friction, and do not generalise a small study to all models
- [x] Review results alongside the assessment and agree the first implementation
      milestone with the user before scheduling feature work

Acceptance: a reproducible feasibility report, limitations and proposed priority
changes, not a predetermined win or a claim of mainstream adoption. Bound the
experiment scope and budget before execution; do not introduce a Python runtime
dependency into Panackelty's own development or validation workflow.

### Completed: sorting and literal suffix helpers

<a id="rm-21"></a>

**RM#21: Sorting and suffix helpers** · [GI#89: Sorting and suffix helpers](https://github.com/sproates/panackelty/issues/89).

Work record: [issue #89](https://github.com/sproates/panackelty/issues/89).
State: Done. PR #90 merged after all 21 CI jobs passed, including the three
sanitizer partitions and Linux/macOS packages; issue #89 is closed.

The user selected this S–M item on 2026-09-29 because both ledger trials wrote
sorting helpers and both inventory trials wrote suffix helpers. It offers a
small, directly evidenced reduction in application code before broader host
architecture work. Additive portable APIs keep compatibility risk low.

Scope: stable generic `array_sort_by` with a pure strict-order comparator and
literal, case-sensitive `text_ends_with`; tests, contracts and an executable
example. No syntax, VM, namespace, networking or diagnostics changes. Acceptance
requires `make check`, comparator/Unicode edge cases, unchanged input and equal-key
order, and replay of the four affected maintained pilot applications after
replacing their local helpers. Private pilot sources remain private.

Focused evidence: 24 direct library assertions and three comparator type/effect
contracts pass. Both maintained inventory programs pass 9/9 original checks
after suffix replacement; both ledgers pass 14/14 after sort replacement
(46 checks total, unchanged evaluator). The collections case passes source,
compile and saved-bytecode execution. The new stdlib artifact matches the
unmodified historical compiler; existing disassembled functions are unchanged.
Canonical `make check` passed in 148s (unit 102s), including bootstrap and
packaged quick-start validation. The existing timing warnings remain recorded
in the non-blocking performance backlog. Local sanitizer execution was blocked
by LeakSanitizer being unable to inspect process threads in this workspace;
the full sanitizer CI checks subsequently passed before merge.

### Completed: execution, concurrency and host integration design

<a id="rm-22"></a>

**RM#22: Execution architecture design** · [GI#91: Execution architecture design](https://github.com/sproates/panackelty/issues/91).

Work record: [issue #91](https://github.com/sproates/panackelty/issues/91).
State: Done. The [design report](docs/EXECUTION_CONCURRENCY_DESIGN.md) merged in
PR #92 after all 21 CI jobs passed; issue #91 is closed. The report remains a
proposal for later stages rather than a frozen public language contract.

Reason: networking was absent in both HTTP pilot trials, but the VM's synchronous
execution and host ownership make a concurrency decision a prerequisite to
coherent public networking APIs. Compare blocking calls, callback/event loops,
cooperative tasks with explicit suspension, and threads against server, daemon,
GUI, game, browser and mobile workflows. Weigh effort, safety and readable code
for developers and AI agents rather than selecting syntax by familiarity alone.

The proposal recommends one VM-owning thread, resumable cooperative tasks,
structured task/resource lifetime and host-driven event pumping, with an eventual
explicit-await interface and named callbacks for event adapters. This is not an
accepted model. The user subsequently approved the fake-host resumable-execution
feasibility step below; sockets and public async syntax remain unapproved.

Acceptance for this M-sized design investigation: source-backed alternatives,
clearly hypothetical server/GUI examples, cancellation/error/cleanup/backpressure
contracts, compatibility constraints, staged PRs with failure-oriented gates,
unresolved decisions and concise chat review. Implementation is likely L across
multiple PRs; no toolkit, network backend, ABI change or delivery date is selected.
Report review and merge are complete.
Design-PR validation: `make check` passed in 148s (unit 102s); documentation
links and whitespace checks passed. These are regression checks, not evidence
that the proposed scheduler or platform adapters have been implemented.

### Completed: resumable VM execution feasibility

<a id="rm-23"></a>

**RM#23: Resumable VM feasibility** · [GI#93: Resumable VM feasibility](https://github.com/sproates/panackelty/issues/93).

Work record: [issue #93](https://github.com/sproates/panackelty/issues/93).
State: Done. PR #94 merged after all 21 hosted checks passed, including sanitizer
partitions and Linux/macOS packaging; issue #93 is closed.

Replace recursive native calls with owned VM frames; expose an internal budgeted
advance handle, explicit terminal outcomes and safe destruction. Use a fake,
immediate host adapter to verify re-entry, exit and unsupported service policy.
Keep the synchronous CLI, bytecode v8 and bootstrap behavior compatible. The
experimental host-controlled profile rejects nested execution rather than
allowing it to bypass budgets. No task scheduler, pending I/O, sockets, threads,
public embedder ABI or async source syntax is included.

Effort M–L, with ownership and performance as the main risks. Preserve the
baseline VM before edits. Acceptance is required **before PR review/merge**:
forced yields across calls/iterators, interleaved sessions, terminal/lifetime
contracts, allocation-failure cleanup, public CLI recursion, full validation and
paired performance measurements. Investigate median overhead above 10% on fixed
workloads; this is an investigation threshold, not a universal latency promise.
Record all regressions and sanitizer/environment limitations. Subsequent task
scheduling requires a separate decision based on this evidence.

Implementation evidence (PR #94 merged; hosted gates passed): the owned-frame API,
fake-host policy, forced-yield contracts and 20,000-call CLI regression are in
place. Canonical `make check` passed in 150s (unit 105s) after a clean rebuild
with native prerequisites prepared first. The existing timing warnings remain
non-blocking. Paired CLI median changes are −0.24% for recursive calls, +1.74%
for indirect iteration and +0.50% for compiler compilation. See the
[full feasibility report](tests/VALIDATION_PROFILE.md#resumable-vm-feasibility--2026-09-29)
for samples, frequent-yield overhead and local sanitizer/permission limitations.
The subsequent milestone was separately authorised in the decision below.

<a id="now-task-and-lifecycle-feasibility"></a>

### Completed: task and lifecycle feasibility

<a id="rm-24"></a>

**RM#24: Task lifecycle feasibility** · [GI#95: Task lifecycle feasibility](https://github.com/sproates/panackelty/issues/95).

Work record: [issue #95](https://github.com/sproates/panackelty/issues/95).
State: Done. PR #96 merged with explicit approval on 2026-09-29 after all 21
hosted checks passed, including sanitizer partitions and Linux/macOS packages;
issue #95 is closed.

Decision: prioritise enabling network services and host-driven applications. Both
HTTP pilot attempts were blocked by missing capability; targeted compiler/API
assistance remains the strongest smaller alternative because it improves tasks
that already succeed. Source mapping/coverage, namespaces, focused quality work
and validation speed remain candidates, not an automatically scheduled sequence.
Reassess after this gate before approving further runtime infrastructure.

Scope (M–L): an internal, single-thread task session, parent/child joins,
round-robin instruction budgets, inherited virtual monotonic deadlines, fixed
fake pending print acknowledgements, bounded task/event storage, and qualified
operation identities. This experiment reserves task slots until session destruction;
it does not claim a reusable production scheduler or service backend. No sockets,
threads, source syntax, bytecode changes, public ABI or user async finalisers.

Acceptance: forced bytecode waits; parent/child and nested joins; cancellation
before/during waits and joins; queued completion/cancel order; late, duplicate,
stale and wrong-session completions; capacity rejection and retry; independent
progress; failure propagation; exactly-once pending-request cleanup; destruction
with work pending; allocation sweeps, unchanged CLI/bootstrap, canonical
`make check` and sanitizer gates. Record timing and limitations before merge
review. The [internal contract](src/vm/README.md#internal-task-lifecycle-experiment)
defines observable ordering and bounds. Later syntax/effects and native transport
work require separate decisions.

Evidence: direct lifecycle contracts, 177 VM probe assertions and 1,844 native
allocation-failure injections pass, together with local VM/oracle/runner sanitizer
suites. A clean validation sample took 103s (unit 69s); the existing unit-budget
warning remains non-blocking. The [feasibility record](tests/VALIDATION_PROFILE.md#tasklifecycle-feasibility--2026-09-29)
records reproduction commands, evidence scope and limitations. Final clean
validation passed in 103s (unit 68s). The bounded fake-host investigation is
complete; production concurrency support remains separate.

### Async programming interface: proposal for review

<a id="rm-25"></a>

**RM#25: Async interface design** · [GI#97: Async interface design](https://github.com/sproates/panackelty/issues/97).

Work record: [issue #97](https://github.com/sproates/panackelty/issues/97).
State: Done. The investigation report merged in PR #98 with explicit approval
after all 21 hosted checks passed; issue #97 is closed. Its implementation
recommendation is ready for prioritisation, not scheduled. The user authorised
the investigation on 2026-09-29 after PR #96 merged and alternatives were discussed.

The [proposal](docs/ASYNC_PROGRAMMING_DESIGN.md) compares a finite 16-client
request/reply server in callback and explicit-await forms, with a UI refresh/close
cross-check. It recommends explicit async activation, named owning scopes and a
restricted first callable/effect contract. Scope-owned results and resources,
shutdown, cancellation races and negative compiler/runtime examples are explicit.
All code is hypothetical. No language, VM, bytecode or ABI contract changes here.

Reason: define how someone writes the intended application before committing to a
public concurrency interface. A native TCP/timer C-harness spike would test real
backend cleanup sooner; targeted compiler assistance would benefit existing
programs sooner. Both remain credible alternatives. Prefer the source interface
investigation for this decision, while retaining a real-backend gate before
freezing resource/cancellation semantics. Effort M for design; a minimal awaited
source-to-VM fake-service slice is estimated M–L, full scope/resource delivery L.

Findings: prefer await for sequential application I/O and named callbacks at host
event boundaries. Reject bare async calls and ordinary impure calls from async
bodies initially. Expected per-client cancellation needs an explicit collect
policy; the shipped prototype instead fails the parent when a child independently
cancels. Resource escape checks, that policy, source syntax and typed async
completions are proposals, not tested capabilities. A new bytecode version and
coordinated compiler/verifier/bootstrap migration are required for public async.

Acceptance: equivalent bounded server flows, activation/type/effect and ownership
rules, failure/shutdown behaviour, a source-gap assessment, a staged compatibility
plan, negative examples and a bounded implementation recommendation. The report
provides those outcomes for review; documentation checks and canonical validation
cover repository regressions, not execution of the invented examples. No usability
trial or native networking experiment is claimed. Merge adopts an investigation
report, not automatic permission to implement its recommendation.

### Completed: bounded source-to-VM async/await slice

<a id="rm-26"></a>

**RM#26: Async source integration** · [GI#102: Async source integration](https://github.com/sproates/panackelty/issues/102).

Work record: [issue #102](https://github.com/sproates/panackelty/issues/102).
Selected on 2026-09-29 after comparing real TCP/timer investigation, targeted
compiler assistance and website assessment. M–L, estimated one cohesive PR (two
only if migration can be independently delivered). This prioritises proving the
application interface through the compiler/runtime; real backend cleanup remains
a required later gate before freezing resource/cancellation semantics.

Scope: async main returning Unit, direct/indirect awaited helpers, typed fixed
fake reads, source and bytecode effect checks, cancellation, v9 migration and seed
refresh. No networking, spawn, resources or stable ABI. Acceptance before review:
meaningful source/CLI and forged-runtime tests, success/error and wrong-completion
cases, cancellation races, independent progress, allocation-failure cleanup,
canonical validation, sanitizer gates and paired performance evidence.

Delivered in [PR #103](https://github.com/sproates/panackelty/pull/103), merged
at `e71449f` on 2026-09-29. All 21 Check jobs and both profiling jobs passed,
including the hosted sanitizer gates. The final local clean checks took 360s
and 358s. A harness isolation fix prevents corrupt-seed testing from relinking
the shared VM while compiler checks run. Real networking, spawning and resource
scopes remain outside this delivered slice.

### Completed: bounded validation performance investigation

<a id="rm-27"></a>

**RM#27: Validation overlap investigation** · [GI#104: Validation overlap investigation](https://github.com/sproates/panackelty/issues/104).

Work record: [issue #104](https://github.com/sproates/panackelty/issues/104).
Approved on 2026-09-29 after the async slice: establish a same-host clean baseline,
identify dominant costs and make a bounded improvement only where measurements
support it. Medium, estimated one or two PRs. The developer's repeated six-minute
wait takes priority over starting native TCP/timer or compiler-assistance work.

The controlled Linux comparison passed at 341s before async (`30b584a`) and 362s
after (`e71449f`): 21s / 6.2% longer, with most cost already present beforehand.
The delivered bounded change overlaps independent harness/compiler suites within
the existing worker budget, using the pair already exercised by CI. Every suite,
bootstrap proof and timing warning remains required. See the
[measurement report](tests/VALIDATION_PROFILE.md#clean-validation-comparison--2026-09-29)
for phase evidence, method and limits. Further compiler/runtime optimisation
requires separate evidence and scope; this investigation does not promise 120s.

The bounded overlap candidate passed in 346s (unit 224s), saving 16s / 4.4%
against merged async in one clean sample, with all 1,390 PASS observations
retained. The 120s full-check and 15s unit targets remain unmet; bootstrap also
warned at 61s against 60s. PR #105 merged at `dd7306b` with explicit approval on
2026-09-30 (Europe/Gibraltar), after all 21 hosted Check jobs passed; issue #104
is closed. The timing budgets remain open goals, not completed by this merge.

### Incremental and modular builds

<a id="rm-28"></a>

**RM#28: Incremental and modular builds** · [GI#106: Incremental and modular builds](https://github.com/sproates/panackelty/issues/106).

<a id="future-candidate-incremental-and-modular-builds"></a>

Work record: [issue #106](https://github.com/sproates/panackelty/issues/106).
The VM/compiler boundary audit completed in PR #109. The bounded validation
slice is **Done**, effective on its delivery PR merge; see the
[completion record](#completed-modular-validation-and-component-boundaries).
Shared local/CI selection, the audited process route, explicit component
ownership, conservative integration selection and failure regressions are
implemented. The supported native/Panackelty/POSIX tooling, canonical full
checks, bootstrap, sanitizer, release and stable required-check contracts remain.

The reproducible validation/build baseline is **Done on this delivery's merge**
within [RM#123: Standing performance engineering](#rm-123). The bounded POSIX
[harness](tests/README.md#reproducible-build-and-validation-baseline) retains an
exact isolated candidate archive, environment/seed/VM identities and repeated
clean/full, unchanged warm/focused and unrelated/direct/transitive edit observations.
All 18 observations pass, with correct outputs and explicit compile/execution counts;
the [measured evidence and reviewed disposition](tests/VALIDATION_PROFILE.md#reproducible-build-baseline)
retain full/focused budget misses. No post-merge acceptance is needed for this
measurement slice; the broader GI#106 remains open. Website impact: none, because
this changes contributor measurement tooling, not public language/release claims.
Dependency-aware probe reuse and separate-compilation design remain unscheduled;
establishing the baseline authorises neither optimisation nor a compiler redesign.
Completing the earlier slice does not close the broader issue. The existing broad
invalidation concern follows.

Problem: `tests/run_probe.sh` fingerprints every Panackelty source under `src`,
`tests`, `examples` and the selected standard library for each compiled probe.
An unrelated edit can therefore invalidate otherwise reusable compiled tests.
Native C objects already build incrementally and focused checks exist, but the
Panackelty loader combines reachable modules before checking/emission. Separate
source files do not yet provide independently compiled module artifacts.

Later cache/design slices, separately selected:

1. The bounded baseline above completes the first measurement step. Attribute
   the representative critical path before selecting further optimisation;
   compare any candidate against the same clean/warm/edit workloads and preserve
   all assertions. Tiny-fixture rebuild cost does not predict whole-suite savings.
2. Deliver dependency-aware compiled-probe reuse if justified, using canonical
   import resolution and retaining compiler/seed/VM identity and other inputs.
   Unknown dependencies must conservatively invalidate reuse. Tests must prove
   unrelated edits reuse artifacts, relevant edits rebuild, and corruption,
   graph changes, relocation, concurrent publication and input races remain safe.
   Every invocation still executes the tests; never cache their outcomes.
3. Produce a separate-compilation design comparing reusable frontend work with
   compiled module artifacts/interfaces and linking. Address names, generics,
   guarded types, purity/async effects, compatibility, deterministic bytecode and
   bootstrap. Recommend a bounded future prototype, or a reason not to proceed.

Acceptance: reproducible before/after evidence, sound invalidation tests,
unchanged validation coverage/budgets, canonical `make check` and relevant hosted
gates for implementation changes, and a reviewed design with explicit open
decisions. Stop at an evidence-backed recommendation if caching has negligible
benefit or disproportionate cost. No linker, namespace/export syntax, package
manager, stable module ABI or two-minute cold-check promise is included.

Later cache/design estimate: medium, **2–3 PRs** including tests and documentation:
one or two for dependency evidence and safe reuse (depending on whether compiler
dependency reporting/seed refresh is independently deliverable), then one design
PR. Full separate compilation is large and must be re-estimated after design.

Initial priority comparison — historical recommendation before website selection:

| Candidate / bounded outcome | Why worthwhile and relative priority | Size / estimated PRs |
| --- | --- | --- |
| Dependency-aware reuse and modular-build design (#106) | Proposed Next: observed rebuild friction affects work across the project; bound the effort before committing to a compiler redesign | Medium / 2–3 |
| Native TCP/timer feasibility | Strongest alternative: unlocks network applications blocked in the delivery pilot; choose first if a near-term network application becomes the main goal or cache measurements show poor value | Medium–large investigation / 1–2, not production networking |
| Targeted compiler/API assistance | Addresses observed interpolation, numeric-proof and discovery friction; valuable but narrower than the current cross-project rebuild issue | Medium for one selected pain point / 1–2 |
| Website assessment (#99) | Useful for onboarding, but concrete gaps remain unaudited and it does not shorten current development loops | Small assessment / 1; implementation separately estimated |

Known correctness/security defects take precedence on risk. Reassess after the
bounded build task; do not put networking or inexpensive useful features behind
the whole modular-build programme. The goal is to rebuild the changed part and
its dependents, not every unrelated part, while retaining full final validation.

### Completed: VM/compiler boundary audit

<a id="rm-29"></a>

**RM#29: VM compiler boundary audit**.

The user selected this bounded investigation on 2026-09-30, within
[issue #106](https://github.com/sproates/panackelty/issues/106), before choosing
browser or build implementation. The [audit evidence](ARCHITECTURE.md#vmcompiler-boundary-audit--2026-09-30)
shows that the VM already builds and passes native tests and 145 fixed bytecode
cases without compiler source, seed or stdlib. Launcher/package prerequisites
and broader test orchestration are the remaining practical coupling.

<a id="rm-105"></a>

**RM#105: Runtime-only packaging** (unscheduled follow-up).

No compiler/VM redesign is recommended. A runtime-only packaging/test entry point
could be a small-to-medium one-PR slice; browser feasibility can proceed without
waiting for it, but must address POSIX host integration. PR #109 merged with
explicit approval after its documentation checks passed. Browser feasibility
was subsequently selected below; runtime-only packaging remains unscheduled.
The audit does not complete #106's caching or separate-compilation work.

### Completed: browser-playground feasibility

<a id="rm-30"></a>

**RM#30: Browser playground feasibility** · [GI#110: Browser playground feasibility](https://github.com/sproates/panackelty/issues/110).

Work record: [issue #110](https://github.com/sproates/panackelty/issues/110).
State: Done. PR #111 merged with explicit approval and successful documentation
checks at `875b567`; issue #110 is closed.
Authorised on 2026-09-30 after the boundary audit. Medium, one investigation PR;
production delivery remains unscheduled.

Prove a WebAssembly VM can run existing compiler bytecode, compile editable
complete programs and display their output and diagnostics. Investigate host
restrictions, cancellation, memory/output limits, download size and latency.
Deliver a private hosted demonstrator for phone review if feasible, or concrete
reproducible blockers and a bounded recommendation. Preserve verification,
exact values, the native toolchain and its validation requirements.

Reason: a direct way to try the language builds on the completed website and
audit without waiting for infrastructure. Runtime-only packaging is the smaller
alternative but does not establish browser portability; dependency-aware caching
addresses daily build friction but has unmeasured savings. Native TCP/timers
remain the strongest application-enabling alternative if networking becomes the
immediate goal. No persistent REPL, main-site deployment, new bytecode ABI or
production browser-platform commitment is included.

Evidence: the existing compiler seed compiles editable programs on a Wasm VM;
one exact-arithmetic program emits identical native/Wasm bytecode. Of 145 fixed
VM cases, 131 match exactly and 14 deliberately reject unavailable host services
with different diagnostics. Worker cancellation/restart, source/output bounds,
memory growth rejection, diagnostics and stdlib imports pass under Node.
The [architecture report](ARCHITECTURE.md#browser-playground-feasibility--2026-09-30)
and [measurements](tests/VALIDATION_PROFILE.md#browser-playground-feasibility--2026-09-30)
record reproduction, individual samples and limits. The private hosted review
prototype is deployed with a source/evidence download; the public site is unchanged.

Conclusion: promising runtime/compiler feasibility, with explicit remaining
gates. Real browser/iPhone rendering and execution were not tested in this
environment. The isolated Emscripten research SDK uses Python internally, so
production delivery needs a build-policy-compatible route or a separately agreed
policy decision. No new interpreter dependency was added to this repository.
The 256 MiB cap is per Wasm instance, not total browser memory. A persistent REPL,
production host adapter and public-site integration remain separate work.

### Completed: playground delivery preparation

<a id="rm-31"></a>

**RM#31: Playground delivery preparation** · [GI#112: Playground delivery preparation](https://github.com/sproates/panackelty/issues/112).

Work record: [issue #112](https://github.com/sproates/panackelty/issues/112).
State: Done. PR #113 merged on 2026-09-30 at `149d7d5` with explicit approval.
Local `make check` passed in 297s; Check and Playground preparation workflows
passed, including Chromium, Firefox and WebKit. Physical iPhone evidence is separate.
Selected on 2026-09-30 after reviewing the alternatives.
Medium, estimated one preparation PR; public-site integration remains a later
separately reviewed step. Establish a reproducible build without adding a Python
dependency, then verify loading, compilation, execution, diagnostics, limits and
cancellation in real browser engines where an authorised environment permits.
Distinguish automated WebKit evidence from physical iPhone/Safari review.

Reason: close the concrete delivery gaps from #111 before changing the public
website. Runtime-only packaging is smaller but not a prerequisite; caching has
unmeasured incremental benefit; native TCP/timers remain the strongest alternative
if networking applications become the immediate goal. Preserve native builds,
bytecode verification and exact values. No persistent REPL, public-site deployment,
policy relaxation or broad host-platform rewrite is included. Stop and re-estimate
if a compliant build requires disproportionate host/toolchain work.

The WASI SDK route now builds the existing VM with its native Clang/linker and
an SDK-only subprocess PATH, without the Emscripten tooling. Maintained build,
host, worker and lifecycle sources initially lived under `src/playground`; they
now belong to the independent browser repository. Local compatibility and limit
tests pass; the separate browser workflow passed on the merged PR head. Native
`make check`, browser CI and physical iPhone evidence must be reported distinctly.
The existing clean-check/unit timing-budget backlog remains open; this browser
task does not change those targets or claim improved native validation speed.

### Completed: website playground integration

<a id="rm-32"></a>

**RM#32: Website playground integration** · [GI#114: Website playground integration](https://github.com/sproates/panackelty/issues/114).

Work record: [issue #114](https://github.com/sproates/panackelty/issues/114).
State: Done. Delivered in PR #115; issue #114 is closed. The user subsequently
tried the live playground and requested the core-library ergonomics work below.
Authorised on 2026-09-30 after PR #113.
The user reviewed the private preview successfully on their phone and requested
expanded examples before merge. Nine examples now include explanations, expected
output and suggested edits; the expanded implementation is merged.
Medium, estimated 1–2 PRs for the page, deployment integration, tests and review.
Deliver an editable `/playground/` page with selectable examples, Run/Stop,
output and diagnostics, using the existing site design and coordinated Pages/
coverage publication. Supply an iPhone-accessible private preview before merge.

Reason: the build/browser prerequisites now pass; complete the useful website
experience ahead of further infrastructure. Dependency-aware caching (#106) is
the strongest alternative if development latency becomes the immediate priority.
Native networking remains valuable but requires further design and investigation.

Acceptance: pinned build; actual browser example/navigation/error/cancellation
tests; complete artifact assembly and failure handling; canonical validation;
phone review reported separately; live artifact verification after approved merge.
No persistent REPL, new host capabilities, network backend or bytecode change.
Issue #114 is closed following delivery. The existing validation budget warnings
remain applicable; no speed improvement is claimed.

### Completed: content-led website expansion

<a id="rm-33"></a>

**RM#33: Website content expansion** · [GI#99: Website content expansion](https://github.com/sproates/panackelty/issues/99).

Work record: [issue #99](https://github.com/sproates/panackelty/issues/99).
Selected and authorised on 2026-09-30 ahead of further build infrastructure:
useful public-facing work must not wait indefinitely behind infrastructure.
Scope: current capabilities with examples; released versus unreleased versus
proposed status; planned direction and vision; accessible explanations of the
self-hosted compiler, reproducible bootstrap, runtime and testing evidence;
on-site getting started; mobile navigation and preserved coverage access.
Keep the established visual direction and existing Pages publication path.

Acceptance: claims grounded in source/specification and the release boundary;
displayed programs checked through the public CLI and saved bytecode, with the
release-labelled greeting checked against alpha.9; working local links/anchors;
desktop/mobile review; Pages regressions and canonical validation. PR #108 merged with explicit user approval at `5b508e7`. All 21 hosted Check
jobs passed, followed by main validation and Pages build/deploy/live verification.
The published homepage exactly matched the merged source; website and coverage
provenance identified the same commit. Issue #99 is closed.

The website branch passed clean canonical validation in 339s on 2026-09-30
(unit 220s), plus Pages assembly/navigation tests and the alpha.9 greeting.
The 120s full-check and 15s unit budgets still warn; the bounded build candidate
above remains the follow-up assessment. Browser permissions blocked rendered
desktop/mobile inspection in this workspace; a private hosted preview was subsequently supplied for iPhone review and
approved by the user before merging.

The subsequent embedded playground shipped in PR #115 after feasibility and
real-browser preparation in PRs #111 and #113. Persistent REPL sessions remain
independently scoped; the completed website work does not select them.

### Remaining findings from the delivery pilot

Core library ergonomics shipped in PR #118, and finite TCP client/server support
shipped in PRs #127 and #130. HTTP framing and indefinite services remain absent;
the blocked HTTP pilot tasks have not been rerun or declared successful.
Interpolation, numeric-proof and API discovery friction feed the current workflow
and compiler-explanation candidates. Compare these against the changed baseline,
not the pilot's earlier implementation recommendations.

### Developer experience: useful assistance from the compiler

<a id="rm-34"></a>

**RM#34: Compiler assistance**.

The [workflow assessment](#development-workflow-assessment) and
[compiler explanations](#compiler-explanations) now give these ideas bounded
first outcomes. Runtime diagnostics have their [own scope](#source-aware-runtime-errors).
The following list remains the broader assistance context.

Make working in Panackelty enjoyable, with substantial attention to helping
programmers understand and develop their programs. This goes beyond polished
error messages. Assess assistance throughout writing, checking, exploring,
changing and debugging code, including potential support for:

- Explaining inferred types, effects and guard/proof obligations, including why
  a program is accepted or rejected and what evidence would satisfy a requirement
- Context-aware suggestions for names, imports and available operations, missing
  match cases, and useful feedback for incomplete code
- Detecting likely mistakes such as unused bindings or unreachable code, with
  focused, low-noise guidance rather than indiscriminate warnings
- Safe, actionable fixes and refactoring assistance whose effects can be checked;
  keep proposed edits reviewable and preserve programmer intent
- Source-aware runtime failures, discoverable compiler queries and structured
  output that CLI/editor tooling can share without duplicating compiler knowledge

These are assessment candidates, not promises of specific features or a required
IDE, language server or REPL. Evaluate usefulness on representative tasks: time
and friction to reach correct code, discoverability, precision, false positives,
feedback latency and consistency. Consider approachable defaults and opt-in depth.

### Backlog navigation

The [current grooming table](#groomed-candidates) owns candidate state and the
[comparison](#proposed-first-step) records the recommendation. Existing namespace,
host/library, build and language explorations remain visible in their sections;
none becomes scheduled through its position in this document. Completed milestones
retain their evidence below and in the historical sections.

### How to turn the backlog into PRs

The [decision process](docs/ROADMAP_PROCESS.md) is authoritative for these rules.

- The assessment and delivery pilot are complete. Choose the next principal
  initiative from the current comparison after reviewing its scope and evidence;
  the placement of a detailed proposal does not assign priority.
- Before implementation, define scope, dependencies, explicit non-goals, failure
  cases and acceptance evidence. A design PR may finish with a decision or a
  bounded feasibility result rather than claiming an implemented feature.
- Close a task only against its stated evidence. Record partial completion
  explicitly and link follow-up work; do not infer completeness from test counts
  or broad checklist wording such as “comprehensive”.
- Update status when a PR ships. Keep performance experiments in
  [the profiling report](tests/VALIDATION_PROFILE.md), behavioral evidence in
  [the coverage matrix](tests/COVERAGE.md), and bootstrap history in
  [the self-hosting record](SELF_HOSTING.md).

## Improve and expand the website

Roadmap item: [RM#33: Website content expansion](#rm-33).

Work record: [issue #99](https://github.com/sproates/panackelty/issues/99).
The [current decision and acceptance](#completed-content-led-website-expansion) above
owns the state and scope. The initial unscheduled idea was recorded on
2026-09-29; the user selected and authorised the content expansion on 2026-09-30.
The existing static GitHub Pages and coverage flow remains in use. PR #108 was approved, merged and live-verified; the issue is closed.

## Developer preview alpha.11 release

<a id="rm-128"></a>

**RM#128: Alpha.11 release** — Done (2026-10-04).
Selected on 2026-10-04. Release delivery owner: maintainer delivering the promotion.
[PR#271: Alpha.11 release preparation](https://github.com/sproates/panackelty/pull/271)
merged as `f1d37b46145b502f7e38e674d0f071934f85166a`. The
[Release workflow](https://github.com/sproates/panackelty/actions/runs/37214140201)
passed validation, Linux/macOS packaging and publication. The
[alpha.11 release](https://github.com/sproates/panackelty/releases/tag/v0.1.0-alpha.11)
was published on 2026-10-04 at 15:51:12 UTC from that exact source commit.

All eight public assets were downloaded. Checksums matched all three archives;
both native provenance records matched the release source. The public macOS
archive passed release smoke testing. This release includes existing native TCP
client/server operations, guard-fact repair, optional source maps, bounded
subtraction/effect explanations, declaration-lookup improvements, measured build
baselines and generated website history. Namespace metadata/binding work remains
internal staged infrastructure, not executable namespace support.

[PR#272: Alpha.11 downloads and installer](https://github.com/sproates/panackelty/pull/272)
merged as `23e20c07397790be0b4b6a450283a15fa57902a7`, promoting native
homepage/README downloads and installer defaults. Independent review, owner
preview approval and public README/installer acceptance on Linux and macOS passed.
[Production Check](https://github.com/sproates/panackelty/actions/runs/37215512240)
and [Pages deployment and live verification](https://github.com/sproates/panackelty/actions/runs/37215665667)
passed for that revision, including byte comparisons of published pages,
playground assets and publication provenance. No dedicated issue.
Browser v0.1.1 remains pinned; RM#102, RM#103 and RM#104 retain their separate
example, documentation and browser-adoption acceptance.

Performance disposition retained through the approved release: retain all safety/coverage
and unchanged 120s full / 15s focused targets. The controlled RM#28 baseline has
clean median 134.42s and unchanged focused median 26.46s, above those targets.
The performance delivery owner retains GI#106: inspect the measured critical path
and evaluate a bounded reuse candidate before the next affected compiler delivery.
The release carries this known development-latency limitation; it makes no runtime
performance adequacy claim and does not reopen deferred runtime benchmarks.

## Developer preview alpha.10 release

<a id="rm-35"></a>

**RM#35: Alpha.10 release**.

State: Done. PR #122 merged and alpha.10 published on 2026-09-30 from
`8cb6b75328aae8f6febf02529e9ac1798c7056b4`. Linux x86-64 and macOS arm64
release gates passed. Both public downloads passed checksum/provenance checks;
the downloaded macOS archive passed quick-start and release smoke tests. Linux
archive execution was verified by the release matrix. The website deployed
successfully with alpha.10 download instructions and migration notes.

The release aligns downloads with the playground's core language APIs and v9
bytecode. Real networking and broader build-cache work remain separate.

## Real async TCP from Panackelty

<a id="rm-36"></a>

**RM#36: Finite TCP client** · [GI#126: Finite TCP client](https://github.com/sproates/panackelty/issues/126).

Work record: [issue #126](https://github.com/sproates/panackelty/issues/126).
State: Done. PR #127 merged on 2026-09-30 after all 23 hosted checks passed.
Canonical validation passed in 117s, with native sanitizer, independent-peer
source/bytecode tests, bootstrap and browser rejection evidence in the PR.
This functionality is included in the published alpha.11 native release; it is
not part of alpha.10.
Issue #126 was closed during the subsequent grooming pass.
Selected on 2026-09-30 after deployment
reliability, explicitly ahead of build-cache work and a C-only networking spike.
The user wants real source programs to benefit from async/await.

Bounded scope: `await tcp_exchange` owns one numeric-IPv4 request/response
connection, half-closes after sending, reads to EOF, and returns bytes/error.
Bound memory and total time; use nonblocking sockets with owner-thread polling
and close on every outcome. Native embedded execution is opt-in; WASI remains
without raw networking. No source handles, spawning, listening, DNS, TLS or HTTP
framing. This is an additive development feature, not an alpha.10 release claim.

Medium–large, one PR including compiler/decoder effects, seed refresh, native
backend, source/bytecode tests, fault/sanitizer evidence and documentation.
Acceptance requires partial/binary/empty I/O, response limits, timeout/refusal,
independent progress, cancellation cleanup, explicit host restrictions,
canonical `make check` and Linux/macOS/browser CI. The finite server subsequently
shipped below; broader server capabilities and build-cache improvements remain
independently scoped follow-ups.

## Bounded async TCP server

<a id="rm-37"></a>

**RM#37: Finite TCP server** · [GI#128: Finite TCP server](https://github.com/sproates/panackelty/issues/128).

Work record: [issue #128](https://github.com/sproates/panackelty/issues/128).
State: Done. Contract PR #129 and implementation PR #130 merged on 2026-09-30;
implementation merge commit `43c09b947ead3e62a7aa3a4cd18b98f1c64d44ee`.
All 23 hosted validation/build/browser checks passed; deployment-only jobs were
skipped for the PR. The implementation records a passing 110s `make check`, native
ASan/UBSan, 16 WASI runtime/asset tests and identical compiler/stdlib bootstrap
fixed points. The existing unit-phase budget warning remains. This completes the
selected finite server stage and allows backlog grooming to proceed. Issue #128
can close when this repository completion summary merges. No release is included.

The [server contract](ARCHITECTURE.md#bounded-tcp-server-contract--proposed-implementation)
defines an awaited finite server owner with named async byte-request handlers,
bounded concurrent admission, EOF request/reply framing, monotonic deadlines and
graceful draining. The runtime owns sockets and handler lifetimes. Expected
client failures are isolated; runtime traps cancel the owner. Listening is a
separate embedded capability; WASI remains unavailable. General source spawning,
resource handles, HTTP, DNS, TLS and indefinite service operation are excluded.

Delivered in two PRs: contract, then source/native implementation. Independent
source and saved-bytecode peers exercise binary, fragmented and empty transfers,
request/response limits, stalled-reader and busy-handler fairness, nested outbound
waits and connection reuse. Tests cover capabilities, forged bytecode, allocation
and descriptor failures, pending-phase shutdown and final-admission cancellation.
The runnable finite echo example and documented limits are included. General
spawning, DNS, TLS, HTTP and indefinite services remain separate proposals.

## Website deployment source selection

<a id="rm-38"></a>

**RM#38: Website source selection** · [GI#123: Website source selection](https://github.com/sproates/panackelty/issues/123).

Work record: [issue #123](https://github.com/sproates/panackelty/issues/123).
State: Done. PR #124 merged after all 23 checks passed. Production Pages run
36723094458 passed build, deployment and live verification; public provenance
reported merge commit `cdff3ef04ca40952934b6f3e5768306ce9474ca5`.
Issue #123 was closed during the subsequent grooming pass.
Selected on 2026-09-30 after alpha.10 release.
An earlier Pages attempt selected historical content and failed on a missing
playground SDK installer; retry succeeded. The original API response was not
retained, so the upstream cause is unproven. The selector's reliance on the first
successful result, without checking current main, is reproducible with stale
or unordered history.

Pin main at selection time and require a trusted successful Check for that exact
commit. Select coverage by Check run number, retaining earlier coverage for
documentation-only changes. Missing validation, API errors and missing/expired
coverage stop publication and preserve the live site. This deliberately waits
when current main is pending or failed, rather than publishing an older commit.

Small–medium, one PR including regression tests and documentation. Acceptance:
focused source-selection tests, canonical `make check`, hosted Pages validation
and, after separately approved merge, live source-provenance verification.
This observed delivery failure takes priority over build-cache measurement and
TCP/timer discovery; no compiler, website appearance or cache changes are needed.

## Playground deployment cache consistency

<a id="rm-39"></a>

**RM#39: Playground cache consistency** · [GI#119: Playground cache consistency](https://github.com/sproates/panackelty/issues/119).

Work record: [issue #119](https://github.com/sproates/panackelty/issues/119).
State: Done. PR #120 merged and deployed on 2026-09-30. All 23 hosted checks
passed; live reload loaded versioned assets and the import-free text example
ran successfully. Selected on 2026-09-30 after the browser kept
an obsolete imported text example following the core-method deployment.

Version the entire playground asset set together and test normal reload from a
warm HTTP cache across deployments. Acceptance includes deterministic identities,
new example/library execution in all three browser projects, publishing checks,
canonical validation and live verification after separately approved merge.
Cached entry HTML and already open tabs are not automatically refreshed; missing
old assets must fail visibly rather than silently mixing versions.

Small, one implementation PR. This observed onboarding defect takes priority over
the separate alpha.10 release, dependency-aware probe reuse and TCP/timer discovery.
No language, bytecode or downloadable release-version change is included.

## Core types and discoverable text/collection methods

<a id="rm-40"></a>

**RM#40: Core types and methods** · [GI#116: Core types and methods](https://github.com/sproates/panackelty/issues/116).

Work record: [issue #116](https://github.com/sproates/panackelty/issues/116).
State: Done. Completed in PR #118, merged and deployed on 2026-09-30.
Canonical checks, browser suites and live import-free core/method execution passed;
issue #116 is closed. The cached-example defect discovered afterwards is tracked
separately below. The following scope records the accepted implementation.
Recorded and groomed on 2026-09-30 following hands-on website playground feedback.
The user selected implementation on 2026-09-30 as one cohesive PR covering core
types, text/collection methods and migration. The user approved the merge.

`Option[T]` and `Result[T,E]` should be usable without stdlib imports, and
ordinary text/collection operations should be discoverable as methods on values
of the appropriate type. Existing import-free `.starts_with()` and `.reverse()`
make the imported `text_ends_with` helper inconsistent. The proposed equivalent
is `print("hello.panack".ends_with(".panack"))`, preserving the literal suffix.
Most current dot calls still resolve global functions; the proposal must settle
receiver-type lookup rather than merely shorten prefixed function names.

- [x] Define minimal implicit availability for Option/Result and their
      constructors, including canonical definitions, removal of obsolete imports and name
      collisions; decide helper exposure separately from core types
- [x] Inventory text and collection APIs and select concise, type-appropriate
      methods, beginning with `Str.ends_with`; cover literals, variables and
      chaining, global receiver-first calls, fields, generics, purity and useful
      wrong-receiver/unknown-method diagnostics
- [x] Apply the agreed breaking preview migration for existing helpers,
      imports and user names; coordinate with the namespace proposals below
      without assuming full namespaces must ship first
- [x] Deliver independently scoped slices with meaningful compiler and public-CLI
      tests, source/saved-bytecode parity, installed-package/bootstrap checks,
      browser integration and updated language/library examples and contracts

Value: less import ceremony and a more consistent API for developers and coding
agents. Estimated M for core availability and M–L for methods and migration,
including tests, docs and integration; one cohesive implementation PR, as selected by the user.
Resolve the bounded design decisions within the relevant PR rather than requiring
a separate design report up front. Confidence in the friction is high; lookup rules and
migration cost need assessment. Risks are name capture, duplicate definitions,
inference/effect regressions and differences between native and browser builds.
Delay prolongs learning friction rather than a known correctness defect.

Acceptance includes import-free construction and matching of both core types,
migration of existing callers, name-resolution failures, unchanged purity and
persistent collection semantics, suffix edge cases and real playground usage.
Implementation must pass `make check` and relevant browser checks. Keep one
coherent contract across the compiler, stdlib, packages and browser assets.
Full namespaces, classes, inheritance, dynamic dispatch and general user-defined
extension methods are outside this proposal. A minimal core prelude is a candidate
mechanism; implicitly importing the entire stdlib is not the proposed outcome.

### Historical scope and recommendation before PR #118

This comparison records the recommendation before PR #118 shipped. The public
playground had made basic API inconsistencies visible to new users. The rationale
was to reduce migration cost before more libraries and examples depended on the
earlier spellings; this was a reasoned expectation, not a measured adoption result.

| Candidate | Value and trade-off | Size / estimated PRs |
| --- | --- | --- |
| Core types and standard methods (#116) | Recommended Next: direct user feedback, immediate benefit in ordinary programs, and existing method machinery to build on | M–L overall / 1 |
| Dependency-aware probe reuse (#106), excluding separate compilation | Strongest alternative: slow validation affects every change, but savings from narrower invalidation remain unmeasured | M / 1–2 |
| Native TCP/timer feasibility | Enables applications blocked in the pilot, but carries greater lifecycle uncertainty and does not resolve basic API friction | M–L investigation / 1–2 |

Choose caching first if measurements show iteration cost obstructs this work;
choose transport feasibility first if a concrete network application becomes the
immediate objective. Full namespaces and separate compilation remain broader
follow-ups, not prerequisites for this bounded initiative.

Delivery scope, combined in one implementation PR:

1. **Core availability (M component).** Provide `Option[T]`, `Result[T,E]`, `None`,
   `Some`, `Ok` and `Error` by default. Prefer one source-defined core loaded once
   through the existing loader; settle behaviour when the stdlib root is missing
   or overridden. Remove obsolete core imports from maintained sources. Conflicting user
   declarations should produce an explicit diagnostic rather than silent
   shadowing. Keep value-or helpers and host/testing modules outside the implicit
   public surface. Verify annotations, inference and pattern matching without
   imports, nested imports and collision failures. Preserve existing inference
   limits; this slice does not promise inference for an unconstrained `None()`.
2. **Text methods (M component).** Make `len`, `slice`, `starts_with`,
   `starts_with_at`, `ends_with`, `reverse`, `is_digit`, `is_letter`,
   `is_whitespace` and `parse_nat` available on `Str` without imports. Preserve
   existing Unicode/code-point, ASCII classification and parse-failure contracts.
   Use receiver-type lookup for standard method names; an unrelated global
   function must not capture a standard method call. Preserve current user
   receiver-first calls for names outside the standard method set. Define the
   reserved-name boundary and wrong-receiver behaviour explicitly, including
   names shared by supported types such as `len`. Reuse library algorithms and
   existing runtime primitives where possible; a new VM opcode is not assumed.
3. **Collection methods and migration (M component).** Provide import-free array
   `first()` and `sort_by(comparator)` alongside `len`, `append`, `concat`, `map`
   and `reduce`; retain Map `put`/`has`/`get` and Set `add`/`has`, and make their
   existing operations consistent with the same receiver lookup rules. Map/Set
   length is not currently supported and is not added in this change.
   Preserve stable sorting, pure callback requirements and immutable updates.
   Keep current missing-key behaviour; safe optional Map lookup is separate.
   Inventory Bytes explicitly and defer new byte-buffer method names to a
   follow-up rather than leaving its coverage ambiguous. Migrate public examples
   and playground lessons to the preferred spellings.

Migration decision, 2026-09-30: the user reports no external Panackelty authors,
so preserving obsolete imports and prefixed functions is not a delivery
requirement. Prefer a clean breaking preview migration. Update compiler and
stdlib sources, tests, examples, packages and playground lessons together in
each affected slice; remove superseded public wrappers and redundant import
modules once their remaining responsibilities have been accounted for. Do not
remove useful helper behaviour merely because its old module also defined a
now-implicit type. Record source/API changes in the changelog under the existing
preview release policy; this is not a compatibility-preserving patch release.

Retain an old spelling only where the reproducible bootstrap demonstrably needs
it, with the exact dependency, limited scope and removal condition documented.
Do not assume a historic compiler requires public aliases: prove the requirement
and prefer isolated bootstrap staging. Acceptance includes a reference audit for
obsolete calls/imports and a passing bootstrap after migration. Unrelated stdlib
imports and general user-defined receiver-first functions remain in scope only
where affected by the new lookup contract. New method names must not become new
unqualified global functions. Do not promise editor completion in
this item: discoverability means a consistent documented type API and relevant
diagnostics. General extension methods, new namespaces and Option/Result helper
methods remain separate scope.

Technical evidence: `loader.panack` already deduplicates resolved module paths;
`parser.panack` marks selected collection methods for checker resolution, but
ordinary dot calls currently lose their method identity. The implementation must
retain enough identity for the proposed lookup rules. The browser runtime
already supplies a stdlib root and runs the same compiler bytecode, so implement
this in the shared compiler/library path, without browser-only source rewriting.
An implicit prelude must not leak extra helper declarations simply because the
current option/result source files contain them alongside their enums.

The implementation PR needs unit and public-CLI positive/negative cases, canonical
`make check`, bootstrap/package evidence and rebuilt playground assets with
relevant browser tests. Compare compilation latency and asset size before and
after implicit loading; investigate material regressions rather than assuming
all-module loading is free. Final acceptance includes running import-free
Option/Result examples, the exact suffix example above and array first/sort
examples in the published playground after separately approved merge/deployment.
Implementation and live acceptance are complete. The release version remains unchanged during implementation; prepare the next
alpha (currently expected `0.1.0-alpha.10`) and breaking-change notes in a separate
release PR. The bytecode format stays v9.

## Language namespaces — idea

<a id="rm-41"></a>

**RM#41: Language namespaces** · [GI#198: Language namespaces](https://github.com/sproates/panackelty/issues/198).

Work record: [#198](https://github.com/sproates/panackelty/issues/198).
Current state: **In progress / Now** as P2 in the [programme register](#rm-108),
explicitly selected on 2026-10-04. P1 supplies the accepted
[shared design](docs/MODULE_PACKAGE_DESIGN.md). The following assessment
originated on 2026-10-01 and does not override that production scope. It asks
how code in one file references functions and types in another. Existing quoted
file-relative and logical imports already work; declarations currently share one
program namespace. This extends the namespace idea, not completed import work.

**Intermediate foundation — merged in PR#261 as `a5cdb1a`:** retain per-file import,
visibility and declaration-span metadata; tagged standalone/toolchain module and
declaration identities; local binding/export inventories and collision/private
lookup diagnostics; attach existing resolver conflict diagnostics to their owning
declarations. Public `pub`, alias and selective-import forms fail closed while the
checked/emitted AST still uses current names. There is no executable namespace
mode or permanent compatibility switch. Unit and public-CLI fixtures cover the
staged boundary and repeated cross-module diagnostic origins. The refreshed v9
seed reaches the fresh compiler/library fixed points; 59 namespace assertions,
91 CLI integration assertions and all 343 functional cases pass under canonical
`make check` (126s, unit 91s, functional 5s, bootstrap 20s). The full 120s and unit
15s budgets remain exceeded: retain the prioritized
[GI#106: Validation performance](https://github.com/sproates/panackelty/issues/106)
reminder. See [validation evidence](tests/VALIDATION_PROFILE.md#namespace-modulebinding-foundation--2026-10-04).
GI#198 remains open.

**Intermediate qualified-reference/binding-resolution slice:** raw `DotCallExpr` retains namespace/value ambiguity until binding;
qualified types, function references, constructors and enum patterns parse without
enabling execution. Explicit graph edges resolve namespace/selective imports,
re-exports and nested enum selectors to original declaration identities. The raw
use pass checks private/missing/wrong-kind bindings, namespace values and import
collisions with parameters, type parameters, locals, loops and patterns. Shared
completed-node traversal avoids repeated diamond subtree work. The loader reports
staged graph/use errors while preserving the execution gate, including unmarked
qualified forms. Focused coverage has 153 assertions; the diagnostic functional
case adds 18 public-CLI boundary checks and absent-bytecode evidence. Validation
performance remains tracked under GI#106: after correcting the legacy
basename/value-scope regression, a complete passing run took 216s (unit 173s),
exceeding the 120s full and 15s unit budgets. The gate now retains only candidate
namespace roots in its lexical set. Final canonical validation passes in 211s
(unit 168s, functional 5s, bootstrap 28s), still over budget; the observed timing
change is not an isolated performance attribution. Measurements are recorded in the
[validation profile](tests/VALIDATION_PROFILE.md#namespace-raw-references-and-binding-resolution--2026-10-04).
This is an
intermediate GI#198 delivery, not namespace feature acceptance or a release.

**Merged checked declaration/signature slice — PR#275, `0bcf092`:** tagged core/nominal/callable and
binder-position parameter identities replace spelling-based contracts at this
staged boundary. Alias/re-export reachability preserves original identities;
public fields, enum payloads, parameters, results, nested generic/callable types
and guard helpers reject inaccessible dependencies. Public bodies may retain
private helper calls. Identity-based substitution is capture-free. The loader
retains valid contracts and owner-positioned diagnostics while preserving the
execution gate; legacy execution does not run the signature graph pass.
Focused namespace coverage passes 206 assertions. Seed refresh verifies fresh
compiler and standard-library fixed points. This is an intermediate GI#198
slice, not full P2 acceptance; partial completion remains unknown and the accepted
programme contribution remains 10 pp. See the
[performance disposition](tests/VALIDATION_PROFILE.md#namespace-checked-signatures--2026-10-04).

Remaining P2 scope: precise type/pattern use spans; uniform identity-based body
checking, purity/effect evidence, emission and tooling; audited stdlib exports, coordinated source
migration, native/browser and installed/bootstrap conformance. Remove the staged
execution gate and current flattening only with that identity integration and
fresh namespace-capable seed/source migration. GI#198 remains open. Website
impact: none for this slice, because executable namespace syntax remains disabled
and existing version-pinned examples and release claims remain accurate.

The following original assessment is historical; P1 delivered its design outcome.
The first outcome is a reviewed design with worked multi-file examples, a current
behavior audit, compatibility/migration decisions and positive/negative acceptance
cases. Cover private helpers, public entry points, aliases, selective imports,
re-exports, same-named symbols, transitive visibility and canonical module identity.
Assessment is provisionally medium / one design PR; implementation cost remains
unknown. Coordinate [package structure](#reusable-modules-and-package-structure--idea),
[dependency management](#dependency-management--idea) and #106's separate-compilation
design without requiring them to ship together. Main risks are ambiguous lookup,
accidental API exposure and breaking existing programs.

Assess first-class language support for namespaces separately from the existing
logical import paths (`stdlib/...` and `project/...`). Import path organisation
alone does not settle qualified symbol lookup or namespace semantics.

- [ ] Identify real name-collision, discoverability and API-organisation problems
      and representative programs that would benefit from namespaces
- [ ] Specify declaration and qualification syntax, namespace/module relationships,
      nesting, aliases, imports, visibility/exports, collision and shadowing rules
- [ ] Assess interactions with functions, types, generics, method syntax and
      compiler assistance; define useful unresolved/ambiguous-name diagnostics
- [ ] Estimate compiler, bytecode/runtime, bootstrap and tooling implications;
      decide compatibility and migration before selecting an implementation
- [ ] Define focused and public-CLI acceptance tests for lookup, imports,
      qualification, visibility and failures across source and installed toolchains

## Namespace the current standard library — idea

<a id="rm-42"></a>

**RM#42: Standard library namespaces**.

Current state: Planned as P4 in the [programme register](#rm-108); the
[shared design](docs/MODULE_PACKAGE_DESIGN.md) sets migration dependencies.
The historical assessment below remains acceptance input.

Review how existing library APIs should be grouped, exported and referenced so
users can discover them and avoid collisions. This is a separate deliverable
from general namespace support, with an explicit dependency assessment.

- [ ] Inventory current modules and exported names; identify collisions,
      inconsistent naming and unnecessarily exposed implementation details
- [ ] Propose coherent namespace boundaries and ergonomic qualified/unqualified
      usage, evaluated on real programs and compiler-assisted discoverability
- [ ] Decide which improvements are possible with today's imports and which
      require the language namespace proposal; avoid assuming they must ship
      together or making incompatible naming decisions independently
- [ ] Plan compatibility or a deliberate preview migration for compiler sources,
      libraries, examples, tests, documentation and installed packages
- [ ] Define acceptance evidence that public APIs remain accessible, names resolve
      predictably and existing behavior is preserved through the migration

Both production outcomes remain open and Planned in the programme register.
P1 design acceptance does not ship them; preserve current public contracts until
their separately selected implementation and migration deliveries.

## Reusable modules and package structure — idea

<a id="rm-43"></a>

**RM#43: Reusable packages** · [GI#199: Reusable packages](https://github.com/sproates/panackelty/issues/199).

Work record: [GI#199: Reusable packages](https://github.com/sproates/panackelty/issues/199).
Current state: Planned as P3 in the [programme register](#rm-108).
The [P1 shared design](docs/MODULE_PACKAGE_DESIGN.md) supersedes the historical
assessment-only scope below; production acceptance remains open.

Define how related files form a reusable library with a deliberate public API,
and how another application consumes it. Distinguish files, source modules,
namespaces, packages and applications. Assess package roots, manifests, exports,
internal modules, tests, resources, documentation and toolchain compatibility.
Compare source distribution with compiled artifacts without assuming a stable ABI
or requiring #106's separate compilation. This concerns language libraries, not
OS packaging of the Panackelty toolchain.

Smallest outcome: a reviewed design and bounded local-package prototype plan,
using one multi-file library from two applications and a relocated installation.
Define round-trip, hidden-internals and malformed/path-escape acceptance cases.
Coordinate [cross-file references](#language-namespaces--idea) and
[dependency management](#dependency-management--idea). Assessment is provisionally
medium / one design PR; implementation and ongoing maintenance need re-estimation.
Public API compatibility and premature artifact-format commitments are key risks.
No registry, separate compiler/linker or package format is selected.

## Dependency management — idea

<a id="rm-44"></a>

**RM#44: Dependency management** · [GI#200: Dependency management](https://github.com/sproates/panackelty/issues/200).

Work record: [GI#200: Dependency management](https://github.com/sproates/panackelty/issues/200).
Current state: Planned as P7 in the [programme register](#rm-108).
The [P1 shared design](docs/MODULE_PACKAGE_DESIGN.md) supersedes the historical
assessment-only scope below; production acceptance remains open.

Design how applications declare, obtain, update and reproduce direct/transitive
and development dependencies. Third-party packages remain pending in the current
specification. Compare local paths/vendoring, immutable Git revisions and versioned
archives before deciding whether a registry is useful. Define manifest/lockfile
roles, version and toolchain compatibility, conflict/diamond resolution, duplicate
versions, cycles and package-to-import identity.

Smallest outcome: a reviewed design and bounded prototype plan for an application,
library and transitive dependency. Specify explicit updates, locked clean/CI and
offline restores, integrity/provenance, caching, unavailable sources, conflicting
versions, corrupt artifacts and mismatched locks. Address untrusted package
contents, credentials/private sources and install/build-script policy. A public
registry is not a prerequisite; any service needs an ownership/maintenance plan.
Coordinate #198, #199 and #106's dependency-aware cache work. Assessment is
provisionally medium / one design PR; implementation/operations may be large.
Supply-chain trust, reproducibility and surprising upgrades are principal risks.
No implementation priority, version policy, network service or ecosystem chosen.

These linked investigations informed P1, while their production outcomes remain
independently scoped and unstarted. Historical sizes and idea headings above are
retained for context; current state, estimates and priority come from the
[programme register](#rm-108), not the original assessment notes.

## Review GitHub repository settings and tooling — idea

<a id="rm-45"></a>

**RM#45: Repository tooling review**.

Assess whether repository configuration and available tooling can improve
security, code quality and contributor experience. This is a backlog item, not
an assertion that any particular feature is disabled or suitable. Include it in
the holistic assessment and compare its value and effort with other candidates.

- [ ] Inventory actual settings, workflows and enabled checks; distinguish
      unavailable, disabled, already configured and redundant capabilities
- [ ] Evaluate applicable code/security scanning, secret scanning and push
      protection, dependency alerts/updates/review (including Actions), and
      static-analysis or code-quality tools; verify language/ecosystem support
      and avoid implying that a C scanner analyses custom `.panack` semantics
- [ ] Review branch/ruleset protections, required checks, workflow/token
      permissions, environment/release protections and contribution settings
      against the project's agreed workflow
- [ ] Record benefit, findings/actionability, false positives, setup and ongoing
      effort, CI latency, cost/plan availability, access needs and ownership
- [ ] Recommend a minimal useful set with a clear enable/retain/defer rationale,
      acceptance checks and an alert-triage process; obtain explicit permission
      before changing settings, protections, permissions or paid services

Acceptance for the review is an evidence-backed inventory and prioritised
recommendations. Enabling selected features is separately scoped work with
its own verification; do not turn on every available feature by default.

## Measure Panackelty source coverage — candidate pending assessment

<a id="rm-46"></a>

**RM#46: Panackelty source coverage** · [GI#131: Panackelty source coverage](https://github.com/sproates/panackelty/issues/131).

Work record: [.panack source coverage](https://github.com/sproates/panackelty/issues/131). State: Idea; implementation
unscheduled. See the [current comparison](#proposed-first-step) for first-slice
estimates and recommendation.

The public LLVM report measures the native C VM only. Existing `.panack` tests
exercise the compiler and libraries, but there is no measured source-line or
branch baseline for those files. Publishing C coverage did not close this gap.

The frontend already carries source positions; the emitted `FunctionCode` and
current v9 bytecode contract do not carry an instruction-to-source map.
Coverage therefore requires compiler/bytecode/VM design, not just an HTML export.
Reuse the [planned mapping foundation](#source-to-bytecode-mapping-foundation--planned-shared-milestone)
where its validated contract fits; coverage denominators, source branches and
counter correctness remain separate acceptance requirements.
Compare deterministic sidecar metadata with a versioned bytecode extension;
do not assume a format change or a particular instrumentation scheme in advance.

Provisional PR boundaries (split further if feasibility or review size requires):

1. **Coverage design and feasibility.** Inventory source-position fidelity and
   lowering; define executable lines, functions and source branches, including
   short-circuit expressions, match arms, loops, generated instructions, imports
   and erased generics. Prove a small source-to-execution mapping. Decide metadata
   identity, compatibility, validation and bootstrap implications; document
   overhead and implementation scope before committing to an estimate.
2. **Collection and correctness.** Implement opt-in measurement and a versioned
   raw format, with exact expected results on small known programs. Preserve
   ordinary outputs and semantics; test malformed/mismatched maps, disabled
   instrumentation and traps. Aggregate across subprocesses and nested VM runs
   without collisions, lost counts or double-counting cached test observations.
3. **Suite integration and published baseline.** Measure the `.panack` compiler
   while it compiles programs, bytecode tooling and standard-library execution,
   including relevant compiler/functional/bootstrap paths. Account for unexecuted
   eligible files, test infrastructure, generated code and intentional exclusions.
   Publish separate source reports through the existing coordinated Pages flow,
   with commit/source identity, suite scope, denominator and freshness visible.
4. **Risk-ranked gap closure and regression policy.** Turn uncovered behaviors
   into bounded test PRs; validate the assertions, not just execution counts.
   Select per-component change/regression policy after a credible baseline exists,
   with reviewed exclusions and no invented universal percentage requirement.

- [ ] Complete the design/feasibility decision and bound the initial scope
- [ ] Establish independently checked source mapping and counter correctness
- [ ] Record source-line/function coverage for the agreed `.panack` scope
- [ ] Record source-branch coverage, or explicitly track it as unfinished if a
      line-first increment is chosen; never substitute VM branch counts silently
- [ ] Include subprocess/nested compiler execution and all eligible unexecuted
      files in the aggregate, with reproducible source identities and exclusions
- [ ] Publish separate native C and `.panack` baselines; never blend percentages
- [ ] Demonstrate instrumented/uninstrumented semantic equivalence and record
      runtime, memory and validation cost without weakening existing checks
- [ ] Establish a documented regression policy and close the first verified gaps

An instrumented compiler must not accidentally change release artifacts or the
ordinary bootstrap fixed-point contract. Decide source-map privacy/path handling,
artifact retention and source-snapshot validation as part of the measurement
format. A report must distinguish zero hits, excluded code, missing data and
failed collection; partial collection must not appear as a complete green report.

## Groomed development and engineering work

The [candidate table](#groomed-candidates) owns state and estimates. Linked issues
hold detailed acceptance, risks and first-slice boundaries.

### Development workflow assessment

<a id="rm-47"></a>

**RM#47: Development workflow assessment** · [GI#133: Development workflow assessment](https://github.com/sproates/panackelty/issues/133).

Work record: [#133](https://github.com/sproates/panackelty/issues/133).

The earlier delivery pilot predates core-method and networking delivery. Observe
a fixed installation-to-maintenance task, including API discovery, errors and
focused tests. Include reproducible debugging tasks covering a runtime trap, an
external-operation failure and an incorrect result. Observe whether a developer
can identify the cause, fix it and add a regression test; record time to a correct
fix, misleading diagnostics and missing tooling, without treating speed alone as
success. Use these findings to improve the debugging guide and bound any debugger
assessment. Record reproducible obstacles and feedback latency, with human
and fresh-context agent evidence where available. Distinguish release and
development toolchains. Produce bounded fixes rather than assuming a project
generator, formatter or package manager is needed.

This delivery adds [reusable project operations](docs/PROJECT_OPERATIONS.md): a
compact head-specific PR snapshot, an exact committed-tree publication manifest
and a checked new-branch publisher. Regression tests cover stale/missing evidence
and identity failures. This is a bounded workflow improvement, not completion of
the broader assessment. Tracking reconciliation and existing-branch publication
remain follow-ups; no new issue or programme is introduced. Website impact: none,
because contributor operations do not change published language/release claims.
Validation: 13 operation tests pass separately; clean-worktree `make check` passes
in 225s (unit 177s, functional 6s, bootstrap 27s). The full-check budget warning
remains an active GI#106 follow-up; this observation is not a controlled baseline
or evidence of a compiler regression caused by these optional scripts.

### Compiler explanations

<a id="rm-48"></a>

**RM#48: Compiler explanations** · [GI#134: Compiler explanations](https://github.com/sproates/panackelty/issues/134).

Work record: [#134](https://github.com/sproates/panackelty/issues/134).

The first U3 query is accepted through #219: `panack explain` reports
actual accepted/unproved subtraction decisions, operand facts and guard source
locations with explicit unavailable results. Diagnostic parity, public-CLI
acceptance and cost are recorded in the [query contract](docs/COMPILER_EXPLANATIONS.md)
and validation profile. Full types/effects/proof explanations remain open within
[programme #180](#compiler-and-runtime-understanding-programme); re-estimate and
select the next slice through programme prioritisation.
Runtime value provenance is its own linked workstream #172; new inference rules
and automatic fixes remain outside #134.

### Systematic invariant testing

<a id="rm-49"></a>

**RM#49: Invariant testing** · [GI#135: Invariant testing](https://github.com/sproates/panackelty/issues/135).

Work record: [#135](https://github.com/sproates/panackelty/issues/135).

Extend existing source/bytecode comparisons, deterministic round trips and
bootstrap fixed points with one bounded invariant family. Use reproducible
generation or justified source transformations, an independently reviewed oracle
and useful reduced failures. Demonstrate detection of an isolated deliberate
perturbation. Shared implementation bugs and invalid transformations are risks;
full random-language generation and a second execution engine are outside scope.

### Source-aware runtime errors

<a id="rm-50"></a>

**RM#50: Source-aware runtime errors** · [GI#136: Source-aware runtime errors](https://github.com/sproates/panackelty/issues/136).

Work record: [#136](https://github.com/sproates/panackelty/issues/136).

Production source positions, instruction mappings and validated sidecar lookup
are available through U2. Automatic runtime error integration remains open; build
on the [mapping foundation](#source-to-bytecode-mapping-foundation--planned-shared-milestone)
for one bounded trap and call-context slice, assessing reuse with coverage
metadata where semantics agree. This is the first source-aware debugging-support
delivery: explain what failed, identify the source expression and show useful
call context. Validate these diagnostics through the debugging tutorial.
Acceptance includes imported/generic code, nested calls, an
async boundary and safe fallback for absent or mismatched source snapshots.
Preserve error meaning and bytecode safety. A debugger and full async history
are separate; report publication is not a prerequisite.

### Learning path and technical documentation

<a id="rm-51"></a>

**RM#51: Learning and debugging guides** · [GI#137: Learning and debugging guides](https://github.com/sproates/panackelty/issues/137).

Work record: [#137](https://github.com/sproates/panackelty/issues/137).

Build on the README, examples, specification, VM guide and playground. Start
with one complete tutorial from clean installation through testing and a
maintenance change, linked to discoverable library/reference material. Run its
commands against the declared version and keep development-only networking
distinct from alpha.10. Subsequent practical and technical guides follow
demonstrated gaps. Website visual changes require a working review preview.

Explicit deliverable: **Debugging Panackelty programs**, a practical guide using
commands supported by its declared release. Cover three worked problems: a
runtime trap, an expected external-operation failure that needs handling, and a
valid program producing the wrong result. Each walkthrough must reproduce the
problem, investigate its cause, fix it and add a regression test. Show how to
inspect intermediate values and reduce a failing case using available tools;
state diagnostic limits honestly. The VM guide's recorded executions are useful
background, not an interactive debugger.

Start with today's tools rather than waiting for source maps or a debugger.
Update the guide when source-aware locations and call context ship under #136.
Coordinate executable examples with #138 and usability evidence with #133.
Initial guide estimate: S–M / 1–2 PRs including examples and CI checks; later
updates follow the capabilities they document.

### Public Markdown quality

<a id="rm-131"></a>

**RM#131: Public Markdown quality** · [GI#281: Public Markdown quality](https://github.com/sproates/panackelty/issues/281).

**Idea / unscheduled.** Recorded at the user's request on 2026-10-04. Review,
expand and improve all public Markdown, including `SPEC.md`, `ARCHITECTURE.md`,
the README, contributor material and guides. Inventory the documents and assess
accuracy, completeness, readability, examples, cross-links and version claims;
reconcile language and architecture descriptions with the implementation.
Complement [RM#51: Learning and debugging guides](#rm-51) and
[RM#52: Executable documentation](#rm-52), preserving their existing scope.

Acceptance: account for every inventoried document, fix or explicitly track
remaining gaps, verify examples against declared versions and check links and
consistency between references and guides. Provisional effort: M–L across staged
PRs; the inventory determines dependencies and final scope. This records a
documentation review, not approval to change language behavior or priorities.

### Separate agent-instructions repository

<a id="rm-132"></a>

**RM#132: Separate agent-instructions repository** · [GI#282: Separate agent-instructions repository](https://github.com/sproates/panackelty/issues/282).

**Idea / unscheduled.** Recorded at the user's request on 2026-10-04. Inventory
`AGENTS.md`, `.agents/` and related agent-only files, then plan their move to a
separate repository. Choose destination, visibility, versioning, discovery and
setup during assessment. Preserve public contributor documentation, test rules
and working links; retain current guidance until the migration is accepted.
Coordinate with [RM#47: Development workflow assessment](#rm-47).

Acceptance: agree the file boundary, remove or update every affected reference,
and verify that a fresh checkout has usable contributor instructions and a
documented, reproducible way to obtain the matching agent guidance. Keep
private configuration out of public material. Effort is unknown;
repository access and the distribution/versioning contract remain
unknown dependencies. Recording this starts no repository creation or file move.

### Definitive Panackelty style guide

<a id="rm-133"></a>

**RM#133: Definitive Panackelty style guide** · [GI#283: Definitive Panackelty style guide](https://github.com/sproates/panackelty/issues/283).

**Idea / unscheduled.** Recorded at the user's request on 2026-10-04. Build on
the existing [Panackelty source conventions](CONTRIBUTING.md#panackelty-source)
to establish one authoritative guide for Panackelty code, covering formatting,
naming, declarations, types, control flow, error handling, effects and examples.
Consolidate overlapping guidance and explain decisions with idiomatic examples;
coordinate with [RM#51: Learning and debugging guides](#rm-51),
[RM#52: Executable documentation](#rm-52) and
[RM#47: Development workflow assessment](#rm-47).

Acceptance: resolve conflicting conventions, link the definitive guide from
contributor material, check examples against their declared language version and
distinguish stylistic recommendations from language requirements. Provisional
effort: M; detailed scope and disputed conventions need assessment. Existing
guidance remains authoritative until the replacement is accepted. This records
no wholesale reformat, formatter implementation or change to programme priorities.

### Website capabilities guide

<a id="rm-130"></a>

**RM#130: Website capabilities guide** · [GI#279: Website capabilities guide](https://github.com/sproates/panackelty/issues/279).

**Idea / unscheduled.** Recorded at the user's request on 2026-10-04. The homepage
capability cards mostly send readers to specification or reference material.
Add a friendly `/capabilities/` page explaining all features available in the
advertised releases, grouped by practical use, with contents navigation and
homepage cards linking to the relevant sections. Explain each feature's purpose
in plain language, with small explained examples, native/browser availability,
version limits and onward links to tutorials, the playground and specification.

Coordinate with [RM#51: Learning and debugging guides](#rm-51) /
[GI#137: Learning and debugging guides](https://github.com/sproates/panackelty/issues/137)
and [RM#52: Executable documentation](#rm-52) /
[GI#138: Executable documentation](https://github.com/sproates/panackelty/issues/138).
Acceptance: inventory the current advertised releases' features, execute examples
against their declared versions, check links, review an accessible desktop/mobile
preview and verify the live page after authorised publication. Provisional medium
effort, around 1–2 PRs, to refine after inventory. Recording this idea starts no
page implementation and changes no
priority, programme weight or existing website scheduling decision.

### Executable documentation

<a id="rm-52"></a>

**RM#52: Executable documentation** · [GI#138: Executable documentation](https://github.com/sproates/panackelty/issues/138).

Work record: [#138](https://github.com/sproates/panackelty/issues/138).

Quick-start, website, functional-example and playground checks already execute
documentation. Inventory checked, illustrative and uncovered content, then
verify one additional guide through the existing toolchain. Incorrect commands
or expected output must fail visibly. Preserve negative examples, clean
setup/cleanup and version boundaries. Coordinate with the first tutorial without
introducing a competing harness or silently skipping platform-dependent
examples.

Include the debugging tutorial explicitly: execute its failing examples, asserted
exit status and relevant diagnostics, corrected programs and regression tests
against the declared release. Incorrect commands or changed expected results
must fail CI. Use deterministic local fixtures for external-operation failures;
keep setup and cleanup reproducible. Distinguish currently supported output from
future source-aware diagnostics instead of accepting either silently.

### Debugging delivery sequence — agreed 2026-10-01

Runtime failures and incorrect results remain possible despite compiler checks.
The user agreed the following debugging sequence; these are scoped additions to
existing work, not a new parallel principal programme or an implementation claim:

1. Deliver the first practical guide under #137/#138 using available tools, with
   #133 recording observed debugging obstacles. This has no source-map prerequisite.
2. Deliver the shared source-mapping foundation after #182 and the first
   source-aware diagnostics under #136; update the guide against their released
   behavior. The current programme remains the principal implementation priority.
3. Assess an interactive debugger against the remaining observed problems before
   committing to production debugger delivery.

Runtime value provenance #172 should evaluate the guide's incorrect-result
scenario: whether retained derivations help identify where a value went wrong.
Keep its independent correctness, opt-in overhead, retention and sensitive-data
acceptance; tracing derivations does not by itself provide stepping or inspection.
Source mapping remains the shared foundation already planned under #180/#173.

This planning update changes no website claims. Guide publication and shipped
debugging capabilities must assess website follow-ups for their stated versions.

### Interactive debugger — candidate for assessment

<a id="rm-53"></a>

**RM#53: Interactive debugger**.

**Idea; implementation unscheduled.** No dedicated issue yet; create its work
record after checking existing issues when the bounded assessment is selected.

Disassembly and recorded VM walkthroughs do not let a developer pause a running
program and inspect its state. Assess whether interactive controls resolve gaps
observed by #133 and the debugging tutorial, particularly incorrect-result bugs.

Start with a bounded synchronous CLI experiment: source breakpoints, stepping,
local-variable inspection and call-frame navigation on small known programs.
Use the shared mapping contract and preserve the VM as the only execution engine.
Define breakpoint resolution and step behavior around lowering, imports and
unmapped instructions; report unavailable state honestly. Compare observed stops,
locals and frames with independently expected results, and verify that ordinary
execution remains equivalent with debugging disabled.

Evaluate VM pause/resume hooks, metadata fidelity, state lifetime, resource cost
and safe inspection without unintended side effects. Unknown feasibility and
maintenance costs argue for an experiment before promising a production debugger.
Deliver a justified proceed/defer decision, demonstrated limitations and revised
estimates. Provisional size: M / one assessment PR; full delivery remains unknown.
Editor integration and async debugging require separate assessments and are not
acceptance requirements for this synchronous experiment.

### Editor support

<a id="rm-54"></a>

**RM#54: Editor support** · [GI#139: Editor support](https://github.com/sproates/panackelty/issues/139).

Work record: [#139](https://github.com/sproates/panackelty/issues/139).

No dedicated editor extension or grammar package was found in the tracked tree
during grooming. Select one editor before implementation, then deliver file
recognition, highlighting, comments, bracket pairing and indentation with
installation instructions. Validate current lexical examples and incomplete
code. Compiler-backed diagnostics, navigation, hover and completion follow
separate assessment; a language server, formatter and marketplace release are
not prerequisites.

### Technical showcase programs

<a id="rm-55"></a>

**RM#55: Showcase programs** · [GI#140: Showcase programs](https://github.com/sproates/panackelty/issues/140).

Work record: [#140](https://github.com/sproates/panackelty/issues/140).

Choose one complete demonstration combining existing capabilities, such as an
exact ledger with checked domain rules or a finite concurrent service with
independent clients. Provide deterministic inputs, expected source/bytecode
results and both success and failure boundaries. Explain guarantees and
platform/release limits. Reuse existing examples and checks; no new language
feature or production HTTP claim is required.

<a id="runtime-and-resource-baselines"></a>

### Performance and benchmarking

<a id="rm-56"></a>

**RM#56: Performance baselines** · [GI#141: Performance baselines](https://github.com/sproates/panackelty/issues/141).

Work record: [#141](https://github.com/sproates/panackelty/issues/141).

The [2026-10-03 source-based performance assessment](tests/VALIDATION_PROFILE.md#runtime-performance-assessment--2026-10-03)
records architecture costs, exact-semantics comparison limits, finite TCP and
cooperative-execution boundaries, existing compiler measurements and a proposed
six-group baseline. No new benchmarks were run; neither broad inadequacy nor
production adequacy is established. This research record does not schedule
benchmark implementation, select optimisations or change programme priority.

Additional questions recorded on 2026-10-03 in GI#141: investigate JIT feasibility,
optimising ahead-of-time compilation directly to native executables, and how those
choices affect startup, sustained throughput, memory use and the cost of preserving
exact semantics in performance comparisons. These are **unscheduled research
questions only**; no investigation, backend implementation or architecture change
is selected by this record.

State: **Planned baseline; execution not started**, selected as a scoped outcome
of [RM#123: Standing performance engineering](#rm-123) on 2026-10-04. This updates
the earlier Idea/unscheduled baseline status while preserving the principal
feature priority and deferred website/CI work. It authorises no benchmark run,
optimisation or backend implementation in the current policy delivery.

Existing validation profiles and paired experiments are useful evidence but do
not form a general maintained performance baseline. Start with a small suite of
representative programs and focused benchmarks covering compilation, exact-value
operations, strings/collections and bounded I/O where supported. Measure
compilation, startup and execution costs, throughput/latency where meaningful,
peak memory and artifact size where reliable measurement is available.

Correctness-check outputs, pin inputs and record toolchain/environment metadata,
replay commands, repetitions and variability. Separate cold/warm and compile/run
costs, compare like-for-like revisions without concurrent benchmark interference,
and label unavailable metrics. The smallest useful outcome is a reproducible
baseline and comparison report, provisionally **M / 1–2 PRs**; host noise and
platform-specific accounting limit confidence in comparisons.

Use subsequent profiling to identify bottlenecks and scope targeted optimisation
with before/after evidence. Set regression budgets or automation only after
baseline noise is understood. Optimisation is separately scoped, not part of the
initial baseline commitment; preserve exactness, safety and correctness.
Universal thresholds and cross-language superiority claims are outside the
initial scope. The planned baseline requires its own selected implementation
delivery; JIT/AOT and other backend investigations remain unselected.

### Independent contract implementation

<a id="rm-57"></a>

**RM#57: Independent contract assessment** · [GI#142: Independent contract assessment](https://github.com/sproates/panackelty/issues/142).

Work record: [#142](https://github.com/sproates/panackelty/issues/142).

The bytecode format, fixed fixtures and VM/compiler audit already establish
substantial contract evidence. Test one small stable subset by implementing a
disposable probe from the written contract before consulting implementation
details. Record ambiguities, independently derived malformed/valid vectors and
any prior knowledge limiting independence. Use the supported toolchain. This
assesses specification precision without adding a second execution engine or
claiming full-language conformance.

## Potential technical publications

Record publication candidates when there is a concrete thesis and supporting
implementation or evaluation evidence. Each candidate needs a contribution to
assess, links to evidence and prior work, and a revisit trigger. This is an
unscheduled backlog, not a publication programme or a commitment to publish
every idea. The change-impact research candidate below may contribute to the
compiler paper. The separate [type-inference research candidates](#type-inference-research-candidates)
may support future papers if their contributions are established; no separate
paper is promised for any candidate.

### Compiler and runtime understanding paper — idea

<a id="rm-58"></a>

**RM#58: Compiler understanding paper** · [GI#211: Compiler understanding paper](https://github.com/sproates/panackelty/issues/211).

Work record: [#211](https://github.com/sproates/panackelty/issues/211).
Agreed for the backlog on 2026-10-02; assessment is unscheduled. Revisit after
programme #180 U8 realistic-program evaluation. This is separate from #180 and
does not block programme acceptance or its final U9 website refresh.

Assess whether the implemented architecture and measured results justify a
technical white paper, experience report or research submission. Compare with
relevant prior work and identify the contribution supported by evidence;
original research novelty is not established. A useful account of engineering
and integration does not require inventing every underlying concept.

The smallest outcome is a recorded assessment and publication recommendation,
including a reason and revisit trigger if deferred. If justified and selected,
draft the paper around architecture, reproducible realistic examples, exact
revisions and commands, measurements, independent evaluation, limitations and
related work. Review claims and reproduce results before an explicitly agreed
publication; recording this candidate does not start writing or submission.
Effort and PR count remain unknown until the evidence and format are assessed.

During programme delivery, retain design rationale, counterexamples, unsuccessful
approaches and performance/usefulness evidence in existing programme records.
Reuse those records rather than introducing a competing progress tracker.
Risks are overstated novelty, unsupported generalisation and distraction from
delivery; do not expand compiler scope merely to obtain a paper.

### Independently checkable change-impact explanations — research idea

<a id="rm-59"></a>

**RM#59: Checkable change impact** · [GI#213: Checkable change impact](https://github.com/sproates/panackelty/issues/213).

Work record: [#213](https://github.com/sproates/panackelty/issues/213).
Agreed for the backlog on 2026-10-02: a bounded research assessment and prototype,
unscheduled. Link findings to paper candidate #211 and coordinate with programme
#180 explanations/provenance and semantic prediction (#134/#173/#175). This is
not an extra mandatory workstream or programme completion requirement.

Investigate whether a small separate verifier can check evidence explaining
which selected guarantees survive a code change, at practical cost. First compare
with proof-carrying code, incremental/differential verification, summary repair
and explanation research; identify a precise contribution or record why further
work is not justified. Novelty is a hypothesis, not an established claim.

If the assessment supports a prototype, begin with guarded arithmetic and
mutation, including cross-module obligations. Define certificates bound to exact
source/dependency revisions, supported reasoning rules and the trusted computing
base. Check reasoning independently of the explanation producer; source hashes
alone establish no semantic guarantee. Distinguish proved preservation, loss of
an existing proof, an exhibited violation and unknown. Loss of proof is not proof
of a bug, and preserved obligations do not establish whole-program equivalence.

Agree scope, representative cases and budgets before evaluation. Test false,
tampered, stale and mismatched evidence as well as valid and unsupported cases.
Compare ordinary diagnostics and dependency-based impact reporting; measure
correctness, useful precision, certificate size, verification cost and developers'
ability to identify a valid repair. Retain failures, limitations and reproducible
results; finish with independent review and a proceed/defer recommendation.

Assessment can precede programme completion; select prototype timing when source
identity and checker evidence interfaces are available. Effort and PR count are
unknown until assessment. Production integration or programme expansion needs a
separate decision. Risks include overlap with prior work, verifier complexity,
unsound non-impact claims, overhead and distraction from agreed delivery.

## Historical grooming gaps and decisions

These earlier findings are retained as supporting context, not a fresh audit.
The broader
[2026-09-29 assessment](docs/ADOPTION_ASSESSMENT.md) now covers application
architecture and adoption as well as testing. Neither report authorises every
feature or settles implementation priorities.

| Gap or ambiguity | Evidence / consequence | Treatment |
| --- | --- | --- |
| Developer experience and compiler assistance | Existing ideas emphasise diagnostics; useful assistance throughout development needs deliberate assessment | Treat enjoyment and productivity as strategic goals alongside strong testing |
| Roadmap documentation and process | Stale status and overlapping documents made the actual priorities unclear | Review document roles, acceptance criteria and the process for maintaining current evidence |
| Source coverage and its denominator | C coverage says nothing about which `.panack` lines or branches execute; omitted files could inflate future reports | Evaluate coverage scope, effort and enabling value in the holistic assessment |
| Test evidence has drifted | Matrix/backlog references include retired test paths, “both VMs”, a compiler “skeleton”, and already-implemented deterministic round trips | Audit current assertions against the matrix before declaring missing tests; retain historical fixture provenance |
| Execution coverage versus test quality | A hit does not establish that a test would catch the wrong result | Keep an explicit assertion-quality backlog; evaluate bounded mutation or deliberately perturbed fixtures for selected high-risk behavior |
| Runtime source diagnostics | Frontend positions exist but emitted instruction records lack source maps | Define a separate follow-up for source-aware runtime traps/call stacks; reuse coverage metadata only where semantics and validation agree |
| Product direction and feature selection | The specification names Euler programs as its proving ground, while the roadmap explores dependable automation and many unrelated differentiators | Agree representative user programs and a next preview outcome before promoting experiments; reconcile product wording in a later scoped change |
| Milestone exit criteria | Initial preview delivery is explicit; the next capability milestone is not | Define observable user outcomes, supported scope, compatibility implications and release evidence, rather than promising a date or a bundle of speculative features |

Existing proposals already cover JSON, generic/inference extensions, host APIs,
error quality, browser targets and contribution criteria. These are prioritisation
choices, not newly discovered omissions. Missing package management, concurrency,
module visibility and compatibility guarantees are explicitly deferred in the
specification; do not promote them merely because other languages have them.

## Harden and expand test coverage — candidate; foundation delivered

<a id="rm-60"></a>

**RM#60: Test coverage hardening**.

The [invariant-testing item](#systematic-invariant-testing) scopes generated inputs
and equivalence checks separately from [source measurement](#measure-panackelty-source-coverage--candidate-pending-assessment).
Both extend existing evidence; neither changes coverage status by being planned.

The goal is to make regressions difficult to introduce and failures easy to
localize while keeping `make check` the canonical validation command.

- [x] Separate internal unit tests from black-box functional program tests
- [x] Discover functional cases and example expectations without a central
      manifest
- [x] Inventory the behavior promised by `SPEC.md` and map it to existing tests
- [ ] Audit success and failure assertions by language construct and runtime
      built-in; turn verified omissions into bounded, risk-ranked test PRs
- [x] Cover every CLI command and shorthand through end-to-end subprocess tests
- [ ] Expand type, refinement, purity, and name-resolution diagnostic coverage
- [ ] Exercise file, import, malformed-input, and operating-system failure paths
- [ ] Expand malformed and adversarial bytecode verifier and VM coverage
- [x] Establish deterministic compilation and canonical bytecode round-trip tests;
      extend edge cases only against identified gaps
- [x] Establish direct lexer, parser, resolver, checker, purity, emitter and
      driver probes; this is a foundation, not proof of exhaustive behavior
- [ ] Audit remaining compiler/library assertions and source coverage by component
- [x] Establish and publish a native C line/branch baseline
- [ ] Establish the separate `.panack` baseline through the candidate measurement initiative
- [ ] Review intentional exclusions and untested host-boundary behavior explicitly
- [x] Organize the suite so focused failures remain fast and the full suite stays
      practical to run after every change

Testing work that is also a prerequisite for self-hosting should be reflected
in both roadmaps when completed.

### Publish public native C coverage reports — complete

<a id="rm-61"></a>

**RM#61: Native coverage publication**.

The native VM's LLVM line and branch coverage remains uploaded as a CI
artifact. The coordinated Pages publisher makes the HTML report available at
`https://panackelty.com/coverage/`, so readers can open it directly without
visiting an Actions run or downloading and extracting an archive. PR #81 merged
as `52fb0d7`; its main validation and Pages build, deployment and live verification
all passed on 2026-09-28. The published report is native C coverage only: 87.43%
lines, 80.65% branches and 100% functions; it is not a `.panack` baseline.

- [x] Implement publication of the successful `main` build's native coverage HTML and summary
      through the project's existing GitHub Pages site, with source commit,
      generation date and an explicit label that this covers the native C VM,
      not the self-hosted compiler or the entire language
- [x] Keep the website and coverage deployment coordinated so a normal site
      update cannot erase the latest report and a report update cannot replace
      the site with stale content; publish only from trusted, successful builds
- [x] Link the stable report from the site and README, add deployment checks for
      its entry page and relative source-navigation links, and keep the existing
      downloadable CI artifact for debugging
- [x] Document how publication failures and stale results appear, and ensure
      release and PR workflows cannot publish an unreviewed coverage site
- [x] Confirm the first merged `main` run publishes successfully and passes the
      live entry-page, source-navigation and provenance checks

## Codebase-wide human readability and refactoring — candidate

<a id="rm-62"></a>

**RM#62: Component readability** · [GI#132: Component readability](https://github.com/sproates/panackelty/issues/132).

Work record: [Component readability and refactoring](https://github.com/sproates/panackelty/issues/132). State: Idea; implementation
unscheduled. See the [current comparison](#proposed-first-step) for first-slice
estimates and recommendation.

Extend the native VM readability cleanup across all project code, including
the compiler, bytecode tooling, runtime, standard library, bootstrap code,
CLI, build and CI scripts, and test harnesses. Make the code straightforward
for a human maintainer to read, navigate and change. Start with an audit, then
refactor in component-sized PRs after establishing
relevant test evidence. The assessment proposes using source coverage to inform
risk without making its delivery a blanket prerequisite for small, well-tested
refactors; review that dependency with the implementation priorities. The REPL is not a
prerequisite; do not mix feature changes into readability refactors.

- [ ] Audit each component for dense or oversized functions, unclear names,
      duplicated logic, hidden dependencies and obsolete code; record a scoped
      refactoring sequence using the native VM cleanup as the model.
- [ ] Split responsibilities into cohesive modules and small, clearly named
      functions; make interfaces, data flow, ownership and error handling
      explicit, with consistent formatting and comments explaining non-obvious
      decisions and invariants.
- [ ] Remove obsolete code and unnecessary duplication, keeping any required
      bootstrap distinctions explicit; update architecture and component
      documentation alongside each refactor.
- [ ] Preserve observable behavior, exact numeric semantics, purity, bytecode
      safety and bootstrap reproducibility. Retain all existing assertions,
      add meaningful regression coverage where gaps are found, and validate
      each scoped refactor with `make check` and applicable sanitizer/platform
      gates; measure and avoid validation or runtime performance regressions.

## Native VM readability and test hardening

<a id="rm-63"></a>

**RM#63: Native VM hardening**.

- [x] Decompose native decoding, verification, values, arithmetic, execution,
      builtins and host services into separately compiled modules.
- [x] Put shared declarations in self-contained headers, retain private local
      types, name wire opcodes, and document ownership and formatting conventions.
- [x] Add direct module ownership, cleanup, builtin registry and header checks,
      deterministic decoder mutations, and an isolated sanitizer target.
- [x] Sweep allocation failures across representative decoding, values, frames,
      exact arithmetic, nested execution and host operations; assert cleanup.
- [x] Add rich deterministic bytecode mutations, persistent ownership sequences,
      numeric boundary properties and selected host syscall failures.
- [x] Run native sanitizers in CI and publish LLVM line/branch coverage reports.
- [ ] Extend coverage-guided decoder fuzzing, host syscall/errno combinations,
      rendering and nested-execution branch coverage, and longer ownership runs;
      retain exact arithmetic, purity and runtime trap conformance throughout.

Manual release initiation retains the full validation gates. Keep its metadata
controls in the existing harness; retain timing warnings and the non-blocking
performance backlog without weakening release tests.
## Keep validation within development budgets — non-blocking backlog

Roadmap item: [RM#28: Incremental and modular builds](#rm-28).

Retain timing warnings and strong coverage. This work no longer blocks source
coverage, test hardening or readability work; the REPL has no scheduled slot.

| Measurement | Latest recorded evidence | Outstanding issue |
| --- | --- | --- |
| Local platform-installation instructions (2026-10-03; warm rerun after clean harness correction) | 119s; unit 84s; functional 4s; canonical checks passed | Existing unit 15s warning persists; retain profiling reminder under #106 |
| Full cold hosted CI | Five follow-up runs: 88/106/107/103/97s, median 103s | Hosted scheduling/completion has no proven hard upper bound |
| Local coverage-host cutover (2026-10-02; Linux workspace) | 196s; unit 135s; canonical checks passed | Existing full/unit budget warnings persist; retain profiling reminder under #106 |
| Local clean check, append experiment | 107s | Environment-specific result, not a universal guarantee |
| Local clean check, coverage publication | 145s, then final 144s; unit phase 99s | Existing clean/unit warning budgets still exceeded in this workspace |
| Local clean check, adoption/workflow docs (2026-09-29) | 147s; conventions follow-up 152s (unit 104s); all checks passed | Existing clean/unit budget warnings persist; remains non-blocking |
| Local assessment validation (2026-09-29; native build already present) | 144s; all checks passed | Existing full-check budget warning persists; remains non-blocking |
| Local delivery-pilot report validation (2026-09-29; native build already present) | 148s; all checks passed | Existing full-check budget warning persists; remains non-blocking |
| Local sorting/suffix validation (2026-09-29; clean build) | 148s; unit 102s; all canonical checks passed | Existing full-check/unit budget warnings persist; remains non-blocking |
| Local execution-design validation (2026-09-29; clean build) | 148s; unit 102s; all canonical checks passed | Existing full-check/unit budget warnings persist; remains non-blocking |
| Local resumable-VM validation (2026-09-29; clean rebuild, native prerequisites prepared first) | 150s; unit 105s; all canonical checks passed | Existing full-check/unit budget warnings persist; remains non-blocking |
| Local task/lifecycle validation sample (2026-09-29; macOS arm64, clean build) | 103s; unit 69s; canonical checks passed | Full check within 120s; unit warning persists; cross-host timings are not directly comparable |
| Local async-interface proposal validation (2026-09-29; macOS arm64, clean build) | 106s; unit 70s; canonical checks passed | Full check within 120s; existing unit warning remains non-blocking |
| Local shared-skill validation (2026-09-29; macOS arm64, clean build) | 104s; unit 70s; canonical checks passed | Full check within 120s; existing unit warning remains non-blocking |
| Local async implementation validation (2026-09-29; Linux workspace) | Final isolated checks passed in 360s and 358s; initial 370s sample overlapped sanitizer work | Existing full-check/unit warnings persist |
| Controlled clean comparison (#104; same Linux workspace) | Pre-async 341s / unit 231s; merged async 362s / unit 245s; both passed | Most cost predates async; prioritise bounded scheduling and retain all coverage |
| Bounded local overlap (#104; same Linux workspace) | 346s / unit 224s; all checks passed; 16s (4.4%) faster in one sample | Full/unit budgets still exceeded; bootstrap warned at 61s / 60s |
| Focused local VM check | Cached median 25.598s; test-edit median 32.993s | Both above the 15s focused target |

- [ ] Revisit compiler/nested-runner and collection costs when the developer
      feedback delay justifies it; use current measurements before changing code
- [ ] Retain the local full-check warning observed during preview work (208s
      against 120s on 2026-10-01), also observed during shared programme-skill
      validation (Linux workspace, clean check 153s on 2026-10-03); this remains
      a non-blocking validation-cost item
- [ ] Keep runner queue/completion tails distinct from actual execution time
- [ ] Demonstrate the 120s clean and 15s focused budgets on named reference
      environments; never remove tests, widen budgets or hide failing samples
- [ ] Preserve per-phase timing, standalone target behavior, sanitizer evidence,
      exact-artifact release checks and isolated bootstrap proofs

The docs-only route, shared validated probes, bounded workers, CI partitioning
and LLVM setup improvements are delivered. Earlier “budget reached” statements
were true of their recorded snapshots, not completion of today's open targets.
Historical observations and the detailed optimisation sequence are retained in
[the profiling report](tests/VALIDATION_PROFILE.md#archived-roadmap-performance-history--2026-09-28).

## Expand automation and host capabilities — foundation delivered; extensions unscheduled

<a id="rm-84"></a>

**RM#84: Host automation APIs**.

Panackelty should gain the general host capabilities needed by dependable
automation programs. These APIs
must be useful outside the test harness, remain visibly effectful, behave
predictably across supported platforms, and expose structured failures rather
than test-specific shortcuts. Logical standard-library imports are a
prerequisite so programs can use these APIs without knowing repository paths.

- [x] Specify a coherent process API for executable selection, arguments,
      standard input, working directory, environment overrides, exit status,
      and captured standard output and error
- [x] Make process streams byte-oriented with explicit checked UTF-8 decoding,
      define resource and output limits, and prevent deadlocks when both output
      streams are active
- [ ] Specify portable directory enumeration, directory creation, file metadata,
      removal, and recursive operations with deterministic ordering and clear
      symbolic-link and failure behavior; immediate enumeration, one-level
      creation, metadata, and nonrecursive removal are implemented, while
      recursive operations remain deferred
- [x] Add collision-safe temporary-file and temporary-directory creation with
      explicit ownership, cleanup, and failure semantics
- [x] Separate wall-clock time from a monotonic elapsed-time API suitable for
      validation budgets and performance measurements
- [ ] Define the supported-platform and capability policy for behavior such as
      permissions that cannot be represented consistently on every host; do not
      add a universal operation solely for a platform-specific test
- [x] Implement the accepted host boundary in the native VM; retain the fixed
      independent expectations from the retired differential-validation workflow
- [x] Add typed standard-library wrappers that keep all process, filesystem,
      temporary-resource, and clock operations effectful
- [ ] Extend focused, cross-platform, and public-CLI conformance coverage, including
      large simultaneous process streams, invalid UTF-8, missing executables,
      environment and working-directory isolation, cleanup failures, path
      traversal, resource limits, and monotonic timing; the initial suite covers
      streams, isolation, temporary cleanup, bounds, and failure categories;
      exhaustive injected host failures and traversal coverage remain pending
- [x] Build a small Panackelty testing library with assertions, structured test
      results, fixture discovery, temporary isolation, command assertions, and
      deterministic reporting for automated validation; the three
      explicitly imported modules now cover pure assertions and ordered reports,
      sorted immediate fixture directories and explicitly owned workspaces,
      plus bounded byte-exact command assertions and expected host errors.

## Language direction and differentiation — exploration

Panackelty should combine strong static guarantees with a low-friction programming
experience. Powerful checking is useful only when programmers can understand a
failure and act on it quickly. New features should therefore be evaluated on
both the guarantees they provide and the clarity of the resulting workflow.

### Error propagation shorthand

<a id="rm-129"></a>

**RM#129: Error propagation shorthand** ·
[GI#274: Consider error propagation shorthand](https://github.com/sproates/panackelty/issues/274).

**Idea / unscheduled.** Recorded at the user's request on 2026-10-04.

Consider shorthand for repeated `Result` handling that extracts an `Ok` value
and returns an `Error` from the enclosing function. Assess readability and
boilerplate in representative programs against explicit matching. Syntax,
error-type compatibility, propagation boundaries, async/effect interactions and
cleanup implications remain to be assessed; preserve explicit typed failures
and the purity boundary.

The smallest outcome is an evidence-based recommendation, including retaining
explicit matching if shorthand offers insufficient value. Assessment is small;
implementation effort and dependencies remain unknown. No urgency is established.
This records an idea, not a syntax decision, exception system or implementation
commitment.

### Type inference and diagnostic experience

<a id="rm-64"></a>

**RM#64: Inference and diagnostics** · [GI#170: Inference and diagnostics](https://github.com/sproates/panackelty/issues/170).

- [x] Add source excerpts and carets to existing positioned compiler errors,
      preserving imported-module ownership and source snapshots; cover tabs,
      Unicode, CRLF, EOF, and header-only fallback in renderer and CLI tests

- [x] Implement initializer-based local inference with optional annotations,
      fixed types, immutable defaults, explicit `mut`, and no shadowing;
      preserve numeric defaults, guarded types, and callable effects
- [x] Reject unresolved local types at their declaration with an annotation hint;
      resolve nested generic evidence consistently within one initializer
- [ ] Expand inference beyond this first local implementation: specify systematic
      expected-type propagation through calls, constructors, collections, and
      future callbacks; avoid unrelated one-off inference exceptions
- [ ] Evaluate explicit inference variables and constraint solving before allowing
      later uses or assignments to resolve incomplete local types; decide whether
      declaration-local resolution remains the default language rule
- [ ] Evaluate delayed numeric defaulting, generic function inference, and effect
      inference separately, retaining explicit public API and domain contracts
- [ ] Specify determinism, principal types where applicable, ambiguity escape
      hatches, diagnostic quality, and compile-time budgets for each expansion
- [ ] Add unused-binding diagnostics to help catch misspelled assignments that
      become new immutable declarations under plain `=` syntax
- [ ] Define a structured diagnostic model with stable error codes, primary and
      secondary source spans, inferred-versus-expected types, and causal chains
- [ ] Make type errors explain the mismatch in source terms and suggest a concrete
      fix when the compiler can do so safely
- [ ] Add machine-applicable fixes for unambiguous cases and test that applying a
      suggested fix produces a valid program
- [ ] Build a diagnostic conformance suite covering usefulness, source accuracy,
      recovery after an error, and avoidance of misleading follow-on errors

### Type-inference research candidates

Agreed for the backlog on 2026-10-02. These are unscheduled research ideas beyond
[#170](https://github.com/sproates/panackelty/issues/170)'s inference foundation,
outside programme #180 and its completion criteria. Begin with prior-work and
contract assessment; select prototypes separately when prerequisites are clear.
Novelty is unestablished. Effort and PR counts await assessment; no implementation
or publication commitment follows from recording these candidates.

| Candidate | Research question and bounded acceptance |
| --- | --- |
| <a id="rm-94"></a>**RM#94: Edit-stable inference** · [GI#215: Edit-stable inference](https://github.com/sproates/panackelty/issues/215) | Define guarantees that selected edits preserve inferred types and operation meanings, and find sufficient annotation boundaries where needed. Start with expression extraction and expected-type context; compare local/bidirectional inference and refactoring research. Establish preservation within a defined fragment and measure annotation burden, usefulness and cost. Compilation depends on current source, never hidden edit history. |
| <a id="rm-95"></a>**RM#95: Exact arithmetic resource inference** · [GI#216: Exact arithmetic resource inference](https://github.com/sproates/panackelty/issues/216) | Infer conditional numerator/denominator bit-size, intermediate-growth and work bounds under explicit input/iteration assumptions and a normalization/cost model. Compare resource-aware and size analyses, prove supported rules and evaluate precision and overhead. Preserve exact semantics; distinguish proved bounds from measurements and runtime estimates, and report unknowns honestly. |
| <a id="rm-96"></a>**RM#96: Annotation selection** · [GI#217: Annotation selection](https://github.com/sproates/panackelty/issues/217) | Select small, understandable sets of source annotation choices sufficient to resolve ambiguity, with consequences for each. Compare annotation synthesis and interactive/refinement inference; define the candidate language and minimality metric before claiming a minimum. Verify offered choices, retain genuine ambiguity, and measure burden, edit stability, developer usefulness and latency. Never guess intent or silently select semantics. |

Each assessment should identify a precise potentially distinct contribution,
reproducible evaluation and a proceed/defer recommendation, retaining unsupported
cases and limitations. Risks include duplicating prior work, restricting useful
inference, combinatorial cost and overstating guarantees. Literature assessment
can precede production inference; implementation depends on the relevant #170
interfaces and explicit scope selection. Record any supported publication thesis
in the [potential technical publications backlog](#potential-technical-publications).

### Core and standard-library types — planned exploration

<a id="rm-65"></a>

**RM#65: Core and library types**.

Keep the primitive type set small while making common terminal-program concepts
explicit in the standard library. Unfinished entries below are proposals;
completed entries describe accepted features. Prefer portable records and tagged unions; add compiler
or VM support only where representation, checking, or the host boundary requires
it. Coordinate generic-function work with `SELF_HOSTING.md`, JSON work with
the JSON data support initiative below, and host types with the automation and
host capabilities initiative.

Generic source functions, a first-class success value, and exact rational
arithmetic are implemented. Opaque paths, exact durations, and monotonic instants
are now implemented, together with typed filesystem and bounded process APIs,
checked decoding, and sleep. The testing-library foundation is complete. Recursive
filesystem operations remain a separate follow-up.

- [x] Specify and implement generic source functions and explicit type arguments,
      including inference, ambiguity diagnostics, and purity preservation, so
      reusable `Option[T]` and `Result[T,E]` helpers need fewer compiler special
      cases

The first generic-function implementation checks abstract bodies once and erases
type arguments into ordinary version-8 calls. It includes inferred and explicit
complete type arguments, recursion, and portable Option/Result/array helpers.
Constraints, generic function references, partial type arguments, and inference
from expected return types remain deferred.

- [x] Implement first-class singleton `Unit`, written `()`, including generic
      success payloads, collections, and callbacks; keep return-only `Void`
      distinct and lower Unit construction through a pure bytecode-8 builtin
- [x] Implement opaque `Path` with checked text/native-byte construction, lexical
      operations, checked UTF-8 conversion, and escaped display on supported POSIX
      hosts, without implying existence or safe containment
- [x] Implement exact signed nanosecond `Duration` and opaque monotonic `Instant`,
      pure arithmetic, checked fractional conversion, and effectful clock reads
- [x] Add typed filesystem queries and I/O accepting `Path`, returning structured
      errors; preserve arbitrary native filenames through enumeration and access
- [x] Add sleep and timeout APIs with negative-duration and host-range validation;
      evaluate a portable system-suspension policy before promising deadline
      behavior across suspended hosts
- [x] Design structured filesystem and process errors returned through `Result`,
      retaining useful operation and failure details; distinguish a process's
      nonzero exit status from failure to launch it
- [ ] Evaluate a standard-library `Json` tagged union for unvalidated external
      data, with parsing and checked conversion to application records; settle
      numeric representation and recursive-type requirements through the JSON
      initiative rather than introducing an unrestricted dynamic value type
- [ ] Complete explicit decimal rounding with specified precision or scale and
      rounding modes, preserving exact existing arithmetic and requiring an
      explicit choice for non-terminating decimal division
- [x] Implement exact `Rat` with integer `/`, normalized arbitrary-precision
      fractions, arithmetic and comparisons, zero-divisor traps, exact `.nat()`
      and `.dec()` conversions, and explicit natural `quotient` division
- [ ] Extend rational-to-decimal conversion with explicit scale or precision and
      rounding modes alongside decimal rounding; add checked conversion helpers
- [ ] Evaluate tuples for temporary pairs and multiple return values only when
      examples demonstrate a meaningful benefit over named records
- [ ] Explore calendar dates and wall-clock time as a separate library design,
      with explicit timezone and calendar semantics rather than reusing
      monotonic `Instant`

Defer binary floating point, fixed-width integer families, a character primitive,
and a universal `Any` type until concrete programs justify their semantics and
maintenance cost. Each accepted addition needs a written semantics proposal,
representative programs, focused failure tests, public-CLI coverage, and any
required cross-VM and bootstrap evidence before it becomes a supported feature.

### Predictable deferred computation

<a id="rm-66"></a>

**RM#66: Lazy computation**.

Explore `lazy` as a narrow, explicit form of call-by-need evaluation. The first
form should be a typed local binding whose pure initializer is evaluated on its
first read and then memoized. This can avoid unnecessary expensive work without
making I/O timing implicit or committing the language to general closures,
lazy parameters, or lazy collections.

```panackelty
lazy report: Str = build_report(records)

if should_save {
  write_file("report.txt", report)
}
```

- [ ] Specify the syntax, typing, scope, forcing behavior, and at-most-once
      memoization semantics of `lazy` bindings
- [ ] Require lazy initializers to be pure so reading an ordinary value cannot
      unexpectedly perform I/O or another visible effect
- [ ] Define capture semantics conservatively, initially allowing references to
      immutable values while rejecting dependencies on mutable local bindings
- [ ] Specify deterministic handling of initializer traps, including whether a
      failed evaluation is memoized, and diagnose cyclic forcing explicitly
- [ ] Design an internal thunk representation without exposing general closure
      or capture semantics as part of the callable-value model
- [ ] Define bytecode instructions and verifier rules for constructing, forcing,
      and caching lazy values, including result-type and state validation
- [ ] Add compiler, verifier, VM, bootstrap, and public-CLI conformance coverage,
      including unused bindings, repeated reads, traps, cycles, and invalid
      effectful or mutable captures
- [ ] Evaluate lazy parameters, module-level bindings, and lazy collection
      elements separately after representative programs demonstrate a need

### Candidate differentiator: contract-driven automation

<a id="rm-67"></a>

**RM#67: Contract-driven automation**.

The strongest current direction is to make Panackelty a contract-driven language for
reliable automation: programs describe data, effects, and behavioral boundaries
in forms the compiler, test runner, and tooling can all understand. This builds
on guarded types, purity, exact values, and the VM instead of adding an unrelated
headline feature.

- [ ] Explore first-class function and module contracts with preconditions,
      postconditions, invariants, and effect expectations
- [ ] Define which contracts are proven statically, checked at runtime, or used
      to generate tests, and make that boundary visible to the programmer
- [ ] Design built-in contract testing, including generated boundary cases,
      reusable fixtures, deterministic execution, and useful counterexamples
- [ ] Integrate linting and static analysis into the compiler and stable CLI,
      sharing its parser, type information, effects, contracts, and diagnostics
- [ ] Support project policies that can promote selected analyses from advice to
      compilation errors without making default builds noisy
- [ ] Prototype automation-oriented standard-library modules, beginning with
      structured data, HTTP, paths, processes, and browser automation
- [ ] Evaluate browser automation against a real end-to-end program before
      committing to a large ecosystem surface

### Candidate differentiator: explainable values — exploration

<a id="rm-68"></a>

**RM#68: Runtime value provenance** · [GI#172: Runtime value provenance](https://github.com/sproates/panackelty/issues/172).

Explore built-in value provenance: an opt-in way to explain a result through the
inputs, calculations, function calls, and branch decisions that produced it.
This builds on exact arithmetic, purity, persistent values, and the single VM
execution model. The intended benefit is practical debugging and inspectable
numerical results: users can ask where a total came from or which condition
selected a value.

`explain(value)` is a working design sketch, not accepted syntax or an available
feature. Tracking must start before the relevant calculation; explanations cannot
recover execution history that was never recorded. Explanations describe recorded
execution dependencies, not a proof that the program's business logic is correct.

- [ ] Specify an opt-in first version covering scalar calculations, function
      arguments and results, and the branch conditions that selected a result
- [ ] Decide the source and CLI interface for enabling tracking and requesting
      explanations, including how explanation output respects the purity boundary
- [ ] Design VM dependency records and compiler source mappings while preserving
      exact values, ordinary program behavior, and bytecode verification
- [ ] Render concise explanations with values, operations, decision outcomes, and
      source locations; validate usefulness against an incorrect invoice total
      and a value selected by an unexpected branch
- [ ] Define tracking scope, retention, and memory limits for loops, recursion,
      and collections; report truncated or unavailable history explicitly
- [ ] Define treatment of sensitive inputs and redaction before explanations can
      be saved or shared; avoid exposing input contents by default
- [ ] Measure execution and memory overhead with tracking enabled and disabled
      before deciding whether to expand the initial scope
- [ ] Add compiler, verifier, VM, and public-CLI coverage for explanation accuracy,
      control dependencies, source locations, limits, and unchanged value semantics
- [ ] Evaluate later extensions separately: structured input origins such as CSV
      rows and columns, collection provenance, exported explanations, and comparison
      of recorded dependencies across runs

Related work includes [language-integrated provenance in Links](https://arxiv.org/abs/1607.04104)
and [Whyline for Java](https://www.cs.cmu.edu/~NatProg/whyline-java.html).
The proposed distinction is approachable explanations for ordinary calculations
and control flow in the standard language toolchain, not a claim to have invented
provenance or causal debugging.

### Candidate differentiator: previewable effects — exploration

<a id="rm-69"></a>

**RM#69: Previewable effects**.

Explore a VM-enforced preview mode for dependable scripts. A program would
produce an inspectable plan of supported changes before applying them, including
content diffs and the inputs on which those changes depend. Proposed commands
such as `panack plan script.panack -o changes.plan` and
`panack apply changes.plan` are design sketches, not available CLI features.

- [ ] Specify an opt-in first version for local file reads and writes, with
      staged writes in a simulated filesystem so subsequent reads observe them
- [ ] Define the supported host effects and enforce coverage at the VM boundary,
      including nested execution; stop explicitly on unsupported effects rather
      than silently executing external commands or remote mutations
- [ ] Define permitted reads during planning and handling of terminal output,
      environment values, arguments, path resolution, and filesystem queries
- [ ] Render file changes and content diffs and save the concrete operations for
      later application without rerunning the program against new inputs
- [ ] Record relevant input and destination preconditions; reject stale plans
      and define race handling between validation and application, including
      symlinks and concurrent filesystem changes
- [ ] Specify a versioned, validated plan format, binding plans to their execution
      assumptions and defining handling of sensitive contents and file permissions
- [ ] Define partial-failure reporting and recovery during application; do not
      imply that a sequence of filesystem operations is automatically atomic
- [ ] Preserve purity and bytecode safety boundaries and assess planning overhead
- [ ] Add VM and public-CLI coverage proving that preview leaves target files
      unchanged, staged reads are coherent, diffs match applied changes, stale
      plans fail, and unsupported effects cannot bypass preview enforcement
- [ ] Evaluate moves, deletes, external adapters, and links to value explanations
      separately after the local read/write workflow is proven useful

Related work includes [Terraform saved plans](https://developer.hashicorp.com/terraform/cli/commands/plan)
and [PowerShell ShouldProcess and WhatIf](https://learn.microsoft.com/en-us/powershell/scripting/learn/deep-dives/everything-about-shouldprocess).
The intended distinction is VM-enforced planning for a defined set of effects in
ordinary imperative scripts, with explicit limits on what can be simulated.

### Candidate differentiator: resumable execution — exploration

<a id="rm-70"></a>

**RM#70: Durable execution**.

Explore opt-in durable execution for ordinary local scripts: preserve progress
across interruptions without requiring users to implement their own progress
store or operate a separate workflow service. A batch conversion or import
should recover recorded work and continue from a supported checkpoint.
Proposed commands such as `panack run import.panack --durable import.run` and
`panack resume import.run` are design sketches, not available CLI features.

- [ ] Specify a first version with explicit checkpoints, serialisable VM state,
      and a small, documented set of recoverable local file operations
- [ ] Define checkpoint placement and state capture, including call frames,
      local values, persistent collections, and the treatment of unsupported
      resources and nested VM execution
- [ ] Bind recovery to the exact bytecode and compatible runtime/checkpoint
      versions; reject incompatible code rather than silently resuming it
- [ ] Specify durable, crash-consistent checkpoint and operation records, with
      validation of untrusted or corrupted state and exclusive ownership of a run
- [ ] Define how recorded arguments, environment values, read results, and changed
      external inputs affect recovery while preserving the purity boundary
- [ ] Reuse durably recorded completed-operation results; distinguish operations
      safe to repeat from operations requiring explicit reconciliation
- [ ] Handle the crash window between an external action succeeding and its
      completion being recorded; stop on an unknown outcome unless a supported
      recovery protocol can establish it, without claiming universal exactly-once
      execution or silently repeating an unsafe action
- [ ] Define cancellation, failed-run inspection, checkpoint retention, sensitive
      state handling, and clear CLI reports of recovered and pending work
- [ ] Add VM and public-CLI tests with interruptions around checkpoints and effect
      recording, including corrupted state, changed bytecode, concurrent resume,
      repeated recovery, and supported file-operation failure cases
- [ ] Measure checkpoint size, storage growth, and execution overhead on a
      representative batch-processing script before broadening the scope
- [ ] Evaluate automatic checkpoints, durable waits, remote-service adapters,
      code migration, and integration with preview plans and explanations later

Related work includes [DBOS workflow recovery](https://docs.dbos.dev/production/workflow-recovery)
and [Temporal durable execution](https://assets.temporal.io/durable-execution.pdf).
The intended distinction is a local workflow integrated with the normal language
runtime and command, with explicit recovery guarantees for supported operations.

### Candidate differentiator: enforceable data-flow restrictions — exploration

<a id="rm-71"></a>

**RM#71: Data-flow restrictions**.

Explore data that carries enforceable rules about where its information may go.
Restricted inputs would retain their confidentiality policies through function
calls, transformations, collections, and control flow. The compiler and VM host
boundary would reject disallowed output, such as logging a credential embedded in
request headers, and explain the source and destination of the prohibited flow.
This is a proposed capability, not an implemented security guarantee or accepted
syntax. It complements purity by constraining where effectful code may send data.

- [ ] Define a scoped first version with confidentiality labels, explicit allowed
      output destinations or trusted operations, and compiler-checked propagation
      through ordinary values, calls, records, tagged unions, and collections
- [ ] Specify label inference and policy composition when values with different
      restrictions are combined; encoding, hashing, interpolation, and container
      construction must not silently remove restrictions
- [ ] Track implicit flows through branch conditions and other control dependencies,
      including output whose occurrence reveals restricted information; define
      treatment of errors, traps, and program termination explicitly
- [ ] Define trusted policy declarations and authority for intentional disclosure
      (declassification); ordinary helpers must not grant themselves permission
      to weaken a policy, and sanitising a value must not imply automatic release
- [ ] Specify the relationship between permitted recipients and permitted operations;
      an authentication-only credential must not become generally printable merely
      because the destination is trusted
- [ ] Enforce policies at terminal, file, and other supported host boundaries,
      including nested execution; specify validation or runtime enforcement for
      untrusted bytecode so source-level checking cannot be bypassed
- [ ] Preserve the existing purity boundary and define module and callable contracts
      so passing restricted data to a helper retains the applicable restrictions
- [ ] Design diagnostics that identify the restricted source, propagation path, and
      prohibited destination without including the sensitive value itself
- [ ] Apply the same policies to future explanations, preview artifacts, checkpoints,
      and diagnostic exports so tooling does not introduce an alternate output path
- [ ] State the threat model and limits, including timing and resource side channels,
      native integrations, and behaviour after an authorised recipient receives data;
      do not promise unrestricted non-disclosure across all possible observations
- [ ] Validate a credential-use workflow, accidental header logging, encoded and
      nested values, secret-dependent output, combined policies, and authorised
      disclosure with compiler, verifier, VM, and public-CLI tests
- [ ] Measure annotation burden and runtime overhead before expanding the scope;
      evaluate dynamic policies, remote-service adapters, and integrity labels
      separately after the initial confidentiality model is practical

Related work includes [Jif information-flow checking and controlled disclosure](https://www.cs.cornell.edu/jif/doc/jif-3.3.0/label_checking.html).
The intended distinction is approachable policy-carrying data in ordinary scripts,
with useful diagnostics and consistent enforcement across language tooling, not
an invention of information-flow security.

### Candidate differentiator: change contracts — exploration

<a id="rm-72"></a>

**RM#72: Change contracts**.

Explore executable contracts describing which behavioural differences are allowed
between a new implementation and a pinned older version. Users could require an
optimisation to preserve results, permit a feature change only for selected inputs,
or broaden accepted input while retaining existing meanings and rejection rules.
Proposed forms such as `change ... against ...`, `preserve always`, and conditional
preservation are design sketches, not accepted syntax or available verification.

- [ ] Start with pure functions, explicit old/new bindings, result comparison,
      generated inputs, and concrete counterexamples for unconditional preservation
- [ ] Pin the baseline to an immutable artifact with its dependencies and execution
      semantics; define symbol matching and reject incompatible signatures or
      unsupported runtime versions explicitly
- [ ] Specify observable equivalence, including collection order, tagged results,
      traps, and termination; distinguish value equality from changes in timing or
      resource use and define how timeouts affect conclusions
- [ ] Design conditional preservation and required new behaviour using pure
      predicates; allowing a difference must not by itself establish that the new
      behaviour is correct, and overlapping or uncovered conditions need clear rules
- [ ] Support explicit finite input domains for exhaustive bounded checks and
      reproducible generated tests with seeds, search budgets, and domain constraints
- [ ] Report proved, exhaustively checked within bounds, counterexample found,
      no counterexample found by testing, and unresolved as distinct outcomes;
      never present testing or a timeout as a universal proof
- [ ] Preserve domain guards and exact numeric semantics in input generation and
      any future solver encoding; reject unsupported operations rather than silently
      approximate the language's behaviour
- [ ] Render counterexamples with inputs, old/new outcomes, and the violated rule;
      evaluate reduction and regression-test export while respecting data restrictions
- [ ] Specify contract placement, baseline acquisition, and stable CLI/CI handling,
      including explicit policy for unresolved checks and trusted baseline execution
- [ ] Validate representative changes: stable-order duplicate removal, a new member
      delivery benefit that preserves non-member fees, and a configuration default
      that accepts missing fields without accepting explicitly invalid values
- [ ] Add compiler and public-CLI coverage for preservation rules, baseline failures,
      counterexamples, deterministic search, and honest reporting of bounded or
      incomplete checks; measure cost before making checks part of routine builds
- [ ] Evaluate proof support for a carefully defined subset after the testing
      workflow is useful, with explicit assumptions and sound result reporting
- [ ] Later compare structured preview plans against the same simulated inputs,
      including permitted additional deletions and preserved writes or moves;
      limit claims to effects faithfully modelled by preview mode

Related work includes [SymDiff differential program verification](https://www.microsoft.com/en-us/research/project/symdiff-differential-program-verifier/).
The intended distinction is approachable change boundaries in normal development
and release workflows, not a claim that arbitrary program equivalence is decidable.

### JSON data support — exploration

<a id="rm-73"></a>

**RM#73: JSON support**.

Explore a standard `Json` tagged value type and a coherent library workflow for
configuration, data transformations, and future API clients: parse external text,
validate it into domain types, work with ordinary typed values, and encode results.
Keep dynamic JSON objects distinct from statically checked records. Prioritise
parsing, access, encoding, and typed decoding before special literal syntax;
compiler assistance should serve typed integration where needed. These are design
priorities, not implemented features or accepted API syntax.

- [ ] Specify JSON null, booleans, exact numbers, strings, arrays, and string-keyed
      objects; preserve exact numeric values without implicit binary floating-point
      conversion, including exponent notation and documented resource limits
- [ ] Design pure parsing and encoding APIs with structured results; distinguish
      malformed JSON from valid JSON that fails domain validation
- [ ] Support direct inspection and persistent updates for small transformations,
      keeping missing fields, explicit null, and wrong value types distinct
- [ ] Design typed decoding for records, collections, optional fields, and guarded
      types, with explicit policies for missing, null, and unknown fields; start
      with strict rejection of unknown record fields as the proposed default
- [ ] Treat decoding into guarded types as an explicit checked conversion returning
      success or a structured error; preserve existing proof requirements for
      ordinary conversions and reject unsupported guards explicitly
- [ ] Report parse errors with source locations and decoding errors with field/index
      paths, expected types or guards, and useful descriptions of offending values
- [ ] Reject duplicate object keys by default; preserve input object order for
      readable transformations and offer deterministic sorted-key output
- [ ] Specify escaping, Unicode handling, number formatting, nesting limits, and
      round-trip value semantics; distinguish these from preserving original
      whitespace, escapes, and number spelling in a future document-editing model
- [ ] Validate the workflow with configuration decoding (including an invalid Port)
      and a small dynamic JSON transformation through the public CLI
- [ ] Add unit and functional coverage for malformed input, duplicates, exact-number
      round trips, missing/null distinctions, nested error paths, guarded decoding,
      unknown-field policy, ordering, and resource-limit failures
- [ ] Evaluate JSON literals after the core workflow, including syntax ambiguity,
      interpolation, inferred types, and whether literals construct `Json` values
      or use an explicitly selected typed representation

### Type-driven input handling — exploration

<a id="rm-74"></a>

**RM#74: Type-driven input handling**.

Extend the JSON decoding foundation above into reusable type-driven input handling:
record fields and guarded types should supply a coherent description for input
validation, serialisation, and machine-readable schemas. Begin with JSON and
configuration, then evaluate command-line arguments and other adapters. This is a
proposed direction, not implemented derivation, field-default syntax, or an API.

- [ ] Share field and domain-rule metadata across decoding, validation, encoding,
      and schema generation so independently maintained definitions cannot drift
- [ ] Keep external validation explicit, returning valid domain values or structured
      errors; preserve proof requirements for ordinary guarded-type conversions
- [ ] Define defaults, missing versus null values, unknown fields, coercion, and
      nested error accumulation consistently with the JSON proposal above
- [ ] Specify which guards can be checked and exported to each schema format;
      reject or clearly report unrepresentable constraints rather than weakening them
- [ ] Allow separate input and output models and explicit field mappings; do not
      expose internal or restricted fields automatically through derived encoders
- [ ] Evaluate CLI parsing from the same metadata, including option names, defaults,
      help, and error locations, without making every domain type a CLI interface
- [ ] Validate a server configuration with guarded name and port fields, including
      multiple nested errors and schema changes after a field or guard is revised
- [ ] Add compiler/library and public-CLI coverage for agreement between types,
      validation, and schemas, including unsupported guards and disclosure policies

Related work includes [Pydantic validation](https://pydantic.dev/docs/validation/latest/concepts/json/)
and [schema generation](https://pydantic.dev/docs/validation/2.9/concepts/json_schema/).
The goal is a native connection between Panackelty domain types and external data,
building on the JSON backlog rather than a second independent validation system.

### Typed edits and patches — exploration

<a id="rm-75"></a>

**RM#75: Typed patches**.

Explore scoped editing of persistent values that produces both a new value and a
typed description of its changes. This could support concise nested updates,
configuration previews, undo/redo, and incremental data exchange. An `edit` block
and operations such as `change.value`, `change.patch`, and `change.inverse` are
working sketches, not accepted syntax. This concerns data values, independently
of the more ambitious recovery or correction of external effects.

- [ ] Specify scoped drafts for records and persistent collections; retain the
      original value, prevent draft references from escaping, and preserve purity
- [ ] Define typed field paths, update operations, and patch representations so
      field renames and incompatible value types are checked by the compiler
- [ ] Define patch preconditions and explicit conflict results when applying edits
      to a changed base; inverse patches must check their own applicability too
- [ ] Specify array insertion, removal, and update semantics, distinguishing index
      positions from stable element identities instead of silently conflating them
- [ ] Define patch composition and inversion laws, including no-op edits, overlapping
      updates, and retention of old values needed for inverse operations
- [ ] Preserve domain guards, information-flow restrictions, and exact value
      semantics in drafts, patches, conflicts, and rendered change descriptions
- [ ] Evaluate structural sharing, batched updates, and memory cost without promising
      minimal patches or universally efficient application
- [ ] Validate nested task/settings edits, successful undo/redo, stale-base conflicts,
      and collection edits with compiler, VM, and public-CLI coverage
- [ ] Evaluate serialisable patches and schema/version compatibility separately
      before using them for remote updates or persisted change histories

Related work includes [Immer immutable editing](https://immerjs.github.io/immer/)
and [patches](https://immerjs.github.io/immer/patches/).
The intended distinction is native typed patches with explicit applicability,
composition, and conflict semantics across Panackelty data types.

### Useful execution of unfinished code — exploration

<a id="rm-76"></a>

**RM#76: Typed holes and partial execution**.

Explore typed holes and development execution that keeps completed portions of a
program inspectable while unfinished expressions remain explicitly unresolved.
For example, a report's totals could be inspected while its title is still a hole.
A spelling such as `?report_title` is a design sketch, not accepted source syntax.

- [ ] Define typed holes with expected types and lexical context, retaining useful
      type checking and diagnostics for the completed portions of a program
- [ ] Specify partial evaluation and blocked dependencies without substituting
      guessed values; define holes in conditions, calls, loops, and compound values
- [ ] Start with pure computations and explicit fixture inputs in an opt-in
      development workflow; production checks and builds must reject unresolved holes
- [ ] Keep bytecode and the VM as the execution model; design validated development
      representations for partial values without weakening production verification
- [ ] Inspect completed intermediate values and unresolved dependencies through the
      CLI/editor, observing information-flow restrictions and resource limits
- [ ] Specify effect handling before allowing effectful development execution;
      blocked work must not silently trigger real writes or external operations
- [ ] Validate a partially implemented report and branch-by-branch development,
      including informative expected types and inspection of independent results
- [ ] Add compiler, verifier, VM, and public-CLI tests for partial results, blocked
      control flow, rejection in production mode, and absence of unintended effects
- [ ] Evaluate watch mode and editor integration after a coherent source-file and
      CLI workflow, without requiring a browser-only or structured editor

Related work includes [Hazel's live programming with typed holes](https://hazel.org/).
The goal is useful feedback during ordinary incomplete development in Panackelty's
source-file workflow, with an explicit boundary between partial and runnable code.

### Data and target-platform experiments

<a id="rm-77"></a>

**RM#77: Platform and ecosystem experiments**.

- [x] Establish a bounded WebAssembly-hosted VM profile for the complete-program
      playground; PRs #111, #113 and #115 record feasibility and delivery
- [ ] Assess broader browser application integration, including DOM/Web APIs,
      asynchronous effects and source debugging separately from the playground;
      direct code generation remains an alternative only if evidence warrants it
- [ ] Keep browser execution optional so terminal programs and the native seed VM
      do not inherit unnecessary platform complexity
- [ ] Define ecosystem and standard-library contribution criteria around API
      stability, deterministic tests, security review, and long-term ownership

Before promoting an experiment into the language specification, require a
representative program, a written semantics proposal, implementation and
maintenance estimates, and evidence that it strengthens Panackelty's identity more
than an ordinary library would.

## Interactive REPL — exploration; website learning use case identified

<a id="rm-78"></a>

**RM#78: Interactive REPL**.

Add a read-evaluate-print loop for exploring Panackelty expressions, trying
standard-library APIs and learning the language without creating a source file
for every experiment. This is an idea with no implementation slot or dependency
claim on other work. On 2026-09-30 the user identified trying the language on
the website as a concrete learning workflow. The complete-program browser
playground now provides that starting point; stateful REPL semantics remain
separately scoped. The website content update shipped in PR #108 and the complete-program playground shipped
in PR #115. Stateful REPL work remains unscheduled and should reuse the toolchain.

- [ ] Specify the entry command (for example `panack repl`), expression result
      display, multiline input and incomplete-input detection
- [ ] Define session semantics for bindings, functions, imports, mutation and
      redefinition, including what state survives compilation or runtime errors
- [ ] Compile interactive input to bytecode and execute it through the existing
      native VM, preserving exact numerics, static checks and purity/effect
      boundaries; do not introduce a separate evaluation engine
- [ ] Provide useful diagnostics and session commands for help, reset and exit;
      define interrupt, EOF, history and noninteractive-input behavior
- [ ] Add transcript and failure-recovery tests covering state across inputs,
      multiline definitions, imports, type/purity errors, runtime traps and
      interruption on both supported platforms
- [ ] Document the workflow and ship the REPL in the standalone toolchain,
      with packaging and release smoke coverage

## Delivered milestones — historical detail

The sections below record completed scope. Unfinished extensions are scheduled
only by the priority table above, not by their position in this history.

## Deliver developer preview `0.1.0-alpha.1` — delivered

<a id="rm-79"></a>

**RM#79: Initial developer preview**.

The delivered initial product goal was a public developer preview that lets a new user
download Panackelty, put `panack` on `PATH`, and check, compile, and run a source
file using the downloaded toolchain. The preview is
an explicitly experimental release rather than a claim of language or bytecode
stability.

The initial target matrix is Linux x86-64 and macOS arm64. Each target remains
in the matrix only if its final downloadable artifact can be built and exercised
on that platform in release automation. Windows, additional architectures, and
package-manager distribution must not delay the preview.

### 1. Freeze the preview contract

- [x] Declare the release version `0.1.0-alpha.1` and document what the `alpha`
      stability level promises for source syntax, standard-library APIs, CLI
      behavior, and bytecode compatibility
- [x] Record Linux x86-64 and macOS arm64 as the initial supported targets,
      including the oldest tested operating-system versions and the policy for
      best-effort behavior elsewhere
- [x] Publish the deliberately postponed language features and known test or
      implementation limitations as preview limitations rather than implicit
      promises
- [x] Freeze unrelated language feature work until the preview release gates
      below are satisfied

### 2. Establish the public project boundary

- [x] Choose and add the source and binary distribution license
- [x] Confirm that the project name, documentation, examples, and supplied
      photograph may be published under the chosen terms
- [x] Add concise security reporting, contribution, support, and release-notes
      documents appropriate to an experimental compiler and native runtime
- [x] Audit the publishable tree for secrets, personal data, local configuration,
      generated output, accidental binaries, and material that should remain
      private
- [x] Ensure the license, release notes, and required notices are present in
      both the source repository and every binary archive

### 3. Version the complete toolchain

- [x] Define one canonical source of the Panackelty release version
- [x] Add `panack --version` with focused and public-CLI tests, reporting the
      release version, bytecode version, and enough build provenance to identify
      a published artifact
- [x] Keep version injection deterministic so the stage-2/stage-3 fixed-point
      proof and reproducible package build remain meaningful
- [x] Name artifacts with release, operating system, and architecture, for
      example `panackelty-0.1.0-alpha.1-macos-arm64.tar.gz`

### 4. Produce download-and-run archives

- [x] Make the installed toolchain self-contained, including the launcher,
      native VM, compiler bytecode, standard-library modules, and resource
      discovery outside a source checkout
- [x] Replace the archive's installation-shaped `usr/local` root with a friendly,
      relocatable top-level `panackelty/` directory containing `bin`, `libexec`,
      `share`, documentation, and license files
- [x] Build an archive independently for every supported target containing only
      the runtime toolchain and user-facing files
- [x] Verify that moving the extracted directory does not break compiler or
      standard-library discovery
- [x] Publish SHA-256 checksums and build provenance beside every archive

### 5. Add exact-artifact release gates

- [x] Add a release smoke test that starts from the final archive rather than
      the source or staging installation tree
- [x] On every supported target, unpack the archive into a fresh directory with
      only runtime tools available and no repository checkout
- [x] Require the unpacked toolchain to report its version and help, check a
      source file, compile it, run source and bytecode, pass program arguments,
      import the bundled standard library, and reject malformed bytecode
- [x] Require `make check`, the fixed-point bootstrap proof, native conformance,
      and the exact-artifact smoke test before a release tag can publish assets
- [x] Make the tag-driven release workflow upload only artifacts and checksums
      produced by successful matrix jobs

### 6. Make the first-user workflow usable

- [x] Put a download-and-run quick start before build-from-source instructions
      in `README.md`, covering archive selection, extraction, `PATH`, the first
      program, checking, compilation, execution, upgrade, and removal
- [x] Add a compact language tour that links each preview feature to a runnable
      example and its relevant specification section
- [x] Add at least a primary `file:line:column` location to lexer, parser, name,
      and type diagnostics so a new user can find the reported error; richer
      recovery across errors, stable codes, and automated fixes may
      follow the preview
- [x] Test the published quick start literally in a clean shell and require its
      stated output to match
- [x] Give preview users a clear place and template for actionable bug reports,
      including `panack --version`, host platform, source input, and output

### 7. Publish and verify

- [x] Export the reviewed working tree into a new isolated repository without
      the current `.git` directory, branches, tags, reflogs, remotes, or other
      local history; keep this working repository and its history intact
- [x] Configure publication identity and authenticate the separate personal
      GitHub account only in that isolated repository, without changing global
      Git configuration, the current GitHub login, or files under `~/.ssh`
- [x] Create and inspect one clean initial commit, publish the repository, then
      clone it into a fresh directory and run the documented contributor checks
      plus `make package`
- [x] Publish the annotated `0.1.0-alpha.1` tag and release only after its platform
      matrix and exact-archive gates pass
- [x] Download each public release asset by its published URL, verify its
      checksum, repeat the quick start, and record the evidence in the release
      notes

The developer preview is delivered only when an unaffiliated user can follow
the public quick start on a supported machine and reach this workflow using only
the downloaded archive:

```sh
panack --version
panack check hello.panack
panack compile hello.panack
panack run hello.bc
```

The preview does not require Windows support, a single-file executable,
package-manager installation, generic functions,
new automation APIs, complete diagnostic rendering, or a backwards-compatibility
guarantee. Those remain independent follow-up initiatives.

## Ergonomic control flow and collection APIs — complete

<a id="rm-80"></a>

**RM#80: Control flow and collection ergonomics**.

The algorithm examples show several places where the language's surface syntax
is noisier than its semantics. Improve those areas as one staged initiative so
that control flow, persistent collections, strings, and functional operations
form a coherent API rather than a collection of unrelated special cases.

The intended direction is type-directed method syntax such as `memo.has(key)`,
`memo.put(key, value)`, and `text.reverse()`. Collection updates remain
persistent: methods such as `put` and `add` return a new value rather than
mutating their receiver. An `if` without `else` is valid only in statement or
`Void` position; an `if` used as a value remains exhaustive. Semicolons become
optional line terminators but remain available to separate statements on one
line and resolve otherwise ambiguous layouts.

- [x] Specify newline handling, optional-semicolon parsing, ambiguous multiline
      expressions, and the remaining cases where an explicit separator is
      required
- [x] Make `else` optional for statement-position and `Void` `if` expressions
      while retaining mandatory exhaustiveness in value position
- [x] Implement the grammar changes in both compiler frontends with focused
      parser, type-checker, diagnostic, and public-CLI coverage
- [x] Add type-directed method-call syntax and define its interaction with
      existing record field access, generic types, diagnostics, and name lookup
- [x] Expose existing persistent operations as methods, beginning with map
      `has`, `get`, and `put`; set `has` and `add`; and array `append` and
      `concat`
- [x] Expose string operations as methods, including `len`, `slice`,
      `starts_with`, and `reverse`; define `reverse` over Unicode code points to
      match current string indexing semantics
- [x] Decide and document whether legacy free-function spellings remain as a
      bootstrap compatibility layer or are removed in one coordinated migration
- [x] Design callable types and named function references with deterministic
      generic argument and effect inference
- [x] Add generic array `map` and `reduce` operations with pure callback
      contracts, accumulator inference, and persistent results
- [x] Evaluate concise lambda syntax after named callbacks, function types, and
      effect checking are stable rather than special-casing lambdas for
      collection operations
- [x] Migrate the self-hosted compiler, standard library, examples, tests, and
      documentation to optional `else` and newline statement termination
- [x] Migrate those sources to the accepted method and callable APIs as each
      later stage becomes stable
- [x] Add representative programs and complete source, bytecode, bootstrap, and
      cross-VM conformance coverage for the grammar changes
- [x] Complete the same conformance coverage for method calls, callable values,
      and functional collection operations

The callable stage uses explicit non-capturing `@name` references,
`PureFn[...]`/`Fn[...]` types, and `.call(...)`. Array `map` and `reduce` accept
only pure callbacks and lower to ordinary iteration plus verified indirect
calls. Concise lambdas were evaluated but intentionally deferred: introducing
capture and closure lifetime semantics solely as collection shorthand would
weaken the small, explicit callable model. They can be reconsidered alongside
local type inference if representative programs demonstrate a clear need.

## Make imports independent of repository paths — complete

<a id="rm-81"></a>

**RM#81: Logical imports**.

User programs and examples no longer need to know the source-tree location of
the standard library. The canonical `import stdlib/option` and
`import project/shared/module` forms use reserved logical namespaces;
file-relative imports remain quoted. A terminal `.panack` suffix and quoted
logical paths are accepted compatibility spellings. The launcher supplies the
toolchain-owned library root, including from installed layouts, while the
project root is the entry source file's directory.

- [x] Choose and specify the canonical logical-import syntax, including whether
      quotes and the `.panack` suffix are required, optional, or distinguish
      logical imports from file-relative imports
- [x] Define deterministic resolution rules for file-relative, project-local,
      and standard-library modules without depending on the process working
      directory
- [x] Define project-root discovery, search precedence, shadowing, ambiguity,
      path traversal, canonical identity, and useful missing-module diagnostics
- [x] Make the compiler locate bundled standard-library modules in both a source
      checkout and an installed or packaged toolchain
- [x] Preserve load-once and cycle-detection behavior when the same module is
      reachable through different valid import spellings
- [x] Implement the accepted syntax and resolution rules in every compiler and
      loader that remains part of the development and bootstrap workflow
- [x] Migrate examples, tests, compiler sources where appropriate, and
      documentation away from repository-relative standard-library paths
- [x] Add focused and public-CLI coverage for logical standard-library imports,
      project-local imports, installed layouts, ambiguity and shadowing, missing
      modules, invalid paths, cycles, and source/bytecode execution

## Self-hosted development toolchain — complete

<a id="rm-82"></a>

**RM#82: Self-hosted development tools**.

The compiler and test probes run on the native VM. Shell harnesses cover build,
repository and release contracts; fixed independent fixtures and bootstrap
identity checks provide additional evidence. See [the testing guide](tests/README.md)
for suite ownership and commands.

`make policy` enforces the source-tree dependency boundary.
`make check-no-interpreter` starts from a clean build and validates unit and
functional checks, bootstrap, conformance, packaging and release smoke with an
allowlisted `PATH`. Both supported platforms run this proof across five clean
CI partitions, sharing canonical test targets.

## Change Panackelty syntax — complete

<a id="rm-83"></a>

**RM#83: Syntax simplification**.

The accepted syntax removes redundant declaration keywords, uses a colon for
function return types, and distinguishes no-return functions with `Void`.

| Concern | Legacy syntax | Current syntax |
| --- | --- | --- |
| Function | `fn answer(): Nat` | `answer(): Nat` |
| Pure function | `pure fn answer(): Nat` | `pure answer(): Nat` |
| Immutable binding | `let answer: Nat = 42;` | `answer: Nat = 42` |
| Mutable binding | `let mut total: Nat = 0;` | `mut total: Nat = 0` |
| No returned value | `main(): Unit { () }` | `main(): Void {}` |
| Result failure | `Err(message)` | `Error(message)` |

- [x] Record the goals and non-goals of the syntax change
- [x] Write representative before-and-after examples
- [x] Draft and specify the revised lexical and grammar rules
- [x] Resolve declaration ambiguity by retaining mandatory type annotations
- [x] Adopt a clean break and reject the legacy syntax
- [x] Update `SPEC.md` with the accepted syntax and `Void` semantics
- [x] Update the bootstrap lexer, parser, checker, compiler, VM, and tests
- [x] Apply the same syntax to the Panackelty-hosted compiler sources
- [x] Update every example and user-facing command snippet
- [x] Advance the bytecode version for the `Void` value-tag change
