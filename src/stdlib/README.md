# Panackelty standard library

The standard library is an explicit logical module namespace. There is no
implicit prelude: programs import either the modules they use, such as
`import stdlib/option`, or the complete surface with
`import stdlib/prelude`. Logical names are independent of the source checkout
or installed toolchain layout. Imports currently combine declarations into one
program-wide namespace, so public functions use descriptive prefixes.

| Module | Public API | Implementation |
| --- | --- | --- |
| `option.panack` | `Option[T]`, `None`, `Some`, `option_value_or[T]` | Portable enum and generic helper |
| `result.panack` | `Result[T,E]`, `Ok`, `Error`, `result_value_or[T,E]` | Portable enum and generic helper |
| `collections.panack` | `array_first[T]` returning `Option[T]`; array methods `append`/`concat`/`map`/`reduce`, Map methods `put`/`has`/`get`, Set methods `add`/`has`, plus legacy compatibility spellings | Portable generic helper plus compiler-known operations and VM primitives |
| `text.panack` | `text_length`, `text_slice`, `text_starts_with`, `text_starts_with_at`, `text_reverse`, `text_is_digit`, `text_is_letter`, `text_is_whitespace`, `text_parse_nat` | Portable wrappers over deterministic VM primitives |
| `bytes.panack` | `bytes_empty`, `bytes_push`, `bytes_join`, `bytes_length`, `bytes_at`, `text_encode_utf8`, `text_decode_utf8` | Portable wrappers over immutable byte-buffer primitives |
| `time.panack` | `Duration`, `Instant`, checked construction, arithmetic, ratios, monotonic clock reads | Opaque VM values, clock ABI, and portable helpers |
| `path.panack` | `Path`, `PathError`, checked construction and lexical operations; compatibility APIs: `path_parent`, `path_join`, `path_suffix`, `path_with_suffix`, `path_is_absolute`, `path_resolve`, `file_exists` | Lexical VM primitives plus explicit host queries |
| `environment.panack` | `environment(name): Option[Str]` | Portable checked wrapper over the environment ABI |
| `prelude.panack` | All of the above | Imports the complete library surface |

The portable generic helpers are ordinary Panackelty source. The raw storage
primitives and functional array methods retain their compiler-known signatures
and lowering; introducing source generics does not replace their VM operations.
For an empty array, use `array_first[Str]([])` or a typed array binding. For
`result_value_or`, both success and error types must be known from the argument
or supplied explicitly, for example `result_value_or[Nat,Str](Ok(42), 0)`.
They are still distinct from operating-system intrinsics. Pure primitives are
implemented inside each VM; only the services listed in
[`../runtime/ABI.md`](../runtime/ABI.md) cross the host boundary.

The concise collection operations are method-only and type-directed, so they
do not reserve `put`, `has`, `get`, or `add` in the global source namespace.
Legacy prefixed free functions remain available for bootstrap compatibility.

The library conformance program is compiled once by the bootstrap compiler and
again by the complete Panackelty-hosted compiler. Tests require byte-identical
artifacts and execute the result, so every currently available compiler stage
checks this module graph.

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
