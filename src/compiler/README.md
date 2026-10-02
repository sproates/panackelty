# Panackelty compiler

This directory contains the compiler being implemented in Panackelty:

- `types.panack` defines file-aware source positions and half-open expression spans, tokens, diagnostics, and
  their public `file:line:column: message` header rendering.
- `diagnostics.panack` adds numbered source excerpts and aligned carets from
  retained source snapshots, with deterministic tab and Unicode display and
  header-only fallback when no valid excerpt is available.
- `lexer.panack` tokenizes the complete Panackelty lexical vocabulary,
  normalizes terminating physical line breaks while preserving continued
  expressions, and reports positioned invalid-character and unterminated-string
  diagnostics.
Expression `LocatedExpr` wrappers carry `SourceSpan(start, end)`: both endpoints
refer to the original module and use Unicode code-point offsets with one-based
line/column coordinates. Ranges include delimiters and explicit generic arguments;
method lowering retains the written receiver/method range. Parentheses extend the
outer range without changing rendering or child expression ranges. Synthetic
unlocated expressions return `None` from `expression_span`. This is expression
attribution, not spans for every declaration, statement or generated instruction.
The emitter can retain these ranges through `compile_program_with_sources`.
Its sparse entries identify function-local instruction indices and distinguish
expression operations from lowered machinery. Missing entries mean unavailable;
there is no nearest-instruction fallback. Ordinary `compile_program` retains no
entries. Serialization, trusted metadata consumption and public CLI exposure
remain U2 work.

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
  per function, and exposes differential disassembly and opt-in instruction-source
  boundaries. Both paths emit identical executable bytes; source tables undergo
  the same core-function pruning as the instruction table.
- `loader.panack` is the effectful project boundary. It resolves quoted paths
  relative to their importer, `project/` paths from the entry directory, and
  `stdlib/` paths from the active toolchain. It canonicalizes and recursively
  reads each module once, detects invalid paths, cycles, and missing modules,
  retains ordered source paths and snapshots for diagnostic rendering and source maps, and hands one combined
  program to the pure frontend and emitter.
- `source_maps.panack` owns canonical optional sidecars and exact local replay
  validation, portable identifiers, bounded file comparison and location rendering.
  It never decodes foreign map paths or lengths. See the
  [source-map contract](../../docs/SOURCE_MAPS.md).
- `driver.panack` implements `check`, `compile`, `run`, `disasm` and `locate` for source
  and version-9 bytecode, including default output paths and primary positioned
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


Async declarations carry a separate AST effect flag. Await uses a unary AST node
whose operand must be a call; emission rewrites only that call's final invocation,
leaving argument calls unchanged. The effect pass checks ordinary, pure and async
contexts, including inferred callable targets. AsyncFn never coerces to Fn/PureFn.

## Implicit core loading

The project loader requires the bundled `core.panack` and loads it once before entry/import traversal. Core function tokens receive internal identities while preserving locations. Standard `ends_with`, `first`, `sort_by` and `parse_nat` dot calls select fixed implementations; signatures enforce receiver types and existing generic/purity rules. Ordinary user receiver-first calls remain supported outside that method set. The emitter omits unreachable core algorithms only; it does not remove user functions. Pure parser/checker test helpers still accept explicit AST/module inputs and do not perform filesystem-based implicit loading.

## Mutable guard facts

The checker invalidates bounds for assigned locals across statements, nested
expressions and loop iterations. Compound expressions conservatively discard
incoming bounds for every local they may write; fresh branch/loop guards can
restore facts. Direct guarded decrement remains valid because its RHS is checked
before the assignment kills the old fact. Regression fixtures named
`fact_mutation_*` cover stale subtraction and guarded-type proofs, nested writes,
loop-carried changes, fresh facts and unrelated bindings. No new explanation
command or relational constraint solver is introduced by this repair.

The mutation regression suite also covers upper bounds, false branches,
short-circuit writes, declaration initialisers and match subjects. A 48-case
matrix pairs stale-proof rejection with freshly guarded acceptance and actual
VM execution against hand-calculated results. Existing public CLI failure tests
exercise `check`, `compile`, `run` and `disasm` for the mutation fixtures and
verify failed compilation leaves no artifact.
