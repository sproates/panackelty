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

Local archives, Git sources and remote archives are materialized under
`.panack_modules/<package-name>`. This is a per-checkout cache and is reused only
when its recorded identity matches the lock. Local directories are not
materialized. The lock records source identity and exact remote content; it does
not record whether a checkout has already unpacked a dependency. Absolute local
directory entries are machine-specific. The `panack` command layer obtains and
prepares sources, while the compiler receives resolved paths and does no network
access. Archive extraction must stay within its dependency destination.

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
| P01 | [Lock and restore contract](https://github.com/sproates/panackelty/issues/566) | — | In progress | 15% | 80% | 12 pp |
| P02 | [Locked local dependencies](https://github.com/sproates/panackelty/issues/567) | P01 | Planned | 25% | 0% | 0 pp |
| P03 | [Pinned remote sources](https://github.com/sproates/panackelty/issues/568) | P01–P02 | Planned | 35% | 0% | 0 pp |
| P04 | [`panack` workflow integration](https://github.com/sproates/panackelty/issues/569) | P01–P03 | Planned | 25% | 0% | 0 pp |
| **Total** |  |  |  | **100%** | **12%** | **12 pp** |

The [P01 contract](../../design/PACKAGE_MANAGEMENT_CONTRACT.md) is drafted and
validated, pending owner review; its remaining acceptance is not yet complete.
It defines the
lockfile and manifest forms, update and restore behavior, cache verification,
offline operation, path-safe archive handling, diagnostics and the test evidence
needed by P02–P04. P01 specifies the implementation; it does not count as
dependency acquisition or package-management implementation. GI#200 completion
requires the full graph and `panack` acceptance in P04.

## Packet records

<a id="p01"></a>
<a id="rm-158"></a>

**P01 · RM#158: Lock and restore contract** · [GI#566](https://github.com/sproates/panackelty/issues/566).
In progress; 15% of GI#200. Defines the implementation contract and acceptance.

<a id="p02"></a>
<a id="rm-159"></a>

**P02 · RM#159: Locked local dependencies** · [GI#567](https://github.com/sproates/panackelty/issues/567).
Planned; 25% of GI#200. Implements local directory and archive sources.

<a id="p03"></a>
<a id="rm-160"></a>

**P03 · RM#160: Pinned remote sources** · [GI#568](https://github.com/sproates/panackelty/issues/568).
Planned; 35% of GI#200. Implements Git and remote archive acquisition.

<a id="p04"></a>
<a id="rm-161"></a>

**P04 · RM#161: `panack` workflow integration** · [GI#569](https://github.com/sproates/panackelty/issues/569).
Planned; 25% of GI#200. Integrates restore, CLI operations and end-to-end
acceptance.

The previous design-only Alpha.15 direction and its unresolved product questions
were superseded by the owner's 22 October decision. PR#240's historic P7 design
remains context only where it does not conflict with the current contract.

[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)
