[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Self-hosted development toolchain — complete

<a id="rm-82"></a>

**RM#82: Self-hosted development tools**.

The compiler and test probes run on the native VM. Shell harnesses cover build,
repository and release contracts; fixed independent fixtures and bootstrap
identity checks provide additional evidence. See [the testing guide](../../../tests/README.md)
for suite ownership and commands.

`make policy` enforces the source-tree dependency boundary.
`make check-no-interpreter` starts from a clean build and validates unit and
functional checks, bootstrap, conformance, packaging and release smoke with an
allowlisted `PATH`. Both supported platforms run this proof across five clean
CI partitions, sharing canonical test targets.
