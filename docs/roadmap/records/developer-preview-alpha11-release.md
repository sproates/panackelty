[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Developer preview alpha.11 release

<a id="rm-128"></a>

**RM#128: Alpha.11 release** — Done (2026-10-04).
Selected on 2026-10-04. Release delivery owner: maintainer delivering the promotion.
[PR#271: Alpha.11 release preparation](https://github.com/sproates/panackelty/pull/271)
merged as `f1d37b46145b502f7e38e674d0f071934f85166a`. The
[Release workflow](https://github.com/sproates/panackelty/actions/runs/37214140201)
passed validation, Linux/macOS packaging and publication. The
[alpha.11 release](https://github.com/sproates/panackelty/releases/tag/v0.1.0-alpha.11)
was published on 2026-10-04 at 15:51:12 UTC from that exact source commit.

All eight public assets were downloaded. Checksums matched all three archives;
both native provenance records matched the release source. The public macOS
archive passed release smoke testing. This release includes existing native TCP
client/server operations, guard-fact repair, optional source maps, bounded
subtraction/effect explanations, declaration-lookup improvements, measured build
baselines and generated website history. Namespace metadata/binding work remains
internal staged infrastructure, not executable namespace support.

[PR#272: Alpha.11 downloads and installer](https://github.com/sproates/panackelty/pull/272)
merged as `23e20c07397790be0b4b6a450283a15fa57902a7`, promoting native
homepage/README downloads and installer defaults. Independent review, owner
preview approval and public README/installer acceptance on Linux and macOS passed.
[Production Check](https://github.com/sproates/panackelty/actions/runs/37215512240)
and [Pages deployment and live verification](https://github.com/sproates/panackelty/actions/runs/37215665667)
passed for that revision, including byte comparisons of published pages,
playground assets and publication provenance. No dedicated issue.
Browser v0.1.1 remains pinned; RM#102, RM#103 and RM#104 retain their separate
example, documentation and browser-adoption acceptance.

Performance disposition retained through the approved release: retain all safety/coverage
and unchanged 120s full / 15s focused targets. The controlled RM#28 baseline has
clean median 134.42s and unchanged focused median 26.46s, above those targets.
The performance delivery owner retains GI#106: inspect the measured critical path
and evaluate a bounded reuse candidate before the next affected compiler delivery.
The release carries this known development-latency limitation; it makes no runtime
performance adequacy claim and does not reopen deferred runtime benchmarks.
