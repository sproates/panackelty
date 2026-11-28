# Roadmap

Panackelty **0.1.0-alpha.16 is released**. Native packages, the default installer,
matching browser playground and website are published and verified. See
the [release notes](CHANGELOG.md) and [acceptance evidence](docs/releases/ALPHA16_RELEASE_ACCEPTANCE.md).

## Released baseline: alpha.16 — faster tools and more expressive programs

The active integration branch is `next`. See [the current status and allocations](docs/NEXT_RELEASE_STATUS.md).
Alpha.15 is complete. Alpha.16 scope is closed around the delivered VM/compiler
optimisations, async stdlib, anonymous functions, closures, State and their
examples/documentation. Release packages, installer and playground verification
completed before the final website promotion; live acceptance passed. Unfinished work is allocated to alpha.17.

## Nightly artifact resilience: 26 October 2026

Nightly uploads capture bounded DNS, resolver and HTTPS evidence after each
failure and retry up to twice before failing. Failed attempts remain visible even
when a retry succeeds. The original DNS failure is still unexplained; transient
failure remains a hypothesis. Delivery validation covers native Linux/macOS
upload success, recovery and exhaustion independently of full nightly validation.
See [diagnostics and limits](tests/VALIDATION_PROFILE.md#nightly-artifact-upload-recovery-and-dns-evidence--2026-10-26).
This is CI-only work with no product or website behaviour change. Successful
uploads add no diagnostics or backoff; failed uploads have explicit bounds. No
issue was created for this repair. Completion evidence is recorded in the PR.

## Completed: alpha.15

GI#409: Compiler/stdlib boundary assessment is complete on next (PR#635).
The [assessment](docs/COMPILER_STDLIB_BOUNDARY_ASSESSMENT.md) verifies the full
42-file closure and five change cases. Retain source organisation. Narrower
package-tool prerequisites were considered in GI#106 and remain unselected. Native coupling
is allocated to alpha.17. No refactor is delivered.

GI#106: Validation speed — alpha.15 milestone done. The owner accepted the
implementation and replay on 27 October 2026; implementation is merged in PR#637.
[Acceptance evidence](docs/VALIDATION_SPEED_ACCEPTANCE.md) meets the agreed targets:
clean full validation is 16.1% faster, compiler-edit checks 20.05% faster, and
hosted initial compiler checks 24.9%/36.8% faster. Regression guards passed,
including the ordinary four-worker setting. The bounded alpha.15 milestone is accepted;
the wider GI#106 issue is not closed. No tests or validation cadence were removed.

RM#154: Faster coverage is complete for alpha.15. PR#644 delivers local preparation reuse and PR#645 hosted persistence. Matched local acceptance shows 88.70% faster repeats and 88.75% faster reporting changes; cold/dependency regressions are 0.65%/2.35%, within the agreed 10% limit. Hosted collection fell from 30m22s to 3m35s with all 29 preparations reused and 31 fresh sessions per run. [Evidence and limitations](docs/COVERAGE_PERFORMANCE_BASELINE.md#final-acceptance-28-october-2026). Cold preparation remains slow; redesign is outside this completed optimisation.

For the current release summary, read [Next release status](docs/NEXT_RELEASE_STATUS.md).
Keep that summary current alongside the detailed records when release work changes.

Alpha.14 delivers type-owned methods and dot notation (GI#525), the routed HTTP
server framework (GI#518), and the HTTP reference and runnable examples slice of
GI#552. Alpha.15 delivered the remaining foundation work under the agreed
release rebalance, including package-management implementation (GI#200) and the
GI#106 test-speed milestone.

- [Release plans](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md)
- [All roadmap records](docs/roadmap/README.md)
- [Decision and completion process](docs/roadmap/ROADMAP_PROCESS.md)

## Release direction: alpha.16 through alpha.20

Owner-approved release boundary, 28 November 2026. Alpha.14 and alpha.15 are
released. See the [current allocation and boundaries](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#release-boundary-agreed--28-november-2026).

- **alpha.16:** delivered VM/compiler optimisations and incremental compilation;
  redesigned ordinary/async stdlib; anonymous functions, closures and State;
  tested combined examples and API/concept/cookbook documentation. Published and verified.
- **alpha.17:** the unfinished useful-program work: GI#166 typed HTTP exchanges,
  GI#167 file discovery, GI#168 subprocess examples, GI#169 tiny build tool,
  GI#138/GI#310 tested guides/examples, GI#170 inference assessment, remaining
  RM#157 performance review, GI#435 guard alternatives, native-interface/compiler
  decoupling, coverage approach review, continuing RM#162 real-program fixes and
  [RM#163: Install and run usability](#rm-163).
- **alpha.18:** everyday development: GI#139 editor support, GI#136 source-aware
  runtime errors and GI#147 Homebrew installation.
- **alpha.19:** interactive/connected applications: GI#165 browser event handlers
  and GI#142 service contract testing; Docker images/Linux ARM64 remain optional.
- **alpha.20:** language ergonomics: GI#274 error propagation and further GI#435
  guard families, subject to design/evidence.

Existing alpha.17–alpha.19 allocations each move one release later. GI#552 full
API-reference completion and GI#553 load profiling remain candidates in alpha.17;
GI#555 coloured diagnostics remains a candidate in alpha.18. No additional
implementation scope is created. Completed work stays credited to its release.

<a id="rm-44"></a>

**RM#44: Alpha.15 package-management packets P01–P04.** Complete on merge of
P04c final acceptance. [P01: lock and restore contract](docs/roadmap/records/dependency-management--idea.md#p01),
[P02: locked local dependencies](docs/roadmap/records/dependency-management--idea.md#p02),
[P03: pinned remote sources](docs/roadmap/records/dependency-management--idea.md#p03),
and [P04: `panack` workflow integration](docs/roadmap/records/dependency-management--idea.md#p04)
are sequenced in the [package-management plan](docs/roadmap/records/dependency-management--idea.md#alpha15-implementation-plan).
Their provisional weights are 15%, 25%, 35% and 25% of GI#200, respectively.
P01 is accepted in PR#571. P02 merged as PR#575, taking weighted
completion to 40%. P03a completes on merge of PR#578, taking estimated weighted
completion to 50.5%. P03b completes on merge of PR#580, taking the total to
62.75% under the same P03 allocation. P03c completes on merge of PR#581,
raising the total to 69.75%. [P03d remote-source acceptance](docs/design/PACKAGE_REMOTE_ACCEPTANCE.md)
completed P03, taking the accepted subtotal to 75%. Git and archive diagnostic
corrections retain specific causes and document real CLI output. The final
[P04a diagnostic audit](docs/design/PACKAGE_DIAGNOSTIC_ACCEPTANCE.md) covers all
24 package modules, with 100 exact CLI transcripts and no remaining wording batch.
P04a completes on merge of [PR#593](https://github.com/sproates/panackelty/pull/593) after local and hosted validation,
bringing weighted programme completion to 87.5%. Weights are unchanged;
P04b completes on merge of the installed-workflow delivery, taking the total to
93.75%. Its finite acceptance covers package help, relocated archive operations,
installed HTTPS without Git, native library paths and missing-tool behaviour.
P04c completes [final installed acceptance](docs/design/PACKAGE_FINAL_ACCEPTANCE.md)
on merge, closing GI#569 and GI#200 at 100%. The four-source, relocated and
offline workflow is verified alongside existing failure/containment tests.
Native release publication remains separate. The three tracked slices are:
[P04a diagnostics, P04b installed workflow and P04c final acceptance](docs/roadmap/records/dependency-management--idea.md#p04-delivery-slices). PR#573 reverted the initial P02
implementation to the P01 baseline. The P01
[contract](docs/design/PACKAGE_MANAGEMENT_CONTRACT.md) defines lock and restore
behaviour. The [implementation boundaries](docs/design/PACKAGE_MANAGEMENT_IMPLEMENTATION.md)
require a separately built Panackelty package tool, zlib for gzip and a small
compiler-handoff proof before archive implementation. PR#575 implements the
local workflow; the [acceptance record](docs/design/PACKAGE_LOCAL_ACCEPTANCE.md)
records verification, performance evidence and publication limits.
Website changes wait for a matching native release
and are recorded in the website follow-up register.
P03a starts with the [bounded HTTP streaming prerequisite](docs/design/HTTP_STREAMING_ACCEPTANCE.md),
followed by [HTTPS archive acquisition](docs/design/PACKAGE_HTTPS_ACCEPTANCE.md).
P03b adds [controlled Git acquisition](docs/design/PACKAGE_GIT_ACQUISITION_ACCEPTANCE.md)
in PR#580; prerequisites alone earn no additional programme completion.
[PR#579](https://github.com/sproates/panackelty/pull/579) delivers the approved
P03b generic process/filesystem prerequisite on merge: exact child
environments, confined staged link creation and link-target inspection. This
prepares controlled Git acquisition without moving package policy into native
code. PR#580 stages exact Git commits with isolated configuration, raw blob
extraction, canonical hashes, safe symlinks and cleanup on failure. Public Git
manifests, locks and shared snapshot graphs are delivered by P03c; see its
[integration record](docs/design/PACKAGE_GIT_INTEGRATION_ACCEPTANCE.md). The alpha.17 coupling
task stays separate.

<a id="rm-158"></a> [RM#158](docs/roadmap/records/dependency-management--idea.md#rm-158),
<a id="rm-159"></a> [RM#159](docs/roadmap/records/dependency-management--idea.md#rm-159),
<a id="rm-160"></a> [RM#160](docs/roadmap/records/dependency-management--idea.md#rm-160) and
<a id="rm-161"></a> [RM#161](docs/roadmap/records/dependency-management--idea.md#rm-161).

The original modules/packages/HTTP programme baseline remains **90/100 earned**;
P7 retains its deferred 10-point allocation. Alpha.13 release acceptance is complete.
RM#156 records the GI#518 selection and RM#157 records the performance outcome.
RM#158–RM#161 identify the Alpha.15 package-management packets.
**Next selected work, 28 November 2026:** improve stdlib test coverage for
alpha.17. The [proposed delivery plan](docs/STDLIB_COVERAGE_PLAN.md) inventories
existing tests and measurement gaps before adding cases. Implementation has not
started; this does not start the broader coverage architecture redesign.

<a id="rm-162"></a> **[RM#162: Real-program fixes](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-162)**
continues in alpha.17 after its accepted alpha.16 batches. The ordinary/async
stdlib and anonymous-function, closure and State programmes are complete and
released. See [release acceptance](docs/releases/ALPHA16_RELEASE_ACCEPTANCE.md)
and the [historical implementation snapshot](docs/ALPHA16_IMPLEMENTATION_STATUS.md).
Future real-program fixes remain an implementation allocation with unknown scope
and effort; the delivered batches are not reopened. Installation and program-running
usability is independently tracked as RM#163 below.

<a id="rm-163"></a>

**[RM#163: Install and run usability](docs/roadmap/records/install-and-run-usability.md).** Planned for alpha.17.
Simplify installation, project creation and running third-party programs while
preserving dependency locks and reproducibility. The clean-install HTTP status
service walkthrough is the end-to-end acceptance case. Implementation has not
started; the record defines the agreed outcomes and boundaries.

**Next available roadmap identifier: RM#164.**

## Example inventory preflight — 2026-10-20

The selected authoring-friction improvement adds a compiler-free preflight before
canonical validation builds. It detects stale example counts, ordered smoke lists,
output pairs and transcript totals while retaining independent runtime assertions.
Isolated mutations verify both rejection and correctly maintained additions.
This bounded delivery does not replace full validation or resolve GI#106's broader
performance reassessment. Website: no impact; this is contributor tooling only.
No issue was created solely for this task. Acceptance is recorded in the delivery PR.

## Routed HTTP server: GI#518 / RM#156

**Done on the final acceptance PR's merge into `next`.** All P01–P15 outcomes are
accounted for by the unchanged weighted baseline in the
[implementation plan](docs/design/routed-http-server/implementation-plan.md) and
[acceptance register](docs/design/routed-http-server/acceptance-register.md).
Foundation PR#548, runtime PR#549 and public migration PR#550 deliver typed
ordinary/contextual servers, named route handlers, request accessors, response
writers, bounded framing/Expect handling, request-local failure recovery and
lifecycle ownership. The final acceptance slice preserves committed responses
when a handler exits the process and verifies allocation-failure cleanup,
independent contexts, callback rejection, shutdown escalation and byte boundaries.

The static preview uses a contextual fallback; loopback/`--host`, root confinement,
indexes, redirects, MIME and application limits remain. Obsolete public server
APIs are removed; HTTP client regressions remain covered. Public source/bytecode
and relocated installation/archive examples are included. The migration's full
`make check` passed in 509 seconds; final acceptance evidence and CI are recorded
in the linked register and delivery PR. P01–P15 completion is 100% on final merge.

Alpha.14 publication is separate. The
[website handover](docs/roadmap/records/independent-website-publishing.md#website-follow-up-register)
retains promotion prerequisites and ownership; no website deployment or current
release-pin change is part of this delivery. Existing GI#106/RM#123 performance
work remains open; this delivery does not establish a throughput benchmark.

## HTTP reference review slice: GI#552

The alpha.14 GI#552 slice adds HTTP Server and Client references and an HTTP
playbook: first server, request/response handling, routing, application
configuration, listener lifecycle and static serving. The guides include complete programs and
manifests, curl requests and expected results. Ordinary and contextual examples
run from source and saved bytecode in the existing installed-package checks;
the lifecycle example runs in the HTTP package suite. The original contract
example retains its own assertions, separate from the installed query-header check.

The companion website renders exact Markdown snapshots, with a local cross-repository
comparison and a CI generation check. Both versions target alpha.14 and publish with that release. The client reference
covers GET, custom limits, response/error types and a runnable example; the
playbook includes the bundled static tool and its package integration. This
completes the selected HTTP slice, not the wider standard-library work in GI#552.
Performance: no compiler/runtime changes; additional example assertions reuse
existing server runs, with one small lifecycle program checked in both modes.

## Bounded method experiment — 2026-10-17

Owner-selected proof of concept: two `Fruit` instances call owned `apple` and
`banana` methods using `->`, yielding 10/11/20/21 while existing dot notation is
preserved. The initial local-record slice passed `make check` (432 seconds).
The owner-selected extension adds explicitly public methods callable through
imported records, preserving original type identity through aliases and
re-exports. Private methods remain module-local. Focused import and visibility
checks pass, and the extension passes `make check` (578 seconds), including
400 functional assertions, unit suites, bootstrap and release smoke. The
extended PoC is complete locally. Draft
[PR#524: Type-owned method proof of concept](https://github.com/sproates/panackelty/pull/524)
provides the bounded starting point for [GI#525: Type-owned dispatch migration](https://github.com/sproates/panackelty/issues/525), with the [implementation plan](docs/design/type-owned-dispatch/dispatch-migration-plan.md) included in the PR. The owner approved integrating the PoC into `next`; the migration remains follow-on work. [GI#525 P01 inventory](docs/design/type-owned-dispatch/p01-inventory.md) accounts for all 615 tracked Panack sources and records migration actions for 24,125 call/reference sites. [P02 explicit ownership metadata](docs/design/type-owned-dispatch/p02-owned-metadata.md) replaces spelling-based method lookup with a separate canonical method index and binds each receiver once. [P03 Server/Map coexistence](docs/design/type-owned-dispatch/p03-server-map.md) adds exact-output import/identity fixtures and rejection tests. [P04 generic nominal owners](docs/design/type-owned-dispatch/p04-generic-owners.md) adds record/enum implementation binders, method parameters and receiver/result inference. [P05 built-in ownership registry](docs/design/type-owned-dispatch/p05-builtin-owners.md) restricts built-in source methods to designated toolchain core identity, with canonical family metadata and spoof rejection. [P06 owned intrinsic dispatch](docs/design/type-owned-dispatch/p06-owned-intrinsics.md) adds 30 compiler-provided methods with typed operation identities and upgrades the Server/Map fixtures. [P07 callable and effect contracts](docs/design/type-owned-dispatch/p07-callable-effects.md) adds owned map/reduce/call operations, async methods and HTTP-shaped handler acceptance. [P08 source core methods](docs/design/type-owned-dispatch/p08-core-methods.md) adds 19 source-owned methods and canonical PathError/DurationError re-exports, preserving compatibility wrappers. [P09 audited seed checkpoint](docs/design/type-owned-dispatch/p09-arrow-seed-checkpoint.md) verifies the unchanged saved compiler and library fixed points before caller migration. [P10 stdlib callers](docs/design/type-owned-dispatch/p10-stdlib-callers.md) migrates library receivers and conformance programs, adds TestWorkspace.remove_empty, and records all 32 temporary source wrappers. [P11 remaining callers](docs/design/type-owned-dispatch/p11-remaining-callers.md) migrates compiler, package, tool and executable test callers with a verified new seed and an explicit legacy-test boundary for P12. [P12 legacy removal](docs/design/type-owned-dispatch/p12-legacy-removal.md) deletes receiver-to-global fallback, special collection dispatch and compatibility exports; checked identities own source acceptance and explanation evidence. [P13 owned-dot bridge](docs/design/type-owned-dispatch/p13-owned-dot-bridge.md) routes dots and arrows through the same owned resolver and saves the bridge seed before caller conversion. [P14 dot cutover](docs/design/type-owned-dispatch/p14-dot-cutover.md) converts maintained sources and generated fixtures, rejects receiver arrows and saves the dot-only seed. [P15 consumer acceptance](docs/design/type-owned-dispatch/p15-consumer-acceptance.md) verifies relocated packages and the matched browser distribution, records the final deletion audit and migration guide, and completes P01–P15 on integration. Programme completion is 100% under unchanged stage weights; GI#525 is Done. Native alpha.14 publication and website promotion remain separate release work. The [public method examples](examples/README.md#type-owned-methods-alpha14) now demonstrate custom ownership and generic callbacks alongside built-in dot methods. No HTTP API work is included, and no issue was created solely for this experiment.
The [specification](docs/reference/SPEC.md#type-owned-methods-next)
records its limits. Runtime performance uses existing direct calls; extra work is
confined to parsing and checking arrow calls. Website: no current-release claim
changes are needed for this unreleased experiment; reassess at release selection.

## Compiler fixture progress deadlines: 26 October 2026

The compiler-source fixture now detects stalled module passes and compiler
stages instead of applying one 90-second deadline to a complete compilation.
Function activity does not renew module deadlines. The enclosing process bound
remains a final backstop. A native test supervisor and separately instrumented
test VM retain paths, function observations, timings and original output for
the next investigation; production compiler policy and the seed are unchanged.

Done: normal runtime validation includes progress-budget contracts, deliberate
compiler stalls and recovery; the existing source/bytecode fixture assertions
remain. Hosted workflows retain failure evidence. Full local and hosted
validation results are recorded in the delivery PR. No issue was created solely
for this repair. The original scheduled timeout's cause remains unconfirmed;
focused measurements support cumulative healthy work as a hypothesis only.
This changes hang detection, not the performance acceptance criteria under
GI#106. Performance cost is confined to native test observation and bounded
fault tests. Website impact: none; installation and language behaviour are
unchanged. See [watchdog contracts and evidence](tests/compiler_watchdog/README.md).

## Nightly package-tool preparation repair: 26 October 2026

Nightly run `40176587642` failed package-aware CLI assertions because isolated
compiler, runtime, conformance, sanitizer and coverage suites had not built
`build/tools/panack-packages.bc`. The full `make check` prepared that artifact,
so its successful results did not expose the missing standalone prerequisites.
The affected entry points now prepare the separate package tool before tests
run. Instrumented suites prepare the ordinary CLI artifact before switching
build directories; `make native` remains independent of package management.

Regression checks cover the expanded Make dependency graph and sanitizer
preparation order. Completion requires full local validation and the affected
hosted nightly suites, with results recorded in the delivery PR. No issue was
created solely for this repair. No product behaviour, compiler or seed changes
are included. Performance impact is limited to building the required package
tool once in an otherwise unprepared suite. Website: no impact; released APIs
and installation instructions are unchanged.

Post-merge nightly acceptance passed 16 of 17 jobs. The isolated Linux compiler
job then repeated a separate 90-second timeout in the spaced-checkout
distribution fixture. That fixture now prepares the native VM and package tool
before running archive and release checks. Both commands retain
90-second safety limits; all checksum, installation, relocation and shared-VM
assertions remain. This separates build preparation from the release check
without changing product performance criteria. The follow-up PR records local
validation and the final hosted rerun.
The HTTP-server bytecode target also now depends on the package-tool artifact,
not its phony convenience target. Installation reuses prepared bytecode while
source, seed, VM and package-tool changes still invalidate it. The distribution
fixture asserts that a prepared installation does not invoke compilation.
Manual nightly dispatch accepts a branch name so repairs can be tested before
merge. Scheduled runs still select `next`, and workflow permissions remain
read-only.

## Nightly HTTP fixture repair — 2026-10-19

The 18 October macOS runtime nightly failed its oversized-header HTTP 431
assertion with an empty response while testing `next`. A focused local probe
took 1.21 seconds; lowering its server deadline to one second reproduced the
same empty response and reported `TCP client timed out`. The framing fixture
now allows ten seconds per connection, thirty seconds for admission and twelve
seconds for the oversized-header peer's receive, retaining its twenty-second
outer alarm. The request and response assertions and native deadline tests are
unchanged. This is a test allowance repair, not a runtime performance improvement
or a new latency target. Local acceptance passed the focused source/bytecode
HTTP suite and full `make check` in 640 seconds, including bootstrap, functional
and release smoke. Hosted acceptance is recorded in the delivery PR against the
exact integrated revision. No issue was created solely for
this repair. Website: no impact; production APIs and released claims are unchanged.

## Nightly conformance isolation — 2026-10-19

The macOS ARM64 bytecode-conformance failure in nightly run `39226029349`
was a nested compiler-source timeout. The conformance scheduler now runs the
complete nested fixture corpus after ordinary workers finish, retaining all
assertions and the existing 90-second compiler-source bound. Acceptance and
performance evidence are recorded in [validation profiling](tests/VALIDATION_PROFILE.md#nightly-nested-corpus-isolation--2026-10-19).
The repair completes on integration and verification of the failed hosted lane.
No compiler/VM/seed change or new timing policy is included. No issue was created
solely for this repair. Website: no impact; product behavior is unchanged.

## Existing links

Existing RM links remain valid below and lead to their detailed record.

<details>
<summary>Roadmap identifiers and previous section links</summary>

<a id="roadmap-identifiers"></a> [roadmap-identifiers](docs/roadmap/records/roadmap-identifiers.md#roadmap-identifiers)
<a id="priority-review-policy--2026-10-03"></a> [priority-review-policy--2026-10-03](docs/roadmap/records/priority-review-policy--2026-10-03.md#priority-review-policy--2026-10-03)
<a id="next-release--agreed-scope-2026-10-06"></a> [next-release--agreed-scope-2026-10-06](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#next-release--agreed-scope-2026-10-06)
<a id="rm-143"></a> [rm-143](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-143)
<a id="alpha-13--http-and-expanded-explanations"></a> [alpha-13--http-and-expanded-explanations](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#alpha-13--http-and-expanded-explanations)
<a id="rm-144"></a> [rm-144](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-144)
<a id="alpha13-release-completion--2026-10-15"></a> [alpha13-release-completion--2026-10-15](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#alpha13-release-completion--2026-10-15)
<a id="alpha-14--compiler-understanding-and-correctness"></a> [alpha-14--compiler-understanding-and-correctness](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#alpha-14--compiler-understanding-and-correctness)
<a id="rm-151"></a> [rm-151](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-151)
<a id="rm-152"></a> [rm-152](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-152)
<a id="rm-154"></a> [rm-154](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-154)
<a id="rm-155"></a> [rm-155](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-155)
<a id="rm-156"></a> [rm-156](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-156)
<a id="rm-157"></a> [rm-157](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-157)
<a id="else-if-chains"></a> [else-if-chains](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#else-if-chains)
<a id="rm-142"></a> [rm-142](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-142)
<a id="first-non-alpha-release--milestone-definition"></a> [first-non-alpha-release--milestone-definition](docs/roadmap/records/first-non-alpha-release--milestone-definition.md#first-non-alpha-release--milestone-definition)
<a id="rm-126"></a> [rm-126](docs/roadmap/records/first-non-alpha-release--milestone-definition.md#rm-126)
<a id="standing-performance-engineering--ongoing"></a> [standing-performance-engineering--ongoing](docs/roadmap/records/standing-performance-engineering--ongoing.md#standing-performance-engineering--ongoing)
<a id="rm-123"></a> [rm-123](docs/roadmap/records/standing-performance-engineering--ongoing.md#rm-123)
<a id="continuous-scorecard"></a> [continuous-scorecard](docs/roadmap/records/standing-performance-engineering--ongoing.md#continuous-scorecard)
<a id="finite-core-establishment-tranche"></a> [finite-core-establishment-tranche](docs/roadmap/records/standing-performance-engineering--ongoing.md#finite-core-establishment-tranche)
<a id="rm-124"></a> [rm-124](docs/roadmap/records/standing-performance-engineering--ongoing.md#rm-124)
<a id="rm-125"></a> [rm-125](docs/roadmap/records/standing-performance-engineering--ongoing.md#rm-125)
<a id="ambition-and-adoption-focus"></a> [ambition-and-adoption-focus](docs/roadmap/records/ambition-and-adoption-focus.md#ambition-and-adoption-focus)
<a id="platform-specific-installation-instructions"></a> [platform-specific-installation-instructions](docs/roadmap/records/platform-specific-installation-instructions.md#platform-specific-installation-instructions)
<a id="rm-117"></a> [rm-117](docs/roadmap/records/platform-specific-installation-instructions.md#rm-117)
<a id="modules-packages-and-http-programme-sp1--2026-10-03"></a> [modules-packages-and-http-programme-sp1--2026-10-03](docs/roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#modules-packages-and-http-programme-sp1--2026-10-03)
<a id="rm-108"></a> [rm-108](docs/roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#rm-108)
<a id="rm-149"></a> [rm-149](docs/roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#rm-149)
<a id="rm-150"></a> [rm-150](docs/roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#rm-150)
<a id="rm-109"></a> [rm-109](docs/roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#rm-109)
<a id="rm-110"></a> [rm-110](docs/roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#rm-110)
<a id="rm-111"></a> [rm-111](docs/roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#rm-111)
<a id="rm-112"></a> [rm-112](docs/roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#rm-112)
<a id="namespace-release-checkpoint"></a> [namespace-release-checkpoint](docs/roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#namespace-release-checkpoint)
<a id="programme-pause-and-resumption-checkpoint--2026-10-03"></a> [programme-pause-and-resumption-checkpoint--2026-10-03](docs/roadmap/records/programme-pause-and-resumption-checkpoint--2026-10-03.md#programme-pause-and-resumption-checkpoint--2026-10-03)
<a id="u3-per-function-effect-recovery--2026-10-03"></a> [u3-per-function-effect-recovery--2026-10-03](docs/roadmap/records/u3-per-function-effect-recovery--2026-10-03.md#u3-per-function-effect-recovery--2026-10-03)
<a id="rm-107"></a> [rm-107](docs/roadmap/records/u3-per-function-effect-recovery--2026-10-03.md#rm-107)
<a id="u0-semantic-impact-experiment--2026-10-03"></a> [u0-semantic-impact-experiment--2026-10-03](docs/roadmap/records/u0-semantic-impact-experiment--2026-10-03.md#u0-semantic-impact-experiment--2026-10-03)
<a id="rm-106"></a> [rm-106](docs/roadmap/records/u0-semantic-impact-experiment--2026-10-03.md#rm-106)
<a id="u3-effect-explanation-slice--2026-10-03"></a> [u3-effect-explanation-slice--2026-10-03](docs/roadmap/records/u3-effect-explanation-slice--2026-10-03.md#u3-effect-explanation-slice--2026-10-03)
<a id="rm-1"></a> [rm-1](docs/roadmap/records/u3-effect-explanation-slice--2026-10-03.md#rm-1)
<a id="u0-bounded-runtime-retention-experiment--2026-10-03"></a> [u0-bounded-runtime-retention-experiment--2026-10-03](docs/roadmap/records/u0-bounded-runtime-retention-experiment--2026-10-03.md#u0-bounded-runtime-retention-experiment--2026-10-03)
<a id="rm-2"></a> [rm-2](docs/roadmap/records/u0-bounded-runtime-retention-experiment--2026-10-03.md#rm-2)
<a id="standard-io-streaming-and-logging"></a> [standard-io-streaming-and-logging](docs/roadmap/records/standard-io-streaming-and-logging.md#standard-io-streaming-and-logging)
<a id="rm-3"></a> [rm-3](docs/roadmap/records/standard-io-streaming-and-logging.md#rm-3)
<a id="compiler-options-and-optimisation-modes"></a> [compiler-options-and-optimisation-modes](docs/roadmap/records/compiler-options-and-optimisation-modes.md#compiler-options-and-optimisation-modes)
<a id="rm-4"></a> [rm-4](docs/roadmap/records/compiler-options-and-optimisation-modes.md#rm-4)
<a id="performance-priority-review--2026-10-02"></a> [performance-priority-review--2026-10-02](docs/roadmap/records/performance-priority-review--2026-10-02.md#performance-priority-review--2026-10-02)
<a id="browserwasm-boundary-investigation--2026-10-01"></a> [browserwasm-boundary-investigation--2026-10-01](docs/roadmap/records/browserwasm-boundary-investigation--2026-10-01.md#browserwasm-boundary-investigation--2026-10-01)
<a id="rm-5"></a> [rm-5](docs/roadmap/records/browserwasm-boundary-investigation--2026-10-01.md#rm-5)
<a id="browser-website-publication-connection--verified"></a> [browser-website-publication-connection--verified](docs/roadmap/records/browserwasm-boundary-investigation--2026-10-01.md#browser-website-publication-connection--verified)
<a id="rm-6"></a> [rm-6](docs/roadmap/records/browserwasm-boundary-investigation--2026-10-01.md#rm-6)
<a id="browser-repository-separation--done"></a> [browser-repository-separation--done](docs/roadmap/records/browserwasm-boundary-investigation--2026-10-01.md#browser-repository-separation--done)
<a id="current-status-and-grooming-2026-10-01"></a> [current-status-and-grooming-2026-10-01](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#current-status-and-grooming-2026-10-01)
<a id="completed-playground-footer-layout"></a> [completed-playground-footer-layout](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#completed-playground-footer-layout)
<a id="rm-7"></a> [rm-7](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-7)
<a id="next-fast-website-ci-and-prepared-browser-test-environments"></a> [next-fast-website-ci-and-prepared-browser-test-environments](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#next-fast-website-ci-and-prepared-browser-test-environments)
<a id="accepted-website-delivery-remaining-ci-follow-ups-deferred"></a> [accepted-website-delivery-remaining-ci-follow-ups-deferred](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#accepted-website-delivery-remaining-ci-follow-ups-deferred)
<a id="rm-8"></a> [rm-8](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-8)
<a id="compiler-and-runtime-understanding-programme-delivery-and-resumption"></a> [compiler-and-runtime-understanding-programme-delivery-and-resumption](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#compiler-and-runtime-understanding-programme-delivery-and-resumption)
<a id="rm-9"></a> [rm-9](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-9)
<a id="compiler-and-runtime-understanding-programme"></a> [compiler-and-runtime-understanding-programme](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#compiler-and-runtime-understanding-programme)
<a id="rm-91"></a> [rm-91](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-91)
<a id="rm-92"></a> [rm-92](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-92)
<a id="rm-93"></a> [rm-93](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-93)
<a id="programme-delivery-register"></a> [programme-delivery-register](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#programme-delivery-register)
<a id="rm-97"></a> [rm-97](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-97)
<a id="rm-98"></a> [rm-98](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-98)
<a id="rm-99"></a> [rm-99](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-99)
<a id="rm-100"></a> [rm-100](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-100)
<a id="next-action-and-delivery-handover"></a> [next-action-and-delivery-handover](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#next-action-and-delivery-handover)
<a id="source-to-bytecode-mapping-foundation-u1-and-u2-done"></a> [source-to-bytecode-mapping-foundation-u1-and-u2-done](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#source-to-bytecode-mapping-foundation-u1-and-u2-done)
<a id="programme-acceptance-trustworthy-and-useful-on-realistic-programs"></a> [programme-acceptance-trustworthy-and-useful-on-realistic-programs](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#programme-acceptance-trustworthy-and-useful-on-realistic-programs)
<a id="rm-10"></a> [rm-10](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-10)
<a id="final-task-u9-website-positioning-and-demonstrations"></a> [final-task-u9-website-positioning-and-demonstrations](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#final-task-u9-website-positioning-and-demonstrations)
<a id="rm-11"></a> [rm-11](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-11)
<a id="now-portable-automatic-pr-previews"></a> [now-portable-automatic-pr-previews](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#now-portable-automatic-pr-previews)
<a id="completed-portable-author-local-previews"></a> [completed-portable-author-local-previews](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#completed-portable-author-local-previews)
<a id="rm-12"></a> [rm-12](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-12)
<a id="rm-101"></a> [rm-101](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-101)
<a id="programme-priority-review-2026-10-02"></a> [programme-priority-review-2026-10-02](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#programme-priority-review-2026-10-02)
<a id="historical-delivery-record"></a> [historical-delivery-record](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#historical-delivery-record)
<a id="groomed-candidates"></a> [groomed-candidates](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#groomed-candidates)
<a id="proposed-first-step"></a> [proposed-first-step](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#proposed-first-step)
<a id="completed-modular-validation-and-component-boundaries"></a> [completed-modular-validation-and-component-boundaries](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#completed-modular-validation-and-component-boundaries)
<a id="rm-13"></a> [rm-13](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-13)
<a id="additional-ideas-recorded-on-2026-10-01"></a> [additional-ideas-recorded-on-2026-10-01](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#additional-ideas-recorded-on-2026-10-01)
<a id="rm-85"></a> [rm-85](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-85)
<a id="rm-86"></a> [rm-86](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-86)
<a id="rm-87"></a> [rm-87](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-87)
<a id="rm-88"></a> [rm-88](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-88)
<a id="rm-89"></a> [rm-89](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-89)
<a id="rm-90"></a> [rm-90](docs/roadmap/records/current-status-and-grooming-2026-10-01.md#rm-90)
<a id="next-integration-workflow--active-promotion-acceptance-remains"></a> [next-integration-workflow--active-promotion-acceptance-remains](docs/roadmap/records/next-integration-workflow--active-promotion-acceptance-remains.md#next-integration-workflow--active-promotion-acceptance-remains)
<a id="rm-135"></a> [rm-135](docs/roadmap/records/next-integration-workflow--active-promotion-acceptance-remains.md#rm-135)
<a id="independent-website-publishing"></a> [independent-website-publishing](docs/roadmap/records/independent-website-publishing.md#independent-website-publishing)
<a id="rm-14"></a> [rm-14](docs/roadmap/records/independent-website-publishing.md#rm-14)
<a id="website-follow-up-register"></a> [website-follow-up-register](docs/roadmap/records/independent-website-publishing.md#website-follow-up-register)
<a id="rm-127"></a> [rm-127](docs/roadmap/records/independent-website-publishing.md#rm-127)
<a id="rm-122"></a> [rm-122](docs/roadmap/records/independent-website-publishing.md#rm-122)
<a id="rm-119"></a> [rm-119](docs/roadmap/records/independent-website-publishing.md#rm-119)
<a id="rm-118"></a> [rm-118](docs/roadmap/records/independent-website-publishing.md#rm-118)
<a id="rm-102"></a> [rm-102](docs/roadmap/records/independent-website-publishing.md#rm-102)
<a id="rm-103"></a> [rm-103](docs/roadmap/records/independent-website-publishing.md#rm-103)
<a id="rm-104"></a> [rm-104](docs/roadmap/records/independent-website-publishing.md#rm-104)
<a id="additional-platform-and-distribution-ideas"></a> [additional-platform-and-distribution-ideas](docs/roadmap/records/independent-website-publishing.md#additional-platform-and-distribution-ideas)
<a id="linux-arm64-support"></a> [linux-arm64-support](docs/roadmap/records/independent-website-publishing.md#linux-arm64-support)
<a id="support-linux-arm64-release-artifacts"></a> [support-linux-arm64-release-artifacts](docs/roadmap/records/independent-website-publishing.md#support-linux-arm64-release-artifacts)
<a id="rm-15"></a> [rm-15](docs/roadmap/records/independent-website-publishing.md#rm-15)
<a id="container-distribution"></a> [container-distribution](docs/roadmap/records/independent-website-publishing.md#container-distribution)
<a id="publish-versioned-panackelty-docker-images"></a> [publish-versioned-panackelty-docker-images](docs/roadmap/records/independent-website-publishing.md#publish-versioned-panackelty-docker-images)
<a id="rm-16"></a> [rm-16](docs/roadmap/records/independent-website-publishing.md#rm-16)
<a id="homebrew-distribution"></a> [homebrew-distribution](docs/roadmap/records/independent-website-publishing.md#homebrew-distribution)
<a id="provide-panackelty-installation-through-a-homebrew-tap"></a> [provide-panackelty-installation-through-a-homebrew-tap](docs/roadmap/records/independent-website-publishing.md#provide-panackelty-installation-through-a-homebrew-tap)
<a id="rm-17"></a> [rm-17](docs/roadmap/records/independent-website-publishing.md#rm-17)
<a id="debian-package-distribution"></a> [debian-package-distribution](docs/roadmap/records/independent-website-publishing.md#debian-package-distribution)
<a id="assess-debian-packages-and-apt-distribution"></a> [assess-debian-packages-and-apt-distribution](docs/roadmap/records/independent-website-publishing.md#assess-debian-packages-and-apt-distribution)
<a id="rm-18"></a> [rm-18](docs/roadmap/records/independent-website-publishing.md#rm-18)
<a id="additional-installation-proposals--2026-10-04"></a> [additional-installation-proposals--2026-10-04](docs/roadmap/records/independent-website-publishing.md#additional-installation-proposals--2026-10-04)
<a id="rm-120"></a> [rm-120](docs/roadmap/records/independent-website-publishing.md#rm-120)
<a id="rm120-graphical-installer"></a> [rm120-graphical-installer](docs/roadmap/records/independent-website-publishing.md#rm120-graphical-installer)
<a id="rm-121"></a> [rm-121](docs/roadmap/records/independent-website-publishing.md#rm-121)
<a id="rm121-one-command-installer"></a> [rm121-one-command-installer](docs/roadmap/records/independent-website-publishing.md#rm121-one-command-installer)
<a id="scope-retained-outside-this-grooming-batch"></a> [scope-retained-outside-this-grooming-batch](docs/roadmap/records/independent-website-publishing.md#scope-retained-outside-this-grooming-batch)
<a id="now-expanded-holistic-and-architectural-gap-assessment"></a> [now-expanded-holistic-and-architectural-gap-assessment](docs/roadmap/records/independent-website-publishing.md#now-expanded-holistic-and-architectural-gap-assessment)
<a id="completed-expanded-holistic-and-architectural-gap-assessment"></a> [completed-expanded-holistic-and-architectural-gap-assessment](docs/roadmap/records/independent-website-publishing.md#completed-expanded-holistic-and-architectural-gap-assessment)
<a id="rm-19"></a> [rm-19](docs/roadmap/records/independent-website-publishing.md#rm-19)
<a id="completed-sorting-and-literal-suffix-helpers"></a> [completed-sorting-and-literal-suffix-helpers](docs/roadmap/records/independent-website-publishing.md#completed-sorting-and-literal-suffix-helpers)
<a id="rm-21"></a> [rm-21](docs/roadmap/records/independent-website-publishing.md#rm-21)
<a id="completed-execution-concurrency-and-host-integration-design"></a> [completed-execution-concurrency-and-host-integration-design](docs/roadmap/records/independent-website-publishing.md#completed-execution-concurrency-and-host-integration-design)
<a id="rm-22"></a> [rm-22](docs/roadmap/records/independent-website-publishing.md#rm-22)
<a id="completed-resumable-vm-execution-feasibility"></a> [completed-resumable-vm-execution-feasibility](docs/roadmap/records/independent-website-publishing.md#completed-resumable-vm-execution-feasibility)
<a id="rm-23"></a> [rm-23](docs/roadmap/records/independent-website-publishing.md#rm-23)
<a id="now-task-and-lifecycle-feasibility"></a> [now-task-and-lifecycle-feasibility](docs/roadmap/records/independent-website-publishing.md#now-task-and-lifecycle-feasibility)
<a id="completed-task-and-lifecycle-feasibility"></a> [completed-task-and-lifecycle-feasibility](docs/roadmap/records/independent-website-publishing.md#completed-task-and-lifecycle-feasibility)
<a id="rm-24"></a> [rm-24](docs/roadmap/records/independent-website-publishing.md#rm-24)
<a id="async-programming-interface-proposal-for-review"></a> [async-programming-interface-proposal-for-review](docs/roadmap/records/independent-website-publishing.md#async-programming-interface-proposal-for-review)
<a id="rm-25"></a> [rm-25](docs/roadmap/records/independent-website-publishing.md#rm-25)
<a id="completed-bounded-source-to-vm-asyncawait-slice"></a> [completed-bounded-source-to-vm-asyncawait-slice](docs/roadmap/records/independent-website-publishing.md#completed-bounded-source-to-vm-asyncawait-slice)
<a id="rm-26"></a> [rm-26](docs/roadmap/records/independent-website-publishing.md#rm-26)
<a id="completed-bounded-validation-performance-investigation"></a> [completed-bounded-validation-performance-investigation](docs/roadmap/records/independent-website-publishing.md#completed-bounded-validation-performance-investigation)
<a id="rm-27"></a> [rm-27](docs/roadmap/records/independent-website-publishing.md#rm-27)
<a id="incremental-and-modular-builds"></a> [incremental-and-modular-builds](docs/roadmap/records/independent-website-publishing.md#incremental-and-modular-builds)
<a id="rm-28"></a> [rm-28](docs/roadmap/records/independent-website-publishing.md#rm-28)
<a id="future-candidate-incremental-and-modular-builds"></a> [future-candidate-incremental-and-modular-builds](docs/roadmap/records/independent-website-publishing.md#future-candidate-incremental-and-modular-builds)
<a id="completed-vmcompiler-boundary-audit"></a> [completed-vmcompiler-boundary-audit](docs/roadmap/records/independent-website-publishing.md#completed-vmcompiler-boundary-audit)
<a id="rm-29"></a> [rm-29](docs/roadmap/records/independent-website-publishing.md#rm-29)
<a id="rm-105"></a> [rm-105](docs/roadmap/records/independent-website-publishing.md#rm-105)
<a id="completed-browser-playground-feasibility"></a> [completed-browser-playground-feasibility](docs/roadmap/records/independent-website-publishing.md#completed-browser-playground-feasibility)
<a id="rm-30"></a> [rm-30](docs/roadmap/records/independent-website-publishing.md#rm-30)
<a id="completed-playground-delivery-preparation"></a> [completed-playground-delivery-preparation](docs/roadmap/records/independent-website-publishing.md#completed-playground-delivery-preparation)
<a id="rm-31"></a> [rm-31](docs/roadmap/records/independent-website-publishing.md#rm-31)
<a id="completed-website-playground-integration"></a> [completed-website-playground-integration](docs/roadmap/records/independent-website-publishing.md#completed-website-playground-integration)
<a id="rm-32"></a> [rm-32](docs/roadmap/records/independent-website-publishing.md#rm-32)
<a id="completed-content-led-website-expansion"></a> [completed-content-led-website-expansion](docs/roadmap/records/independent-website-publishing.md#completed-content-led-website-expansion)
<a id="rm-33"></a> [rm-33](docs/roadmap/records/independent-website-publishing.md#rm-33)
<a id="developer-experience-useful-assistance-from-the-compiler"></a> [developer-experience-useful-assistance-from-the-compiler](docs/roadmap/records/independent-website-publishing.md#developer-experience-useful-assistance-from-the-compiler)
<a id="rm-34"></a> [rm-34](docs/roadmap/records/independent-website-publishing.md#rm-34)
<a id="backlog-navigation"></a> [backlog-navigation](docs/roadmap/records/independent-website-publishing.md#backlog-navigation)
<a id="how-to-turn-the-backlog-into-prs"></a> [how-to-turn-the-backlog-into-prs](docs/roadmap/records/independent-website-publishing.md#how-to-turn-the-backlog-into-prs)
<a id="improve-and-expand-the-website"></a> [improve-and-expand-the-website](docs/roadmap/records/improve-and-expand-the-website.md#improve-and-expand-the-website)
<a id="developer-preview-alpha11-release"></a> [developer-preview-alpha11-release](docs/roadmap/records/developer-preview-alpha11-release.md#developer-preview-alpha11-release)
<a id="rm-128"></a> [rm-128](docs/roadmap/records/developer-preview-alpha11-release.md#rm-128)
<a id="developer-preview-alpha10-release"></a> [developer-preview-alpha10-release](docs/roadmap/records/developer-preview-alpha10-release.md#developer-preview-alpha10-release)
<a id="rm-35"></a> [rm-35](docs/roadmap/records/developer-preview-alpha10-release.md#rm-35)
<a id="real-async-tcp-from-panackelty"></a> [real-async-tcp-from-panackelty](docs/roadmap/records/real-async-tcp-from-panackelty.md#real-async-tcp-from-panackelty)
<a id="rm-36"></a> [rm-36](docs/roadmap/records/real-async-tcp-from-panackelty.md#rm-36)
<a id="bounded-async-tcp-server"></a> [bounded-async-tcp-server](docs/roadmap/records/bounded-async-tcp-server.md#bounded-async-tcp-server)
<a id="rm-37"></a> [rm-37](docs/roadmap/records/bounded-async-tcp-server.md#rm-37)
<a id="website-deployment-source-selection"></a> [website-deployment-source-selection](docs/roadmap/records/website-deployment-source-selection.md#website-deployment-source-selection)
<a id="rm-38"></a> [rm-38](docs/roadmap/records/website-deployment-source-selection.md#rm-38)
<a id="playground-deployment-cache-consistency"></a> [playground-deployment-cache-consistency](docs/roadmap/records/playground-deployment-cache-consistency.md#playground-deployment-cache-consistency)
<a id="rm-39"></a> [rm-39](docs/roadmap/records/playground-deployment-cache-consistency.md#rm-39)
<a id="core-types-and-discoverable-textcollection-methods"></a> [core-types-and-discoverable-textcollection-methods](docs/roadmap/records/core-types-and-discoverable-textcollection-methods.md#core-types-and-discoverable-textcollection-methods)
<a id="rm-40"></a> [rm-40](docs/roadmap/records/core-types-and-discoverable-textcollection-methods.md#rm-40)
<a id="historical-scope-and-recommendation-before-pr-118"></a> [historical-scope-and-recommendation-before-pr-118](docs/roadmap/records/core-types-and-discoverable-textcollection-methods.md#historical-scope-and-recommendation-before-pr-118)
<a id="language-namespaces--idea"></a> [language-namespaces--idea](docs/roadmap/records/language-namespaces--idea.md#language-namespaces--idea)
<a id="rm-41"></a> [rm-41](docs/roadmap/records/language-namespaces--idea.md#rm-41)
<a id="p2-delivery-checklist--delivery-and-remaining-work"></a> [p2-delivery-checklist--delivery-and-remaining-work](docs/roadmap/records/language-namespaces--idea.md#p2-delivery-checklist--delivery-and-remaining-work)
<a id="p2-delivery-checklist"></a> [p2-delivery-checklist](docs/roadmap/records/language-namespaces--idea.md#p2-delivery-checklist)
<a id="namespace-completion-assessment"></a> [namespace-completion-assessment](docs/roadmap/records/language-namespaces--idea.md#namespace-completion-assessment)
<a id="standard-library-module-api-migration"></a> [standard-library-module-api-migration](docs/roadmap/records/standard-library-module-api-migration.md#standard-library-module-api-migration)
<a id="rm-42"></a> [rm-42](docs/roadmap/records/standard-library-module-api-migration.md#rm-42)
<a id="reusable-modules-and-package-structure--idea"></a> [reusable-modules-and-package-structure--idea](docs/roadmap/records/reusable-modules-and-package-structure--idea.md#reusable-modules-and-package-structure--idea)
<a id="rm-43"></a> [rm-43](docs/roadmap/records/reusable-modules-and-package-structure--idea.md#rm-43)
<a id="dependency-management--idea"></a> [dependency-management--idea](docs/roadmap/records/dependency-management--idea.md#dependency-management--idea)
<a id="rm-44"></a> [rm-44](docs/roadmap/records/dependency-management--idea.md#rm-44)
<a id="review-github-repository-settings-and-tooling--idea"></a> [review-github-repository-settings-and-tooling--idea](docs/roadmap/records/review-github-repository-settings-and-tooling--idea.md#review-github-repository-settings-and-tooling--idea)
<a id="rm-45"></a> [rm-45](docs/roadmap/records/review-github-repository-settings-and-tooling--idea.md#rm-45)
<a id="measure-panackelty-source-coverage--candidate-pending-assessment"></a> [measure-panackelty-source-coverage--candidate-pending-assessment](docs/roadmap/records/review-github-repository-settings-and-tooling--idea.md#measure-panackelty-source-coverage--candidate-pending-assessment)
<a id="panackelty-source-coverage-programme"></a> [panackelty-source-coverage-programme](docs/roadmap/records/panackelty-source-coverage-programme.md#panackelty-source-coverage-programme)
<a id="rm-46"></a> [rm-46](docs/roadmap/records/panackelty-source-coverage-programme.md#rm-46)
<a id="source-coverage-programme-baseline"></a> [source-coverage-programme-baseline](docs/roadmap/records/panackelty-source-coverage-programme.md#source-coverage-programme-baseline)
<a id="rm-136"></a> [rm-136](docs/roadmap/records/panackelty-source-coverage-programme.md#rm-136)
<a id="rm-137"></a> [rm-137](docs/roadmap/records/panackelty-source-coverage-programme.md#rm-137)
<a id="rm-138"></a> [rm-138](docs/roadmap/records/panackelty-source-coverage-programme.md#rm-138)
<a id="rm-139"></a> [rm-139](docs/roadmap/records/panackelty-source-coverage-programme.md#rm-139)
<a id="rm-140"></a> [rm-140](docs/roadmap/records/panackelty-source-coverage-programme.md#rm-140)
<a id="rm-141"></a> [rm-141](docs/roadmap/records/panackelty-source-coverage-programme.md#rm-141)
<a id="groomed-development-and-engineering-work"></a> [groomed-development-and-engineering-work](docs/roadmap/records/groomed-development-and-engineering-work.md#groomed-development-and-engineering-work)
<a id="development-workflow-assessment"></a> [development-workflow-assessment](docs/roadmap/records/groomed-development-and-engineering-work.md#development-workflow-assessment)
<a id="rm-47"></a> [rm-47](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-47)
<a id="compiler-explanations"></a> [compiler-explanations](docs/roadmap/records/groomed-development-and-engineering-work.md#compiler-explanations)
<a id="rm-48"></a> [rm-48](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-48)
<a id="systematic-invariant-testing"></a> [systematic-invariant-testing](docs/roadmap/records/groomed-development-and-engineering-work.md#systematic-invariant-testing)
<a id="rm-49"></a> [rm-49](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-49)
<a id="source-aware-runtime-errors"></a> [source-aware-runtime-errors](docs/roadmap/records/groomed-development-and-engineering-work.md#source-aware-runtime-errors)
<a id="rm-50"></a> [rm-50](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-50)
<a id="learning-path-and-technical-documentation"></a> [learning-path-and-technical-documentation](docs/roadmap/records/groomed-development-and-engineering-work.md#learning-path-and-technical-documentation)
<a id="rm-51"></a> [rm-51](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-51)
<a id="public-markdown-quality"></a> [public-markdown-quality](docs/roadmap/records/groomed-development-and-engineering-work.md#public-markdown-quality)
<a id="rm-131"></a> [rm-131](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-131)
<a id="definitive-panackelty-style-guide"></a> [definitive-panackelty-style-guide](docs/roadmap/records/groomed-development-and-engineering-work.md#definitive-panackelty-style-guide)
<a id="rm-133"></a> [rm-133](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-133)
<a id="website-capabilities-guide"></a> [website-capabilities-guide](docs/roadmap/records/groomed-development-and-engineering-work.md#website-capabilities-guide)
<a id="rm-130"></a> [rm-130](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-130)
<a id="executable-documentation"></a> [executable-documentation](docs/roadmap/records/groomed-development-and-engineering-work.md#executable-documentation)
<a id="rm-52"></a> [rm-52](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-52)
<a id="debugging-delivery-sequence--agreed-2026-10-01"></a> [debugging-delivery-sequence--agreed-2026-10-01](docs/roadmap/records/groomed-development-and-engineering-work.md#debugging-delivery-sequence--agreed-2026-10-01)
<a id="interactive-debugger--candidate-for-assessment"></a> [interactive-debugger--candidate-for-assessment](docs/roadmap/records/groomed-development-and-engineering-work.md#interactive-debugger--candidate-for-assessment)
<a id="rm-53"></a> [rm-53](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-53)
<a id="editor-support"></a> [editor-support](docs/roadmap/records/groomed-development-and-engineering-work.md#editor-support)
<a id="rm-54"></a> [rm-54](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-54)
<a id="technical-showcase-programs"></a> [technical-showcase-programs](docs/roadmap/records/groomed-development-and-engineering-work.md#technical-showcase-programs)
<a id="rm-55"></a> [rm-55](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-55)
<a id="runtime-and-resource-baselines"></a> [runtime-and-resource-baselines](docs/roadmap/records/groomed-development-and-engineering-work.md#runtime-and-resource-baselines)
<a id="performance-and-benchmarking"></a> [performance-and-benchmarking](docs/roadmap/records/groomed-development-and-engineering-work.md#performance-and-benchmarking)
<a id="rm-56"></a> [rm-56](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-56)
<a id="compiler-performance-investigation"></a> [compiler-performance-investigation](docs/roadmap/records/groomed-development-and-engineering-work.md#compiler-performance-investigation)
<a id="rm-134"></a> [rm-134](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-134)
<a id="independent-contract-implementation"></a> [independent-contract-implementation](docs/roadmap/records/groomed-development-and-engineering-work.md#independent-contract-implementation)
<a id="rm-57"></a> [rm-57](docs/roadmap/records/groomed-development-and-engineering-work.md#rm-57)
<a id="potential-technical-publications"></a> [potential-technical-publications](docs/roadmap/records/potential-technical-publications.md#potential-technical-publications)
<a id="compiler-and-runtime-understanding-paper--idea"></a> [compiler-and-runtime-understanding-paper--idea](docs/roadmap/records/potential-technical-publications.md#compiler-and-runtime-understanding-paper--idea)
<a id="rm-58"></a> [rm-58](docs/roadmap/records/potential-technical-publications.md#rm-58)
<a id="independently-checkable-change-impact-explanations--research-idea"></a> [independently-checkable-change-impact-explanations--research-idea](docs/roadmap/records/potential-technical-publications.md#independently-checkable-change-impact-explanations--research-idea)
<a id="rm-59"></a> [rm-59](docs/roadmap/records/potential-technical-publications.md#rm-59)
<a id="historical-grooming-gaps-and-decisions"></a> [historical-grooming-gaps-and-decisions](docs/roadmap/records/historical-grooming-gaps-and-decisions.md#historical-grooming-gaps-and-decisions)
<a id="harden-and-expand-test-coverage--candidate-foundation-delivered"></a> [harden-and-expand-test-coverage--candidate-foundation-delivered](docs/roadmap/records/harden-and-expand-test-coverage--candidate-foundation-delivered.md#harden-and-expand-test-coverage--candidate-foundation-delivered)
<a id="rm-60"></a> [rm-60](docs/roadmap/records/harden-and-expand-test-coverage--candidate-foundation-delivered.md#rm-60)
<a id="publish-public-native-c-coverage-reports--complete"></a> [publish-public-native-c-coverage-reports--complete](docs/roadmap/records/harden-and-expand-test-coverage--candidate-foundation-delivered.md#publish-public-native-c-coverage-reports--complete)
<a id="rm-61"></a> [rm-61](docs/roadmap/records/harden-and-expand-test-coverage--candidate-foundation-delivered.md#rm-61)
<a id="codebase-wide-human-readability-and-refactoring--candidate"></a> [codebase-wide-human-readability-and-refactoring--candidate](docs/roadmap/records/codebase-wide-human-readability-and-refactoring--candidate.md#codebase-wide-human-readability-and-refactoring--candidate)
<a id="rm-62"></a> [rm-62](docs/roadmap/records/codebase-wide-human-readability-and-refactoring--candidate.md#rm-62)
<a id="native-vm-readability-and-test-hardening"></a> [native-vm-readability-and-test-hardening](docs/roadmap/records/native-vm-readability-and-test-hardening.md#native-vm-readability-and-test-hardening)
<a id="rm-63"></a> [rm-63](docs/roadmap/records/native-vm-readability-and-test-hardening.md#rm-63)
<a id="historical-validation-timing-record--nfrs-under-gi106-reassessment"></a> [historical-validation-timing-record--nfrs-under-gi106-reassessment](docs/roadmap/records/historical-validation-timing-record--nfrs-under-gi106-reassessment.md#historical-validation-timing-record--nfrs-under-gi106-reassessment)
<a id="expand-automation-and-host-capabilities--foundation-delivered-extensions-unscheduled"></a> [expand-automation-and-host-capabilities--foundation-delivered-extensions-unscheduled](docs/roadmap/records/expand-automation-and-host-capabilities--foundation-delivered-extensions-unscheduled.md#expand-automation-and-host-capabilities--foundation-delivered-extensions-unscheduled)
<a id="rm-84"></a> [rm-84](docs/roadmap/records/expand-automation-and-host-capabilities--foundation-delivered-extensions-unscheduled.md#rm-84)
<a id="language-direction-and-differentiation--exploration"></a> [language-direction-and-differentiation--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#language-direction-and-differentiation--exploration)
<a id="long-term-direction-grow-panackelty-through-real-programs"></a> [long-term-direction-grow-panackelty-through-real-programs](docs/roadmap/records/language-direction-and-differentiation--exploration.md#long-term-direction-grow-panackelty-through-real-programs)
<a id="further-language-research--unscheduled"></a> [further-language-research--unscheduled](docs/roadmap/records/language-direction-and-differentiation--exploration.md#further-language-research--unscheduled)
<a id="rm-145"></a> [rm-145](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-145)
<a id="rm-146"></a> [rm-146](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-146)

RM#146 now proposes a bounded investigation into isolated multicore workers and
HTTP scaling, with sequential correctness comparisons, ownership/cancellation
evidence and measured scaling costs. It remains unscheduled research, with no
release allocation; see the linked record for scope and acceptance.
<a id="rm-147"></a> [rm-147](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-147)
<a id="rm-148"></a> [rm-148](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-148)
<a id="error-propagation-shorthand"></a> [error-propagation-shorthand](docs/roadmap/records/language-direction-and-differentiation--exploration.md#error-propagation-shorthand)
<a id="rm-129"></a> [rm-129](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-129)
<a id="type-inference-and-diagnostic-experience"></a> [type-inference-and-diagnostic-experience](docs/roadmap/records/language-direction-and-differentiation--exploration.md#type-inference-and-diagnostic-experience)
<a id="rm-64"></a> [rm-64](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-64)
<a id="type-inference-research-candidates"></a> [type-inference-research-candidates](docs/roadmap/records/language-direction-and-differentiation--exploration.md#type-inference-research-candidates)
<a id="rm-94"></a> [rm-94](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-94)
<a id="rm-95"></a> [rm-95](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-95)
<a id="rm-96"></a> [rm-96](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-96)
<a id="core-and-standard-library-types--planned-exploration"></a> [core-and-standard-library-types--planned-exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#core-and-standard-library-types--planned-exploration)
<a id="rm-65"></a> [rm-65](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-65)
<a id="predictable-deferred-computation"></a> [predictable-deferred-computation](docs/roadmap/records/language-direction-and-differentiation--exploration.md#predictable-deferred-computation)
<a id="rm-66"></a> [rm-66](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-66)
<a id="candidate-differentiator-contract-driven-automation"></a> [candidate-differentiator-contract-driven-automation](docs/roadmap/records/language-direction-and-differentiation--exploration.md#candidate-differentiator-contract-driven-automation)
<a id="rm-67"></a> [rm-67](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-67)
<a id="candidate-differentiator-explainable-values--exploration"></a> [candidate-differentiator-explainable-values--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#candidate-differentiator-explainable-values--exploration)
<a id="rm-68"></a> [rm-68](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-68)
<a id="candidate-differentiator-previewable-effects--exploration"></a> [candidate-differentiator-previewable-effects--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#candidate-differentiator-previewable-effects--exploration)
<a id="rm-69"></a> [rm-69](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-69)
<a id="candidate-differentiator-resumable-execution--exploration"></a> [candidate-differentiator-resumable-execution--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#candidate-differentiator-resumable-execution--exploration)
<a id="rm-70"></a> [rm-70](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-70)
<a id="candidate-differentiator-enforceable-data-flow-restrictions--exploration"></a> [candidate-differentiator-enforceable-data-flow-restrictions--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#candidate-differentiator-enforceable-data-flow-restrictions--exploration)
<a id="rm-71"></a> [rm-71](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-71)
<a id="candidate-differentiator-change-contracts--exploration"></a> [candidate-differentiator-change-contracts--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#candidate-differentiator-change-contracts--exploration)
<a id="rm-72"></a> [rm-72](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-72)
<a id="guarded-types--exploration"></a> [guarded-types--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#guarded-types--exploration)
<a id="rm-153"></a> [rm-153](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-153)
<a id="json-data-support--exploration"></a> [json-data-support--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#json-data-support--exploration)
<a id="rm-73"></a> [rm-73](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-73)
<a id="type-driven-input-handling--exploration"></a> [type-driven-input-handling--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#type-driven-input-handling--exploration)
<a id="rm-74"></a> [rm-74](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-74)
<a id="typed-edits-and-patches--exploration"></a> [typed-edits-and-patches--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#typed-edits-and-patches--exploration)
<a id="rm-75"></a> [rm-75](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-75)
<a id="useful-execution-of-unfinished-code--exploration"></a> [useful-execution-of-unfinished-code--exploration](docs/roadmap/records/language-direction-and-differentiation--exploration.md#useful-execution-of-unfinished-code--exploration)
<a id="rm-76"></a> [rm-76](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-76)
<a id="data-and-target-platform-experiments"></a> [data-and-target-platform-experiments](docs/roadmap/records/language-direction-and-differentiation--exploration.md#data-and-target-platform-experiments)
<a id="rm-77"></a> [rm-77](docs/roadmap/records/language-direction-and-differentiation--exploration.md#rm-77)
<a id="interactive-repl--exploration-website-learning-use-case-identified"></a> [interactive-repl--exploration-website-learning-use-case-identified](docs/roadmap/records/interactive-repl--exploration-website-learning-use-case-identified.md#interactive-repl--exploration-website-learning-use-case-identified)
<a id="rm-78"></a> [rm-78](docs/roadmap/records/interactive-repl--exploration-website-learning-use-case-identified.md#rm-78)
<a id="delivered-milestones--historical-detail"></a> [delivered-milestones--historical-detail](docs/roadmap/records/delivered-milestones--historical-detail.md#delivered-milestones--historical-detail)
<a id="deliver-developer-preview-010-alpha1--delivered"></a> [deliver-developer-preview-010-alpha1--delivered](docs/roadmap/records/deliver-developer-preview-010-alpha1--delivered.md#deliver-developer-preview-010-alpha1--delivered)
<a id="rm-79"></a> [rm-79](docs/roadmap/records/deliver-developer-preview-010-alpha1--delivered.md#rm-79)
<a id="1-freeze-the-preview-contract"></a> [1-freeze-the-preview-contract](docs/roadmap/records/deliver-developer-preview-010-alpha1--delivered.md#1-freeze-the-preview-contract)
<a id="2-establish-the-public-project-boundary"></a> [2-establish-the-public-project-boundary](docs/roadmap/records/deliver-developer-preview-010-alpha1--delivered.md#2-establish-the-public-project-boundary)
<a id="3-version-the-complete-toolchain"></a> [3-version-the-complete-toolchain](docs/roadmap/records/deliver-developer-preview-010-alpha1--delivered.md#3-version-the-complete-toolchain)
<a id="4-produce-download-and-run-archives"></a> [4-produce-download-and-run-archives](docs/roadmap/records/deliver-developer-preview-010-alpha1--delivered.md#4-produce-download-and-run-archives)
<a id="5-add-exact-artifact-release-gates"></a> [5-add-exact-artifact-release-gates](docs/roadmap/records/deliver-developer-preview-010-alpha1--delivered.md#5-add-exact-artifact-release-gates)
<a id="6-make-the-first-user-workflow-usable"></a> [6-make-the-first-user-workflow-usable](docs/roadmap/records/deliver-developer-preview-010-alpha1--delivered.md#6-make-the-first-user-workflow-usable)
<a id="7-publish-and-verify"></a> [7-publish-and-verify](docs/roadmap/records/deliver-developer-preview-010-alpha1--delivered.md#7-publish-and-verify)
<a id="ergonomic-control-flow-and-collection-apis--complete"></a> [ergonomic-control-flow-and-collection-apis--complete](docs/roadmap/records/ergonomic-control-flow-and-collection-apis--complete.md#ergonomic-control-flow-and-collection-apis--complete)
<a id="rm-80"></a> [rm-80](docs/roadmap/records/ergonomic-control-flow-and-collection-apis--complete.md#rm-80)
<a id="make-imports-independent-of-repository-paths--complete"></a> [make-imports-independent-of-repository-paths--complete](docs/roadmap/records/make-imports-independent-of-repository-paths--complete.md#make-imports-independent-of-repository-paths--complete)
<a id="rm-81"></a> [rm-81](docs/roadmap/records/make-imports-independent-of-repository-paths--complete.md#rm-81)
<a id="self-hosted-development-toolchain--complete"></a> [self-hosted-development-toolchain--complete](docs/roadmap/records/self-hosted-development-toolchain--complete.md#self-hosted-development-toolchain--complete)
<a id="rm-82"></a> [rm-82](docs/roadmap/records/self-hosted-development-toolchain--complete.md#rm-82)
<a id="change-panackelty-syntax--complete"></a> [change-panackelty-syntax--complete](docs/roadmap/records/change-panackelty-syntax--complete.md#change-panackelty-syntax--complete)
<a id="rm-83"></a> [rm-83](docs/roadmap/records/change-panackelty-syntax--complete.md#rm-83)

</details>

RM#162 HTTP inspection repair (22 November): all HTTP builtin arities and local
wrapper permissions now match the native verifier in the self-hosted decoder.
Focused client/server disassembly and native artifact checks pass; final local make check passed in 674s, as did relocated acceptance. No new public API or website change is
required: this restores inspection of already supported bytecode.

RM#162 / AS5 (23 November): core ordinary/async reference and migration guidance
now link three validated timing, file-cleanup and subprocess examples. Focused
source/bytecode, inventory mutation and documentation checks pass. Runtime and
compiler are unchanged from the preceding full validation. Broader examples,
website adaptation/review and integration remain; the existing stdlib website
follow-up applies. No website publication is included.

RM#162 / AS5 resource follow-up (23 November): SHA-256 and gzip examples and the
resource walkthrough pass focused source/bytecode, inventory and documentation
checks. Uptime uses canonical duration naming with its HTTP fixture passing.
The documentation coverage checklist names the four remaining core gaps before
website adaptation/review; no additional example quota or runtime scope is added.
No runtime performance impact; the two bounded examples add small fixture work.
Website follow-up remains open and nothing is published.

AS5 core gaps covered (23 November): added console EOF/bounds and staged/confined
publication examples, a diagnostic HTTP client package example, and their
walkthroughs. Canonical UTF-8 decoding and checked reporting examples retain
their transcripts; compatibility filesystem/process recipes are explicitly
labelled rather than silently removed. Focused named source/compile/bytecode
checks, console Unicode/bounds/failure cases, HTTP 200/404/invalid URL cases,
45-example/610-assertion inventory mutation checks and documentation links pass.
Only example/guide/test-registration work changed; compiler, VM, stdlib and
shared runner logic are unchanged. No full-suite rerun or development CI.
Website adaptation is ready in [website PR #77](https://github.com/sproates/panackelty-website/pull/77): five development-labelled pages and nine complete examples, with 49 local website tests and assembly checks passing. Desktop and phone-width preview checked; owner review, integration and publication remain pending. No development CI was triggered.

Owner review correction (23 November): website PR #77 now provides nineteen self-contained module references with 72 function entries, parameter/result/error details, bounds and ownership, plus embedded complete examples. All signatures match pinned core source; 51 local website tests and assembly checks pass. The persistent preview is refreshed. Owner review, integration and publication remain pending; no CI was triggered.

23 November integration handoff: the owner approved the website documentation approach and preview. Core PR #693 is ready for integration into next under the alpha.16 trial; its merge starts hosted validation. AS1–AS4 implementation and AS5 core guides/examples are locally validated; hosted acceptance remains pending. Website PR #77 has preview approval, not publication permission. RM#162 remains open for the broader real-program work; this stdlib batch does not implement closures, anonymous functions or shared state.

23 November: async stdlib core PR #693 merged into next as `930f951`. [integration run](https://github.com/sproates/panackelty/actions/runs/45163842662) is running. Website PR #77 has preview approval; publication remains separately authorised. No new implementation work is included in this status update.

Integration follow-up, 23 November: run 45163842662 passed both platform package jobs but stopped before full validation because the manifest regression expected an older unmeasured-source list. The manifest already accounts for the ten added error/compatibility modules with reasons. This correction updates only the explicit expected inventory; coverage eligibility, measurement and thresholds are unchanged. The exact seven-file policy suite passes locally. Hosted full acceptance remains pending a new integration run.

23 November: core stdlib PR #693 and inventory correction PR #697 are merged. [integration run 45164349102](https://github.com/sproates/panackelty/actions/runs/45164349102) passed on `12dccfb` (both platform package jobs, coverage policy and integration gate). AS1–AS4 are accepted; AS5 website preview is approved, with publication/live verification remaining. This is not completion of the broader RM#162 language work or a full hosted release-matrix run.

### Expanded async stdlib documentation: complete, 23 November 2026

23 November publication acceptance: the owner authorised website checks and publication.
[Website PR #77](https://github.com/sproates/panackelty-website/pull/77) merged as
`e59e93e` after [checks 45165626275](https://github.com/sproates/panackelty-website/actions/runs/45165626275)
passed. [Deployment 45166480787](https://github.com/sproates/panackelty-website/actions/runs/45166480787)
passed, including verification of deployed bytes and provenance. Live browser review
confirmed the Time reference, signatures, errors, complete example and stdlib-before-HTTP
navigation. The [published reference](https://panackelty.com/reference/standard-library/)
contains nineteen module pages and 72 function entries, alongside four guides and nine
complete examples. AS5 is accepted: all five slices of the expanded async stdlib and
documentation batch are complete (100/100 weighted points). RM#162 remains open for
broader real-program work, including anonymous functions, closures and shared state.
These are alpha.16 development docs; release pins, the playground and published articles
are unchanged. This completion record changes no runtime or build behaviour and requires
no performance measurement or repeated product validation.


## Callable and state contract proposal, 23 November 2026

The owner selected the next design task after stdlib completion. The
[contract proposal](docs/CALLABLES_AND_STATE_PROPOSAL.md) records syntax, effects, immutable capture,
explicit state, lifetime/cancellation rules, diagnostics and the complete target
uptime/counter service. Four provisional implementation slices carry weights
20/30/30/20; all are proposed with zero earned contribution. Owner review is
pending; this is not implementation approval or a current language specification.
No runtime/performance impact or CI is required for this informational record.
Website and published-article updates await agreement and separate review.

23 November review revision: PR #700 now defines scheduler-preemptible cell
reservations with cancellation before commit, State identity and closure capture
equality, explicit resource-capture restrictions, contextual callback parameter
types and committed-increment counting. Owner contract review remains pending;
implementation has not started and the proposed weights remain unchanged.

PR #700 second-review clarifications: infer callback input types before checking
the body, then infer generic output types from its result; preserve existing
scalar Map-key/Set-element restrictions. Acceptance cases updated. Contract
approval remains pending; implementation and proposed weights are unchanged.

23 November: the owner approved PR #700 for merge and requested a stop. The
callable/state contract is accepted on merge. CS1–CS4 are planned, unstarted
(0/100 earned implementation points); work pauses before CS1.

25 November: the owner resumed CS1 on `feature/rm162-anonymous-functions`.
Anonymous header parsing is under development; 224 parser assertions and the
candidate compiler build pass. Expression integration, contextual checking,
effects and lowering remain; no anonymous-function feature is accepted yet.
CS2–CS4 remain planned. No CI, seed refresh or final full validation at this
checkpoint; implementation weights and zero accepted contribution are unchanged.

27 November: CS1 now parses complete anonymous expressions, including block and
nested bodies. Parameter scopes, qualified annotations, source inventory and
cache encoding retain the new syntax. Focused checks cover 237 parser, 31 resolver,
38 source-span, 28 inventory and 2,042 module-binding/cache assertions; the
candidate compiler builds and verifies. Anonymous functions still fail closed at
type checking: contextual types, effects and noncapturing lowering remain. No CI,
bootstrap refresh or final full validation; no implementation slice is accepted.

27 November, working execution checkpoint: CS1 now checks and lowers noncapturing
anonymous functions using the existing bytecode format. Pure, ordinary and async
bodies have separate effect scopes. Direct annotated targets, direct return
values and callback arguments supply parameter types; map results infer from the
body. Generated functions survive body/code caches and generic type identities.
The focused module-binding/cache suite passes 2,151 assertions, including effect
rejection and fresh/cached equivalence. Source and saved-bytecode execution agree
for the new runtime fixture; the async fixture passes. A confirmed remaining gap
is contextual typing through conditional return branches. Full contextual
propagation, explanation/diagnostic acceptance and final compatibility/full
validation remain before CS1 acceptance. No CI or bootstrap refresh; CS2–CS4 and
the zero accepted implementation contribution are unchanged.

27 November, compound contextual typing checkpoint: expected callable inputs now
flow through if/match results, block tails, reassignment and compound callback
arguments. Nested anonymous bodies also receive an explicit result annotation.
Argument checking replays compound callbacks in source order after gathering
input types, preserving mutation-based invalidation of guarded facts. A paired
regression accepts guarded subtraction without a preceding mutation and rejects
it when an earlier argument writes the guarded variable. Runtime selection
effects occur once.

The module-binding/cache suite passes 2,180 assertions, explanations 266 and
purity 39. Candidate source execution and verified saved bytecode match the
expanded fixed transcript. Remaining work is the contextual-type acceptance
audit (including aggregate/constructor positions), explanation/diagnostic review,
final functional registration and compatibility/full validation. CS1 remains in
progress, with zero accepted implementation points; CS2–CS4 remain planned. No
CI or bootstrap refresh. The extra checking pass is limited to calls containing
compound callback expressions; ordinary calls retain their current path. No VM
or bytecode changes. Website claims remain unchanged until feature acceptance.


27 November, collection and evidence checkpoint: known array element types now
reach anonymous parameters, including nested arrays, returned arrays and callback
arrays supplied as arguments. Existing record constructor signatures also supply
callback inputs. Regressions reject incompatible parameter types, impure bodies
in pure callback arrays and unannotated inputs without context. Explanation
evidence retains the exact anonymous body expression and its checked result; the
missing-parameter diagnostic is asserted verbatim. The module-binding/cache suite
passes 2,205 assertions. Expanded source and saved-bytecode transcripts match.
The candidate passes native-oracle-artifacts for unchanged-program bytecode.

CS1 remains in progress: finish the constructor/generic contextual boundary audit,
end-to-end explanation review, functional registration and final bootstrap/full
validation. No CI or bootstrap refresh. CS2–CS4 remain planned and accepted
implementation contribution stays zero. The array pass reuses source-order child
checking; no VM, bytecode-format or runtime representation change. No website
publication; the proposal and test evidence describe development support.


27 November, CS1 acceptance checkpoint: constructor context now supplies callback
inputs through generic records and enum payloads, including nested arrays and
qualified constructors. Ordinary function calls retain input-only inference; a
regression rejects deriving an anonymous parameter from the receiving result.
Explanation traversal enters anonymous bodies for type-at-position, inferred
locals and guard evidence. The public CLI checks the original source locations.

The module/cache suite passes 2,219 assertions. Ordinary and async functional cases
pass source, compile and saved-bytecode checks; the CLI explanation fixture also
passes all three modes, with 121 internal assertions. The fixture inventory and
its mutation tests pass (616 outer transcript assertions). Candidate unchanged-
program bytecode compatibility passed. Bootstrap refresh produced identical
stage 2/3/4 compilers (88fa8a2ec9c850afb7e64bc714bca7eec194e9e8524e5821b8338e0be53c25ad)
and matching standard-library artifacts/output. Final make check is running.
CS1 remains unaccepted pending that result and integration; CS2–CS4 remain planned.
No development CI or website publication. Specification and architecture now
describe noncapturing support; closures and State are not claimed. The runtime
and bytecode version are unchanged. Constructor-context inference is restricted
to constructor calls with an expected type; ordinary calls keep their rules.


CS1 final local acceptance, 27 November: make check passed in 708 seconds with
host access (unit 421s, bootstrap 94s and functional 220s; bootstrap overlaps
unit). Relocated-package smoke and quick-start passed. The suite caught a stale
watchdog observation hook after the contextual block checker was introduced; the
test-only observer now recognises both block entry points, with explicit stall,
recovery and unchanged-budget assertions. No production VM or timeout policy
change was needed. Earlier sandbox failure and the watchdog failure were fixed
before this successful final run.

CS1 implementation and local acceptance are complete, pending next integration
and hosted acceptance. CS2–CS4 remain planned. Accepted implementation points
stay at zero until integration acceptance; RM#162 remains open. The measured
full-run duration is evidence, not a new performance target. No development CI
or website publication occurred.


CS1 merged into next through [PR #701](https://github.com/sproates/panackelty/pull/701) at `14b709d` on 27 November. [Integration run 46042786755](https://github.com/sproates/panackelty/actions/runs/46042786755) is running; hosted acceptance remains pending. Local make check passed in 708s. CS2–CS4 remain planned; accepted implementation contribution stays zero until that run is verified.


CS1 accepted, 27 November: [PR #701](https://github.com/sproates/panackelty/pull/701) is merged and [integration run 46042786755](https://github.com/sproates/panackelty/actions/runs/46042786755) passed for `14b709d`. Full local make check passed in 708s. Subsequent status changes contain informational Markdown only. CS1: Anonymous functions is complete (20/20 points); CS2: Retained captures (30), CS3: State (30) and CS4: Examples/documentation/installed acceptance (20) remain planned. Programme earned contribution is 20/100. RM#162 remains open; CS1 has no separate issue to close. No website publication or closure/State completion is claimed.


CS2 started, 27 November, on feature/rm162-retained-captures from next 4cc5d00.
The first runtime checkpoint adds an owned closure value with lexical target
identity and retained capture values. Equality compares target and captures, not
allocation identity. Capture eligibility follows actual ownership edges through
collections, records, variants and other closure environments, including hidden
shared-array suffixes. A non-recursive worklist with a visited-pointer set avoids
revisiting shared graphs. Native resources, Void, iterators and unclassified new
value kinds fail closed. Array append checks now follow closure ownership edges
to prevent introducing shared-storage reference cycles.

This is runtime groundwork, not executable source closure support. No instruction
or builtin constructs the new value yet. Source capture analysis, checked
construction/invocation, verifier/effect/arity enforcement, async/native handler
integration, iterator snapshots, cache/provenance evidence and final functional/
full validation remain. CS2 is in progress with zero earned points; CS1 remains
accepted and programme progress is 20/100. No CI, bootstrap rebuild or website
change accompanies this checkpoint.

CS2 checked invocation checkpoint, 27 November: CALL_VALUE and AWAIT_VALUE now
accept internal closure values. Captures precede explicit arguments in the
callee frame; target lookup, arity and the existing pure/async call-edge rules
are checked before entry. Named references keep their existing path. Small
combined argument lists use stack storage; larger lists use a temporary borrowed
pointer vector, with frame ownership handled by the existing retain/release path.
Native tests cover argument order, repeated invocation, the effect matrix,
missing/builtin targets, arity mismatch, suspension, cancellation and callee
traps. Source construction/lowering, host-retained callbacks, iterator snapshots,
cache/provenance and final slice acceptance remain. No bytecode format or compiler
seed change. CS2 remains in progress with zero accepted points (programme 20/100).

CS2 source-lowering checkpoint, 27 November: referenced immutable lexical values
now become closure captures, including captures used by nested closures. Generated
functions receive a hidden capture-parameter prefix; the public callable signature
retains only explicit arguments. Generic types are rebound to the anonymous body
while lexical declaration identities remain available for provenance. Capture
reads are children of the creation node; the nested function body remains a
separate effect scope. Known mutable/resource captures are diagnosed, and the
runtime validates target, layout and resource graphs again at construction.

The reserved pure `$intrinsic_closure` builtin is an additive v9 capability;
there is no opcode or constant-layout change. Older runtimes reject the unknown
intrinsic. Noncapturing and named callables remain unchanged. The compiler rejects
combined capture/explicit parameter counts beyond v9's 255-parameter limit.
Ordinary and async candidate fixtures pass source execution and saved bytecode,
including creator-return lifetime, nested/generic captures, captured callbacks,
collection callbacks, equality and repeated awaits. This is candidate evidence;
the public launcher still uses the unchanged bootstrap compiler. Host-retained
callbacks, iterator snapshots, dedicated capture explanations, bootstrap refresh
and final acceptance remain. CS2 stays in progress; programme earned contribution
is 20/100. No development CI or website publication.

CS2 retained-host checkpoint, 27 November: HTTP routes and fallbacks, contextual
HTTP handlers, TCP workers and service readiness callbacks now retain closure
environments. The shared checked callable descriptor preserves target identity,
explicit arity and async effects; child frames retain captures independently and
server destruction releases retained environments. Verified-program target lookup
uses the existing index. No throughput improvement is claimed.

Native lifetime and allocation-failure checks pass, including sanitizer checks.
The candidate HTTP fixture passes ordinary/contextual routes and fallback in
source and saved-bytecode modes after the creating function returns. The binding
and cache suite passes 2,265 assertions. Captured arrays and ranges can be iterated
from the start on every invocation; loop-created closures retain each iteration's
value. Source and saved-bytecode transcripts agree.

Iterator clarification supersedes earlier pending-snapshot notes: source iteration
exposes iterable values, not the VM's mutable V_ITER cursor. Each loop creates a
fresh cursor. Raw internal cursors remain capture-ineligible; no new public
iterator API or cursor snapshot feature is introduced.

The existing zero-grace TCP cancellation assertion failed once, then passed on
retry and under sanitizers. No timeout or assertion was weakened; watch this
case during final validation. Capture explanations, bootstrap refresh, public
launcher/full validation and integration remain. CS2 is not accepted; programme
progress remains 20/100. No development CI or website publication.

CS2 explanation checkpoint, 27 November: the checker now records capture eligibility
and reasons in explanation mode. Text and terminal views show accepted/rejected
captures and follow lexical identities to declaration context, including shadowed
parameters. Evidence is not inferred from diagnostic text. The ordinary body cache
retains lexical identities but omits explanation evidence; explain checks source
afresh. No cache-schema or bytecode-format change is needed. The terminal's generated
instruction description names closure creation rather than its reserved intrinsic.
Candidate CLI explanation checks pass (123 cases), and unchanged-program bytecode
contracts pass. Bootstrap refresh and final local validation are in progress.
CS2 remains unaccepted and programme progress stays 20/100. CS3 State and CS4 final
examples/installed/website acceptance remain planned; no website publication or
CI has been started by this development checkpoint.

CS2 local acceptance, 27 November: full `make check` passed in 725s, including
unit/functional suites, fixed-point bootstrap, packaging, relocation and quick
start. The existing zero-grace TCP assertion passed in this full run. Focused
capture checks passed 2,313 binding/cache assertions and 123 CLI cases; terminal
navigation, capture origin and TTY restoration passed. Bootstrap compiler and
stdlib conformance outputs matched across stages. Unchanged-program bytecode
compatibility and native closure lifetime/allocation/sanitizer checks passed.
The 725s broad run versus CS1's 708s is not a controlled performance comparison;
existing historical advisory budget warnings remain unchanged.

CS2 implementation and local acceptance are ready for integration. CS2 remains
unaccepted until next integration CI passes; no task points are earned early.
CS1 remains 20/20, CS2 0/30 pending integration, CS3 State 0/30 and CS4 examples/
installed/website acceptance 0/20. Programme accepted progress remains 20/100.
RM#162 stays open; CS2 has no separate issue to close. The existing CS4 website
follow-up covers closure examples/reference updates; nothing is published here.

CS2 integration correction, 27 November: PR #704 merged at `149f629`. Integration
run 46067182572 passed both platform package checks but failed coverage policy:
the new capture_explanations.panack file was missing from the manifest. The follow-up
adds it to eligible compiler sources and refreshes declaration exclusions with the
production parser/inventory (53 eligible files, 556 excluded declarations). No
executable code or coverage threshold is excluded or relaxed. All 77 hosted-policy
tests pass locally. Product code is unchanged from the 725s full validation.
CS2 acceptance awaits the repaired integration gate; programme progress stays 20/100.

CS2 complete, 27 November: retained captures and explanations merged through
[PR #704](https://github.com/sproates/panackelty/pull/704); coverage inventory was
reconciled in [PR #705](https://github.com/sproates/panackelty/pull/705).
[Integration run 46067730132](https://github.com/sproates/panackelty/actions/runs/46067730132)
passed for `31422e0`, including both platform package checks, coverage policy and
the integration gate. Full local make check passed in 725s; focused terminal,
CLI, cache, lifetime, allocation-failure and sanitizer evidence is recorded above.
CS2 earns 30/30 points. CS1 remains 20/20; CS3 State (30) and CS4 combined examples,
installed acceptance and website guidance (20) remain planned. Accepted programme
progress is 50/100. RM#162 remains open; there is no separate CS2 issue to close.
No State implementation, release or website publication is claimed.

### CS3 local acceptance, 28 November 2026

Ordinary and async State cells now pass final local `make check` in 725 seconds,
including native, compiler, functional, bootstrap, archive relocation and quick-start
checks. The final seed is `987b93428bd3bb92becfa011c376d9379ebe97dc03ba9037b822cfbecd78331e`;
three bootstrap stages are byte-identical and stdlib artifacts/output agree.
Unchanged-program artifact checks pass. The full run includes 2,313 binding/cache
assertions, State source/bytecode and cold/warm caller-edit acceptance. Separate
sanitizer and 59 allocation-failure checks passed during development.

The final audit gives forged handles a specific diagnostic, preserves source spans
for tagged named references, and documents State ownership, cancellation and generic
type erasure. Four old test expectations were corrected after reviewing the emitted
instruction offsets and package members: one disassembly, two instruction-source
traces, and the inventory entries for the two new modules. The final full run has
no failures. Runtime payload/callback safety remains enforced with v9 bytecode;
source generic/refinement proofs are not reconstructed from erased bytecode.

Performance: the final full run is comparable to CS2's 725-second run, with no new
numeric budget claim. Collection-key checks use a propagated flag rather than a
recursive walk per lookup; generic obligations converge over finite parameter sets.
Existing historical validation-budget warnings remain informational.

CS3 implementation and local acceptance are complete, pending next integration
checks. CS3 earns 0/30 until that result; CS1 and CS2 retain 20/20 and 30/30.
CS4 combined HTTP examples, installed acceptance and website guidance remains 0/20.
Accepted programme progress is 50/100. RM#162 stays open; CS3 has no separate issue.
Website follow-up remains with CS4; no release or website publication is claimed.

CS3 accepted, 28 November: [PR #707](https://github.com/sproates/panackelty/pull/707) merged at `a5a4a08`; [integration run 46081707303](https://github.com/sproates/panackelty/actions/runs/46081707303) passed. The next workflow ran coverage policy and integration result gates; full platform builds were intentionally skipped. Full local make check passed in 725s. CS3 earns 30/30; accepted callable/State progress is 80/100 (CS1 20/20, CS2 30/30, CS4 0/20).

CS4 started on `feature/rm162-state-examples`: equivalent closure and named contextual HTTP examples, a standalone counter, and focused source/bytecode tests now cover increasing uptime, concurrent increments, uncounted routes, startup clock failure and post-commit clock failure. All cases also pass from a relocated installation with compiler/stdlib overrides removed and the installed compiler hash matched to the validated seed. No compiler or VM source changed. The example-only validation reuses the CS3 product baseline; the mistakenly invoked broad packaging conformance run was stopped, and package assembly/quick-start reused its unchanged native-check prerequisite. Website State API/reference guidance is prepared separately for preview review and approval; CS4 remains unaccepted. RM#162 stays open.


## Callable and State programme complete — 28 November 2026

CS4 is accepted (20/20): core [PR #708](https://github.com/sproates/panackelty/pull/708) and [integration run 46088757576](https://github.com/sproates/panackelty/actions/runs/46088757576) passed. Focused source/bytecode and relocated-installation checks cover both HTTP handler styles, the standalone counter, concurrent counts and clock failures. Website [PR #80](https://github.com/sproates/panackelty-website/pull/80) was approved after preview review and published at `d51cfcc`. [Publication run 46091796386](https://github.com/sproates/panackelty-website/actions/runs/46091796386) reused successful PR validation, including browser/mobile tests, and passed exact deployed-byte/provenance checks. Live concept pages for anonymous functions, closures and State, the cookbook and State API reference were verified. All four concept teaching programs and 55 local website tests passed.

CS1 20/20, CS2 30/30, CS3 30/30 and CS4 20/20: this bounded programme is complete at 100/100. RM#162 remains open as the ongoing real-program-fixes allocation; no separate CS4 issue exists. Alpha.16 is not released, and alpha.15/playground pins are unchanged.
