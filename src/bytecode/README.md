# Panackelty bytecode

Panackelty bytecode is the contract between the compiler and VM. Version 7 begins
with the `PANACKBC` magic header and a two-byte version, followed by a compact,
typed binary payload containing the function table, purity metadata, and
instruction streams. `main` is the implicit entry point.

The loader verifies structure, operands, jump targets, callees, arities, and
purity before returning executable code. It also enforces the versioned resource
limits in [FORMAT.md](FORMAT.md). The loader rejects legacy versions, truncation,
trailing bytes, invalid UTF-8, unknown codes, non-canonical numeric encodings,
and reserved flags before semantic verification.

The frozen execution contract and exact version-7 byte layout, including every
instruction's stack effect and encoded operand, are specified in
[FORMAT.md](FORMAT.md).

Portable canonical and malformed-artifact vectors live in
[`tests/fixtures/bytecode`](../../tests/fixtures/bytecode). They are stored as
plain hexadecimal text so the Python oracle, native VM, and self-hosted tooling
can all validate the exact same artifact bytes.

## Canonical ordering and deterministic artifacts

Within bytecode version 7, serialization is canonical:

- functions are ordered by ascending Unicode function name, independent of the
  insertion order of the in-memory function table;
- every function-table key must exactly equal the function name stored in its
  `Code` record;
- parameters, instructions, record fields, variant payloads, and interpolation
  parts retain their source-defined order;
- compiler-generated temporary names are numbered from zero independently in
  each function and allocated in deterministic AST traversal order;
- opcodes, operator codes, constant tags, and count widths are fixed;
- text is length-prefixed UTF-8, integers use minimal big-endian magnitudes, and
  decimals use their exact sign, coefficient digits, and exponent.

Consequently, compiling identical source and dependency inputs with the same
bytecode version produces byte-identical artifacts. Loading and reserializing a
canonical artifact also preserves every byte. Unit tests enforce repeated-build,
function-order, and load/reserialize identity.

The loader currently accepts version 7 only; backwards compatibility remains
deliberately postponed. Any semantic or encoding change requires a bytecode
version increment.

## Self-hosted implementation

[`codec.panack`](codec.panack) contains the Panackelty-hosted version-7
serializer. It consumes the typed IR from `src/compiler/emitter.panack`, sorts
functions canonically, and emits the same bytes as the bootstrap serializer.
[`decoder.panack`](decoder.panack) performs bounded binary reads, strict UTF-8
and numeric validation, structural and semantic verification, disassembly, and
canonical reserialization. It consumes the same portable malformed vectors as
the bootstrap loader. Differential tests compare complete artifacts byte for
byte.

This directory contains only the portable format contract and Panackelty-hosted
implementation. Temporary Python tooling is confined to `src/bootstrap`.
