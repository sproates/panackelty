[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Dependency management — idea

<a id="rm-44"></a>

**RM#44: Dependency management** · [GI#200: Dependency management](https://github.com/sproates/panackelty/issues/200).

Work record: [GI#200: Dependency management](https://github.com/sproates/panackelty/issues/200).
Current Alpha.15 state: **design discussion in progress; no document or
implementation started**. The outcome remains a detailed package-management
design, produced only after the owner says discussion and refinement are ready
to record. Implementation scope and acceptance remain a later owner decision.

The immutable-Git, lockfile, cache/vendor, integrity and offline requirements in
the P1 shared design were recorded in PR #240 but are not confirmed owner
requirements. They are historical design input, not the current P7 specification.
Current local package support uses explicitly declared relative paths to source
packages; absolute paths are rejected. Remote Panack source packages, their
distribution, and version selection remain under discussion.

The design discussion will define how application authors declare Panack source
packages and how developers obtain and select versions. It will cover both
`panack` commands and direct edits to `panack.json`. Compare local paths, remote
URLs, published source archives, manually downloaded archives and any other
relevant distribution choices without assuming Git or a registry. Define how
authors publish versions, how importers select them, and whether locking,
integrity checks, caching and offline use belong in the initial product scope.

Current Alpha.15 outcome: an owner-reviewed detailed design document. It is not an
implementation authorization. Before drafting the document, discuss and refine
the product model, expected workflows, versioning and security assumptions with
the owner. The design must distinguish Panack source-package dependencies from
native libraries such as libcurl, the compiler/toolchain and bytecode artifacts.
Implementation scope, effort and acceptance remain for a later owner decision;
the former implementation estimate is not a current estimate for the design-only
Alpha.15 outcome.

These linked investigations informed P1 but do not settle P7's product scope.
Their original implementation assumptions remain historical context; current
P7 discussion and design scope is recorded above for Alpha.15.
