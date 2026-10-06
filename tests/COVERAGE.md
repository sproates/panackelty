# Specification coverage

Guard-fact mutation regression coverage (#182) adds 25 checker fixtures: 20
rejections previously accepted by the old compiler, two review regressions for
writes during while-condition and for-iterable evaluation, plus three positive workflows
covering guarded RHS decrement, while refresh, fresh guards in a for loop and
unrelated immutable bounds. Failures cover direct/nested writes, operand and
argument evaluation, arrays/indexes, match/condition writes, loop-carried bounds,
post-loop use and guarded assignments/calls. Eight public CLI failure fixtures
and `guard_fact_refresh` exercise source checking/running and bytecode workflows.
A deterministic 48-scenario matrix adds 96 assertions: stale proofs must fail,
freshly guarded variants must pass, and all 48 accepted variants execute against
a hand-calculated result table. The focused checker runner now has 162 assertions. These tests establish this
bounded invalidation contract, not completeness of static proofs.

Website preview, release-pin, assembly and deployed-byte coverage belongs to
the [website repository](https://github.com/sproates/panackelty-website); these are separate from language execution coverage.

[Panackelty Browser](https://github.com/sproates/panackelty-browser) owns the
additional runtime and browser tests: unchanged bytecode compatibility, native
compiler equivalence, exact values, Unicode, source errors, explicit unavailable
hosts, UTF-8 input/output bounds, memory growth rejection and worker lifecycle.
Its declared host restrictions are not new native language semantics. Browser
engine checks supplement the native evidence below; physical iPhone validation
remains distinct from automated WebKit tests.
The website repository runs the pinned downstream integration suite against
its assembled artifact. Core does not duplicate website test sources.
Asset identity tests cover every staged file and deterministic ordering. Real
browser tests warm an HTTP cache, switch deployments, reload and verify matching
example/library execution with versioned module, worker and binary requests.
The guides' expected output is checked through WASI and the native public CLI.
Suggested invoice edits must recalculate correctly and an invalid guarded value
must fail. Browser checks ensure each loaded program's guide matches its selection.

This matrix maps the behavior promised by `SPEC.md` to the automated evidence
in the unit and functional suites. It tracks behavioral protection, not merely
line coverage. Update it whenever a language promise or its tests change.
The [testing guide](README.md) describes suite ownership and validation commands.
Planning a test does not change a coverage status.

Measured coverage is published separately for the [native C VM](https://sproates.github.io/panackelty-coverage/)
from `main` and the [production `.panack` source](https://sproates.github.io/panackelty-coverage/source/html/index.html)
from `next`. The first verified source baseline covers all 38 tracked production
compiler, bytecode and standard-library files across 30 fresh executions: lines
86.22%, functions 89.85%, source outcomes 79.57%, no unavailable measurements.
The [baseline contract](../docs/SOURCE_COVERAGE_BASELINE.md) defines the exact
initial corpus, exclusions, identities and completeness rules. Lines are unique
eligible expression/statement start lines; branches are original-source outcomes,
not native or VM instruction branches. The corpus does not include every canonical
test or browser/WASI execution. SC5 is accepted; bounded gap closure and regression
policy remain SC6 under [RM#46 / GI#131](../ROADMAP.md#rm-46).
Execution coverage does not establish assertion quality. Do not interpret this
behavior matrix, test counts or C percentages as a `.panack` coverage percentage.

The matrix is undergoing evidence reconciliation: older rows contain historical
test names and runtime descriptions. Check the current probes and migrated
fixtures before treating a “Gap” or “Partial” label as verified missing tests.

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
| Local binding inference | `runner/compiler_contracts_unit.panack`; `fixtures/compiler_contracts` local-inference cases; parser and emitter probes; `functional/cases/local_inference`; `functional/failures/inference_*`; release smoke | **Covered** by migrated frontend contracts for numeric defaults, strings/booleans/bytes, imported constructors and functions, nested collection evidence, complete-type requirements, fixed mutable types, guarded types, callable effects, scope/order/no-shadowing, Void rejection, exact CLI diagnostics, and identical inferred/annotated emission. |
| Keyword-free functions, colon return types, local declarations, and block tails | `fixtures/compiler_contracts/syntax`; all functional programs | **Covered** for accepted syntax, rejection of legacy `fn`, `->`, and `let` forms, and trailing-semicolon value discard. |
| Newline statement termination and explicit semicolons | Bootstrap syntax tests; self-hosted lexer/parser tests; `semicolonless` functional case; `same_line_without_separator` failure | **Covered** for bindings, assignments, calls, imports, guarded types, blank lines, comments, block tails, multiline operators/parentheses/brackets, same-line separators, and source/bytecode execution. |
| Receiver-first method calls and callable values | Bootstrap syntax tests; self-hosted parser/resolver/checker/purity/emitter differential tests; callables and collections functional cases; public failures | **Covered** for explicit `@name` references, `PureFn`/`Fn` effects, indirect invocation, array `map`/`reduce`, lowering, chaining, typed Map/Set methods, record-field distinction, collision-free lookup, receiver/callback diagnostics, and source/bytecode execution. |
| Staged namespace body call contracts | `runner/compiler_module_bindings_unit.panack` (`namespace_body_tests`, `namespace_aggregate_tests`) | **Partial** — slice 3 checks identity-bound indirect calls, `len`, core array methods and operations, and Map/Set receivers, including reserved core identity and invalid argument/result cases. Guard proofs, effects/await, emission and executable namespace migration remain deferred. |
| Non-first-class `Void` and implicit fallthrough | `fixtures/compiler_contracts/syntax`; all functional entry points | **Covered** for empty returns, required non-`Void` results, and invalid value positions. |

## Guarded types, effects, and bindings

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Literal guard proof and rejection | `fixtures/compiler_contracts/checker`; `functional/failures/guard_not_proven` | **Covered** for a simple comparison guard through internal and public CLI paths. |
| Facts introduced by `if` | `fixtures/compiler_contracts/checker` | **Partial** — cover compound `&&`/`||` guards, arithmetic guards, and false branches. |
| Guards remain pure and decidable | `runner/compiler_purity_unit.panack`; `fixtures/compiler_purity` guard cases; checker contracts | **Partial** — guard I/O rejection is already tested. Audit supported/unsupported predicates and call cases before specifying the remaining rejection tests. |
| Pure functions cannot call effects | `test_pure_function_cannot_print`, `test_pure_loop_cannot_hide_io`, and `functional/failures/pure_io` | **Partial** — `print` rejection reaches the public CLI; cover calls to user-defined impure functions and every effectful built-in. |
| Local mutation is allowed in pure code | for/while accumulator tests in `runner/vm_unit.panack` | **Covered** for `mut`, assignment, `while`, and `for`. |
| Immutable locals, parameters, and bindings | `test_assignment_requires_mut`, loop-shadowing test, and `functional/failures/immutable_assignment` | **Partial** — immutable-local rejection reaches the public CLI; add parameter assignment, ordinary shadowing, and match-binding mutation cases. |
| Terminal, process, environment, path, and text file boundary | Functional outputs; `runner/host_runtime_unit.panack`; public CLI host-boundary tests | **Partial** — text round trips, missing and denied paths, missing parents, invalid UTF-8, embedded-NUL rejection, arguments, environment, stderr, process exit, path operations, file existence, and verified nested execution are covered. `read_line` remains. |

## Strings, arrays, ranges, and control flow

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| Concatenation and scalar interpolation | `runner/vm_unit.panack`; strings functional example | **Partial** — add `Bool` and guarded-scalar interpolation and malformed interpolation tests. |
| Unicode code-point indexing, length, and reversal | `test_str_unicode_indexing_and_length`, `test_string_methods_reverse_unicode_code_points`, bounds-trap test, strings and two-pointer palindrome functional examples | **Covered** for multibyte code points, receiver-first reversal, algorithmic indexing, and out-of-bounds access. |
| Conditional expressions, else-if chains and optional `else` | Bootstrap and self-hosted parser/checker/emitter tests; `optional_else` success case; `if_without_else_value` public failure | **Covered** for exhaustive/chained value branches, repeated partial chains, ordered/skipped conditions and bodies, mixed discarded arm values, source spans, async arms, namespace checking, balanced bytecode paths and rejection as a non-`Void` result. |
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
| Persistent array operations | collection VM/compiler tests; callables and collections functional cases; callables and collections-and-bytes examples | **Partial** — `append`, `concat`, pure `map`, and accumulator-typed `reduce` cover source/bytecode and both VMs; the tour example proves an append leaves the original array unchanged. Direct native contracts additionally cover shared-prefix release order, prompt hidden-child reclamation, branching, geometric growth, nested collection/iterator aliases, bounded inspection and size overflow; allocation sweeps exercise each append failure site. The collections program checks retained snapshots, append during iteration and nested arrays/records through the public CLI. Expand callback edge cases. |
| Persistent maps and sets | `test_persistent_maps_and_sets_are_pure`; short-alias VM/compiler tests; collections, collections-and-bytes, and memoized-Fibonacci functional examples; shared forged-runtime corpus | **Partial** — legacy and concise typed methods execute in both VMs and missing lookups trap; add scalar-key restrictions, replacement behavior, and explicit Map/Set immutability checks. |
| Byte buffers and UTF-8 conversion | `test_byte_buffers_and_utf8`; collections functional case; collections-and-bytes example; shared forged-runtime corpus | **Partial** — construction, append, length, UTF-8 conversion, and invalid byte/UTF-8 traps execute in both VMs; add index bounds, concat, empty buffers, and immutability cases. |
| Binary file I/O | `runner/host_runtime_unit.panack`; `functional/cases/cli_environment_files` | **Covered** for exact byte round trips plus missing, denied, missing-parent, and embedded-NUL path failures through native and public CLI tests. |
| Lexer-oriented string built-ins | lexer foundation example and compiler-skeleton functional case | **Partial** — individually test slicing bounds, prefixes, character classes, and `nat_from_str` failures. |
| Panackelty-hosted lexer | `runner/compiler_lexer_unit.panack`; compiler-lexer and positioned-failure functional cases | **Covered** for comments, whitespace, identifiers, integers, decimals, strings, every symbol, longest-match boundaries, half-open offsets, invalid characters, file-aware positioned diagnostics, and unterminated strings. |
| Panackelty-hosted parser | `runner/compiler_parser_unit.panack`; compiler-skeleton and positioned-failure functional cases | **Covered** for the complete accepted grammar and focused malformed input, including one-based file, line, and column reporting through the project loader. The parser probe checks 192 assertions in 38 groups. |
| Panackelty-hosted name resolver | `runner/compiler_resolver_unit.panack`; compiler-skeleton and positioned-failure functional cases | **Partial** — top-level declarations, constructors, built-ins, lexical scopes, calls, assignments, loops, guards, match bindings, and already-loaded module graphs are covered, including source-accurate imported-module name failures. The resolver probe checks 26 assertions in 12 groups. Interpolation references remain. |
| Panackelty-hosted type/refinement checker | `runner/compiler_checker_unit.panack`; `fixtures/compiler_checker`; `fixtures/compiler_checker`; compiler-skeleton and positioned-failure functional cases | **Covered** for declared and generic type references, records, enums, constructor inference, operators, arrays, indexing, persistent collections, built-ins, bindings, assignment, loops, branches, returns, entry points, exhaustive matches, guarded literals and branch facts, safe `Nat` subtraction, cross-module types, and source-accurate primary type failures. 37 native assertions cover the former direct checker contracts and sorting callback contracts, with explicit `ok` on success and the existing diagnostic substrings on failure. All 31 former differential source cases retain these same fixed native expectations. |
| Panackelty-hosted purity checker | `runner/compiler_purity_unit.panack`; `fixtures/compiler_purity`; `fixtures/compiler_purity`; compiler-skeleton functional case from source and bytecode | **Covered** for pure recursion, constructors and built-ins, direct and nested impure calls, user-function effects, guarded-type predicates, and calls across an already-loaded module graph. 11 native assertions preserve all former direct purity contracts, requiring `ok` for success and the original diagnostic substrings on failure. All ten former differential sources now use fixed native acceptance and diagnostic expectations. |
| Panackelty-hosted bytecode emitter | `runner/compiler_contracts_unit.panack` | **Covered** against fixed instruction listings for constants, calls, branches, short-circuiting, loops, arrays, indexing, records, variants, matches, interpolation, exact decimals, escaped strings, and deterministic temporary/jump allocation. |
| Panackelty-hosted bytecode tooling | `runner/bytecode_unit.panack`, `runner/bytecode_native_unit.panack`, `unit/vm/native_modules.c`, `runner/bytecode_unit.panack`; portable vectors in `tests/fixtures/bytecode` | **Covered** against fixed artifact vectors for byte-identical version-8 serialization, canonical function ordering, direct and indirect calls, the complete instruction mix, scalar constants, exact numerics, records, variants, matches, and control flow. The bounded decoder, verifier, disassembler, and reserializer reject portable malformed vectors plus invalid UTF-8, flags, ordering, constants, calls, arities, and purity edges. |
| Panackelty-hosted project loader and CLI | `runner/compiler_integration_unit.panack`; `runner/compiler_driver.panack`; `native_conformance.sh` | **Covered** for source and bytecode `check`, `compile`, `run`, and `disasm`, bare-path execution, file-relative, project-root, and toolchain-standard-library imports, canonical load-once identity, installed resource discovery, invalid logical paths, canonical output, missing modules, cycles, byte-identical differential artifacts, and execution through the public native command. |
| Internal resumable VM | `unit/vm/resumable.c`; `unit/vm/native_faults.c`; `functional/cases/callables` | **Covered** for zero/single/multiple instruction budgets, sticky completion/trap/exit, argument/result ownership, direct/indirect calls, independent interleaved VMs, array/byte/range iterators, frame-array growth and deep CLI recursion, suspended destruction, fake-host re-entry/failure, and rejection of unsupported/nested host services. Allocation sweeps cover creation, return, trap and suspended cleanup. The immediate adapter has no pending I/O. |
| Internal task lifecycle | `unit/vm/tasks.c`; `unit/vm/native_faults.c`; `runner/vm_unit.panack` | **Covered** for scoped and nested joins, child failure/sibling cancellation, cancellation before/during/after waits and queued delivery, inherited virtual deadlines, round-robin progress, bounded admissions/queue retry, stale/wrong-session/duplicate completions, destruction with pending work and allocation failures. Three verified bytecode replays preserve independent output oracles. Fake print acknowledgements only; no OS backend, source async semantics or production scheduler scalability claim. |
| Sorting and literal suffix helpers | `runner/stdlib_unit.panack`; `runner/compiler_checker_unit.panack`; `functional/cases/collections`; `examples/collections_and_bytes.panack` | **Covered** for stable record sorting, equivalent keys, empty/singleton/odd-sized/already sorted inputs, descending order, exact large naturals, input preservation, invalid comparator effects/result type, Unicode code-point suffix boundaries, empty/long suffixes, literal punctuation, case sensitivity and no normalization. Source and saved bytecode execute through the CLI; `.panack` execution coverage remains unmeasured. |
| Standard library and host ABI | `native_oracle_contracts.sh`; `runner/host_runtime_unit.panack`; `functional/cases/stdlib`; `functional/cases/cli_environment_files`; `make bootstrap-check` | **Covered** for the complete prelude module graph, canonical option/result use, collection/text/byte/path APIs, checked environment access, inherited and explicit VM argument snapshots, bootstrap-stage byte-identical compilation, and source/compiled execution. |
| Testing library assertions and reporting | `runner/host_runtime_unit.panack`; `functional/cases/testing_library` | **Partial** — empty, mixed, and failed assertions cover structured results, caller order, summary, and returned failure count in direct native probes and public source/bytecode CLI tests. Expand malformed-report presentation cases as needed. |
| Testing fixture discovery and isolation | `runner/host_runtime_unit.panack`; `functional/cases/testing_fixtures` | **Partial** — missing-root errors, sorted directory filtering, explicitly owned temporary creation, nonempty cleanup failure, and successful explicit cleanup run through native direct checks and public source/bytecode CLI. Add metadata-race and non-UTF-8 fixture conformance cases. |
| Testing command assertions | `runner/host_runtime_unit.panack`; `functional/cases/testing_commands` | **Covered** for exit/signal and byte-stream comparison failures, host-error conversion, successful nonzero exit, expected launch and output-limit failures, and unexpected completion through direct native and public source/bytecode CLI tests. Expand timeout and invalid-UTF-8 cases with the wider host conformance backlog. |
| Panackelty-hosted fixture orchestration | `runner/main.panack`; `functional/cases/runner_smoke`; `functional/cases/cli_check_disasm`; `functional/cases/cli_commands`; `functional/cases/cli_environment_files`; `functional/cases/cli_diagnostic_display`; `functional/cases/cli_rational_failures`; `runner/compiler_driver.panack`; `unit/harness/runner.sh` | **Covered** — twenty-seven selected success fixtures and twenty example programs run source, compile, and bytecode with byte-exact streams/status; forty-seven failure fixtures check exact normalized diagnostics for check/compile and absent artifacts; six also check run/disasm. Source and bytecode checks, matching disassembly, malformed bytecode and legacy extension rejection, bare-path execution, default compile output, arguments, stderr/exit status, help and version, environment override, text and binary file round trips, missing paths, invalid UTF-8, and permission denial on unprivileged POSIX hosts, paired example discovery, explicit artifact and workspace cleanup, deterministic reports, injected output/diagnostic mismatches, missing fixture expectations, nonempty-workspace recovery, isolated inherited environment for `stdlib`, and checkout-bound compiler `source.path` are covered. The canonical check captures one complete runner report during the native oracle smoke execution and checks its exact bytes in both functional smoke modes; standalone targets still run the full runner. |
| Native C11 seed VM | `runner/vm_unit.panack`; `native_oracle_contracts.sh`; portable vectors | **Covered** for strict C11 compilation, bounded decoding and verification, exhaustive truncations, shared and forged malformed artifacts, differential program output, large integers, exact decimals, collections, UTF-8, host arguments/environment/status, nested execution, forged dynamic traps, byte-identical self-hosted compilation, and running the complete compiler. |
| Verified self-hosted seed refresh | `seed_refresh.sh`; `make bootstrap-check` | **Covered** for recorded input digest verification before execution, fresh verified compiler stages 2–4, compiler/library byte identity, exact conformance output, failure preservation, locks/signals/concurrent edits, publication and idempotence. A real refresh uses a restricted `PATH`; failure injection uses a shell fake VM. |
| Reproducible native distribution | `unit/harness/bootstrap.sh`; `make bootstrap-check`; `unit/harness/distribution.sh`; `unit/harness/layout.sh`; `native_conformance.sh`; `release_archive_smoke.sh`; `quick_start.sh`; `make package` | **Covered** for seed verification, corrupt-seed rejection in an isolated checkout with shared VM inode/byte preservation, stage-2/stage-3 compiler and standard-library identity, installed layout with bundled logical standard-library imports, canonical release and bytecode version reporting, native conformance, and a friendly single-root archive containing the launcher, native VM, compiler seed, standard-library sources, license, user-facing release documents, and the complete tested example set. Distribution regressions also exercise functional compiler handoff, packaging, checksums, and the documented quick start from a checkout path containing spaces and parentheses, plus an installation destination with those characters. The archive structure test rejects unsafe or unexpected paths, files, ownership metadata, and platform sidecars; checksum coverage recomputes and compares the published SHA-256 digest, and a relocated packaged tour example executes with its exact expected output. The exact-artifact release gate relocates the final archive, removes development tools from `PATH`, creates its inputs outside the checkout, and verifies help, version, source checking and execution, compilation, bytecode execution, argument forwarding, standard-library discovery, and malformed-bytecode rejection. The final package gate extracts its source and expected transcript from the packaged README, then verifies checksum, install, version, check, source execution, compilation, bytecode execution, upgrade, and removal from a clean home and runtime-only `PATH`. Workflow contract coverage verifies PR and main-only push triggers, cancellation of superseded PR runs, and shared canonical suites across the validation matrix plus isolated full coverage on each packaging platform. Native build-flag coverage checks the optimisation default, user overrides, and retained strict C11 warnings. Workflow contract coverage fixes the independent packaging matrix at Ubuntu 22.04 x86-64 and macOS 14 arm64, retains checksums and build provenance beside both archives, and rejects premature publication. The tag workflow additionally requires an exact canonical version tag, complete validation, both successful package jobs, downloaded checksum and provenance verification, and write permission isolated to final prerelease publication. |
| Public bug reporting | `.github/ISSUE_TEMPLATE/bug_report.yml`; `unit/harness/layout.sh`; `CONTRIBUTING.md` | **Covered** by a required GitHub issue form for version, platform, minimal source, command, expected behavior, actual output, and saved-bytecode behavior; blank issues are disabled and security reports are redirected to the private channel. |
| Public project website | [website repository](https://github.com/sproates/panackelty-website) | Independently validated and published; no core website test dependency. |
| Short-circuit `&&` and `||` | `runner/vm_unit.panack` | **Covered** for avoiding an unsafe right-hand expression. |

## Modules, compilation, bytecode, and CLI

| Behavior | Evidence | Status and remaining work |
| --- | --- | --- |
| `panack` command identity and `.panack` source extension | `functional/cases/cli_commands`; `functional/cases/cli_check_disasm`; all discovered functional programs | **Covered** for help output, canonical source discovery, and rejection of the former `.nu` extension. |
| Release identity | `functional/cases/cli_commands`; `unit/harness/layout.sh`; installed-distribution test; `native_conformance.sh` | **Covered** for one canonical semantic prerelease version, source-checkout and installed `panack --version` output, bytecode-format identity, and absence of a duplicated release literal in the launcher. |
| Relative import resolution | `fixtures/compiler_contracts/imports`; modules functional case | **Covered** for a successful relative import. |
| Logical import resolution | `fixtures/compiler_contracts/imports`; self-hosted driver tests; option/result and standard-library functional programs; installed-distribution test | **Covered** for canonical extensionless `stdlib/` and `project/` imports, quoted and suffixed compatibility, entry-root behavior from nested modules, load-once canonicalization, reserved standard-library ownership, invalid paths and suffixes, source/bytecode execution, and installed resource discovery. |
| Namespace resolution (P2 intermediate) | `runner/compiler_module_bindings_unit.panack`; namespace fixtures through `runner/compiler_integration_unit.panack`; `functional/cases/cli_diagnostic_display` | **Covered** for raw dot calls, qualified types/references/patterns and nested type leaves; namespace/selective imports and re-exports retaining original function/nominal/variant IDs; private access and transitive invisibility; import/local/type-parameter collisions; shared diamond traversal, missing/duplicate/cyclic graphs; source spans and fail-closed parser/checker/purity/emitter/codec/public CLI paths, including unmarked qualified uses and absent bytecode output. Existing declaration-origin tests remain. **Covered** staged declaration/signature identities: original-ID public reachability, nested generic/array/callable privacy, binder-position substitution, core aliases, callable effects and guard helper access including receiver methods and enum re-exports. Public CLI privacy failures remain positioned and write no bytecode. **Covered** identity-bound body structure, lexical binder ownership/assignment reuse, annotations and known Void rejection, direct/qualified/helper argument and return compatibility, same-spelled nominal distinction, explicit generic binder substitution, callable effect identity, reserved method/direct-call distinction, retained constructor/core-alias identities and explicit pending core/proof checks. Public CLI nominal-body failures retain source ownership and write no bytecode. **Covered** ordinary scalar operators, Bool conditions and branches, typed iterable/index paths, lexical interpolation (ordered/repeated references, escapes and invalid scopes), known Void failures and invalid/deferred/checked-subset status. Nat subtraction and mixed Rat/integer joins retain explicit obligations; nested child obligations are not erased by known result types. `functional/cases/expression_contracts` checks exact results, loops, interpolation and short-circuit traps through source and bytecode; public CLI fixtures check positioned failures and the unchanged execution gate. **Covered** identity-based record/variant construction, nested arrays and branch evidence, nominal field lookup, payload types/immutability, exhaustive/duplicate variant identities, argument-only generic inference, rigid caller parameters and explicit type arguments. Shared expression holes retain contextual constraints across calls and statements; conflicting/recursive evidence rejects, inferred locals cannot borrow later evidence, and unresolved retained children remain deferred. `functional/cases/aggregate_inference` checks legacy source/saved-bytecode behavior, while namespace CLI fixtures preserve positioned errors and absent artifacts. **Pending** general core/indirect-call contracts, guard proofs, effect/emission/tooling migration, coordinated source migration and native/browser namespace execution. Whole-body constraint errors use the enclosing original function span. Type/pattern use locations currently fall back to the declaration/match span. |
| Cycle detection | `fixtures/compiler_contracts/imports`; `functional/failures/import_cycle` | **Covered** for a two-module cycle through internal and public CLI paths. |
| Import validation, load-once behavior, and duplicate declarations | compiler import tests; self-hosted driver tests; `functional/failures/missing_import`, `invalid_import_suffix`, `invalid_logical_import`, and `invalid_logical_segment` | **Partial** — missing files, invalid suffixes, logical traversal and segment validation, canonical logical load-once behavior, and cycles reach focused or public CLI paths; add duplicate imported names and absolute-path functional cases. |
| Source executes only through bytecode and VM | Functional harness runs every program from source and compiled bytecode | **Covered** at the public CLI boundary. |
| Entry point and isolated call frames | Recursive Euler and memoized-Fibonacci functional examples; helper programs; `functional/failures/missing_main` and `main_parameters` | **Partial** — missing and parameterized `main` reach the public CLI; add recursion-depth and frame-isolation failures. |
| Bytecode header, version, and serialization | `runner/bytecode_unit.panack`, `runner/bytecode_native_unit.panack`, shared portable vectors and codec goldens, and compiled functional programs | **Covered** for the compact version-8 binary layout, header, version, truncation, trailing data, function records, opcodes, tagged scalar values, minimal numeric encodings, canonical function ordering, repeated-compilation identity, byte-identical load/reserialize round trips, and implementation-neutral golden artifacts. |
| Verification of emitted and untrusted bytecode | direct C `unit/vm/native_modules.c` verifier tests, native/Panackelty malformed-vector probes, and retained bootstrap-only source-build hook and limit safeguards | **Covered** for compiler output entering the VM, documented structural rejection rules, portable malformed artifacts, pre-decode artifact size, and versioned count, text, numeric, and collection limits. Static stack-shape validation remains a hardening gap. |
| Frozen bytecode execution contract | `runner/vm_unit.panack`; `fixtures/vm_contracts`; `unit/vm/native_modules.c`; native C and runtime suites | **Covered** for operand order, isolated frames, direct/indirect calls, dynamic callable validation, return delivery, conditional stack effects, typed collection methods, Unicode reversal, and fixed native traps for forged failures. Every version-9 instruction and value rule is specified in `src/bytecode/FORMAT.md`. |
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

Mutable collection elements, generic constraints, explicit checked construction,
traits, package management and bytecode compatibility guarantees remain deferred.
Named function references, async/await and finite concurrent TCP servers are
implemented; general source spawning, resource scopes and parallel execution
remain outside that delivered scope. Extend tests when a postponed feature
becomes accepted language behaviour.

## Coverage backlog: groomed candidates

### Measurement and evidence reconciliation — candidate

1. Design and implement `.panack` source mapping, instrumentation and report
   aggregation, including compiler execution, nested VMs and subprocesses.
2. Establish line/function and branch baselines with explicit scope, eligible
   unexecuted files, exclusions, source identity and collection-failure handling.
3. Publish `.panack` results separately from native C coverage; define regression
   policy only after validating the denominator and measurement correctness.
4. Reconcile matrix rows against current probes and fixtures, including migrated
   names, already-tested negative cases and retired-runtime references. Preserve
   historical provenance in fixture inventories; do not relabel it as active code.

Known correctness or safety defects preempt measurement work. High percentages
are not an acceptance substitute for assertions, negative tests or invariants.
The [systematic invariant-testing candidate](../ROADMAP.md#systematic-invariant-testing)
extends existing deterministic, source/bytecode and bootstrap checks with one
bounded generated-input or semantics-preserving transformation family. It does
not depend on source-coverage publication. Planning adds no new test evidence;
individual matrix rows retain their evidence status until verified.

### Delivered safety and public-contract foundation

- [x] Add adversarial bytecode verifier tests for every documented structural
      rejection.
- [x] Add runtime traps for forged `Nat` underflow, invalid bytes/UTF-8, missing map
      keys, division by zero, and malformed stack behavior.
- [x] Add functional coverage for `panack check` and `panack disasm` on both success
      and failure paths.
- [x] Add text and binary file I/O round trips plus missing, denied, and invalid
      path failures.

### Compiler correctness — candidate

1. Expand guarded-type, purity, name-resolution, and binding diagnostics.
2. Cover record, enum, match, and generic failure cases.
3. Cover module load-once behavior, duplicate declarations, and invalid imports.
4. Expand parser edge cases against the implemented grammar; blocks and
   declarations already have direct probes.
5. Check assertion quality for selected high-risk cases; evaluate bounded
   mutation or perturbed fixtures, with cost measured before broader adoption.

### Further completeness and hardening — candidate

1. Fill remaining numeric, string, collection, and control-flow edge cases.
2. Extend existing deterministic-compilation and canonical bytecode round-trip
   comparisons only where new edge cases are identified.
3. Expand the direct lexer/parser/resolver/checker/purity/emitter/driver probes
   against measured and behavior-specific gaps; the compiler corpus is already
   substantially broader than its historical skeleton program.
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
line/branch summaries and HTML. The [public report](https://sproates.github.io/panackelty-coverage/)
is published by the independent `panackelty-coverage` repository from successful
trusted `main` validation, with source identity and archive date. That repository
owns report-selection/navigation and full deployed-report verification tests.
Website assembly, compatibility landing pages and live asset verification
now belong to the website repository. The independent production publisher
passed initial live verification in run `36933404078`. PR #203 website cutover
passed live byte/MIME/provenance verification in Pages `36935939432`; both old
entry points match the merged landing, which was visually inspected. Manual
coverage refresh `36938575919` advanced the independent report to core `5381bc5`
and verified every file without another website publication. Scheduled-event
delivery and core-only production no-op evidence remain open under #187.
This publication does not add language or native branch coverage.
The initial local baseline is approximately 85%
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
against the portable corpus in `tests/fixtures/vm_contracts`. Its 177 assertions
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
packaging through five clean `make check-no-interpreter CI_SUITE=…` partitions.
`tests/ci_partition.sh` checks dispatch, invalid selections, failure propagation
and shared canonical targets. The stable gates require the entire matrix;
sanitizer and coverage jobs retain their independent instrumented corpus.
Sanitizer CI partitions VM contracts, ordinary oracle cases, and the nested
functional runner. `tests/ci_sanitize.sh` compares standalone and partitioned
operation multiplicity and injects compile, verify, execution, stderr and output
failures; invalid selections fail before executing the VM. The standalone
sanitizer and coverage targets execute all three shared implementations.
Distribution regressions also retain the shared VM inode and bytes while
building and installing from an isolated checkout, detecting accidental relinks
that can disrupt concurrent compiler commands.

`tests/ci_partition.sh` executes the actual canonical unit recipe with stubbed
suites at one, two and three workers. It checks exact suite multiplicity, worker
allocation, concurrent harness/compiler starts, runtime ordering after both
succeed, and failure propagation from each suite. Bounded marker waits reject
a serial regression without hanging the test.

Runtime scheduling controls inject failures into each concurrent branch and
use a FIFO rendezvous to check overlap without relying on elapsed timing.
`tests/ci_conformance.sh` proves that the source and bytecode partitions, with serial and parallel workers, together
perform the standalone conformance observations. Controls reject failed commands,
unexpected output/stderr, accepted negative cases and invalid mode selections.

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
audited process and unlisted policy paths, mixed changes, executable/symlink documents, unusual filenames,
missing revisions, valid and broken local links, incoming deleted-file links,
conflict markers, NUL bytes, incomplete links, and failed/cancelled/skipped
classifier outcomes. Workflow contracts retain both named package checks and
the test check, ensure their bounded result guards run unconditionally, reject failed/cancelled
execution results, and require cancellable full-route execution jobs. These tests need only shell, Git
and the existing native command allowlist; no compiler bootstrap is needed.
Independent component cases assert retention of compiler, bytecode, runtime/TCP,
bootstrap, package and browser consumers. Local/committed-plan parity, staged
reversals, untracked inputs, missing refs and committed whitespace are covered.
The real local entry point and Makefile run in a fixture without compiler sources
and with compiler/network commands forbidden, asserting no native artifacts,
read-only planning and propagation of document failures. Workflow contracts
require the shared selector in all three validation consumers.

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


## Async/await slice (issue #102)

Compiler effect contracts cover direct/indirect activation, bare calls, ordinary
and pure contexts, blocking helpers/callbacks, AsyncFn conversions, nested awaited
arguments, generic helpers and entry-point results. The public CLI case covers
source and saved bytecode success/error branches and 128 nested async frames;
negative CLI fixtures reject bare calls and blocking effects.

Native async contracts enumerate all caller/callee/activation combinations,
including forged indirect edges, and exercise typed completion schemas, bounded
queues, independent progress, wrong-session/generation/duplicate deliveries and
cancellation before registration, during waits, after enqueue and after delivery.
Allocation sweeps cover typed completion construction, delivery and destruction.
This is fake-host evidence, not real network producer quiescence or source scopes.

## Website content and examples

Displayed examples are checked against the selected released toolchain in the
[website repository](https://github.com/sproates/panackelty-website). Core retains its independent source/bytecode
functional suite and packaged quick-start acceptance.

## Implicit core and method migration

The `core_methods` functional case exercises import-free Option/Result construction and matching across modules, method chaining, explicit empty-array element types and unrelated global names. Parser/checker unit contracts cover private dispatch, wrong receivers and comparator purity/type errors. Existing stdlib assertions retain stable-sort and Unicode suffix edge cases. The migrated playground examples are compared through native CLI and browser execution. Existing fixed bytecode oracles remain independent: unused core algorithms must not alter their artifacts.

## Native async TCP request/response

The bounded `tcp_exchange` contract is covered by `tests/unit/vm/tcp.c`, the
TCP allocation sweep in `native_faults.c` and public-CLI `tests/tcp.sh`.
Evidence covers binary/fragmented and empty transfers, response bounds, refusal,
timeouts, independent progress, destruction cleanup, embedded opt-in and source
and forged-bytecode effect checking. The WASI runtime test requires explicit
unavailability. These checks do not establish DNS/TLS, server APIs, source
spawning, arbitrary network stress or exhaustive kernel-error coverage.


## Bounded TCP server

`unit/vm/tcp_server.c` covers fast progress beside a stalled reader or outbound
await, slot reuse, request/response limits, empty data, client/admission expiry,
stop/drain and destruction (including final admission racing writable clients
with zero grace), handler traps/malformed results, finite computation
budgets, bind conflict, foreign handlers and separate embedded capabilities.
`native_faults.c` sweeps successful server allocations and cancellation lifetimes;
injected socket/bind/listen/accept/poll/read/write/clock failures track all server
and accepted descriptors. `tcp_serve.sh` runs source and saved-bytecode programs
against independent binary/fragmented/empty peers, checks ordered success/error
reports, and exercises busy-handler fairness and source typing/effect rejection.
Both bytecode verifiers reject the independent server arity/effect fixtures.
The pre-server seed's `legacy-server-name.hex` preserves old user-function behavior.
Browser runtime tests check explicit listening rejection.

Evidence concerns finite EOF-framed TCP only: no TLS/HTTP/IPv6, general source
spawning, unbounded service operation or large-scale reactor performance claim.
Platform and sanitizer validation results belong in the implementation PR.

Website publication conditions are regression-tested directly from the workflow:
skipped browser ancestors must not suppress deployment after successful packaging,
and failed/skipped/cancelled packaging or deployment must never permit the next
stage. Pull requests and workflow cancellation cannot deploy. Hosted production
acceptance remains required because these tests do not emulate GitHub's scheduler.


## Semantic-impact feasibility experiment

`make semantic-impact-experiment` tests pre-change predictions from internal
checker records against actually changed checking and public CLI acceptance.
It covers direct guard/effect consequences, positively justified local proof and
ordinary-call preservation, conditional two-edit effect propagation, recovered
caller boundaries after a type-invalid leaf, nested guard replacement and mutation invalidation. Compiler CI runs
this separate Node-based experiment. This is not a general semantic-diff feature
or proof of transitive return guarantees; see the
[report and limitations](experiments/semantic_impact/README.md).

## Native source-map acceptance experiment

`make source-mapping-experiment` now tests the production CLI against independently
specified actual VM function/PC/file/range expectations. Local/imported/generic,
Unicode, inner and earlier indexes, callbacks, function reordering, relocation,
valid non-trapping execution and ordinary bytecode identity remain covered.
Malformed/oversized maps, duplicate records and coherent instruction/source/range
forgeries must fail closed. Core changes, missing/stale/invalid source inputs and
mismatched pairs do too. The old checksum forgery finding is now a rejection
regression against local reproduction, not an accepted metadata weakness.

This additional Node-based suite runs in Linux/macOS compiler CI. It observes the
real dispatcher and does not add an interpreter dependency to `make check`.
The [contract](../docs/SOURCE_MAPS.md) and [experiment](experiments/source_mapping/README.md)
state limits; automatic source-aware runtime errors and programme-wide realistic
usefulness are not claimed.


## U2 frontend source-span foundation

`runner/compiler_source_spans_unit.panack` runs in both the ordinary unit/compiler
suite and `check-compiler`. Its 33 fixed expectations cover every expression
category, nested precedence and associativity, postfix chains, explicit generic
arguments, lowered methods, parenthesised receivers, match-arm blocks, optional
else, await, original-file coordinates, CRLF, Unicode, trailing comments,
bindings/assignments/loops and implicit semicolons. It also checks that synthetic
unlocated ASTs have no invented origin. Expected ranges are literal lexical
positions rather than values derived from the parser under test.

Six compiler integration assertions run complete failing programs through public
`panack check`, `run` and `compile`, asserting exact local binary and imported
generic indexing error locations/excerpts. Existing accept/reject, purity,
control-flow and emitted-bytecode tests remain intact. The native trap assertions consume actual retained spans rather than recovering
ranges by re-parsing. Public source mapping is covered below; automatic runtime
diagnostics remain separate.

The seed refresh also updates 15 existing functional failure transcripts. Their
rejection status and diagnostic messages are unchanged; expectations now include
newly available excerpts or point to the offending compound expression rather
than its enclosing call. These remain byte-exact assertions, including all
stale-guard failure cases; no diagnostic matching was relaxed.


### U2 instruction-source emission

`compiler_instruction_sources_unit.panack`, registered in both compiler and unit
suites, fixes instruction indices/opcodes and direct/lowered/unavailable ranges
for nested arithmetic/indexing, nonzero offsets, short-circuit operators, both
conditional forms, matches, while/for bodies, map/reduce, direct and indirect
await, arrays, fields, ranges, interpolation and synthetic ASTs. Whole loaded
program checks assert ordered unique indices, function/source alignment after
core pruning and map/plain byte identity, including async, standard-library,
imported/generic and nested match/conditional collection code. The imported
Unicode fixture checks original module, line/column and source slice.

The compiler integration suite executes the complete nested lowering fixture
through the public CLI and asserts its six output lines. The runtime source-map
experiment retains its invalid/stale/missing metadata cases and now proves exact
attribution for inner indexes, binding initializers, earlier indexes and a trap
inside a collection callback. It uses actual emitter entries, not AST-tail
reconstruction. Production sidecar coverage follows below; public source-aware runtime messages
remain future work.

### U2 validated sidecar and public CLI

`runner/compiler_source_maps_unit.panack` checks argument boundaries, portable
identity, deterministic bytes, balanced chunk order, Unicode coordinates, missing
functions/PCs, exact loaded closure and source count/byte caps.
`runner/compiler_source_maps_cli.panack` runs complete programs through public
`panack` commands and verifies direct/lowered/unavailable output, repeated map
and plain/mapped byte identity, execution, corrupt maps, missing pairs, comment
changes in local/imported/transitive-unused sources and invalid UTF-8. It checks
fresh-output requirements, direct/directory/hard-link/symlink aliases, preservation
of source and artifact bytes, partial writes and strict argument rejection.
Both suites run under `make unit` and `make check-compiler`, preserving all earlier
span/emitter/control-flow tests. Native-PC and coherent binary-forgery checks use
the separate observer suite above. Hostile concurrent output-directory mutation,
source-free verification and authenticated original producers are outside the
[explicit contract](../docs/SOURCE_MAPS.md).

## U3 first subtraction explanation query

The checker explanation unit suite covers constant and lower-bound decisions,
true/false branch source origins, nested refinements, mutation invalidation and
fresh guards, while/for facts, unsupported operands and generic definitions.
Each source case checks diagnostic parity with ordinary checking and zero
ordinary evidence retention. Existing adversarial mutation fixtures also check
retention parity. The public `cli_explanations` functional case verifies imported
Unicode/generic source ranges, local proof versus whole-program rejection,
missing/unsupported evidence, no program execution, malformed input/options and
ordinary source/bytecode execution. These tests cover the bounded subtraction
query, not the unimplemented type/effect, change-impact or runtime explanations.
Panackelty source execution coverage percentages remain unmeasured.

## Indexed compiler declaration lookup

The existing compiler-contract unit suite adds 19 fixed assertions for declaration
order, per-kind namespace separation, duplicate last-match signatures/effects,
variant owner/type/payload retention, variants retained from earlier duplicate
enums, missing/empty/case-sensitive names, interleaved imports, rebuilt module
indexes and unchanged input modules. Resolver duplicate rejection remains tested.
The compiler integration suite runs the imported `indexed_lookup` project through
the public CLI, covering guarded types, generic records/variants, forward calls
and independently expected output. Existing diagnostics, mutation, source-map,
explanation, generic/effect, full functional and bootstrap tests remain required.
No tests are removed or relaxed to obtain the performance improvement.

## U3 effect explanation acceptance

The existing explanation unit probe adds fixed effect expectations for declarations,
callable parameters/bindings/fields, widening, async/await, nested calls, branches,
loops, generic definitions, match payloads, source ranges and unlocated ASTs.
Retention-on/off diagnostics and empty ordinary evidence are compared. Public CLI
coverage adds imported generic rejection, allowed ordinary/async calls, callable
classification, discarded await and explicit unavailability after frontend errors.
No tests are removed; existing subtraction cases remain. The contract is
[documented here](../docs/COMPILER_EXPLANATIONS.md).

Validation: 196 explanation unit assertions, 36 explanation CLI assertions and
343 functional cases pass through canonical `make check` (150s). Existing
compiler-contract fixtures also preserve exact baseline output/status/artifacts
across 227 cases. Performance and fixed-point evidence are in
[the validation profile](VALIDATION_PROFILE.md#u3-local-effect-explanations--2026-10-03).

## U3 per-function effect recovery acceptance

Explanation unit and public-CLI tests cover valid and rejected siblings after body
errors; invalid callable annotations/receivers, await return types and iterables;
return mismatches; late global signatures, guarded types, records/enums and resolver
failures; imported generic source origins; unchanged ordered stderr; and refreshed
evidence after repair. Invalid callee bodies do not invalidate an otherwise valid
local declaration boundary, nor does that boundary establish callee correctness.
Ordinary mode retains no function availability or recovered effect evidence.
The semantic-impact experiment positively checks a caller boundary after leaf
proof rejection rather than treating missing evidence as non-impact. A type-valid
but impure guard plus an unrelated body error preserves local evidence and
original diagnostics without implying guard or project validity.

Validation: 266 explanation unit assertions, 53 CLI assertions, 25 experiment
assertions and all 343 functional cases pass. Clean canonical `make check` passed
in 154s with compiler/library fixed points; 227 baseline compiler-contract fixtures
preserve exact diagnostics/status and 43 accepted artifacts. See the
[validation profile](VALIDATION_PROFILE.md#u3-per-function-effect-recovery--2026-10-03).

## Runtime provenance feasibility evidence

`make runtime-provenance-experiment` executes the actual native VM and checks
fixed arithmetic/function-return derivations, repeated call/loop/recursive
occurrences, chronological branch context, ring eviction/prefix discard,
frame/local/capacity limits, unsupported host/async/collection/indirect evidence,
redaction and oversized scalar refusal. Public CLI source and bytecode runs
both print 42, and production source mapping confirms the observed multiplication
PC. Native unit validation includes retention ID/edge lookup checks. Linux/macOS
compiler CI also runs the full experiment. This tests a bounded observer, not a
public tracing feature or coverage of general value provenance. See the
[report and limitations](experiments/runtime_provenance/README.md).

Optional installer coverage (`tests/unit/harness/installer.sh`) checks argument
validation, hostile/space-containing HOME paths, foreign destinations, supported
target rejection, repeat installs, version changes/rollback, transport/checksum
failures, unsafe archive paths and links, wrong executable versions, locally
modified/non-executable installs, native compiler startup failures, locking,
signal cleanup and owned removal (including a dangling owned command link).
`tests/release_installer.sh` independently downloads the published alpha.11
archive and asserts the complete README Hello World/check/run/compile/bytecode
output, repeat installation and removal. Check runs it on Ubuntu 22.04 x86_64 and
macOS 14 arm64; network acceptance is separate from offline `make check`.

Build-baseline harness regression coverage runs in canonical policy validation.
It verifies source isolation, exact tracked/untracked/deleted candidate snapshots,
real probe-driver compilation versus execution counts, output validation, failed
command propagation, repeat bounds and output-directory protection. The controlled
real-toolchain observations and their limitations are recorded in
[the validation profile](VALIDATION_PROFILE.md). No language/VM coverage is replaced.

The [website repository](https://github.com/sproates/panackelty-website) owns capabilities-page layout, navigation and
published-release source/bytecode example acceptance.

## SC2 source inventory acceptance

`runner/compiler_source_inventory_unit.panack` supplies literal syntax-inventory
oracles for functions, statements, expressions and source decision outcomes.
`runner/compiler_source_inventory_cli.panack` checks the public producer/validator,
explicit unloaded roots, source and compiler identity, corruption/truncation,
output protection and bytecode equivalence. Both are part of `make check` and
`make check-compiler`. This establishes the denominator contract, not runtime hits
or a measured `.panack` coverage percentage. See
[the inventory contract](../docs/SOURCE_INVENTORY.md).

## SC3 runtime collection acceptance

`runner/compiler_coverage_unit.panack` validates literal raw records, stale
identities, truncation, flags, terminal status and counter relationships.
`runner/compiler_coverage_cli.panack` checks production commands against literal
source counts for branches, loop exits, match arms, line anchors, unused/empty
functions, recursion, generics, lowering and trapping calls, plus semantic
equivalence and exclusive output. Both run in compiler and full validation.
Native `coverage.c` contracts exercise suspension, typed reads, sticky terminal
states, zero budgets, overflow, bounds and equal-destination branch outcomes.
Allocation fault sweeps include collector creation and covered execution.
See [the runtime contract](../docs/SOURCE_COVERAGE.md). Complete child collection
and suite percentages remain SC4–SC5.

### SC4 registered execution aggregation

`runner/compiler_coverage_session_cli.panack` checks exact multi-session branch
and function totals, nested bytecode and native subprocesses, fresh execution of
reused bytecode, exclusive admission, missing and wholly deleted children,
truncated/surplus/mixed/stale records, duplicate sessions, killed claimed
collectors, traps, unregistered code and the cached-transcript guard.
`runner/compiler_coverage_unit.panack` also checks canonical execution IDs,
aggregate overflow and indexed expression entry/completion rules.

Native `coverage_session.c` contracts check task completion/cancellation,
subprocess claim exclusivity and isolated server-handler counters under slot
reuse. The allocation-failure corpus sweeps registry creation, nested collection
and inherited process tickets. `make source-coverage-session-smoke` is the larger
opt-in real-compiler acceptance fixture: it registers and runs the compiler in a
nested VM and asserts exactly one compiler entry plus an unused command at zero.

See [the session contract](../docs/SOURCE_COVERAGE.md). Complete execution data is
not a claim that every source association is available, or that a test suite has
been measured. SC5 still owns baseline scope and publication.


## Else-if delivery evidence — 2026-10-06

The chain feature adds 31 focused assertions: nine parser, one lexer, three
source-span, eleven type/effect/guard, five staged-namespace and two inventory
contracts. Current suite totals are 202 parser, 13 lexer, 36 spans, 250 compiler
contracts, 470 namespace and 24 inventory assertions. Exact CLI fixtures reject
non-Bool conditions, incompatible arm results, missing bodies and partial-chain
value use, with positioned transcripts and no generated artifact. The negative
runner has 59 fixtures and 207 assertions; full functional execution has 355.
Source and saved-bytecode cases verify repeated exhaustive/partial chains,
ordered/skipped conditions and bodies, mixed discarded tail types, nested chains,
async arms and balanced continuation after no match. Namespace execution stays
gated; its direct body-checker assertions do not claim executable namespaces.

Compiler and stdlib fresh fixed points pass. Canonical `make check` passes in
322 seconds, including a 247-second unit phase; the unchanged 120-second full
and 15-second unit targets remain exceeded. These local timings overlapped source
collection and are not a controlled before/after comparison. Seven unchanged
example compilations in seven alternating-order rounds reproduce identical
bytes across all 49 pairs, with median batch times 0.766 seconds before and
0.776 after (+1.3%) and broad overlapping ranges. This bounded shared-host sample
does not establish universal performance acceptance. Seed size changes from
505,229 to 506,859 bytes (+0.32%); bytecode v9 and the stdlib artifact are unchanged.

The complete local source collection validates all 31 manifest contexts, all
38 eligible files, zero unavailable items and the compiler fixed point. Proposed
compiler floors change as follows; all three covered ratios increase:

| Metric | Previous floor | Proposed measured floor |
| --- | --- | --- |
| Executable start lines | 8,332 / 10,038 (83.00%) | 8,367 / 10,068 (83.10%) |
| Functions | 403 / 435 (92.64%) | 405 / 437 (92.68%) |
| Source outcomes | 5,511 / 6,915 (79.70%) | 5,533 / 6,931 (79.83%) |

Both new parser helpers execute. Parser outcome coverage increases 970/1,108 to
986/1,122; lexer outcomes increase 210/213 to 212/215. Existing checker, namespace
body checker and purity outcome coverage gains two, one and one respectively;
all other function/outcome counts remain unchanged. The manifest, bytecode/stdlib
floors and all 68 protected source items are preserved. The 185 existing
exclusions remain, and the new compile-time `ParsedConditional` record adds one
record-declaration exclusion. Parser declaration IDs and coordinates also shift.
The proposed exclusion digest and exact floors explicitly account for these
changes; no runtime exclusion or corpus reduction is introduced. The normal
policy validator passes against the complete evidence. PRs targeting `next` run
policy tests; hosted full collection follows the existing marked-next-refresh or
main-promotion route. These measurements retain the report's bounded-corpus and
assertion-quality limits.
