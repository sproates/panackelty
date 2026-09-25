# VM contract corpus

These version-8 hexadecimal artifacts were captured from the original tests at
`9677518` before retirement. All 38 original selected methods passed during
capture. The compiler/stdlib loader oracle and compile-time nonterminating
Decimal check remain in Python; the other 36 methods expand into 145 native
observations. `manifest.json` records each original method and case name.

`tests/runner/vm_unit.panack` runs each artifact through the native VM's `run`
or `check` command and compares exact status, stdout and stderr, with a timeout,
output bounds and checked temporary-file cleanup. This is direct VM evidence;
the existing functional suite still compiles and executes source programs.

The 61 cases carrying `python_*` fields also run in `test_vm_oracle.py` until
the separately planned oracle replacement. They preserve the original Python
output, trap text and return type, including Void's None payload. Native and
Python diagnostic wording is recorded separately where it differs. Native
Void return kind also has a direct C assertion. The corpus includes all 27
former `FORGED_DYNAMIC_FAILURES`, every loader truncation, the eight string
index/slice boundaries, all rational conversion/zero failures, exact 150-digit
Decimal multiplication, calls, frames, branching, collections and Unicode.

## Original method inventory

| Original method | Expanded cases |
|---|---:|
| `tests.unit.vm.test_execution.ExecutionTests.test_array_index_is_bounds_checked_by_vm` | 1 |
| `tests.unit.vm.test_execution.ExecutionTests.test_arrays_iteration_indexing_and_len` | 1 |
| `tests.unit.vm.test_execution.ExecutionTests.test_boolean_operators_short_circuit` | 1 |
| `tests.unit.vm.test_execution.ExecutionTests.test_pure_for_loop_with_mutable_accumulator` | 1 |
| `tests.unit.vm.test_execution.ExecutionTests.test_pure_while_loop_with_mutable_locals` | 1 |
| `tests.unit.vm.test_execution.ExecutionTests.test_str_index_is_bounds_checked_by_vm` | 1 |
| `tests.unit.vm.test_execution.ExecutionTests.test_str_interpolation_concatenation_and_numeric_values` | 1 |
| `tests.unit.vm.test_execution.ExecutionTests.test_str_unicode_indexing_and_length` | 1 |
| `tests.unit.vm.test_execution.ExecutionTests.test_string_methods_reverse_unicode_code_points` | 1 |
| `tests.unit.vm.test_numeric.NumericTests.test_big_natural_arithmetic` | 1 |
| `tests.unit.vm.test_numeric.NumericTests.test_dec_arithmetic_exceeds_host_context_without_rounding` | 1 |
| `tests.unit.vm.test_numeric.NumericTests.test_dec_finite_division_is_exact` | 1 |
| `tests.unit.vm.test_numeric.NumericTests.test_dec_is_exact` | 1 |
| `tests.unit.vm.test_numeric.NumericTests.test_integer_division_produces_rational` | 1 |
| `tests.unit.vm.test_numeric.NumericTests.test_rational_conversion_and_zero_failures` | 6 |
| `tests.unit.vm.test_numeric.NumericTests.test_rational_unit_program_matches_contract` | 1 |
| `tests.unit.vm.test_collections.CollectionTests.test_byte_buffers_and_utf8` | 1 |
| `tests.unit.vm.test_collections.CollectionTests.test_empty_array_append_and_concat_are_pure` | 1 |
| `tests.unit.vm.test_collections.CollectionTests.test_persistent_maps_and_sets_are_pure` | 1 |
| `tests.unit.vm.test_collections.CollectionTests.test_short_collection_aliases_dispatch_by_receiver_type` | 1 |
| `tests.unit.bytecode.test_contract.BytecodeContractTests.test_binary_operands_preserve_push_order` | 1 |
| `tests.unit.bytecode.test_contract.BytecodeContractTests.test_call_frames_have_isolated_locals_and_return_values` | 1 |
| `tests.unit.bytecode.test_contract.BytecodeContractTests.test_conditional_jump_consumes_its_condition` | 1 |
| `tests.unit.bytecode.test_contract.BytecodeContractTests.test_forged_runtime_safety_failures_trap_in_the_oracle` | 27 |
| `tests.unit.bytecode.test_contract.BytecodeContractTests.test_indirect_calls_recheck_purity_at_runtime` | 1 |
| `tests.unit.bytecode.test_contract.BytecodeContractTests.test_indirect_calls_validate_and_deliver_results` | 1 |
| `tests.unit.bytecode.test_contract.BytecodeContractTests.test_invalid_dynamic_state_becomes_a_vm_trap` | 4 |
| `tests.unit.vm.test_native_loader.NativeLoaderTests.test_accepts_minimal_version_eight_artifact` | 1 |
| `tests.unit.vm.test_native_loader.NativeLoaderTests.test_rejects_every_truncation_of_a_valid_artifact` | 19 |
| `tests.unit.vm.test_native_loader.NativeLoaderTests.test_rejects_forged_structural_and_semantic_violations` | 9 |
| `tests.unit.vm.test_native_loader.NativeLoaderTests.test_rejects_shared_malformed_artifacts` | 6 |
| `tests.unit.vm.test_native_execution.NativeExecutionTests.test_string_index_and_slice_boundaries_still_trap` | 8 |
| `tests.unit.vm.test_native_execution.NativeExecutionTests.test_native_vm_matches_numeric_and_trap_semantics` | 5 |
| `tests.unit.vm.test_native_execution.NativeExecutionTests.test_rational_conversion_and_zero_failures` | 6 |
| `tests.unit.vm.test_native_execution.NativeExecutionTests.test_native_vm_traps_on_forged_dynamic_failures` | 29 |
| `tests.unit.vm.test_native_execution.NativeExecutionTests.test_native_vm_rechecks_indirect_call_purity` | 1 |

## Native wrapper replacements

- `NativeBigIntTests.test_arithmetic_exceeds_host_word_size`: the unchanged C
  source is now `unit/vm/native_bigint.c`, built with the selected toolchain and
  instrumentation flags. The Panackelty runner checks its exact three-line output.
- `NativeModuleTests.test_ownership_traps_unicode_and_decoder_mutations`:
  the same C harness runs with its exact success output and 20-second bound.
- `NativeModuleTests.test_headers_are_self_contained_and_repeatable`:
  `tests/native_headers.sh` compiles every header twice in one translation unit
  with the original strict C flags and selected `CC`.
- `NativeModuleTests.test_decimal_division_removes_only_redundant_fractional_zeros`:
  all five original commands and exact results run through the C arithmetic probe.
- The three `NativeFaultTests` methods: unchanged C allocation/syscall sweeps,
  all-operand decoder/mutation corpus, and execute/trap ownership fixtures now
  run from Panackelty with the original 30-second bounds. Ownership artifacts
  still require INTERPOLATE, UNARY, CALL_VALUE, ITER_NEXT, MAKE_RECORD and
  MAKE_VARIANT in their disassembly. Decode-only artifacts are never executed.

`ownership_execute.panack` and `ownership_trap.panack` preserve the original
source behind their reviewed bytecode fixtures. The operand corpus retains all
22 opcodes and all six constant forms. No fixture is regenerated during tests.

The runner respects the selected native VM/module/fault/bigint executables and
forwards sanitizer settings and LLVM_PROFILE_FILE. Both instrumentation gates
run the same corpus in separate build directories; they do not overwrite the
ordinary CLI binary. Native C tests remain in C.

## Retained work and ownership

Python's seeded integer/decimal/Fraction properties, builtin registry comparison,
bootstrap-compiled compiler/stdlib loading and cross-compiler program artifacts
remain independent oracle evidence for the oracle-replacement milestone.
`test_native_execution.py` also retains inherited SIGPIPE, environment and file
checks for the host/runtime milestone. `test_runtime.py`, host capabilities,
host types and stdlib/testing-library wrappers remain assigned to that milestone.
This PR does not count those retained tests as removed Python.
