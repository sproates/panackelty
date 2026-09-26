# Development harness retirement

This is a historical audit of the completed toolchain migration. For current
test commands and suite ownership, see [the testing guide](README.md).

The pre-migration suite retained 52 Python test methods. This milestone replaces
31 development contracts with POSIX shell checks under `tests/unit/harness`,
using the native `tests/runner/harness_command.panack` for bounded subprocesses.
The final 21 methods protected the transitional Python implementation itself;
they retired with that implementation. All native replacement contracts remain.
No language feature, VM semantics, compiler seed or bytecode format changes.

`make harness` runs every migrated group. `make unit` includes it,
and `make check-compiler` includes the seed and runner groups. Both Linux x86-64
and macOS arm64 package gates require the full harness alongside packaging. The supervisor
is compiled once into a temporary directory per invocation and removed on exit.
Each group owns an isolated temporary tree, including paths with spaces.

The shell owns host orchestration, archive/Make/CI contracts, and fault injection.
Panackelty owns timeout, signal, exit-status and binary-stream capture; the child
programs still execute through the public CLI. Native C fault, sanitizer and
coverage gates are unchanged. Expected failure output is asserted explicitly;
an unexpected signal or host timeout cannot masquerade as an expected exit.

## Retired method inventory

Every row below identifies the original method. All subcases and negative
assertions are preserved by its replacement group. The runner output-mismatch
case now copies just its selected `hello_world` fixture, not unselected fixtures.
Distribution checks independently verify the archive digest, numeric and named
owners, member paths, exact installed/archived file sets, and moved imports.
They unset the checkout's stdlib override before running installed binaries so
missing packaged modules cannot be masked by the source tree.

| Former file under `tests/unit` | Replacement under `tests/unit/harness` | Contract |
| --- | --- | --- |
| `test_layout.py` | `layout.sh` | Exact repository files, version, release and CI policy, issue form, website and tour links |
| `test_native_distribution.py` | `distribution.sh` | Build flags, checksums, archive metadata and file sets, relocation, installed imports and spaced paths |
| `test_validation.py` | `validation.sh` | Timing warnings, original failure status and machine-readable records |
| `compiler/test_panackelty_runner.py` | `runner.sh` | Runner positive selections and adversarial fixture, report, cleanup and path cases |
| `compiler/test_bootstrap.py` | `bootstrap.sh` | Corrupt seed rejected before stage two |

| Former method | Replacement group |
| --- | --- |
| `test_layout.py: test_python_implementation_is_confined_to_bootstrap` | `layout.sh` |
| `test_layout.py: test_public_launcher_has_no_python_dependency` | `layout.sh` |
| `test_layout.py: test_release_version_has_one_canonical_source` | `layout.sh` |
| `test_layout.py: test_ci_packages_every_supported_release_target` | `layout.sh` |
| `test_layout.py: test_ci_runs_full_validation_once_per_pr_revision` | `layout.sh` |
| `test_layout.py: test_release_publication_requires_every_gate` | `layout.sh` |
| `test_layout.py: test_bug_report_form_requires_actionable_reproduction_details` | `layout.sh` |
| `test_layout.py: test_readme_quick_start_is_an_executable_release_gate` | `layout.sh` |
| `test_layout.py: test_project_website_is_static_and_deploys_only_from_main` | `layout.sh` |
| `test_layout.py: test_language_tour_links_tested_examples_and_specification_sections` | `layout.sh` |
| `test_native_distribution.py: test_native_build_flags_are_configurable` | `distribution.sh` |
| `test_native_distribution.py: test_release_checksum_matches_archive` | `distribution.sh` |
| `test_native_distribution.py: test_compiler_artifact_path_from_spaced_checkout` | `distribution.sh` |
| `test_native_distribution.py: test_installed_cli_runs_without_source_tree_layout` | `distribution.sh` |
| `test_native_distribution.py: test_release_archive_is_friendly_and_relocatable` | `distribution.sh` |
| `test_validation.py: test_reports_phase_timing_and_budget_warning` | `validation.sh` |
| `test_validation.py: test_preserves_command_failure_status` | `validation.sh` |
| `test_validation.py: test_appends_machine_readable_timing_record` | `validation.sh` |
| `compiler/test_panackelty_runner.py: test_smoke_rejects_mismatched_or_missing_captured_report` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_failure_commands_cover_run_and_disassembly` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_failed_compilation_corpus_is_available_to_incremental_check` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_cli_environment_files_case_runs_through_public_runner` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_cli_commands_case_runs_through_public_runner` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_cli_check_disasm_case_runs_through_public_runner` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_failure_diagnostics_and_artifact_cleanup` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_example_output_mismatch_and_missing_pair_fail` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_nonempty_workspace_fails_and_is_recovered` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_expected_output_mismatch_fails_the_runner` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_stdlib_fixture_discards_inherited_value` | `runner.sh` |
| `compiler/test_panackelty_runner.py: test_source_path_rejects_escape_and_symlink` | `runner.sh` |
| `compiler/test_bootstrap.py: test_corrupt_seed_is_rejected_before_bootstrap` | `bootstrap.sh` |

## Implementation-only methods retired with the implementation

These methods have no remaining development-harness ownership. Native wire,
compiler and VM contracts already have separate evidence in
[ORACLE_REPLACEMENT.md](ORACLE_REPLACEMENT.md); Python object shapes, adjustable
limits, verifier hooks, hash randomisation and bootstrap-specific wording must
not be misrepresented as native APIs.

| Former file | Methods |
| --- | --- |
| `tests/unit/bytecode/test_serialization.py` | `test_rejects_artifacts_over_the_size_limit_before_decoding`, `test_decoder_checks_declared_resource_limits`, `test_source_build_verifies_emitted_bytecode`, `test_adt_bytecode_round_trip`, `test_cli_compilation_is_identical_across_hash_seeds` |
| `tests/unit/bytecode/test_vectors.py` | `test_minimal_v8_vector_loads_runs_and_is_canonical` |
| `tests/unit/bytecode/test_verifier.py` | `test_requires_valid_entry_point`, `test_rejects_invalid_function_signatures`, `test_rejects_empty_functions_and_missing_returns`, `test_rejects_malformed_and_unknown_instructions`, `test_rejects_invalid_simple_operands`, `test_rejects_invalid_control_flow_targets`, `test_rejects_invalid_composite_operands`, `test_rejects_invalid_constants`, `test_rejects_malformed_unknown_and_wrong_arity_calls`, `test_bytecode_verifier_preserves_purity`, `test_validates_indirect_call_arity_operand`, `test_enforces_resource_limits_on_in_memory_bytecode` |
| `tests/unit/compiler/test_bootstrap_diagnostics.py` | `test_impure_callable_wording`, `test_logical_extension_wording` |
| `tests/unit/vm/test_numeric.py` | `test_dec_nonterminating_division_requires_rounding` |

## Additional failure evidence

The migrated timing tests append success and failure records to the same report,
check report-write failures preserve the child status, and exercise the native
supervisor with unexpected status, signal termination, a bounded timeout, and
NUL-containing stdout/stderr. Archive validation uses a valid control and rejects
wrong ownership, symlink entries and the wrong top-level directory before any
extraction. Corrupt-seed validation also asserts stage two is never produced.

## Final removal — complete

The 21 implementation-only methods, compatibility facade and transitional source
are removed together. Make and CI contain no interpreter setup or invocation.
The repository policy and isolated-platform validation are described in
[PYTHON_REMOVAL.md](PYTHON_REMOVAL.md). Every native harness, functional,
seed-refresh and instrumented gate remains active.
