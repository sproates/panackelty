[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Modules, packages and HTTP programme (SP1) — 2026-10-03

<a id="rm-108"></a>

**RM#108: Modules, packages and HTTP programme (SP1)** ·
[GI#233: Modules programme](https://github.com/sproates/panackelty/issues/233).

SP1 is the owner-selected name and sequence for the existing RM#108/GI#233
programme. It reuses the existing work records and weighted baseline; it does
not create duplicate tasks or progress accounting. P2, P3 and P4 are complete
on `next`; their clean-installed language/local-package acceptance evidence
remains part of P8 and did not block P5 or P6.

P2 foundations and slices 1–5 are merged on `next`.
P1 was completed by [PR#240: Module and package design](https://github.com/sproates/panackelty/pull/240),
merged on 2026-10-03 as `80cd50f`. The user selected P2 on 2026-10-04;
its first module/binding foundation merged in
[PR#261: Establish module binding identities](https://github.com/sproates/panackelty/pull/261)
as `a5cdb1a`, followed by qualified-reference syntax and cross-module binding
resolution in PR#262. [PR#275: Checked namespace signatures](https://github.com/sproates/panackelty/pull/275)
merged checked declaration/signature identities as `0bcf092`. PR#284 merged bounded
function-body identities as `9f8f8a0`. These foundations are delivered; P2
selection is separate from design acceptance. P3 and P4 later completed on
`next`; see the current status table and SP1 sequence below.
The compiler-understanding programme was paused at this stage; the later
Alpha.14 direction resumes planning and scope assessment only (see RM#151).

User decision, 2026-10-04 (superseded on 2026-10-14): **P2 → P3 → P4 →
language/local-package release, then HTTP client work (P5)**. The original
checkpoint made clean-installed namespace and local-package evidence the first
of [P8's three gates](#rm-112), before HTTP. The owner later removed the
intermediate release requirement: retain the evidence for final P8 acceptance,
but do not require release or promotion before HTTP. P7 can proceed independently
once P1/P3 are accepted, but is not a prerequisite. This changes sequencing and
partitions acceptance without adding scope, changing programme weights or
allocating a version.

Owner decision, 2026-10-10: before starting P3, complete two namespace follow-ups:
[RM#149: Namespace example set](#rm-149) and
[RM#150: Namespace performance investigation](#rm-150). These follow P2 and
precede P3; neither reopens P2 or changes the original P1–P8 scope weights. RM#150
is also the alpha.12 release prerequisite recorded below. The two follow-ups have
no dependency on each other. The implementation sequence is P2 → RM#149/RM#150 →
P3 → P4 → P5 → P6; clean-installed language/local-package acceptance evidence
is retained for P8, not as an intermediate release before HTTP.

Owner decision, 2026-10-13: name this sequence **Strategic Programme 1 (SP1)**.
The original sequence placed a clean-installed language/local-package release
gate before the HTTP client (P5 / GI#236) and server (P6 / GI#237). P3 and P4
completed on `next` in PRs #386 and #404. The owner superseded the intermediate
release requirement on 2026-10-14: keep clean-installed and relocated-package
checks as final P8 acceptance evidence, but do not require an intermediate
release or core promotion before HTTP. P5 and P6 are now complete on `next` in
PRs #456 and #467; final release acceptance is complete in P8 / GI#238. P7 / GI#200
is not a prerequisite and was moved from Alpha.13 to Alpha.14 on 2026-10-15.

SP1 uses these implementation layers:

1. Before implementation, trace the required package, namespace, async and
   transport behavior through the compiler, VM/runtime interfaces, bytecode,
   host capabilities and bootstrap. Make a compiler-impact inventory, select
   and evidence the native transport/TLS backend, and record the expected
   compiler rebuild and validation costs. This reduces late discovery risk but
   cannot prove that every compiler requirement has been found.
2. Complete the identified compiler/runtime/ABI/bootstrap changes together,
   with their regression coverage, before adding source-level HTTP packages.
   If later work reveals another compiler-affecting requirement, pause that
   layer, update the inventory, and handle it here before continuing. Required
   PR checks may still rebuild the compiler for each PR.
3. Add compiler-independent standard-library support only where it is reusable
   outside HTTP. Keep HTTP-specific parsing and protocol rules in the HTTP
   package; this layer may have no changes if no reusable gap is found.
4. Implement the HTTP package APIs after the compiler/runtime and any selected
   standard-library layers are stable. Preserve the client-before-server
   milestone order and validate each through local packages and the installed
   toolchain.

The package shape is one local `http` package with separately importable
`client` and `server` modules. The client milestone exports `http/client`; the
server module follows after client acceptance. A single shared module owns
`Header`, `Response` and `HttpError`, re-exported by both facades so those types
retain one identity. `Request` belongs to the server API, and client/server
resource limits stay specific to their respective APIs. The accepted P1
[module/package design](../../design/MODULE_PACKAGE_DESIGN.md#http-client-and-server-boundary)
owns the protocol, security, resource and lifecycle contracts.

Owner decision, 2026-10-13: SP1's order is local reusable packages (P3 / GI#199),
standard-library namespaces (P4 / GI#235), the clean-installed language/local-
package release gate (first gate in P8 / GI#238), then the HTTP client (P5 / GI#236)
and server (P6 / GI#237). HTTP work must not start before the first gate passes.
At that checkpoint P7 / GI#200 remained part of final P8 acceptance. The
2026-10-15 decision below removes P7 from Alpha.13 and its P8 release gate.

Owner decision, 2026-10-14: SP1 implementation will finish before the alpha.13
release. The earlier plan to publish/promote a P1–P4 language/local-package
release before P5 is superseded. Keep its clean-installed and relocated-package
checks as P8 acceptance evidence, but they do not block P5 and do not trigger an
intermediate release. Complete the selected Alpha.13 implementation and final
P8 acceptance before promotion or publication. The subsequent 2026-10-15 P7
deferral narrows release scope; the original SP1 weight baseline remains recorded.

Owner decision, 2026-10-14: use libcurl as the initial native client backend for
P5 within the alpha.13 proof of concept. Do not spend a separate pre-implementation
PR on comparing backends. Keep libcurl behind an internal replaceable adapter and
keep Panackelty's HTTP parser and public package contract independent of it. Fold
version/build, licensing/transitive-dependency, verified TLS, asynchronous DNS,
cancellation, packaging and interpreter-free build checks into implementation.
If implementation evidence shows a hard incompatibility with those requirements,
pause and revisit the backend choice. This is the P5 client direction; it does not
select libcurl as a server backend for P6 or create a permanent dependency promise.

SP1's HTTP work follows four layers: inventory compiler, VM/runtime, bytecode,
host and bootstrap impact; complete identified compiler-affecting changes
together; add only reusable compiler-independent standard-library support; then
implement the HTTP package using the libcurl POC direction above. If later work
reveals another compiler need, return to the compiler layer and update the
inventory. This reduces late discovery risk but cannot guarantee there will be no
later compiler rebuilds. The accepted
[module/package design](../../design/MODULE_PACKAGE_DESIGN.md#http-client-and-server-boundary)
owns the package API, transport, security and lifecycle details.

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

The [P1 design](../../design/MODULE_PACKAGE_DESIGN.md) selects syntax, public types/errors,
ownership, transport/DNS/TLS prerequisites, compatibility and migration. HTTP framing is distinct from
today's finite EOF-framed TCP. Each implementation slice retains canonical tests,
bootstrap, docs, independent review and explicit merge approval.

| Stage | Task | State | Dependencies | Estimate |
| --- | --- | --- | --- | --- |
| P1 | [RM#109: Module and package design](#rm-109) · [GI#234: Module and package design](https://github.com/sproates/panackelty/issues/234) | Done | None | Medium / 1 design PR |
| P2 | [RM#41: Language namespaces](language-namespaces--idea.md#rm-41) · [GI#198: Language namespaces](https://github.com/sproates/panackelty/issues/198) | Done on `next` through merged PR#358; alpha.12 promotion/release acceptance remains | P1 | Large; delivered |
| Before P3 | [RM#149: Namespace example set](#rm-149) · [GI#359](https://github.com/sproates/panackelty/issues/359) | Done on `next` through merged PR#362 | Namespace-capable toolchain | Medium / 1 PR |
| Before P3 | [RM#150: Namespace performance investigation](#rm-150) · [GI#360](https://github.com/sproates/panackelty/issues/360) | Done for alpha.12 on `next` through merged PR#363; validation budgets remain exceeded under the accepted disposition | Slice 8 evidence; coordinate with GI#106 and GI#285 | Medium / 1–2 PRs |
| P3 | [RM#43: Local reusable packages](reusable-modules-and-package-structure--idea.md#rm-43) · [GI#199: Local reusable packages](https://github.com/sproates/panackelty/issues/199) | Done on `next` in PR#386; clean-installed release acceptance complete in alpha.13 | P1; P2 boundaries; RM#149 and RM#150 complete | Large / 2–4 PRs |
| P4 | [RM#42: Standard library namespaces](standard-library-module-api-migration.md#rm-42) · [GI#235: Standard library namespaces](https://github.com/sproates/panackelty/issues/235) | Done on `next` in PR#404; alpha.13 release adoption accepted | P1–P3 as needed | Medium / 1–2 PRs |
| P5 | [RM#110: HTTP client package](#rm-110) · [GI#236: HTTP client package](https://github.com/sproates/panackelty/issues/236) | Complete on `next` in PR#456; P8 release acceptance complete in alpha.13 | P1–P3; transport/DNS/TLS | Large, delivered / 4–7 PRs |
| P6 | [RM#111: HTTP server package](#rm-111) · [GI#237: HTTP server package](https://github.com/sproates/panackelty/issues/237) | Complete on `next` in PR#467; P8 release acceptance complete in alpha.13 | P1–P3; transport/lifecycle | Large, delivered / 2–4 PRs |
| P7 | [RM#44: Package management](dependency-management--idea.md#rm-44) · [GI#200: Panack package management](https://github.com/sproates/panackelty/issues/200) | Selected for full Alpha.15 implementation on 2026-10-22; P01–P04 planned, implementation not started | P1 and P3 inform the design; P01–P04 (RM#158–RM#161) form the delivery sequence | 10% of the original SP1 baseline remains unearned; packet weights are recorded under RM#44 |
| P8 | [RM#112: Package and HTTP acceptance](#rm-112) · [GI#238: Package and HTTP acceptance](https://github.com/sproates/panackelty/issues/238) | Done; alpha.13 public artifacts and all three acceptance gates verified | Language/local-package evidence; client evidence; final integration and release artifacts; GI#200 is no longer an Alpha.13 dependency | Medium / 1–2 PRs, provisional; reassess across the gates |

<a id="rm-149"></a>

**RM#149: Namespace example set** ·
[GI#359: Demonstrate namespaces with a tested example set](https://github.com/sproates/panackelty/issues/359).
Before P3, deliver complementary programs under `examples/namespaces/`, including
a multi-file application and focused examples for distinct namespace facets.
Use only the supported contract: aliases, selected imports, qualified names,
public visibility, re-exports and same-named declarations. Document the
programs and check exact output from source and saved bytecode. Keep examples
deterministic and do not imply package, HTTP or other future support. This
bounded task coordinates with the broader
[GI#310 example expansion](https://github.com/sproates/panackelty/issues/310)
and does not complete it. This delivery adds the trip planner, twin sensor type,
and public API facade programs. It adds three examples (nine public CLI source,
compile and bytecode executions); there is no compiler/runtime implementation
change. No website claims change in this core-only delivery.

<a id="rm-150"></a>

**RM#150: Namespace performance investigation** ·
[GI#360: Attribute namespace migration performance costs](https://github.com/sproates/panackelty/issues/360).
The Slice 8 retrospective and pinned validation profiles reproduce and attribute
the matched runner-smoke change. Separate compiler phases from subprocess and
test orchestration; test possible repeated module discovery/loading,
identity-aware checking/emission cost and expanded import closure rather than
assuming a cause. The recorded single-host 30.005s → 55.515s result is not an
isolated compiler measurement or proof of namespace causation. Reuse GI#106's
budget evidence and contribute findings to the broader
[RM#134/GI#285 compiler investigation](groomed-development-and-engineering-work.md#rm-134). Preserve tests and budgets.

**User decision, 2026-10-10:** deliver and validate at least one evidence-backed
compiler performance improvement before alpha.12 is released. This makes RM#150
a release prerequisite; it does not imply that the 120s clean-check or 15s
focused-check budgets have been met. The first measured candidate indexes parsed
declarations by name for signature resolution. Its three-pair compiler-source
check comparison shows a 3.5% median reduction on this host. Splitting
registered coverage-session scenarios into four isolated probes preserves all
41 assertions while
reducing warm `make check-compiler` from 104s to 82s at two workers and from 96s
to 77s at four workers. Host-enabled canonical `make check` passes in 286s
(unit 115s, functional 11s, bootstrap 133s, release smoke 0s, quick start 1s).
The 120s full, 60s bootstrap and 15s focused targets remain unmet; the aggregate
unit phase is not a focused incremental sample. The seed namespace acceptance
fixture was updated for the indexed `LoadedModule` API and passes both focused
and canonical acceptance. This one local run confirms correctness, not stable
timing attribution. See the [performance profile](../../../tests/VALIDATION_PROFILE.md#namespace-declaration-lookup-index--2026-10-10).
This namespace-specific work completes neither broader GI#106 nor RM#134.

**RM#150 status: Done for the alpha.12 performance gate.** PR #363 delivered the
measured compiler lookup and validation-runner improvements; the clean canonical
and hosted-profile follow-up and the accepted validation-time trade-off are
recorded in the [performance profile](../../../tests/VALIDATION_PROFILE.md#alpha12-performance-close-out--2026-10-10).
This completes neither GI#106 nor RM#134. RM#150 remains outside the P1–P8
weighted programme baseline and was required before P3 starts.

The bounded clean-run concurrency experiment then passed canonical `make check`
in 530.32s at two workers and 379s at four workers on the same clean revision and
macOS arm64 host; the unit phase moved from 354s to 202s while bootstrap remained
135s. Local Make-driven validation now defaults to four workers. This is one
ordered pair and remains host-specific evidence; CI's explicit worker settings
are unchanged. The 120s full, 60s bootstrap and 15s focused targets remain open.
See the [clean validation comparison](../../../tests/VALIDATION_PROFILE.md#clean-validation-worker-count-comparison--2026-10-10).

<a id="rm-109"></a>

**RM#109: Module and package design** — **Done**;
[GI#234: Module and package design](https://github.com/sproates/panackelty/issues/234).
Delivered [source-grounded design](../../design/MODULE_PACKAGE_DESIGN.md): module identity,
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
Owner selected P6 before P5 on 2026-10-14. P6 is divided into three implementation
slices in `docs/design/MODULE_PACKAGE_DESIGN.md`. Slice 1's context-bearing streaming
worker and capped per-child reads merged in PR#447, preserving `tcp_serve` EOF
behavior. Slice 2's Panackelty-owned HTTP/1.1 parser, response serializer, shared
public types and finite `http/server` facade merged in PR#448; source and saved-
bytecode parser/serializer regressions pass. Slice 3 completed in [PR#467](https://github.com/sproates/panackelty/pull/467),
merged to `next` as `0ee6694`. It adds P5-client interoperability to the existing
C-peer, lifecycle, runnable-example and installed/source/bytecode acceptance.
The P5 client runs from source and saved bytecode against servers running from
source and saved bytecode. GI#237 is complete; P8 final release acceptance
remains. Server TLS remains outside the initial contract.

<a id="rm-112"></a>

**RM#112: Package and HTTP acceptance** — P8 in GI#238 has three gates.

**Done — 2026-10-15.** All three gates are accepted in published alpha.13:
full promotion and Release validation, both native CI-smoked archive bytes
verified against public downloads, exact source provenance/checksums, and local
public macOS installed/relocated source/bytecode acceptance. See the
[release evidence](../../releases/ALPHA13_RELEASE_ACCEPTANCE.md). GI#238 closes with this
completion record; P7 remains deferred and is not counted as delivered.

Owner decision, 2026-10-15: GI#200 package management has moved to Alpha.14.
Its discussion and any later design or implementation do not block Alpha.13 or
P8 acceptance. The selected Alpha.13 outcome for GI#134 is complete; its broader
checker-decision scope moves to Alpha.14 and does not block this release. GI#162
is delivered through core PRs #495–#498 and merged website PR #42; Alpha.13
release acceptance is complete, as recorded below.

1. **Language/local-package evidence for P1–P4.** A clean installed toolchain
   builds and runs two independent applications consuming the same local library;
   relocated installations and package trees work. Namespace/stdlib migration,
   executed documentation, fresh bootstrap and the required native/browser
   baseline pass. Keep this as P8 acceptance evidence; no intermediate release
   or core promotion is required before HTTP. HTTP server implementation and P7
   package-management design are outside this evidence checkpoint.
2. **HTTP-client acceptance after P5.** Verify the independently consumed client
   with local packages, including the bounded hostname HTTP/HTTPS, TLS rejection,
   failure and cleanup evidence above. P6/P7 do not block this gate.
3. **Final programme acceptance.** Verify the scoped implementation outcomes,
   including P6 server lifecycle/walkthroughs, with clean/relocated workflows,
   executed documentation and supporting release availability. GI#200 package
   management is now scoped to Alpha.14 and is not an Alpha.13 release condition.

The gates partition P8's existing scope and weight; they do not create three new
tasks or award completion merely for announcing a checkpoint. GI#166 typed
service exchange and GI#162 preview-server replacement remain separate from
SP1's P8 gates; GI#162 remains an Alpha.13 outcome and is not silently completed
by raw HTTP.

P2/P3/P4/P7 reuse RM#41/RM#43/RM#42/RM#44 respectively. This register supersedes
their earlier Idea/unscheduled and assessment-only status: P2 slices 1–8 and the
identity-only cutover are complete on `next` through merged PR#358. RM#149 and
RM#150 are also complete on `next`; their alpha.12 evidence is recorded above.
P3's confined package-root read prerequisite merged in PR#372. PR#374 integrated
the strict `panack.json` parser with explicit dependency-free application
`check`, `compile` and `run --project` commands, confined source loading and
relocation-stable module identity. PR#375 added checking of exported library
modules without a `main` entry. PR#376 added direct local-library dependencies,
export-only package imports and two-app shared-library acceptance. The current
slice pins source reads to the physical package-root identity captured from its
manifest and rejects an exported-source symlink that escapes the package root.
It also exercises the package loader during atomic replacement of an application
root symlink between two valid trees: a successful run must read one coherent
tree, and changed or temporarily unavailable roots fail closed. PR#379 rejects
physical source-root overlap across the application and direct dependencies.
PR#380 adds bounded nested graph resolution with owner-scoped aliases,
shared-package reuse and cycle rejection. PR#382 merged nested package version
and physical-root conflicts plus acceptance/rejection at exact graph limits.
PR#383 checks the 256-loaded-source, 1 MiB-per-source and 8 MiB-total boundaries
in the loader, plus oversized-source rejection through the CLI. PR#384 makes the
root-replacement race fixture reliable on Linux and macOS. The final acceptance
delivery tests two applications sharing a multi-file library, moves the whole
sibling workspace, then rechecks/runs both apps and executes pre-move bytecode
from an unrelated working directory. This closes scoped P3 package acceptance
effective on merge; P8 retains the separate clean-installed release gate. P4
subsequently completed on merge of PR#404. At this earlier checkpoint P5–P7
remained planned and P8's first gate was in progress; current P5 status is in
the alpha.13 summary and P5 stage row above. Their shared design and remaining
HTTP/reproducibility slices are recorded in P1.
No public registry, separate compilation, web framework or new engine is
required. Website impact: none for this test-only acceptance; it does not change
shipped behavior or user-facing package documentation.

Current scope supersedes the historical checkpoint above: P1–P6 and P8 are
accepted in published alpha.13. P7 is selected for full Alpha.15 implementation
under the owner decision of 22 October 2026. Its 10% remains unearned on the
original SP1 baseline; the four packet weights in RM#44 are a separate planning
baseline. No public registry, separate compilation, web framework or new engine
is required.

Provisional scope/effort baseline recorded on 2026-10-03 in
[GI#233: Modules programme](https://github.com/sproates/panackelty/issues/233):

| Stage / task | Programme weight | Task completion | Earned contribution |
| --- | ---: | ---: | ---: |
| P1 / [RM#109: Module and package design](#rm-109) | 10% | 100% | 10 pp |
| P2 / [RM#41: Language namespaces](language-namespaces--idea.md#rm-41) | 20% | 100%; coordinated migration, identity-only route, compiler, canonical, native/browser, installed and 59-root acceptance completed in merged PR#358 | 20 pp |
| P3 / [RM#43: Local reusable packages](reusable-modules-and-package-structure--idea.md#rm-43) | 15% | 100%; PR#386 and alpha.13 clean-installed release acceptance | 15 pp |
| P4 / [RM#42: Standard library namespaces](standard-library-module-api-migration.md#rm-42) | 5% | 100%; PR#404 and alpha.13 release adoption | 5 pp |
| P5 / [RM#110: HTTP client package](#rm-110) | 20% | 100% implementation on `next`; P8 acceptance complete in alpha.13 | 20 pp |
| P6 / [RM#111: HTTP server package](#rm-111) | 15% | 100% implementation on `next`; P8 acceptance complete in alpha.13 | 15 pp |
| P7 / [RM#44: Package management](dependency-management--idea.md#rm-44) | 10% (original SP1 implementation baseline) | Selected for full Alpha.15 implementation; P01–P04 planned, implementation not started | 0 pp |
| P8 / [RM#112: Package and HTTP acceptance](#rm-112) | 5% | 100%; published artifact and installed/relocated acceptance | 5 pp |
| **Total** | **100% original SP1 baseline** | P1–P6 and P8 accepted in alpha.13; P7 planned for Alpha.15 | **90 pp earned on the original baseline; P7 retains 10%** |

P1 earned its design-task credit through merged PR#240. P2 is complete on `next`:
its four foundations are PR#261, PR#262, PR#275 and PR#284; slice 1 is PR#297; slice 2
is complete through PR#298; slice 3 is complete on `next` through
[PR#321](https://github.com/sproates/panackelty/pull/321) and
[PR#326](https://github.com/sproates/panackelty/pull/326); and slice 4 is complete
through [PR#327](https://github.com/sproates/panackelty/pull/327). PR#319's slice 3 delivery was reverted from `main`
by PR#320; PR#321 delivered the core/indirect-call work on the selected `next`
integration branch, followed by the completed contract matrix in PR#326.

Slice 6 has seven sequenced acceptance gates. PR#331, PR#333, PR#334 and PR#337
provide earlier partial evidence; PR#342 completes callable identity emission
(6a), and PR#343–PR#348 add evidence for gates 6b–6g. This delivery hardens the
compiler driver with one staged-root program that checks all seven gates together:
identity-aware emission, ordinary bodies, same-spelled aggregates and patterns,
proof/effect behavior, confined source loading, saved-bytecode execution after
source deletion, and shared core identity. PR#349 merged this integrated
acceptance to `next` on 2026-10-08 and closes slice 6. PR#350–#355 completed
slice 7; PR#358 completed slice 8, coordinated source/seed migration, and
browser/installed acceptance. GI#198's implementation acceptance is complete;
the alpha.12 promotion and package gates remain separately in RM#143.

Merged PR#358 completes Slice 8. The unchanged scope-weighted assessment gives
P2 100% completion: foundations 20%, slices 1–5 45%, slice 6 14%, slice 7 6%,
and slice 8 15%. With P2's 20% programme weight, it contributes 20 percentage
points. P1, P3 and P4 add 10, 15 and 5 points respectively, bringing the SP1
baseline to 50 points delivered on `next`. P5 and P6 add 35 points, bringing
implementation delivery to 85 points on the original baseline. Completed P8
release acceptance adds 5 points, bringing the total to 90; P7 retains its 10%
weight in Alpha.14. Namespace execution and clean-installed package evidence
are accepted for the checked subset.
Namespace/compiler integration and HTTPS client correctness
were the largest implementation shares; local packages and server lifecycle
followed, with smaller shares for design, reproducibility, migration and final
cross-task acceptance. These coarse estimates include each task's own tests/docs;
P8 covers the separate integration/release acceptance, without counting them twice.
P1 records bounded protocol scope; transport/backend feasibility remains a P5
prerequisite and may change its estimate. Keep this baseline stable until explicitly revised with its reason and effect on the total;
implementation sizes remain uncertain. Follow the shared
[programme tracking rules](../ROADMAP_PROCESS.md#programme-tracking).

Track each task as Planned, In progress, In review, Verification pending or
Accepted. On selection record owner, PR/revision, evidence, blockers and next action
in its issue and update this register at delivery. PR#358 completes the P2
production task on `next`; release-specific acceptance remains under RM#143. A
design or experiment does not complete a production task. Website impact: the
site remains truthful for alpha.11; prepare the separate alpha.12 adoption after
public artifact verification. P1 design acceptance is complete. On `next`, P2 slices
1–8 are complete on `next`: PR#297 delivered slice 1; slice 2 is complete; PR#321 and
PR#326 complete slice 3; PR#327 completes slice 4; PR#329 completes slice 5;
and PR#349 completes slice 6. PR#351 adds match-pattern span retention and
precise arm-diagnostic locations, PR#352 declaration-signature type spans, PR#353
body-local annotation spans, and PR#354 explicit call type-argument spans. This
delivery completes Slice 7 acceptance. Slice 8 migrates compiler/tooling imports and standard-library API exports,
refreshes the seed, and passes native, browser, installed-package, and maintained
59-root identity-only acceptance. The legacy flat project route is removed;
incomplete identity closures fail closed. The final delivery PR records this
Done summary in PR#358; its exact-head checks passed before merge. GI#198's
implementation criteria are complete; alpha.12 promotion and release checks are
tracked separately in RM#143.
P3 and P4 are complete on `next` in PR#386 and PR#404. The clean-installed
language/local-package release evidence is accepted in alpha.13. P5 and P6
implementation and P8 acceptance are complete.
Website impact: no adoption update has been published. The released alpha.11
artifacts do not expose namespaces, so their version-pinned examples remain
accurate. The later namespace release checkpoint requires its own adoption
follow-up.

### Namespace release checkpoint

The 2026-10-04 sequencing decision originally placed a release before HTTP.
The owner superseded that requirement on 2026-10-14: complete P2, P3 and P4 in
sequence, but retain clean installed two-application local-package use,
relocation, coordinated stdlib/source migration and required native/browser
baseline as final P8 acceptance evidence. No intermediate release or core
promotion was required before P5/P6. P7 remains independent after P1/P3 and is
not a release prerequisite.

The 2026-10-03 decision originally selected coherent namespace migration as a
release checkpoint. The intervening [RM#128: Alpha.11 release](developer-preview-alpha11-release.md#rm-128), selected
on 2026-10-04, shipped already delivered features without waiting for namespaces.
The current P8 acceptance retains the namespace migration requirements and makes
local-package readiness and standard-library migration explicit. Record
before/after import, visibility and name-resolution
migration notes and website adoption follow-ups, preserving truthful pinned claims.

Choose the version under the release policy when final SP1 acceptance is ready.
No version is allocated, no move beyond alpha is implied, and no future merge or
publication is approved by this planning record. Scope and weights remain
unchanged; P2 is partial and the formally accepted programme contribution remains
10 pp. Current estimated programme progress, including P2's partial 11 pp, is
approximately 21 pp as recorded in the programme register.
