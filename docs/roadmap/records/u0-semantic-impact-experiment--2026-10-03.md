[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## U0 semantic-impact experiment — 2026-10-03

<a id="rm-106"></a>

**RM#106: Semantic-impact experiment** — bounded outcome **Done on this PR's
merge**, under [RM#97: Shared programme investigation](current-status-and-grooming-2026-10-01.md#rm-97) and
[GI#175: Semantic change prediction](https://github.com/sproates/panackelty/issues/175).
The user selected one research PR; no separate issue exists for this bounded
outcome. The [reproducible experiment and report](../../../tests/experiments/semantic_impact/README.md)
predict guard and declared-effect consequences from actual baseline compiler
records before applying edits, then validate through changed checking and the
public CLI. It positively substantiates one caller-local Nat obligation and an
ordinary caller's permitted effect boundary. Nested guards, mutation and missing
effect evidence challenge unsupported independence claims.

The conditional two-edit effect cascade is verified; a single leaf edit does not
infer transitive effects or export return-bound guarantees. That negative finding
leaves GI#175's transitive acceptance unmet. The report measures metadata and
latency and recommends bounded declaration-effect dependencies before any public
prediction query. Production U6 is unstarted, broader GI#175/GI#180 remain open,
and further production scope requires separate selection. No post-merge acceptance
for this experiment. Website: no impact because no released behavior, command or
feature claim changes.

Acceptance evidence: 25 focused experiment assertions passed. Clean canonical
`make check` passed in 150s (unit 113s, functional 4s / 343 cases, bootstrap 19s),
including compiler/library fixed points and package/quick-start validation.
The 120s full and 15s unit budgets were exceeded; retain the existing
[GI#106: Validation performance](https://github.com/sproates/panackelty/issues/106)
reminder for these measured overruns rather than relaxing budgets. Independent
review found no blocking findings; its minor architecture wording correction
was included. Final informational completion text is checked with `make docs`.
