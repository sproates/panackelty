[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

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

**RM#8: Website CI follow-ups** · [Website GI#3: Website CI follow-ups](https://github.com/sproates/panackelty-website/issues/3).

Active ownership is now the [website roadmap](https://github.com/sproates/panackelty-website/blob/main/ROADMAP.md#rm-8)
and Website GI#3. The dated core evidence below is retained as history.

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
[#187](https://github.com/sproates/panackelty-website/issues/3). On 2026-10-01 the user
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
acceptance requirements owned by the delivery author. #187 stays open; neither
this repair nor the action upgrade establishes the agreed timing budgets.
PR #191 is merged as `67fa8f0`. Production run `36910124129` passed live byte
and provenance verification: validation was 132s excluding observed dispatch,
and merge-to-live wall time was 174s. Automatic follow-up `36910376804` found
the trusted artifact and skipped browsers, packaging and deployment. This proves
duplicate suppression, not warm restoration with changed coverage. The next
repair overlaps bounded artifact-history reads and shortens exact-source polling,
preserving all validation gates. Hosted timing and changed-coverage publication
remain post-merge acceptance owned by the delivery author; #187 and ledger 2/3
remain unchanged until the full outcome is accepted.

PR #192 is merged as `58a07bd`; production run `36912354959` passed live
verification but measured 155s validation / 191s merge-to-live wall. Artifact
lookup fell to 5s; exact-source Check selection and coverage download grew to
50s. Coverage completed before platform packaging, but publication requires
the whole current-main Check. See the
[publication dependency assessment](../../../tests/VALIDATION_PROFILE.md#website-publication-dependency-assessment-2026-10-01)
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
and live verification remain post-merge acceptance owned by the delivery author;
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
split [#178](independent-website-publishing.md#independent-website-publishing).

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

**Included in Alpha.14; release-specific scope undefined.** Owner direction,
2026-10-14: include GI#180 in Alpha.14, but leave the selected workstreams,
deliverables and acceptance criteria for this release undefined. Resume planning
to define a bounded scope; implementation remains unselected. The existing
programme and its five workstreams remain recorded below, with their prior
evidence and acceptance intact.

<a id="compiler-and-runtime-understanding-programme"></a>

Programme tracker: [#180](https://github.com/sproates/panackelty/issues/180).
**Overall: Alpha.14 scope definition planned; implementation paused; 0 of 5
workstreams accepted.** This counts accepted workstreams, not an effort
percentage. The initial shared investigation is
recorded in [ARCHITECTURE.md](../../internals/ARCHITECTURE.md#compiler-and-runtime-understanding-initial-investigation-2026-10-01)
with [reproducible probes](../../../tests/VALIDATION_PROFILE.md#compiler-understanding-probes-2026-10-01).
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
see the [repair measurements](../../../tests/VALIDATION_PROFILE.md#guard-fact-repair-and-test-hardening-2026-10-01). Browser/native release adoption and
live verification remain separately recorded in the website follow-up register.

| Workstream | State | Acceptance focus / progress |
| --- | --- | --- |
| [RM#48: Compiler explanations](groomed-development-and-engineering-work.md#rm-48) · [GI#134: Compiler explanations](https://github.com/sproates/panackelty/issues/134) | Open; Alpha.14 selection undefined | First guarded-subtraction query accepted through #219; full types/effects/proof scope remains open. |
| <a id="rm-91"></a>**RM#91: Compilation provenance** · [GI#173: Compilation provenance](https://github.com/sproates/panackelty/issues/173) | Open; selected for Alpha.14 | U1 attribution and U2 production mapping accepted; U4 connections through checking/lowering remain open. No accepted workstream delivery yet. |
| <a id="rm-92"></a>**RM#92: Counterfactual compilation** · [GI#174: Counterfactual compilation](https://github.com/sproates/panackelty/issues/174) | Open; Alpha.14 selection undefined | Derive sufficient requirements and validate them by actual compilation. No accepted delivery yet. |
| <a id="rm-93"></a>**RM#93: Semantic change prediction** · [GI#175: Semantic change prediction](https://github.com/sproates/panackelty/issues/175) | Open; Alpha.14 selection undefined | [RM#106: Semantic-impact experiment](u0-semantic-impact-experiment--2026-10-03.md#rm-106) establishes bounded direct predictions and positive local non-impact; one-change transitive acceptance and production remain open. |
| [RM#68: Runtime value provenance](language-direction-and-differentiation--exploration.md#rm-68) · [GI#172: Runtime value provenance](https://github.com/sproates/panackelty/issues/172) | Open; Alpha.14 selection undefined | Explain opt-in computation/value derivations with bounded runtime overhead and retention. No accepted delivery yet. |

### Programme delivery register

This register is the implementation plan for #180. It gives a contributor enough
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
[programme tracking rules](../ROADMAP_PROCESS.md#programme-tracking). Do not
interpret its acceptance count as 0% estimated completion.

| ID | Outcome and acceptance | Dependencies | Current state / evidence | Initial size and PR estimate |
| --- | --- | --- | --- | --- |
| <a id="rm-97"></a>**RM#97: Shared programme investigation** (U0) | Complete shared investigation across all five: evidence inventory, source identity, runtime retention, positive non-impact probes, budgets and revised scope estimates | Existing #183 findings | Partial: #183 establishes the inventory; U1 supplies bounded identity/fallback evidence; local replay resolves U2 attribution trust; bounded runtime retention and [positive local non-impact](u0-semantic-impact-experiment--2026-10-03.md#rm-106) are investigated; authenticated external producers and broader semantic dependencies remain | Remaining investigation still required before U5–U7 scope commitments |
| <a id="rm-98"></a>**RM#98: Guard-fact correctness repair** (C0) · [GI#182: Guard-fact correctness repair](https://github.com/sproates/panackelty/issues/182) | Invalidate stale guard facts and preserve valid refreshed guards | None | Accepted: #182 closed by #185; 25 fixed fixtures and 48 generated pairs with VM execution | Delivered |
| <a id="rm-99"></a>**RM#99: Source attribution experiment** (U1) | Prove exact source attribution for a bounded runtime trap; local, imported and generic cases; reject stale/malformed/mismatched maps; compare representations and measure overhead | C0 and existing investigation | Accepted: PR #208 (`de26483`); [experiment, decision and limits](../../../tests/experiments/source_mapping/README.md); no production ABI | Delivered in 1 feasibility PR |
| <a id="rm-100"></a>**RM#100: Production source maps** (U2) | Deliver the production source-map contract chosen from U1: deterministic identity, validation, compatibility and safe missing-map behaviour, with public-CLI tests | U1 design decision | Accepted: frontend #209, emission #214, exact local-replay sidecar and public CLI #218 (`cbae41b`); [contract](../../guides/SOURCE_MAPS.md) and [evidence](../../../tests/VALIDATION_PROFILE.md) | Delivered in 3 PRs |
| [RM#48: Compiler explanations](groomed-development-and-engineering-work.md#rm-48) (U3) | Retain checker evidence and explain every relevant type, effect and proof decision made while checking a function; show actual outcomes, supporting facts and source locations, and explicit reasons when a decision is genuinely unavailable | U1; production attribution from U2 before feature acceptance | Partial explanation families accepted through #466. A working decision inventory and mapping to output, regressions and known gaps is now in `docs/guides/COMPILER_EXPLANATIONS.md`; match uniqueness/payload arity/exhaustiveness coverage is in progress | Re-estimate after remaining decision gaps are closed |
| [RM#91: Compilation provenance](#rm-91) (U4) | Complete #173 compilation provenance, connecting source, checking, lowering and actual emitted bytecode; include generated instructions with unavailable attribution | U2 and relevant U0 design evidence | Unstarted; mapping alone does not complete provenance | Estimate after U1/U3 evidence |
| [RM#92: Counterfactual compilation](#rm-92) (U5) | Complete #174 inferred requirements; recompile proposed requirements and distinguish sufficient conditions from unsupported/minimality claims | Retained checker evidence from U3 and U0 constraint experiments | Unstarted | Estimate after U3 |
| [RM#93: Semantic change prediction](#rm-93) (U6) | Complete #175 semantic change prediction; apply changes and verify direct/transitive effects and claimed non-impact | U3 evidence and U0 dependency experiments; reuse U4/U5 where justified | Production unstarted; [RM#106: Semantic-impact experiment](u0-semantic-impact-experiment--2026-10-03.md#rm-106) supports direct/local claims, not automatic transitive guarantees | Smallest evidence slice: medium / 1 PR, separately selected; broader scope unknown |
| [RM#68: Runtime value provenance](language-direction-and-differentiation--exploration.md#rm-68) (U7) | Complete #172 opt-in runtime value derivations, including retention, privacy and overhead controls | U0 runtime feasibility and U2 source identity; does not need to wait for U5/U6 | Production unstarted; bounded retention experiment and go/no-go recommendation recorded above | First scalar production slice: large / 2–3 PRs, separately authorised |
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

### Next action and delivery handover

**U3 first query: accepted in PR #219 (`f379c50`).** The user selected
this bounded slice after U2 (#218) merged. Delivered command:
`panack explain SOURCE.panack --function NAME` reports the actual Nat subtraction
proof decision, constants/lower bound and guard branch/source from the checker.
It preserves checker diagnostics and explicitly distinguishes whole-project
validity from local proof status. See the [contract](../../guides/COMPILER_EXPLANATIONS.md)
and [validation evidence](../../../tests/VALIDATION_PROFILE.md).

The query has no remaining acceptance; full local validation and all hosted
checks passed, and independent review found no actionable findings.
There is no release/live-site acceptance prerequisite for this core slice.
No child issue closes. The three-outcome grooming review selected bounded
performance work under #106 before the next principal programme feature; see
[the current priority decision](performance-priority-review--2026-10-02.md#performance-priority-review--2026-10-02).
U0 investigations remain open and U2 mapping alone does not complete #173.

A contributor resuming this work should:

1. Read local and repository instructions, this programme section, #180, the
   relevant child issue and the linked investigation/acceptance evidence.
2. Check the actual checkout, current main, open PRs and worktrees. A different
   chat may have advanced the programme. Preserve unrelated work and never infer
   progress from this conversation or a stale branch.
3. Reconcile the register with merged evidence. Check current priorities
   and any unresolved release correctness obligation. If a milestone already has
   an owner or active PR, continue that delivery or agree a separate bounded
   scope rather than duplicating it.
4. Record the selected milestone, branch/PR, responsible maintainer,
   acceptance cases, dependencies and next concrete action in #180. The contributor
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

- Milestone and child issue; state; responsible maintainer.
- Branch, PR, tested commit and merged commit if applicable.
- Accepted criteria and linked commands/results; independent review and findings.
- Decisions, unresolved questions, blockers and remaining acceptance.
- Next concrete action, with enough context for another contributor to execute it.
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
the [accepted contract](../../guides/SOURCE_MAPS.md), not deferred implementation details.

Acceptance evidence includes self-hosted unit/public-CLI tests, independent
actual-VM trap locations and coherent-forgery probes, ordinary/mapped byte
identity, fresh bootstrap fixed points, canonical validation and a compiler-sized
measurement. The [validation profile](../../../tests/VALIDATION_PROFILE.md) records the
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

Acceptance is bounded by the [experiment report](../../../tests/experiments/source_mapping/README.md):
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
2026-10-01. Previews must work independently of contributor tooling.
The user clarified the scope on the same date: a temporary local preview for
the author, not a public URL or permanent staging site. The hosted-publication
proposal is superseded; no hosting provider or contributor account is required.
PR #161 delivers shared assembly, a checksummed portable build, loopback serving
and a tested CI artifact. The follow-up adds one-command build/start with fresh
temporary output, stop/cleanup and explicit rebuild instructions. See
[the local workflow](https://github.com/sproates/panackelty-website#develop-and-preview).

**Done: PR #171 merged on 2026-10-01; #160 is closed.** Local validation passes: 24 website tests, complete
site assembly, a real default-command HTTP/Wasm/start-stop check, and canonical
`make check` (137s; existing non-blocking timing warning). Production is unchanged.
No acceptance remains for the agreed #160 scope.

<a id="rm-101"></a>

**RM#101: Panackelty preview server** · [GI#162: Panackelty preview server](https://github.com/sproates/panackelty/issues/162).

**Done, 2026-10-15**: [#162](https://github.com/sproates/panackelty/issues/162)
provides a generic directory-serving HTTP package and a tool under `tools/`,
with website preview supplying its configuration. Reuse the existing HTTP package
and isolate necessary standard-library additions. Keep the existing website build.
The [implementation proposal](../../design/GI162_STATIC_HTTP_PLAN.md) records the design,
verified prerequisites and delivery sequence. HEAD support is delivered by PR #495
and asynchronous confined reads by PR #496. Continuous service lifecycle is
merged in PR #497, with final `make check` passing in 523 seconds. The generic
`packages/static_http` package and `tools/http-server` executable are implemented;
focused source, bytecode and relocated-installation checks pass. Final `make check`
passes in 413 seconds; PR #498 merged as `90661d4`. The installed command runs bytecode
compiled during installation. Website adoption is implemented in [website PR #42](https://github.com/sproates/panackelty-website/pull/42): 30 automation tests, 81 byte-matched files and 33 browser checks pass. The owner inspected the local preview positively and explicitly approved website
PR #42, merged as `18b4cd5`. Exact-head and post-merge website checks, including
three-engine browser integration, passed. This completes GI#162.
The [plan's performance evidence](../../design/GI162_STATIC_HTTP_PLAN.md#performance-evidence-and-proposed-disposition)
records a material latency regression against Node, accepted for bounded previews
after successful real-site use. The delivery maintainer will revisit it if it obstructs use.

The finite TCP client/server stage is implemented on `main`: client PR #127,
server contract PR #129 and server implementation PR #130. Both operations are included in the published alpha.11 native release and were
absent from alpha.10 downloads. The server completion evidence
is recorded [below](bounded-async-tcp-server.md#bounded-async-tcp-server).

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
   core #158 and the [verified completion record](browserwasm-boundary-investigation--2026-10-01.md#browser-repository-separation--done) in #159.
3. Author-local website previews under #160, delivered through #161 and #171,
   with the [documented local review workflow](#completed-portable-author-local-previews).

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
| [RM#46: Panackelty source coverage](panackelty-source-coverage-programme.md#rm-46) ([GI#131: Panackelty source coverage](https://github.com/sproates/panackelty/issues/131)) | Weighted programme SC-1: SC1–SC6 accepted; 100% complete. Baseline and bounded UTF-8/fallback/duration batch verified live. | Maintain reviewed policy; further corpus expansion is separate work |
| [RM#62: Component readability](codebase-wide-human-readability-and-refactoring--candidate.md#rm-62) ([GI#132: Component readability](https://github.com/sproates/panackelty/issues/132)) | Inspect one component and fix a concrete readability or responsibility problem, with behaviour protection and a short follow-up list. | S–M / 1 for first component |
| [RM#47: Development workflow assessment](groomed-development-and-engineering-work.md#rm-47) ([GI#133: Development workflow assessment](https://github.com/sproates/panackelty/issues/133)) | Observe installation through maintenance, including whether developers can diagnose and fix traps, external failures and incorrect results. | S–M / 1 assessment |
| [RM#48: Compiler explanations](groomed-development-and-engineering-work.md#rm-48) ([GI#134: Compiler explanations](https://github.com/sproates/panackelty/issues/134)) | Expose one useful compiler-backed explanation of checked types, effects or guard facts, including why a case is rejected or unresolved. | M / 1–2 for one query |
| [RM#49: Invariant testing](groomed-development-and-engineering-work.md#rm-49) ([GI#135: Invariant testing](https://github.com/sproates/panackelty/issues/135)) | Extend existing equivalence tests with one bounded, reproducible generated-input or semantics-preserving transformation family. | M / 1–2 for one family |
| [RM#50: Source-aware runtime errors](groomed-development-and-engineering-work.md#rm-50) ([GI#136: Source-aware runtime errors](https://github.com/sproates/panackelty/issues/136)) | Explain a bounded set of runtime failures with source expressions and useful call context; validate through the debugging guide and preserve safe metadata fallback. | M–L / 1–2 after mapping design |
| [RM#51: Learning and debugging guides](groomed-development-and-engineering-work.md#rm-51) ([GI#137: Learning and debugging guides](https://github.com/sproates/panackelty/issues/137)) | Deliver the installation-to-maintenance tutorial and an explicit debugging guide using available tools, with later source-aware updates. | M / 1–2 for first tutorial |
| [RM#52: Executable documentation](groomed-development-and-engineering-work.md#rm-52) ([GI#138: Executable documentation](https://github.com/sproates/panackelty/issues/138)) | Verify documentation commands and outputs, explicitly including the debugging guide’s failing examples, fixes and regression tests for its declared release. | M / 1–2 for one surface |
| [RM#53: Interactive debugger](groomed-development-and-engineering-work.md#rm-53) (no issue yet) | Assess a bounded synchronous CLI experiment for breakpoints, stepping, locals and call frames against observed debugging gaps. | M / 1 assessment; delivery estimate follows evidence |
| [RM#54: Editor support](groomed-development-and-engineering-work.md#rm-54) ([GI#139: Editor support](https://github.com/sproates/panackelty/issues/139)) | Provide highlighting and basic editing in one selected editor; assess compiler-backed features separately. | S–M / 1–2 for one editor |
| [RM#55: Showcase programs](groomed-development-and-engineering-work.md#rm-55) ([GI#140: Showcase programs](https://github.com/sproates/panackelty/issues/140)) | Deliver one complete, tested demonstration combining existing language capabilities and explicit failure boundaries. | M / 1–2 for one demonstration |
| [RM#56: Performance baselines](groomed-development-and-engineering-work.md#rm-56) ([GI#141: Performance baselines](https://github.com/sproates/panackelty/issues/141)) | Planned core baseline under [RM#123](standing-performance-engineering--ongoing.md#rm-123); execution not started. Reproducible compile/run, memory and artifact-size evidence guides separately selected profiling/optimisation. | M / 1–2 for initial baseline; later optimisation separately scoped |
| [RM#57: Contract testing for services and libraries](groomed-development-and-engineering-work.md#rm-57) ([GI#142: Contract testing for services and libraries](https://github.com/sproates/panackelty/issues/142)) | Make contract testing between services or libraries a first-class Panackelty capability; use-case, design and first outcome remain to be discussed. | Unestimated until scope is agreed |

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
[dependency map](../../../tests/README.md#component-dependencies-and-retained-coupling)
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
remains recorded in the [measurement report](../../../tests/VALIDATION_PROFILE.md#modular-validation-route--2026-09-30).

This delivers the first useful slice in one PR. It does not narrow individual
code-component test envelopes or implement cached outcomes, dependency-aware
artifact reuse or separate source-module compilation. Those require independent
evidence; the launcher, probes, bootstrap and packaging retain coupling.

The prior selection favoured this task over source coverage because unnecessary
socket/build dependencies were directly observed during process editing.
Source-coverage feasibility (#131), editor basics (#139) and invariant testing
(#135) should be reassessed next; none is automatically scheduled by completion.

### Additional ideas recorded on 2026-10-01

Entries #164–#169 remain **Idea, unscheduled**. GI#170 is selected for Alpha.14
as an investigation under RM#151; implementation remains unselected. Entries
#172–#175 are **In progress (shared investigation)** within
[programme #180](#compiler-and-runtime-understanding-programme), whose table
above owns current progress. The summaries below retain their scope;
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
| [RM#64: Inference and diagnostics](language-direction-and-differentiation--exploration.md#rm-64) · [GI#170: Inference and diagnostics](https://github.com/sproates/panackelty/issues/170) | Demonstrate current inference limits and assess one predictable, sound expansion. |
| [RM#68: Runtime value provenance](language-direction-and-differentiation--exploration.md#rm-68) · [GI#172: Runtime value provenance](https://github.com/sproates/panackelty/issues/172) | Investigate opt-in runtime value derivations; distinguish provenance from instruction tracing. |
| [RM#91: Compilation provenance](#rm-91) · [GI#173: Compilation provenance](https://github.com/sproates/panackelty/issues/173) | Connect one source construct through checking/lowering to actual emitted bytecode. |
| [RM#92: Counterfactual compilation](#rm-92) · [GI#174: Counterfactual compilation](https://github.com/sproates/panackelty/issues/174) | Derive a sufficient requirement from checker evidence and verify it by recompilation. |
| [RM#93: Semantic change prediction](#rm-93) · [GI#175: Semantic change prediction](https://github.com/sproates/panackelty/issues/175) | Predict direct/transitive proof consequences and verify against an actual change; unaffected claims require evidence. |

Durable resumable execution remains in the existing exploration below: persisted
checkpoints and crash recovery are not delivered by the completed in-memory VM
suspension work. It is distinct from value provenance and remains unscheduled.
#162, the Panackelty-written preview server, remains separate from completed #160.
