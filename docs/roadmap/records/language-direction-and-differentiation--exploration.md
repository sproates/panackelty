[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Language direction and differentiation — exploration

Panackelty should combine strong static guarantees with a low-friction programming
experience. Powerful checking is useful only when programmers can understand a
failure and act on it quickly. New features should therefore be evaluated on
both the guarantees they provide and the clarity of the resulting workflow.

### Long-term direction: grow Panackelty through real programs

Treat the following as strategic challenges and sources of direction, not release
commitments or a fixed implementation plan. None is inherently infeasible; the
cost and sequencing differ. Let representative programs, measurements, and user
needs guide the work.

- **Performance:** maintain representative benchmarks, then investigate options
  such as fixed-width arithmetic with explicit overflow and conversion semantics,
  JIT compilation, or ahead-of-time native compilation. These are distinct
  possibilities to evaluate; recording them does not select one.
- **Ecosystem:** grow the ecosystem by building useful software in Panackelty:
  reusable libraries, tools, services, and eventually frameworks. Let real
  projects expose which language, standard-library, packaging, and tooling gaps
  matter next.
- **Platforms:** expand the supported targets and distribution options when they
  are useful and maintainable. Assess compiler/runtime portability, CI, packaging,
  and ongoing support for each target. Windows is a candidate to consider, not a
  commitment in this direction statement.
- **Ease of use and guarantees:** use ordinary programs to find where the language
  creates helpful confidence and where it adds avoidable friction. Improve common
  workflows through clear diagnostics, inference, APIs, or syntax as evidence
  supports. Strong checking should not be confused with requiring pure programs
  or unnecessary defensive runtime guards.

This direction does not set a release date, promise a particular platform or
compiler strategy, or relax Panackelty's guarantees. Specific proposals still
need their own semantics, evidence, and maintenance assessment.

### Further language research — unscheduled

The following are research and discussion topics only. They are not selected
implementation work, are not included in alpha 12, alpha 13, or the first
non-alpha release plan, and have no target release or date. Each needs a
separate user decision before scope or delivery is selected.

<a id="rm-145"></a>

**RM#145: Foreign-function interface (FFI).**

**Idea; research and discussion only.** Assess whether Panackelty should call
foreign code, and what a safe, maintainable boundary would require. Compare
possible ABI and platform scope, mapping of Panackelty types and effects,
ownership and lifetime rules, error handling, and containment of unsafe
behavior. Identify which use cases cannot be served by the current host boundary.
This does not select a foreign language, ABI, syntax, or implementation.

<a id="rm-146"></a>

**RM#146: Tracked concurrency.**

**Idea; research and discussion only.** Explore how pure computations and an
explicit model for effects and shared resources could prevent data races while
allowing useful multi-core scaling. Define what the compiler and runtime would
need to track across task creation, communication, cancellation, and resource
ownership. Test the idea against representative parallel workloads and compare
its guarantees, determinism, usability, and overhead with simpler alternatives.
Purity alone is not assumed to solve synchronization or scheduling.

<a id="rm-147"></a>

**RM#147: Compile-time macros.**

**Idea; research and discussion only.** Assess whether compile-time macros
would solve demonstrated repetition or abstraction problems. Compare expansion
models and their effects on type checking, hygiene, diagnostics, build
reproducibility, caching, and compiler trust boundaries. Define useful examples
and costs before considering syntax or implementation; no macro system is
selected.

<a id="rm-148"></a>

**RM#148: Resource lifecycle types.**

**Idea; research and discussion only.** Compare linear types and typestate as
alternative ways for the compiler to track the lifecycle of external resources
such as files, sockets, or processes. Evaluate ownership, valid state
transitions, error and cancellation paths, API complexity, and whether the
guarantees prevent practical resource misuse without making ordinary programs
harder to write. No type system or syntax is selected.

JIT feasibility is already recorded as an unscheduled research question under
[RM#56: Performance baselines](groomed-development-and-engineering-work.md#rm-56); retain that record rather than creating
a duplicate item. Research there does not select a JIT, native backend, or
release scope.

### Error propagation shorthand

<a id="rm-129"></a>

**RM#129: Error propagation shorthand** ·
[GI#274: Consider error propagation shorthand](https://github.com/sproates/panackelty/issues/274).

**Idea / unscheduled.** Recorded at the user's request on 2026-10-04.

Consider shorthand for repeated `Result` handling that extracts an `Ok` value
and returns an `Error` from the enclosing function. Assess readability and
boilerplate in representative programs against explicit matching. Syntax,
error-type compatibility, propagation boundaries, async/effect interactions and
cleanup implications remain to be assessed; preserve explicit typed failures
and the purity boundary.

The smallest outcome is an evidence-based recommendation, including retaining
explicit matching if shorthand offers insufficient value. Assessment is small;
implementation effort and dependencies remain unknown. No urgency is established.
This records an idea, not a syntax decision, exception system or implementation
commitment.

### Type inference and diagnostic experience

<a id="rm-64"></a>

**RM#64: Inference and diagnostics** · [GI#170: Inference and diagnostics](https://github.com/sproates/panackelty/issues/170).

**Selected for Alpha.14 as an investigation.** Owner direction, 2026-10-15:
assess current inference limits and the diagnostic experience under the existing
scope below. This does not select an inference architecture, language change or
implementation slice; recommend any implementation only after the assessment.

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

### Type-inference research candidates

Agreed for the backlog on 2026-10-02. These are unscheduled research ideas beyond
[#170](https://github.com/sproates/panackelty/issues/170)'s inference foundation,
outside programme #180 and its completion criteria. Begin with prior-work and
contract assessment; select prototypes separately when prerequisites are clear.
Novelty is unestablished. Effort and PR counts await assessment; no implementation
or publication commitment follows from recording these candidates.

| Candidate | Research question and bounded acceptance |
| --- | --- |
| <a id="rm-94"></a>**RM#94: Edit-stable inference** · [GI#215: Edit-stable inference](https://github.com/sproates/panackelty/issues/215) | Define guarantees that selected edits preserve inferred types and operation meanings, and find sufficient annotation boundaries where needed. Start with expression extraction and expected-type context; compare local/bidirectional inference and refactoring research. Establish preservation within a defined fragment and measure annotation burden, usefulness and cost. Compilation depends on current source, never hidden edit history. |
| <a id="rm-95"></a>**RM#95: Exact arithmetic resource inference** · [GI#216: Exact arithmetic resource inference](https://github.com/sproates/panackelty/issues/216) | Infer conditional numerator/denominator bit-size, intermediate-growth and work bounds under explicit input/iteration assumptions and a normalization/cost model. Compare resource-aware and size analyses, prove supported rules and evaluate precision and overhead. Preserve exact semantics; distinguish proved bounds from measurements and runtime estimates, and report unknowns honestly. |
| <a id="rm-96"></a>**RM#96: Annotation selection** · [GI#217: Annotation selection](https://github.com/sproates/panackelty/issues/217) | Select small, understandable sets of source annotation choices sufficient to resolve ambiguity, with consequences for each. Compare annotation synthesis and interactive/refinement inference; define the candidate language and minimality metric before claiming a minimum. Verify offered choices, retain genuine ambiguity, and measure burden, edit stability, developer usefulness and latency. Never guess intent or silently select semantics. |

Each assessment should identify a precise potentially distinct contribution,
reproducible evaluation and a proceed/defer recommendation, retaining unsupported
cases and limitations. Risks include duplicating prior work, restricting useful
inference, combinatorial cost and overstating guarantees. Literature assessment
can precede production inference; implementation depends on the relevant #170
interfaces and explicit scope selection. Record any supported publication thesis
in the [potential technical publications backlog](potential-technical-publications.md#potential-technical-publications).

### Core and standard-library types — planned exploration

<a id="rm-65"></a>

**RM#65: Core and library types**.

Keep the primitive type set small while making common terminal-program concepts
explicit in the standard library. Unfinished entries below are proposals;
completed entries describe accepted features. Prefer portable records and tagged unions; add compiler
or VM support only where representation, checking, or the host boundary requires
it. Coordinate generic-function work with `docs/internals/SELF_HOSTING.md`, JSON work with
the JSON data support initiative below, and host types with the automation and
host capabilities initiative.

Generic source functions, a first-class success value, and exact rational
arithmetic are implemented. Opaque paths, exact durations, and monotonic instants
are now implemented, together with typed filesystem and bounded process APIs,
checked decoding, and sleep. The testing-library foundation is complete. Recursive
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

<a id="rm-66"></a>

**RM#66: Lazy computation**.

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

<a id="rm-67"></a>

**RM#67: Contract-driven automation**.

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

<a id="rm-68"></a>

**RM#68: Runtime value provenance** · [GI#172: Runtime value provenance](https://github.com/sproates/panackelty/issues/172).

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

<a id="rm-69"></a>

**RM#69: Previewable effects**.

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

<a id="rm-70"></a>

**RM#70: Durable execution**.

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

<a id="rm-71"></a>

**RM#71: Data-flow restrictions**.

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

<a id="rm-72"></a>

**RM#72: Change contracts**.

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

### Guarded types — exploration

<a id="rm-153"></a>

**RM#153: Expand guarded types beyond numeric bounds** · [GI#435: Expand guarded types beyond numeric bounds](https://github.com/sproates/panackelty/issues/435).

Assess how Panackelty could express and prove useful invariants beyond numeric
bounds. Candidate examples include non-empty or constrained strings,
identifier-like values, enum alternatives and collection constraints; these
need evaluation, not automatic inclusion. Define the boundary between static
proof and explicit runtime validation, sound proof propagation and invalidation,
diagnostics, and limits on guard expressions. Purity alone does not make
arbitrary computation terminating, cheap or decidable.

The first implementation slice is selected for Alpha.14 under
[RM#151](next-release--agreed-scope-2026-10-06.md#rm-151): a non-empty string guard. Its syntax, proof behavior,
conversion and validation semantics, failure behavior, and tests remain for
slice planning. Other non-numeric guards and broader validation semantics
remain unscheduled.
Untrusted-input adapters remain coordinated with
[RM#73: JSON support](#rm-73) and
[RM#74: Type-driven input handling](#rm-74), avoiding duplicate scope.
Further guard families may remain candidates for future compiler/correctness
work under [RM#151: Alpha.14 direction](next-release--agreed-scope-2026-10-06.md#rm-151); this Alpha.13 slice does not
preselect them.

### JSON data support — exploration

<a id="rm-73"></a>

**RM#73: JSON support**.

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

<a id="rm-74"></a>

**RM#74: Type-driven input handling**.

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

<a id="rm-75"></a>

**RM#75: Typed patches**.

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

<a id="rm-76"></a>

**RM#76: Typed holes and partial execution**.

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

<a id="rm-77"></a>

**RM#77: Platform and ecosystem experiments**.

- [x] Establish a bounded WebAssembly-hosted VM profile for the complete-program
      playground; PRs #111, #113 and #115 record feasibility and delivery
- [ ] Assess broader browser application integration, including DOM/Web APIs,
      asynchronous effects and source debugging separately from the playground;
      direct code generation remains an alternative only if evidence warrants it
- [ ] Keep browser execution optional so terminal programs and the native seed VM
      do not inherit unnecessary platform complexity
- [ ] Define ecosystem and standard-library contribution criteria around API
      stability, deterministic tests, security review, and long-term ownership

Before promoting an experiment into the language specification, require a
representative program, a written semantics proposal, implementation and
maintenance estimates, and evidence that it strengthens Panackelty's identity more
than an ordinary library would.
