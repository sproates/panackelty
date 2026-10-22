[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Dependency management — idea

<a id="rm-44"></a>

**RM#44: Dependency management** · [GI#200: Dependency management](https://github.com/sproates/panackelty/issues/200).

Work record: [GI#200: Panack package management](https://github.com/sproates/panackelty/issues/200).
On 22 October 2026, the owner extended Alpha.15 from design discussion to the
full implementation of the selected package-management model. GI#200 remains
open as the umbrella until implementation and final acceptance are complete.
The detailed lock and restore contract is the first packet; implementation is
then sequenced through local dependencies, remote acquisition and `panack`
integration. The detailed decisions and packet issues are linked below.

The owner selected four dependency sources:

- Local directories, relative or absolute. They remain at their declared paths.
- Local `.tar.gz` archives.
- Git repository URLs, optionally with a path inside the repository.
- Remote `.tar.gz` archive URLs.

Local archives, Git sources and remote archives are materialized under
`.panack_modules/<dependency-name>`. The lockfile pins each dependency's source
identity and exact remote content. A checkout's `.panack_modules` directory is
a materialization cache and is reusable only when its recorded identity matches
the lock entry. The lockfile does not record whether a checkout has already
unpacked a dependency. Absolute local-directory entries are machine-specific.
The `panack` command layer obtains and prepares sources; the compiler receives
resolved source paths and performs no network access. Archive extraction must
stay within the dependency destination.

The lockfile format and precise update/offline behavior are not yet settled.
That contract is the first Alpha.15 packet. Registries, package publication,
bytecode distribution, native library management and dependency build scripts
are outside this scope.

## Alpha.15 implementation plan

<a id="alpha15-implementation-plan"></a>

The packets have a dependency order because later work relies on a stable lock
and restore contract. Their weights are provisional estimates of relative scope,
including implementation, tests, integration and documentation. They account
for all of GI#200's implementation scope and will remain fixed unless new
evidence justifies a recorded re-estimate.

| Packet | Work | Dependencies | Status | Weight | Complete | Earned |
| --- | --- | --- | --- | ---: | ---: | ---: |
| P01 | [Lock and restore contract](https://github.com/sproates/panackelty/issues/566) | — | Planned | 15% | 0% | 0 pp |
| P02 | [Locked local dependencies](https://github.com/sproates/panackelty/issues/567) | P01 | Planned | 25% | 0% | 0 pp |
| P03 | [Pinned remote sources](https://github.com/sproates/panackelty/issues/568) | P01–P02 | Planned | 35% | 0% | 0 pp |
| P04 | [`panack` workflow integration](https://github.com/sproates/panackelty/issues/569) | P01–P03 | Planned | 25% | 0% | 0 pp |
| **Total** |  |  |  | **100%** | **0%** | **0 pp** |

The compiler resolves imports against paths supplied by `panack`; it does not
fetch or restore dependencies. Direct edits to `panack.json` remain supported,
alongside dependency-management commands. Acceptance must cover each source form,
lock reproducibility, cache mismatch and recovery, offline failure behavior,
path-safe extraction, useful diagnostics, and complete Panack programs run via
`panack`. The detailed packet criteria are in the linked issues.

## Packet records

<a id="p01"></a>
<a id="rm-158"></a>

**P01 · RM#158: Lock and restore contract** · [GI#566](https://github.com/sproates/panackelty/issues/566).
Planned; 15% of GI#200. Defines the implementation contract and acceptance.

<a id="p02"></a>
<a id="rm-159"></a>

**P02 · RM#159: Locked local dependencies** · [GI#567](https://github.com/sproates/panackelty/issues/567).
Planned; 25% of GI#200. Implements directory and local-archive sources.

<a id="p03"></a>
<a id="rm-160"></a>

**P03 · RM#160: Pinned remote sources** · [GI#568](https://github.com/sproates/panackelty/issues/568).
Planned; 35% of GI#200. Implements Git and remote-archive acquisition.

<a id="p04"></a>
<a id="rm-161"></a>

**P04 · RM#161: `panack` workflow integration** · [GI#569](https://github.com/sproates/panackelty/issues/569).
Planned; 25% of GI#200. Integrates restore, CLI operations and end-to-end
acceptance.

The earlier design-only Alpha.15 direction and unresolved product questions
were superseded by the owner's 22 October decision. The historic P1 design in
PR#240 remains background material only where it conflicts with the selected
scope above.

[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)
