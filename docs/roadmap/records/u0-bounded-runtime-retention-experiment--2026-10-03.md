[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## U0 bounded runtime-retention experiment — 2026-10-03

<a id="rm-2"></a>

**RM#2: Runtime retention experiment**.

Selected by the user after the U3 local-effect delivery: investigate one actual
arithmetic/function-call value derivation, repeated call/loop identity, bounded
retention, explicit missing/evicted/unsupported evidence, CPU/memory costs and
sensitive-value policy. Medium / one experiment PR under
[#172](https://github.com/sproates/panackelty/issues/172) and
[#180](https://github.com/sproates/panackelty/issues/180). This completed experiment
preceded [RM#106: Semantic-impact experiment](u0-semantic-impact-experiment--2026-10-03.md#rm-106); broader production U7 is
not selected.

**Bounded outcome Done on this PR's merge:** the test-only observer follows the
actual VM's `multiply(6, 7)` return to its arithmetic/operand/call origins and
validates the actual PC against production source attribution. Repeated calls,
loops and recursion have distinct occurrence IDs. Ring and prefix policies
retain fixed memory and report evicted/discarded ancestry; metadata limits and
unsupported collection/host/async/indirect boundaries give explicit unavailable
roots without inventing a derivation. Payloads are opt-in fixed-width Nat/Bool;
metadata itself is not confidential. Chronological branch context is explicitly
not a proven control-dependence graph.

[Experiment, acceptance and measurements](../../../tests/experiments/runtime_provenance/README.md):
570,010 events in the scalar loop, 238,648 bytes total retained metadata at
1,024 slots (including all shadow state), approximately 2.21× bulk CPU and
1.97× single-step CPU for ring observation in the recorded five-run sample.
A retained final root can still have evicted ancestry. Equal observed process
RSS does not establish zero allocation overhead. Recommend a separately scoped
U7 scalar slice, large / 2–3 PRs, with disabled-by-default hooks, validated
source/session identity, explicit queries and gaps, privacy controls and new
representative performance acceptance. Do not ship the experiment as a feature.

This finishes only the bounded U0 retention question. U7/#172 and all five
programme workstreams remain open; other U0 investigations, including positive
non-impact evidence, remain. No post-merge execution acceptance is required for
this experiment. Acceptance: 65 focused real-VM/CLI assertions, native retention
unit checks, and clean canonical `make check` pass (151s total, unit 114s,
functional 4s, all 343 functional cases, bootstrap and package gates). The
120s full and 15s unit budgets remain explicit #106 reminders. Independent
review found and verified a fix for unsafe observer PC inspection on a verified
fallthrough path; scalar and unsupported-frontier regressions now pass. No
remaining correctness findings. No release or website feature claim changes.
