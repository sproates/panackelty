# Specification coverage

This matrix maps the behavior promised by `SPEC.md` to the automated evidence
in the unit and functional suites. It tracks behavioral protection, not merely
line coverage. Update it whenever a language promise or its tests change.

The VM guide's `vm_arithmetic`, `vm_branch_call`, and `vm_loop` examples are
discovered by the functional suite and checked from source and compiled bytecode
against their expected stdout fixtures. They add readable success-path evidence
for stack arithmetic, local storage, calls, conditional jumps, and iteration.
The guide's recorded instruction-state tables are documentation, not automated
trace assertions; they do not close the failure-coverage gaps below.

Status meanings:

- **Covered** — representative success and important failure behavior are tested.
- **Partial** — useful evidence exists, but important cases remain untested.
- **Gap** — the specified behavior has no direct automated evidence.
- **Deferred** — the specification deliberately postpones the behavior.

## Values and numeric semantics

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Arbitrary-precision `Nat` | `unit/vm/test_numeric.py::test_big_natural_arithmetic`; Project Euler 1–5 functional examples | **Covered** for large arithmetic in focused and algorithm-level programs. |
| Signed `Int` and literal inference | `unit/compiler/test_checker.py::test_int_accepts_negative_values_but_nat_does_not`; string functional example | **Partial** — add mixed `Nat`/`Int` operator and comparison cases. |
| Exact `Dec` arithmetic and scale | `unit/vm/test_numeric.py`; decimal functional example | **Covered** for addition, multiplication, finite division, large coefficients, and non-terminating division rejection. Add focused subtraction and remainder cases. |
| Integral integer division | `unit/vm/test_numeric.py::test_integer_division_stays_integral`; shared forged-runtime corpus | **Partial** — integral results and cross-VM division-by-zero traps are covered; add negative operands and remainder edge cases. |
| Proven-safe `Nat` subtraction | while-loop and guarded-fact tests in `unit/vm/test_execution.py` and `unit/compiler/test_checker.py`; shared forged-runtime corpus | **Partial** — safe source behavior and cross-VM runtime underflow traps are covered; add direct compile rejection. |
| `Bool` values | Broad unit and functional usage | **Partial** — add focused type-error and display cases. |
| Local binding inference | `unit/compiler/test_local_inference.py`; parser and emitter tests; `functional/cases/local_inference`; `functional/failures/inference_*`; release smoke | **Covered** for both frontends, numeric defaults, strings/booleans/bytes, imported constructors and functions, nested collection evidence, complete-type requirements, fixed mutable types, guarded types, callable effects, scope/order/no-shadowing, Void rejection, exact CLI diagnostics, and identical inferred/annotated emission. |
| Keyword-free functions, colon return types, local declarations, and block tails | `unit/compiler/test_syntax.py`; all functional programs | **Covered** for accepted syntax, rejection of legacy `fn`, `->`, and `let` forms, and trailing-semicolon value discard. |
| Newline statement termination and explicit semicolons | Bootstrap syntax tests; self-hosted lexer/parser tests; `semicolonless` functional case; `same_line_without_separator` failure | **Covered** for bindings, assignments, calls, imports, guarded types, blank lines, comments, block tails, multiline operators/parentheses/brackets, same-line separators, and source/bytecode execution. |
| Receiver-first method calls and callable values | Bootstrap syntax tests; self-hosted parser/resolver/checker/purity/emitter differential tests; callables and collections functional cases; public failures | **Covered** for explicit `@name` references, `PureFn`/`Fn` effects, indirect invocation, array `map`/`reduce`, lowering, chaining, typed Map/Set methods, record-field distinction, collision-free lookup, receiver/callback diagnostics, and source/bytecode execution. |
| Non-first-class `Void` and implicit fallthrough | `unit/compiler/test_syntax.py`; all functional entry points | **Covered** for empty returns, required non-`Void` results, and invalid value positions. |

## Guarded types, effects, and bindings

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Literal guard proof and rejection | `unit/compiler/test_checker.py`; `functional/failures/guard_not_proven` | **Covered** for a simple comparison guard through internal and public CLI paths. |
| Facts introduced by `if` | `unit/compiler/test_checker.py::test_guard_is_proven_by_if_fact` | **Partial** — cover compound `&&`/`||` guards, arithmetic guards, and false branches. |
| Guards remain pure and decidable | Checker implementation only | **Gap** — add rejection tests for I/O, calls, and unsupported expressions in guards. |
| Pure functions cannot call effects | `test_pure_function_cannot_print`, `test_pure_loop_cannot_hide_io`, and `functional/failures/pure_io` | **Partial** — `print` rejection reaches the public CLI; cover calls to user-defined impure functions and every effectful built-in. |
| Local mutation is allowed in pure code | for/while accumulator tests in `unit/vm/test_execution.py` | **Covered** for `mut`, assignment, `while`, and `for`. |
| Immutable locals, parameters, and bindings | `test_assignment_requires_mut`, loop-shadowing test, and `functional/failures/immutable_assignment` | **Partial** — immutable-local rejection reaches the public CLI; add parameter assignment, ordinary shadowing, and match-binding mutation cases. |
| Terminal, process, environment, path, and text file boundary | Functional outputs; `unit/vm/test_runtime.py`; public CLI host-boundary tests | **Partial** — text round trips, missing and denied paths, missing parents, invalid UTF-8, embedded-NUL rejection, arguments, environment, stderr, process exit, path operations, file existence, and verified nested execution are covered. `read_line` remains. |

## Strings, arrays, ranges, and control flow

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Concatenation and scalar interpolation | `unit/vm/test_execution.py::test_str_interpolation_concatenation_and_numeric_values`; strings functional example | **Partial** — add `Bool` and guarded-scalar interpolation and malformed interpolation tests. |
| Unicode code-point indexing, length, and reversal | `test_str_unicode_indexing_and_length`, `test_string_methods_reverse_unicode_code_points`, bounds-trap test, strings and two-pointer palindrome functional examples | **Covered** for multibyte code points, receiver-first reversal, algorithmic indexing, and out-of-bounds access. |
| Conditional expressions and optional `else` | Bootstrap and self-hosted parser/checker/emitter tests; `optional_else` success case; `if_without_else_value` public failure | **Covered** for exhaustive value branches, omitted `else` in `Void` position, discarded body values, balanced bytecode paths, and rejection as a non-`Void` result. |
| Half-open natural ranges and `for` | accumulator unit test; iterative Euler, FizzBuzz, and numeric-palindrome functional examples; `functional/failures/for_iterable_type` | **Partial** — invalid iterable rejection reaches the public CLI; add empty ranges and invalid bound-type rejection. |
| `while` checking and facts | factorial-style unit test and `functional/failures/while_condition_type` | **Partial** — non-`Bool` rejection reaches the public CLI; add additional fact shapes. |
| Homogeneous arrays, inference, iteration, length, and indexing | `test_arrays_iteration_indexing_and_len`; array bounds test; collections functional case | **Partial** — invalid index type coverage remains; local-inference tests cover heterogeneous literals and empty inferred arrays. |
| Contextually typed empty arrays | `unit/vm/test_collections.py::test_empty_array_append_and_concat_are_pure` | **Covered** for annotated construction and subsequent persistent operations. |

## Records, enums, and generics

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Record construction and field access | `unit/compiler/test_types.py::test_records_enums_and_exhaustive_match`; records functional case | **Partial** — add unknown fields, duplicate fields, bad field types, and field access on non-records. |
| Enum construction and payload binding | Same unit and functional cases; constructor arity rejection | **Partial** — add payload type and unknown-variant failures. |
| Exhaustive match | exhaustive execution and missing-arm unit tests; `functional/failures/non_exhaustive_match` | **Partial** — missing-arm rejection reaches the public CLI; add duplicate arms, incompatible result types, wrong bindings, and non-enum subjects. |
| Generic records, `Option`, and `Result` inference | `test_generic_records_option_and_result_inference`; records and option/result functional cases | **Partial** — local-inference tests cover unresolved nested arguments; expand conflicting and wrong-arity failures. |
| Erased generic bytecode representation | ADT bytecode round-trip test | **Partial** — inspect or compare emitted representation directly. |

## Persistent collections, bytes, and lexer primitives

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Persistent array operations | collection VM/compiler tests; callables and collections functional cases; callables and collections-and-bytes examples | **Partial** — `append`, `concat`, pure `map`, and accumulator-typed `reduce` cover source/bytecode and both VMs; the tour example proves an append leaves the original array unchanged. Expand callback edge cases. |
| Persistent maps and sets | `test_persistent_maps_and_sets_are_pure`; short-alias VM/compiler tests; collections, collections-and-bytes, and memoized-Fibonacci functional examples; shared forged-runtime corpus | **Partial** — legacy and concise typed methods execute in both VMs and missing lookups trap; add scalar-key restrictions, replacement behavior, and explicit Map/Set immutability checks. |
| Byte buffers and UTF-8 conversion | `test_byte_buffers_and_utf8`; collections functional case; collections-and-bytes example; shared forged-runtime corpus | **Partial** — construction, append, length, UTF-8 conversion, and invalid byte/UTF-8 traps execute in both VMs; add index bounds, concat, empty buffers, and immutability cases. |
| Binary file I/O | Shared file-I/O cases in oracle, native, and public-CLI tests | **Covered** for exact byte round trips plus missing, denied, missing-parent, and embedded-NUL path failures. |
| Lexer-oriented string built-ins | lexer foundation example and compiler-skeleton functional case | **Partial** — individually test slicing bounds, prefixes, character classes, and `nat_from_str` failures. |
| Panackelty-hosted lexer | `unit/compiler/test_self_hosted_lexer.py`; compiler-lexer and positioned-failure functional cases | **Covered** for comments, whitespace, identifiers, integers, decimals, strings, every symbol, longest-match boundaries, half-open offsets, invalid characters, file-aware positioned diagnostics, and unterminated strings. |
| Panackelty-hosted parser | `unit/compiler/test_self_hosted_parser.py`; compiler-skeleton and positioned-failure functional cases | **Covered** for the complete accepted grammar and focused malformed input, including one-based file, line, and column reporting through the project loader. |
| Panackelty-hosted name resolver | `unit/compiler/test_self_hosted_resolver.py`; compiler-skeleton and positioned-failure functional cases | **Partial** — top-level declarations, constructors, built-ins, lexical scopes, calls, assignments, loops, guards, match bindings, and already-loaded module graphs are covered, including source-accurate imported-module name failures. Interpolation references remain. |
| Panackelty-hosted type/refinement checker | `unit/compiler/test_self_hosted_checker.py`; compiler-skeleton and positioned-failure functional cases | **Covered** for declared and generic type references, records, enums, constructor inference, operators, arrays, indexing, persistent collections, built-ins, bindings, assignment, loops, branches, returns, entry points, exhaustive matches, guarded literals and branch facts, safe `Nat` subtraction, cross-module types, and source-accurate primary type failures. Differential cases compare acceptance with the bootstrap checker. |
| Panackelty-hosted purity checker | `unit/compiler/test_self_hosted_purity.py`; compiler-skeleton functional case from source and bytecode | **Covered** for pure recursion, constructors and built-ins, direct and nested impure calls, user-function effects, guarded-type predicates, and calls across an already-loaded module graph. Differential cases compare complete-frontend acceptance with the bootstrap checker. |
| Panackelty-hosted bytecode emitter | `unit/compiler/test_self_hosted_emitter.py` | **Covered** differentially against the bootstrap emitter for constants, calls, branches, short-circuiting, loops, arrays, indexing, records, variants, matches, interpolation, exact decimals, escaped strings, and deterministic temporary/jump allocation. Binary artifact tooling remains the next backend layer. |
| Panackelty-hosted bytecode tooling | `unit/bytecode/test_self_hosted_codec.py`; portable vectors in `tests/fixtures/bytecode` | **Covered** differentially for byte-identical version-8 serialization, canonical function ordering, direct and indirect calls, the complete instruction mix, scalar constants, exact numerics, records, variants, matches, and control flow. The bounded decoder, verifier, disassembler, and reserializer reject portable malformed vectors plus invalid UTF-8, flags, ordering, constants, calls, arities, and purity edges. |
| Panackelty-hosted project loader and CLI | `unit/compiler/test_self_hosted_driver.py`; `functional/test_programs.py::test_self_hosted_compiler_driver_matches_bootstrap_artifacts`; `native_conformance.sh` | **Covered** for source and bytecode `check`, `compile`, `run`, and `disasm`, bare-path execution, file-relative, project-root, and toolchain-standard-library imports, canonical load-once identity, installed resource discovery, invalid logical paths, canonical output, missing modules, cycles, byte-identical differential artifacts, and execution through the public native command. |
| Standard library and host ABI | `unit/compiler/test_stdlib.py`; `unit/vm/test_runtime.py`; `functional/cases/stdlib`; public environment functional test; `make bootstrap-check` | **Covered** for the complete prelude module graph, canonical option/result use, collection/text/byte/path APIs, checked environment access, inherited and explicit VM argument snapshots, bootstrap-stage byte-identical compilation, and source/compiled execution. |
| Testing library assertions and reporting | `unit/compiler/test_testing_library.py`; `functional/cases/testing_library` | **Partial** — empty, mixed, and failed assertions cover structured results, caller order, summary, and returned failure count in the oracle and public source/bytecode CLI. Command checks remain. |
| Testing fixture discovery and isolation | `unit/compiler/test_testing_files.py`; `functional/cases/testing_fixtures` | **Partial** — missing-root errors, sorted directory filtering, explicitly owned temporary creation, nonempty cleanup failure, and successful explicit cleanup run through the oracle and public source/bytecode CLI. Add metadata-race and non-UTF-8 fixture conformance cases. |
| Native C11 seed VM | `unit/vm/test_native_loader.py`; `unit/vm/test_native_execution.py`; portable vectors | **Covered** for strict C11 compilation, bounded decoding and verification, exhaustive truncations, shared and forged malformed artifacts, differential program output, large integers, exact decimals, collections, UTF-8, host arguments/environment/status, nested execution, forged dynamic traps, byte-identical self-hosted compilation, and running the complete compiler. |
| Reproducible native distribution | `unit/compiler/test_bootstrap.py`; `make bootstrap-check`; `unit/test_native_distribution.py`; `unit/test_layout.py`; `native_conformance.sh`; `release_archive_smoke.sh`; `quick_start.sh`; `make package` | **Covered** for seed verification, corrupt-seed rejection, stage-2/stage-3 compiler and standard-library identity, installed layout with bundled logical standard-library imports, canonical release and bytecode version reporting, Python-free conformance, and a friendly single-root archive containing the launcher, native VM, compiler seed, standard-library sources, license, user-facing release documents, and the complete tested example set. Distribution regressions also exercise functional compiler handoff, packaging, checksums, and the documented quick start from a checkout path containing spaces and parentheses, plus an installation destination with those characters. The archive structure test rejects unsafe or unexpected paths, files, ownership metadata, and platform sidecars; checksum coverage recomputes and compares the published SHA-256 digest, and a relocated packaged tour example executes with its exact expected output. The exact-artifact release gate relocates the final archive, removes development tools from `PATH`, creates its inputs outside the checkout, and verifies help, version, source checking and execution, compilation, bytecode execution, argument forwarding, standard-library discovery, and malformed-bytecode rejection. The final package gate extracts its source and expected transcript from the packaged README, then verifies checksum, install, version, check, source execution, compilation, bytecode execution, upgrade, and removal from a clean home and runtime-only `PATH`. Workflow contract coverage verifies PR and main-only push triggers, cancellation of superseded PR runs, and one canonical full-suite invocation. Native build-flag coverage checks the optimisation default, user overrides, and retained strict C11 warnings. Workflow contract coverage fixes the independent packaging matrix at Ubuntu 22.04 x86-64 and macOS 14 arm64, retains checksums and build provenance beside both archives, and rejects premature publication. The tag workflow additionally requires an exact canonical version tag, complete validation, both successful package jobs, downloaded checksum and provenance verification, and write permission isolated to final prerelease publication. |
| Public bug reporting | `.github/ISSUE_TEMPLATE/bug_report.yml`; `unit/test_layout.py`; `CONTRIBUTING.md` | **Covered** by a required GitHub issue form for version, platform, minimal source, command, expected behavior, actual output, and saved-bytecode behavior; blank issues are disabled and security reports are redirected to the private channel. |
| Public project website | `site/index.html`; `.github/workflows/pages.yml`; `unit/test_layout.py` | **Covered** by a dependency-free responsive static site whose workflow validates the exact published file set on pull requests and limits Pages deployment permissions to post-merge runs from protected `main`. |
| Short-circuit `&&` and `||` | `unit/vm/test_execution.py::test_boolean_operators_short_circuit` | **Covered** for avoiding an unsafe right-hand expression. |

## Modules, compilation, bytecode, and CLI

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| `panack` command identity and `.panack` source extension | `test_help_uses_panack_command_name`; `test_legacy_source_extension_is_rejected`; all discovered functional programs | **Covered** for help output, canonical source discovery, and rejection of the former `.nu` extension. |
| Release identity | `test_version_identifies_release_and_bytecode_format`; `unit/test_layout.py`; installed-distribution test; `native_conformance.sh` | **Covered** for one canonical semantic prerelease version, source-checkout and installed `panack --version` output, bytecode-format identity, and absence of a duplicated release literal in the launcher. |
| Relative import resolution | `unit/compiler/test_imports.py::test_relative_module_imports`; modules functional case | **Covered** for a successful relative import. |
| Logical import resolution | `unit/compiler/test_imports.py`; self-hosted driver tests; option/result and standard-library functional programs; installed-distribution test | **Covered** for canonical extensionless `stdlib/` and `project/` imports, quoted and suffixed compatibility, entry-root behavior from nested modules, load-once canonicalization, reserved standard-library ownership, invalid paths and suffixes, source/bytecode execution, and installed resource discovery. |
| Cycle detection | `unit/compiler/test_imports.py::test_import_cycles_are_rejected`; `functional/failures/import_cycle` | **Covered** for a two-module cycle through internal and public CLI paths. |
| Import validation, load-once behavior, and duplicate declarations | compiler import tests; self-hosted driver tests; `functional/failures/missing_import`, `invalid_import_suffix`, `invalid_logical_import`, and `invalid_logical_segment` | **Partial** — missing files, invalid suffixes, logical traversal and segment validation, canonical logical load-once behavior, and cycles reach focused or public CLI paths; add duplicate imported names and absolute-path functional cases. |
| Source executes only through bytecode and VM | Functional harness runs every program from source and compiled bytecode | **Covered** at the public CLI boundary. |
| Entry point and isolated call frames | Recursive Euler and memoized-Fibonacci functional examples; helper programs; `functional/failures/missing_main` and `main_parameters` | **Partial** — missing and parameterized `main` reach the public CLI; add recursion-depth and frame-isolation failures. |
| Bytecode header, version, and serialization | malformed artifact tests in `unit/bytecode/test_serialization.py`, portable vectors in `unit/bytecode/test_vectors.py`, and compiled functional programs | **Covered** for the compact version-8 binary layout, header, version, truncation, trailing data, function records, opcodes, tagged scalar values, minimal numeric encodings, canonical function ordering, repeated-compilation identity, byte-identical load/reserialize round trips, and implementation-neutral golden artifacts. |
| Verification of emitted and untrusted bytecode | source-build verification test plus adversarial operand, constant, control-flow, call, entry-point, signature, purity, resource-limit, and malformed-vector tests in `unit/bytecode` | **Covered** for compiler output entering the VM, documented structural rejection rules, portable malformed artifacts, pre-decode artifact size, and versioned count, text, numeric, and collection limits. Static stack-shape validation remains a hardening gap. |
| Frozen bytecode execution contract | `unit/bytecode/test_contract.py`; `unit/forged_runtime.py`; VM execution, numeric, collection, and runtime suites | **Covered** for operand order, isolated frames, direct/indirect calls, dynamic callable validation, return delivery, conditional stack effects, typed collection methods, Unicode reversal, and matching Python/native traps for forged failures. Every version-8 instruction and value rule is specified in `src/bytecode/FORMAT.md`. |
| `run` source, `compile`, and `run` bytecode | Functional harness | **Covered** with exact stdout assertions over all discovered programs. |
| Bare-path run shorthand | `test_bare_source_path_runs_program`; `test_compile_default_output_and_bare_bytecode_path` | **Covered** for source and bytecode input. |
| Compiler diagnostic excerpts | `unit/compiler/test_diagnostics.py`; positioned failure fixtures; CLI display edge cases; release archive smoke | **Covered** for source snapshots, imported errors, line/caret output, tabs, Unicode/control escapes, CRLF, EOF, empty lines, missing-source/invalid-position fallback, and all four source commands. |
| `check` and `disasm` commands | invalid source cases; `test_check_accepts_source_and_bytecode`; source/bytecode parity and malformed-artifact disassembly tests | **Covered** for successful source and bytecode input, exact source diagnostics, equivalent disassembly, and malformed bytecode rejection. |
| Default and explicit compile output | discovered-program compilation; `test_compile_default_output_and_bare_bytecode_path` | **Covered** for `-o`, the beside-source `.bc` default, and suppression of artifacts after invalid input. |

## Generic source functions

`unit/compiler/test_generic_functions.py` checks both frontends for inference,
explicit type arguments, lexical type scope, nested and guarded types, empty
collection evidence, argument-order independence, recursion, callable parameters,
invalid declarations, conflicting evidence, unresolved types, unused invalid
bodies, and purity. Parser round trips preserve type lists and indexing. Emitter
checks compare both compilers' disassembly and require one erased function body.

The `generic_functions` example exercises imported Option/Result/array helpers
through source and bytecode execution, and the native VM suite compares it with
the Python oracle. Functional failure fixtures cover ambiguous inference,
conflicting types, explicit type arity, abstract arithmetic, purity, and generic
main rejection. Bootstrap validation includes the generic standard-library
helpers in its stage-2/stage-3 compiler and library identity checks.

## Deliberately postponed behavior

Mutable collection elements, generic constraints and function references, explicit checked construction,
traits, package management, bytecode compatibility guarantees, and concurrency
remain deferred. Tests should be added when any of these become accepted
language behavior.

## Prioritized coverage backlog

### P0 — safety and public-contract risks

- [x] Add adversarial bytecode verifier tests for every documented structural
      rejection.
- [x] Add runtime traps for forged `Nat` underflow, invalid bytes/UTF-8, missing map
      keys, division by zero, and malformed stack behavior.
- [x] Add functional coverage for `panack check` and `panack disasm` on both success
      and failure paths.
- [x] Add text and binary file I/O round trips plus missing, denied, and invalid
      path failures.

### P1 — compiler correctness

1. Expand guarded-type, purity, name-resolution, and binding diagnostics.
2. Cover record, enum, match, and generic failure cases.
3. Cover module load-once behavior, duplicate declarations, and invalid imports.
4. Expand focused parser coverage as blocks and declarations are implemented.

### P2 — completeness and measurement

1. Fill remaining numeric, string, collection, and control-flow edge cases.
2. Add deterministic compilation and bytecode round-trip comparisons.
3. Expand the Panackelty-hosted compiler corpus beyond its current skeleton program.
4. Establish a line/branch coverage baseline for the Python bootstrap while
   keeping this behavioral matrix as the primary completeness measure.

## Validation performance regressions

- `unit/test_support.py` verifies compiled probe reuse across distinct arguments
  and a failed invocation, while requiring fresh VM execution.
- `functional/cases/string_boundaries` checks length, indexing, slicing, and
  prefix offsets on ASCII, mixed-width Unicode, combining characters, empty and
  NUL-containing strings, plus concatenated, interpolated, reversed, and decoded
  values. The ordinary functional and native-conformance discovery paths run it.
- `NativeExecutionTests.test_string_index_and_slice_boundaries_still_trap`
  retains empty, past-end, huge-index, reversed-slice, and past-end-slice traps
  across the ASCII fast path and non-ASCII traversal.

## Rational and Unit evidence

`compiler/test_rational_unit.py` checks both frontends for inference, generic
payloads, callbacks, conversions, and invalid type combinations.
`vm/test_numeric.py` and `vm/test_native_execution.py` cover normalization,
large integers, signed arithmetic, exact conversion failures, zero divisors,
and Unit behavior; seeded rational arithmetic is checked against `Fraction`.
`functional/cases/rational_unit` runs through source and compiled public CLI
paths, including collections and `Result[Unit,Str]`. Functional conversion
failure cases exercise both paths. Forged runtime fixtures check operand types
and rational remainder rejection. Codec tests check canonical, deterministic
serialization and round trips; version-8 vectors preserve legacy rejection.
Rounded decimal conversion and Rat/Unit guarded-type bases remain unsupported.

## Path, Duration, and Instant evidence

`unit/compiler/test_host_types.py` compares both frontends for opaque value
use, generic storage, forbidden constructors/fields, wrong arguments, reserved
names, and pure-clock rejection. `unit/vm/test_host_types.py` exercises native
path spellings, structured clock failure in the oracle, and verifier purity.
The shared forged-runtime corpus rejects wrong operand tags and records forged
under opaque type names in both VMs.

`functional/cases/host_types` covers empty/NUL paths, non-UTF-8 bytes, checked
text conversion, lexical append and parent operations, exact equality, signed
and very large durations, fractional nanosecond rejection, zero division,
monotonic reads, and deadline arithmetic through source and compiled public-CLI
execution and native/oracle conformance. The prelude and fixed-point bootstrap
gates include the new module. Native clock failure injection is covered by the fault harness. System
suspension and non-POSIX hosts remain uncovered; a nanosecond representation is not a clock-accuracy claim.

## Typed host capabilities

`tests/unit/vm/test_host_capabilities.py` covers structured filesystem failures,
symlinks, temporary permissions and uniqueness, raw filenames where supported,
process environment/cwd isolation, simultaneous streams, early stdin closure,
exit/signal outcomes, timing bounds, and checked decoding. Compiler host-type
tests cover operand types and purity; forged-runtime fixtures cover invalid
operands across both VMs. The `host_capabilities` and `host_process` functional
cases run source and bytecode through the public CLI and native conformance.
They include combined output exhaustion, absent executables, invalid environment
entries, negative/oversized/zero timeouts, and descendants retaining output pipes.
The native fault harness adds allocation failures and selected process, clock,
sleep and filesystem syscall failures. All errno mappings and real system
suspension remain outside this focused coverage.

## Native VM module and memory contracts

`tests/unit/vm/test_native_modules.py` compiles every native header independently
and twice to check self-containment and guards, and compares every oracle builtin's
arity and purity with the native registry while requiring a non-null handler.
Its direct C harness, `native_modules.c`, exercises copied byte/name ownership,
retained collection children, exact arithmetic without operand mutation, Unicode
offsets and invalid encodings, frame cleanup on return and trap, partial operand
underflow, nested calls, and builtin routing/error propagation.

The harness checks all truncations and 7,168 single-byte mutations of a minimal
version-8 artifact, decoding/verifying and releasing partial programs without
executing mutated code. This complements the existing structural/semantic forged
artifact corpus and native end-to-end tests. Regression assertions cover released
operands on range/index stack underflow and rejected nested bytecode cleanup. `make native-sanitize` runs the C harness
and native loader/execution suites with address and undefined-behaviour checks.

`test_native_faults.py` and its separate C harness fail every allocation position
in representative decoder, constructor, frame-growth, numeric, nested-execution,
process and file-I/O operations. They assert that tracked allocations, descriptors
and child processes return to baseline. Rich compiled programs exercise callable
values, interpolation, loops, persistent collections, variants and traps with
live caller values. Selected clock/pipe/fork/poll/read/write/open/fstat/ftruncate
failures and retryable interruptions exercise host cleanup and result contracts.
Wrappers track VM allocations and selected descriptors, not libc internals.

A valid artifact containing all 22 opcodes and all six constant forms supplies
every truncation, eight bit flips and six boundary substitutions per byte;
mutated code is decoded/verified/released, never executed. Seeded arithmetic
properties compare signed limb boundaries and 100-digit operands with Python
integers, and decimal scales through +/-4096 with exact fractions. Persistent
array versions share children across nonsequential release. Exact-output checks
and `functional/cases/vm_numeric_boundaries` cover quotient formatting and large
numeric boundaries through both runtimes and the public CLI.

Regressions found by these tests include partial-program/record cleanup, failed
frame-growth ownership, ignored expression-allocation failures, incomplete nested
argument construction, bigint temporaries and full-width unsigned conversion,
allocation-dependent decimal comparison, and decimal division trailing zeros.

CI runs `make native-sanitize` and `make native-coverage`, publishing native
line/branch summaries and HTML. The initial local baseline is approximately 85%
lines and 79% branches; this measures the native corpus, not every full-suite
execution. Host error paths, rendering and nested execution retain gaps.
Coverage-guided fuzzing, exhaustive syscall/errno combinations and unbounded
ownership sequences remain follow-up work. Passing sanitizers is evidence for
exercised paths, not proof of all memory safety.
