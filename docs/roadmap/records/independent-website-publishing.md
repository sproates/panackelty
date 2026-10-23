[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Independent website publishing

<a id="rm-14"></a>

**RM#14: Independent website publishing** · [GI#178: Independent website publishing](https://github.com/sproates/panackelty/issues/178).

Work record: [#178](https://github.com/sproates/panackelty/issues/178).

State: **Done — 2026-10-05**.
Scope: maintenance. Website separation is accepted; this selects no next implementation task.
The public `sproates/panackelty-website` repository has been created with an
owner-authorised empty base commit; all website content goes through reviewed
PRs. Main requires current passing website checks, PR-only squash merges,
conversation resolution and linear history, with force pushes/deletion disabled.
The same owner-review exception applies only to `sproates` through PRs;
independent review and explicit approval of each merge remain required.

[Website PR#1: Standalone website](https://github.com/sproates/panackelty-website/pull/1) merged as `66fb316`.
Independent release/archive checks, previews, browser integration and protected
publication are delivered. Core PR#294 removed website source, validation coupling
and the automatic Pages publisher. Website main `7fa4ef8` was published at
`panackelty.com` by [run 37245940764, attempt 3](https://github.com/sproates/panackelty-website/actions/runs/37245940764/attempts/3)
on 2026-10-05. Root/www browser access, playground execution, release history,
coverage compatibility, exact deployed bytes and HTTPS passed. Website
[PR#6: Production acceptance](https://github.com/sproates/panackelty-website/pull/6)
records initial and repeated validation: full runs 100s/98s and routine smoke
runs 92s/70s; observed main-to-verified-staging publication was 118s/130s.
No action cache is configured, so these are initial/repeated measurements, not
evidence of warm-cache speedup. The final routine production run and queue
details are recorded in GI#178. Final owner-phone confirmation is unobserved
additional evidence; it is not represented as passed or required approval. The initial
domain-only cutover failed; core PR#295 and recovery run 37245222845 restored the
original artifact and owner-device access. The retained manual-only recovery
workflow requires the domain to be deliberately returned to core; it cannot
reclaim production itself. The website owns [deferred CI work](https://github.com/sproates/panackelty-website/issues/3);
its broader trials remain deferred.

Website separation completed and was accepted on 2026-10-05. The user's
2026-10-05 decision superseded the earlier release-before-activation prerequisite
and activated [RM#135: Next integration workflow](next-integration-workflow--active-promotion-acceptance-remains.md#rm-135); remaining promotion
and synchronization acceptance is tracked there.

The [website repository](https://github.com/sproates/panackelty-website) owns pages, assets, release-note snapshots,
previews and publishing. Compiler, VM, specification and runtime bundle contracts
remain core responsibilities. Browser adaptation and releases remain in the
browser repository; coverage remains independently published. Core release
creation cannot update the website: a separate reviewed website PR promotes
native/browser artifacts, recorded notes and immutable installer/version pins.

Acceptance for this migration:

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
  [website impact process](../ROADMAP_PROCESS.md#website-impact-and-follow-ups).
  Recording a follow-up never schedules or starts the update. Version lag is
  acceptable only while every published claim remains true for its stated version.

Delivery spans reviewed core and website PRs, including recovery from the initial
cutover failure. The [website acceptance record](https://github.com/sproates/panackelty-website/blob/main/docs/MIGRATION.md)
records artifacts, deployment runs, timings and limitations. No compiler changes
or automatic release-upgrade bot are included. Analytics and broader CI trials
remain unscheduled/deferred in the website backlog.

### Website follow-up register

- **P03a streaming transport prerequisite: Pending release.** Document
  `stdlib/http_stream` as blocking raw transport only when a matching native
  release is available. Preserve the existing async HTTP client's 1 MiB body
  limit in current website examples. Do not claim remote package support from
  this prerequisite. Ownership: core release and website maintainers.

- **GI#200 local package locks: Pending release.** On release of the matching
  native toolchain, update package examples, installation prerequisites and
  capability descriptions for explicit locks, local archive restore and the
  separate Panack package tool. Keep alpha.14 instructions unchanged. Remote
  source claims require P03/P04 acceptance. Ownership: core release and website
  maintainers; evidence is the [P02 acceptance record](../../design/PACKAGE_LOCAL_ACCEPTANCE.md).

- **Alpha.14 methods, HTTP and documentation: Done, 21 October 2026.**
  Website PR #51 published GI#518/GI#525 learning material and the HTTP slice of
  GI#552 after verifying the native alpha.14 and browser 0.2.1 releases. The
  homepage, capabilities, examples, downloads, API Docs, roadmap and methods
  article match the released toolchain. Alpha.13 now appears in Previous releases.
  The core and website HTTP Markdown are aligned. The static server retains
  loopback/`--host`; HEAD receives 405 under the routed four-method contract.

- **Release rebalance and planned scope: Done, 21 October 2026.**
  Website PR #51 incorporates the scope changes from PR #50. Alpha.15–19 remain
  planned, with the GI#106 validation-speed milestone in alpha.15. GI#435's
  string, collection and cross-field guards are planned for alpha.15; further
  guard families remain alpha.19 design work. This supersedes the earlier
  alpha.14–18 assignments and their pending website handovers, without adding
  optional candidates to release scope. Historical article text is retained
  with a dated alpha.14 completion note.

  Website commit `4331330b1a43728a71ad5c708242d69d8d4950c6` passed publication
  and deployed-byte verification in run 39537834581, reusing PR validation run
  39537462276. Live browser checks confirmed the current and previous release
  pages. See [alpha.14 acceptance](../../releases/ALPHA14_RELEASE_ACCEPTANCE.md).

- **GI#162 preview-tool adoption** — Done, 2026-10-15. Core PR #498 supplies
  the generic tool; website PR #42 adopted it with a pinned source installation.
  All 81 assembled assets matched; readiness, error handling, signals, parent loss,
  cleanup and desktop/phone browser checks passed. Owner preview and merge approval
  are recorded; post-merge website Check run 38760731372 passed.
  Published alpha.13 download adoption remains a separate release follow-up.

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
correctness defects under the separate [website process](../ROADMAP_PROCESS.md#website-impact-and-follow-ups).

- <a id="rm-122"></a>**RM#122: Optional one-command installation guidance** ·
  Follow-up to [RM#121 / GI#256](#rm-121) — **Done (2026-10-04)**.
  [PR#259: Optional installation guidance](https://github.com/sproates/panackelty/pull/259)
  merged as `3ee554f`. The optional route preserves all manual alpha.10 commands
  and adds eight independently labelled copy controls. Main checks and
  [Pages publication](https://github.com/sproates/panackelty/actions/runs/37171626046)
  passed. The live page exactly matches merged HTML, including command/link
  targets; the public installer matches reviewed bytes. The owner accepted the
  live phone clipboard/layout verification on 2026-10-04. No platform or release
  support changes. See [acceptance evidence](../../../tests/VALIDATION_PROFILE.md).

- <a id="rm-119"></a>**RM#119: Cookie-free website analytics** ·
  [Website GI#2: Cookie-free website analytics](https://github.com/sproates/panackelty-website/issues/2) — Website-owned; see the [canonical website record](https://github.com/sproates/panackelty-website/blob/main/ROADMAP.md#rm-119).
  Core programme/release prerequisites remain in their original records; this
  ownership move neither starts deferred work nor changes programme weights.

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
  See [verification evidence](../../../tests/VALIDATION_PROFILE.md#website-installation-command-parity--2026-10-04).

- <a id="rm-102"></a>**RM#102: Source-map website adoption** — Website-owned; see the [canonical website record](https://github.com/sproates/panackelty-website/blob/main/ROADMAP.md#rm-102).
  Core programme/release prerequisites remain in their original records; this
  ownership move neither starts deferred work nor changes programme weights.

- [RM#8: Website CI follow-ups](current-status-and-grooming-2026-10-01.md#rm-8) — **Separate coverage host:** website cutover and independent manual report
  refresh verified; owner: delivery author. PR #203 / Pages `36935939432` passed
  live homepage, playground, both old entry points and website identity checks;
  landing layout inspected. Coverage run `36938575919` advanced the report to
  core `5381bc5` without another website deployment. No runtime version change.
  Automatic scheduling, core-only no-publication and timing acceptance remain
  open but deferred under #187 by user decision; they no longer block other work.

- <a id="rm-103"></a>**RM#103: Corrective compiler release adoption** — Website-owned; see the [canonical website record](https://github.com/sproates/panackelty-website/blob/main/ROADMAP.md#rm-103).
  Core programme/release prerequisites remain in their original records; this
  ownership move neither starts deferred work nor changes programme weights.

Record core release prerequisites here and link concrete website work to the
[website roadmap](https://github.com/sproates/panackelty-website/blob/main/ROADMAP.md). The website owns its pages,
claims, promotion and live acceptance; core programme weights remain unchanged.

- <a id="rm-104"></a>**RM#104: Explanation website adoption** — Website-owned; see the [canonical website record](https://github.com/sproates/panackelty-website/blob/main/ROADMAP.md#rm-104).
  Core programme/release prerequisites remain in their original records; this
  ownership move neither starts deferred work nor changes programme weights.

- [RM#11: Programme website demonstrations](current-status-and-grooming-2026-10-01.md#rm-11) — Website implementation is
  tracked in the [website roadmap](https://github.com/sproates/panackelty-website/blob/main/ROADMAP.md#rm-11);
  programme acceptance and weights remain here. **Final website refresh (U9):** promotion pending; owner is the
  maintainer delivering #180 until explicitly handed over. Affected
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
installation friction to the [development workflow assessment](groomed-development-and-engineering-work.md#development-workflow-assessment)
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

The self-contained release archives are suitable inputs for repeatable CI environments, but no maintained container image is currently provided.

Publish a toolchain image to GHCR from validated release artifacts. Verify version/checksum provenance, check/compile/run workflows, bind-mounted projects and output ownership. Use explicit version tags and document digest pinning and base-image maintenance. Publish amd64/arm64 manifests only after both Linux targets pass release gates. Exclude application hosting and runtime-only images.

M / provisionally 1–2 PRs. Depends on existing release gates; native multi-architecture delivery depends on the Linux arm64 proposal. Registry permissions, base-image updates and partial publication recovery need assessment. No urgency or implementation order is agreed.

<a id="homebrew-distribution"></a>

#### Provide Panackelty installation through a Homebrew tap

<a id="rm-17"></a>

**RM#17: Homebrew distribution** · [GI#147: Homebrew distribution](https://github.com/sproates/panackelty/issues/147).

Work record: [#147](https://github.com/sproates/panackelty/issues/147).

Manual archive installation is supported; a maintained Homebrew tap would simplify installation and upgrades for developers.

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
[the validation profile](../../../tests/VALIDATION_PROFILE.md).

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
The [assessment report](../../history/ADOPTION_ASSESSMENT.md) was merged in PR #87; issue
#85 is closed. Its implementation ordering remained provisional. Evidence covers host integration,
execution lifecycle, composition, developer assistance, platform/library
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
- [x] For each finding, record value, affected users, effort including
      tests/docs/maintenance, risk, dependencies, confidence and a smallest
      useful PR or investigation; compare useful assistance with infrastructure
- [x] Reconcile stale roadmap states, record remaining documentation drift
      for scoped follow-up, and apply the repository-led
      [Issues workflow](../ROADMAP_PROCESS.md#github-issues-workflow)
- [x] Present concise findings and a provisional Now / Next / Later comparison
      in chat; identify prerequisite decisions and deliberate deferrals

Acceptance: evidence-backed assessment across the intended application range,
with effort and architectural uncertainty explicit. It does not promise all
platforms or authorise implementation of every identified gap.

### Completed: sorting and literal suffix helpers

<a id="rm-21"></a>

**RM#21: Sorting and suffix helpers** · [GI#89: Sorting and suffix helpers](https://github.com/sproates/panackelty/issues/89).

Work record: [issue #89](https://github.com/sproates/panackelty/issues/89).
State: Done. PR #90 merged after all 21 CI jobs passed, including the three
sanitizer partitions and Linux/macOS packages; issue #89 is closed.

The user selected this S–M item on 2026-09-29. Additive portable sorting and
suffix APIs reduce application code while keeping compatibility risk low.

Scope: stable generic `array_sort_by` with a pure strict-order comparator and
literal, case-sensitive `text_ends_with`; tests, contracts and an executable
example. No syntax, VM, namespace, networking or diagnostics changes. Acceptance
requires `make check`, comparator/Unicode edge cases, unchanged input and equal-key
order.

Focused evidence: 24 direct library assertions and three comparator type/effect
contracts pass. The collections case passes source,
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
State: Done. The [design report](../../design/EXECUTION_CONCURRENCY_DESIGN.md) merged in
PR #92 after all 21 CI jobs passed; issue #91 is closed. The report remains a
proposal for later stages rather than a frozen public language contract.

Reason: networking was absent, and the VM's synchronous
execution and host ownership make a concurrency decision a prerequisite to
coherent public networking APIs. Compare blocking calls, callback/event loops,
cooperative tasks with explicit suspension, and threads against server, daemon,
GUI, game, browser and mobile workflows. Weigh effort, safety and readable code
for developers rather than selecting syntax by familiarity alone.

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
[full feasibility report](../../../tests/VALIDATION_PROFILE.md#resumable-vm-feasibility--2026-09-29)
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

Decision: prioritise enabling network services and host-driven applications. Targeted compiler/API
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
review. The [internal contract](../../../src/vm/README.md#internal-task-lifecycle-experiment)
defines observable ordering and bounds. Later syntax/effects and native transport
work require separate decisions.

Evidence: direct lifecycle contracts, 177 VM probe assertions and 1,844 native
allocation-failure injections pass, together with local VM/oracle/runner sanitizer
suites. A clean validation sample took 103s (unit 69s); the existing unit-budget
warning remains non-blocking. The [feasibility record](../../../tests/VALIDATION_PROFILE.md#tasklifecycle-feasibility--2026-09-29)
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

The [proposal](../../design/ASYNC_PROGRAMMING_DESIGN.md) compares a finite 16-client
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
[measurement report](../../../tests/VALIDATION_PROFILE.md#clean-validation-comparison--2026-09-29)
for phase evidence, method and limits. Further compiler/runtime optimisation
requires separate evidence and scope; this investigation does not promise 120s.

The bounded overlap candidate passed in 346s (unit 224s), saving 16s / 4.4%
against merged async in one clean sample, with all 1,390 PASS observations
retained. At the time, the 120s full-check and 15s unit comparisons were unmet;
bootstrap also warned at 61s against 60s. PR #105 merged at `dd7306b` with explicit approval on
2026-09-30 (Europe/Gibraltar), after all 21 hosted Check jobs passed; issue #104
is closed. The timing figures remain in the historical record; current NFR status
is governed by the 2026-10-12 owner direction under GI#106 below.

### Incremental and modular builds

<a id="rm-28"></a>

**RM#28: Incremental and modular builds** · [GI#106: Incremental and modular builds](https://github.com/sproates/panackelty/issues/106).

<a id="future-candidate-incremental-and-modular-builds"></a>

Work record: [issue #106](https://github.com/sproates/panackelty/issues/106).

**Current Alpha.13 disposition — accepted by the user on 2026-10-15:** the user
said the current workflow is acceptable for now and no further work is needed for
Alpha.13. This satisfies the user-defined Alpha.13 acceptance criterion. Keep
GI#106 open as a broader tracker, with no active follow-up; resume only if the
user explicitly raises it. Statements below that say acceptance is pending are
historical snapshots and are superseded for Alpha.13 by this disposition.

The VM/compiler boundary audit completed in PR #109. The bounded validation
slice is **Done**, effective on its delivery PR merge; see the
[completion record](current-status-and-grooming-2026-10-01.md#completed-modular-validation-and-component-boundaries).
Shared local/CI selection, the audited process route, explicit component
ownership, conservative integration selection and failure regressions are
implemented. The supported native/Panackelty/POSIX tooling, canonical full
checks, bootstrap, sanitizer, release and stable required-check contracts remain.

The reproducible validation/build baseline is **Done on this delivery's merge**
within [RM#123: Standing performance engineering](standing-performance-engineering--ongoing.md#rm-123). The bounded POSIX
[harness](../../../tests/README.md#reproducible-build-and-validation-baseline) retains an
exact isolated candidate archive, environment/seed/VM identities and repeated
clean/full, unchanged warm/focused and unrelated/direct/transitive edit observations.
All 18 observations pass, with correct outputs and explicit compile/execution counts;
the [measured evidence and reviewed disposition](../../../tests/VALIDATION_PROFILE.md#reproducible-build-baseline)
retain observations against the then-used full/focused comparisons. No post-merge acceptance is needed for this
measurement slice; the broader GI#106 remains open. Website impact: none, because
this changes contributor measurement tooling, not public language/release claims.
At that baseline delivery, no optimisation or compiler redesign was selected.
The later owner direction below changes that status; completing the baseline
slice still did not close GI#106.

**Owner-directed scope and acceptance refresh (2026-10-12):** first inspect the
complete local-to-CI workflow, its run frequency, repeated work, queue and
review/reporting waits, test-to-behavior evidence, and the compiler/stdlib/
bytecode/VM/runtime/package boundaries that determine what could regress. Assess
whether test selection or cadence should change while preserving regression-risk
coverage and running every selected test's assertions. Reevaluate the old
120s/15s/75s/60s figures as historical data. Replacement NFRs and build/test changes
are proposals for the user's decision, not acceptance criteria the task may set
for itself. Only the user's explicit statement that they are happy completes
GI#106. The owner selected dependency-aware compiled-probe reuse (candidate 2)
and a separate-compilation design (candidate 3), while retaining every test
assertion and full validation. The probe-reuse pilot is implemented locally on
`feature/dependency-aware-probes`; a matched unrelated-edit sample shows a
bounded compiler-probe win. The host-enabled local `make check` passed in 421s
on a reused build tree (unit 241s, functional 13s, bootstrap 137s, release smoke
4s, quick-start under 1s). Its historical budget warnings are not current
acceptance gates, and this is not a clean-check speedup comparison.
The separate-compilation work is design-only, captured in
[`docs/design/GI106_SEPARATE_COMPILATION_DESIGN.md`](../../design/GI106_SEPARATE_COMPILATION_DESIGN.md).
**Local full-check phase-overlap pilot (2026-10-13):** the serial recipe took
422.80s on a clean macOS arm64 run; overlapping unit (three workers) with
bootstrap (one worker) took 335.77s on the paired clean run. Both passed every
phase, saving 87.03s (20.6%) in this single observation. User CPU time increased
from 672.65s to 718.61s, so the elapsed-time gain uses more concurrent CPU. Cache
state was uncontrolled and this is one pair; it is a promising local pilot, not
a stable cross-host claim or GI#106 acceptance. Details and phase timings are in
the [validation profile](../../../tests/VALIDATION_PROFILE.md#local-full-check-phase-overlap-pilot--2026-10-13).
**Cold compiler integration overlap pilot (2026-10-13):** the compiler-unit and
compiler-check paths now prepare the integration runner once and schedule its
four independent test groups through the bounded worker pool. All 133 assertions
remain. On the same macOS arm64 host, three fresh-cache cold samples fell from a
53s median to 31s (about 42% faster); all baseline and candidate runs passed.
The warm whole-unit difference was only 16s to 14s at one-second granularity,
so no broad-suite gain is claimed. The canonical `make check` passed locally in
244s on the reused build tree (unit 161s, bootstrap 200s overlapped, functional
14s, release smoke 4s, quick start under 1s); this is not a matched clean-check
comparison. CPU use under the extra parallel work was not measured. GI#106
remains open pending the user's explicit acceptance. See the
[cold integration profile](../../../tests/VALIDATION_PROFILE.md#cold-compiler-integration-probe-overlap--2026-10-13).
**Earlier baseline evidence (2026-10-12):** on macOS arm64, a clean `make check`
at two validation workers passed three times with a 608.81s median; the warm
`check-compiler` collection took 114.44s at two workers and 105s at the default
four workers. A directly selected lexer probe executed its 13 assertions in
0.30s with a fresh cache. The original probe edit matrix showed the broad source
fingerprint recompiling a probe after unrelated, direct and transitive source
edits, but its tiny fixture did not establish whole-suite savings. See the
[GI#106 findings and options](../../../tests/VALIDATION_PROFILE.md#gi106-findings-and-options)
and [workflow inventory and measurements](../../../tests/VALIDATION_PROFILE.md#gi106-workflow-inventory).

**Earlier recommendation (superseded for `next` on 2026-10-13):** retain full PR
and post-merge validation and all product-test assertions while GI#106 assesses
the workflow. Three earlier
post-merge push runs failed after successful PR Checks on identical source
trees. The matched runner-environment comparison and added command diagnostics
([PR #391](https://github.com/sproates/panackelty/pull/391),
[PR #392](https://github.com/sproates/panackelty/pull/392), and
[PR #393](https://github.com/sproates/panackelty/pull/393)) did not reproduce
or explain those failures. Keep them visible as unresolved reliability evidence.

**Owner-directed CI cadence change (2026-10-13):** the owner asked for nightly
builds for active branches such as `next` and for pull requests into `next` not
to build distributions. The owner then clarified that full Linux verification,
macOS jobs and repeated merge-push validation should not remain on the `next`
path. The current `next` PR gate is one bounded Linux source/bytecode smoke;
the merge push skips product smoke. Full Linux suites, the macOS matrix and
packages run nightly; the four compiler experiments are omitted from the `next`
PR route. Full Check runs targeting `main` and release validation remain
unchanged.
This changes CI scheduling only; it does not change product behavior, release
artifacts or website claims, so no website follow-up is needed.

**Assessment and evidence:** this changes when several suites detect a
regression, not which assertions exist in the full nightly/release path. A
compiler, runtime, package, bootstrap, sanitizer, coverage, or source/bytecode
conformance failure can reach `next` before its next nightly run. One prior `next` PR Check (run
38336471151) used 49m50s of runner time across the ten-job package/platform
matrix; that matrix also ran tests, so this is not a measured package-only
saving or a wall-time estimate. The focused route's actual hosted cost and
feedback time remain to be measured after the PRs are available. The scheduled
workflow must be merged to the default `main` branch before GitHub will run it.
GI#106 remains open; only the owner's explicit acceptance can complete it.

The four standalone compiler experiments now run on PRs and are skipped on the
same-tree post-merge push ([PR #394](https://github.com/sproates/panackelty/pull/394)).
The matched run record ([PR #395](https://github.com/sproates/panackelty/pull/395))
shows 105 runner-seconds of repeated experiment work avoided, but elapsed Check
time was 8m34s on the PR and 8m30s on the push, so no wall-time improvement was
observed. Local two-to-four-worker tuning improved the focused compiler
collection by about 8%, but it still took about 105s and did not address the
clean-check unit/bootstrap costs. The separate hosted three-to-four-worker
pilot ([PR #397](https://github.com/sproates/panackelty/pull/397)) failed in both
Ubuntu compiler lanes on a timed-out `cli_commands/source` harness invocation;
its lone faster macOS lane is an unmatched observation. The PR was closed
without merging, so hosted compiler validation remains at three workers. Do
not promote four workers based on the local result.

PR #396 merged the Slice 2 component-to-test map, covering compiler,
standard-library, bytecode, VM/runtime, CLI, package/bootstrap and
validation-routing boundaries. The small hosted sample of PR lifecycle and
Check durations distinguishes docs-only from full validation but cannot split
developer active time from waiting or review/reporting. It supports retaining
the existing gates and rejecting the four-worker setting; it does not select a
replacement NFR or justify another optimization by itself.

The selected cache pilot and design work do not resolve which developer and
release scenarios should define acceptable feedback time. During the P4
standard-library migration, capture a real focused fail/pass cycle and its cache
state, then compare it with the clean full check and hosted PR path. Keep active
work distinct from waiting; do not infer suite-wide savings from the artificial
unrelated-edit sample or change test cadence from fixture overlap alone.
Replacement NFRs and acceptance remain for the user to decide; GI#106 is
complete only when the user explicitly says they are happy.

Baseline problem: the former `tests/run_probe.sh` key fingerprinted every
Panackelty source under `src`, `tests`, `examples` and the selected standard
library for each compiled probe, so an unrelated edit invalidated otherwise
reusable bytecode. The local candidate now keys ordinary probes by their static
import closure and falls back to the broad fingerprint on uncertainty. Native C
objects already build incrementally and focused checks exist, but the
Panackelty loader combines reachable modules before checking/emission. Separate
source files do not yet provide independently compiled module artifacts.

Selected bounded cache/design work:

1. The bounded baseline above completes the first measurement step. Attribute
   the representative critical path before proposing replacement timing NFRs;
   compare any candidate against the same clean/warm/edit workloads and preserve
   all assertions. Tiny-fixture rebuild cost does not predict whole-suite savings.
2. **In progress:** dependency-aware compiled-probe reuse now fingerprints the
   entry's static import closure and conservatively falls back to the broad
   source fingerprint when an import cannot be resolved. Focused harness cases
   cover stdlib, project, relative and transitive edges, unrelated reuse,
   corruption and input races. A matched compile-only compiler-contract sample
   reused an artifact in 0.04–0.05s where the broad-key baseline recompiled in
   18.32–18.48s after an unrelated source edit. This is not a representative
   bug-fix result or a clean/full-check speedup; relevant dependency edits still
   rebuild, and normal probe invocations still execute assertions.
3. **Design prepared, implementation not selected:** the separate-compilation
   sketch addresses interface artifacts, generics, guarded types, effects,
   identity, invalidation, linking, deterministic bytecode and bootstrap. A
   future prototype needs the user's separate decision after design review and
   representative measurement.

Evidence to support the user's decision: a workflow and test/change-impact map,
comparable observations for selected improvements, relevant correctness and
safety evidence, and NFR recommendations. Numeric targets and test cadence are
not set by this task. Any implementation still uses applicable canonical and
hosted validation, and each selected test runs its assertions; test-result
caching is not part of the plan. These deliverables do not replace the user's
explicit statement that they are happy. No linker, namespace/export syntax,
package manager or stable module ABI is preselected.

Later cache/design estimate: medium, **2–3 PRs** including tests and documentation:
one or two for dependency evidence and safe reuse (depending on whether compiler
dependency reporting/seed refresh is independently deliverable), then one design
PR. Full separate compilation is large and must be re-estimated after design.

Initial priority comparison — historical recommendation before website selection:

| Candidate / bounded outcome | Why worthwhile and relative priority | Size / estimated PRs |
| --- | --- | --- |
| Dependency-aware reuse and modular-build design (#106) | Proposed Next: observed rebuild friction affects work across the project; bound the effort before committing to a compiler redesign | Medium / 2–3 |
| Native TCP/timer feasibility | Strongest alternative: unlocks network applications; choose first if a near-term network application becomes the main goal or cache measurements show poor value | Medium–large investigation / 1–2, not production networking |
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
browser or build implementation. The [audit evidence](../../internals/ARCHITECTURE.md#vmcompiler-boundary-audit--2026-09-30)
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
The [architecture report](../../internals/ARCHITECTURE.md#browser-playground-feasibility--2026-09-30)
and [measurements](../../../tests/VALIDATION_PROFILE.md#browser-playground-feasibility--2026-09-30)
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

### Developer experience: useful assistance from the compiler

<a id="rm-34"></a>

**RM#34: Compiler assistance**.

The [workflow assessment](groomed-development-and-engineering-work.md#development-workflow-assessment) and
[compiler explanations](groomed-development-and-engineering-work.md#compiler-explanations) now give these ideas bounded
first outcomes. Runtime diagnostics have their [own scope](groomed-development-and-engineering-work.md#source-aware-runtime-errors).
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

The [current grooming table](current-status-and-grooming-2026-10-01.md#groomed-candidates) owns candidate state and the
[comparison](current-status-and-grooming-2026-10-01.md#proposed-first-step) records the recommendation. Existing namespace,
host/library, build and language explorations remain visible in their sections;
none becomes scheduled through its position in this document. Completed milestones
retain their evidence below and in the historical sections.

### How to turn the backlog into PRs

The [decision process](../ROADMAP_PROCESS.md) is authoritative for these rules.

- The architecture assessment is complete. Choose the next principal
  initiative from the current comparison after reviewing its scope and evidence;
  the placement of a detailed proposal does not assign priority.
- Before implementation, define scope, dependencies, explicit non-goals, failure
  cases and acceptance evidence. A design PR may finish with a decision or a
  bounded feasibility result rather than claiming an implemented feature.
- Close a task only against its stated evidence. Record partial completion
  explicitly and link follow-up work; do not infer completeness from test counts
  or broad checklist wording such as “comprehensive”.
- Update status when a PR ships. Keep performance experiments in
  [the profiling report](../../../tests/VALIDATION_PROFILE.md), behavioral evidence in
  [the coverage matrix](../../../tests/COVERAGE.md), and bootstrap history in
  [the self-hosting record](../../internals/SELF_HOSTING.md).
