# Panackelty standard library

The standard library is an explicit logical module namespace. There is no
implicit prelude: programs import either the modules they use, such as
`import stdlib/option`, or the complete surface with
`import stdlib/prelude`. Logical names are independent of the source checkout
or installed toolchain layout. Imports currently combine declarations into one
program-wide namespace, so public functions use descriptive prefixes.

| Module | Public API | Implementation |
| --- | --- | --- |
| `option.panack` | `Option[T]`, `None`, `Some` | Portable enum |
| `result.panack` | `Result[T,E]`, `Ok`, `Error` | Portable enum |
| `collections.panack` | Array methods `append`/`concat`/`map`/`reduce`, Map methods `put`/`has`/`get`, Set methods `add`/`has`, plus legacy compatibility spellings | Pure persistent compiler-known operations and VM primitives |
| `text.panack` | `text_length`, `text_slice`, `text_starts_with`, `text_starts_with_at`, `text_reverse`, `text_is_digit`, `text_is_letter`, `text_is_whitespace`, `text_parse_nat` | Portable wrappers over deterministic VM primitives |
| `bytes.panack` | `bytes_empty`, `bytes_push`, `bytes_join`, `bytes_length`, `bytes_at`, `text_encode_utf8`, `text_decode_utf8` | Portable wrappers over immutable byte-buffer primitives |
| `path.panack` | `path_parent`, `path_join`, `path_suffix`, `path_with_suffix`, `path_is_absolute`, `path_resolve`, `file_exists` | Lexical VM primitives plus explicit host queries |
| `environment.panack` | `environment(name): Option[Str]` | Portable checked wrapper over the environment ABI |
| `prelude.panack` | All of the above | Imports the complete library surface |

The raw primitives and functional array methods remain compiler-known because
Panackelty does not yet have generic source functions: collection operations
need checker-supported polymorphism.
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
