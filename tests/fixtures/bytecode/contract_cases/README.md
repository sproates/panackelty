# Bytecode unit contract fixtures

All `.hex` files contain lowercase complete artifact bytes, with whitespace
ignored by the Panackelty test readers. `contract_cases/manifest.json` names
each expanded Python serialization rejection, its original test method, and
its original diagnostic substring. There are 33 exact original artifacts:
18 prefixes and a trailing-byte case from the truncation method, plus malformed
header/version, function records, opcodes and constants. The original method
`test_rejects_every_truncation_and_trailing_data` expands to 18 fixtures.
`contract_cases/codec_manifest.json` retains all nine further structural
call, arity, purity, ordering, UTF-8, flag and constant cases from the
self-hosted decoder tests. Five `limit-*.hex` vectors exercise actual v8
limits for function count, name, text, integer digits and instruction count.
The existing 26 versioned vectors remain unchanged.

`valid_contracts/` contains five reviewed Python-encoded artifact bytes and
canonical disassemblies: all scalar constants (including 100-digit Nat,
negative Int, decimal parts, signed zero with exponent, Unicode, Bool and Void), every instruction and
operand, canonical function order and a loaded reserialization case.
`codec_contracts/` holds nine actual source programs and bootstrap-produced
bytecode goldens (plus the opposite declaration order of the ordering source).
No Python generator runs in the native suite. The native codec tests require
identical byte arrays, instruction listings, reserialization and repeat
compilation. The C public command checks exact stderr/status for each rejected
artifact and exact successful disassembly; both suites run from `make unit`
and `make check-bytecode`. Direct C verifier checks in
`tests/unit/vm/native_modules.c` cover unrepresentable forged native objects,
including missing entry, signatures, empty/missing returns, jumps, empty
interpolation, call targets/arity, purity and the pre-read 16 MiB size bound.

Some diagnostics have implementation-specific wording. The Panackelty probe
requires the self-hosted decoder's original messages. The C command requires
its established exact output, including `invalid function flags`, canonical
ordering for duplicate/out-of-order names, generic unknown-call/arity/purity
messages, and a version rejection without the particular version number.
Bootstrap-specific implementation tests keep their patched low-bound checks
until the stage-0 implementation retires. These exceptions are
named per input; they are never treated as arbitrary rejection.

Python remains for tests of the Python bootstrap's in-memory verifier and
artificially patched limits, the source-build verification hook, stage-0 ADT/minimal-Void self-checks and
CLI hash-seed isolation. Live codec/VM comparisons are retired; fixed native
expectations and per-artifact C return-kind checks replace them. Bytecode wire format, native verifier and direct codec coverage
has a Python-free path; the final seed and harness retirement remains. No Python-only assertion is deleted because
its invalid object has no wire encoding.

## Migrated source-method inventory

| Original method | Exact artifacts | Replacement |
|---|---:|---|
| `test_serialization.py::test_rejects_invalid_magic_and_version` | 3 | Panackelty decoder + C command |
| `test_serialization.py::test_rejects_every_truncation_and_trailing_data` | 18 | Panackelty decoder + C command |
| `test_serialization.py::test_rejects_malformed_function_records_and_opcodes` | 6 | Panackelty decoder + C command |
| `test_serialization.py::test_rejects_noncanonical_and_invalid_constants` | 6 | Panackelty decoder + C command |
| `test_self_hosted_codec.py::test_rejects_structural_call_and_purity_violations` | 9 | Panackelty decoder + C command + fixed expectations |
| `test_vectors.py::test_legacy_vectors_are_identified_and_rejected` | 4 | Shared versioned vectors in both native suites |
| `test_vectors.py::test_malformed_vectors_are_rejected` | 6 | Shared versioned vectors in both native suites |
| `test_serialization.py::test_all_scalar_constants_round_trip_canonically` | 1 | Exact scalar artifact and disassembly, codec round trip |
| `test_serialization.py::test_every_instruction_operand_round_trips` | 1 | Exact operand artifact and disassembly, codec round trip |
| `test_serialization.py::test_serialization_uses_canonical_function_order` | 1 | Opposite-order source inputs, same golden bytes and sorted disassembly |
| `test_serialization.py::test_repeated_compilation_is_byte_identical` | 1 | Repeated Panackelty codec comparison against golden |
| `test_serialization.py::test_load_and_reserialize_is_byte_identical` | 1 | Exact reserialized artifact and disassembly |
| `test_self_hosted_codec.py` serialization, round-trip and disassembly methods | 9 sources + 5 special artifacts | Shared goldens, native exact assertions and fixed independent listings |
