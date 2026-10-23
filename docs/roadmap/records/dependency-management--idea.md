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
| P02 | [Locked local dependencies](https://github.com/sproates/panackelty/issues/567) | P01 | In progress | 25% | 0% | 0 pp |
| P03 | [Pinned remote sources](https://github.com/sproates/panackelty/issues/568) | P01–P02 | Planned | 35% | 0% | 0 pp |
| P04 | [`panack` workflow integration](https://github.com/sproates/panackelty/issues/569) | P01–P03 | Planned | 25% | 0% | 0 pp |
| **Total** |  |  |  | **100%** | **15%** | **15 pp** |

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
In progress; 25% of GI#200, with 0% earned until packet acceptance. P02a is
implementing the independent tool and compiler handoff; P02b implements local
graphs and locks; P02c adds bounded local archives and the complete local command
workflow. The interim implementation includes the separate tool, stricter
compiler handoff, local-directory locks, native staging/gzip/digest adapters and
Panack tar/tree-digest modules. Focused checks pass, but this remains a draft
checkpoint. Archive/cache integration, selective updates, the reproducible seed refresh
and coverage inventories are now implemented. Focused local CLI, transitive
archive, relocation and sanitizer checks pass. Full-suite acceptance and exact-head
CI remain outstanding. No P02 completion
credit is earned by publishing the interim PR.

The owner approved OpenSSL 3+ for generic SHA-256 after the pure-Panack version
took 35.05 seconds for the 64 KiB benchmark. The EVP version took 0.11 seconds
with the same digest. This is a focused measurement, not a claim that package
performance acceptance is complete. Website impact: no published feature or
release claim changes at this draft checkpoint.

<a id="p03"></a>
<a id="rm-160"></a>

**P03 · RM#160: Pinned remote sources** · [GI#568](https://github.com/sproates/panackelty/issues/568).
Planned; 35% of GI#200. Implements Git and remote archive acquisition.

<a id="p04"></a>
<a id="rm-161"></a>

**P04 · RM#161: `panack` workflow integration** · [GI#569](https://github.com/sproates/panackelty/issues/569).
Planned; 25% of GI#200. Completes installation, help, diagnostics and end-to-end
acceptance across all sources. Ordinary project commands never restore implicitly.

The previous design-only Alpha.15 direction and its unresolved product questions
were superseded by the owner's 22 October decision. PR#240's historic P7 design
remains context only where it does not conflict with the current contract.

[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)
