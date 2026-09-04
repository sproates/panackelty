# Panackelty release policy

Panackelty `0.1.0-alpha.1` is a developer preview. It is intended for learning,
experimentation, feedback, and non-critical terminal programs. It is not yet
recommended for production systems or irreplaceable data.

## Supported systems

The initial preview targets:

- Ubuntu 22.04 or newer on x86-64
- macOS 14 or newer on Apple silicon (arm64)

Each supported release archive must pass the exact-artifact smoke test on its
target system. Other POSIX-like systems may work from source but are best effort.
Windows and other processor architectures are not supported by the initial
preview.

Downloaded toolchains require only the operating system, a terminal, `tar`, and
the platform's standard SHA-256 utility for download verification. Python,
`make`, a C compiler, and a source checkout are not runtime or compilation
dependencies.

## Release gate

A release tag must exactly match `v` followed by the version in `VERSION`. The
tag workflow first runs the complete development suite, then independently
builds and smoke-tests the Linux x86-64 and macOS arm64 archives without Python.
Only after both matrix jobs succeed does the final job download their retained
archives, verify their SHA-256 checksums and provenance, and publish the tag as
a GitHub prerelease. Only that final job has repository write permission.

## Compatibility during the preview

Panackelty uses semantic versions with prerelease identifiers. While the project
is in alpha:

- Source syntax, type-checking behavior, and standard-library APIs may change
  between preview releases. Changes will be described in `CHANGELOG.md`.
- The command names `check`, `compile`, `run`, and `disasm` form the preview CLI
  surface. Incompatible CLI changes require release notes and a new preview
  version.
- Bytecode is an exchange format within one toolchain release, not a durable
  distribution format. The VM currently accepts bytecode version 7 only, and
  compatibility with bytecode produced by another Panackelty release is not
  promised.
- Patch releases in the same preview series should correct defects without
  deliberately changing accepted source programs.

The language specification remains authoritative for accepted behavior. Items
listed under its deliberately postponed section are not part of the preview
contract.

## Support lifetime

Only the latest developer-preview release receives fixes. A newer preview
supersedes earlier preview artifacts. Security handling is described in
`SECURITY.md`.
