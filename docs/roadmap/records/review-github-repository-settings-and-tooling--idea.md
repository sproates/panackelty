[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Review GitHub repository settings and tooling — idea

<a id="rm-45"></a>

**RM#45: Repository tooling review**.

Assess whether repository configuration and available tooling can improve
security, code quality and contributor experience. This is a backlog item, not
an assertion that any particular feature is disabled or suitable. Include it in
the holistic assessment and compare its value and effort with other candidates.

- [ ] Inventory actual settings, workflows and enabled checks; distinguish
      unavailable, disabled, already configured and redundant capabilities
- [ ] Evaluate applicable code/security scanning, secret scanning and push
      protection, dependency alerts/updates/review (including Actions), and
      static-analysis or code-quality tools; verify language/ecosystem support
      and avoid implying that a C scanner analyses custom `.panack` semantics
- [ ] Review branch/ruleset protections, required checks, workflow/token
      permissions, environment/release protections and contribution settings
      against the project's agreed workflow
- [ ] Record benefit, findings/actionability, false positives, setup and ongoing
      effort, CI latency, cost/plan availability, access needs and ownership
- [ ] Recommend a minimal useful set with a clear enable/retain/defer rationale,
      acceptance checks and an alert-triage process; obtain explicit permission
      before changing settings, protections, permissions or paid services

Acceptance for the review is an evidence-backed inventory and prioritised
recommendations. Enabling selected features is separately scoped work with
its own verification; do not turn on every available feature by default.

<a id="measure-panackelty-source-coverage--candidate-pending-assessment"></a>
