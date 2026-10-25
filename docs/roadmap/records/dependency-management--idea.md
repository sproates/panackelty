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
| P03 | [Pinned remote sources](https://github.com/sproates/panackelty/issues/568) | P01–P02 | Done on merge of PR#582 | 35% | 100% | 35 pp |
| P04 | [`panack` workflow integration](https://github.com/sproates/panackelty/issues/569) | P01–P03 | Planned | 25% | 0% | 0 pp |
| **Total on merge of PR#582** |  |  |  | **100%** | **75%** | **75 pp** |

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
Planned; 25% of GI#200. Completes installation, help, diagnostics and end-to-end
acceptance across all sources. Ordinary project commands never restore implicitly.

### P04 delivery slices

The owner selected this delivery breakdown on 25 October 2026. P04 remains 25%
of GI#200; all three slices are planned and earn no completion yet. The provisional
internal allocation is 50% diagnostics, 25% installed workflow and 25% final
acceptance, reflecting the larger diagnostic implementation and test surface.
These are scope weights, not PR counts. Overall programme completion stays 75%.

| Slice | Deliverable | Status | Share of P04 | Programme weight | Complete | Earned | Estimated PRs |
| --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| P04a: Diagnostics | Preserve causes and dependency context; explain failures and safe recovery; test public messages and document real examples | Planned | 50% | 12.5% | 0% | 0 pp | 1–2 |
| P04b: Installed workflow | Verify both tools and native dependencies are packaged correctly; finish command help and installed-tool behaviour | Planned | 25% | 6.25% | 0% | 0 pp | 1 |
| P04c: Final acceptance | Exercise all four source types together, relocation, standalone commands and network-free ordinary builds on supported platforms | Planned | 25% | 6.25% | 0% | 0 pp | 1 |
| **P04 total** | | **Planned** | **100%** | **25%** | **0%** | **0 pp** | **3–4** |

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
