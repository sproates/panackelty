[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Reusable modules and package structure — idea

<a id="rm-43"></a>

**RM#43: Reusable packages** · [GI#199: Reusable packages](https://github.com/sproates/panackelty/issues/199).

Work record: [GI#199: Reusable packages](https://github.com/sproates/panackelty/issues/199).
Current state: Done on merge of PR#386, the final P3 acceptance delivery; the completion
summary takes effect when its exact-head checks pass and it merges.
Confined package-root reads merged in PR#372; explicit manifest-root application
`check`/`compile`/`run`, dependency-free package loading and relocation-stable
identity merged in PR#374; exported-library checking in PR#375; direct local
library dependencies and two-app acceptance in PR#376; physical-root identity,
source-root overlap rejection, nested graph resolution, conflicts and exact graph
limits in PR#377–#382; and source-count/byte boundaries in PR#383. PR#384 widens
the root-replacement race fixture and waits for an in-flight replacer before
cleanup. The final acceptance fixture gives two independent applications one
shared multi-file library, checks/runs/compiles both from an unrelated working
directory, moves their complete sibling workspace, then rechecks/runs both and
executes bytecode compiled before the move. It demonstrates relocation without
copying the library sources and
closes the scoped local-package acceptance when PR#386 merges. P8 retains
the separate clean-installed release gate; this does not close the wider P1–P8
modules programme.
Website impact for this test-only slice: none; it adds internal regression
coverage and does not change shipped behavior or user-facing documentation.
The [P1 shared design](../../design/MODULE_PACKAGE_DESIGN.md) supersedes the historical
assessment-only scope below; P3 acceptance is completed by the final delivery
above, while P8's clean-installed release gate remains separate.

Define how related files form a reusable library with a deliberate public API,
and how another application consumes it. Distinguish files, source modules,
namespaces, packages and applications. Assess package roots, manifests, exports,
internal modules, tests, resources, documentation and toolchain compatibility.
Compare source distribution with compiled artifacts without assuming a stable ABI
or requiring #106's separate compilation. This concerns language libraries, not
OS packaging of the Panackelty toolchain.

Smallest outcome: a reviewed design and bounded local-package prototype plan,
using one multi-file library from two applications and a relocated installation.
Define round-trip, hidden-internals and malformed/path-escape acceptance cases.
Coordinate [cross-file references](language-namespaces--idea.md#language-namespaces--idea) and
[dependency management](dependency-management--idea.md#dependency-management--idea). Assessment is provisionally
medium / one design PR; implementation and ongoing maintenance need re-estimation.
Public API compatibility and premature artifact-format commitments are key risks.
No registry, separate compiler/linker or package format is selected.
