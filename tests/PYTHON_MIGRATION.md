# Python-free development test architecture

This is the migration contract for removing Python from the *repository*.
The downloadable compiler, native VM, package, and release smoke path already
run without it. The functional suite now uses Panackelty; the Python unit
oracle and unit harness remain required by `make check` until their separate
parity gates pass.

## Baseline and ownership (September 2026)

The baseline suite had 294 discovered unit test methods (160 compiler, 43
bytecode, 72 VM, 19 other) and 20 functional test methods. All twenty
functional methods have been replaced; the Panackelty runner now discovers
25 selected case directories, 20 example output files,
and 41 failure directories. The bytecode fixture directory has 26 `.hex`
artifacts spanning supported and deliberately rejected historical versions.
Methods are not a coverage metric: parameterized subtests, generated cases,
and the discovered programs exercise substantially more observations.

| Current owner | Required replacement | Evidence not to lose |
| --- | --- | --- |
| Former `tests/functional/test_programs.py` | `tests/runner/main.panack`, `tests/runner/compiler_driver.panack`, and `functional/cases/*` | Exact stdout/stderr and status for source and saved bytecode; `check`, `compile`, `run`, `disasm`, shorthand, arguments, imports, examples, invalid source, malformed artifacts, environment and file I/O. Preserve `source.path` validation, failure diagnostic normalization, isolated outputs, and the stage-2 compiler reuse. |
| `tests/unit/compiler/` | Focused Panackelty compiler probes and public-CLI contract fixtures | Lexer, parser, resolver, checker, purity, emitter, loader, driver, diagnostics, generics, type and host boundary failures; deterministic artifacts and bootstrap-stage parity. Do not replace precise negative assertions with only success programs. |
| `tests/unit/bytecode/` and `tests/fixtures/bytecode/` | Portable golden bytecode and malformed vectors, native verifier/decoder tests, and Panackelty-hosted codec assertions | Versioned encoding, canonical round trips, verifier rejection, resource bounds, forged unsafe states, exact error categories, and repeated-compilation identity. |
| `tests/unit/vm/` and `tests/unit/vm/*.c` | Direct C module/fault/sanitizer tests plus black-box Panackelty programs on the native VM | Stack/frame/value ownership, exact numeric semantics, collection and host operations, allocation/syscall injection and cleanup, runtime traps, and native coverage. Keep the C tests; replace their Python launch/assert wrappers. |
| `tests/unit/{support,file_io_cases,forged_runtime,rational_cases}.py` | Portable declarative inputs and expected results, shared by the relevant native and Panackelty tests | Generated source and forged bytecode cases must be inventoried individually before replacing them; preserve boundary values, failure categories, and binary data rather than counting files. |
| `tests/unit/test_{layout,native_distribution,validation}.py` | POSIX packaging/CI contract checks and Panackelty-hosted assertions where appropriate | Exact archive contents and checksums, relocation and paths with spaces, CI safety triggers, clean build flags, timing warnings, and quick-start output. Keep exact-artifact shell gates independent of the source tree. |
| `panackelty.py`, `src/bootstrap/panackelty.py`, `make regenerate-seed` | Checked native/self-hosted cross-checks, portable golden artifacts, fixed-point bootstrap, and a documented staged seed refresh | Python compiler/VM differential accept/reject and trap evidence, seed integrity, stage-2/stage-3 byte identity, and a reviewable seed digest. |

`tests/COVERAGE.md` remains the specification-to-behavior map. For each migrated
area, record the old assertion or generated case and its new evidence there
before deleting the old test. Preserve intentionally uncovered cases as named
backlog items, not implicit losses.

## Runner and fixture contracts

### Compiler unit migration in progress

`tests/runner/compiler_lexer_unit.panack` directly imports the compiler lexer
and runs in `make unit` and `make check-compiler`. It replaces all seven methods
in the former `tests/unit/compiler/test_self_hosted_lexer.py` (eight named
assertions because the two unterminated-string inputs are separate). The
replacement checks exact token classes, all punctuation and longest-match
boundaries, decimal/range ambiguity, every invalid character and position,
both unterminated-string variants, half-open offsets, and semicolon insertion
at line breaks. It asserts on the lexer result before the public CLI's parser
and type checker can affect the result. Remaining compiler unit files and
Python differential checks still run; lexer and parser are completed portions
of the compiler unit step, not its completion gate.

`tests/runner/compiler_parser_unit.panack` replaces all 38 methods from
`tests/unit/compiler/test_self_hosted_parser.py`, expanding them into 192 named
assertions. The migration audit compared every original parser entry point,
input, and expected string with the replacement. The old Python suite and new
native probe both passed, and the saved-bytecode report matched the source
report exactly. An injected incorrect expectation produced one named failure
and exit status 1. No expected string is obtained from the compiler under test.
Both `make unit` and `make check-compiler` require the probe; the unit timer now
covers Python plus Panackelty compilation/execution. These are direct parser
contracts, including malformed fragments that would never pass a full compiler
check. Positioned diagnostics through the loader remain in the existing
functional fixtures and other compiler unit tests.

For every row below, the old method was `test_<group>`, the replacement function
is `parser_unit_<group>`, and the report labels are `parser/<group>/<ordinal>`.
The count includes every expanded former subtest. Each comparison is exact.

| Parser group | Assertions |
| --- | ---: |
| `binary_operators_are_left_associative` | 1 |
| `generic_function_syntax_and_explicit_calls_round_trip` | 5 |
| `match_expressions_participate_in_precedence_and_blocks` | 1 |
| `method_calls_lower_to_receiver_first_calls` | 8 |
| `parentheses_override_precedence` | 1 |
| `parses_bindings_assignments_and_nested_type_references` | 1 |
| `parses_complete_mixed_program` | 1 |
| `parses_complete_nested_type_references` | 4 |
| `parses_empty_and_import_only_programs` | 8 |
| `parses_empty_statement_and_value_blocks` | 5 |
| `parses_enum_declarations` | 5 |
| `parses_every_binary_operator` | 14 |
| `parses_every_binary_precedence_level` | 1 |
| `parses_function_declarations` | 4 |
| `parses_guarded_type_declarations` | 1 |
| `parses_if_expressions_as_block_items` | 1 |
| `parses_if_expressions_with_block_branches` | 4 |
| `parses_inferred_bindings_without_new_operators` | 1 |
| `parses_match_expressions_and_patterns` | 5 |
| `parses_named_function_references_and_functional_methods` | 2 |
| `parses_record_declarations` | 4 |
| `parses_scalar_and_array_literals` | 7 |
| `parses_semicolonless_blocks_and_multiline_expressions` | 3 |
| `parses_unary_calls_fields_and_indexes` | 4 |
| `parses_while_and_for_statements` | 4 |
| `program_parser_propagates_lexer_failures` | 1 |
| `propagates_lexer_failures` | 1 |
| `reports_malformed_blocks` | 9 |
| `reports_malformed_enum_declarations` | 12 |
| `reports_malformed_expressions` | 10 |
| `reports_malformed_function_declarations` | 13 |
| `reports_malformed_guarded_type_declarations` | 7 |
| `reports_malformed_if_expressions` | 4 |
| `reports_malformed_import_declarations` | 5 |
| `reports_malformed_loops` | 7 |
| `reports_malformed_match_expressions` | 11 |
| `reports_malformed_record_declarations` | 10 |
| `reports_malformed_type_references` | 7 |
| **Total** | **192** |

The functional runner is `tests/runner/main.panack`. `make functional` runs it,
the compiler-driver check, and the runner smoke case from source and saved
bytecode without Python. It captures the full runner's successful output once;
both smoke modes compare those exact bytes, and a standalone smoke run invokes
the full runner itself. It discovers and selects `callables`,
`cli_check_disasm`, `cli_commands`, `cli_diagnostic_display`,
`cli_environment_files`, `cli_rational_failures`, `collections`, `compiler_lexer`, `compiler_skeleton`, `hello_world`, `host_capabilities`, `host_process`,
`host_types`, `local_inference`, `modules`, `optional_else`,
`rational_unit`, `records_and_enums`, `semicolonless`, `stdlib`, `string_boundaries`,
`testing_commands`, `testing_fixtures`, `testing_library`, and
`vm_numeric_boundaries`, checking exact stdout, empty stderr,
and zero status for source execution, compilation, and saved-bytecode execution.
It also discovers all twenty example sources and expected outputs, rejects
missing or stale pairs, and checks each program through the same three paths.
All forty-one failure fixtures also run through `check` and `compile`, requiring
exit status one, empty stdout, exact normalized stderr, and no bytecode artifact
after failed compilation. Six targeted failures also require the same exact
normalized diagnostics for `run` and `disasm`. The runner resolves each case directory to a physical
path for `<case>` normalization, including checkouts with spaces and symlinks.
The `cli_check_disasm` case compares source and bytecode disassembly and checks
malformed bytecode and legacy extension rejection through the public CLI.
The `cli_commands` case checks bare source and bytecode paths, default compiler
output, argument forwarding, controlled stderr and exit status, help, and version.
The `cli_environment_files` case checks environment overrides, text and binary file
round trips from source and saved bytecode, missing paths, invalid UTF-8,
and denied access on unprivileged POSIX hosts. `cli_rational_failures` checks
six source and bytecode runtime traps; `cli_diagnostic_display` checks exact
CRLF/Unicode/tab and EOF formatting. The compiler fixture uses `source.path`; its target is physically resolved within
the checkout before execution, including checks against symlink escapes. When
the bootstrap recipe supplies `PANACK_TEST_COMPILER`, the runner copies that
verified compiler artifact for its compiled-output check instead of compiling
the compiler again. Without it, the runner compiles the source normally.
It reports failures through its process status and removes each artifact before
requiring an empty workspace. `functional/cases/runner_smoke` checks its exact
success report through the public CLI; `unit/compiler/test_panackelty_runner.py`
injects wrong expected output and checks both source and bytecode failures. Its
`--test-cleanup-failure <parent>` mode leaves a sentinel in a uniquely created
workspace, verifies that empty-directory removal fails, reports a nonzero exit,
then removes the sentinel and workspace. The focused test requires the supplied
parent directory to be empty afterward.
### Retired Python functional assertions

The parallel runner passed `make check` locally and the required test, Linux
package, and macOS package jobs in PRs #46–#48 before this retirement. The
following observations have exact or stronger equivalents, including child
process status, stdout, stderr, and fixture cleanup:

| Retired `PanackeltyProgramTests` method | New evidence |
| --- | --- |
| `test_help_uses_panack_command_name`, `test_version_identifies_release_and_bytecode_format` | `cli_commands`: help command identity; exact version and bytecode format from `VERSION` |
| `test_bare_source_path_runs_program`, `test_compile_default_output_and_bare_bytecode_path` | `cli_commands`: bare source and saved bytecode, exact default output path and compile transcript |
| `test_run_passes_program_arguments`, `test_program_controls_stderr_and_exit_status` | `cli_commands`: arguments, exact stdout/stderr and exit seven |
| `test_legacy_source_extension_is_rejected`, `test_disasm_rejects_malformed_bytecode` | `cli_check_disasm`: exact failure status and diagnostic, empty stdout |
| `test_check_accepts_source_and_bytecode`, `test_disasm_matches_for_source_and_bytecode` | `cli_check_disasm`: source and saved-bytecode check, exact disassembly parity and key instructions |

The final ten Python functional methods were retired after the following
replacement evidence was added. The `check-compiler`, `check-bytecode`, and
`check-vm` incremental recipes use the same Panackelty cases;
`--failures-only` selects all forty-one negative fixtures for the compiler
check without rerunning successes.

| Retired method | Replacement evidence |
| --- | --- |
| `test_source_and_compiled_program_outputs` | 25 selected cases and 20 discovered examples run from source and bytecode with exact output; `runner_smoke` runs separately from both paths under `make functional` |
| `test_rational_failures_in_source_and_bytecode` | `cli_rational_failures` compiles each of six generated inputs and checks the source and saved-bytecode trap messages and empty stdout |
| `test_standard_library_reads_the_process_environment` | `cli_environment_files` overrides the environment and checks the exact stdout suffix and empty stderr |
| `test_public_cli_file_io_round_trips_and_failures` | `cli_environment_files` checks source/bytecode text and binary round trips, disk bytes, five failing services, and explicit cleanup |
| `test_public_cli_reports_denied_file_io` | `cli_environment_files` checks four permission-denied services on unprivileged POSIX hosts; root hosts skip that portion, as the former Python method did |
| `test_self_hosted_compiler_driver_matches_bootstrap_artifacts` | `runner/compiler_driver.panack` reuses the checked stage-two artifact, asserts check/run/compile/disassembly and byte-identical outputs, and cleans its workspace |
| `test_invalid_source_programs_fail_check`, `test_invalid_source_programs_fail_compile_without_artifacts` | All 41 discovered failures compare exact normalized diagnostics and statuses under check/compile, with no artifact; the focused compiler recipe uses `--failures-only` |
| `test_source_diagnostics_from_run_and_disasm` | Six selected failure fixtures compare normalized stderr byte-for-byte for both additional commands |
| `test_diagnostic_display_for_crlf_unicode_tabs_and_eof` | `cli_diagnostic_display` checks both generated byte-exact diagnostics, including Unicode escaping and caret positions |

The Panackelty runner discovers immediate case and failure directories and
example files in sorted native-byte order. A success case keeps the existing
`main.panack` *or* `source.path` and its expected output; examples and failures retain their current fixture
layout. It checks source and compiled-bytecode execution separately, compares
raw byte streams and exit status, checks empty stderr on success, and reports
case names in a deterministic order. Invalid source must fail the intended
public command without leaving a compiled artifact. The runner's own failure
count sets a nonzero process exit status; `make check` must propagate it.

Use unique temporary workspaces and bounded subprocesses. Track every created
file and directory, remove contents explicitly, then require successful empty
workspace removal. A failed cleanup fails the test rather than silently
discarding evidence. Reuse a verified stage-2 compiler artifact when the
existing harness does, but never bypass the fixed-point bootstrap or the
representative source and bytecode public-CLI paths. Keep `make unit` and
`make functional` separate under `make check`; retain timings and budgets.

Some current harness guarantees need design or host-capability work before a
specific case can migrate:

- `source.path` targets must resolve inside the repository, including when
  they traverse symlinks. The compiler fixture now has that validation in
  the new runner; keep it for any later references.
- The harness removes `PANACKELTY_STDLIB_VALUE` from inherited child environments.
  `process_run` can override but not delete variables. The new runner uses
  `/bin/sh` to unset it for `stdlib` source, compile, and bytecode commands;
  its smoke test injects an ambient value. The explicit environment-override
  contract runs in `cli_environment_files`.
- The former Python harness parallelized independent programs (up to four
  workers). The runner is sequential; profile and improve its timing without
  dropping cases, while retaining the warning budgets.
- Failure fixtures now normalize checkout-dependent diagnostic paths and have
  a focused path-with-spaces regression. Preserve that exact comparison as new
  negative cases are added.
- Some tests construct malformed bytecode and source probes dynamically. Move
  those to shared declarative fixtures or independently validated generators
  before retiring their Python builders.

## Migration order and removal gates

1. Establish a Panackelty runner on a representative fixed fixture subset.
   Run old and new paths together; compare the per-case observation matrix,
   including deliberate failures and cleanup errors. The selected subset and
   output-mismatch and injected nonempty-workspace failures are now covered;
   broader host-error injection still belongs to the full-corpus migration.
2. Port the full functional corpus and orchestration, resolving the contracts
   above. Retire a Python assertion only with equivalent new evidence in
   `tests/COVERAGE.md` and a green `make check` on both supported platforms.
3. Port unit coverage by compiler, bytecode, VM, and host subsystem. Keep
   targeted native C tests for internals and portable vectors for cross-runtime
   contracts. Maintain focused component checks and the full suite throughout.
4. Replace the differential Python oracle and seed-regeneration command with
   golden, native/self-hosted, and fixed-point evidence. Document a verified
   staged seed refresh; audit every former oracle-only failure case.
5. Remove the facade, bootstrap Python, harness, Make/CI Python setup and
   commands only after no consumers remain. Add a policy check against Python
   source, shebangs, and invocations. From a clean environment without Python,
   pass `make check`, native conformance, bootstrap, packaging, and release
   smoke on Linux x86-64 and macOS arm64.

Each migration PR should list the old tests/cases retired, the replacement
fixtures and assertions, the before/after behavior matrix, platform checks,
and any timing change. Keeping both implementations temporarily is a deliberate
migration aid, not a permanent second execution engine.
