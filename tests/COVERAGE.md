# Specification coverage

This matrix maps the behavior promised by `SPEC.md` to the automated evidence
in the unit and functional suites. It tracks behavioral protection, not merely
line coverage. Update it whenever a language promise or its tests change.
The [testing guide](README.md) describes suite ownership and validation commands.
Planning a test does not change a coverage status.

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
| Arbitrary-precision `Nat` | `runner/vm_unit.panack`; `fixtures/vm_contracts`; Project Euler 1–5 functional examples | **Covered** for large arithmetic in focused and algorithm-level programs. |
| Signed `Int` and literal inference | `fixtures/compiler_contracts/checker`; string functional example | **Partial** — add mixed `Nat`/`Int` operator and comparison cases. |
| Exact `Dec` arithmetic and scale | `runner/vm_unit.panack`; stage-0 compile-time safeguard; decimal functional example | **Covered** for addition, multiplication, finite division, large coefficients, and non-terminating division rejection. Add focused subtraction and remainder cases. |
| Integer division produces `Rat` | `runner/vm_unit.panack`; shared VM corpus and fixed Fraction expectations | **Covered** for rational results, negative operands/remainders and division-by-zero traps. |
| Proven-safe `Nat` subtraction | while-loop and guarded-fact tests in `runner/vm_unit.panack` and `fixtures/compiler_contracts/checker`; shared forged-runtime corpus | **Partial** — safe source behavior and cross-VM runtime underflow traps are covered; add direct compile rejection. |
| `Bool` values | Broad unit and functional usage | **Partial** — add focused type-error and display cases. |
| Local binding inference | `unit/compiler/test_local_inference.py`; parser and emitter tests; `functional/cases/local_inference`; `functional/failures/inference_*`; release smoke | **Covered** for both frontends, numeric defaults, strings/booleans/bytes, imported constructors and functions, nested collection evidence, complete-type requirements, fixed mutable types, guarded types, callable effects, scope/order/no-shadowing, Void rejection, exact CLI diagnostics, and identical inferred/annotated emission. |
| Keyword-free functions, colon return types, local declarations, and block tails | `fixtures/compiler_contracts/syntax`; all functional programs | **Covered** for accepted syntax, rejection of legacy `fn`, `->`, and `let` forms, and trailing-semicolon value discard. |
| Newline statement termination and explicit semicolons | Bootstrap syntax tests; self-hosted lexer/parser tests; `semicolonless` functional case; `same_line_without_separator` failure | **Covered** for bindings, assignments, calls, imports, guarded types, blank lines, comments, block tails, multiline operators/parentheses/brackets, same-line separators, and source/bytecode execution. |
| Receiver-first method calls and callable values | Bootstrap syntax tests; self-hosted parser/resolver/checker/purity/emitter differential tests; callables and collections functional cases; public failures | **Covered** for explicit `@name` references, `PureFn`/`Fn` effects, indirect invocation, array `map`/`reduce`, lowering, chaining, typed Map/Set methods, record-field distinction, collision-free lookup, receiver/callback diagnostics, and source/bytecode execution. |
| Non-first-class `Void` and implicit fallthrough | `fixtures/compiler_contracts/syntax`; all functional entry points | **Covered** for empty returns, required non-`Void` results, and invalid value positions. |

## Guarded types, effects, and bindings

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Literal guard proof and rejection | `fixtures/compiler_contracts/checker`; `functional/failures/guard_not_proven` | **Covered** for a simple comparison guard through internal and public CLI paths. |
| Facts introduced by `if` | `fixtures/compiler_contracts/checker` | **Partial** — cover compound `&&`/`||` guards, arithmetic guards, and false branches. |
| Guards remain pure and decidable | Checker implementation only | **Gap** — add rejection tests for I/O, calls, and unsupported expressions in guards. |
| Pure functions cannot call effects | `test_pure_function_cannot_print`, `test_pure_loop_cannot_hide_io`, and `functional/failures/pure_io` | **Partial** — `print` rejection reaches the public CLI; cover calls to user-defined impure functions and every effectful built-in. |
| Local mutation is allowed in pure code | for/while accumulator tests in `runner/vm_unit.panack` | **Covered** for `mut`, assignment, `while`, and `for`. |
| Immutable locals, parameters, and bindings | `test_assignment_requires_mut`, loop-shadowing test, and `functional/failures/immutable_assignment` | **Partial** — immutable-local rejection reaches the public CLI; add parameter assignment, ordinary shadowing, and match-binding mutation cases. |
| Terminal, process, environment, path, and text file boundary | Functional outputs; `runner/host_runtime_unit.panack`; public CLI host-boundary tests | **Partial** — text round trips, missing and denied paths, missing parents, invalid UTF-8, embedded-NUL rejection, arguments, environment, stderr, process exit, path operations, file existence, and verified nested execution are covered. `read_line` remains. |

## Strings, arrays, ranges, and control flow

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Concatenation and scalar interpolation | `runner/vm_unit.panack`; strings functional example | **Partial** — add `Bool` and guarded-scalar interpolation and malformed interpolation tests. |
| Unicode code-point indexing, length, and reversal | `test_str_unicode_indexing_and_length`, `test_string_methods_reverse_unicode_code_points`, bounds-trap test, strings and two-pointer palindrome functional examples | **Covered** for multibyte code points, receiver-first reversal, algorithmic indexing, and out-of-bounds access. |
| Conditional expressions and optional `else` | Bootstrap and self-hosted parser/checker/emitter tests; `optional_else` success case; `if_without_else_value` public failure | **Covered** for exhaustive value branches, omitted `else` in `Void` position, discarded body values, balanced bytecode paths, and rejection as a non-`Void` result. |
| Half-open natural ranges and `for` | accumulator unit test; iterative Euler, FizzBuzz, and numeric-palindrome functional examples; `functional/failures/for_iterable_type` | **Partial** — invalid iterable rejection reaches the public CLI; add empty ranges and invalid bound-type rejection. |
| `while` checking and facts | factorial-style unit test and `functional/failures/while_condition_type` | **Partial** — non-`Bool` rejection reaches the public CLI; add additional fact shapes. |
| Homogeneous arrays, inference, iteration, length, and indexing | `test_arrays_iteration_indexing_and_len`; array bounds test; collections functional case | **Partial** — invalid index type coverage remains; local-inference tests cover heterogeneous literals and empty inferred arrays. |
| Contextually typed empty arrays | `runner/vm_unit.panack` | **Covered** for annotated construction and subsequent persistent operations. |

## Records, enums, and generics

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Record construction and field access | `fixtures/compiler_contracts/types`; records functional case | **Partial** — add unknown fields, duplicate fields, bad field types, and field access on non-records. |
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
| Binary file I/O | `runner/host_runtime_unit.panack`; `functional/cases/cli_environment_files` | **Covered** for exact byte round trips plus missing, denied, missing-parent, and embedded-NUL path failures through native and public CLI tests. |
| Lexer-oriented string built-ins | lexer foundation example and compiler-skeleton functional case | **Partial** — individually test slicing bounds, prefixes, character classes, and `nat_from_str` failures. |
| Panackelty-hosted lexer | `runner/compiler_lexer_unit.panack`; compiler-lexer and positioned-failure functional cases | **Covered** for comments, whitespace, identifiers, integers, decimals, strings, every symbol, longest-match boundaries, half-open offsets, invalid characters, file-aware positioned diagnostics, and unterminated strings. |
| Panackelty-hosted parser | `runner/compiler_parser_unit.panack`; compiler-skeleton and positioned-failure functional cases | **Covered** for the complete accepted grammar and focused malformed input, including one-based file, line, and column reporting through the project loader. The parser probe checks 192 assertions in 38 groups. |
| Panackelty-hosted name resolver | `runner/compiler_resolver_unit.panack`; compiler-skeleton and positioned-failure functional cases | **Partial** — top-level declarations, constructors, built-ins, lexical scopes, calls, assignments, loops, guards, match bindings, and already-loaded module graphs are covered, including source-accurate imported-module name failures. The resolver probe checks 26 assertions in 12 groups. Interpolation references remain. |
| Panackelty-hosted type/refinement checker | `runner/compiler_checker_unit.panack`; `fixtures/compiler_checker`; `fixtures/compiler_checker`; compiler-skeleton and positioned-failure functional cases | **Covered** for declared and generic type references, records, enums, constructor inference, operators, arrays, indexing, persistent collections, built-ins, bindings, assignment, loops, branches, returns, entry points, exhaustive matches, guarded literals and branch facts, safe `Nat` subtraction, cross-module types, and source-accurate primary type failures. 34 native assertions cover all former direct checker contracts, with explicit `ok` on success and the existing diagnostic substrings on failure. All 31 former differential source cases retain these same fixed native expectations. |
| Panackelty-hosted purity checker | `runner/compiler_purity_unit.panack`; `fixtures/compiler_purity`; `fixtures/compiler_purity`; compiler-skeleton functional case from source and bytecode | **Covered** for pure recursion, constructors and built-ins, direct and nested impure calls, user-function effects, guarded-type predicates, and calls across an already-loaded module graph. 11 native assertions preserve all former direct purity contracts, requiring `ok` for success and the original diagnostic substrings on failure. All ten former differential sources now use fixed native acceptance and diagnostic expectations. |
| Panackelty-hosted bytecode emitter | `runner/compiler_contracts_unit.panack` | **Covered** against fixed instruction listings for constants, calls, branches, short-circuiting, loops, arrays, indexing, records, variants, matches, interpolation, exact decimals, escaped strings, and deterministic temporary/jump allocation. |
| Panackelty-hosted bytecode tooling | `runner/bytecode_unit.panack`, `runner/bytecode_native_unit.panack`, `unit/vm/native_modules.c`, `runner/bytecode_unit.panack`; portable vectors in `tests/fixtures/bytecode` | **Covered** against fixed artifact vectors for byte-identical version-8 serialization, canonical function ordering, direct and indirect calls, the complete instruction mix, scalar constants, exact numerics, records, variants, matches, and control flow. The bounded decoder, verifier, disassembler, and reserializer reject portable malformed vectors plus invalid UTF-8, flags, ordering, constants, calls, arities, and purity edges. |
| Panackelty-hosted project loader and CLI | `runner/compiler_integration_unit.panack`; `runner/compiler_driver.panack`; `native_conformance.sh` | **Covered** for source and bytecode `check`, `compile`, `run`, and `disasm`, bare-path execution, file-relative, project-root, and toolchain-standard-library imports, canonical load-once identity, installed resource discovery, invalid logical paths, canonical output, missing modules, cycles, byte-identical differential artifacts, and execution through the public native command. |
| Standard library and host ABI | `native_oracle_contracts.sh`; `runner/host_runtime_unit.panack`; `functional/cases/stdlib`; `functional/cases/cli_environment_files`; `make bootstrap-check` | **Covered** for the complete prelude module graph, canonical option/result use, collection/text/byte/path APIs, checked environment access, inherited and explicit VM argument snapshots, bootstrap-stage byte-identical compilation, and source/compiled execution. |
| Testing library assertions and reporting | `runner/host_runtime_unit.panack`; `functional/cases/testing_library` | **Partial** — empty, mixed, and failed assertions cover structured results, caller order, summary, and returned failure count in direct native probes and public source/bytecode CLI tests. Expand malformed-report presentation cases as needed. |
| Testing fixture discovery and isolation | `runner/host_runtime_unit.panack`; `functional/cases/testing_fixtures` | **Partial** — missing-root errors, sorted directory filtering, explicitly owned temporary creation, nonempty cleanup failure, and successful explicit cleanup run through native direct checks and public source/bytecode CLI. Add metadata-race and non-UTF-8 fixture conformance cases. |
| Testing command assertions | `runner/host_runtime_unit.panack`; `functional/cases/testing_commands` | **Covered** for exit/signal and byte-stream comparison failures, host-error conversion, successful nonzero exit, expected launch and output-limit failures, and unexpected completion through direct native and public source/bytecode CLI tests. Expand timeout and invalid-UTF-8 cases with the wider host conformance backlog. |
| Panackelty-hosted fixture orchestration | `runner/main.panack`; `functional/cases/runner_smoke`; `functional/cases/cli_check_disasm`; `functional/cases/cli_commands`; `functional/cases/cli_environment_files`; `functional/cases/cli_diagnostic_display`; `functional/cases/cli_rational_failures`; `runner/compiler_driver.panack`; `unit/harness/runner.sh` | **Covered** — twenty-five selected success fixtures and twenty example programs run source, compile, and bytecode with byte-exact streams/status; forty-one failure fixtures check exact normalized diagnostics for check/compile and absent artifacts; six also check run/disasm. Source and bytecode checks, matching disassembly, malformed bytecode and legacy extension rejection, bare-path execution, default compile output, arguments, stderr/exit status, help and version, environment override, text and binary file round trips, missing paths, invalid UTF-8, and permission denial on unprivileged POSIX hosts, paired example discovery, explicit artifact and workspace cleanup, deterministic reports, injected output/diagnostic mismatches, missing fixture expectations, nonempty-workspace recovery, isolated inherited environment for `stdlib`, and checkout-bound compiler `source.path` are covered. The canonical check captures one complete runner report during the native oracle smoke execution and checks its exact bytes in both functional smoke modes; standalone targets still run the full runner. |
| Native C11 seed VM | `runner/vm_unit.panack`; `native_oracle_contracts.sh`; portable vectors | **Covered** for strict C11 compilation, bounded decoding and verification, exhaustive truncations, shared and forged malformed artifacts, differential program output, large integers, exact decimals, collections, UTF-8, host arguments/environment/status, nested execution, forged dynamic traps, byte-identical self-hosted compilation, and running the complete compiler. |
| Verified self-hosted seed refresh | `seed_refresh.sh`; `make bootstrap-check` | **Covered** for recorded input digest verification before execution, fresh verified compiler stages 2–4, compiler/library byte identity, exact conformance output, failure preservation, locks/signals/concurrent edits, publication and idempotence. A real refresh uses a restricted `PATH`; failure injection uses a shell fake VM. |
| Reproducible native distribution | `unit/harness/bootstrap.sh`; `make bootstrap-check`; `unit/harness/distribution.sh`; `unit/harness/layout.sh`; `native_conformance.sh`; `release_archive_smoke.sh`; `quick_start.sh`; `make package` | **Covered** for seed verification, corrupt-seed rejection, stage-2/stage-3 compiler and standard-library identity, installed layout with bundled logical standard-library imports, canonical release and bytecode version reporting, native conformance, and a friendly single-root archive containing the launcher, native VM, compiler seed, standard-library sources, license, user-facing release documents, and the complete tested example set. Distribution regressions also exercise functional compiler handoff, packaging, checksums, and the documented quick start from a checkout path containing spaces and parentheses, plus an installation destination with those characters. The archive structure test rejects unsafe or unexpected paths, files, ownership metadata, and platform sidecars; checksum coverage recomputes and compares the published SHA-256 digest, and a relocated packaged tour example executes with its exact expected output. The exact-artifact release gate relocates the final archive, removes development tools from `PATH`, creates its inputs outside the checkout, and verifies help, version, source checking and execution, compilation, bytecode execution, argument forwarding, standard-library discovery, and malformed-bytecode rejection. The final package gate extracts its source and expected transcript from the packaged README, then verifies checksum, install, version, check, source execution, compilation, bytecode execution, upgrade, and removal from a clean home and runtime-only `PATH`. Workflow contract coverage verifies PR and main-only push triggers, cancellation of superseded PR runs, and shared canonical suites across the validation matrix plus isolated full coverage on each packaging platform. Native build-flag coverage checks the optimisation default, user overrides, and retained strict C11 warnings. Workflow contract coverage fixes the independent packaging matrix at Ubuntu 22.04 x86-64 and macOS 14 arm64, retains checksums and build provenance beside both archives, and rejects premature publication. The tag workflow additionally requires an exact canonical version tag, complete validation, both successful package jobs, downloaded checksum and provenance verification, and write permission isolated to final prerelease publication. |
| Public bug reporting | `.github/ISSUE_TEMPLATE/bug_report.yml`; `unit/harness/layout.sh`; `CONTRIBUTING.md` | **Covered** by a required GitHub issue form for version, platform, minimal source, command, expected behavior, actual output, and saved-bytecode behavior; blank issues are disabled and security reports are redirected to the private channel. |
| Public project website | `site/index.html`; `.github/workflows/pages.yml`; `unit/harness/layout.sh` | **Covered** by a dependency-free responsive static site whose workflow validates the exact published file set on pull requests and limits Pages deployment permissions to post-merge runs from protected `main`. |
| Short-circuit `&&` and `||` | `runner/vm_unit.panack` | **Covered** for avoiding an unsafe right-hand expression. |

## Modules, compilation, bytecode, and CLI

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| `panack` command identity and `.panack` source extension | `functional/cases/cli_commands`; `functional/cases/cli_check_disasm`; all discovered functional programs | **Covered** for help output, canonical source discovery, and rejection of the former `.nu` extension. |
| Release identity | `functional/cases/cli_commands`; `unit/harness/layout.sh`; installed-distribution test; `native_conformance.sh` | **Covered** for one canonical semantic prerelease version, source-checkout and installed `panack --version` output, bytecode-format identity, and absence of a duplicated release literal in the launcher. |
| Relative import resolution | `fixtures/compiler_contracts/imports`; modules functional case | **Covered** for a successful relative import. |
| Logical import resolution | `fixtures/compiler_contracts/imports`; self-hosted driver tests; option/result and standard-library functional programs; installed-distribution test | **Covered** for canonical extensionless `stdlib/` and `project/` imports, quoted and suffixed compatibility, entry-root behavior from nested modules, load-once canonicalization, reserved standard-library ownership, invalid paths and suffixes, source/bytecode execution, and installed resource discovery. |
| Cycle detection | `fixtures/compiler_contracts/imports`; `functional/failures/import_cycle` | **Covered** for a two-module cycle through internal and public CLI paths. |
| Import validation, load-once behavior, and duplicate declarations | compiler import tests; self-hosted driver tests; `functional/failures/missing_import`, `invalid_import_suffix`, `invalid_logical_import`, and `invalid_logical_segment` | **Partial** — missing files, invalid suffixes, logical traversal and segment validation, canonical logical load-once behavior, and cycles reach focused or public CLI paths; add duplicate imported names and absolute-path functional cases. |
| Source executes only through bytecode and VM | Functional harness runs every program from source and compiled bytecode | **Covered** at the public CLI boundary. |
| Entry point and isolated call frames | Recursive Euler and memoized-Fibonacci functional examples; helper programs; `functional/failures/missing_main` and `main_parameters` | **Partial** — missing and parameterized `main` reach the public CLI; add recursion-depth and frame-isolation failures. |
| Bytecode header, version, and serialization | `runner/bytecode_unit.panack`, `runner/bytecode_native_unit.panack`, shared portable vectors and codec goldens, and compiled functional programs | **Covered** for the compact version-8 binary layout, header, version, truncation, trailing data, function records, opcodes, tagged scalar values, minimal numeric encodings, canonical function ordering, repeated-compilation identity, byte-identical load/reserialize round trips, and implementation-neutral golden artifacts. |
| Verification of emitted and untrusted bytecode | direct C `unit/vm/native_modules.c` verifier tests, native/Panackelty malformed-vector probes, and retained bootstrap-only source-build hook and limit safeguards | **Covered** for compiler output entering the VM, documented structural rejection rules, portable malformed artifacts, pre-decode artifact size, and versioned count, text, numeric, and collection limits. Static stack-shape validation remains a hardening gap. |
| Frozen bytecode execution contract | `runner/vm_unit.panack`; `fixtures/vm_contracts`; `unit/vm/native_modules.c`; native C and runtime suites | **Covered** for operand order, isolated frames, direct/indirect calls, dynamic callable validation, return delivery, conditional stack effects, typed collection methods, Unicode reversal, and fixed native traps for forged failures. Every version-8 instruction and value rule is specified in `src/bytecode/FORMAT.md`. |
| `run` source, `compile`, and `run` bytecode | Functional harness | **Covered** with exact stdout assertions over all discovered programs. |
| Bare-path run shorthand | `functional/cases/cli_commands` | **Covered** for source and bytecode input. |
| Compiler diagnostic excerpts | `fixtures/compiler_contracts/diagnostics`; positioned failure fixtures; CLI display edge cases; release archive smoke | **Covered** for source snapshots, imported errors, line/caret output, tabs, Unicode/control escapes, CRLF, EOF, empty lines, missing-source/invalid-position fallback, and all four source commands. |
| `check` and `disasm` commands | invalid source cases; `test_check_accepts_source_and_bytecode`; source/bytecode parity and malformed-artifact disassembly tests | **Covered** for successful source and bytecode input, exact source diagnostics, equivalent disassembly, and malformed bytecode rejection. |
| Default and explicit compile output | discovered-program compilation; `functional/cases/cli_commands` | **Covered** for `-o`, the beside-source `.bc` default, and suppression of artifacts after invalid input. |

## Generic source functions

`runner/compiler_contracts_unit.panack` checks fixed expectations for inference,
explicit type arguments, lexical type scope, nested and guarded types, empty
collection evidence, argument-order independence, recursion, callable parameters,
invalid declarations, conflicting evidence, unresolved types, unused invalid
bodies, and purity. Parser round trips preserve type lists and indexing. Emitter
checks compare fixed independent disassembly and require one erased function body.

The `generic_functions` example exercises imported Option/Result/array helpers
through source and bytecode execution, and the native VM suite compares it with
fixed independently reviewed output. Functional failure fixtures cover ambiguous inference,
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
4. Expand native branch coverage and keep this behavioral matrix as the
   primary completeness measure.

## Validation performance regressions

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
`runner/vm_unit.panack` and fixed numeric expectations cover normalization,
large integers, signed arithmetic, exact conversion failures, zero divisors,
and Unit behavior; seeded rational arithmetic is checked against `Fraction`.
`functional/cases/rational_unit` runs through source and compiled public CLI
paths, including collections and `Result[Unit,Str]`. Functional conversion
failure cases exercise both paths. Forged runtime fixtures check operand types
and rational remainder rejection. Codec tests check canonical, deterministic
serialization and round trips; version-8 vectors preserve legacy rejection.
Rounded decimal conversion and Rat/Unit guarded-type bases remain unsupported.

## Path, Duration, and Instant evidence

`runner/compiler_contracts_unit.panack` checks fixed expectations for opaque value
use, generic storage, forbidden constructors/fields, wrong arguments, reserved
names, and pure-clock rejection. `runner/host_runtime_unit.panack` exercises native path spellings and verifier purity.
Fixed host conformance output also runs in `native_oracle_contracts.sh`.
`unit/vm/native_faults.c` injects the native clock failure and asserts the
structured `ClockUnavailable` result.
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

`tests/runner/host_runtime_unit.panack` covers structured filesystem failures,
symlinks, temporary permissions and uniqueness, raw filenames where supported,
process environment/cwd isolation, byte-exact 128 KiB simultaneous streams, early stdin closure,
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

`tests/native_headers.sh` compiles every native header independently
and twice to check self-containment and guards. Fixed independent signatures compare every builtin's
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

`runner/vm_unit.panack` drives the separate C fault harness to fail every allocation position
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
vectors compare signed limb boundaries and 100-digit operands with independent
integer expectations, and decimal scales through +/-4096 with frozen exact
fraction results. Persistent
array versions share children across nonsequential release. Exact-output checks
and `functional/cases/vm_numeric_boundaries` cover quotient formatting and large
numeric boundaries through the native VM and the public CLI.

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

Direct compiler coverage migration is complete. The contract and integration
probes provide 201 and 51 assertions respectively; the fixture README maps every
remaining original compiler method and maps retired differential evidence to fixed expectations.
Void-valued call arguments (including nested print) are rejected in the current
self-hosted checker and refreshed seed; command regression checks preserve all
four rejection paths and ensure failed compilation creates no artifact.


Direct VM execution and loader contracts run in `tests/runner/vm_unit.panack`
against the portable corpus in `tests/fixtures/vm_contracts`. Its 174 assertions
include native module, bigint and allocation-failure wrappers; header isolation
runs in `tests/native_headers.sh`. `make native-vm-contracts` runs this group,
and `make unit`, `make check-vm`, sanitizer and coverage gates include it.
The VM corpus checks 61 fixed execution contracts, including 21
per-artifact C return-kind assertions. Fixed independent arithmetic expectations
and builtin signatures run in `make native-oracle-contracts`.

Direct host, runtime and standard-library assertions run in
`tests/runner/host_runtime_unit.panack` with reviewed source and malformed
bytecode fixtures in `tests/fixtures/host_runtime`. The probe asserts native
process, file, path, environment and timing contracts and exact testing-library
reports. Direct C host checks and forced failures run under instrumentation.
The [migration inventory](fixtures/host_runtime/README.md) maps all 37
former methods: 31 migrated to direct native evidence and the final six replaced
by fixed oracle fixtures and native/bootstrap cross-checks. Functional source and bytecode cases still verify
public behaviour on both supported platforms.

## Differential oracle retirement

Live compiler/VM comparisons are replaced by fixed native expectations, 1,800
integer and 422 decimal observations, 96 exact rational results, 82 builtin
signatures, five artifact goldens and compiler/library bootstrap identity.
All 21 successful shared VM artifacts also assert Void return kind in C.
The full case mapping and retained bootstrap-only exceptions are in
[ORACLE_REPLACEMENT.md](ORACLE_REPLACEMENT.md).

## Native development harness evidence

`make unit` includes all 31 migrated development methods through `tests/harness.sh`.
`make harness` also runs on both supported CI platforms. Shell groups
preserve repository/release/website contracts, configurable build flags, exact
archive and installation manifests, ownership and path safety, checksums, relocation,
spaced checkout/compiler paths, timer records/status, corrupt-seed rejection and
all runner report/output/fixture/cleanup/containment failures. The native supervisor
retains command timeouts and binary streams; extra tests reject signal/timeout/status
mismatches and unsafe archives. See `HARNESS_MIGRATION.md` for the complete inventory.
The final 21 implementation-only methods retired with their implementation.
`make policy` rejects source, shebang and command dependencies, with adversarial
controls. Both platforms cover the complete check, native conformance and
packaging through four clean `make check-no-interpreter CI_SUITE=…` partitions.
`tests/ci_partition.sh` checks dispatch, invalid selections, failure propagation
and shared canonical targets. The stable gates require the entire matrix;
sanitizer and coverage jobs retain their independent instrumented corpus.

Manual release initiation is covered by `unit/harness/release.sh` and workflow
contracts in `unit/harness/layout.sh`. Controls reject unconfirmed source/version,
non-main dispatch, noncanonical tags, invalid events and ambiguous changelog
notes. All three release checkouts pin the event SHA; tag identity and artifact
provenance are checked before publication. Live GitHub publication is verified
by the release workflow; local tests do not claim to exercise GitHub permissions.
The publication block also runs against disposable local Git remotes with a
stubbed GitHub CLI: real tags must be annotated, retries preserve their object,
and lightweight/wrong-commit tags prevent release creation.

Detailed profiling contracts in `unit/harness/validation.sh` cover disabled and
enabled execution, byte-exact streams (including NUL), arguments with spaces,
nonzero exits, appended rows, nested parent/run context, report-write failures
and a complete program through the public CLI. Profiling does not replace any
existing validation or change its timing budgets.


CI routing and documentation regressions in `tests/ci_scope.sh` cover the entire
PR delta, merge-base divergence, additions/deletions/renames, unknown/packaged/
policy paths, mixed changes, executable/symlink documents, unusual filenames,
missing revisions, valid and broken local links, incoming deleted-file links,
conflict markers, NUL bytes, incomplete links, and failed/cancelled/skipped
classifier outcomes. Workflow contracts retain both named package checks and
the test check, ensure their bounded result guards run unconditionally, reject failed/cancelled
execution results, and require cancellable full-route execution jobs. These tests need only shell, Git
and the existing native command allowlist; no compiler bootstrap is needed.

## Validation reuse contracts

`unit/harness/probes.sh` covers content-based probe invalidation (imports,
additions/deletions, restored timestamps, seed and VM changes), corrupt bytecode,
missing digests, failed builds, edits during compilation, paths with spaces,
execution of cached probes, compile-only copies, missing inputs, failure
propagation and cleanup. Batch tests prove concurrent execution, deterministic
stdout/stderr ordering, preservation of a failed status while all probes run,
serial execution and rejection of invalid concurrency limits.
It also proves check sessions reject ambient report paths, allocate fresh state,
clean up on success/failure and preserve command status.
`runner/report_capture_unit.panack` checks exact successful capture, absent
optional destinations, stdout/status/signal/stderr mismatches, host errors,
write failures and cleanup. Existing smoke-report and fixture-runner fault
injection remains in place. Reuse does not change any language coverage status.
