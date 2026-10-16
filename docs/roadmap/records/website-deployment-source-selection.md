[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Website deployment source selection

<a id="rm-38"></a>

**RM#38: Website source selection** · [GI#123: Website source selection](https://github.com/sproates/panackelty/issues/123).

Work record: [issue #123](https://github.com/sproates/panackelty/issues/123).
State: Done. PR #124 merged after all 23 checks passed. Production Pages run
36723094458 passed build, deployment and live verification; public provenance
reported merge commit `cdff3ef04ca40952934b6f3e5768306ce9474ca5`.
Issue #123 was closed during the subsequent grooming pass.
Selected on 2026-09-30 after alpha.10 release.
An earlier Pages attempt selected historical content and failed on a missing
playground SDK installer; retry succeeded. The original API response was not
retained, so the upstream cause is unproven. The selector's reliance on the first
successful result, without checking current main, is reproducible with stale
or unordered history.

Pin main at selection time and require a trusted successful Check for that exact
commit. Select coverage by Check run number, retaining earlier coverage for
documentation-only changes. Missing validation, API errors and missing/expired
coverage stop publication and preserve the live site. This deliberately waits
when current main is pending or failed, rather than publishing an older commit.

Small–medium, one PR including regression tests and documentation. Acceptance:
focused source-selection tests, canonical `make check`, hosted Pages validation
and, after separately approved merge, live source-provenance verification.
This observed delivery failure takes priority over build-cache measurement and
TCP/timer discovery; no compiler, website appearance or cache changes are needed.
