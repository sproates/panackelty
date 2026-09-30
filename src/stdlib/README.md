# Panackelty standard library

Every source program automatically receives the core types `Option[T]` and
`Result[T,E]`, their constructors, and standard text/collection methods.
Other APIs require explicit imports, or `import stdlib/prelude` for the combined
surface. `stdlib/option` and `stdlib/result` contain value-or helpers only.
Logical names are independent of checkout and installed layouts. Ordinary
imports still combine declarations into one program-wide namespace.

| Module | Public API | Implementation |
| --- | --- | --- |
| `core.panack` | Implicit Option/Result types and constructors; text and array methods | Source-defined types and private method algorithms, loaded once |
| `option.panack` | `option_value_or[T]` | Explicit generic helper |
| `result.panack` | `result_value_or[T,E]` | Explicit generic helper |
| `testing.panack` | `TestOutcome`, `TestResult`, `test_expect`, `test_equal_str`, `test_equal_nat`, `test_report` | Pure assertions and explicit ordered reporting; imported separately from the prelude |
| `testing_commands.panack` | `TestCommand`, `test_run_command`, `test_expect_command_error`, `test_command_output`, `test_command_result` | Bounded command execution with byte-exact output and structured host-error assertions; imported separately from the prelude |
| `testing_files.panack` | `TestWorkspace`, `test_workspace_create`, `test_workspace_remove_empty`, `test_discover_fixtures` | Effectful sorted fixture discovery and explicitly owned temporary isolation; imported separately from the prelude |
| `bytes.panack` | `bytes_empty`, `bytes_push`, `bytes_join`, `bytes_length`, `bytes_at`, `text_encode_utf8`, `text_decode_utf8` | Portable wrappers over immutable byte-buffer primitives |
| `time.panack` | `Duration`, `Instant`, checked construction, arithmetic, ratios, monotonic clock reads | Opaque VM values, clock ABI, and portable helpers |
| `path.panack` | `Path`, `PathError`, checked construction and lexical operations; compatibility APIs: `path_parent`, `path_join`, `path_suffix`, `path_with_suffix`, `path_is_absolute`, `path_resolve`, `file_exists` | Lexical VM primitives plus explicit host queries |
| `environment.panack` | `environment(name): Option[Str]` | Portable checked wrapper over the environment ABI |
| `prelude.panack` | All of the above | Imports the complete library surface |

The portable generic helpers are ordinary Panackelty source. The raw storage
primitives and functional array methods retain their compiler-known signatures
and lowering; introducing source generics does not replace their VM operations.
For an empty array, use `[].first[Str]()` or a typed array binding. For
`result_value_or`, both success and error types must be known from the argument
or supplied explicitly, for example `result_value_or[Nat,Str](Ok(42), 0)`.
They are still distinct from operating-system intrinsics. Pure primitives are
implemented inside each VM; only the services listed in
[`../runtime/ABI.md`](../runtime/ABI.md) cross the host boundary.

The concise collection operations are method-only and type-directed, so they
do not reserve `put`, `has`, `get`, or `add` in the global source namespace.
Existing compiler-known storage primitives remain available; the removed
stdlib text/collection wrappers have no compatibility aliases.

The library conformance program is compiled by the audited seed and fresh
self-hosted stages. Tests require byte-identical artifacts, compare against
frozen independent bytes and execute the result, so every compiler stage
checks this module graph.

The loader internalises the three `core_*` algorithm names before parsing the
canonical core module. These are not callable source globals. The emitter keeps
only core algorithms reachable from ordinary functions, including recursive
dependencies; ordinary user functions are never pruned. Removed `text_*`,
`array_first` and `array_sort_by` wrappers have no compatibility aliases. The
byte-buffer `text_encode_utf8` and `text_decode_utf8` helpers remain explicit.

## Sorting and suffix matching

Use `values.sort_by(@less)` without imports, where
`less: PureFn[T,T,Bool]` and `values: [T]`.
It returns a stable sorted array without changing its input. The comparator
must return true only when the first element belongs strictly before the second,
with a consistent strict weak ordering. Equivalent elements retain their input
order. Empty and singleton arrays require no comparisons. Comparators must be
pure; their ordering laws are the caller's responsibility and are not checked.
Comparator traps propagate. Merge sort uses O(n log n) comparisons and O(log n)
recursive depth; it builds temporary immutable arrays rather than sorting in place.

Use `value.ends_with(suffix): Bool` without imports.
This pure helper matches a literal, case-sensitive suffix. An empty suffix
matches every value, including empty text; a longer suffix returns false.
Matching respects Unicode code-point boundaries, without normalization,
locale rules or glob expansion. A combining code point can itself be a suffix.

Both methods are available without the prelude. See the executable
[collections example](../../examples/collections_and_bytes.panack) for sorting
filenames with a named pure comparator and filtering by suffix.

## Typed paths and time

Import `stdlib/path` and `stdlib/time`, or use the complete prelude.
[The language specification](../../SPEC.md#paths-and-monotonic-time) lists every
function and checked error. Constructors use `Result`; raw values have no public
fields. `duration_seconds(5)` creates an exact duration,
`instant_add(start, duration_seconds(5))` creates a deadline, and
`instant_difference(end, start)` measures elapsed time. Only `instant_now()`
reads the clock and is effectful. See the executable
[conformance program](../../tests/functional/cases/host_types/main.panack).

`host.panack` defines the shared `HostError` record. `filesystem.panack` defines
`FileKind` and `FileMetadata`; `process.panack` defines `ProcessOutput`. The
prelude imports all three. Their VM services support typed byte I/O, sorted
directory enumeration, explicit temporary-resource ownership, bounded process
execution, checked decoding, and sleep; see
[the complete API contract](../../SPEC.md#typed-filesystem-process-and-sleep-apis).

## Testing assertions and reports

Import `stdlib/testing` explicitly. `test_expect(name, condition, reason)`,
`test_equal_str(name, actual, expected)`, and
`test_equal_nat(name, actual, expected)` return a `TestResult` containing a
`TestPassed` or `TestFailed(reason)` outcome. Assertions are pure and do not
terminate a program. `test_report(results)` prints `PASS name` or
`FAIL name: reason` for each result in the supplied order, then prints
`tests: N, failures: M` and returns the failure count. An empty list reports
zero tests and zero failures. A caller can choose a nonzero exit status from
the returned count.

See the [functional case](../../tests/functional/cases/testing_library/main.panack)
for a complete program.

Import `stdlib/testing_files` to discover immediate **directory** children
of a root with `test_discover_fixtures(root)`. It returns their full `Path` values
in unsigned native-byte order and excludes regular files and final symlinks.
If enumeration or metadata fails, it returns a `HostError` without partial
results. The root is a caller-supplied path; it may itself resolve through a
symlink. Discovery observes the filesystem as it changes, not a snapshot.

`test_workspace_create(parent)` atomically creates a unique temporary directory
and returns `TestWorkspace { root }`. The caller owns its contents and must
remove them, then call `test_workspace_remove_empty(workspace)`. A nonempty
workspace returns `not_empty` and is left intact for inspection. There is no
automatic or recursive cleanup, and `TestWorkspace` is an ordinary constructible
record, not a containment or authorization boundary. See the
[fixture case](../../tests/functional/cases/testing_fixtures/main.panack).

Import `stdlib/testing_commands` for process assertions. `TestCommand` carries
an exact executable `Path`, arguments, input bytes, working directory,
environment overrides, timeout, and combined output limit; no PATH search or
implicit shell is added. `test_run_command(name, command, expected)` returns
a `TestResult` comparing the completed process's exit code, signal, stdout,
and stderr. Output comparison is byte-exact; the first mismatch is reported
without rendering potentially binary streams. A host failure is a failed test
with its portable error category. `test_expect_command_error` instead asserts
an expected host-error category such as `launch_failed` or `output_limit`;
a completed process is not a host error, even when it exits nonzero.
`test_command_output` and `test_command_result` are pure helpers for already
captured results. None of these helpers provides process containment or
automatic fixture cleanup. See the
[command case](../../tests/functional/cases/testing_commands/main.panack).

Direct host, runtime and standard-library assertions run in
`tests/runner/host_runtime_unit.panack` with reviewed source and malformed
bytecode fixtures in `tests/fixtures/host_runtime`. The probe asserts native
process, file, path, environment and timing contracts and exact testing-library
reports. Direct C host checks and forced failures run under instrumentation.
The [migration inventory](../../tests/fixtures/host_runtime/README.md) maps all 37
former methods: 31 migrated to direct native evidence and the final six replaced
by fixed oracle fixtures and native/bootstrap cross-checks. Functional source and bytecode cases still verify
public behaviour on both supported platforms.


`import stdlib/tcp` supplies the ordinary `TcpServerLimits` record for finite
native servers. The compiler builtin `tcp_serve` checks its named async handler
and returns ordered per-client outcomes. See the [TCP server contract](../../SPEC.md#native-tcp-server-development-toolchain)
for field order, bounds, EOF framing and shutdown. Raw TCP is unavailable in the
browser and absent from alpha.10 downloads.
