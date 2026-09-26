# Panackelty compiler

This directory contains the compiler being implemented in Panackelty:

- `types.panack` defines file-aware source positions, tokens, diagnostics, and
  their public `file:line:column: message` header rendering.
- `diagnostics.panack` adds numbered source excerpts and aligned carets from
  retained source snapshots, with deterministic tab and Unicode display and
  header-only fallback when no valid excerpt is available.
- `lexer.panack` tokenizes the complete Panackelty lexical vocabulary,
  normalizes terminating physical line breaks while preserving continued
  expressions, and reports positioned invalid-character and unterminated-string
  diagnostics.
- `parser.panack` contains the recursive expression AST and parser for literals,
  operators, calls, explicit named function references, receiver-first
  method-call lowering, field access, indexing,
  blocks, bindings, assignments, and
  optional local type annotations, including exhaustive value conditionals, optional
  `else` for `Void` conditionals, and `while` and `for` statements. Pattern matching supports variant payload bindings plus
  expression and block arms. Program-level parsing accepts quoted file-relative
  and extensionless logical import declarations and guarded type declarations,
  with complete nested generic and
  array type references. Generic record fields and enum variant payloads are
  parsed alongside pure and impure functions with scoped type parameters and
  optional explicit type arguments at direct and receiver-first calls. The parser now covers the complete
  accepted language grammar.
- `resolver.panack` collects top-level functions and constructors, validates
  conflicts with built-ins, and resolves lexical names across functions,
  blocks, loops, conditionals, and pattern arms. Its pure module-graph boundary
  accepts source units already held in memory, walks the units reachable from
  an entry module, detects missing, duplicate, and cyclic graphs, and resolves
  their combined namespace without performing file I/O. Plain `name = value`
  introduces an immutable local only when no local or parameter is visible;
  explicit declarations retain the no-shadowing rule.
- `checker.panack` validates type references and generic arity, checks the full
  expression and statement AST, infers local bindings and generic constructors,
  checks generic function bodies with abstract parameters and resolves complete
  call substitutions from explicit types or all value arguments,
  rejects unresolved inferred locals at their declaration, merges nested
  constructor/array type evidence, preserves fixed binding types,
  verifies effect-bearing callable types, indirect invocation, functional array
  operations, collection built-ins, and type-directed Map/Set method aliases,
  exhaustive matches, joins control-flow
  result types, and proves the supported guarded assignments and safe natural
  subtraction facts. It accepts either one parsed program or an already-loaded
  module graph.
- `purity.panack` completes the frontend by walking guarded-type predicates and
  pure function bodies, including nested blocks, branches, loops, matches, and
  call arguments. It rejects calls to impure built-ins, user functions, and
  callable values and retains inferred callable signatures through local, loop, and pattern scopes.
  It exposes complete single-source and already-loaded-module frontend entry
  points.
- `emitter.panack` defines the typed bytecode IR and lowers the complete AST to
  deterministic VM instructions, erasing generic type arguments into one ordinary
  function body per declaration, including indirect calls and the iterator
  lowering for persistent array `map`/`reduce`. It computes absolute control-flow targets
  while using persistent arrays, allocates compiler temporaries independently
  per function, and exposes a differential disassembly boundary.
- `loader.panack` is the effectful project boundary. It resolves quoted paths
  relative to their importer, `project/` paths from the entry directory, and
  `stdlib/` paths from the active toolchain. It canonicalizes and recursively
  reads each module once, detects invalid paths, cycles, and missing modules,
  retains each source snapshot for diagnostic rendering, and hands one combined
  program to the pure frontend and emitter.
- `driver.panack` implements `check`, `compile`, `run`, and `disasm` for source
  and version-8 bytecode, including default output paths and primary positioned
  lexer, parser, name, and type diagnostics.
- `main.panack` is the executable self-hosted compiler entry point.

The public frontend, backend, project loader, and driver live here and execute
from the audited compiler seed on the native VM. The stage-0 implementation
and its implementation-only tests are retired. This directory contains only
Panackelty implementation sources and
documentation.
Direct lexer, parser, resolver, type-checker, and purity contracts live in
`tests/runner/compiler_{lexer,parser,resolver,checker,purity}_unit.panack`. These import the implementation
modules and use `stdlib/testing`; `make unit` and `make check-compiler` run
them on the native VM. The parser probe preserves all 192 expanded expectations
for expressions, blocks, types and programs; the resolver checks 26 source and
module-graph assertions, including exact positioned diagnostics. The checker
and purity probes keep all 31/10 source expectations and 3/1 module contracts.
The fixed-expectation audit is in
`tests/ORACLE_REPLACEMENT.md`.

The pure backend boundary is:

```panackelty
pure compile_program(program: Program): BytecodeProgram
```

Both frontends infer `Rat` from integer division and `Unit` from `()`. Empty
parentheses lower to the pure internal `$unit` call, so no new AST variant or
constant tag is needed. Rational conversions use ordinary receiver-first calls.
Compiler byte packing uses `quotient` explicitly under bytecode 8.

The checker reserves opaque `Path`, `Duration`, and `Instant` types and checks
their builtin signatures. The purity checker rejects `instant_now` inside pure
functions. These operations lower to ordinary verified calls; they do not add
bytecode constants or expose record constructors.

Both frontends register the typed filesystem, process, sleep, and checked-decoding
services with identical signatures and effects. These calls lower through the
existing named-call ABI; their record and enum definitions live in the stdlib.

The remaining direct compiler contracts now run in
`tests/runner/compiler_contracts_unit.panack` (201 assertions) and
`tests/runner/compiler_integration_unit.panack` (51 assertions), under both
`make unit` and `make check-compiler`. They cover emitter instructions, diagnostic
rendering and source snapshots, loader/imports and driver commands, generics,
inference, types and host boundaries. The probes use fixed expectations;
their provenance is recorded in
`tests/ORACLE_REPLACEMENT.md`. Seed regeneration uses verified self-hosted stages.
