[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Ergonomic control flow and collection APIs — complete

<a id="rm-80"></a>

**RM#80: Control flow and collection ergonomics**.

The algorithm examples show several places where the language's surface syntax
is noisier than its semantics. Improve those areas as one staged initiative so
that control flow, persistent collections, strings, and functional operations
form a coherent API rather than a collection of unrelated special cases.

The intended direction is type-directed method syntax such as `memo.has(key)`,
`memo.put(key, value)`, and `text.reverse()`. Collection updates remain
persistent: methods such as `put` and `add` return a new value rather than
mutating their receiver. An `if` without `else` is valid only in statement or
`Void` position; an `if` used as a value remains exhaustive. Semicolons become
optional line terminators but remain available to separate statements on one
line and resolve otherwise ambiguous layouts.

- [x] Specify newline handling, optional-semicolon parsing, ambiguous multiline
      expressions, and the remaining cases where an explicit separator is
      required
- [x] Make `else` optional for statement-position and `Void` `if` expressions
      while retaining mandatory exhaustiveness in value position
- [x] Implement the grammar changes in both compiler frontends with focused
      parser, type-checker, diagnostic, and public-CLI coverage
- [x] Add type-directed method-call syntax and define its interaction with
      existing record field access, generic types, diagnostics, and name lookup
- [x] Expose existing persistent operations as methods, beginning with map
      `has`, `get`, and `put`; set `has` and `add`; and array `append` and
      `concat`
- [x] Expose string operations as methods, including `len`, `slice`,
      `starts_with`, and `reverse`; define `reverse` over Unicode code points to
      match current string indexing semantics
- [x] Decide and document whether legacy free-function spellings remain as a
      bootstrap compatibility layer or are removed in one coordinated migration
- [x] Design callable types and named function references with deterministic
      generic argument and effect inference
- [x] Add generic array `map` and `reduce` operations with pure callback
      contracts, accumulator inference, and persistent results
- [x] Evaluate concise lambda syntax after named callbacks, function types, and
      effect checking are stable rather than special-casing lambdas for
      collection operations
- [x] Migrate the self-hosted compiler, standard library, examples, tests, and
      documentation to optional `else` and newline statement termination
- [x] Migrate those sources to the accepted method and callable APIs as each
      later stage becomes stable
- [x] Add representative programs and complete source, bytecode, bootstrap, and
      cross-VM conformance coverage for the grammar changes
- [x] Complete the same conformance coverage for method calls, callable values,
      and functional collection operations

The callable stage uses explicit non-capturing `@name` references,
`PureFn[...]`/`Fn[...]` types, and `.call(...)`. Array `map` and `reduce` accept
only pure callbacks and lower to ordinary iteration plus verified indirect
calls. Concise lambdas were evaluated but intentionally deferred: introducing
capture and closure lifetime semantics solely as collection shorthand would
weaken the small, explicit callable model. They can be reconsidered alongside
local type inference if representative programs demonstrate a clear need.
