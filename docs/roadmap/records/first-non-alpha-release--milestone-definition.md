[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## First non-alpha release — milestone definition

<a id="rm-126"></a>

**RM#126: First non-alpha release** ·
[GI#265: First non-alpha release](https://github.com/sproates/panackelty/issues/265).

User decision, 2026-10-04: this is an overarching release-readiness milestone
bringing programmes and workstreams together, not another programme. Version,
date and final scope remain undecided; further alpha releases may continue.
The following minimum outcomes are required, with detailed acceptance still to
be defined where stated. Open scope prevents a credible overall completion
percentage; do not double-count contributing programmes or call planning delivery.

| Required outcome | Existing work and remaining definition |
| --- | --- |
| Compiler work | GI#180 is included in Alpha.14 (RM#151); its release-specific workstreams, scope and acceptance criteria remain undefined. Define a bounded scope before selecting implementation. |
| Namespaces, packages and HTTP | Complete GI#233, including migration, reusable packages, HTTP client/server, reproducibility and integrated acceptance. |
| Performance of a good standard | GI#263 supplies maintained measurements and explicit regression decisions. Agree representative compile/startup/run, resource and developer-feedback targets from repeatable evidence; no unmeasured performance claim. |
| Expanded standard library | Define practical application coverage, consistent APIs, tests and examples. This is broader than GI#235's namespace migration; detailed scope is open. |
| Expanded tooling and developer helpers | Define supported creation/edit/build/test/inspect/debug/maintenance workflows; reuse GI#139 editor work and relevant existing tool proposals. The full tooling/helper set remains open. |
| Code tidying | GI#132 / RM#62 component cleanup, maintainability and removal of obsolete transitional code with behavior protection. |
| Documentation | Coherent installation, learning path, language/library references, tooling guides, worked applications and migration notes; reuse GI#137/GI#138 and verify documented workflows. |
| Website outside the main repository | GI#178 / RM#14: source ownership/extraction and independent development/publication. The existing browser repository alone does not meet this requirement. |
| Faster local and collaboration workflow | GI#133/GI#106 cover developer feedback and build costs. Explicitly assess task-tracking/edit/review/publication latency too: elapsed time, active work versus waiting, repeated tool/review/approval overhead and resource/token use where observable. Administrative tracking overhead is a remaining scope gap, not a claimed existing acceptance criterion or measured cause. |
| Release readiness | Proposed supporting gates: reliable installation, packaging/upgrade, supported-platform evidence, compatibility/migration and support expectations; refine these with the owner. |
| Further requirements | Maintain an explicit open-scope list. Additional release requirements may be agreed; omission is not acceptance or a waiver. |

Release acceptance requires agreed bounded criteria for each area, evidence-linked
gate status and an independently verified clean-install walkthrough building a
useful multi-file application from public documentation. Resolve open scope before
choosing the release version; record limitations and accepted trade-offs. Individual
PR checks alone do not establish this milestone. Track gates as scope-to-define,
planned, in progress or accepted rather than inventing a total percentage.

Immediate workflow response: batch related bookkeeping into existing delivery or
planning PRs, use the established informational-document validation route, reuse
verified evidence and keep review proportionate to changed scope. Avoid repeated
status-only polling and duplicate approval rounds. This records the excessive
task-tracking delay as a concern; it does not claim the bottlenecks are measured or
fixed. No runtime/build measurements are required for this informational update.

Namespaces remains the principal feature initiative; this record starts no new
implementation or release. Ordinary website content updates remain deferred until
a supporting release, while repository extraction must be complete by this
milestone. Detailed decisions and future gate evidence belong in GI#265 and this
register, reusing existing child work rather than duplicating it.
