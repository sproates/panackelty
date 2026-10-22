# Panackelty standard library

Every source program receives the core `Option[T]` and `Result[T,E]` types,
their constructors, and standard text and collection methods. Other library
APIs are explicit modules. Import a module with a short alias and call a concise
name within that domain:

```panackelty
import stdlib/path as path

main(): Void {
  location: Result[Path,path.PathError] = path.from_text("notes.txt")
  encoded: Bytes = "hello".encode_utf8()
  print(encoded.length())
}
```

## Curated prelude

`stdlib/prelude` re-exports a curated set of modules while preserving each
module boundary:

| Prelude module | Source module |
| --- | --- |
| `prelude.byte_buffer` | `stdlib/bytes` |
| `prelude.path` | `stdlib/path` |
| `prelude.environment` | `stdlib/environment` |
| `prelude.time` | `stdlib/time` |
| `prelude.filesystem` | `stdlib/filesystem` |
| `prelude.process` | `stdlib/process` |

The byte module is called `byte_buffer` in the prelude so that it does not
shadow the core `bytes()` constructor. Host, package-filesystem,
testing, and TCP modules remain separate imports.

## Module API

| Module | Public surface |
| --- | --- |
| `bytes` | `empty`; receiver operations belong to Bytes |
| `path` | `PathError`, `from_text`, `from_native`, `current`; receiver operations belong to Path |
| `environment` | `get` |
| `time` | `Duration`, `Instant`, checked construction and monotonic clock reads; arithmetic and ratios are owned methods |
| `filesystem` | `FileKind`, `FileMetadata`, `metadata`, `read`, `write`, `list_directory`, `remove_file`, `remove_directory`, `create_directory`, `temp_directory`, `temp_file` |
| `async_host` | async invocation arguments, bounded stdout/stderr, and exit |
| `tcp_service` | continuous native service limits, readiness, summary and configured serving |
| `async_filesystem` | async `root_identity` and `read_beneath`; shared root identity/read and host error types |
| `process` | `ProcessOutput`, bounded `run` |
| `host` | `HostError`, nested bytecode `run`, `arguments`, `exit`, `stderr`, `stdout`, `sleep`, checked `decode_utf8` |
| `package_filesystem` | `FilesystemRootIdentity`, `FilesystemRootRead`, `directories_overlap`, `read_beneath` |
| `package_archive` | `PackageArchiveInspection`, `inspect_local_archive`, `restore_local_archive` |
| `testing` | `TestOutcome`, `TestResult`, `expect`, `equal_str`, `equal_nat`, `report` |
| `testing_files` | `TestWorkspace`, `workspace_create`, `discover_fixtures`; TestWorkspace owns `remove_empty` |
| `testing_commands` | `TestCommand`, `command_output`, `command_result`, `harness_command`, `capture_report`, `run_command`, `expect_command_error` |
| `http_transport` | async raw `exchange` |
| `testing_async` | deterministic async `read` |
| `compiler_files` / `compiler_paths` | compiler file and lexical-path services |
| `tcp` | `TcpServerLimits`, EOF-delimited and bounded streaming native TCP servers |

The function signatures and checked error behavior are documented in the
[language specification](../../docs/reference/SPEC.md). `HostError` is defined by
`stdlib/host`; `filesystem` and `process` re-export the same type identity.
`FileKind` and `FileMetadata` belong to `filesystem`, while
`ProcessOutput` belongs to `process`.

P12 removes the temporary receiver-function exports listed in the
[P10 migration report](../../docs/design/type-owned-dispatch/p10-stdlib-callers.md).
The wrapper-only text, numeric, option and result modules are removed.
New code uses owned methods for receiver operations; constructors and services
remain module functions. Older
`path_string_*` and trapping `filesystem_*` wrappers are not exported by the
stdlib modules or the prelude. Compiler Str path and file adapters use the explicit
`stdlib/compiler_paths` and `stdlib/compiler_files` services while the compiler's source paths remain strings.

## Core operations and generic helpers

Persistent array, map, and set operations and compiler-known conversions keep
their VM lowering. Use owned calls such as `values.map(@transform)` and
`answer.value_or(0)`. If a generic owner cannot be inferred, give the receiver
a type, for example `answer: Result[Nat,Str] = Ok(42)`, then call its method.
The method itself does not take the owner's type arguments.

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
`start.add(time.seconds(5))` creates a deadline, and
`end.difference(start)` measures elapsed time. Only `time.now()` reads
the monotonic clock. See the executable
[host types case](../../tests/functional/cases/host_types/main.panack) and the
[path and timing contract](../../docs/reference/SPEC.md#paths-and-monotonic-time).

## Filesystem, process, and host operations

Import `stdlib/filesystem` for typed byte I/O, metadata, sorted directory
enumeration, and explicitly owned temporary resources. Import
`stdlib/process` for bounded subprocess execution. These APIs use `Path`,
`Duration`, and structured `Result` errors. They do not provide recursive
deletion, atomic writes, implicit PATH search, or a shell.

Import `stdlib/host` for process arguments, exit, terminal output, nested
bytecode execution, checked UTF-8 decoding, and sleep. The pure portable
`buffer.decode_utf8()` operation traps on invalid input; `host.decode_utf8`
returns a checked result.

See the [host capability case](../../tests/functional/cases/host_capabilities/main.panack)
and the [complete API contract](../../docs/reference/SPEC.md#typed-filesystem-process-and-sleep-apis).

## Testing modules

Import `stdlib/testing` for pure assertions. `testing.expect`,
`testing.equal_str`, and `testing.equal_nat` return a `TestResult` containing
a `TestPassed` or `TestFailed(reason)` outcome. Assertions do not terminate a
program. `testing.report` prints results in caller order and returns the
failure count; a caller chooses the exit policy.

Import `stdlib/testing_files` to discover immediate directory children in
unsigned native-byte order or to create and explicitly remove an owned empty
temporary workspace. Call `workspace.remove_empty()` to remove it; a nonempty
workspace returns the existing host error. Import `stdlib/testing_commands` for structured,
byte-exact subprocess assertions. No testing helper provides process
containment or automatic fixture cleanup.

See the [assertion case](../../tests/functional/cases/testing_library/main.panack),
[fixture case](../../tests/functional/cases/testing_fixtures/main.panack), and
[command case](../../tests/functional/cases/testing_commands/main.panack).

`stdlib/tcp` remains a separate native module. See the
[TCP server contract](../../docs/reference/SPEC.md#native-tcp-server-development-toolchain);
raw TCP is unavailable in the browser.

## Owned source methods on next

Core now owns Str.ends_with, Array.first/sort_by, Option.value_or and
Result.value_or, Path's existing receiver operations, and Duration's existing
receiver arithmetic/comparisons. P14 uses owned dots,
such as `items.first().value_or(fallback)` and `base.join(child)`. Library bodies and their callers use dots.
PathError and DurationError are defined once in core and re-exported from path
and time, including their variants. Receiver wrapper functions are removed. Path/time factories and clock
services remain module functions; ClockError stays in time.

P10 migrates library bodies and conformance callers to owned dispatch.
`TestWorkspace.remove_empty` is defined in its owning testing_files module.
Its old module function is removed. Compiler and remaining callers use owned
methods after P11; P12 removes their compatibility implementation.
