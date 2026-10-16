[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Standard I/O, streaming and logging

<a id="rm-3"></a>

**RM#3: Streams and logging** · [GI#222: Streams and logging](https://github.com/sproates/panackelty/issues/222).

**Idea; awaiting planning, implementation unscheduled:**
[#222](https://github.com/sproates/panackelty/issues/222), requested 2026-10-03.
Existing `read_line`, `print` and `eprint` cover basic stdin/stdout/stderr;
whole-file text/binary I/O and subprocess capture also exist. Assess the gaps
and discoverability before proposing replacements. Plan coherent line/chunk and
text/byte stream APIs, EOF/error contracts, buffering/flush, pipelines and host
support, plus logging levels, filtering, sinks and structured context. Preserve
existing APIs and effect checks; keep machine-readable stdout separate from logs.
The first outcome is a capability matrix, examples, API/lifecycle recommendation
and prioritised implementation slices, not a delivered logging framework.
Medium / provisionally 1–2 investigation/design PRs; implementation estimated
after assessment. Coordinate with subprocess POC #168 and build-tool POC #169.
This records future planning without changing programme #180 priority. No release or website impact from this record.
