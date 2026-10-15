[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Historical grooming gaps and decisions

These earlier findings are retained as supporting context, not a fresh audit.
The broader
[2026-09-29 assessment](../../history/ADOPTION_ASSESSMENT.md) now covers application
architecture and adoption as well as testing. Neither report authorises every
feature or settles implementation priorities.

| Gap or ambiguity | Evidence / consequence | Treatment |
| --- | --- | --- |
| Developer experience and compiler assistance | Existing ideas emphasise diagnostics; useful assistance throughout development needs deliberate assessment | Treat enjoyment and productivity as strategic goals alongside strong testing |
| Roadmap documentation and process | Stale status and overlapping documents made the actual priorities unclear | Review document roles, acceptance criteria and the process for maintaining current evidence |
| Source coverage and its denominator | C coverage says nothing about which `.panack` lines or branches execute; omitted files could inflate future reports | Evaluate coverage scope, effort and enabling value in the holistic assessment |
| Test evidence has drifted | Matrix/backlog references include retired test paths, “both VMs”, a compiler “skeleton”, and already-implemented deterministic round trips | Audit current assertions against the matrix before declaring missing tests; retain historical fixture provenance |
| Execution coverage versus test quality | A hit does not establish that a test would catch the wrong result | Keep an explicit assertion-quality backlog; evaluate bounded mutation or deliberately perturbed fixtures for selected high-risk behavior |
| Runtime source diagnostics | Frontend positions exist but emitted instruction records lack source maps | Define a separate follow-up for source-aware runtime traps/call stacks; reuse coverage metadata only where semantics and validation agree |
| Product direction and feature selection | The specification names Euler programs as its proving ground, while the roadmap explores dependable automation and many unrelated differentiators | Agree representative user programs and a next preview outcome before promoting experiments; reconcile product wording in a later scoped change |
| Milestone exit criteria | Initial preview delivery is explicit; the next capability milestone is not | Define observable user outcomes, supported scope, compatibility implications and release evidence, rather than promising a date or a bundle of speculative features |

Existing proposals already cover JSON, generic/inference extensions, host APIs,
error quality, browser targets and contribution criteria. These are prioritisation
choices, not newly discovered omissions. Missing package management, concurrency,
module visibility and compatibility guarantees are explicitly deferred in the
specification; do not promote them merely because other languages have them.
