# Panackelty roadmap

This roadmap tracks post-bootstrap language and engineering initiatives. The
completed compiler bootstrap and reproducibility guarantees
are recorded in [SELF_HOSTING.md](SELF_HOSTING.md).

Use the [roadmap decision process](docs/ROADMAP_PROCESS.md) for item states,
assessment criteria, priorities, document ownership and review. Implementation
items require their agreed scope, meaningful tests, affected documentation and
canonical validation; documentation and design items use their applicable checks.

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

## Draft status and next tasks — updated 2026-09-30

**The implementation roadmap remains a draft for discussion.** The decision
process was adopted in PR #83; the assessment was adopted in PR #87. The
12-trial delivery pilot was adopted in PR #88; issue #86 is closed.
Sorting and literal suffix helpers shipped in PR #90. On 2026-09-29 the user
authorised an execution, concurrency and host-integration design investigation
before networking APIs. The report merged in PR #92. The user then authorised the first resumable-VM
feasibility step, with correctness and performance evidence required before
merge review. PR #94 merged after all 21 CI jobs passed; issue #93 is closed. The user then
authorised the bounded task/lifecycle feasibility milestone, now merged in PR #96
with all 21 hosted checks passed and issue #95 closed. The user subsequently
selected an application-facing async design proposal. PR #98 is now merged,
with all 21 hosted checks passed and issue #97 closed. The bounded source-to-VM
await slice shipped in PR #103, and the validation investigation shipped in
PR #105; both passed all 21 hosted Check jobs. The website expansion shipped in PR #108 with live verification on
2026-09-30. The user then selected the bounded VM/compiler boundary audit;
its findings are prepared for review. Remaining incremental/modular build work
is unscheduled. Networking, source spawning and resource
scopes remain draft.

<a id="now-expanded-holistic-and-architectural-gap-assessment"></a>

### Completed: expanded holistic and architectural gap assessment

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

### Future candidate: incremental and modular builds

Work record: [issue #106](https://github.com/sproates/panackelty/issues/106).
State: VM/compiler boundary audit selected and prepared for review below;
remaining cache/design implementation is unscheduled. The initial high-priority
**Next** recommendation was superseded by the user's website selection. Reassess
the bounded cache slice alongside the audit follow-ups; full separate-compilation implementation remains
**Later**, conditional on design evidence. On 2026-09-30 the user requested a backlog task and priority
assessment, not implementation or a committed schedule. This section is the
authoritative status; the issue contains detailed acceptance and failure cases.

Problem: `tests/run_probe.sh` fingerprints every Panackelty source under `src`,
`tests`, `examples` and the selected standard library for each compiled probe.
An unrelated edit can therefore invalidate otherwise reusable compiled tests.
Native C objects already build incrementally and focused checks exist, but the
Panackelty loader combines reachable modules before checking/emission. Separate
source files do not yet provide independently compiled module artifacts.

Bounded task:

1. Measure clean, unchanged warm, unrelated-edit and dependency-edit workflows
   on a named host/toolchain, separating compilation from test execution and
   counting actual rebuilds. Existing 346s clean-check evidence does not quantify
   incremental-cache savings.
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

Size: medium first slice, estimated **2–3 PRs** including tests and documentation:
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

### In review: VM/compiler boundary audit

The user selected this bounded investigation on 2026-09-30, within
[issue #106](https://github.com/sproates/panackelty/issues/106), before choosing
browser or build implementation. The [audit evidence](ARCHITECTURE.md#vmcompiler-boundary-audit--2026-09-30)
shows that the VM already builds and passes native tests and 145 fixed bytecode
cases without compiler source, seed or stdlib. Launcher/package prerequisites
and broader test orchestration are the remaining practical coupling.

No compiler/VM redesign is recommended. A runtime-only packaging/test entry point
could be a small-to-medium one-PR slice; browser feasibility can proceed without
waiting for it, but must address POSIX host integration. Neither is selected yet.
The audit does not complete #106's caching or separate-compilation work.

### Completed: content-led website expansion

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

An embedded playground (edit a complete program, run, view output/diagnostics)
is a separate follow-up investigation. The learning/onboarding use case is a
concrete reason to reassess the previously low-priority REPL idea. Browser VM
execution, limits and diagnostics need feasibility evidence; persistent REPL
sessions add state/redefinition decisions. Neither blocks this content update.

### Remaining recommendations from the pilot — provisional

1. Following website delivery and the VM/compiler boundary audit, reassess bounded build improvements,
   native TCP/timer feasibility and a browser playground against the alternatives.
2. Targeted compiler/API assistance for observed interpolation, numeric proof
   and discovery friction; retain broader diagnostics as an assessed candidate.

These remaining items are recommendations, not implementation authorisation. Source coverage,
namespaces, security tooling and other backlog work remain visible for comparison.

### Developer experience: useful assistance from the compiler

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

### Provisional backlog grouping — not an agreed implementation order

| Status | Initiative | Outcome to assess |
| --- | --- | --- |
| Done | Adopt the roadmap decision process | Merged in PR #83; implementation ordering remains draft |
| Done | [Expanded assessment (#85)](https://github.com/sproates/panackelty/issues/85) | Report adopted in PR #87; issue closed |
| Done | [Delivery pilot (#86)](https://github.com/sproates/panackelty/issues/86) | Report adopted in PR #88; issue closed |
| Done | [Sorting and suffix helpers (#89)](https://github.com/sproates/panackelty/issues/89) | Shipped in PR #90; all 21 CI jobs passed; issue closed |
| Done | [Execution/concurrency/host design (#91)](https://github.com/sproates/panackelty/issues/91) | Report merged in PR #92; later implementation stages remain proposals |
| Done | [Resumable VM feasibility (#93)](https://github.com/sproates/panackelty/issues/93) | PR #94 merged; all 21 hosted gates passed; issue closed |
| Done | [Task/lifecycle feasibility (#95)](https://github.com/sproates/panackelty/issues/95) | PR #96 merged; all 21 hosted checks passed; issue closed |
| Done | [Async interface investigation (#97)](https://github.com/sproates/panackelty/issues/97) | PR #98 merged; all 21 hosted checks passed; bounded await implementation selected in #102 |
| Done | [Bounded async/await (#102)](https://github.com/sproates/panackelty/issues/102) | PR #103 merged; v9 source-to-VM slice and all hosted gates passed |
| Done | [Validation performance investigation (#104)](https://github.com/sproates/panackelty/issues/104) | PR #105 merged; 346s clean sample, all 21 hosted Check jobs passed; issue closed |
| Audit in review; remaining work unscheduled | [Incremental and modular builds (#106)](https://github.com/sproates/panackelty/issues/106) | VM/compiler audit evidence below; dependency-aware reuse and separate-compilation design remain candidates |
| Done | [Website improvement and expansion (#99)](https://github.com/sproates/panackelty/issues/99) | PR #108 merged; content expansion and coordinated website/coverage deployment verified |
| Idea / assess in holistic review | GitHub repository settings and tooling | Inventory current settings and evaluate useful security/quality features, effort and maintenance |
| Idea / assess in holistic review | Language namespaces and standard-library namespacing | Assess language semantics separately from library organisation, then plan dependencies and migration |
| Candidate | `.panack` source coverage | Trustworthy mapping/collection and published compiler, bytecode-tooling and library baselines |
| Candidate | Developer experience and compiler assistance | Practical help for understanding, writing and changing programs; assess useful increments and cost |
| Candidate | Reconcile test evidence and close verified gaps | Accurate matrix and bounded correctness/safety test PRs |
| Candidate | Codebase readability and refactoring | Audit and component-sized improvements with relevant test protection |
| Non-blocking backlog | Validation performance | Retain targets/warnings; weigh further work against developer impact |
| Candidate | Language/library and runtime improvements | Assess JSON, host APIs, inference, numerics and source diagnostics against real workflows |
| Exploration | REPL / browser playground | Website learning use case identified; assess a complete-program playground separately; does not block website content |
| Unscheduled exploration | Broader language ideas | Preserve proposals without giving them automatic implementation slots |
| Delivered | Public native C coverage | Live and verified; `.panack` measurement remains a separate gap |

Known correctness or safety defects should be considered promptly on their
actual risk. Coverage is execution evidence, not proof of assertion quality;
preserve the established validation gates while reviewing future priorities.

### How to turn the backlog into PRs

The [decision process](docs/ROADMAP_PROCESS.md) is authoritative for these rules.

- Complete the holistic assessment before choosing the principal implementation
  initiative. Subsequent work must follow the reviewed priority order, not the
  placement of a detailed proposal in this draft.
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

Work record: [issue #99](https://github.com/sproates/panackelty/issues/99).
The [current decision and acceptance](#completed-content-led-website-expansion) above
owns the state and scope. The initial unscheduled idea was recorded on
2026-09-29; the user selected and authorised the content expansion on 2026-09-30.
The existing static GitHub Pages and coverage flow remains in use. PR #108 was approved, merged and live-verified; the issue is closed.

## Language namespaces — idea

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

Both items remain unscheduled assessment candidates. Preserve existing public
contracts until the namespace design and any migration are explicitly agreed.

## Review GitHub repository settings and tooling — idea

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

The public LLVM report measures the native C VM only. Existing `.panack` tests
exercise the compiler and libraries, but there is no measured source-line or
branch baseline for those files. Publishing C coverage did not close this gap.

The frontend already carries source positions; the emitted `FunctionCode` and
version-8 bytecode contract currently do not carry an instruction-to-source map.
Coverage therefore requires compiler/bytecode/VM design, not just an HTML export.
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

## Grooming gaps and decisions — newly explicit

These earlier findings are retained as supporting context. The broader
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

Retain timing warnings and strong coverage. This work no longer blocks source
coverage, test hardening or readability work; the REPL has no scheduled slot.

| Measurement | Latest recorded evidence | Outstanding issue |
| --- | --- | --- |
| Full cold hosted CI | Five follow-up runs: 88/106/107/103/97s, median 103s | Hosted scheduling/completion has no proven hard upper bound |
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

### Type inference and diagnostic experience

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

### Core and standard-library types — planned exploration

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

- [ ] Investigate an optional browser target, including VM portability, DOM and
      Web API bindings, sandboxing, asynchronous effects, artifact size, and
      source-level debugging
- [ ] Compare a WebAssembly-hosted Panackelty VM with direct code generation before
      selecting a browser execution model
- [ ] Keep browser execution optional so terminal programs and the native seed VM
      do not inherit unnecessary platform complexity
- [ ] Define ecosystem and standard-library contribution criteria around API
      stability, deterministic tests, security review, and long-term ownership

Before promoting an experiment into the language specification, require a
representative program, a written semantics proposal, implementation and
maintenance estimates, and evidence that it strengthens Panackelty's identity more
than an ordinary library would.

## Interactive REPL — exploration; website learning use case identified

Add a read-evaluate-print loop for exploring Panackelty expressions, trying
standard-library APIs and learning the language without creating a source file
for every experiment. This is an idea with no implementation slot or dependency
claim on other work. On 2026-09-30 the user identified trying the language on
the website as a concrete learning workflow. Assess a complete-program browser
playground first; stateful REPL semantics remain separately scoped. The website
content update proceeds independently. If accepted, reuse the existing toolchain.

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
