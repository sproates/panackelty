# Panackelty roadmap

This roadmap tracks post-bootstrap language and engineering initiatives. The
completed compiler bootstrap and removal of Python from the public toolchain
are recorded in [SELF_HOSTING.md](SELF_HOSTING.md).

An item is complete only when its implementation, focused tests, end-to-end
coverage, and affected documentation are complete and `make check` passes.

## Deliver developer preview `0.1.0-alpha.1` — delivered

The immediate product goal is a public developer preview that lets a new user
download Panackelty, put `panack` on `PATH`, and check, compile, and run a source
file without Python, `make`, a C compiler, or a source checkout. The preview is
an explicitly experimental release rather than a claim of language or bytecode
stability.

The initial target matrix is Linux x86-64 and macOS arm64. Each target remains
in the matrix only if its final downloadable artifact can be built and exercised
on that platform in release automation. Windows, additional architectures, and
package-manager distribution must not delay the preview.

Python removal from the development repository is not a preview prerequisite.
The shipped compiler, VM, standard library, installation path, package path,
and release smoke tests must remain Python-free; contributors may continue to
use the transitional Python oracle and test harness described below.

### 1. Freeze the preview contract

- [x] Declare the release version `0.1.0-alpha.1` and document what the `alpha`
      stability level promises for source syntax, standard-library APIs, CLI
      behavior, and bytecode compatibility
- [x] Record Linux x86-64 and macOS arm64 as the initial supported targets,
      including the oldest tested operating-system versions and the policy for
      best-effort behavior elsewhere
- [x] Publish the deliberately postponed language features and known test or
      implementation limitations as preview limitations rather than implicit
      promises
- [x] Freeze unrelated language feature work until the preview release gates
      below are satisfied

### 2. Establish the public project boundary

- [x] Choose and add the source and binary distribution license
- [x] Confirm that the project name, documentation, examples, and supplied
      photograph may be published under the chosen terms
- [x] Add concise security reporting, contribution, support, and release-notes
      documents appropriate to an experimental compiler and native runtime
- [x] Audit the publishable tree for secrets, personal data, local configuration,
      generated output, accidental binaries, and material that should remain
      private
- [x] Ensure the license, release notes, and required notices are present in
      both the source repository and every binary archive

### 3. Version the complete toolchain

- [x] Define one canonical source of the Panackelty release version
- [x] Add `panack --version` with focused and public-CLI tests, reporting the
      release version, bytecode version, and enough build provenance to identify
      a published artifact
- [x] Keep version injection deterministic so the stage-2/stage-3 fixed-point
      proof and reproducible package build remain meaningful
- [x] Name artifacts with release, operating system, and architecture, for
      example `panackelty-0.1.0-alpha.1-macos-arm64.tar.gz`

### 4. Produce download-and-run archives

- [x] Make the installed toolchain self-contained, including the launcher,
      native VM, compiler bytecode, standard-library modules, and resource
      discovery outside a source checkout
- [x] Replace the archive's installation-shaped `usr/local` root with a friendly,
      relocatable top-level `panackelty/` directory containing `bin`, `libexec`,
      `share`, documentation, and license files
- [x] Build an archive independently for every supported target without Python
      or other development-only tools in the resulting artifact
- [x] Verify that moving the extracted directory does not break compiler or
      standard-library discovery
- [x] Publish SHA-256 checksums and build provenance beside every archive

### 5. Add exact-artifact release gates

- [x] Add a release smoke test that starts from the final archive rather than
      the source or staging installation tree
- [x] On every supported target, unpack the archive into a fresh directory with
      no repository checkout, Python, `make`, or C compiler available
- [x] Require the unpacked toolchain to report its version and help, check a
      source file, compile it, run source and bytecode, pass program arguments,
      import the bundled standard library, and reject malformed bytecode
- [x] Require `make check`, the fixed-point bootstrap proof, native conformance,
      and the exact-artifact smoke test before a release tag can publish assets
- [x] Make the tag-driven release workflow upload only artifacts and checksums
      produced by successful matrix jobs

### 6. Make the first-user workflow usable

- [x] Put a download-and-run quick start before build-from-source instructions
      in `README.md`, covering archive selection, extraction, `PATH`, the first
      program, checking, compilation, execution, upgrade, and removal
- [x] Add a compact language tour that links each preview feature to a runnable
      example and its relevant specification section
- [x] Add at least a primary `file:line:column` location to lexer, parser, name,
      and type diagnostics so a new user can find the reported error; richer
      excerpts, multiple diagnostics, stable codes, and automated fixes may
      follow the preview
- [x] Test the published quick start literally in a clean shell and require its
      stated output to match
- [x] Give preview users a clear place and template for actionable bug reports,
      including `panack --version`, host platform, source input, and output

### 7. Publish and verify

- [x] Export the reviewed working tree into a new isolated repository without
      the current `.git` directory, branches, tags, reflogs, remotes, or other
      local history; keep this working repository and its history intact
- [x] Configure publication identity and authenticate the separate personal
      GitHub account only in that isolated repository, without changing global
      Git configuration, the current GitHub login, or files under `~/.ssh`
- [x] Create and inspect one clean initial commit, publish the repository, then
      clone it into a fresh directory and run the documented contributor checks
      plus `make package PYTHON=false`
- [x] Publish the annotated `0.1.0-alpha.1` tag and release only after its platform
      matrix and exact-archive gates pass
- [x] Download each public release asset by its published URL, verify its
      checksum, repeat the quick start, and record the evidence in the release
      notes

The developer preview is delivered only when an unaffiliated user can follow
the public quick start on a supported machine and reach this workflow using only
the downloaded archive:

```sh
panack --version
panack check hello.panack
panack compile hello.panack
panack run hello.bc
```

The preview does not require Windows support, a single-file executable,
package-manager installation, repository-wide Python removal, generic functions,
new automation APIs, complete diagnostic rendering, or a backwards-compatibility
guarantee. Those remain independent follow-up initiatives.

## Language direction and differentiation — exploration

Panackelty should combine strong static guarantees with a low-friction programming
experience. Powerful checking is useful only when programmers can understand a
failure and act on it quickly. New features should therefore be evaluated on
both the guarantees they provide and the clarity of the resulting workflow.

### Type inference and diagnostic experience

- [ ] Design automatic local type inference while retaining explicit annotations
      where they document public APIs, resolve ambiguity, or express a contract
- [ ] Specify which types, generic arguments, effects, and guarded facts may be
      inferred without making compilation unpredictable
- [ ] Preserve principal, deterministic results and provide an explicit escape
      hatch when inference is ambiguous
- [ ] Define a structured diagnostic model with stable error codes, primary and
      secondary source spans, inferred-versus-expected types, and causal chains
- [ ] Make type errors explain the mismatch in source terms and suggest a concrete
      fix when the compiler can do so safely
- [ ] Add machine-applicable fixes for unambiguous cases and test that applying a
      suggested fix produces a valid program
- [ ] Build a diagnostic conformance suite covering usefulness, source accuracy,
      recovery after an error, and avoidance of misleading follow-on errors

### Predictable deferred computation

Explore `lazy` as a narrow, explicit form of call-by-need evaluation. The first
form should be a typed local binding whose pure initializer is evaluated on its
first read and then memoized. This can avoid unnecessary expensive work without
making I/O timing implicit or committing the language to general closures,
lazy parameters, or lazy collections.

```panackelty
lazy report: Str = build_report(records)

if should_save {
  write_file("report.txt", report)
}
```

- [ ] Specify the syntax, typing, scope, forcing behavior, and at-most-once
      memoization semantics of `lazy` bindings
- [ ] Require lazy initializers to be pure so reading an ordinary value cannot
      unexpectedly perform I/O or another visible effect
- [ ] Define capture semantics conservatively, initially allowing references to
      immutable values while rejecting dependencies on mutable local bindings
- [ ] Specify deterministic handling of initializer traps, including whether a
      failed evaluation is memoized, and diagnose cyclic forcing explicitly
- [ ] Design an internal thunk representation without exposing general closure
      or capture semantics as part of the callable-value model
- [ ] Define bytecode instructions and verifier rules for constructing, forcing,
      and caching lazy values, including result-type and state validation
- [ ] Add compiler, verifier, VM, bootstrap, and public-CLI conformance coverage,
      including unused bindings, repeated reads, traps, cycles, and invalid
      effectful or mutable captures
- [ ] Evaluate lazy parameters, module-level bindings, and lazy collection
      elements separately after representative programs demonstrate a need

### Candidate differentiator: contract-driven automation

The strongest current direction is to make Panackelty a contract-driven language for
reliable automation: programs describe data, effects, and behavioral boundaries
in forms the compiler, test runner, and tooling can all understand. This builds
on guarded types, purity, exact values, and the VM instead of adding an unrelated
headline feature.

- [ ] Explore first-class function and module contracts with preconditions,
      postconditions, invariants, and effect expectations
- [ ] Define which contracts are proven statically, checked at runtime, or used
      to generate tests, and make that boundary visible to the programmer
- [ ] Design built-in contract testing, including generated boundary cases,
      reusable fixtures, deterministic execution, and useful counterexamples
- [ ] Integrate linting and static analysis into the compiler and stable CLI,
      sharing its parser, type information, effects, contracts, and diagnostics
- [ ] Support project policies that can promote selected analyses from advice to
      compilation errors without making default builds noisy
- [ ] Prototype automation-oriented standard-library modules, beginning with
      structured data, HTTP, paths, processes, and browser automation
- [ ] Evaluate browser automation against a real end-to-end program before
      committing to a large ecosystem surface

### Data and target-platform experiments

- [ ] Design JSON literals with unambiguous syntax, exact numeric behavior,
      duplicate-key rules, interpolation, and predictable inferred types
- [ ] Decide whether JSON literals produce a dynamic `Json` value, inferred
      structural data, nominal records, or an explicitly selected representation
- [ ] Prototype JSON parsing, validation, and typed decoding as one coherent API
- [ ] Investigate an optional browser target, including VM portability, DOM and
      Web API bindings, sandboxing, asynchronous effects, artifact size, and
      source-level debugging
- [ ] Compare a WebAssembly-hosted Panackelty VM with direct code generation before
      selecting a browser execution model
- [ ] Keep browser execution optional so terminal programs and the native seed VM
      do not inherit unnecessary platform complexity
- [ ] Define ecosystem and standard-library contribution criteria around API
      stability, deterministic tests, security review, and long-term ownership

Before promoting an experiment into the language specification, require a
representative program, a written semantics proposal, implementation and
maintenance estimates, and evidence that it strengthens Panackelty's identity more
than an ordinary library would.

## Ergonomic control flow and collection APIs — complete

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

## Make imports independent of repository paths — complete

User programs and examples no longer need to know the source-tree location of
the standard library. The canonical `import stdlib/option` and
`import project/shared/module` forms use reserved logical namespaces;
file-relative imports remain quoted. A terminal `.panack` suffix and quoted
logical paths are accepted compatibility spellings. The launcher supplies the
toolchain-owned library root, including from installed layouts, while the
project root is the entry source file's directory.

- [x] Choose and specify the canonical logical-import syntax, including whether
      quotes and the `.panack` suffix are required, optional, or distinguish
      logical imports from file-relative imports
- [x] Define deterministic resolution rules for file-relative, project-local,
      and standard-library modules without depending on the process working
      directory
- [x] Define project-root discovery, search precedence, shadowing, ambiguity,
      path traversal, canonical identity, and useful missing-module diagnostics
- [x] Make the compiler locate bundled standard-library modules in both a source
      checkout and an installed or packaged toolchain
- [x] Preserve load-once and cycle-detection behavior when the same module is
      reachable through different valid import spellings
- [x] Implement the accepted syntax and resolution rules in every compiler and
      loader that remains part of the development and bootstrap workflow
- [x] Migrate examples, tests, compiler sources where appropriate, and
      documentation away from repository-relative standard-library paths
- [x] Add focused and public-CLI coverage for logical standard-library imports,
      project-local imports, installed layouts, ambiguity and shadowing, missing
      modules, invalid paths, cycles, and source/bytecode execution

## Expand automation and host capabilities — planned

Panackelty should gain the general host capabilities needed by dependable
automation programs before its test suite is moved away from Python. These APIs
must be useful outside the test harness, remain visibly effectful, behave
predictably across supported platforms, and expose structured failures rather
than test-specific shortcuts. Logical standard-library imports are a
prerequisite so programs can use these APIs without knowing repository paths.

- [ ] Specify a coherent process API for executable selection, arguments,
      standard input, working directory, environment overrides, exit status,
      and captured standard output and error
- [ ] Make process streams byte-oriented with explicit checked UTF-8 decoding,
      define resource and output limits, and prevent deadlocks when both output
      streams are active
- [ ] Specify portable directory enumeration, directory creation, file metadata,
      removal, and recursive operations with deterministic ordering and clear
      symbolic-link and failure behavior
- [ ] Add collision-safe temporary-file and temporary-directory creation with
      explicit ownership, cleanup, and failure semantics
- [ ] Separate wall-clock time from a monotonic elapsed-time API suitable for
      validation budgets and performance measurements
- [ ] Define the supported-platform and capability policy for behavior such as
      permissions that cannot be represented consistently on every host; do not
      add a universal operation solely for a platform-specific test
- [ ] Implement the accepted host boundary in the native VM and every
      transitional runtime still required for differential validation
- [ ] Add typed standard-library wrappers that keep all process, filesystem,
      temporary-resource, and clock operations effectful
- [ ] Add focused, cross-runtime, and public-CLI conformance coverage, including
      large simultaneous process streams, invalid UTF-8, missing executables,
      environment and working-directory isolation, cleanup failures, path
      traversal, resource limits, and monotonic timing
- [ ] Build a small Panackelty testing library with assertions, structured test
      results, fixture discovery, temporary isolation, command assertions, and
      deterministic reporting as the foundation for Python removal

## Eliminate Python from the repository — planned after host capabilities

Python has been removed from the public toolchain, but it still implements the
transitional development oracle, test harness, compatibility facade, and seed
regeneration command. Retire those uses after logical imports, the required host
capabilities, and the Panackelty testing foundation are complete. Completion
means the current repository contains no Python source or Python command
invocation and its full development, bootstrap, conformance, packaging, and
release validation succeeds on a machine where no Python interpreter is
installed.

- [ ] Complete and document the replacement test architecture: use the
      Panackelty-hosted library for compiler, language, standard-library, and
      functional behavior; focused C tests for native VM internals; and portable
      declarative fixtures shared between them
- [ ] Port compiler, bytecode, verifier, VM, runtime, and standard-library unit
      coverage without losing focused assertions or important failure cases
- [ ] Port functional-test discovery, subprocess orchestration, environment and
      file fixtures, output comparisons, and exit-status assertions
- [ ] Replace differential reliance on the Python compiler and VM with portable
      golden artifacts, contract tests, native/self-hosted cross-checks, and
      fixed-point bootstrap evidence
- [ ] Replace `regenerate-seed` with a documented staged self-hosted process that
      verifies its input seed and resulting compiler artifacts
- [ ] Remove the root compatibility facade and the transitional implementation
      under `src/bootstrap`
- [ ] Remove Python variables, commands, cache cleanup, and file-pattern handling
      from the Makefile and other development scripts
- [ ] Remove Python setup and execution from CI
- [ ] Update architecture, bootstrap, contributor, test, and user documentation
      so none describes Python as a current project component
- [ ] Add a repository policy check that rejects Python source files, Python
      shebangs, and Python command invocations
- [ ] Prove `make check`, native conformance, bootstrap verification, packaging,
      and release smoke tests from a clean environment without Python

## Keep validation within development budgets — in progress

Validation speed is an internal nonfunctional requirement because slow feedback
discourages frequent checking and compounds the cost of every implementation
change. On the reference CI or development environment, a clean `make check`
should finish within 120 seconds and a focused incremental check with an already
built native toolchain should finish within 15 seconds. Exceeding a budget must
produce a visible warning and a tracked follow-up rather than silently becoming
the new baseline. Coverage must not be weakened to meet either budget.

The suite reports stable per-phase and total wall-clock timings. A September
2026 clean run after adding source locations completed its unit phase in 13
seconds, functional phase in 80 seconds, and complete `make check` in 132
seconds. The functional and complete phases therefore exceed their 75- and
120-second budgets; the warnings remain visible until the regression is
removed.
Component-focused compiler, bytecode, and VM checks retain representative
public-CLI coverage. CI publishes and archives each timing row.

- [x] Add stable wall-clock timing for the complete suite and its unit,
      functional, and bootstrap phases
- [x] Emit a warning when a clean `make check` exceeds 120 seconds or a focused
      incremental check exceeds 15 seconds
- [x] Define fast, component-focused incremental targets that preserve the
      relevant internal and end-to-end evidence for a change
- [x] Run the fixed-point bootstrap proof exactly once per complete validation
- [x] Compile the self-hosted compiler once per validation and safely reuse its
      checked artifact across compatible functional cases
- [x] Remove redundant semantic compilation while retaining representative
      coverage of every public CLI workflow and failure behavior
- [x] Record timing trends in CI so regressions are visible before they compound
- [x] Reach both budgets without skipping, weakening, or relocating required
      coverage outside the canonical validation workflow
- [x] Restore the functional phase below its 75-second budget by running each
      program's source and compiled forms in one balanced worker task,
      parallelizing independent invalid cases, and reusing the already-verified
      stage-2 artifact for the compiler program's compiled execution
- [ ] Recover the clean validation budgets after file-aware token and expression
      positions increased self-hosted compiler build time, without reducing
      fixed-point, functional, or diagnostic coverage

## Harden and expand test coverage — in progress

The goal is to make regressions difficult to introduce and failures easy to
localize while keeping `make check` the canonical validation command.

- [x] Separate internal unit tests from black-box functional program tests
- [x] Discover functional cases and example expectations without a central
      Python manifest
- [x] Inventory the behavior promised by `SPEC.md` and map it to existing tests
- [ ] Add focused success and failure tests for every language construct and
      runtime built-in
- [x] Cover every CLI command and shorthand through end-to-end subprocess tests
- [ ] Expand type, refinement, purity, and name-resolution diagnostic coverage
- [ ] Exercise file, import, malformed-input, and operating-system failure paths
- [ ] Expand malformed and adversarial bytecode verifier and VM coverage
- [ ] Add deterministic compilation and bytecode round-trip tests
- [ ] Add comprehensive tests for the Panackelty-hosted compiler components
- [ ] Establish a useful coverage baseline and record intentionally uncovered
      host-boundary code
- [x] Organize the suite so focused failures remain fast and the full suite stays
      practical to run after every change

Testing work that is also a prerequisite for self-hosting should be reflected
in both roadmaps when completed.

## Change Panackelty syntax — complete

The accepted syntax removes redundant declaration keywords, uses a colon for
function return types, and distinguishes no-return functions with `Void`.

| Concern | Legacy syntax | Current syntax |
| --- | --- | --- |
| Function | `fn answer(): Nat` | `answer(): Nat` |
| Pure function | `pure fn answer(): Nat` | `pure answer(): Nat` |
| Immutable binding | `let answer: Nat = 42;` | `answer: Nat = 42` |
| Mutable binding | `let mut total: Nat = 0;` | `mut total: Nat = 0` |
| No returned value | `main(): Unit { () }` | `main(): Void {}` |
| Result failure | `Err(message)` | `Error(message)` |

- [x] Record the goals and non-goals of the syntax change
- [x] Write representative before-and-after examples
- [x] Draft and specify the revised lexical and grammar rules
- [x] Resolve declaration ambiguity by retaining mandatory type annotations
- [x] Adopt a clean break and reject the legacy syntax
- [x] Update `SPEC.md` with the accepted syntax and `Void` semantics
- [x] Update the bootstrap lexer, parser, checker, compiler, VM, and tests
- [x] Apply the same syntax to the Panackelty-hosted compiler sources
- [x] Update every example and user-facing command snippet
- [x] Advance the bytecode version for the `Void` value-tag change
