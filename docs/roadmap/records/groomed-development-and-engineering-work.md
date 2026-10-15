[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Groomed development and engineering work

The [candidate table](current-status-and-grooming-2026-10-01.md#groomed-candidates) owns state and estimates. Linked issues
hold detailed acceptance, risks and first-slice boundaries.

### Development workflow assessment

<a id="rm-47"></a>

**RM#47: Development workflow assessment** · [GI#133: Development workflow assessment](https://github.com/sproates/panackelty/issues/133).

Work record: [#133](https://github.com/sproates/panackelty/issues/133).

Observe a fixed installation-to-maintenance task, including API discovery, errors and
focused tests. Include reproducible debugging tasks covering a runtime trap, an
external-operation failure and an incorrect result. Observe whether a developer
can identify the cause, fix it and add a regression test; record time to a correct
fix, misleading diagnostics and missing tooling, without treating speed alone as
success. Use these findings to improve the debugging guide and bound any debugger
assessment. Record reproducible obstacles and feedback latency. Distinguish release and
development toolchains. Produce bounded fixes rather than assuming a project
generator, formatter or package manager is needed.

### Compiler explanations

<a id="rm-48"></a>

**RM#48: Compiler explanations** · [GI#134: Compiler explanations](https://github.com/sproates/panackelty/issues/134).

Work record: [#134](https://github.com/sproates/panackelty/issues/134).

**Owner direction, 2026-10-15:** completion means `panack explain` covers every
relevant type, effect and proof decision the checker makes while checking a
function. Each decision must expose its actual outcome, supporting checker facts
and source location. When checking cannot reach a decision, the output must say
unavailable and give the reason. A decision the checker makes but the explanation
does not yet cover is a gap, not an unavailable result. Before choosing further
implementation slices, build a complete checker-decision inventory and map every
entry to output and regression coverage. Existing slices, including record-field
access in PR #466, are partial progress toward this target.

The first U3 query is accepted through #219: `panack explain` reports
actual accepted/unproved subtraction decisions, operand facts and guard source
locations with explicit unavailable results. Diagnostic parity, public-CLI
acceptance and cost are recorded in the [query contract](../../guides/COMPILER_EXPLANATIONS.md)
and validation profile. Full types/effects/proof explanations remain open within
[programme #180](current-status-and-grooming-2026-10-01.md#compiler-and-runtime-understanding-programme); re-estimate and
select the next slice through programme prioritisation.
The selected GI#134 follow-up explains proof-bearing conversions into guarded
targets, including core-value facts and guarded source invariants. It also
accepts a guarded source for a compatible guarded target when the source
invariant proves the target; stricter targets still require sufficient facts.
Guarded-to-base widening is excluded because it needs no proof. The current
follow-up adds inferred-local explanations from checked initializer types and
source expressions, omits annotated locals, and reports incomplete inference as
unavailable without changing inference rules. This follow-up adds an optional
one-based line/column query for the smallest checked expression at that location,
including explicit unavailable output when no complete function body or matching
expression type is retained. PR #419 accepted this query and PR #420 accepted
the check between a function's actual result type and its declared return
type, including rejected and unavailable outcomes with the returned expression's
source range. PR #421 accepted direct function-call argument checks against
parameter types and PR #422 accepted explicitly annotated local initializer
checks. PR #423 accepted local assignment-value compatibility, PR #424 accepted
selected text and byte builtin argument checks, PR #425 accepted numeric builtin
argument checks, and PR #426 accepted collection mutation and lookup argument
checks, and PR #427 accepted typed path builtin argument checks. The current
follow-up retains environment builtin argument checks. PR #457 delivers checked
joins for array elements, conditional results and match arms, including
unavailable joins and unchanged checker diagnostics. PR #458 adds explanations
for the existing `map` and `reduce` callback contract checks, including callback
purity, arity, parameter types, result constraints, and accepted, rejected or
unavailable outcomes. PR #459 exposes the element-type join used by array
`concat`, preserving accepted, rejected and unavailable outcomes. PR #461
extends checked type-join evidence to repeated arguments that constrain one
generic function type parameter, preserving the existing inference result and
rejected diagnostics. PR #462 explains checked unary and binary operator type
decisions, including accepted, rejected and unavailable operands and their
result type. An accepted operator type decision does not claim that a separate
proof obligation, such as Nat subtraction underflow, was discharged. PR #463
explains index receiver and position checks plus `for` iterable/item types. The
PR #464 explains `Bool` requirements for `if` and `while` conditions, and PR
#465 explains whether each resolved `match` variant belongs to the matched enum,
independently of exhaustiveness, duplicates and binding counts; PR #466 explains
record-field access compatibility and the resulting field type. PR #477 adds
the match subject enum type, arm uniqueness, payload binding arity and
exhaustiveness decisions. PR #478 adds accepted, rejected and unavailable
constructor-argument compatibility. PR #479 adds accepted and rejected call
arity for resolved direct declarations and record/enum constructors. The
PR #480 adds fixed-arity core calls, indirect callable arity with an explicit
unavailable result when the callable type cannot be resolved, and explicit
type-argument counts for generic functions plus zero-count rejections on
non-generic core calls. PR #481 adds accepted, rejected and unavailable
indirect callable argument compatibility. PR #482 verifies zero-count
rejections for explicit type arguments on generic record and enum variant
constructors. The current slice exposes the accepted, rejected and unavailable
input-type requirement for `len` (`Str`, `Bytes` or array). PR #483 adds that
`len` evidence. PR #484 adds accepted, rejected and unavailable input
requirements for `print`/`eprint`, including rejection of `Void`. The current
slice explains whether an indirect call target has a known callable type,
including rejected non-callables and unresolved target types; PR #485 adds this
evidence. The current slice retains the native TCP/HTTP transport argument
requirements, including accepted, rejected and unavailable results; PR #486 adds
this evidence. The current slice explains the substituted type assigned to a
generic enum match payload binding, including unavailable output for an
unresolved subject type; PR #487 adds this evidence. The current bounded review
maps checker decision families to their source sites, explanation records,
regression suites and known evidence gaps in
`docs/guides/COMPILER_EXPLANATIONS.md`. Known gaps include specialized core-call
arguments, collection receiver requirements, declaration-level and inferred
callable effect decisions, and complete generic inference traces. These are
partial families within the owner-defined complete
checker-decision target above. Each implementation slice keeps ordinary
diagnostics and runtime behavior unchanged and retains evidence only for
explanation queries.
The current slice adds accepted, rejected and unavailable type evidence for
string interpolation, including guarded-local deferral; source locations cover
the full string literal because interpolation identifiers have no individual
retained spans.
This slice records why `Void` is rejected as local storage or an array element,
preserving the existing diagnostics and source locations.
The current effect slice retains unresolved indirect-call boundaries as
unavailable and renders boundaries the effect pass retained for deferred
functions, labelled with the function's deferred type-check status.
Ordinary checking keeps this evidence empty. These
remain bounded, unreleased CLI slices; the source-to-evidence inventory and any
uncovered type/effect/proof families remain open. The current website
stays accurate for its published versions; review website promotion at the next
supporting release. Runtime value provenance is its own linked workstream #172;
automatic fixes remain outside #134.

### Systematic invariant testing

<a id="rm-49"></a>

**RM#49: Invariant testing** · [GI#135: Invariant testing](https://github.com/sproates/panackelty/issues/135).

Work record: [#135](https://github.com/sproates/panackelty/issues/135).

Extend existing source/bytecode comparisons, deterministic round trips and
bootstrap fixed points with one bounded invariant family. Use reproducible
generation or justified source transformations, an independently reviewed oracle
and useful reduced failures. Demonstrate detection of an isolated deliberate
perturbation. Shared implementation bugs and invalid transformations are risks;
full random-language generation and a second execution engine are outside scope.

### Source-aware runtime errors

<a id="rm-50"></a>

**RM#50: Source-aware runtime errors** · [GI#136: Source-aware runtime errors](https://github.com/sproates/panackelty/issues/136).

Work record: [#136](https://github.com/sproates/panackelty/issues/136).

Production source positions, instruction mappings and validated sidecar lookup
are available through U2. Automatic runtime error integration remains open; build
on the [mapping foundation](current-status-and-grooming-2026-10-01.md#source-to-bytecode-mapping-foundation-u1-and-u2-done)
for one bounded trap and call-context slice, assessing reuse with coverage
metadata where semantics agree. This is the first source-aware debugging-support
delivery: explain what failed, identify the source expression and show useful
call context. Validate these diagnostics through the debugging tutorial.
Acceptance includes imported/generic code, nested calls, an
async boundary and safe fallback for absent or mismatched source snapshots.
Preserve error meaning and bytecode safety. A debugger and full async history
are separate; report publication is not a prerequisite.

### Learning path and technical documentation

<a id="rm-51"></a>

**RM#51: Learning and debugging guides** · [GI#137: Learning and debugging guides](https://github.com/sproates/panackelty/issues/137).

Work record: [#137](https://github.com/sproates/panackelty/issues/137).

Build on the README, examples, specification, VM guide and playground. Start
with one complete tutorial from clean installation through testing and a
maintenance change, linked to discoverable library/reference material. Run its
commands against the declared version and keep development-only networking
distinct from alpha.10. Subsequent practical and technical guides follow
demonstrated gaps. Website visual changes require a working review preview.

Explicit deliverable: **Debugging Panackelty programs**, a practical guide using
commands supported by its declared release. Cover three worked problems: a
runtime trap, an expected external-operation failure that needs handling, and a
valid program producing the wrong result. Each walkthrough must reproduce the
problem, investigate its cause, fix it and add a regression test. Show how to
inspect intermediate values and reduce a failing case using available tools;
state diagnostic limits honestly. The VM guide's recorded executions are useful
background, not an interactive debugger.

Start with today's tools rather than waiting for source maps or a debugger.
Update the guide when source-aware locations and call context ship under #136.
Coordinate executable examples with #138 and usability evidence with #133.
Initial guide estimate: S–M / 1–2 PRs including examples and CI checks; later
updates follow the capabilities they document.

### Public Markdown quality

<a id="rm-131"></a>

**RM#131: Public Markdown quality** · [GI#281: Public Markdown quality](https://github.com/sproates/panackelty/issues/281).

**Idea / unscheduled.** Recorded at the user's request on 2026-10-04. Review,
expand and improve all public Markdown, including `docs/reference/SPEC.md`, `docs/internals/ARCHITECTURE.md`,
the README, contributor material and guides. Inventory the documents and assess
accuracy, completeness, readability, examples, cross-links and version claims;
reconcile language and architecture descriptions with the implementation.
Complement [RM#51: Learning and debugging guides](#rm-51) and
[RM#52: Executable documentation](#rm-52), preserving their existing scope.

Acceptance: account for every inventoried document, fix or explicitly track
remaining gaps, verify examples against declared versions and check links and
consistency between references and guides. Provisional effort: M–L across staged
PRs; the inventory determines dependencies and final scope. This records a
documentation review, not approval to change language behavior or priorities.

### Definitive Panackelty style guide

<a id="rm-133"></a>

**RM#133: Definitive Panackelty style guide** · [GI#283: Definitive Panackelty style guide](https://github.com/sproates/panackelty/issues/283).

**In progress — selected 2026-10-06.** Establish the authoritative coding
standard in [CONTRIBUTING.md](../../../CONTRIBUTING.md#panackelty-source), expanding the
existing conventions with required versus preferred rules, checked examples,
exceptions and incremental adoption. Covers formatting, naming, imports,
declarations, types, control flow, error handling, effects and behavioural tests.
Coordinates with GI#304's paired source-readability/coverage programme. No
repository-wide reformat or formatter implementation is included. Acceptance
requires checked snippets, documentation validation and review of the proposed
standard; this entry does not claim merge or independent review.

### Website capabilities guide

<a id="rm-130"></a>

**RM#130: Website capabilities guide** · [GI#279: Website capabilities guide](https://github.com/sproates/panackelty/issues/279).

**Done.** Delivered by [PR#290: Website capabilities guide](https://github.com/sproates/panackelty/pull/290).
The [live capabilities page](https://panackelty.com/capabilities/) provides friendly
feature explanations, four tested code examples, responsive contents navigation,
shared site navigation and homepage links to individual topics. Async/await has
its own section; upcoming namespaces, modules/packages, compiler explanations,
HTTP, source coverage and performance are explained separately from released
native alpha.11 and browser v0.1.1 capabilities.

Acceptance: independent review, desktop/mobile preview checks, all eight preview
regressions, Pages/link validation and the full native suite passed. All four
examples passed against checksum-verified native alpha.11 through source and
saved bytecode. The user approved the hosted staging preview before merge.
[Main validation](https://github.com/sproates/panackelty/actions/runs/37239178241)
and [publication with live-byte verification](https://github.com/sproates/panackelty/actions/runs/37239451252)
passed. Live page, contents anchors and homepage topic navigation were verified;
publication provenance identifies `2d3c96630cf0966ee2ba6bc7902a71c8941206fe`.
No release pins or programme weights changed. The local 146s full-suite observation
remains above the unchanged 120s target under the existing [RM#123](standing-performance-engineering--ongoing.md#rm-123) /
[GI#106](https://github.com/sproates/panackelty/issues/106) validation-cost concern;
this single observation does not establish a causal regression.

Related learning guides ([RM#51](#rm-51) / [GI#137](https://github.com/sproates/panackelty/issues/137))
and executable documentation ([RM#52](#rm-52) / [GI#138](https://github.com/sproates/panackelty/issues/138))
remain separate work.

### Executable documentation

<a id="rm-52"></a>

**RM#52: Executable documentation** · [GI#138: Executable documentation](https://github.com/sproates/panackelty/issues/138).

Work record: [#138](https://github.com/sproates/panackelty/issues/138).

Quick-start, website, functional-example and playground checks already execute
documentation. Inventory checked, illustrative and uncovered content, then
verify one additional guide through the existing toolchain. Incorrect commands
or expected output must fail visibly. Preserve negative examples, clean
setup/cleanup and version boundaries. Coordinate with the first tutorial without
introducing a competing harness or silently skipping platform-dependent
examples.

Include the debugging tutorial explicitly: execute its failing examples, asserted
exit status and relevant diagnostics, corrected programs and regression tests
against the declared release. Incorrect commands or changed expected results
must fail CI. Use deterministic local fixtures for external-operation failures;
keep setup and cleanup reproducible. Distinguish currently supported output from
future source-aware diagnostics instead of accepting either silently.

### Debugging delivery sequence — agreed 2026-10-01

Runtime failures and incorrect results remain possible despite compiler checks.
The user agreed the following debugging sequence; these are scoped additions to
existing work, not a new parallel principal programme or an implementation claim:

1. Deliver the first practical guide under #137/#138 using available tools, with
   #133 recording observed debugging obstacles. This has no source-map prerequisite.
2. Deliver the shared source-mapping foundation after #182 and the first
   source-aware diagnostics under #136; update the guide against their released
   behavior. The current programme remains the principal implementation priority.
3. Assess an interactive debugger against the remaining observed problems before
   committing to production debugger delivery.

Runtime value provenance #172 should evaluate the guide's incorrect-result
scenario: whether retained derivations help identify where a value went wrong.
Keep its independent correctness, opt-in overhead, retention and sensitive-data
acceptance; tracing derivations does not by itself provide stepping or inspection.
Source mapping remains the shared foundation already planned under #180/#173.

This planning update changes no website claims. Guide publication and shipped
debugging capabilities must assess website follow-ups for their stated versions.

### Interactive debugger — candidate for assessment

<a id="rm-53"></a>

**RM#53: Interactive debugger**.

**Idea; implementation unscheduled.** No dedicated issue yet; create its work
record after checking existing issues when the bounded assessment is selected.

Disassembly and recorded VM walkthroughs do not let a developer pause a running
program and inspect its state. Assess whether interactive controls resolve gaps
observed by #133 and the debugging tutorial, particularly incorrect-result bugs.

Start with a bounded synchronous CLI experiment: source breakpoints, stepping,
local-variable inspection and call-frame navigation on small known programs.
Use the shared mapping contract and preserve the VM as the only execution engine.
Define breakpoint resolution and step behavior around lowering, imports and
unmapped instructions; report unavailable state honestly. Compare observed stops,
locals and frames with independently expected results, and verify that ordinary
execution remains equivalent with debugging disabled.

Evaluate VM pause/resume hooks, metadata fidelity, state lifetime, resource cost
and safe inspection without unintended side effects. Unknown feasibility and
maintenance costs argue for an experiment before promising a production debugger.
Deliver a justified proceed/defer decision, demonstrated limitations and revised
estimates. Provisional size: M / one assessment PR; full delivery remains unknown.
Editor integration and async debugging require separate assessments and are not
acceptance requirements for this synchronous experiment.

### Editor support

<a id="rm-54"></a>

**RM#54: Editor support** · [GI#139: Editor support](https://github.com/sproates/panackelty/issues/139).

Work record: [#139](https://github.com/sproates/panackelty/issues/139).

No dedicated editor extension or grammar package was found in the tracked tree
during grooming. Select one editor before implementation, then deliver file
recognition, highlighting, comments, bracket pairing and indentation with
installation instructions. Validate current lexical examples and incomplete
code. Compiler-backed diagnostics, navigation, hover and completion follow
separate assessment; a language server, formatter and marketplace release are
not prerequisites.

### Technical showcase programs

<a id="rm-55"></a>

**RM#55: Showcase programs** · [GI#140: Showcase programs](https://github.com/sproates/panackelty/issues/140).

Work record: [#140](https://github.com/sproates/panackelty/issues/140).

Choose one complete demonstration combining existing capabilities, such as an
exact ledger with checked domain rules or a finite concurrent service with
independent clients. Provide deterministic inputs, expected source/bytecode
results and both success and failure boundaries. Explain guarantees and
platform/release limits. Reuse existing examples and checks; no new language
feature or production HTTP claim is required.

<a id="runtime-and-resource-baselines"></a>

### Performance and benchmarking

<a id="rm-56"></a>

**RM#56: Performance baselines** · [GI#141: Performance baselines](https://github.com/sproates/panackelty/issues/141).

Work record: [#141](https://github.com/sproates/panackelty/issues/141).

The [2026-10-03 source-based performance assessment](../../../tests/VALIDATION_PROFILE.md#runtime-performance-assessment--2026-10-03)
records architecture costs, exact-semantics comparison limits, finite TCP and
cooperative-execution boundaries, existing compiler measurements and a proposed
six-group baseline. No new benchmarks were run; neither broad inadequacy nor
production adequacy is established. This research record does not schedule
benchmark implementation, select optimisations or change programme priority.

Additional questions recorded on 2026-10-03 in GI#141: investigate JIT feasibility,
optimising ahead-of-time compilation directly to native executables, and how those
choices affect startup, sustained throughput, memory use and the cost of preserving
exact semantics in performance comparisons. These are **unscheduled research
questions only**; no investigation, backend implementation or architecture change
is selected by this record.

State: **Planned baseline; execution not started**, selected as a scoped outcome
of [RM#123: Standing performance engineering](standing-performance-engineering--ongoing.md#rm-123) on 2026-10-04. This updates
the earlier Idea/unscheduled baseline status while preserving the principal
feature priority and deferred website/CI work. It authorises no benchmark run,
optimisation or backend implementation in the current policy delivery.

Existing validation profiles and paired experiments are useful evidence but do
not form a general maintained performance baseline. Start with a small suite of
representative programs and focused benchmarks covering compilation, exact-value
operations, strings/collections and bounded I/O where supported. Measure
compilation, startup and execution costs, throughput/latency where meaningful,
peak memory and artifact size where reliable measurement is available.

Correctness-check outputs, pin inputs and record toolchain/environment metadata,
replay commands, repetitions and variability. Separate cold/warm and compile/run
costs, compare like-for-like revisions without concurrent benchmark interference,
and label unavailable metrics. The smallest useful outcome is a reproducible
baseline and comparison report, provisionally **M / 1–2 PRs**; host noise and
platform-specific accounting limit confidence in comparisons.

Use subsequent profiling to identify bottlenecks and scope targeted optimisation
with before/after evidence. Set regression budgets or automation only after
baseline noise is understood. Optimisation is separately scoped, not part of the
initial baseline commitment; preserve exactness, safety and correctness.
Universal thresholds and cross-language superiority claims are outside the
initial scope. The planned baseline requires its own selected implementation
delivery; JIT/AOT and other backend investigations remain unselected.

### Compiler performance investigation

<a id="rm-134"></a>

**RM#134: Compiler performance investigation** ·
[GI#285: Compiler performance investigation](https://github.com/sproates/panackelty/issues/285).
**Idea / unscheduled**, requested 2026-10-04. Investigate reproducible CPU/wall-time,
memory and scaling costs across tiny/large programs, self-compilation and module
graphs; attribute startup, core/module loading, parsing/resolution, checking and
emission with controlled repeated comparisons, correctness/output parity and
explicit instrumentation/noise limits. Deliver independently reviewed evidence
and ranked measured optimisation proposals; implementation is separately selected.
PR#284's initial +11.37% self-compilation comparison, noisy +1.40% confirmation,
higher tiny-program medians, +6.19% seed size and passing 140s canonical check
motivate investigation, not a causal regression claim; the prior controlled
clean baseline already exceeded 120s. See
[pinned evidence](https://github.com/sproates/panackelty/blob/490f8bce1b46de0c1a429851ef484c0bf6a64244/tests/VALIDATION_PROFILE.md#L3101).
Reuse GI#106 build/cache evidence, GI#141 general baselines and GI#220 mode work
without duplicating their outcomes. GI#106's owned next-compiler-delivery review
trigger and GI#263's weights remain unchanged. Provisional medium–large / 1–2
investigation PRs; profiler support and noise are risks. Assign a compiler/performance
maintainer when selected. No profiling, optimisation, new gate or reprioritisation
starts here; namespaces remain principal and compiler understanding remains paused.
Website/performance impact of this record: none; it changes no code or public claim.

### Independent contract implementation

<a id="rm-57"></a>

**RM#57: Independent contract assessment** · [GI#142: Independent contract assessment](https://github.com/sproates/panackelty/issues/142).

Work record: [#142](https://github.com/sproates/panackelty/issues/142).

The bytecode format, fixed fixtures and VM/compiler audit already establish
substantial contract evidence. Test one small stable subset by implementing a
disposable probe from the written contract before consulting implementation
details. Record ambiguities, independently derived malformed/valid vectors and
any prior knowledge limiting independence. Use the supported toolchain. This
assesses specification precision without adding a second execution engine or
claiming full-language conformance.
