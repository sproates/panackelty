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
      recovery across errors, stable codes, and automated fixes may
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

- [x] Add source excerpts and carets to existing positioned compiler errors,
      preserving imported-module ownership and source snapshots; cover tabs,
      Unicode, CRLF, EOF, and header-only fallback in renderer and CLI tests

- [x] Implement initializer-based local inference with optional annotations,
      fixed types, immutable defaults, explicit `mut`, and no shadowing;
      preserve numeric defaults, guarded types, and callable effects
- [x] Reject unresolved local types at their declaration with an annotation hint;
      resolve nested generic evidence consistently within one initializer
- [ ] Expand inference beyond this first local implementation: specify systematic
      expected-type propagation through calls, constructors, collections, and
      future callbacks; avoid unrelated one-off inference exceptions
- [ ] Evaluate explicit inference variables and constraint solving before allowing
      later uses or assignments to resolve incomplete local types; decide whether
      declaration-local resolution remains the default language rule
- [ ] Evaluate delayed numeric defaulting, generic function inference, and effect
      inference separately, retaining explicit public API and domain contracts
- [ ] Specify determinism, principal types where applicable, ambiguity escape
      hatches, diagnostic quality, and compile-time budgets for each expansion
- [ ] Add unused-binding diagnostics to help catch misspelled assignments that
      become new immutable declarations under plain `=` syntax
- [ ] Define a structured diagnostic model with stable error codes, primary and
      secondary source spans, inferred-versus-expected types, and causal chains
- [ ] Make type errors explain the mismatch in source terms and suggest a concrete
      fix when the compiler can do so safely
- [ ] Add machine-applicable fixes for unambiguous cases and test that applying a
      suggested fix produces a valid program
- [ ] Build a diagnostic conformance suite covering usefulness, source accuracy,
      recovery after an error, and avoidance of misleading follow-on errors

### Core and standard-library types — planned exploration

Keep the primitive type set small while making common terminal-program concepts
explicit in the standard library. Unfinished entries below are proposals;
completed entries describe accepted features. Prefer portable records and tagged unions; add compiler
or VM support only where representation, checking, or the host boundary requires
it. Coordinate generic-function work with `SELF_HOSTING.md`, JSON work with
the JSON data support initiative below, and host types with the automation and
host capabilities initiative.

Generic source functions, a first-class success value, and exact rational
arithmetic are implemented. Opaque paths, exact durations, and monotonic instants
are now implemented, together with typed filesystem and bounded process APIs,
checked decoding, and sleep. The testing-library foundation is complete;
repository-wide Python removal is the next migration initiative. Recursive
filesystem operations remain a separate follow-up.

- [x] Specify and implement generic source functions and explicit type arguments,
      including inference, ambiguity diagnostics, and purity preservation, so
      reusable `Option[T]` and `Result[T,E]` helpers need fewer compiler special
      cases

The first generic-function implementation checks abstract bodies once and erases
type arguments into ordinary version-8 calls. It includes inferred and explicit
complete type arguments, recursion, and portable Option/Result/array helpers.
Constraints, generic function references, partial type arguments, and inference
from expected return types remain deferred.

- [x] Implement first-class singleton `Unit`, written `()`, including generic
      success payloads, collections, and callbacks; keep return-only `Void`
      distinct and lower Unit construction through a pure bytecode-8 builtin
- [x] Implement opaque `Path` with checked text/native-byte construction, lexical
      operations, checked UTF-8 conversion, and escaped display on supported POSIX
      hosts, without implying existence or safe containment
- [x] Implement exact signed nanosecond `Duration` and opaque monotonic `Instant`,
      pure arithmetic, checked fractional conversion, and effectful clock reads
- [x] Add typed filesystem queries and I/O accepting `Path`, returning structured
      errors; preserve arbitrary native filenames through enumeration and access
- [x] Add sleep and timeout APIs with negative-duration and host-range validation;
      evaluate a portable system-suspension policy before promising deadline
      behavior across suspended hosts
- [x] Design structured filesystem and process errors returned through `Result`,
      retaining useful operation and failure details; distinguish a process's
      nonzero exit status from failure to launch it
- [ ] Evaluate a standard-library `Json` tagged union for unvalidated external
      data, with parsing and checked conversion to application records; settle
      numeric representation and recursive-type requirements through the JSON
      initiative rather than introducing an unrestricted dynamic value type
- [ ] Complete explicit decimal rounding with specified precision or scale and
      rounding modes, preserving exact existing arithmetic and requiring an
      explicit choice for non-terminating decimal division
- [x] Implement exact `Rat` with integer `/`, normalized arbitrary-precision
      fractions, arithmetic and comparisons, zero-divisor traps, exact `.nat()`
      and `.dec()` conversions, and explicit natural `quotient` division
- [ ] Extend rational-to-decimal conversion with explicit scale or precision and
      rounding modes alongside decimal rounding; add checked conversion helpers
- [ ] Evaluate tuples for temporary pairs and multiple return values only when
      examples demonstrate a meaningful benefit over named records
- [ ] Explore calendar dates and wall-clock time as a separate library design,
      with explicit timezone and calendar semantics rather than reusing
      monotonic `Instant`

Defer binary floating point, fixed-width integer families, a character primitive,
and a universal `Any` type until concrete programs justify their semantics and
maintenance cost. Each accepted addition needs a written semantics proposal,
representative programs, focused failure tests, public-CLI coverage, and any
required cross-VM and bootstrap evidence before it becomes a supported feature.

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

### Candidate differentiator: explainable values — exploration

Explore built-in value provenance: an opt-in way to explain a result through the
inputs, calculations, function calls, and branch decisions that produced it.
This builds on exact arithmetic, purity, persistent values, and the single VM
execution model. The intended benefit is practical debugging and inspectable
numerical results: users can ask where a total came from or which condition
selected a value.

`explain(value)` is a working design sketch, not accepted syntax or an available
feature. Tracking must start before the relevant calculation; explanations cannot
recover execution history that was never recorded. Explanations describe recorded
execution dependencies, not a proof that the program's business logic is correct.

- [ ] Specify an opt-in first version covering scalar calculations, function
      arguments and results, and the branch conditions that selected a result
- [ ] Decide the source and CLI interface for enabling tracking and requesting
      explanations, including how explanation output respects the purity boundary
- [ ] Design VM dependency records and compiler source mappings while preserving
      exact values, ordinary program behavior, and bytecode verification
- [ ] Render concise explanations with values, operations, decision outcomes, and
      source locations; validate usefulness against an incorrect invoice total
      and a value selected by an unexpected branch
- [ ] Define tracking scope, retention, and memory limits for loops, recursion,
      and collections; report truncated or unavailable history explicitly
- [ ] Define treatment of sensitive inputs and redaction before explanations can
      be saved or shared; avoid exposing input contents by default
- [ ] Measure execution and memory overhead with tracking enabled and disabled
      before deciding whether to expand the initial scope
- [ ] Add compiler, verifier, VM, and public-CLI coverage for explanation accuracy,
      control dependencies, source locations, limits, and unchanged value semantics
- [ ] Evaluate later extensions separately: structured input origins such as CSV
      rows and columns, collection provenance, exported explanations, and comparison
      of recorded dependencies across runs

Related work includes [language-integrated provenance in Links](https://arxiv.org/abs/1607.04104)
and [Whyline for Java](https://www.cs.cmu.edu/~NatProg/whyline-java.html).
The proposed distinction is approachable explanations for ordinary calculations
and control flow in the standard language toolchain, not a claim to have invented
provenance or causal debugging.

### Candidate differentiator: previewable effects — exploration

Explore a VM-enforced preview mode for dependable scripts. A program would
produce an inspectable plan of supported changes before applying them, including
content diffs and the inputs on which those changes depend. Proposed commands
such as `panack plan script.panack -o changes.plan` and
`panack apply changes.plan` are design sketches, not available CLI features.

- [ ] Specify an opt-in first version for local file reads and writes, with
      staged writes in a simulated filesystem so subsequent reads observe them
- [ ] Define the supported host effects and enforce coverage at the VM boundary,
      including nested execution; stop explicitly on unsupported effects rather
      than silently executing external commands or remote mutations
- [ ] Define permitted reads during planning and handling of terminal output,
      environment values, arguments, path resolution, and filesystem queries
- [ ] Render file changes and content diffs and save the concrete operations for
      later application without rerunning the program against new inputs
- [ ] Record relevant input and destination preconditions; reject stale plans
      and define race handling between validation and application, including
      symlinks and concurrent filesystem changes
- [ ] Specify a versioned, validated plan format, binding plans to their execution
      assumptions and defining handling of sensitive contents and file permissions
- [ ] Define partial-failure reporting and recovery during application; do not
      imply that a sequence of filesystem operations is automatically atomic
- [ ] Preserve purity and bytecode safety boundaries and assess planning overhead
- [ ] Add VM and public-CLI coverage proving that preview leaves target files
      unchanged, staged reads are coherent, diffs match applied changes, stale
      plans fail, and unsupported effects cannot bypass preview enforcement
- [ ] Evaluate moves, deletes, external adapters, and links to value explanations
      separately after the local read/write workflow is proven useful

Related work includes [Terraform saved plans](https://developer.hashicorp.com/terraform/cli/commands/plan)
and [PowerShell ShouldProcess and WhatIf](https://learn.microsoft.com/en-us/powershell/scripting/learn/deep-dives/everything-about-shouldprocess).
The intended distinction is VM-enforced planning for a defined set of effects in
ordinary imperative scripts, with explicit limits on what can be simulated.

### Candidate differentiator: resumable execution — exploration

Explore opt-in durable execution for ordinary local scripts: preserve progress
across interruptions without requiring users to implement their own progress
store or operate a separate workflow service. A batch conversion or import
should recover recorded work and continue from a supported checkpoint.
Proposed commands such as `panack run import.panack --durable import.run` and
`panack resume import.run` are design sketches, not available CLI features.

- [ ] Specify a first version with explicit checkpoints, serialisable VM state,
      and a small, documented set of recoverable local file operations
- [ ] Define checkpoint placement and state capture, including call frames,
      local values, persistent collections, and the treatment of unsupported
      resources and nested VM execution
- [ ] Bind recovery to the exact bytecode and compatible runtime/checkpoint
      versions; reject incompatible code rather than silently resuming it
- [ ] Specify durable, crash-consistent checkpoint and operation records, with
      validation of untrusted or corrupted state and exclusive ownership of a run
- [ ] Define how recorded arguments, environment values, read results, and changed
      external inputs affect recovery while preserving the purity boundary
- [ ] Reuse durably recorded completed-operation results; distinguish operations
      safe to repeat from operations requiring explicit reconciliation
- [ ] Handle the crash window between an external action succeeding and its
      completion being recorded; stop on an unknown outcome unless a supported
      recovery protocol can establish it, without claiming universal exactly-once
      execution or silently repeating an unsafe action
- [ ] Define cancellation, failed-run inspection, checkpoint retention, sensitive
      state handling, and clear CLI reports of recovered and pending work
- [ ] Add VM and public-CLI tests with interruptions around checkpoints and effect
      recording, including corrupted state, changed bytecode, concurrent resume,
      repeated recovery, and supported file-operation failure cases
- [ ] Measure checkpoint size, storage growth, and execution overhead on a
      representative batch-processing script before broadening the scope
- [ ] Evaluate automatic checkpoints, durable waits, remote-service adapters,
      code migration, and integration with preview plans and explanations later

Related work includes [DBOS workflow recovery](https://docs.dbos.dev/production/workflow-recovery)
and [Temporal durable execution](https://assets.temporal.io/durable-execution.pdf).
The intended distinction is a local workflow integrated with the normal language
runtime and command, with explicit recovery guarantees for supported operations.

### Candidate differentiator: enforceable data-flow restrictions — exploration

Explore data that carries enforceable rules about where its information may go.
Restricted inputs would retain their confidentiality policies through function
calls, transformations, collections, and control flow. The compiler and VM host
boundary would reject disallowed output, such as logging a credential embedded in
request headers, and explain the source and destination of the prohibited flow.
This is a proposed capability, not an implemented security guarantee or accepted
syntax. It complements purity by constraining where effectful code may send data.

- [ ] Define a scoped first version with confidentiality labels, explicit allowed
      output destinations or trusted operations, and compiler-checked propagation
      through ordinary values, calls, records, tagged unions, and collections
- [ ] Specify label inference and policy composition when values with different
      restrictions are combined; encoding, hashing, interpolation, and container
      construction must not silently remove restrictions
- [ ] Track implicit flows through branch conditions and other control dependencies,
      including output whose occurrence reveals restricted information; define
      treatment of errors, traps, and program termination explicitly
- [ ] Define trusted policy declarations and authority for intentional disclosure
      (declassification); ordinary helpers must not grant themselves permission
      to weaken a policy, and sanitising a value must not imply automatic release
- [ ] Specify the relationship between permitted recipients and permitted operations;
      an authentication-only credential must not become generally printable merely
      because the destination is trusted
- [ ] Enforce policies at terminal, file, and other supported host boundaries,
      including nested execution; specify validation or runtime enforcement for
      untrusted bytecode so source-level checking cannot be bypassed
- [ ] Preserve the existing purity boundary and define module and callable contracts
      so passing restricted data to a helper retains the applicable restrictions
- [ ] Design diagnostics that identify the restricted source, propagation path, and
      prohibited destination without including the sensitive value itself
- [ ] Apply the same policies to future explanations, preview artifacts, checkpoints,
      and diagnostic exports so tooling does not introduce an alternate output path
- [ ] State the threat model and limits, including timing and resource side channels,
      native integrations, and behaviour after an authorised recipient receives data;
      do not promise unrestricted non-disclosure across all possible observations
- [ ] Validate a credential-use workflow, accidental header logging, encoded and
      nested values, secret-dependent output, combined policies, and authorised
      disclosure with compiler, verifier, VM, and public-CLI tests
- [ ] Measure annotation burden and runtime overhead before expanding the scope;
      evaluate dynamic policies, remote-service adapters, and integrity labels
      separately after the initial confidentiality model is practical

Related work includes [Jif information-flow checking and controlled disclosure](https://www.cs.cornell.edu/jif/doc/jif-3.3.0/label_checking.html).
The intended distinction is approachable policy-carrying data in ordinary scripts,
with useful diagnostics and consistent enforcement across language tooling, not
an invention of information-flow security.

### Candidate differentiator: change contracts — exploration

Explore executable contracts describing which behavioural differences are allowed
between a new implementation and a pinned older version. Users could require an
optimisation to preserve results, permit a feature change only for selected inputs,
or broaden accepted input while retaining existing meanings and rejection rules.
Proposed forms such as `change ... against ...`, `preserve always`, and conditional
preservation are design sketches, not accepted syntax or available verification.

- [ ] Start with pure functions, explicit old/new bindings, result comparison,
      generated inputs, and concrete counterexamples for unconditional preservation
- [ ] Pin the baseline to an immutable artifact with its dependencies and execution
      semantics; define symbol matching and reject incompatible signatures or
      unsupported runtime versions explicitly
- [ ] Specify observable equivalence, including collection order, tagged results,
      traps, and termination; distinguish value equality from changes in timing or
      resource use and define how timeouts affect conclusions
- [ ] Design conditional preservation and required new behaviour using pure
      predicates; allowing a difference must not by itself establish that the new
      behaviour is correct, and overlapping or uncovered conditions need clear rules
- [ ] Support explicit finite input domains for exhaustive bounded checks and
      reproducible generated tests with seeds, search budgets, and domain constraints
- [ ] Report proved, exhaustively checked within bounds, counterexample found,
      no counterexample found by testing, and unresolved as distinct outcomes;
      never present testing or a timeout as a universal proof
- [ ] Preserve domain guards and exact numeric semantics in input generation and
      any future solver encoding; reject unsupported operations rather than silently
      approximate the language's behaviour
- [ ] Render counterexamples with inputs, old/new outcomes, and the violated rule;
      evaluate reduction and regression-test export while respecting data restrictions
- [ ] Specify contract placement, baseline acquisition, and stable CLI/CI handling,
      including explicit policy for unresolved checks and trusted baseline execution
- [ ] Validate representative changes: stable-order duplicate removal, a new member
      delivery benefit that preserves non-member fees, and a configuration default
      that accepts missing fields without accepting explicitly invalid values
- [ ] Add compiler and public-CLI coverage for preservation rules, baseline failures,
      counterexamples, deterministic search, and honest reporting of bounded or
      incomplete checks; measure cost before making checks part of routine builds
- [ ] Evaluate proof support for a carefully defined subset after the testing
      workflow is useful, with explicit assumptions and sound result reporting
- [ ] Later compare structured preview plans against the same simulated inputs,
      including permitted additional deletions and preserved writes or moves;
      limit claims to effects faithfully modelled by preview mode

Related work includes [SymDiff differential program verification](https://www.microsoft.com/en-us/research/project/symdiff-differential-program-verifier/).
The intended distinction is approachable change boundaries in normal development
and release workflows, not a claim that arbitrary program equivalence is decidable.

### JSON data support — exploration

Explore a standard `Json` tagged value type and a coherent library workflow for
configuration, data transformations, and future API clients: parse external text,
validate it into domain types, work with ordinary typed values, and encode results.
Keep dynamic JSON objects distinct from statically checked records. Prioritise
parsing, access, encoding, and typed decoding before special literal syntax;
compiler assistance should serve typed integration where needed. These are design
priorities, not implemented features or accepted API syntax.

- [ ] Specify JSON null, booleans, exact numbers, strings, arrays, and string-keyed
      objects; preserve exact numeric values without implicit binary floating-point
      conversion, including exponent notation and documented resource limits
- [ ] Design pure parsing and encoding APIs with structured results; distinguish
      malformed JSON from valid JSON that fails domain validation
- [ ] Support direct inspection and persistent updates for small transformations,
      keeping missing fields, explicit null, and wrong value types distinct
- [ ] Design typed decoding for records, collections, optional fields, and guarded
      types, with explicit policies for missing, null, and unknown fields; start
      with strict rejection of unknown record fields as the proposed default
- [ ] Treat decoding into guarded types as an explicit checked conversion returning
      success or a structured error; preserve existing proof requirements for
      ordinary conversions and reject unsupported guards explicitly
- [ ] Report parse errors with source locations and decoding errors with field/index
      paths, expected types or guards, and useful descriptions of offending values
- [ ] Reject duplicate object keys by default; preserve input object order for
      readable transformations and offer deterministic sorted-key output
- [ ] Specify escaping, Unicode handling, number formatting, nesting limits, and
      round-trip value semantics; distinguish these from preserving original
      whitespace, escapes, and number spelling in a future document-editing model
- [ ] Validate the workflow with configuration decoding (including an invalid Port)
      and a small dynamic JSON transformation through the public CLI
- [ ] Add unit and functional coverage for malformed input, duplicates, exact-number
      round trips, missing/null distinctions, nested error paths, guarded decoding,
      unknown-field policy, ordering, and resource-limit failures
- [ ] Evaluate JSON literals after the core workflow, including syntax ambiguity,
      interpolation, inferred types, and whether literals construct `Json` values
      or use an explicitly selected typed representation

### Type-driven input handling — exploration

Extend the JSON decoding foundation above into reusable type-driven input handling:
record fields and guarded types should supply a coherent description for input
validation, serialisation, and machine-readable schemas. Begin with JSON and
configuration, then evaluate command-line arguments and other adapters. This is a
proposed direction, not implemented derivation, field-default syntax, or an API.

- [ ] Share field and domain-rule metadata across decoding, validation, encoding,
      and schema generation so independently maintained definitions cannot drift
- [ ] Keep external validation explicit, returning valid domain values or structured
      errors; preserve proof requirements for ordinary guarded-type conversions
- [ ] Define defaults, missing versus null values, unknown fields, coercion, and
      nested error accumulation consistently with the JSON proposal above
- [ ] Specify which guards can be checked and exported to each schema format;
      reject or clearly report unrepresentable constraints rather than weakening them
- [ ] Allow separate input and output models and explicit field mappings; do not
      expose internal or restricted fields automatically through derived encoders
- [ ] Evaluate CLI parsing from the same metadata, including option names, defaults,
      help, and error locations, without making every domain type a CLI interface
- [ ] Validate a server configuration with guarded name and port fields, including
      multiple nested errors and schema changes after a field or guard is revised
- [ ] Add compiler/library and public-CLI coverage for agreement between types,
      validation, and schemas, including unsupported guards and disclosure policies

Related work includes [Pydantic validation](https://pydantic.dev/docs/validation/latest/concepts/json/)
and [schema generation](https://pydantic.dev/docs/validation/2.9/concepts/json_schema/).
The goal is a native connection between Panackelty domain types and external data,
building on the JSON backlog rather than a second independent validation system.

### Typed edits and patches — exploration

Explore scoped editing of persistent values that produces both a new value and a
typed description of its changes. This could support concise nested updates,
configuration previews, undo/redo, and incremental data exchange. An `edit` block
and operations such as `change.value`, `change.patch`, and `change.inverse` are
working sketches, not accepted syntax. This concerns data values, independently
of the more ambitious recovery or correction of external effects.

- [ ] Specify scoped drafts for records and persistent collections; retain the
      original value, prevent draft references from escaping, and preserve purity
- [ ] Define typed field paths, update operations, and patch representations so
      field renames and incompatible value types are checked by the compiler
- [ ] Define patch preconditions and explicit conflict results when applying edits
      to a changed base; inverse patches must check their own applicability too
- [ ] Specify array insertion, removal, and update semantics, distinguishing index
      positions from stable element identities instead of silently conflating them
- [ ] Define patch composition and inversion laws, including no-op edits, overlapping
      updates, and retention of old values needed for inverse operations
- [ ] Preserve domain guards, information-flow restrictions, and exact value
      semantics in drafts, patches, conflicts, and rendered change descriptions
- [ ] Evaluate structural sharing, batched updates, and memory cost without promising
      minimal patches or universally efficient application
- [ ] Validate nested task/settings edits, successful undo/redo, stale-base conflicts,
      and collection edits with compiler, VM, and public-CLI coverage
- [ ] Evaluate serialisable patches and schema/version compatibility separately
      before using them for remote updates or persisted change histories

Related work includes [Immer immutable editing](https://immerjs.github.io/immer/)
and [patches](https://immerjs.github.io/immer/patches/).
The intended distinction is native typed patches with explicit applicability,
composition, and conflict semantics across Panackelty data types.

### Useful execution of unfinished code — exploration

Explore typed holes and development execution that keeps completed portions of a
program inspectable while unfinished expressions remain explicitly unresolved.
For example, a report's totals could be inspected while its title is still a hole.
A spelling such as `?report_title` is a design sketch, not accepted source syntax.

- [ ] Define typed holes with expected types and lexical context, retaining useful
      type checking and diagnostics for the completed portions of a program
- [ ] Specify partial evaluation and blocked dependencies without substituting
      guessed values; define holes in conditions, calls, loops, and compound values
- [ ] Start with pure computations and explicit fixture inputs in an opt-in
      development workflow; production checks and builds must reject unresolved holes
- [ ] Keep bytecode and the VM as the execution model; design validated development
      representations for partial values without weakening production verification
- [ ] Inspect completed intermediate values and unresolved dependencies through the
      CLI/editor, observing information-flow restrictions and resource limits
- [ ] Specify effect handling before allowing effectful development execution;
      blocked work must not silently trigger real writes or external operations
- [ ] Validate a partially implemented report and branch-by-branch development,
      including informative expected types and inspection of independent results
- [ ] Add compiler, verifier, VM, and public-CLI tests for partial results, blocked
      control flow, rejection in production mode, and absence of unintended effects
- [ ] Evaluate watch mode and editor integration after a coherent source-file and
      CLI workflow, without requiring a browser-only or structured editor

Related work includes [Hazel's live programming with typed holes](https://hazel.org/).
The goal is useful feedback during ordinary incomplete development in Panackelty's
source-file workflow, with an explicit boundary between partial and runnable code.

### Data and target-platform experiments

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

## Expand automation and host capabilities — in progress

Panackelty should gain the general host capabilities needed by dependable
automation programs before its test suite is moved away from Python. These APIs
must be useful outside the test harness, remain visibly effectful, behave
predictably across supported platforms, and expose structured failures rather
than test-specific shortcuts. Logical standard-library imports are a
prerequisite so programs can use these APIs without knowing repository paths.

- [x] Specify a coherent process API for executable selection, arguments,
      standard input, working directory, environment overrides, exit status,
      and captured standard output and error
- [x] Make process streams byte-oriented with explicit checked UTF-8 decoding,
      define resource and output limits, and prevent deadlocks when both output
      streams are active
- [ ] Specify portable directory enumeration, directory creation, file metadata,
      removal, and recursive operations with deterministic ordering and clear
      symbolic-link and failure behavior; immediate enumeration, one-level
      creation, metadata, and nonrecursive removal are implemented, while
      recursive operations remain deferred
- [x] Add collision-safe temporary-file and temporary-directory creation with
      explicit ownership, cleanup, and failure semantics
- [x] Separate wall-clock time from a monotonic elapsed-time API suitable for
      validation budgets and performance measurements
- [ ] Define the supported-platform and capability policy for behavior such as
      permissions that cannot be represented consistently on every host; do not
      add a universal operation solely for a platform-specific test
- [x] Implement the accepted host boundary in the native VM and every
      transitional runtime still required for differential validation
- [x] Add typed standard-library wrappers that keep all process, filesystem,
      temporary-resource, and clock operations effectful
- [ ] Add focused, cross-runtime, and public-CLI conformance coverage, including
      large simultaneous process streams, invalid UTF-8, missing executables,
      environment and working-directory isolation, cleanup failures, path
      traversal, resource limits, and monotonic timing; the initial suite covers
      streams, isolation, temporary cleanup, bounds, and failure categories;
      exhaustive injected host failures and traversal coverage remain pending
- [x] Build a small Panackelty testing library with assertions, structured test
      results, fixture discovery, temporary isolation, command assertions, and
      deterministic reporting as the foundation for Python removal; the three
      explicitly imported modules now cover pure assertions and ordered reports,
      sorted immediate fixture directories and explicitly owned workspaces,
      plus bounded byte-exact command assertions and expected host errors.
      Migration of the Python oracle and harness is a separate initiative below

## Eliminate Python from the repository — planned after host capabilities

Python has been removed from the public toolchain, but it still implements the
transitional development oracle, test harness, compatibility facade, and seed
regeneration command. Retire those uses after logical imports, the required host
capabilities, and the Panackelty testing foundation are complete. Completion
means the current repository contains no Python source or Python command
invocation and its full development, bootstrap, conformance, packaging, and
release validation succeeds on a machine where no Python interpreter is
installed.

- [x] Complete and document the replacement test architecture: use the
      Panackelty-hosted library for compiler, language, standard-library, and
      functional behavior; focused C tests for native VM internals; and portable
      declarative fixtures shared between them. The ownership inventory,
      parity gates, migration sequence, and unresolved runner prerequisites are
      recorded in `tests/PYTHON_MIGRATION.md`; the old checks remain active
- [ ] Port compiler, bytecode, verifier, VM, runtime, and standard-library unit
      coverage without losing focused assertions or important failure cases
      Direct compiler coverage is complete: lexer, parser, resolver, checker,
      purity, emitter, diagnostics, loader/imports, driver, generics, inference,
      types and host-boundary assertions run natively. The two final compiler
      probes add 201 direct and 51 integration assertions. Bytecode and verifier
      wire coverage now runs in two Panackelty probes and direct C verifier
      checks. Python-only in-memory object and adjustable-limit checks remain
      until the oracle replacement. Direct VM execution and loader coverage now runs in 153 Panackelty
      assertions plus native C/header contracts. Runtime and standard-library
      coverage remain in this combined item.
      Python differential compiler evidence remains until the oracle milestone;
      the exact retained-case and ownership audit is in `tests/PYTHON_MIGRATION.md`.
- [x] Port functional-test discovery, subprocess orchestration, environment and
      file fixtures, output comparisons, and exit-status assertions. The
      Panackelty runner checks twenty-five selected cases, twenty examples,
      forty-one failure fixtures, and the complete CLI/environment/file
      contracts. `make functional` also validates the stage-two compiler
      driver and runs its smoke case from source and bytecode; no Python
      functional methods remain.
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

The resolver migration exposed the compiler fixture's 20-second subprocess
limit on this environment: unchanged compiler source execution succeeded in
21.8 seconds when measured separately, while the first full run failed its
source and compile commands. These two compiler-building commands now use the
existing compiler-driver build allowance of 90 seconds; ordinary fixture
commands and phase warning budgets are unchanged. The failed run took 183
seconds overall (unit 171). Profile compiler self-compilation as part of the
prioritized timing work; increasing a command allowance is not a speed fix.

The September 2026 clean local check after the lexer unit migration took 125
seconds (120-second budget); its unit phase took 71 seconds (15-second budget).
Profile the remaining Python unit harness and native build/bootstrap on this
environment while retaining all compiler unit assertions. CI timings remain
the reference for the cross-platform validation budget.
The VM milestone adds portable execution/loader and native-wrapper probes.
Its local focused `make check-vm` passed in 48 seconds against the 15-second
budget, including the retained host and differential Python tests.
Keep their process-launch and fixture-decoding costs in the same prioritized
unit-budget investigation; the 120/15-second targets are unchanged.
The bytecode milestone adds two portable codec/native command probes; a local
focused check took 19 seconds against its 15-second warning budget. Profile
fixture decoding, redundant process launches and retained Python oracle work
without dropping malformed inputs or changing the timing budgets.

The unit timer now includes all seven Panackelty compiler probes as well as
the remaining Python tests. Keep their compilation and execution cost visible
when profiling the existing unit-budget warning. The final compiler migration
adds a direct driver build, source/bytecode commands and snapshot checks; profile
these separately from the retained Python oracle before increasing allowances.

Current environment follow-up: the September 2026 testing-library branch
reported a 21-second unit phase against its 15-second warning threshold, also
observed on the unmodified checkout in this environment. With the expanded
fixture runner, clean checks observed 32–36 seconds for units and 79–84 seconds
overall. Profile the unit phase here and address its dominant cost without
reducing coverage; the earlier full check remained within its 120-second budget.
The compiler `source.path` fixture raised one clean check to 177 seconds (unit
97 seconds, functional 63 seconds). The cleanup failure unit test now selects
one fixture, reducing the next clean unit phase to 68 seconds. Next, reuse the
compiler result across the remaining integration checks
without dropping the source, bytecode, or path containment assertions.
The example migration adds twenty source and bytecode checks to each full runner
invocation. Profile that added work as part of the same prioritized timing fix.
The failure migration adds forty-one check and compile diagnostic pairs plus
artifact assertions; measure the full runner and remove redundant invocations
without weakening the new negative coverage.
The final functional migration's first clean run took 251 seconds (unit 147,
functional 86), with warnings at all three budgets. Its checkout-with-spaces
unit regression redundantly reran the complete functional suite; that test now
checks the stage-two compiler path and byte-identical output directly. Continue
profiling the remaining sequential runner work and reuse verified artifacts
to recover the 15/75/120-second budgets without removing assertions.
The functional runner took 24.6 seconds and each smoke invocation repeated its
full work (23.8 seconds for source). The functional recipe now captures one
successful report and checks it byte-for-byte from the smoke source and saved
bytecode; a focused run fell from about 73 to 25 seconds. Keep the standalone
smoke path and the remaining unit/full-check timing follow-up.

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
A September 2026 macOS checkout baseline after fixing paths containing spaces
passed 227 unit tests and 17 functional tests, but reported 322 seconds overall:
12 seconds for unit tests, 71 for the functional phase, and 238 for bootstrap.
Rebuilding stage 3 immediately afterwards took 37.31 seconds elapsed,
36.55 seconds of user CPU time, and 0.60 seconds of system CPU time. The
238-second result was not reproduced; the validation timer measures wall-clock
time and can include host interruptions.

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
- [x] Recover the clean validation budgets after file-aware token and expression
      positions increased self-hosted compiler build time, without reducing
      fixed-point, functional, or diagnostic coverage; use repeated elapsed and
      CPU measurements to distinguish compiler cost from host interruptions

- [x] Recover the remaining Linux CI budgets after optimisation: the full
      check fell from 134 to 38 seconds (budget 120), unit tests from 29 to 10
      seconds (budget 15), and package bootstrap from 75 to 19 seconds (budget
      60), retaining all coverage and cross-platform bootstrap evidence.

### CI feedback improvements

The Check workflow runs once per pull-request update, with pushes limited to
`main`, and cancels superseded runs for the same PR. The test job invokes
`make check` once instead of preceding it with overlapping component checks.
Both required platform packaging jobs and their complete validation gates remain.

The native VM now defaults to `-O2` with standard overridable build flags. A
macOS compiler benchmark took 38.77 seconds without optimisation and 13.16
seconds with `-O2`; the generated compiler artifacts were byte-identical. An
optimised clean `make check` passed 229 unit tests and 17 functional tests in
54 seconds, compared with the preceding 140-second local baseline. Full
validation still includes the stage-2/stage-3 fixed-point proof. Cross-platform
CI timings remain the measure of PR feedback speed; compiler-only benchmarks
must not be presented as full-workflow savings.

### Linux validation follow-up

A unit-test profile found repeated compilation of self-hosted probes. The lexer,
resolver, checker, purity, emitter, driver, and codec tests now compile each
parameterised probe once per class and run every input in a fresh VM. Local unit
validation fell from 13.50 seconds to 6.65 seconds, with all prior assertions
retained and new regression coverage included.

The native VM now records string code-point counts and ASCII metadata once,
avoiding repeated scans for length and ASCII offsets. A paired compiler build
measured 12.79 seconds before and 5.85 seconds after, producing byte-identical
compiler artifacts. Hosted CI passed all three required jobs: tests in 43
seconds, Linux packaging in 49 seconds, and macOS packaging in 63 seconds.
The complete workflow finished in 73 seconds. Linux phase timings were 10
seconds for units, 18 for functional validation, 9 for the remaining bootstrap
phase, and 38 for the complete check; separate package bootstrap took 19
seconds. Every measured phase met its budget, with no platform checks or
fixed-point evidence skipped.

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

## Native VM readability and test hardening

- [x] Decompose native decoding, verification, values, arithmetic, execution,
      builtins and host services into separately compiled modules.
- [x] Put shared declarations in self-contained headers, retain private local
      types, name wire opcodes, and document ownership and formatting conventions.
- [x] Add direct module ownership, cleanup, builtin registry and header checks,
      deterministic decoder mutations, and an isolated sanitizer target.
- [x] Sweep allocation failures across representative decoding, values, frames,
      exact arithmetic, nested execution and host operations; assert cleanup.
- [x] Add rich deterministic bytecode mutations, persistent ownership sequences,
      numeric boundary properties and selected host syscall failures.
- [x] Run native sanitizers in CI and publish LLVM line/branch coverage reports.
- [ ] Extend coverage-guided decoder fuzzing, host syscall/errno combinations,
      rendering and nested-execution branch coverage, and longer ownership runs;
      retain exact arithmetic, purity and runtime trap conformance throughout.
