# P11 legacy test boundary

P11 migrates maintained executable callers. These fixture inputs intentionally
continue exercising the legacy frontend until P12 removes that frontend and
replaces the relevant assertions with owned-dispatch or old-syntax rejection
coverage. They are test data, not recommended application APIs. Removing them
in P11 would stop testing still-supported compatibility behavior.

This conservative token audit uses P01's closed API mappings and distinguishes
namespace imports from value receivers. Counts are candidate calls, not a new
semantic inventory or proof of accepted signatures. Historical P01 ledgers retain
their original baseline. P12 must inspect each input's assertion before changing
it; do not simply rewrite diagnostic text to match new failures.

## Standalone legacy inputs

The bytecode codec fixture uses the legacy `compile_source_bytecode` adapter;
its public-compiler bytes remain identical, but that adapter rejects owned calls.
Compiler checker/purity/contract inputs exercise raw syntax, old lowering,
inference and exact diagnostics. Functional failure fixtures retain their exact
rejection contracts until legacy semantics are removed.

| Fixture | Candidate calls |
| --- | ---: |
| `tests/fixtures/bytecode/codec_contracts/rational_unit.panack` | 2 |
| `tests/fixtures/compiler_checker/checks_callable_values_and_functional_array_methods-1.panack` | 3 |
| `tests/fixtures/compiler_checker/checks_callable_values_and_functional_array_methods-2.panack` | 1 |
| `tests/fixtures/compiler_checker/checks_method_calls_as_receiver_first_calls-1.panack` | 7 |
| `tests/fixtures/compiler_checker/checks_method_calls_as_receiver_first_calls-2.panack` | 1 |
| `tests/fixtures/compiler_checker/checks_method_calls_as_receiver_first_calls-4.panack` | 1 |
| `tests/fixtures/compiler_checker/checks_persistent_collections_and_builtin_arguments-1.panack` | 7 |
| `tests/fixtures/compiler_checker/checks_persistent_collections_and_builtin_arguments-2.panack` | 1 |
| `tests/fixtures/compiler_contracts/emitter/emits_function_references_indirect_calls_map_and_reduce-1.panack` | 3 |
| `tests/fixtures/compiler_contracts/emitter/emits_method_calls_as_receiver_first_calls-1.panack` | 7 |
| `tests/fixtures/compiler_contracts/generic_functions/bodies_are_checked_even_when_unused-2.panack` | 1 |
| `tests/fixtures/compiler_contracts/generic_functions/empty_evidence_and_argument_order-1.panack` | 1 |
| `tests/fixtures/compiler_contracts/generic_functions/empty_evidence_and_argument_order-2.panack` | 1 |
| `tests/fixtures/compiler_contracts/generic_functions/generic_helpers_cannot_hide_callable_effects-1.panack` | 1 |
| `tests/fixtures/compiler_contracts/generic_functions/generic_helpers_cannot_hide_callable_effects-2.panack` | 1 |
| `tests/fixtures/compiler_contracts/generic_functions/generic_helpers_cannot_hide_callable_effects-3.panack` | 1 |
| `tests/fixtures/compiler_contracts/generic_functions/inference_explicit_arguments_and_lexical_type_scope-1.panack` | 2 |
| `tests/fixtures/compiler_contracts/generic_functions/purity_and_generic_function_references-3.panack` | 1 |
| `tests/fixtures/compiler_contracts/generic_functions/recursion_mutual_calls_and_abstract_callback_parameters-1.panack` | 1 |
| `tests/fixtures/compiler_contracts/host_types/invalid_types_forged_construction_and_impure_clock_are_rejected-10.panack` | 1 |
| `tests/fixtures/compiler_contracts/host_types/invalid_types_forged_construction_and_impure_clock_are_rejected-11.panack` | 1 |
| `tests/fixtures/compiler_contracts/host_types/invalid_types_forged_construction_and_impure_clock_are_rejected-9.panack` | 1 |
| `tests/fixtures/compiler_contracts/host_types/opaque_values_work_in_generics_records_and_pure_calls-1.panack` | 3 |
| `tests/fixtures/compiler_contracts/local_inference/incomplete_initializers_need_annotations_now-15.panack` | 1 |
| `tests/fixtures/compiler_contracts/local_inference/incomplete_initializers_need_annotations_now-17.panack` | 1 |
| `tests/fixtures/compiler_contracts/local_inference/inferred_callables_keep_effects_in_nested_scopes-1.panack` | 3 |
| `tests/fixtures/compiler_contracts/local_inference/inferred_callables_keep_effects_in_nested_scopes-2.panack` | 1 |
| `tests/fixtures/compiler_contracts/local_inference/infers_scalars_expressions_and_preserves_numeric_defaults-1.panack` | 1 |
| `tests/fixtures/compiler_contracts/local_inference/nested_collection_and_constructor_evidence_is_order_independent-10.panack` | 1 |
| `tests/fixtures/compiler_contracts/local_inference/nested_collection_and_constructor_evidence_is_order_independent-11.panack` | 1 |
| `tests/fixtures/compiler_contracts/namespaces/aggregate_types.panack` | 1 |
| `tests/fixtures/compiler_contracts/rational_unit/invalid_conversions_arithmetic_and_void_remain_rejected-12.panack` | 1 |
| `tests/fixtures/compiler_contracts/rational_unit/invalid_conversions_arithmetic_and_void_remain_rejected-13.panack` | 1 |
| `tests/fixtures/compiler_contracts/rational_unit/invalid_conversions_arithmetic_and_void_remain_rejected-5.panack` | 1 |
| `tests/fixtures/compiler_contracts/rational_unit/invalid_conversions_arithmetic_and_void_remain_rejected-6.panack` | 1 |
| `tests/fixtures/compiler_contracts/rational_unit/invalid_conversions_arithmetic_and_void_remain_rejected-7.panack` | 1 |
| `tests/fixtures/compiler_contracts/rational_unit/rational_and_unit_types_in_generic_values_and_callbacks-1.panack` | 4 |
| `tests/fixtures/compiler_contracts/syntax/callable_diagnostics_preserve_types_and_purity-1.panack` | 1 |
| `tests/fixtures/compiler_contracts/syntax/callable_diagnostics_preserve_types_and_purity-2.panack` | 1 |
| `tests/fixtures/compiler_contracts/syntax/callable_diagnostics_preserve_types_and_purity-3.panack` | 1 |
| `tests/fixtures/compiler_contracts/syntax/callable_diagnostics_preserve_types_and_purity-4.panack` | 1 |
| `tests/fixtures/compiler_contracts/syntax/method_call_receiver_is_checked_as_argument_one-1.panack` | 1 |
| `tests/fixtures/compiler_contracts/syntax/method_calls_lower_to_calls_with_the_receiver_first-1.panack` | 7 |
| `tests/fixtures/compiler_contracts/syntax/named_callable_values_map_reduce_and_indirect_calls-1.panack` | 4 |
| `tests/fixtures/compiler_contracts/syntax/short_collection_methods_are_type_directed-1.panack` | 1 |
| `tests/fixtures/compiler_contracts/syntax/short_collection_methods_are_type_directed-2.panack` | 1 |
| `tests/fixtures/compiler_contracts/syntax/short_collection_methods_are_type_directed-3.panack` | 1 |
| `tests/fixtures/compiler_contracts/types/generic_records_option_and_result_inference-1.panack` | 1 |
| `tests/fixtures/compiler_purity/callable_effects_are_part_of_purity_checking-1.panack` | 1 |
| `tests/fixtures/compiler_purity/callable_effects_are_part_of_purity_checking-2.panack` | 1 |
| `tests/functional/failures/callable_effect/main.panack` | 1 |
| `tests/functional/failures/core_text_receiver/main.panack` | 1 |
| `tests/functional/failures/inference_empty_array/main.panack` | 1 |
| `tests/functional/failures/inference_impure_callable/main.panack` | 1 |
| `tests/functional/failures/method_collection_receiver/main.panack` | 1 |

## Quoted compiler inputs and expected text

The following hosts contain candidate receiver syntax inside string literals.
This includes deliberate compiler inputs, diagnostic excerpts, VM instruction
names rendered as source-like strings and malformed fragments. Host code itself
is migrated. In particular, CLI explanations still test legacy-call explanations,
and the rational trap case deliberately pairs legacy and owned spellings. The
P01 [generated source register](p01-source-templates.md) supplies baseline context.

| Host | Candidate string literals |
| --- | ---: |
| `src/compiler/module_bodies.panack` | 2 |
| `tests/functional/cases/cli_commands/main.panack` | 1 |
| `tests/functional/cases/cli_explanations/main.panack` | 52 |
| `tests/functional/cases/cli_rational_failures/main.panack` | 4 |
| `tests/runner/compiler_checker_unit.panack` | 7 |
| `tests/runner/compiler_explanations_unit.panack` | 12 |
| `tests/runner/compiler_instruction_sources_unit.panack` | 3 |
| `tests/runner/compiler_integration_unit.panack` | 1 |
| `tests/runner/compiler_loader_confinement_unit.panack` | 1 |
| `tests/runner/compiler_module_bindings_unit.panack` | 203 |
| `tests/runner/compiler_parser_unit.panack` | 11 |
| `tests/runner/compiler_purity_unit.panack` | 5 |
| `tests/runner/compiler_resolver_unit.panack` | 1 |
| `tests/runner/compiler_source_spans_unit.panack` | 3 |

The generated namespace seed probe in `tests/namespace_seed.sh` also contains
30 candidate quoted inputs for its raw binding/purity/parser contracts. Its
executable host calls are migrated; the quoted compatibility inputs remain for
P12 alongside the standalone compiler fixtures above.

## Other retained surfaces

The 32 source wrappers remain as listed in the [P10 register](p10-stdlib-callers.md).
The compiler still implements legacy syntax and low-level primitive identities;
its diagnostic strings, intrinsic tables and emitted VM names are implementation
data, not caller migration omissions. Core/stdlib backend primitives, explicit
printing, host/transport services and compiler path/filesystem adapters retain
existing contracts. P12 handles their internal binding and old alias removal.
Historical roadmap/design reports and bytecode/disassembly expectations remain
historical or ABI evidence; active language-tour examples use owned arrows.
