[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Performance priority review — 2026-10-02

Historical bounded investigation; ongoing obligations now belong to
[RM#123: Standing performance engineering](standing-performance-engineering--ongoing.md#rm-123).

**Agreed with the user after PR #219:** take a bounded validation-performance
investigation under [#106](https://github.com/sproates/panackelty/issues/106)
before further compiler features. Determine whether recent compiler changes
caused the observed 378s full check (290s unit, 71s bootstrap), separating compiler
implementation cost, growing compiler/test inputs, test execution and repeated
build work. Compare historical seeds on identical inputs with the same VM and
host; profile clean and incremental validation before choosing an optimisation.
Preserve every test, proof rule, diagnostic and bootstrap/release gate. No claim
of reaching the 120s budget or delivering separate compilation is made in advance.

Selected scope: performance evidence and one justified bounded improvement.
Delivered in one PR; acceptance takes effect on merge. Next: return to programme #180 and
reassess the next U3 type/effect query using the measured cost. U0 runtime-retention
and positive non-impact experiments remain required before their dependent work.
The file-discovery POC #167 and general benchmark suite #141 remain unscheduled.
Website CI #187 stays deferred; this task does not reopen it. The #182 corrective
release/browser adoption obligation remains pending and must be revisited on
publication correctness evidence or release preparation; no release is started.

This review considered U1 (#208), U2 (#209/#214/#218 counted once), and U3's first
query (#219, `f379c50`) as three accepted outcomes. The new baseline is **0 of 3
accepted outcomes** since this review; grooming itself does not add an outcome.
The performance delivery author owns measurements, validation, the PR and tracker
handover. The bounded implementation replaces repeated interpreted declaration
scans with per-kind indexes, preserving declaration order and lookup semantics.
The [validation profile](../../../tests/VALIDATION_PROFILE.md#compiler-lookup-and-validation-cost--2026-10-02)
records fixed-input history, clean/warm phase costs, paired results and limits.
**Bounded outcome Done on this PR's merge:** indexed lookup, regression coverage,
seed reproduction and independent correctness review are complete. Final clean
`make check` passed in **186s**, versus the same-host 366s baseline: unit 138s
(previously 274s), functional 6s, bootstrap 23s (previously 68s). The 120s full
and 15s incremental/unit budgets remain open #106 concerns; warm compiler checks
measured 33s before / 32s after, so no material warm-path improvement is claimed.
No tests were removed.
Broader #106 cache/design work remains open. No website correction or release promotion is required for this internal
speedup; emitted user-program bytes and advertised features remain unchanged.
The earlier #182 corrective release/adoption obligation is still separate. Further PR merges still require explicit user approval.
