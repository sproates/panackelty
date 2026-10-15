[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Potential technical publications

Record publication candidates when there is a concrete thesis and supporting
implementation or evaluation evidence. Each candidate needs a contribution to
assess, links to evidence and prior work, and a revisit trigger. This is an
unscheduled backlog, not a publication programme or a commitment to publish
every idea. The change-impact research candidate below may contribute to the
compiler paper. The separate [type-inference research candidates](language-direction-and-differentiation--exploration.md#type-inference-research-candidates)
may support future papers if their contributions are established; no separate
paper is promised for any candidate.

### Compiler and runtime understanding paper — idea

<a id="rm-58"></a>

**RM#58: Compiler understanding paper** · [GI#211: Compiler understanding paper](https://github.com/sproates/panackelty/issues/211).

Work record: [#211](https://github.com/sproates/panackelty/issues/211).
Agreed for the backlog on 2026-10-02; assessment is unscheduled. Revisit after
programme #180 U8 realistic-program evaluation. This is separate from #180 and
does not block programme acceptance or its final U9 website refresh.

Assess whether the implemented architecture and measured results justify a
technical white paper, experience report or research submission. Compare with
relevant prior work and identify the contribution supported by evidence;
original research novelty is not established. A useful account of engineering
and integration does not require inventing every underlying concept.

The smallest outcome is a recorded assessment and publication recommendation,
including a reason and revisit trigger if deferred. If justified and selected,
draft the paper around architecture, reproducible realistic examples, exact
revisions and commands, measurements, independent evaluation, limitations and
related work. Review claims and reproduce results before an explicitly agreed
publication; recording this candidate does not start writing or submission.
Effort and PR count remain unknown until the evidence and format are assessed.

During programme delivery, retain design rationale, counterexamples, unsuccessful
approaches and performance/usefulness evidence in existing programme records.
Reuse those records rather than introducing a competing progress tracker.
Risks are overstated novelty, unsupported generalisation and distraction from
delivery; do not expand compiler scope merely to obtain a paper.

### Independently checkable change-impact explanations — research idea

<a id="rm-59"></a>

**RM#59: Checkable change impact** · [GI#213: Checkable change impact](https://github.com/sproates/panackelty/issues/213).

Work record: [#213](https://github.com/sproates/panackelty/issues/213).
Agreed for the backlog on 2026-10-02: a bounded research assessment and prototype,
unscheduled. Link findings to paper candidate #211 and coordinate with programme
#180 explanations/provenance and semantic prediction (#134/#173/#175). This is
not an extra mandatory workstream or programme completion requirement.

Investigate whether a small separate verifier can check evidence explaining
which selected guarantees survive a code change, at practical cost. First compare
with proof-carrying code, incremental/differential verification, summary repair
and explanation research; identify a precise contribution or record why further
work is not justified. Novelty is a hypothesis, not an established claim.

If the assessment supports a prototype, begin with guarded arithmetic and
mutation, including cross-module obligations. Define certificates bound to exact
source/dependency revisions, supported reasoning rules and the trusted computing
base. Check reasoning independently of the explanation producer; source hashes
alone establish no semantic guarantee. Distinguish proved preservation, loss of
an existing proof, an exhibited violation and unknown. Loss of proof is not proof
of a bug, and preserved obligations do not establish whole-program equivalence.

Agree scope, representative cases and budgets before evaluation. Test false,
tampered, stale and mismatched evidence as well as valid and unsupported cases.
Compare ordinary diagnostics and dependency-based impact reporting; measure
correctness, useful precision, certificate size, verification cost and developers'
ability to identify a valid repair. Retain failures, limitations and reproducible
results; finish with independent review and a proceed/defer recommendation.

Assessment can precede programme completion; select prototype timing when source
identity and checker evidence interfaces are available. Effort and PR count are
unknown until assessment. Production integration or programme expansion needs a
separate decision. Risks include overlap with prior work, verifier complexity,
unsound non-impact claims, overhead and distraction from agreed delivery.
