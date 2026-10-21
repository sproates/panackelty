# P01 call-site ledger and scope

The source corpus is every tracked `.panack` file under `src`, `packages`, `tools`, `examples` and `tests` at commit `5327c2b6789e47801a03e663b007fd2a6f9da0c9`: 615 files. No product source was changed. There are 24,125 unique parsed call/reference spans. 24,099 match checked body nodes; two pure guard calls are resolved from their local declarations because guards are checked outside function-body traversal; 24 are intentionally rejected calls or pruned children in rejection fixtures. All production and example call sites match checked nodes.

## Ledgers

| Corpus | Unique spans | Ledger |
| --- | ---: | --- |
| src | 8912 | [src call sites](p01-calls-src.md) |
| packages | 1006 | [packages call sites](p01-calls-packages.md) |
| tools | 40 | [tools call sites](p01-calls-tools.md) |
| examples | 233 | [examples call sites](p01-calls-examples.md) |
| tests | 13934 | [tests call sites](p01-calls-tests.md) |

## How to interpret a row

- Offsets are zero-based Unicode code-point offsets with an exclusive end, matching compiler SourceSpan. Lines and columns are one-based.
- `namespace` includes namespace-qualified functions and constructors/variants. `receiver` means current receiver-first sugar, not future owned dispatch. `owned` is the existing arrow PoC. `ref` is a named function reference. `dot` is a call rejected before a checked call node was retained.
- The selected target is a core primitive tag or a physical source declaration identity. Package-aware loading and explicit package tags disambiguate same-name modules before conversion to a tracked source path. The source path is unique in this fixed workspace; portable packages must continue to use full package/module/declaration identity in the compiler.
- `first_argument_type` is the receiver type only for receiver/owned calls. For ordinary/namespace calls it is the first explicit argument, and for zero-argument/reference rows it is empty.
- A retained bound node is not a success assertion. The ledger deliberately includes invalid argument types and error cases from negative tests. Never infer that Array.has, Set.get, Nat.starts_with or Unit.nat are legal just because a rejection fixture retains that target and type.
- The complete API replacement/disposition is in [the API register](p01-api-register.md); stdlib source signatures are in [the source inventory](dispatch-stdlib-inventory.md). Rows link each caller to those policies and its delivery/removal packets.
- Repeated module loading is deduplicated by physical source span. When an initial package report had a shortened ambiguous module name, the later package-identity-aware result supersedes it. No ambiguous short name is treated as a declaration identity.

## Explicit exceptions

| Source | Call | Evidence and disposition |
| --- | --- | --- |
| `tests/fixtures/compiler_checker/checks_method_calls_as_receiver_first_calls-3.panack:1:48` | `value` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/fixtures/compiler_checker/checks_method_calls_as_receiver_first_calls-3.panack:1:48` | `Box` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/fixtures/compiler_contracts/namespaces/emission/left.panack:17:31` | `positive` | Local pure guard helper; retain explicit call. Its declaration and type guard are in the same module. |
| `tests/fixtures/compiler_contracts/namespaces/emission/right.panack:17:31` | `positive` | Local pure guard helper; retain explicit call. Its declaration and type guard are in the same module. |
| `tests/fixtures/compiler_contracts/spans/imported.panack:2:29` | `broken` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/fixtures/compiler_contracts/syntax/parenthesized_dot_name_is_a_method_not_a_record_field_call-1.panack:1:68` | `value` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/fixtures/compiler_purity/guard_expressions_are_pure-1.panack:2:27` | `read_line` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/core_no_global_method/main.panack:1:22` | `ends_with` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/core_private_helper/main.panack:1:22` | `core_ends_with` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/imported_unknown_name/main.panack:4:3` | `broken` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_duplicate/main.panack:6:16` | `print` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_duplicate/main.panack:6:22` | `$owned_call$apple` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_duplicate/main.panack:6:22` | `Fruit` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_import_no_fallback/main.panack:12:9` | `$owned_call$apple` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_import_no_fallback/main.panack:12:9` | `Fruit` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_no_global_fallback/main.panack:4:9` | `$owned_call$apple` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_no_global_fallback/main.panack:4:9` | `Fruit` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_private_import/main.panack:8:9` | `$owned_call$banana` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_private_import/main.panack:8:9` | `Fruit` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_private_import/main.panack:9:9` | `$owned_call$banana` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_private_import/main.panack:9:9` | `RenamedFruit` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_private_import/main.panack:10:9` | `$owned_call$banana` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/owned_private_import/main.panack:10:9` | `ReexportedFruit` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/stdlib_legacy_api_removed/main.panack:3:27` | `path_string_join` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/stdlib_legacy_api_removed/main.panack:4:36` | `filesystem_read_text` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |
| `tests/functional/failures/stdlib_legacy_api_removed/main.panack:5:34` | `filesystem_exists` | Rejected or child pruned by failed parent. Preserve rejection coverage; no method owner inferred. |

## Parser-rejected source files

These ten inputs produce no complete AST. They are maintained malformed-input fixtures, not silently omitted production code. Their lexer/parser diagnostics are the reason no call sites can be bound.

| Source | Phase | Diagnostic |
| --- | --- | --- |
| `tests/fixtures/compiler_contracts/imports/logical_standard_library_imports_are_extensionless_and_load_once-1/stdlib/option.panack` | lex | unexpected character # |
| `tests/fixtures/compiler_contracts/syntax/legacy_function_keywords_and_arrows_are_rejected-1.panack` | parse | expected opening function parenthesis |
| `tests/fixtures/compiler_contracts/syntax/legacy_function_keywords_and_arrows_are_rejected-2.panack` | parse | expected colon after function parameters |
| `tests/fixtures/compiler_contracts/syntax/legacy_function_keywords_and_arrows_are_rejected-3.panack` | parse | expected opening function parenthesis |
| `tests/fixtures/compiler_contracts/syntax/legacy_let_and_missing_unit_return_are_rejected-1.panack` | parse | expected semicolon or closing brace |
| `tests/fixtures/compiler_contracts/syntax/same_line_statements_still_require_a_separator-1.panack` | parse | expected line break, semicolon, or closing brace after binding |
| `tests/functional/failures/else_if_missing_body/main.panack` | parse | expected opening brace |
| `tests/functional/failures/legacy_function_keyword/main.panack` | parse | expected opening function parenthesis |
| `tests/functional/failures/same_line_without_separator/main.panack` | parse | expected line break, semicolon, or closing brace after binding |
| `tests/functional/failures/unexpected_character/main.panack` | lex | unexpected character # |

## Generated and documented source

[Template register](p01-source-templates.md) records quoted Panack fragments, host-language source templates/heredocs, and Panack code fences that contain receiver-call syntax. They are separately identified as templates, parser-valid snippets or fragments. They are not counted as checked runnable programs and are not mixed into the 24,125-site denominator. P11/P14 must migrate the generator and its expected diagnostics/spans together; proposed and historical examples must keep their status explicit.

## Reproduction

[Probe and reproduction procedure](p01-reproduction.md) describes the real-parser/real-binder pass, package loading, source-root fallback and integrity checks. Source-name searches were used to locate candidates and audit fixtures, never to decide namespace-versus-value dispatch.

## Completion interpretation

This inventory accounts for the current corpus and identifies every call-site action. It does not implement generic dispatch or certify future signatures. P02 is the next implementation packet after accepting the inventory; P03 then demonstrates server/Map coexistence. The migration issue remains open.
