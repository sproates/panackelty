# Contributing to Panackelty

Panackelty welcomes focused bug reports, reduced failing programs, documentation
improvements, and implementation changes that advance the current roadmap.

## Before contributing

Read `README.md` for the source-build requirements, `SPEC.md` for accepted
language behavior, `ARCHITECTURE.md` for component boundaries, and `ROADMAP.md`
for current priorities. Security-sensitive reports must follow `SECURITY.md`
instead of using a public issue.

The developer preview has shipped; implementation priorities are being assessed.
Follow the [roadmap decision process](docs/ROADMAP_PROCESS.md) and discuss
substantial features before implementing them. Use the **Roadmap proposal** issue
form for ideas or bounded investigations; an open issue is not a commitment.
The repository records agreed direction and priorities, with Issues providing
linked work details and discussion.

## Conventions

These conventions apply to new and substantially changed work. Preserve existing
public names and keep unrelated formatting or renaming out of focused changes.
Language syntax and semantics remain defined by [SPEC.md](SPEC.md).

### Branches, commits and pull requests

- For core feature work, branch from current `next` and target the PR to `next`,
  using `<kind>/<short-kebab-case-description>`.
  Use `feat/`, `fix/`, `docs/`, `refactor/`, `test/`, `ci/` or `chore/`
  for the principal purpose; use `chore/` for maintenance that does not fit
  another kind. For example: `feat/compiler-name-suggestions` or
  `docs/adoption-direction-and-issue-workflow`. An issue number is optional:
  `fix/123-import-collision`. Existing branches need not be renamed.
- Use focused commits with a short imperative subject describing the result,
  such as “Explain unresolved names with nearby suggestions”. Add a body when
  the reason or trade-off is not obvious. Conventional Commit prefixes are
  not required. Use your GitHub noreply identity and verify it before committing.
- Give PRs a descriptive title and explain the problem, resulting behavior,
  scope, validation and material limitations. Link related issues and roadmap
  items; reserve automatic issue-closing links for work completed on merge.
- Include a proportionate [performance assessment](docs/ROADMAP_PROCESS.md#performance-impact-and-regression-decisions)
  for substantive deliveries: relevant evidence and comparability limits, or a
  reasoned no-impact statement. Present unresolved regressions and their explicit
  disposition; passing correctness checks alone do not establish performance acceptance.
- Keep each PR reviewable around one coherent outcome. Update affected docs,
  report actual test results and preserve unrelated work. Follow the
  [roadmap process](docs/ROADMAP_PROCESS.md) for priorities and decisions.
- Obtain independent review of substantive delivery revisions before merge approval;
  record the reviewed commit or tree, findings and resolution under the
  [review rules](docs/ROADMAP_PROCESS.md#independent-review).
- Never push directly to `main` or `next`; integrate through PRs and required
  checks. Maintainers merge agreed-scope core work into `next` under the active
  integration workflow. Do not enable auto-merge. Promotions to `main` require
  a separate PR and explicit user approval.

### Panackelty source

This section is the authoritative coding standard for hand-written `.panack`
source. It applies to compiler, tools, libraries, tests and examples. The
[specification](SPEC.md) defines legal syntax and behaviour; this standard defines
how we present that syntax for readers. Examples below target the current `next`
compiler using bytecode v9; namespace execution remains gated.

**Required** rules are review requirements for new code and deliberate readability
batches. **Preferred** rules allow a clearer alternative with a brief explanation
in the PR. Small fixes may preserve surrounding layout; migrate existing files
incrementally with their tests rather than reformatting the repository at once.
Public names, source-location fixtures and generated artifacts require the
exceptions below.

#### Readable Panackelty

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

#### Layout and whitespace

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
    Ok(quotient(request.numerator, request.denominator))
  }
}

main(): Void {
  request = DivisionRequest(84, 2)
  print(safe_divide(request))
}
```

#### Names and file organisation

Required:

- Use descriptive `snake_case` for functions, bindings, fields and source files;
  use `PascalCase` for types and enum variants. Use conventional type parameters
  such as `T` or meaningful short names when several parameters differ in role.
- Preserve established public names and compiler/ABI spellings. A style cleanup
  is not permission to rename an API, change a diagnostic or alter behaviour.
- Keep imports together at the start, followed by types and cohesive groups of
  functions. Put an application's `main` after its supporting declarations.
- Place code by responsibility using the component READMEs and
  [repository layout](ARCHITECTURE.md#repository-layout). User-facing runnable
  programs belong in `examples/`; expected outputs belong in the test tree.

Preferred: name values by their meaning (`denominator`, `source_path`,
`failure_count`) rather than their representation (`number`, `data`, `thing`).
Short indices and mathematical names are appropriate in small, clearly bounded
algorithms. Give predicates names that describe the question they answer.
Keep functions cohesive; split at a named responsibility, not an arbitrary
line-count limit. Put public concepts before implementation helpers when that
improves discovery; language-required declaration ordering takes precedence.

#### Imports and module boundaries

Required: use logical `stdlib/...` imports for public standard-library modules
and quoted relative imports for local source dependencies. Preserve import order
when loader identity, diagnostics or fixtures depend on it; do not mechanically
sort or deduplicate imports without validation. Ordinary executable imports
currently share a program-wide namespace. Do not present planned package or
namespace syntax as executable conventions.

Preferred: import the modules actually needed instead of `stdlib/prelude`.
Group standard-library imports and local imports separately, with stable ordering
inside a group where order is immaterial. Keep existing public stdlib prefixes
until an explicitly approved namespace migration; see
[standard-library contracts](src/stdlib/README.md).

#### Types, declarations and effects

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
pure identity[T](value: T): T { value }

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

#### Control flow, patterns and errors

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

#### Comments, tests and examples

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
[testing conventions](tests/README.md) rather than inventing another harness.

For each deliberate readability batch, improve or verify behavioural evidence
at the same time under [GI#304: Source readability and coverage](https://github.com/sproates/panackelty/issues/304).
Record what is asserted and what remains unmeasured. Execution coverage does not
establish assertion quality. Coverage-policy denominators and protected identities
may change only through a documented, reviewed baseline update.

#### Exceptions, adoption and enforcement

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

### C, tests and repository structure

- Follow [VM editing and ownership conventions](src/vm/README.md#ownership-and-editing-conventions)
  and `src/vm/.clang-format` for C: four spaces, braces, descriptive names and
  explicit ownership contracts. Format changed C code and inspect the diff;
  keep unrelated formatting out of the PR.
- Follow [ARCHITECTURE.md](ARCHITECTURE.md#repository-layout) and component READMEs
  for file placement and responsibility boundaries. Keep implementation in
  `src/`, user examples in `examples/`, and tests in the documented test layout.
- Follow [tests/README.md](tests/README.md) for fixtures, runners and assertions,
  and the validation requirements below for meaningful coverage and cleanup.
  Test names should describe the behavior or failure being checked.
- Follow [RELEASE_POLICY.md](RELEASE_POLICY.md) for versions, tags, compatibility
  and release gates rather than defining a separate release naming scheme here.

These are review conventions, not a claim of automated enforcement. Existing
policy, formatting and validation tools cover only their documented scope.

## Validate a change

Website source, previews, release promotion and publishing belong in the
[website repository](https://github.com/sproates/panackelty-website). Follow its contributor and validation instructions.

Run `bash scripts/validate_change.sh --plan origin/main` to inspect the affected
components and required checks, then use `--run` to execute the selected local
route. It includes branch changes plus staged, unstaged and untracked edits.
For changes limited to the informational/process files in `scripts/ci_docs.sh`,
this runs `make docs`; review the content as well. CI checks those documents and local file
links without building the toolchain. See
[change-aware CI](tests/README.md#change-aware-ci) for the exact boundary.

For implementation, specification, packaged-input, workflow, mixed or unknown
changes, run from the repository root:

```sh
make check
```

This is the canonical validation command and includes unit tests, complete
program tests through `panack`, and the reproducible-bootstrap proof. Add focused
unit tests for changed internals and functional tests for observable behavior.

Development uses C, Panackelty and POSIX tools. `make policy` rejects interpreter
dependencies; `make check-no-interpreter` repeats the complete development and
package workflow with an allowlisted command environment. See
[the testing guide](tests/README.md) for the validation commands.

### Documentation and cleanup

Update the documents affected by a change: language contracts in `SPEC.md`,
component boundaries in `ARCHITECTURE.md`, user workflows in `README.md`, bootstrap
progress in `SELF_HOSTING.md`, delivery state in `ROADMAP.md`, and test evidence in
`tests/COVERAGE.md`. Keep component READMEs accurate. Explain when documentation
was reviewed and no update was needed.

Remove obsolete files, callers, fixtures and references in the same change.
Preserve unrelated work and user-owned configuration. Remove generated outputs
with `make clean` where applicable, then inspect `git status` and the final diff.
Never weaken required coverage to make validation pass. If checks cannot run,
identify the limitation and unverified behavior.

A clean `make check` has a 120-second reference-environment budget; a focused
incremental check with the toolchain built has a 15-second budget. Report timing,
apply each budget to its stated workload, and follow the performance disposition
process for breaches. Preserve required coverage when improving validation speed.

Keep the VM as the only execution engine, source compilation before execution,
the purity boundary, exact numeric semantics, bytecode verification and runtime
safety checks. Preserve the stable `panack` command unless the agreed change
explicitly alters it.

## Report a bug

Open the repository's **Issues** page, select **New issue**, and choose
**Bug report**. The structured form requires the output of `panack --version`,
the host operating system and processor, the smallest complete `.panack` program
that reproduces the problem, the command used, and the complete standard output
and error output. It also asks what you expected and whether the issue occurs
after compiling the program to bytecode.

Do not include secrets or private data. Suspected vulnerabilities belong in the
private reporting channel described by `SECURITY.md`, not in a public bug report.
