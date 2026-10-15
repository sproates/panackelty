[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## U3 per-function effect recovery — 2026-10-03

<a id="rm-107"></a>

**RM#107: Per-function effect recovery** — bounded outcome **Done on this PR's
merge**, following [RM#106: Semantic-impact experiment](u0-semantic-impact-experiment--2026-10-03.md#rm-106). The user selected
one medium feature PR: body type errors no longer suppress trustworthy local
effect explanations for separately type-valid functions. The checker retains
per-function validity and a conservative global gate for loading, resolution,
type errors in signatures and type declarations. Invalid functions explicitly remain
unavailable; no expression-level or transitive recovery is introduced.

Recovery is explanation-only to preserve ordinary acceptance, rejection, ordered
diagnostics and emitted artifacts. Local recovered violations appear in evidence;
original type diagnostics remain on stderr. Imported/generic source origins and
allowed versus whole-program-rejected status remain explicit. Unit and public-CLI
regressions cover unsafe callable/await/iterable inference, final return mismatch,
late global errors, diagnostic parity and fresh evidence after repair. The retained
semantic-impact experiment now positively checks the recovered caller boundary;
its original measurements remain labelled historical.

Acceptance: 266 focused unit assertions, 53 public-CLI assertions and all 343
functional cases pass. Clean canonical `make check` passed in **154s** (unit 116s,
functional 5s, bootstrap 19s), including fresh compiler/library fixed points and
package/quick-start checks. All 227 compiler-contract fixtures preserve exact
baseline diagnostics/status; all 43 accepted artifacts remain identical. The
updated semantic-impact experiment passes 25 assertions. The full 120s and unit
15s budgets remain exceeded; retain the existing
[GI#106: Validation performance](https://github.com/sproates/panackelty/issues/106)
reminder rather than relaxing them. See the
[validation profile](../../../tests/VALIDATION_PROFILE.md#u3-per-function-effect-recovery--2026-10-03).

This completes only the bounded recovery feature; no separate issue exists.
[GI#134: Compiler explanations](https://github.com/sproates/panackelty/issues/134)
and [GI#180: Understandable-programming programme](https://github.com/sproates/panackelty/issues/180)
remain open. No post-merge feature acceptance is required. Supporting native
release and website adoption remain [RM#104: Explanation website adoption](independent-website-publishing.md#rm-104);
this PR starts no website implementation or promotion.
