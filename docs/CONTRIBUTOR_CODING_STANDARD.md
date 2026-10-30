# Contributor coding standard

Detailed source conventions for Panackelty contributors. Start with the
[contribution guide](../CONTRIBUTING.md) for scope, AI assistance, pull requests
and validation.

## Panackelty source

This section is the authoritative coding standard for hand-written `.panack`
source. It applies to compiler, tools, libraries, tests and examples. The
[specification](../docs/reference/SPEC.md) defines legal syntax and behaviour; this standard defines
how we present that syntax for readers. Examples below target the current `next`
compiler using bytecode v9. Namespace execution is supported for the checked
subset described in the Modules section of `docs/reference/SPEC.md`; release promotion remains
gated by the roadmap acceptance checkpoint.

**Required** rules are review requirements for new code and deliberate readability
batches. **Preferred** rules allow a clearer alternative with a brief explanation
in the PR. Small fixes may preserve surrounding layout; migrate existing files
incrementally with their tests rather than reformatting the repository at once.
Public names, source-location fixtures and generated artifacts require the
exceptions below.

### Readable Panackelty

**Readable Panackelty is code whose purpose, main flow and important constraints
a human can understand without fighting its presentation.** It should look
inviting on the page as well as follow the mechanical coding standard.
Formatting compliance is necessary for deliberate readability batches, but
does not by itself establish readability.

Evaluate these qualities together:

- **Breathing room:** separate meaningful phases with blank lines, wrap long
  expressions at natural boundaries and group related declarations. Avoid walls
  of code, dense statement chains and excessive spacing that disconnects ideas.
- **Visible structure:** make the main path easy to follow and error paths easy
  to recognise. Reduce deep nesting where supported syntax or cohesive helpers
  make the algorithm clearer. Extra indirection and a forest of tiny functions
  can be just as difficult to read as nested code.
- **Intent in names:** name responsibilities and domain values precisely. Use
  intermediate bindings when they explain a calculation or decision; avoid
  abbreviations and clever compression that force readers to decode the code.
- **Useful explanation:** comment purpose, algorithms, invariants, ownership,
  bounds and non-obvious trade-offs. Give complex routines a short overview
  where needed. Comments should add understanding, not narrate each line or
  compensate for misleading names and structure.
- **Coherent detail:** keep related work together, use a consistent visual
  rhythm and let readers see one responsibility at a time. Preserve helpful
  type/effect contracts and intentional fixture or generated-source exceptions.

For each agreed readability batch, read the resulting file from top to bottom.
A reviewer should be able to identify its purpose, explain the main algorithm,
recognise its failure paths and find the important invariants without repeatedly
tracing deeply nested blocks. Show representative before/after excerpts and
describe what became easier to understand. No fixed nesting limit, function
length, comment quota or prettiness score replaces this human judgement.

Readability changes must preserve behaviour and be paired with meaningful test
evidence under [GI#304](https://github.com/sproates/panackelty/issues/304).
Check bootstrap, diagnostic/source-identity and performance implications when
applicable. This definition guides incremental refactoring; it does not require
unrelated rewrites in a focused fix or authorise new language syntax.

### Layout and whitespace

Required:

- Use UTF-8, LF line endings, two spaces per indentation level, no indentation
  tabs, no trailing whitespace and one final newline.
- Separate top-level declarations with one blank line. Keep related bindings
  together; use blank lines to distinguish meaningful phases inside a function.
- Put opening braces on the declaration or control-flow line. Align closing
  braces with the construct that opened them. Use `} else if condition {` and `} else {` for expanded branches.
- Use one space after commas and after annotation colons, and around assignment
  and binary operators. Do not pad parentheses, brackets, member dots or ranges:
  `name: Nat`, `Result[Nat, Str]`, `values[index]`, `value.first()`, `0..count`.
- Write new ordinary statements without optional semicolons. Keep each statement
  on its own line; do not join operations with semicolons.
- Put multi-step branches and loop bodies on multiple lines. A compact function
  or match arm is allowed only when its body is one short expression.

Preferred:

- Aim for at most 100 columns. Wrap at argument, field or element boundaries;
  use another two spaces for continuation indentation. Avoid deep alignment that
  shifts whenever an identifier changes. Long literal data, URLs and fixed
  diagnostic expectations may exceed the target rather than change their value.
- Expand long records/enums and argument lists with one member per line. Keep
  a comma between members; omit the final comma in new code for a consistent
  form across declarations, collections and match arms.
- Keep a short related expression together when wrapping would obscure it.
  Extract meaningful intermediate bindings rather than arbitrarily splitting
  complex expressions or introducing names for every operation.

```panack
record DivisionRequest {
  numerator: Nat,
  denominator: Nat
}

pure safe_divide(request: DivisionRequest): Result[Nat, Str] {
  if request.denominator == 0 {
    Error("division by zero")
  } else {
    Ok(request.numerator.divide(request.denominator))
  }
}

main(): Void {
  request = DivisionRequest(84, 2)
  print(safe_divide(request))
}
```

### Names and file organisation

Required:

- Use descriptive `snake_case` for functions, bindings, fields and source files;
  use `PascalCase` for types and enum variants. Use conventional type parameters
  such as `T` or meaningful short names when several parameters differ in role.
- Preserve established public names and compiler/ABI spellings. A style cleanup
  is not permission to rename an API, change a diagnostic or alter behaviour.
- Keep imports together at the start, followed by types and cohesive groups of
  functions. Put an application's `main` after its supporting declarations.
- Place code by responsibility using the component READMEs and
  [repository layout](../docs/internals/ARCHITECTURE.md#repository-layout). User-facing runnable
  programs belong in `examples/`; expected outputs belong in the test tree.

Preferred: name values by their meaning (`denominator`, `source_path`,
`failure_count`) rather than their representation (`number`, `data`, `thing`).
Short indices and mathematical names are appropriate in small, clearly bounded
algorithms. Give predicates names that describe the question they answer.
Keep functions cohesive; split at a named responsibility, not an arbitrary
line-count limit. Put public concepts before implementation helpers when that
improves discovery; language-required declaration ordering takes precedence.

### Imports and module boundaries

Required: use logical `stdlib/...` imports for public standard-library modules
and quoted relative imports for local source dependencies. Namespace examples
may use the supported `project/...` imports described in `docs/reference/SPEC.md`. Preserve
import order when loader identity, diagnostics or fixtures depend on it; do not
mechanically sort or deduplicate imports without validation. Namespace execution
is limited to the documented checked subset; do not imply package behavior or
general support beyond that contract.

Preferred: import the modules actually needed instead of `stdlib/prelude`.
Group standard-library imports and local imports separately, with stable ordering
inside a group where order is immaterial. Keep existing public stdlib prefixes
until an explicitly approved namespace migration; see
[standard-library contracts](../src/stdlib/README.md).

### Types, declarations and effects

Required: preserve function parameter/return contracts, explicit `pure` markers
and supported async/effect declarations. Use `mut` only for bindings that must
be reassigned. Keep exact numeric and checked domain semantics; do not introduce
lossy conversions or replace typed values with strings for convenience.

Preferred: infer obvious local values, but annotate empty collections,
ambiguous generic constructions and important domain boundaries. Retain useful
annotations when they explain an invariant. Use immutable bindings and pure
helpers where they fit the behaviour; isolate host operations from calculations.
Do not force purity by hiding effects or change the public effect contract.

```panack
pure identity[T](../value: T): T { value }

pure total(values: [Nat]): Nat {
  mut sum: Nat = 0
  for value in values {
    sum = sum + value
  }
  sum
}

main(): Void {
  empty: [Nat] = []
  print(total(empty))
  print(identity(42))
}
```

### Control flow, patterns and errors

Required: handle relevant `Option`/`Result` cases explicitly through exhaustive
matching or an appropriate documented helper. Preserve errors when they matter
to callers. A fallback must be part of the intended contract, not a way to conceal
unexpected failures. Do not introduce sentinel values, unchecked indexing or
traps to replace a recoverable error contract.

Use a final expression for a block's result where supported; do not discard a
meaningful result accidentally. Give each match arm its own line, with braces
for multiple operations. Preserve enum payload types and constructor arity.
Prefer `else if` for conditional chains rather than nesting `if` inside an
`else` block. Repeated arms and an optional final `else` are supported; a chain
without a final `else` is statement-only (`Void`). Prefer shallow control flow
and named predicates; extract a helper
when nested conditions obscure intent. Do not prescribe early-return or propagation syntax
that the supported compiler does not implement.

```panack
pure describe(result: Result[Nat, Str]): Str {
  match result {
    Ok(value) => "value ${value}",
    Error(message) => "error: ${message}"
  }
}

main(): Void {
  print(describe(Ok(42)))
  print(describe(Error("unavailable")))
}
```

Numeric equality must respect the actual operand types. Do not assume an `Int`
and a `Nat` compare as equal merely because both represent zero; use values in
an appropriate common domain and test boundary cases. Opaque paths and durations
should retain their checked constructors and typed operations.

### Comments, tests and examples

Required: comments must explain intent, invariants, bounds, ownership or
non-obvious trade-offs and stay accurate. Explain exported helpers' contracts
where their names/types are insufficient, especially failure, resource and
ordering behaviour. Preserve useful algorithm explanations; avoid narrating each
assignment. Document native/browser restrictions and the supported version.

Use descriptive test names and assertions of observable results, not merely
successful execution or raised hit counts. Check failure and boundary cases
relevant to the change. Host/process/network tests require bounded work,
deterministic inputs and explicit resource cleanup. Keep source and saved-bytecode
example expectations consistent with the functional runners. Follow
[testing conventions](../tests/README.md) rather than inventing another harness.

For each deliberate readability batch, improve or verify behavioural evidence
at the same time under [GI#304: Source readability and coverage](https://github.com/sproates/panackelty/issues/304).
Record what is asserted and what remains unmeasured. Execution coverage does not
establish assertion quality. Coverage-policy denominators and protected identities
may change only through a documented, reviewed baseline update.

### Exceptions, adoption and enforcement

- Intentionally invalid syntax, byte-exact diagnostic/source-span fixtures and
  parser/formatter test data keep their required spelling and layout. Explain
  deliberate exceptions in the fixture's documentation or PR.
- Generated sources and bootstrap artifacts follow their generator or regeneration
  procedure. Improve the generator where appropriate; do not hand-format fixed
  bytecode or break the bootstrap fixed point.
- Existing public API/ABI spellings remain compatible. A focused bug fix may keep
  surrounding legacy style; a readability batch should apply the standard to its
  agreed file scope without mixing unrelated semantic changes.
- Reviewers check this standard manually. No Panackelty formatter or complete
  style linter is claimed. Evaluate automation separately against syntax,
  comments, source mappings, invalid fixtures and bootstrap reproducibility.
- Change the standard through a focused PR with a rationale, checked examples
  and migration implications. Update this section rather than create competing
  rules in component READMEs. The specification remains the language authority;
  component guidance supplies API and ownership details.

## C, tests and repository structure

- Follow [VM editing and ownership conventions](../src/vm/README.md#ownership-and-editing-conventions)
  and `src/vm/.clang-format` for C: four spaces, braces, descriptive names and
  explicit ownership contracts. Format changed C code and inspect the diff;
  keep unrelated formatting out of the PR.
- Follow [ARCHITECTURE.md](../docs/internals/ARCHITECTURE.md#repository-layout) and component READMEs
  for file placement and responsibility boundaries. Keep implementation in
  `src/`, user examples in `examples/`, and tests in the documented test layout.
- Follow [tests/README.md](../tests/README.md) for fixtures, runners and assertions,
  and the validation requirements below for meaningful coverage and cleanup.
  Test names should describe the behavior or failure being checked.
- Follow [RELEASE_POLICY.md](../docs/releases/RELEASE_POLICY.md) for versions, tags, compatibility
  and release gates rather than defining a separate release naming scheme here.

These are review conventions, not a claim of automated enforcement. Existing
policy, formatting and validation tools cover only their documented scope.
