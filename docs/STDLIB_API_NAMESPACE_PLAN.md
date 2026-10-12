# Standard-library API namespace plan

**Status:** migration plan. The naming rules and compatibility boundary below
reflect the owner's choices; no standard-library source API is changed by this
document alone.

## Confirmed direction

- Call functions through their domain module and use short names within that
  module, such as `path.from_text` and `bytes.join`.
- Keep current domain modules mostly flat. Do not split `path`, `filesystem`,
  `time` or `bytes` into additional nested module trees as part of this work.
- Migrate callers, then remove the public `path_string_*` functions and
  Str-based trapping filesystem wrappers.
- Keep `stdlib/prelude` as an explicit curated facade which re-exports module
  namespaces. A caller can use `prelude.path.from_text`; the facade does not
  flatten module functions back into one global list.

## Proposed public names

| Module | Current public names | Proposed names and treatment |
| --- | --- | --- |
| `bytes` | `bytes_empty`, `bytes_push`, `bytes_join`, `bytes_length`, `bytes_at` | `empty`, `push`, `join`, `length`, `at` |
| `text` | `text_reverse`; UTF-8 encode/decode currently live in `bytes` | `reverse`, `encode_utf8`, `decode_utf8`; move UTF-8 functions from `bytes` to `text` |
| `path` | `path_value_from_text`, `path_value_from_native`, `path_value_append`, `path_value_to_text`, `path_value_native_bytes`, `path_value_display`, `path_value_is_absolute`, `path_value_directory`, `path_value_filename`, `path_value_current` | `from_text`, `from_native`, `append`, `to_text`, `native_bytes`, `display`, `is_absolute`, `directory`, `filename`, `current` |
| `filesystem` | `filesystem_metadata`, `filesystem_read_bytes`, `filesystem_write_bytes`, `filesystem_list_directory`, `filesystem_remove_file`, `filesystem_remove_directory`, `filesystem_create_directory`, `filesystem_temp_directory`, `filesystem_temp_file` | `metadata`, `read_bytes`, `write_bytes`, `list_directory`, `remove_file`, `remove_directory`, `create_directory`, `temp_directory`, `temp_file` |
| `environment` | `environment`, `environment_value`, `environment_has_value` | `get` returning `Option[Str]`; remove the empty-string fallback and redundant boolean wrapper after callers migrate |
| `option`, `result` | `option_value_or`, `result_value_or` | `value_or` in each module |
| `numeric` | `nat_quotient` | `quotient` |
| `testing` | `test_expect`, `test_equal_str`, `test_equal_nat`, `test_report` | `expect`, `equal_str`, `equal_nat`, `report` |
| `time` | `duration_value_*`, `instant_value_*`, and existing `duration_*` functions | Remove `_value_` from the public surface; keep `duration_` and `instant_` where they disambiguate the flat module's two value types. Review exact names while migrating to avoid collisions with implicit core operations. |
| `process` | `ProcessOutput`; process execution currently exposed as `host_process_run` in `host` | Keep `ProcessOutput`; expose the operation as `process.run`, moving the wrapper from `host` because process execution belongs to the existing process domain module. |
| `host` | `host_run_bytecode`, `host_command_arguments`, `host_exit`, `host_stderr_text`, `host_stdout_text`, `host_sleep_value`, `host_decode_utf8_value` | `run_bytecode`, `command_arguments`, `exit`, `stderr_text`, `stdout_text`, `sleep`, `decode_utf8` |

`path_string_join`, `path_string_suffix`, `path_string_parent`,
`path_string_is_absolute`, `path_string_with_suffix`, and
`path_string_resolve` are not renamed as public functions. Compiler internals
currently use these Str helpers extensively. During migration, move any still
needed string-path operations behind a compiler-owned support module or convert
those callers to typed `Path` values; do not keep the wrappers publicly
re-exported from `stdlib/path`.

Likewise remove `filesystem_read_text`, `filesystem_exists`,
`filesystem_read_file_bytes`, and `filesystem_write_file_bytes` from the public
filesystem module after callers move to typed `Path`, `Result`-valued APIs.
Callers that need text should explicitly decode bytes and handle an error. A
missing path should be handled as a filesystem result, not converted silently
to a trapping Boolean/string API. This plan does not add a replacement
`read_text` helper without a concrete use and error contract.

## Curated prelude

The first facade should preserve the modules already represented in the current
flat prelude and add `text` because UTF-8 operations move there:
`option`, `result`, `bytes`, `text`, `path`, `environment`, `time`,
`filesystem`, and `process`. The re-exports should be module aliases, for
example `pub import stdlib/path as path`, so a public prelude consumer keeps the
qualifier: `prelude.path.from_text`.

Keep `numeric`, `testing`, `package_filesystem` and low-level `host` as direct
module imports unless an established user-facing use justifies adding them to
the facade. Types stay within their owning module namespace, for example
`prelude.filesystem.FileMetadata` and `prelude.path.PathError`; core `Path`,
`Option` and `Result` retain their existing core identities.

## Migration order

1. Add the short names and module-level imports in a compileable intermediate
   state; migrate callers in compiler, stdlib, examples, tests and docs.
2. Move UTF-8 functions to `text`, relocate process execution to `process`, and
   update curated prelude module re-exports. Check selective imports, namespace
   imports, public re-exports and aliases.
3. Move any remaining compiler-only Str path helpers to compiler-owned support
   or convert them to typed `Path`. Migrate legacy filesystem callers to
   checked APIs and explicit decoding/error handling.
4. Remove the old public wrappers and flattened prelude symbols. Search all
   tracked Panackelty sources and package examples for the removed names.
5. Run focused stdlib and module-binding checks, then the complete check and
   bootstrap acceptance. Compare representative compiler/test timings as
   evidence for GI#106; do not treat the migration as a performance win by
   assumption.

## Main risks

- Moving path operations out of the standard library can touch much of the
  compiler loader, package loader, driver, coverage and source-inventory code.
- The old filesystem wrappers trap on host errors. Converting callers to
  `Result` changes control flow and needs explicit error-path coverage.
- The prelude currently exports a flat surface; moving all call sites while
  keeping namespaced aliases requires exercising public re-export identity and
  alias resolution, including `Path`, `Option` and `Result`.
- Renaming every import may increase compile work while the compiler rebuilds
  its bootstrap stages. This is a useful measurement case, not a reason to
  retain the old public surface.

This plan keeps existing semantic domains recognizable while making imports
and qualified calls reveal which module owns an operation. Its first reviewable
implementation should be a separate stdlib/API slice from the probe-cache
performance pilot.
