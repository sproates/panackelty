# Panackelty VM-to-host ABI

This document freezes the host-service contract implemented by every
Panackelty VM. Bytecode version 8 identifies a host service by the UTF-8 name in
a `CALL` instruction. The verifier rejects unknown names, wrong arities, and
calls from pure functions to effectful services.

All arguments and results use the tagged values defined in
[`../bytecode/FORMAT.md`](../bytecode/FORMAT.md). Every call produces one result;
operations declared `Void` produce the internal `Void` sentinel. Host failures
become structured results where specified, otherwise a Panackelty `I/O error`
or `VM trap`, never a host-language exception.

## Stable services

| Service | Signature | Contract |
| --- | --- | --- |
| `print` | `(Any) -> Void` | Write the canonical display form and one newline to standard output. |
| `eprint` | `(Any) -> Void` | Write the canonical string form and one newline to standard error. |
| `read_line` | `() -> Str` | Read one line from standard input, excluding its line terminator. |
| `read_file` | `(Str) -> Str` | Read the named file completely and require valid UTF-8. |
| `write_file` | `(Str, Str) -> Void` | Replace or create the named file with UTF-8 text. |
| `read_bytes` | `(Str) -> Bytes` | Read the named file completely without decoding. |
| `write_bytes` | `(Str, Bytes) -> Void` | Replace or create the named file with the supplied bytes. |
| `command_args` | `() -> [Str]` | Return an immutable snapshot of arguments after the program path and command separator. |
| `environment_has` | `(Str) -> Bool` | Report whether a key existed in the environment snapshot taken when the VM started. |
| `environment_get` | `(Str) -> Str` | Return a value from that snapshot, or trap when the key was absent. |
| `process_exit` | `(Nat) -> Void` | Stop the complete invocation with the supplied process status. A host may reject statuses outside its supported range. |
| `path_resolve` | `(Str) -> Str` | Resolve a path against the process working directory and normalize it. |
| `file_exists` | `(Str) -> Bool` | Report whether the resolved path names a regular file. |
| `run_bytecode` | `(Bytes) -> Void` | Decode, bound, and verify version-8 bytecode, then run it with the same argument and environment snapshots. |
| `run_bytecode_args` | `(Bytes, [Str]) -> Void` | Decode, bound, and verify version-8 bytecode, then run it with the supplied argument snapshot and the current environment snapshot. |

All services in this table are effectful. The environment and argument arrays
are snapshots so nested VMs and repeated reads see stable input. File writes
replace their target; partial-write recovery and atomic replacement are not
promised by the ABI. Paths and environment strings are Panackelty `Str` values,
so hosts must reject values they cannot represent instead of silently changing
them. In particular, every path service rejects an embedded NUL with a VM trap
before consulting the filesystem. Missing or denied paths, missing parent
directories, invalid UTF-8 text files, and host read/write failures produce an
`I/O error`; diagnostics may add platform-specific context.

## VM primitives outside the host ABI

Exact numerics, persistent arrays/maps/sets, byte buffers, UTF-8 conversion,
text inspection, and lexical path transforms execute inside the VM. Their
source-level names and semantics are specified in [`../../SPEC.md`](../../SPEC.md),
but they do not call the operating system. This distinction lets the portable
standard library use deterministic primitives while keeping all external
authority in the small service table above.

The pure lexical path primitives are `path_parent`, `path_join`, `path_suffix`,
`path_with_suffix`, and `path_is_absolute`. `path_join` follows the frozen
Panackelty behavior of normalizing the joined path; it does not query whether
the result exists.

## Compatibility

Changing a service name, arity, purity, tagged argument/result type, observable
output, snapshot rule, or failure behavior is an ABI change. Additive services
may be introduced without changing the bytecode container version, but all
compiler stages and VM implementations must adopt them together. Incompatible
changes require a new bytecode version or an explicit ABI-version mechanism.

## Rational and Unit operations (bytecode 8)

| Name | Contract | Failure |
| --- | --- | --- |
| `$unit` | `() -> Unit`, pure; source `()` lowers to this internal name | None |
| `nat` | `(Rat) -> Nat`, pure, exact non-negative integer conversion | Wrong runtime type, fractional or negative value |
| `dec` | `(Rat) -> Dec`, pure, exact finite decimal conversion | Wrong runtime type or non-terminating decimal |
| `quotient` | `(Nat, Nat) -> Nat`, pure, truncating quotient | Wrong runtime types or zero divisor |

The native verifier checks arities and purity; runtime checks remain mandatory
for forged bytecode. `Rat` arithmetic is implemented by the existing `BINARY`
instruction with normalized arbitrary-precision values.

## Monotonic clock service

`instant_now(): Result[Instant,ClockError]` is effectful and reads
`CLOCK_MONOTONIC`. Host read failure returns `Error(ClockUnavailable())`, rather
than trapping. Clock suspension behavior follows the host; there is no portable
suspend-time guarantee. Instants share one domain per execution and cannot be
serialized or constructed from source integers. Arithmetic is pure.

All new typed path operations and duration operations are VM primitives, not
host services. Their full signatures and checked errors are specified in
[SPEC.md](../../SPEC.md#paths-and-monotonic-time). Native path bytes are POSIX
bytes on the current target matrix. Existing string-based host path APIs retain
their contracts; typed filesystem access is not introduced by these types.

## Typed filesystem and process services

The signatures, error record, POSIX semantics, ownership, and resource bounds for
`fs_*`, `process_run`, `host_sleep`, and `host_decode_utf8` are specified in
[the language contract](../../SPEC.md#typed-filesystem-process-and-sleep-apis).
All are effectful except checked UTF-8 decoding. They use ordinary version-8
named CALL instructions and record/variant values; old runtimes reject unknown
services. No new value tags or opcode layouts are introduced. Legacy string
file calls retain their trapping behavior for bootstrap compatibility.
