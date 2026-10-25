[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is the retained work record for Alpha.15 package management. Dates and
checkpoints describe their original context; see the roadmap for current status.

## Dependency management

<a id="rm-44"></a>

**RM#44: Dependency management** · [GI#200: Dependency management](https://github.com/sproates/panackelty/issues/200).

On 22 October 2026, the owner selected the full implementation of the package
management model for Alpha.15. GI#200 remains open through implementation and
final acceptance. The work is sequenced as four packets: lock and restore
contract, locked local dependencies, pinned remote sources and `panack` workflow
integration.

The selected sources are:

- Local directories, relative or absolute. They stay at their declared paths.
- Local `.tar.gz` archives.
- Git repositories, optionally selecting a package subdirectory.
- Remote `.tar.gz` archive URLs.

Local and remote archives are materialized under
`.panack_modules/<package-name>`. Git repositories use a shared snapshot under
`.panack_modules/.sources/<source-key>`, keyed by canonical URL, object format
and full commit. Each selected package subdirectory resolves directly within
that snapshot, so repeated or diamond-shaped dependencies share one physical
package root. This is a per-checkout cache and is reused only when its recorded
identity matches the lock. Local directories are not materialized. The lock
records source identity and exact remote content; it does not record whether a
checkout has already unpacked a dependency. Absolute local directory entries
are machine-specific. The `panack` command layer obtains and prepares sources,
while the compiler receives resolved paths and does no network access. Archive
extraction must stay within its dependency destination. Offline restore can
verify and extract a locked local archive from its declared path without a
prior cache entry. Git sibling lookup is limited to the shared repository
snapshot, while source symlinks remain confined to each package root during
compilation.

Package dependencies remain strict entries in `panack.json`, and direct edits to
that manifest remain supported alongside dependency commands. Registries,
package publication, bytecode distribution, native library management and
dependency build scripts are outside the selected scope.

## Alpha.15 implementation plan

Packet weights are provisional estimates of relative scope, including
implementation, tests, integration and documentation. They account for all of
GI#200's implementation scope and remain fixed unless new evidence supports a
recorded re-estimate.

| Packet | Work | Dependencies | Status | Weight | Complete | Earned |
| --- | --- | --- | --- | ---: | ---: | ---: |
| P01 | [Lock and restore contract](https://github.com/sproates/panackelty/issues/566) | — | Done | 15% | 100% | 15 pp |
| P02 | [Locked local dependencies](https://github.com/sproates/panackelty/issues/567) | P01 | Done | 25% | 100% | 25 pp |
| P03 | [Pinned remote sources](https://github.com/sproates/panackelty/issues/568) | P01–P02 | Done (PR#582) | 35% | 100% | 35 pp |
| P04 | [`panack` workflow integration](https://github.com/sproates/panackelty/issues/569) | P01–P03 | P04a complete on merge of the completion PR; P04b–c planned | 25% | 50% | 12.5 pp |
| **Total on merge of P04a completion** |  |  |  | **100%** | **87.5%** | **87.5 pp** |

The [P01 contract](../../design/PACKAGE_MANAGEMENT_CONTRACT.md) was accepted in
[PR#571](https://github.com/sproates/panackelty/pull/571). It defines the lock,
sources, command behaviour, cache safety and acceptance; it does not claim that
package-management functionality has been implemented. GI#200 remains open
through the full graph and public-command acceptance in P04.

On 23 October 2026, [PR#573](https://github.com/sproates/panackelty/pull/573)
reverted the initial P02 archive implementation and restored the P01 tracked
tree. P02 receives no earned completion from that withdrawn work. The revised
[implementation boundaries](../../design/PACKAGE_MANAGEMENT_IMPLEMENTATION.md)
require package policy in Panackelty, a separately built tool and narrow native
adapters, using zlib for gzip. P02 starts with a working compiler handoff and
proof that package-only edits do not rebuild the compiler. It then delivers the
local public-command workflow; P04 completes installation and cross-source
acceptance. These refinements retain the four packet weights.

This planning change has no runtime performance impact and makes no claim of
improved build times. Implementation must measure independent build behaviour,
cold/warm operation and memory use. Website impact: none; no released behaviour
or website claim changes.

## Packet records

<a id="p01"></a>
<a id="rm-158"></a>

**P01 · RM#158: Lock and restore contract** · [GI#566](https://github.com/sproates/panackelty/issues/566).
Done in PR#571; 15% of GI#200. Defines the implementation contract and acceptance.

<a id="p02"></a>
<a id="rm-159"></a>

**P02 · RM#159: Locked local dependencies** · [GI#567](https://github.com/sproates/panackelty/issues/567).
Done on merge of PR#575; 25% of GI#200. The separate Panack package tool resolves
local directory and archive graphs, writes and validates locks, restores offline,
and updates selected dependency closures. The compiler consumes a checked
descriptor. Package-only edits do not rebuild the compiler. Generic filesystem,
zlib and OpenSSL adapters support Panack-owned package policy.
The [acceptance record](../../design/PACKAGE_LOCAL_ACCEPTANCE.md) covers public
CLI behaviour, transitive archives, relocation, tampering, bounds, independent
hash vectors, sanitizers, bootstrap reproducibility and installation. Publication
is atomic per entry; a late publication failure requires restore. Remote sources
and all-source workflow acceptance remain P03 and P04.

The owner approved OpenSSL 3+ for generic SHA-256 after the pure-Panack version
took 35.05 seconds for the 64 KiB benchmark. The EVP version took 0.11 seconds
with the same digest. Archive timing and memory observations are recorded with
their limits in the acceptance record. Website claims wait for a matching native
release, as recorded in the website follow-up register.

<a id="p03"></a>
<a id="rm-160"></a>

**P03 · RM#160: Pinned remote sources** · [GI#568](https://github.com/sproates/panackelty/issues/568).
Done on merge of PR#582; 35% of GI#200. Implements Git and remote archive acquisition.
The owner selected P03a HTTPS archives first, followed by P03b controlled Git
acquisition, P03c Git graph integration and P03d remote-source acceptance.
The separate generic streaming HTTP prerequisite merged as PR#576. It resolves
the existing buffered client's 1 MiB body limit without reducing P01 archive
bounds. This prerequisite alone earns no P03 completion credit; GI#568 stays open.

P03a completes on merge of [PR#578](https://github.com/sproates/panackelty/pull/578),
with [HTTPS archive acceptance](../../design/PACKAGE_HTTPS_ACCEPTANCE.md).
The existing parent weights are unchanged. To credit this delivered sub-outcome,
P03 now has a provisional internal scope allocation: HTTPS archives 30%, controlled
Git acquisition 35%, Git graph integration 20%, and cross-source acceptance 15%.
Git acquisition carries the largest share because process/configuration isolation
and exact-commit snapshot safety remain substantial work. These are scope
estimates, not counts of packets. P03a earns 10.5 percentage points of GI#200;
P03b completes on merge of [PR#580](https://github.com/sproates/panackelty/pull/580),
after the generic process/filesystem prerequisite merged as PR#579. Its
[acquisition record](../../design/PACKAGE_GIT_ACQUISITION_ACCEPTANCE.md) covers
isolated exact-commit snapshots, canonical digests, safe links, unsupported-content
rejection, cleanup and source/bytecode HTTPS tests on Linux and macOS. This earns
12.25 further percentage points, taking P03 to 65% and GI#200 to 62.75% on merge.
The prerequisite itself earns no separate credit. P03c completes on merge of
[PR#581](https://github.com/sproates/panackelty/pull/581): public Git manifests, selector resolution, full-commit locks,
shared snapshots and sibling/diamond graphs. Its
[integration record](../../design/PACKAGE_GIT_INTEGRATION_ACCEPTANCE.md) captures
acceptance and limitations. It earns 7 further percentage points, taking P03 to
85% and GI#200 to 69.75%. [P03d / PR#582](https://github.com/sproates/panackelty/pull/582) completes the remaining remote-source acceptance,
with the [acceptance matrix](../../design/PACKAGE_REMOTE_ACCEPTANCE.md) covering
mixed Git/HTTPS selective updates, offline verification and failure preservation.
On merge, P03 is 100% and GI#200 is 75%; GI#568 is complete. P04 remains planned
and GI#200 stays open.
Native-interface decoupling remains
alpha.16 work. Website impact: no change to published alpha.14 claims; document
the new generic APIs when the matching native release is promoted.

<a id="p04"></a>
<a id="rm-161"></a>

**P04 · RM#161: `panack` workflow integration** · [GI#569](https://github.com/sproates/panackelty/issues/569).
P04 remains in progress and represents 25% of GI#200 overall. Completes installation, help, diagnostics and end-to-end
acceptance across all sources. Ordinary project commands never restore implicitly.

### P04 delivery slices

The owner selected this delivery breakdown on 25 October 2026. P04 remains 25%
of GI#200. P04a completes on merge of the final diagnostic correction; P04b and P04c remain planned. The provisional
internal allocation is 50% diagnostics, 25% installed workflow and 25% final
acceptance, reflecting the larger diagnostic implementation and test surface.
These are scope weights, not PR counts. The earlier P04a acceptance was reopened after wording review. The final
[source audit and acceptance checklist](../../design/PACKAGE_DIAGNOSTIC_ACCEPTANCE.md)
closes that correction on merge, restoring P04a's 12.5 percentage points and
bringing the programme to 87.5%. The denominator and all task weights are unchanged.

| Slice | Deliverable | Status | Share of P04 | Programme weight | Complete | Earned | Estimated PRs |
| --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| P04a: Diagnostics | Preserve causes and dependency context; explain failures and safe recovery; test public messages and document real examples | Complete on merge of final correction | 50% | 12.5% | 100% | 12.5 pp | Final PR |
| P04b: Installed workflow | Verify both tools and native dependencies are packaged correctly; finish command help and installed-tool behaviour | Planned | 25% | 6.25% | 0% | 0 pp | 1 |
| P04c: Final acceptance | Exercise all four source types together, relocation, standalone commands and network-free ordinary builds on supported platforms | Planned | 25% | 6.25% | 0% | 0 pp | 1 |
| **P04 total** | | **In progress** | **100%** | **25%** | **50%** | **12.5 pp** | **2 further after P04a** |

P04a delivers structured errors, labelled dependency context, explanations and
safe recovery guidance. The [verified examples](../../design/PACKAGE_DIAGNOSTIC_EXAMPLES.md)
are tested transcripts with reproducible scenarios and pin-preserving recovery.
No compiler, native interface or bootstrap change is included.

Deliver P04a first, then P04b and P04c. PR counts are estimates, not acceptance
criteria or permission to expand scope. GI#569 stays open until all three slices
are accepted; GI#200 stays open through final programme acceptance.

P04a treats diagnostics as a first-class user experience. Each failure must explain
what happened, why it prevents the requested operation and, where possible, what
the user can safely do next. Identify the dependency alias, declaring manifest,
failed phase and relevant transitive chain. Preserve useful causes without
exposing secrets; state uncertainty rather than inventing a cause or remedy.
Recovery advice must distinguish restoring locked content from updating selected
content, especially after integrity failures. Cryptic messages are unfinished work.

Maintain a diagnostic examples document using actual output from reproducible
public CLI scenarios. Include the command, triggering context, tested version,
explanation, recovery guidance and successful recovery where practical. Keep it
aligned with acceptance tests and distinguish proposed examples from verified
output. This document supplies material for a later article; writing or publishing
that article is separate work.


The previous design-only Alpha.15 direction and its unresolved product questions
were superseded by the owner's 22 October decision. PR#240's historic P7 design
remains context only where it does not conflict with the current contract.

[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

P04a owner review after [PR#584](https://github.com/sproates/panackelty/pull/584)
found vague manifest prose and a selective-update explanation that combined
different causes. The follow-up reviews all package diagnostic producers,
rewrites internal terminology, separates known causes and checks distinct
recovery advice. This is a correction within P04a; weights and P04b/c scope
are unchanged.

The 25 October wording correction is being delivered incrementally. The first
implementation updates lock/cache recovery, archive checksum explanations,
manifest and lockfile JSON errors, selected-update failures and HTTP causes.
The [verified examples](../../design/PACKAGE_DIAGNOSTIC_EXAMPLES.md) record actual
output and distinguish this implementation from the remaining reviewed Git and
archive catalogue. Git-specific work is covered by the correction below;
archive and general-validation messages remain outstanding. Neither correction completes P04b/c
or earn additional programme credit. The installation guide now explicitly says
Git is optional unless a dependency requires Git acquisition.

Performance impact is confined to failure reporting: rejected JSON is parsed
again to distinguish syntax errors from invalid package settings. Successful
parsing and dependency selection are unchanged. No compiler or native-interface
changes are included. Website impact: none; these diagnostics belong to
unreleased alpha.15, and the live alpha.14 claims remain unchanged.

Validation for this first correction: full `make check` passed on 25 October
in 761 seconds, including unit, bootstrap, functional and release-smoke checks.
Focused evidence includes 30 diagnostic assertions, 61 CLI assertions and twelve
captured public-command transcripts. These results verify this slice, not the
outstanding diagnostic catalogue. Timing is one local run, not a paired
performance comparison; the printed legacy timing budgets remain under GI#106.

### Git diagnostic correction

The next correction preserves structured errors through Git acquisition,
reference parsing, inventory, link resolution and cache verification. Git version
2.29 is checked explicitly before acquisition; missing Git, an old release and
unrecognisable output have separate explanations. Reference lookup distinguishes
no match, ambiguity and malformed output. Submodules, LFS pointers and invalid
links retain their specific causes and affected paths instead of becoming a
catch-all fetch error. Process status remains visible without raw Git stderr.

Acceptance includes 74 Git unit assertions, controlled process-failure CLI tests,
real HTTPS Git repository tests and six additional documented transcripts. The
remaining archive/general-validation catalogue must still be completed before
closing P04a; P04b/c remain planned and GI#569 stays open.

Performance: version detection adds a bounded local Git subprocess when Git is
selected for acquisition; it adds no network request. Warm builds and verified
cache reuse still invoke no Git, covered by the existing invocation-count tests.
Cold-lock timings are recorded by the integration suite, but no paired regression
claim is made from a single run. Compiler/native interfaces are unchanged.
Website impact: none; these package diagnostics remain unreleased alpha.15 work.

### P04a archive diagnostic correction

This slice preserves structured causes through tar/PAX parsing, gzip extraction,
local cache verification and HTTPS streaming. Unsupported archive features,
malformed data, path conflicts, limits and filesystem failures have distinct
explanations. An extracted-tree mismatch no longer claims that the compressed
archive bytes changed. Fifteen additional actual CLI transcripts are recorded
in the diagnostic examples document, bringing the total to 34. The general
validation catalogue remains before P04a acceptance; P04b/c remain planned.
Programme weights and the 75% accepted P01–P03 subtotal are unchanged.

Performance: valid archives retain the same streaming bounds, hashing and
filesystem operations. The generic HTTP fold adds no network calls or buffering;
error mapping occurs only on failure. Existing cold/warm tests provide timings,
not a paired regression measurement. Compiler/native interfaces are unchanged.
Website impact: none; this remains unreleased alpha.15 work.

### P04a command, input and graph correction

This intermediate slice distinguishes unsupported commands/options, selected
updates without a baseline lock, input read failures, file kinds and byte/depth
limits. Graph errors explain application dependencies, cycles and conflicting
versions/directories, with both affected locations. Saving the initial lock
reports that publication has not begun. Fifty diagnostic unit assertions and
23 additional public transcripts cover these changes, bringing the example
catalogue to 57. Suggested initial-lock recovery is executed by the tests.

Remaining P04a acceptance includes the wider stale-lock/record validation,
publication and redirect/HTTP response catalogue. P04b/c remain planned; weights
and the 75% accepted P01–P03 subtotal are unchanged. This delivery does not close
GI#569 or GI#200.

Performance: command validation adds one argument scan before existing
work. Successful graph resolution and acquisition retain their operations;
additional context is constructed on failure. No compiler/native change or
paired performance claim is made. Website impact: none, because these changes
remain part of the unreleased alpha.15 package workflow.

Validation: full `make check` passed on 25 October in 718 seconds, including
unit, functional, bootstrap, release relocation and quick-start checks. Focused
checks passed all 50 diagnostic assertions and 61 CLI assertions. The package-tool
mutation test proves that a diagnostic edit rebuilds no compiler or seed artifact.
The examples and final diff were reviewed; the remaining P04a catalogue is not
claimed complete. Historical timing budgets remain subject to GI#106 reassessment.

### P04a lock and publication diagnostic correction

Delivery and final validation results: [PR#592](https://github.com/sproates/panackelty/pull/592).

This intermediate slice explains invalid lock records, including source kinds,
checksum formats, missing/duplicate packages and conflicting versions. Changed
project and dependency manifests are identified separately, with the affected
dependency manifest path. Concurrent edits are classified as changing files,
not as malformed lockfiles. Lock validation uses package-tool wording without
changing the compiler's shared JSON parser.

Publication errors name the destination and distinguish a failed package save
from a failed state-record save after package directories were prepared. Recovery
uses an explicit restore command with the full project path. Tests preserve
conflicting destinations and existing locks, then execute recovery and run the
program. Eighteen additional exact transcripts bring the catalogue to 75; focused
evidence includes 27 lock-parser and 57 diagnostic unit assertions.

The remaining redirect/HTTP response variants still need acceptance before
closing P04a. P04b/c remain planned; programme weights and the 75% accepted
P01–P03 subtotal are unchanged. GI#569 and GI#200 remain open.

Performance: successful acquisition and publication retain the same filesystem
operations. A failed state-record write adds one metadata inspection to identify
a conflicting file kind. Error explanations and manifest-path context are built
on failure; lock validation retains its existing bounds. Compiler/native
interfaces are unchanged. No paired timing improvement is claimed. Website
impact: none, because this remains unreleased alpha.15 work.

### P04a completion

The [finite source audit](../../design/PACKAGE_DIAGNOSTIC_ACCEPTANCE.md) covers
all 24 package modules and their shared input/HTTP error boundaries. This final
correction distinguishes HTTP and redirect causes, provides limit-specific
guidance, identifies filesystem conflicts, fixes unknown-field interpolation and
removes terse internal wording from public failures. The examples now contain
100 exact CLI transcripts. There is no outstanding P04a wording batch.

P04a completes on merge after full local validation and the applicable exact-head
hosted checks pass. P04b and P04c remain planned. P04 is then 50% complete and
GI#200 is 87.5% complete under the unchanged weights. GI#569 and GI#200 remain
open until the installed workflow and final acceptance are delivered.

Performance and website impact, verification evidence and the explicitly deferred
HTTP API detail limitation are recorded in the acceptance document. No compiler,
native-interface or bootstrap source changes are included.
