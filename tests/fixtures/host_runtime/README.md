# Host, runtime, and standard-library contract inventory

The original selected suites on `main` contained 37 test methods. Before
migration, 35 passed and two permission tests were skipped when run as root. Thirty-one
methods now have direct Panackelty/C/public CLI evidence and their Python wrappers
were retired. The other six now use fixed expectations and native/bootstrap
checks documented in `tests/ORACLE_REPLACEMENT.md`. No Python test was retired because of a warning
or speed budget. Native diagnostics may use generic wording where the Python
oracle names a missing environment variable; both check the same failure.

`tests/runner/host_runtime_unit.panack` checks 95 direct observations, including
byte-exact 128 KiB duplex streams, an early-closing child, environment/cwd/signal
isolation, process limits and timeouts, clock duration, final symlinks, FIFO
rejection, temporary mode 0600, raw non-UTF-8 names, lexical paths, nested
bytecode arguments, four embedded-NUL services, checked fixture discovery,
report ordering, and all 12 effects forbidden in a pure function. Temporary
files and workspaces are explicitly removed. Public source and bytecode
workflows also run in `tests/functional/cases/cli_environment_files`,
`host_capabilities`, `host_types`, `host_process`, `stdlib`, and the three
`testing_*` fixtures. The invalid UTF-8 file case now requires the exact
failure detail, in addition to the other I/O failure categories.

Native C tests in `tests/unit/vm/native_modules.c` check all eleven host
service signatures and their wrong-operand traps, plus distinct opaque host
value kinds; `native_faults.c` injects clock, sleep, process and filesystem
failures and verifies cleanup. The Python-only forced clock error is thereby
covered natively, including the `ClockUnavailable` result. Both C suites run
under sanitizers and LLVM coverage. The full functional source/compiled
matrix continues to exercise the actual native VM on Linux and macOS.

## Method-by-method ownership

| Original method | Replacement evidence |
|---|---|
| `tests/unit/vm/test_runtime.py::test_lexer_character_classes_are_deliberately_ascii` | `host_runtime_unit` source, process, or path contract |
| `tests/unit/vm/test_runtime.py::test_text_and_binary_file_round_trips` | `cli_environment_files` source/bytecode and disk bytes |
| `tests/unit/vm/test_runtime.py::test_file_io_failures_become_language_errors` | `cli_environment_files` five failure classes with exact invalid UTF-8 detail |
| `tests/unit/vm/test_runtime.py::test_file_io_rejects_paths_with_embedded_nul` | `host_runtime_unit` four public NUL traps |
| `tests/unit/vm/test_runtime.py::test_file_io_reports_denied_paths` | `cli_environment_files` four POSIX permission cases with root skip |
| `tests/unit/vm/test_runtime.py::test_command_arguments_are_available_to_programs` | `host_runtime_unit` source, process, or path contract |
| `tests/unit/vm/test_runtime.py::test_environment_is_snapshotted_and_missing_values_trap` | `host_runtime_unit` source, process, or path contract |
| `tests/unit/vm/test_runtime.py::test_stderr_and_process_exit_boundary` | `host_runtime_unit` source, process, or path contract |
| `tests/unit/vm/test_runtime.py::test_path_operations_and_nested_bytecode_execution` | `host_runtime_unit` file existence, path suffix, nested output and cleanup |
| `tests/unit/vm/test_runtime.py::test_nested_bytecode_can_receive_an_explicit_argument_snapshot` | `host_runtime_unit` parent/child argument isolation |
| `tests/unit/vm/test_runtime.py::test_path_join_is_lexical_and_does_not_make_relative_paths_absolute` | `host_runtime_unit` source, process, or path contract |
| `tests/unit/vm/test_host_capabilities.py::test_oracle_shared_conformance` | Fixed native oracle contracts; see `../../ORACLE_REPLACEMENT.md` |
| `tests/unit/vm/test_host_capabilities.py::test_large_duplex_streams_and_early_stdin_close` | `host_runtime_unit` exact 128 KiB simultaneous stdout/stderr/input and early stdin close |
| `tests/unit/vm/test_host_capabilities.py::test_environment_cwd_exit_signal_and_invalid_utf8` | `host_runtime_unit` environment/cwd/exit/signal, invalid UTF-8 bytes and decoding |
| `tests/unit/vm/test_host_capabilities.py::test_limits_and_timeouts_are_bounded` | `host_runtime_unit` zero/negative/out-of-range timeout, output limit, elapsed <2s and absent side effect |
| `tests/unit/vm/test_host_capabilities.py::test_filesystem_symlinks_limits_and_explicit_cleanup` | `host_runtime_unit` symlink/FIFO, temp uniqueness/mode, raw names, limits and cleanup |
| `tests/unit/vm/test_host_capabilities.py::test_sleep_waits_for_requested_duration` | `host_runtime_unit` measured >=1 ms monotonic wait |
| `tests/unit/vm/test_host_capabilities.py::test_effects_are_rejected_by_verifier_and_operands_by_runtime` | twelve fixed pure bytecode vectors plus direct C wrong-type checks for eleven host services |
| `tests/unit/vm/test_host_types.py::test_oracle_matches_shared_host_type_conformance` | Fixed native oracle contracts; see `../../ORACLE_REPLACEMENT.md` |
| `tests/unit/vm/test_host_types.py::test_equal_ticks_do_not_make_different_opaque_types_equal` | direct `native_modules.c` value equality for equal ticks and different kinds |
| `tests/unit/vm/test_host_types.py::test_clock_failure_is_a_structured_result` | direct `native_faults.c` clock injection and exact `ClockUnavailable` result |
| `tests/unit/vm/test_host_types.py::test_clock_reading_is_effectful_in_verified_bytecode` | `pure_instant_now.hex` native verifier case and compiler host-type contracts |
| `tests/unit/vm/test_host_types.py::test_path_lexical_boundaries_preserve_native_spelling` | `host_runtime_unit` eight native-byte parent/filename cases |
| `tests/unit/vm/test_native_execution.py::test_process_handles_inherited_ignored_sigpipe` | `host_runtime_unit` inherited ignored SIGPIPE on public host-process program |
| `tests/unit/vm/test_native_execution.py::test_native_vm_matches_program_outputs` | Fixed native oracle contracts; see `../../ORACLE_REPLACEMENT.md` |
| `tests/unit/vm/test_native_execution.py::test_rational_arithmetic_matches_fraction_oracle` | Fixed native oracle contracts; see `../../ORACLE_REPLACEMENT.md` |
| `tests/unit/vm/test_native_execution.py::test_native_vm_runs_the_self_hosted_compiler` | Fixed native oracle contracts; see `../../ORACLE_REPLACEMENT.md` |
| `tests/unit/vm/test_native_execution.py::test_native_host_boundary_passes_arguments_environment_and_status` | `host_runtime_unit` exit 7 with exact binary streams, argument and environment |
| `tests/unit/vm/test_native_execution.py::test_native_file_io_round_trips_and_failures` | `cli_environment_files` source and compiled outputs, disk contents and missing/invalid files |
| `tests/unit/vm/test_native_execution.py::test_native_file_io_rejects_embedded_nul_paths` | `host_runtime_unit` four public NUL traps |
| `tests/unit/vm/test_native_execution.py::test_native_file_io_reports_denied_paths` | `cli_environment_files` four POSIX permission cases with root skip |
| `tests/unit/compiler/test_stdlib.py::test_complete_library_is_byte_identical_at_bootstrap_and_stage_one` | Fixed native oracle contracts; see `../../ORACLE_REPLACEMENT.md` |
| `tests/unit/compiler/test_stdlib.py::test_environment_wrapper_returns_option_without_trapping` | `host_runtime_unit` source imported option wrapper, present/missing values |
| `tests/unit/compiler/test_testing_library.py::test_empty_report_has_zero_failures` | `host_runtime_unit` exact empty report/count |
| `tests/unit/compiler/test_testing_library.py::test_failed_condition_retains_reason_and_order` | `host_runtime_unit` exact ordered mixed report/count |
| `tests/unit/compiler/test_testing_files.py::test_invalid_fixture_roots_are_structured_errors` | `host_runtime_unit` missing and regular-file fixture roots |
| `tests/unit/compiler/test_testing_commands.py::test_output_comparison_and_error_result_are_pure` | `host_runtime_unit` exact six-result report with all mismatch and host-error variants |

The old `tests/unit/file_io_cases.py` generated sources only for retired
methods; its contents are now represented by the functional and direct public
CLI probes. Standard-library byte identity, compiler artifacts, seeded Fraction
properties and program outputs now use fixed independent expectations in
`../oracle_contracts`. Bootstrap and harness safeguards still use Python; their
ownership is recorded in `../../ORACLE_REPLACEMENT.md`.

The direct process environment/cwd/binary-stderr case compares the child's
working directory by filesystem identity. Shells may resolve symlinks when
printing `pwd` (for example `/tmp` to `/private/tmp` on macOS), so path spelling
is not the cwd contract. A mismatched directory exits with status 8; the
expected status remains 9, with exact environment output and binary stderr.
