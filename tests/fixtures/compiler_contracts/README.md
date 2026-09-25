# Direct compiler contracts

These fixtures preserve the source texts and expectations of the remaining
compiler unit migration. `manifest.json` maps the original method and expanded
case to its source, expectation and check/run mode; the native probes name each
case identically. It preserves the former Python acceptance comparison inputs. `emitter/manifest.json` maps the original emitter cases (plus the
generic emission contract) to exact instruction listings. `diagnostics.json`
records the original rendering inputs and expected bytes, also stored under
`diagnostics/` so CRLF, controls and Unicode are read without string-literal
translation. `imports/manifest.json` maps copied module graphs to their original
entry points and expectations. Fixtures are reviewed data, not generated at test
time; changes must update both the inventory and corresponding native assertion.

Run `make check-compiler` from the repository root. `make unit` runs these probes
as well. The direct probe imports the current compiler; the integration probe
runs complete programs through `panack` and compiles a direct driver harness once
in an isolated temporary workspace. Both fail with status 1 on assertion failure.
Missing fixtures are failures. The integration probe checks artifact and workspace
cleanup. Independent artifact goldens added during oracle retirement live under
`../oracle_contracts/`, without changing these direct source/listing expectations.

## Source assertion inventory

| Original suite | Expanded cases | Native evidence |
|---|---:|---|
| `test_generic_functions.py` | 39 | 39 direct frontend checks, 0 execution checks |
| `test_local_inference.py` | 59 | 59 direct frontend checks, 0 execution checks |
| `test_host_types.py` | 27 | 27 direct frontend checks, 0 execution checks |
| `test_rational_unit.py` | 17 | 17 direct frontend checks, 0 execution checks |
| `test_checker.py` | 12 | 11 direct frontend checks, 1 execution checks |
| `test_syntax.py` | 31 | 24 direct frontend checks, 7 execution checks |
| `test_types.py` | 2 | 0 direct frontend checks, 2 execution checks |

There are 177 frontend checks and ten execution checks. The direct probe also
checks ten exact emitter listings and fourteen exact diagnostic renderings (201
assertions total). The integration probe adds generic execution, six import
cases, the driver contracts, the two-assertion source-snapshot helper and the
Void-argument regression (51 outer assertions). The inferred and explicit
binding emitter fixtures have identical complete listings. The generic listing
contains exactly `identity` and `main`, preserving the former function-set
assertion, and native execution must print `1` followed by `x`.

## Retired differential evidence

The 142 acceptance, ten emitter and three driver comparisons, plus the earlier
31/10 checker/purity cases, now use fixed native expectations and artifact goldens.
The old oracle passed before retirement. See `tests/ORACLE_REPLACEMENT.md`.

Two pre-existing diagnostic wording differences are explicit: the bootstrap
checker says `pure function cannot invoke an impure callable`, while the
self-hosted frontend says `pure function cannot call impure function call`;
the bootstrap loader says `logical import extension must be .panack`, while
the self-hosted loader reports `invalid logical import path`. The native probes
assert their established messages and `unit/compiler/test_bootstrap_diagnostics.py` retains both
bootstrap wording contracts alongside their self-hosted counterparts. This is
not a blanket acceptance of arbitrary rejection messages.

## Remaining Python ownership

Seed corruption, bootstrap-specific diagnostic wording, and fixture-runner
failure injection remain in `unit/compiler`. Live compiler differential tests
are retired. Standard-library byte identity now has a fixed reference artifact
and stage-1/stage-2/stage-3 comparisons.

## Per-method source mapping

Each name expands to `GROUP/METHOD-N.panack`, with ordinal starting at one.

| Original method | Cases |
|---|---:|
| `generic_functions.test_bodies_are_checked_even_when_unused` | 5 |
| `generic_functions.test_constructor_evidence_and_explicit_multiple_type_arguments` | 2 |
| `generic_functions.test_empty_evidence_and_argument_order` | 5 |
| `generic_functions.test_generic_helpers_cannot_hide_callable_effects` | 3 |
| `generic_functions.test_guarded_types_are_preserved` | 2 |
| `generic_functions.test_inference_explicit_arguments_and_lexical_type_scope` | 1 |
| `generic_functions.test_invalid_calls_and_conflicting_evidence` | 10 |
| `generic_functions.test_invalid_declarations_and_unknown_types` | 7 |
| `generic_functions.test_purity_and_generic_function_references` | 3 |
| `generic_functions.test_recursion_mutual_calls_and_abstract_callback_parameters` | 1 |
| `local_inference.test_block_scope_no_shadowing_and_declaration_order` | 9 |
| `local_inference.test_domain_types_are_preserved_but_not_invented` | 4 |
| `local_inference.test_incomplete_initializers_need_annotations_now` | 17 |
| `local_inference.test_inferred_callables_keep_effects_in_nested_scopes` | 4 |
| `local_inference.test_infers_scalars_expressions_and_preserves_numeric_defaults` | 4 |
| `local_inference.test_mutable_types_are_fixed_and_nat_facts_remain_sound` | 5 |
| `local_inference.test_nested_collection_and_constructor_evidence_is_order_independent` | 11 |
| `local_inference.test_void_and_incompatible_values_are_rejected` | 5 |
| `host_types.test_host_capability_types_and_effects` | 7 |
| `host_types.test_invalid_types_forged_construction_and_impure_clock_are_rejected` | 19 |
| `host_types.test_opaque_values_work_in_generics_records_and_pure_calls` | 1 |
| `rational_unit.test_invalid_conversions_arithmetic_and_void_remain_rejected` | 16 |
| `rational_unit.test_rational_and_unit_types_in_generic_values_and_callbacks` | 1 |
| `checker.test_assignment_requires_mut` | 1 |
| `checker.test_constructor_arguments_are_checked` | 1 |
| `checker.test_guard_accepts_proven_literal` | 1 |
| `checker.test_guard_is_proven_by_if_fact` | 1 |
| `checker.test_guard_rejects_bad_literal` | 1 |
| `checker.test_int_accepts_negative_values_but_nat_does_not` | 2 |
| `checker.test_loop_variable_cannot_overwrite_outer_binding` | 1 |
| `checker.test_match_must_be_exhaustive` | 1 |
| `checker.test_old_primitive_names_are_rejected` | 1 |
| `checker.test_pure_function_cannot_print` | 1 |
| `checker.test_pure_loop_cannot_hide_io` | 1 |
| `syntax.test_callable_diagnostics_preserve_types_and_purity` | 5 |
| `syntax.test_error_is_the_result_failure_variant` | 1 |
| `syntax.test_if_else_is_optional_in_void_position` | 1 |
| `syntax.test_if_without_else_is_void_in_value_position` | 1 |
| `syntax.test_keyword_free_functions_bindings_and_void_fallthrough` | 1 |
| `syntax.test_legacy_function_keywords_and_arrows_are_rejected` | 3 |
| `syntax.test_legacy_let_and_missing_unit_return_are_rejected` | 3 |
| `syntax.test_line_breaks_replace_statement_semicolons` | 1 |
| `syntax.test_method_call_receiver_is_checked_as_argument_one` | 1 |
| `syntax.test_method_calls_lower_to_calls_with_the_receiver_first` | 1 |
| `syntax.test_named_callable_values_map_reduce_and_indirect_calls` | 1 |
| `syntax.test_non_void_function_requires_a_result` | 1 |
| `syntax.test_parenthesized_dot_name_is_a_method_not_a_record_field_call` | 1 |
| `syntax.test_same_line_statements_still_require_a_separator` | 1 |
| `syntax.test_semicolons_still_separate_same_line_statements` | 1 |
| `syntax.test_short_collection_methods_are_type_directed` | 3 |
| `syntax.test_trailing_semicolon_discards_a_block_value` | 1 |
| `syntax.test_void_is_only_valid_as_a_return_type` | 4 |
| `types.test_generic_records_option_and_result_inference` | 1 |
| `types.test_records_enums_and_exhaustive_match` | 1 |
