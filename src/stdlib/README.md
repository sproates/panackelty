# Panackelty standard library

Every source program receives the core `Option[T]` and `Result[T,E]` types,
their constructors, and standard text and collection methods. Other library
APIs are explicit modules. Import a module with a short alias and call a concise
name within that domain:

```panackelty
import stdlib/path as path
import stdlib/bytes as byte_buffer
import stdlib/text as text

main(): Void {
  location: Result[Path,path.PathError] = path.from_text("notes.txt")
  encoded: Bytes = text.encode_utf8("hello")
  print(byte_buffer.length(encoded))
}
```

## Curated prelude

`stdlib/prelude` re-exports a curated set of modules while preserving each
module boundary:

| Prelude module | Source module |
| --- | --- |
| `prelude.option` | `stdlib/option` |
| `prelude.result` | `stdlib/result` |
| `prelude.byte_buffer` | `stdlib/bytes` |
| `prelude.text` | `stdlib/text` |
| `prelude.path` | `stdlib/path` |
| `prelude.environment` | `stdlib/environment` |
| `prelude.time` | `stdlib/time` |
| `prelude.filesystem` | `stdlib/filesystem` |
| `prelude.process` | `stdlib/process` |

The byte module is called `byte_buffer` in the prelude so that it does not
shadow the core `bytes()` constructor. Host, numeric, package-filesystem,
testing, and TCP modules remain separate imports.

## Module API

| Module | Public surface |
| --- | --- |
| `option` | `value_or[T]` |
| `result` | `value_or[T,E]` |
| `bytes` | `empty`, `push`, `join`, `length`, `at` |
| `text` | `reversed`, `encode_utf8`, `decode_utf8` |
| `path` | `PathError`, `from_text`, `from_native`, `join`, `to_text`, `native_bytes`, `display`, `is_absolute`, `directory`, `filename`, `current` |
| `environment` | `get` |
| `time` | `Duration`, `Instant`, checked construction, arithmetic, ratios, monotonic clock reads |
| `filesystem` | `FileKind`, `FileMetadata`, `metadata`, `read`, `write`, `list_directory`, `remove_file`, `remove_directory`, `create_directory`, `temp_directory`, `temp_file` |
| `async_filesystem` | async `root_identity` and `read_beneath`; shared root identity/read and host error types |
| `process` | `ProcessOutput`, bounded `run` |
| `host` | `HostError`, nested bytecode `run`, `arguments`, `exit`, `stderr`, `stdout`, `sleep`, checked `decode_utf8` |
| `numeric` | `divide` for truncating natural division |
| `package_filesystem` | `FilesystemRootIdentity`, `FilesystemRootRead`, `directories_overlap`, `read_beneath` |
| `testing` | `TestOutcome`, `TestResult`, `expect`, `equal_str`, `equal_nat`, `report` |
| `testing_files` | `TestWorkspace`, `workspace_create`, `workspace_remove_empty`, `discover_fixtures` |
| `testing_commands` | `TestCommand`, `command_output`, `command_result`, `harness_command`, `capture_report`, `run_command`, `expect_command_error` |
| `tcp` | `TcpServerLimits`, EOF-delimited and bounded streaming native TCP servers |

The function signatures and checked error behavior are documented in the
[language specification](../../SPEC.md). `HostError` is defined by
`stdlib/host`; `filesystem` and `process` re-export the same type identity.
`FileKind` and `FileMetadata` belong to `filesystem`, while
`ProcessOutput` belongs to `process`.

The short source wrappers keep operations grouped by domain. Older
`path_string_*` and trapping `filesystem_*` wrappers are not exported by the
stdlib modules or the prelude. Compiler-only Str path and file adapters are in
`src/compiler` while the compiler's source paths remain strings.

## Core operations and generic helpers

Persistent array, map, and set operations and compiler-known conversions keep
their core signatures and VM lowering. Array `map` and `reduce` retain their
compiler lowering. The pure generic `option.value_or[T]` and
`result.value_or[T,E]` functions are ordinary Panackelty source; for example,
write `result.value_or[Nat,Str](Ok(42), 0)` when the argument does not provide
enough type evidence.

The concise collection operations are method-only and type-directed, so they
do not reserve `put`, `has`, `get`, or `add` as global source functions.
Text and array methods are available without the prelude.

The conformance program is compiled and executed by the audited seed and fresh
self-hosted compiler stages. Byte-identical artifacts and frozen reference
outputs check the module graph at each stage.

## Sorting and suffix matching

Use `values.sort_by(@less)` without imports, where
`less: PureFn[T,T,Bool]` and `values: [T]`. It returns a stable sorted array
without changing its input. The comparator must define a consistent strict weak
ordering and be pure. Equivalent elements retain input order; comparator traps
propagate. Merge sort uses O(n log n) comparisons and O(log n) recursive depth,
building temporary immutable arrays.

Use `value.ends_with(suffix): Bool` without imports. It matches a literal,
case-sensitive suffix at Unicode code-point boundaries. An empty suffix matches
every value; a longer suffix never matches. There is no normalization, locale
collation, or wildcard interpretation.

See the executable [collections example](../../examples/collections_and_bytes.panack).

## Typed paths and time

Import `stdlib/path` and `stdlib/time`, or use the prelude modules.
Path constructors return `Result`; operations preserve lexical spelling and
do not access the filesystem. A `Path` does not imply existence, canonical
identity, permissions, or containment.

`time.seconds(5)` creates an exact duration.
`time.add_instant(start, time.seconds(5))` creates a deadline, and
`time.difference(end, start)` measures elapsed time. Only `time.now()` reads
the monotonic clock. See the executable
[host types case](../../tests/functional/cases/host_types/main.panack) and the
[path and timing contract](../../SPEC.md#paths-and-monotonic-time).

## Filesystem, process, and host operations

Import `stdlib/filesystem` for typed byte I/O, metadata, sorted directory
enumeration, and explicitly owned temporary resources. Import
`stdlib/process` for bounded subprocess execution. These APIs use `Path`,
`Duration`, and structured `Result` errors. They do not provide recursive
deletion, atomic writes, implicit PATH search, or a shell.

Import `stdlib/host` for process arguments, exit, terminal output, nested
bytecode execution, checked UTF-8 decoding, and sleep. The pure portable
`text.decode_utf8` operation traps on invalid input; `host.decode_utf8`
returns a checked result.

See the [host capability case](../../tests/functional/cases/host_capabilities/main.panack)
and the [complete API contract](../../SPEC.md#typed-filesystem-process-and-sleep-apis).

## Testing modules

Import `stdlib/testing` for pure assertions. `testing.expect`,
`testing.equal_str`, and `testing.equal_nat` return a `TestResult` containing
a `TestPassed` or `TestFailed(reason)` outcome. Assertions do not terminate a
program. `testing.report` prints results in caller order and returns the
failure count; a caller chooses the exit policy.

Import `stdlib/testing_files` to discover immediate directory children in
unsigned native-byte order or to create and explicitly remove an owned empty
temporary workspace. Import `stdlib/testing_commands` for structured,
byte-exact subprocess assertions. No testing helper provides process
containment or automatic fixture cleanup.

See the [assertion case](../../tests/functional/cases/testing_library/main.panack),
[fixture case](../../tests/functional/cases/testing_fixtures/main.panack), and
[command case](../../tests/functional/cases/testing_commands/main.panack).

`stdlib/tcp` remains a separate native module. See the
[TCP server contract](../../SPEC.md#native-tcp-server-development-toolchain);
raw TCP is unavailable in the browser.
