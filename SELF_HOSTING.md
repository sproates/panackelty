# Self-hosting roadmap

The goal is to run the Panackelty compiler and standard library without a Python
dependency, then reach a reproducible compiler fixed point. A milestone is
complete only when its behavior has end-to-end and focused failure-case
coverage in `make check`.

## Bootstrap status

The self-hosting critical path is complete. The public compiler, VM, build,
conformance, installation, and package path no longer depend on Python. The
transitional Python implementation remains a development oracle and an
explicit seed-regeneration tool, not a release or build dependency.

The completed critical path was:

1. Finish module resolution, type/refinement checking, and purity checking in
   the Panackelty-hosted frontend.
2. Freeze deterministic bytecode and verifier semantics.
3. Implement the compiler backend, artifact tooling, project loader, and CLI in
   Panackelty, then compile the complete compiler with the bootstrap toolchain.
4. Define the minimum standard-library and VM-to-host boundary required by the
   compiler.
5. Implement the portable native seed VM and run the same conformance suite on
   both implementations.
6. Produce stage-1, stage-2, and stage-3 compilers and require a byte-identical
   fixed point before removing Python.

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
- [ ] Generic functions and explicit type arguments
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
- [ ] Accurate line/column tracking and span-bearing tokens and AST nodes
- [ ] Multi-diagnostic rendering with source excerpts

## Milestone 4: Compiler frontend in Panackelty (`src/compiler`)

- [x] Complete the lexer for comments, identifiers, integers, decimals,
      strings, symbols, invalid characters, and malformed input
- [x] Add focused lexer coverage for every token class and failure mode
- [x] Complete the parser and recursive AST for the full language
  - [x] Scalar and array literals, unary expressions, full binary precedence,
        calls, field access, and indexing
  - [x] Blocks, local type annotations, bindings, assignment, tail values,
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
the currently supported guarded-type and `Nat` subtraction proofs. Differential
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

`src/stdlib` now contains the explicit public module graph and complete prelude.
Portable Panackelty definitions are separated from deterministic VM primitives
and from the effectful named-call ABI in `src/runtime/ABI.md`. Environment and
argument inputs are snapshotted per VM and inherited by nested execution. The
standard-library conformance graph compiles with both currently available
compiler stages, and their version-7 artifacts must be byte-identical.

## Milestone 8: Native seed VM — complete

- [x] Define portable value representation, allocation, and memory reclamation
- [x] Implement the VM, verifier, loader, and OS boundary in portable C11
- [x] Match Panackelty numeric, UTF-8, collection, trap, and effect semantics
- [x] Differentially execute the conformance suite on the Python and native VMs
- [x] Validate malformed and adversarial bytecode on the native VM
- [x] Run the Panackelty compiler bytecode on the native VM

## Milestone 9: Reproducible bootstrap — complete

- [x] Use the bootstrap compiler to produce the stage-1 Panackelty compiler
- [x] Use stage 1 to produce stage 2
- [x] Use stage 2 to produce stage 3
- [x] Require stage 2 and stage 3 compiler and standard-library artifacts to be
      byte-identical
- [x] Build, test, and package from a clean environment without Python
- [x] Remove Python from release and build dependencies

The native VM builds and runs the compiler and standard library, reproduces
their artifacts exactly, and passes the native conformance suite without
Python. The checked seed and its digest are documented in `bootstrap/README.md`.
The compiler source itself exercises the typed Map and Set method aliases in
its lexer and project loader, so every bootstrap stage proves those calls as
part of the fixed point.
