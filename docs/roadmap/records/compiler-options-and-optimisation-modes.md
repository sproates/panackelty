[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Compiler options and optimisation modes

<a id="rm-4"></a>

**RM#4: Compiler options** · [GI#220: Compiler options](https://github.com/sproates/panackelty/issues/220).

**Idea; unscheduled:** [#220](https://github.com/sproates/panackelty/issues/220),
requested during the 2026-10-02 performance discussion. Revisit compile-time speed,
generated-program optimisation and optional diagnostic/explanation support as
separate goals. Compare coherent profiles and orthogonal flags, with measured
trade-offs, explicit defaults, compatibility/cache/source-attribution rules and
preserved type/proof checking, exact semantics and runtime safety. Ordinary builds
already omit explanation evidence and instruction-source mappings; do not assume
another flag removes current costs. An `-O2`-style interface is a proposal, not a
selected design. Medium / 1–2 investigation PRs for an options matrix, reproducible
evidence and a separately estimated implementation recommendation. Relates to
#106, #141, #134 and #173; does not replace current performance work or expand #180.
