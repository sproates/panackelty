[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## U3 effect explanation slice — 2026-10-03

<a id="rm-1"></a>

**RM#1: Local effect explanations**.

Selected by the user after performance PR #221 and backlog PR #223: extend the
existing explanation command with retained local purity/ordinary/async call and
await decisions, including callable types and imported/generic definitions.
Medium / 1–2 PRs, targeting one end-to-end delivery. This follows the agreed
return to programme #180; #134 remains open for broader type/proof explanations.
U0 retention and non-impact experiments remain required before dependent work.
No new release or website work is started. Owner: delivery author.

**Bounded outcome Done on this PR's merge:** shared effect checking retains actual
local decisions without changing checking rules or user bytecode. Positive,
rejected and unavailable output covers callable types, imported/generic definitions,
nesting and source ranges. Local boundary status is separate from whole-program
validity; no inferred transitive/runtime effects are claimed. Independent
review found no actionable findings. All 196 focused explanation
assertions, 36 CLI assertions and 343 functional cases passed. Canonical clean
`make check` passed in **150s** (unit 112s, functional 5s, bootstrap 19s), with
fresh compiler/library fixed points and package checks. Exact baseline parity
holds for 227 compiler-contract fixtures (43 accepted artifacts, 184 rejected).

The [validation profile](../../../tests/VALIDATION_PROFILE.md#u3-local-effect-explanations--2026-10-03)
records same-input timing and limitations: two-run compiler averages 5.977s before /
6.127s after (+2.5%); small-program compilation and query medians differ by under
2ms. This does not establish zero overhead. The 120s full and 15s incremental/unit
budgets remain explicit #106 reminders; broader U0 retention experiments remain.

This is an intermediate programme slice, not completion of #134 or #180. Broader
type/proof explanations remain. No post-merge acceptance remains for this local
query; release/website adoption is a separate follow-up. Next principal work
returns to programme prioritisation; no U4–U7 implementation is implied.
