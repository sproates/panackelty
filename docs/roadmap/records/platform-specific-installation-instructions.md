[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Platform-specific installation instructions

<a id="rm-117"></a>

**RM#117: Platform-specific installation instructions** ·
[GI#249: Platform-specific installation instructions](https://github.com/sproates/panackelty/issues/249).
**Done on merge of PR#251: Platform-specific installation instructions.**
The README now gives separate Linux x86_64 and macOS arm64 alpha.10 download,
checksum, extraction and version blocks that stop on failures, followed by direct
local check/run/compile commands and optional home-directory PATH setup. The
published Linux archive passed the exact README commands on Ubuntu 24.04 x86_64;
all six injected download/checksum failures stopped before extraction.
[Installation verification](../../../tests/VALIDATION_PROFILE.md#platform-installation-instructions--2026-10-03)
records evidence and limitations. The existing two-platform packaging CI now
executes the current README against real published assets. Both native hosted
platforms passed those exact commands in [the acceptance run](https://github.com/sproates/panackelty/actions/runs/37160525736);
independent source review also passed. Final-head checks and merge authorization
remain prerequisites to merging. No local macOS execution is claimed. This is an
installation documentation change, not a packaging,
installer or new-platform change. Website parity remains separately unscheduled
under [RM#118: Website installation command parity](independent-website-publishing.md#rm-118); release pins remain
unchanged.
