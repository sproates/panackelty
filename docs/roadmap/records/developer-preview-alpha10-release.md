[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Developer preview alpha.10 release

<a id="rm-35"></a>

**RM#35: Alpha.10 release**.

State: Done. PR #122 merged and alpha.10 published on 2026-09-30 from
`8cb6b75328aae8f6febf02529e9ac1798c7056b4`. Linux x86-64 and macOS arm64
release gates passed. Both public downloads passed checksum/provenance checks;
the downloaded macOS archive passed quick-start and release smoke tests. Linux
archive execution was verified by the release matrix. The website deployed
successfully with alpha.10 download instructions and migration notes.

The release aligns downloads with the playground's core language APIs and v9
bytecode. Real networking and broader build-cache work remain separate.
