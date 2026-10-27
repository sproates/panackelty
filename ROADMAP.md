# Roadmap

Panackelty **0.1.0-alpha.14 is published**. See the [release notes](CHANGELOG.md)
and [acceptance evidence](docs/releases/ALPHA14_RELEASE_ACCEPTANCE.md).

## Next: alpha.15

GI#409: Compiler/stdlib boundary assessment is complete on next (PR#635).
The [assessment](docs/COMPILER_STDLIB_BOUNDARY_ASSESSMENT.md) verifies the full
42-file closure and five change cases. Retain source organisation. Narrower
package-tool prerequisites were considered in GI#106 and remain unselected. Native coupling
stays in alpha.16. No refactor is delivered.

GI#106: Validation speed — alpha.15 milestone done. The owner accepted the
implementation and replay on 27 October 2026; implementation is merged in PR#637.
[Acceptance evidence](docs/VALIDATION_SPEED_ACCEPTANCE.md) meets the agreed targets:
clean full validation is 16.1% faster, compiler-edit checks 20.05% faster, and
hosted initial compiler checks 24.9%/36.8% faster. Regression guards passed,
including the ordinary four-worker setting. The bounded alpha.15 milestone is accepted;
the wider GI#106 issue is not closed. No tests or validation cadence were removed.

RM#154: Faster coverage slice A is in progress. The [baseline and cache contract](docs/COVERAGE_PERFORMANCE_BASELINE.md) records the exporter compatibility repair and a blocking inventory-size limit. A prerequisite decision and complete four-workload measurements precede target approval. Cache implementation has not started.

For the current release summary, read [Next release status](docs/NEXT_RELEASE_STATUS.md).
Keep that summary current alongside the detailed records when release work changes.

Alpha.14 delivers type-owned methods and dot notation (GI#525), the routed HTTP
server framework (GI#518), and the HTTP reference and runnable examples slice of
GI#552. Alpha.15 takes forward the remaining foundation work under the agreed
release rebalance, including package-management implementation (GI#200) and the
GI#106 test-speed milestone.

- [Release plans](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md)
- [All roadmap records](docs/roadmap/README.md)
- [Decision and completion process](docs/roadmap/ROADMAP_PROCESS.md)

## Planned direction: alpha.14 through alpha.19

Owner-approved release rebalance, 20 October 2026. Alpha.14 is released; alpha.15
through alpha.19 remain planned. See the
[scope and boundaries](docs/roadmap/records/next-release--agreed-scope-2026-10-06.md#agreed-release-direction--20-october-2026).

- **alpha.14:** completed type-owned methods and HTTP framework, finished with
  the HTTP API reference and runnable examples slice of GI#552. Colours (GI#555)
  were not included.
- **alpha.15:** remaining foundation work: package-management implementation, compiler
  understanding, validated domain types, inference/stdlib assessments, faster
  coverage, selected friction/performance work, and the GI#106 test-speed milestone.
- **alpha.16:** build useful programs: typed service exchanges, file discovery,
  subprocess examples, a tiny build tool and tested guides.
- **alpha.17:** everyday development: one editor, source-aware runtime errors and
  Homebrew installation.
- **alpha.18:** interactive/connected applications: browser event handlers and
  service contract testing; Docker images/Linux ARM64 remain optional.
- **alpha.19:** language ergonomics: error propagation and further guarded types,
  subject to design/evidence.

The full GI#552 reference, GI#553 load profiling and GI#555 colours retain candidate
status beyond the explicitly selected HTTP documentation slice. No dates or new
implementation authorisation are implied. Release validation/publication approval
remain required; scope does not grow through optional candidates.

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
[integration record](docs/design/PACKAGE_GIT_INTEGRATION_ACCEPTANCE.md). The alpha.16 coupling
task stays separate.

<a id="rm-158"></a> [RM#158](docs/roadmap/records/dependency-management--idea.md#rm-158),
<a id="rm-159"></a> [RM#159](docs/roadmap/records/dependency-management--idea.md#rm-159),
<a id="rm-160"></a> [RM#160](docs/roadmap/records/dependency-management--idea.md#rm-160) and
<a id="rm-161"></a> [RM#161](docs/roadmap/records/dependency-management--idea.md#rm-161).

The original modules/packages/HTTP programme baseline remains **90/100 earned**;
P7 retains its deferred 10-point allocation. Alpha.13 release acceptance is complete.
RM#156 records the GI#518 selection and RM#157 records the performance outcome.
RM#158–RM#161 identify the Alpha.15 package-management packets.
**Next available roadmap identifier: RM#162.**

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
