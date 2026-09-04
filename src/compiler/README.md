# Panackelty compiler

This directory contains the compiler being implemented in Panackelty:

- `types.panack` defines file-aware source positions, tokens, diagnostics, and
  their public `file:line:column: message` rendering.
- `lexer.panack` tokenizes the complete Panackelty lexical vocabulary,
  normalizes terminating physical line breaks while preserving continued
  expressions, and reports positioned invalid-character and unterminated-string
  diagnostics.
- `parser.panack` contains the recursive expression AST and parser for literals,
  operators, calls, explicit named function references, receiver-first
  method-call lowering, field access, indexing,
  blocks, bindings, assignments, and
  local type annotations, including exhaustive value conditionals, optional
  `else` for `Void` conditionals, and `while` and `for` statements. Pattern matching supports variant payload bindings plus
  expression and block arms. Program-level parsing accepts quoted file-relative
  and extensionless logical import declarations and guarded type declarations,
  with complete nested generic and
  array type references. Generic record fields and enum variant payloads are
  parsed alongside pure and impure functions. The parser now covers the complete
  accepted language grammar.
- `resolver.panack` collects top-level functions and constructors, validates
  conflicts with built-ins, and resolves lexical names across functions,
  blocks, loops, conditionals, and pattern arms. Its pure module-graph boundary
  accepts source units already held in memory, walks the units reachable from
  an entry module, detects missing, duplicate, and cyclic graphs, and resolves
  their combined namespace without performing file I/O.
- `checker.panack` validates type references and generic arity, checks the full
  expression and statement AST, infers generic record and enum constructors,
  verifies effect-bearing callable types, indirect invocation, functional array
  operations, collection built-ins, and type-directed Map/Set method aliases,
  exhaustive matches, joins control-flow
  result types, and proves the supported guarded assignments and safe natural
  subtraction facts. It accepts either one parsed program or an already-loaded
  module graph.
- `purity.panack` completes the frontend by walking guarded-type predicates and
  pure function bodies, including nested blocks, branches, loops, matches, and
  call arguments. It rejects calls to impure built-ins, user functions, and
  callable values and
  exposes complete single-source and already-loaded-module frontend entry
  points.
- `emitter.panack` defines the typed bytecode IR and lowers the complete AST to
  deterministic VM instructions, including indirect calls and the iterator
  lowering for persistent array `map`/`reduce`. It computes absolute control-flow targets
  while using persistent arrays, allocates compiler temporaries independently
  per function, and exposes a differential disassembly boundary.
- `loader.panack` is the effectful project boundary. It resolves quoted paths
  relative to their importer, `project/` paths from the entry directory, and
  `stdlib/` paths from the active toolchain. It canonicalizes and recursively
  reads each module once, detects invalid paths, cycles, and missing modules,
  and hands one combined program to the pure frontend and emitter.
- `driver.panack` implements `check`, `compile`, `run`, and `disasm` for source
  and version-7 bytecode, including default output paths and primary positioned
  lexer, parser, name, and type diagnostics.
- `main.panack` is the executable self-hosted compiler entry point.

The public frontend, backend, project loader, and driver live here and execute
from the audited compiler seed on the native VM. The stage-0 implementation in
`src/bootstrap/panackelty.py` remains only as a development oracle and explicit
seed-regeneration tool. This directory contains only Panackelty implementation
sources and documentation.
The pure backend boundary is:

```panackelty
pure compile_program(program: Program): BytecodeProgram
```
