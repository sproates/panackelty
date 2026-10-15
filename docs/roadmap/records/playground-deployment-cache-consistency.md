[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Playground deployment cache consistency

<a id="rm-39"></a>

**RM#39: Playground cache consistency** · [GI#119: Playground cache consistency](https://github.com/sproates/panackelty/issues/119).

Work record: [issue #119](https://github.com/sproates/panackelty/issues/119).
State: Done. PR #120 merged and deployed on 2026-09-30. All 23 hosted checks
passed; live reload loaded versioned assets and the import-free text example
ran successfully. Selected on 2026-09-30 after the browser kept
an obsolete imported text example following the core-method deployment.

Version the entire playground asset set together and test normal reload from a
warm HTTP cache across deployments. Acceptance includes deterministic identities,
new example/library execution in all three browser projects, publishing checks,
canonical validation and live verification after separately approved merge.
Cached entry HTML and already open tabs are not automatically refreshed; missing
old assets must fail visibly rather than silently mixing versions.

Small, one implementation PR. This observed onboarding defect takes priority over
the separate alpha.10 release, dependency-aware probe reuse and TCP/timer discovery.
No language, bytecode or downloadable release-version change is included.
