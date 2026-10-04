# Self-hosting roadmap

The proposed P2 bound-body slice refreshes the v9 seed to
`9944c75c33c7bbac4311190f198f82b3870e5ad7f12619f2bbecfa35ec80839e`
(418,426 bytes). Fresh compiler stages 2/3/4 agree, and standard-library artifact
and expected output retain the fixed point below. The loader retains structural
body identities and checks a bounded type subset; inference/proofs, effects,
emission and coordinated namespace migration remain incomplete. No executable
namespace syntax or bytecode format change is enabled. See the
[body validation evidence](tests/VALIDATION_PROFILE.md#namespace-bound-function-bodies--2026-10-04).

The preceding P2 checked-signature slice refreshed the v9 seed without enabling namespace
execution. Fresh stages 2/3/4 produce the same compiler digest
`aa82e3d6114e66ab1515b9e39586b111da568d2b5454e2926953660a841cd625`
(394,040 bytes), and the standard-library artifact remains
`614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`
with unchanged expected output. Source and stdlib migration remain future work;
the checked signature table is not an executable namespace AST. See the
[validation evidence](tests/VALIDATION_PROFILE.md#namespace-checked-signatures--2026-10-04).

The P2 raw-reference and binding-resolution slice refreshes the v9 seed using
existing executable source syntax. Raw dot calls and qualified type/reference/
pattern spellings are retained alongside per-module tagged identities. Namespace
and selective bindings/re-exports resolve internally, while staged source forms
continue to be rejected before execution. The compiler and
stdlib remain on current syntax until the later namespace identity/migration
slices. No namespace-capable source migration or bytecode version bump is claimed.
Fresh stages 2/3/4 converge to compiler SHA-256
`ef788384cb2d33d7d908253fd7311bb1e52cebc4483e69d7ca649118090187df`;
stdlib conformance remains
`614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`.
Public-CLI rejection tests validate this intermediate seed; see the [namespace validation evidence](tests/VALIDATION_PROFILE.md#namespace-modulebinding-foundation--2026-10-04).

Per-function effect recovery refreshes the v9 seed while preserving ordinary
compilation diagnostics and artifacts. Fresh stages 2/3/4 converge to
`75b4d7cd92596ef6c5be871dfdda243b7ee02ff404f5adba2a21e94b814a6224`
(318,005 bytes); the standard-library fixed point remains
`614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`.
Recovery is opt-in explanation evidence for type-valid functions, with global
declaration failures and invalid function bodies explicitly unavailable.

The U3 effect query extends opt-in explanations to actual local call/await
decisions. Its v9 seed is regenerated from the prior indexed-lookup seed;
fresh stages 2/3/4 converge to
`777282023eab5e6d2760320fffb34dcefd86f281384203514bd2fdd29fad8580`
(316,297 bytes), and the library fixed point remains
`614534e2382ce7999f22652442900c3433824bb6fc72259d63c28049f46465b6`.
Ordinary checking and user artifacts retain their contracts. Validation and
measured overhead are recorded in the validation profile.

The indexed-lookup performance change refreshes the v9 seed while preserving
compiler semantics and user-program bytecode. The previous seed reproduces the
new compiler, and fresh stages 2/3/4 converge with the unchanged standard-library
fixed point. Program declaration indexes are internal compiler data, not a new
bytecode ABI or an unchecked compilation mode. See the indexed-lookup measurements
in [the validation profile](tests/VALIDATION_PROFILE.md).

The first U3 query refreshes the v9 seed with opt-in checker evidence and
`panack explain SOURCE.panack --function NAME`. Ordinary proof rules and emitted
program bytes are unchanged. Fresh stage-2/3/4 compiler and standard-library
fixed points validate the refresh; see the [query contract](docs/COMPILER_EXPLANATIONS.md)
and validation profile. This is a bounded subtraction query, not full compiler
or runtime explanations.

The U2 deliveries refresh the v9 seed with complete frontend expression spans,
optional instruction-source retention and validated public source-map commands.
The previous seed builds the candidate; fresh stage-2/3/4 compiler and standard-
library artifacts converge byte-for-byte. Language syntax, emitted instructions
and bytecode v9 are unchanged. Ordinary compilation omits source tables; opt-in
sidecars preserve executable bytes. Lookup reproduces the compilation and exact
loaded source closure before accepting attribution. See the
[source-map contract](docs/SOURCE_MAPS.md) and validation profile for trust limits,
size bounds and measured cost.

The #182 guard-fact repair refreshes the v9 compiler seed so the public CLI
rejects stale proofs after mutation. The seed refresh verifies identical
stage-2/3/4 compiler and standard-library artifacts; the bytecode format and
release version are unchanged.

The bounded TCP server addition retains v9 and refreshes the compiler seed for
`tcp_serve`, its handler signature, limits record and reserved builtin lowering.
The seed was regenerated with identical stage 2/3/4 compiler and standard-library
artifacts. The VM itself remains independent of compiler source; the new server
owner executes ordinary verified handler bytecode through resumable frames.

The compiler and standard library build on the native VM and reach a
reproducible compiler fixed point. A milestone is complete only when its behavior
has end-to-end and focused failure-case coverage in `make check`.

## Bytecode v9 migration

The async/await slice refreshes the self-hosted seed to `bootstrap/compiler-v9.bc`.
The historical v8 seed built the migrating compiler without using new source
syntax in the compiler itself. Repeated v9 compiler/stdlib builds and the seed
refresh transaction verify convergence; v8 support is retired after that bridge.
The historical milestones below retain their original version numbers.

The subsequent TCP service refresh keeps format v9 and rebuilds the compiler
seed with the additive builtin signature, effect checking and decoder validation.
The seed refresh transaction checks compiler and stdlib fixed points.

## Bootstrap status

The self-hosting critical path is complete. The compiler is written in
Panackelty and executes on the portable C11 VM. Build, validation, installation
and packaging use this toolchain and standard development utilities.

The completed critical path was:

1. Finish module resolution, type/refinement checking, and purity checking in
   the Panackelty-hosted frontend.
2. Freeze deterministic bytecode and verifier semantics.
3. Implement the compiler backend, artifact tooling, project loader, and CLI in
   Panackelty, then compile the complete compiler with the bootstrap toolchain.
4. Define the minimum standard-library and VM-to-host boundary required by the
   compiler.
5. Implement the portable native seed VM and validate its conformance.
6. Produce stage-1, stage-2, and stage-3 compilers and require a byte-identical
   fixed point.

Post-bootstrap language and engineering priorities are tracked in
`ROADMAP.md`.

## Milestone 1: Compiler data model — complete

- [x] Nominal records for positions, tokens, AST nodes, and diagnostics
- [x] Tagged unions for alternatives such as token and expression kinds
- [x] Exhaustive pattern matching with typed payload bindings
- [x] Pure constructors, field access, and matching
- [x] Serialized VM instructions for records and enums
- [x] Source-to-bytecode-to-VM conformance coverage
- [x] First lexer-shaped program in `examples/lexer_foundation.panack`

## Milestone 2: Collections and errors — substantially complete

- [x] Generic record and enum type parameters with constructor inference
- [x] Generic `Option[T]` and `Result[T, E]` can be expressed and used
- [x] Contextually typed empty arrays
- [x] Persistent `append` and `concat` operations
- [x] Persistent maps and sets
- [x] Byte buffers, UTF-8 conversion, and byte-oriented operations
- [x] Generic functions and explicit type arguments, with argument inference,
      abstract body checking, purity preservation, and erased version-8 calls
- [ ] Specify an ownership model for efficient collection construction
- [ ] Implement uniquely owned mutable collection builders if required by
      compiler performance measurements

## Milestone 3: Program structure and platform APIs

- [x] Relative modules and cycle-checked imports
- [x] Command-line arguments
- [x] Environment access
- [x] stderr and process exit codes
- [x] Binary file I/O end-to-end coverage, including failure cases
- [x] Rich path operations and path normalization
- [x] Diagnostic and source-position data model
- [x] File-aware primary line/column tracking and token start/end positions
- [ ] Audit full AST span fidelity and preserve mappings through lowering for
      source coverage; frontend positions alone do not provide VM source maps
- [x] Render existing positioned diagnostics with source excerpts and carets,
      including imported modules and deterministic tab/Unicode display
- [ ] Rich multi-error recovery, secondary spans, and causal diagnostics

## Milestone 4: Compiler frontend in Panackelty (`src/compiler`)

- [x] Complete the lexer for comments, identifiers, integers, decimals,
      strings, symbols, invalid characters, and malformed input
- [x] Add focused lexer coverage for every token class and failure mode
- [x] Complete the parser and recursive AST for the full language
  - [x] Scalar and array literals, unary expressions, full binary precedence,
        calls, field access, and indexing
  - [x] Blocks, optional local type annotations, bindings, assignment, tail values,
        newline termination, and explicit same-line separators
  - [x] Conditionals with block branches and optional `else` in `Void` position
  - [x] `while` and `for` loops
  - [x] Pattern matching with expression and block arms
  - [x] Import declarations and program-level parsing
  - [x] Guarded type declarations and complete type-reference syntax
  - [x] Generic record and enum declarations
  - [x] Pure and impure function declarations
- [x] Top-level symbol collection and lexical name resolution
- [x] Already-loaded module graph resolution
- [x] Type and refinement checker
- [x] Purity checker
- [x] Differential frontend tests against the bootstrap compiler

The lexer now handles the complete token vocabulary, normalizes terminating line
breaks while preserving multiline expressions, skips comments, uses half-open
source offsets, and reports positioned diagnostics for invalid characters and
unterminated strings. The current parser still handles
the complete expression precedence hierarchy, scalar and array literals, calls,
receiver-first method-call lowering, field access, indexing, blocks, bindings,
assignments, tail values, and
conditional expressions with optional `else`, `while` and `for` statements, and
pattern matching with payload bindings. Program-level parsing accepts quoted
file-relative imports plus extensionless `project/` and `stdlib/` logical
imports and guarded type declarations, with nested generic and array type
references. Generic
record and enum declarations plus pure and impure functions are also parsed.
The self-hosted parser now covers the complete accepted language grammar. The
resolver validates top-level symbol conflicts and function-local names, walks
the modules reachable from an entry unit in dependency-first order, rejects
missing, duplicate, and cyclic module graphs, and resolves the reachable
declarations as one namespace. The self-hosted checker validates declared and
generic types, expressions, calls, records, enums, collections, control flow,
function returns, exhaustive matches, mutable assignments, entry points, and
initializer-based local inference with complete types, fixed mutability, and no
shadowing. It retains the currently supported guarded-type and `Nat` subtraction proofs. Differential
tests compare the complete frontend's accept/reject decisions with the bootstrap
checker. The self-hosted purity pass walks guards and every expression and
statement position in pure functions, rejecting direct or transitive calls to
impure built-ins and user functions. The Panackelty-hosted frontend milestone is
complete.

## Milestone 5: Stable bytecode and VM contract — complete

- [x] Freeze and document instruction, value, call-frame, and trap semantics
- [x] Specify canonical ordering and deterministic code generation
- [x] Replace the transitional JSON payload with a compact binary encoding
- [x] Specify limits and validation rules for untrusted bytecode
- [x] Add portable golden bytecode and malformed-artifact test vectors
- [x] Prove repeated compilation produces byte-identical artifacts

The executable contract must be stable before the self-hosted backend and
native VM implement it independently.

## Milestone 6: Compiler backend and driver in Panackelty — complete

- [x] Bytecode emitter targeting the stable contract
- [x] Binary serializer and deserializer
- [x] Bytecode verifier and disassembler
- [x] Effectful project loader for file-relative and logical imports and module graphs
- [x] CLI driver for `check`, `compile`, `run`, and `disasm`
- [x] Differential compiler and artifact tests against the bootstrap toolchain
- [x] Compile the complete Panackelty compiler to bytecode with the bootstrap compiler

The compiler core will receive an already-loaded module graph so its boundary
can remain pure while import discovery and file access remain effectful:

```panackelty
pure compile_program(program: Program): BytecodeProgram
```

The effectful loader resolves and reads the module graph, including
entry-directory `project/` imports and toolchain-owned `stdlib/` imports, then
the resolver, type checker, and purity checker validate the combined `Program`
before it crosses this pure emitter boundary.

## Milestone 7: Standard library and runtime boundary — complete

- [x] Define the standard-library module layout and public API
- [x] Provide canonical `Option`, `Result`, collection, text, byte, and path APIs
- [x] Separate portable Panackelty library code from VM and operating-system intrinsics
- [x] Compile and test the standard library as part of every compiler stage
- [x] Document the stable VM-to-host ABI for terminal, file, environment, and
      process operations

`src/stdlib` contains the implicit core and explicit public modules, with a
combined prelude for the latter.
Portable Panackelty definitions are separated from deterministic VM primitives
and from the effectful named-call ABI in `src/runtime/ABI.md`. Environment and
argument inputs are snapshotted per VM and inherited by nested execution. The
standard-library conformance graph compiles with both currently available
compiler stages, and their current-version artifacts must be byte-identical.

## Milestone 8: Native seed VM — complete

- [x] Define portable value representation, allocation, and memory reclamation
- [x] Implement the VM, verifier, loader, and OS boundary in portable C11
- [x] Match Panackelty numeric, UTF-8, collection, trap, and effect semantics
- [x] Validate the native VM against independent conformance expectations
- [x] Validate malformed and adversarial bytecode on the native VM
- [x] Run the Panackelty compiler bytecode on the native VM

## Milestone 9: Reproducible bootstrap — complete

- [x] Use the bootstrap compiler to produce the stage-1 Panackelty compiler
- [x] Use stage 1 to produce stage 2
- [x] Use stage 2 to produce stage 3
- [x] Require stage 2 and stage 3 compiler and standard-library artifacts to be
      byte-identical
- [x] Build, test, and package with an allowlisted command environment
- [x] Establish a self-contained release and build toolchain

The native VM builds and runs the compiler and standard library, reproduces
their artifacts exactly, and passes the native conformance suite. The checked
seed and its digest are documented in `bootstrap/README.md`.
Stage 2 may reuse bytecode keyed by the complete source/toolchain contents; it
is verified before use and shared by the oracle and functional checks. Stage 3
and the isolated seed-refresh transaction retain their independent compilation
and byte-identity checks.
Seed refresh is now self-hosted: `make regenerate-seed` verifies the input
digest, stages compiler builds 2–4, checks compiler and standard-library
identity plus expected output, and publishes only after all checks pass. A
real refresh with an allowlisted `PATH` runs in `make bootstrap-check`; shell
failure injection runs in the unit and focused compiler suites.
The compiler source itself exercises the typed Map and Set method aliases in
its lexer and project loader, so every bootstrap stage proves those calls as
part of the fixed point.

## Rational and Unit follow-up

The compiler and bytecode tools now use explicit natural quotient division where
integer results are required. Both frontends and VMs support exact `Rat` and
first-class `Unit`. The version-8 seed replaces version 7; normal stage-2/stage-3
fixed-point and standard-library conformance gates continue to apply.

## Path and time follow-up

Both frontends and runtimes support opaque `Path`, `Duration`, and `Instant`.
The version-8 seed has been refreshed for their builtin signatures, allowing
every stage to compile the expanded prelude. Normal stage-2/stage-3 fixed-point
and standard-library artifact comparisons remain required. String path helpers
remain necessary for existing compiler loading and bootstrap compatibility.

## Typed host capability follow-up

Both frontends and runtimes support typed filesystem calls, bounded process
execution, checked UTF-8 decoding, and sleep. The version-8 seed includes their
signatures; the normal compiler and library fixed-point comparisons still apply.
`stdlib/testing` provides pure structured assertions and ordered reporting.
`stdlib/testing_files` provides sorted fixture directory discovery and
explicitly owned temporary workspaces. `stdlib/testing_commands` now checks
bounded process results and expected host errors. The testing-library foundation
and live oracle replacement are complete. Seed refresh is now self-hosted;
bootstrap implementation retirement is complete.
The Panackelty-hosted functional runner now checks twenty-five selected
success fixtures, all twenty examples, and forty-one failure fixtures. It
also checks six `run`/`disasm` failure pairs, rational traps, exact displayed
diagnostics, environment and file I/O, and a stage-two compiler driver
comparison. `runner_smoke` runs from both source and saved bytecode. The
functional suite checks fixed expectations.
The direct lexer, parser, and resolver unit contracts run in Panackelty.
The parser has 192 assertions in 38 groups; the resolver has 26 assertions
in 12 groups. The type checker now has 34 native direct
contracts using fixed source expectations. The purity checker adds 11 native
contracts with ten fixed source inputs. Fixed independent arithmetic/artifact
expectations and bootstrap identity provide additional evidence.
See `tests/ORACLE_REPLACEMENT.md`.

Direct bytecode/verification coverage runs in
`tests/runner/bytecode_unit.panack`, `tests/runner/bytecode_native_unit.panack`
and the native C verifier contracts in `tests/unit/vm/native_modules.c`.
These share fixed version-8 and malformed artifact vectors and compare exact
canonical artifacts and disassemblies. Fixture provenance and wire-format
expectations are documented in
`tests/fixtures/bytecode/contract_cases/README.md`.

The [testing guide](tests/README.md) describes the native and Panackelty-hosted
suites and their validation commands.

The remaining direct compiler contracts now run in
`tests/runner/compiler_contracts_unit.panack` (201 assertions) and
`tests/runner/compiler_integration_unit.panack` (51 assertions), under both
`make unit` and `make check-compiler`. They cover emitter instructions, diagnostic
rendering and source snapshots, loader/imports and driver commands, generics,
inference, types and host boundaries. The probes use fixed expectations;
their provenance is recorded in
`tests/ORACLE_REPLACEMENT.md`. Seed regeneration uses verified self-hosted stages.


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
The [fixture guide](tests/fixtures/host_runtime/README.md) describes direct native
evidence, fixed oracle fixtures and bootstrap cross-checks. Functional source
and bytecode cases verify public behaviour on both supported platforms.

Development harness checks use shell and native tools to validate repository
policy, packaging, timing, seed rejection and fixture-runner failures.
Both supported platforms run the five `make check-no-interpreter CI_SUITE=…`
partitions on the full validation route, retaining the complete check, isolated
bootstrap proof, native conformance and exact-archive checks. Informational-only edits use the documented lightweight
checks and do not rebuild or package the toolchain.

The ordinary bootstrap fixed-point check and native seed-refresh proof execute
concurrently with two workers by default. They retain separate stage artifacts,
digests, compiler/library comparisons and publication checks; neither proof
substitutes artifacts from the other. A single-worker setting runs them serially.

## Core-library migration

The compiler now uses the canonical implicit Option/Result definitions. To cross from the previous v9 seed, a temporary source copy explicitly imported the new core from compiler/types.panack. The old seed compiled that bridge; the bridge compiled the final sources with implicit loading. No staging imports or public compatibility aliases remain in the repository. The final v9 seed and digest are refreshed through the existing three-stage compiler and stdlib fixed-point proof. Bytecode stays v9.
