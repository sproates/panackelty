[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Language namespaces — idea

<a id="rm-41"></a>

**RM#41: Language namespaces** · [GI#198: Language namespaces](https://github.com/sproates/panackelty/issues/198).

Work record: [#198](https://github.com/sproates/panackelty/issues/198).
Current state: **In progress / Now** as P2 in the [programme register](modules-packages-and-http-programme-sp1--2026-10-03.md#rm-108),
explicitly selected on 2026-10-04. P1 supplies the accepted
[shared design](../../design/MODULE_PACKAGE_DESIGN.md). The following assessment
originated on 2026-10-01 and does not override that production scope. It asks
how code in one file references functions and types in another. Existing quoted
file-relative and logical imports already work; declarations currently share one
program namespace. This extends the namespace idea, not completed import work.

**Intermediate foundation — merged in PR#261 as `a5cdb1a`:** retain per-file import,
visibility and declaration-span metadata; tagged standalone/toolchain module and
declaration identities; local binding/export inventories and collision/private
lookup diagnostics; attach existing resolver conflict diagnostics to their owning
declarations. Public `pub`, alias and selective-import forms fail closed while the
checked/emitted AST still uses current names. There is no executable namespace
mode or permanent compatibility switch. Unit and public-CLI fixtures cover the
staged boundary and repeated cross-module diagnostic origins. The refreshed v9
seed reaches the fresh compiler/library fixed points; 59 namespace assertions,
91 CLI integration assertions and all 343 functional cases pass under canonical
`make check` (126s, unit 91s, functional 5s, bootstrap 20s). The full 120s and unit
15s budgets remain exceeded: retain the prioritized
[GI#106: Validation performance](https://github.com/sproates/panackelty/issues/106)
reminder. See [validation evidence](../../../tests/VALIDATION_PROFILE.md#namespace-modulebinding-foundation--2026-10-04).
GI#198 remains open.

**Intermediate qualified-reference/binding-resolution slice:** raw `DotCallExpr` retains namespace/value ambiguity until binding;
qualified types, function references, constructors and enum patterns parse without
enabling execution. Explicit graph edges resolve namespace/selective imports,
re-exports and nested enum selectors to original declaration identities. The raw
use pass checks private/missing/wrong-kind bindings, namespace values and import
collisions with parameters, type parameters, locals, loops and patterns. Shared
completed-node traversal avoids repeated diamond subtree work. The loader reports
staged graph/use errors while preserving the execution gate, including unmarked
qualified forms. Focused coverage has 153 assertions; the diagnostic functional
case adds 18 public-CLI boundary checks and absent-bytecode evidence. Validation
performance remains tracked under GI#106: after correcting the legacy
basename/value-scope regression, a complete passing run took 216s (unit 173s),
exceeding the 120s full and 15s unit budgets. The gate now retains only candidate
namespace roots in its lexical set. Final canonical validation passes in 211s
(unit 168s, functional 5s, bootstrap 28s), still over budget; the observed timing
change is not an isolated performance attribution. Measurements are recorded in the
[validation profile](../../../tests/VALIDATION_PROFILE.md#namespace-raw-references-and-binding-resolution--2026-10-04).
This is an
intermediate GI#198 delivery, not namespace feature acceptance or a release.

**Merged checked declaration/signature slice — PR#275, `0bcf092`:** tagged core/nominal/callable and
binder-position parameter identities replace spelling-based contracts at this
staged boundary. Alias/re-export reachability preserves original identities;
public fields, enum payloads, parameters, results, nested generic/callable types
and guard helpers reject inaccessible dependencies. Public bodies may retain
private helper calls. Identity-based substitution is capture-free. The loader
retains valid contracts and owner-positioned diagnostics while preserving the
execution gate; legacy execution does not run the signature graph pass.
Focused namespace coverage passes 206 assertions. Seed refresh verifies fresh
compiler and standard-library fixed points. This is an intermediate GI#198
slice, not full P2 acceptance; at that handover partial completion was unquantified
and the accepted programme contribution was 10 pp. See the
[performance disposition](../../../tests/VALIDATION_PROFILE.md#namespace-checked-signatures--2026-10-04).

**Merged bound function-body slice — PR#284, `9f8f8a0`:** the loader now prepares callable contracts
before retaining structural function bodies with original call identities, lexical
binder identities and source spans. Bounded checks cover annotated locals,
assignment, direct/qualified/helper argument and return identity compatibility,
explicit generic substitution and callable reference effects. Unknown inference,
constructors, core operations and guard proofs remain explicitly deferred; no
body is claimed effect-checked or executable. The production full-module raw walk
is replaced, without a synthetic-name bridge into the legacy checker. Unit and
public CLI regression evidence, fresh seed and final validation belong to this
intermediate delivery; GI#198 stays open and accepted programme credit remains
10 pp; partial P2 progress was unquantified at that handover. See the
[body identity performance disposition](../../../tests/VALIDATION_PROFILE.md#namespace-bound-function-bodies--2026-10-04).

P2's original large/3–5 total-PR estimate is superseded by the provisional
remaining-work decomposition below, not by counting completed PRs. This changes
no scope or weight and awards no additional completion credit.

### P2 delivery checklist — delivery and remaining work

<a id="p2-delivery-checklist"></a>

The owner's 2026-10-07 request resumes P2 implementation after the selected
[RM#14: Website separation](independent-website-publishing.md#rm-14) work. Retain these eight checklist slices
for the core programme handoff; this sequencing decision changes no P2 scope,
weight or acceptance criteria.
These are ordered acceptance checklists within RM#41 / GI#198,
not eight independently tracked initiatives. PR#261, PR#262, PR#275 and PR#284 are
merged foundations, not complete executable namespace acceptance. The owner selected **1: ordinary expressions and control flow** for implementation
on 2026-10-05 after website separation. **Slice 1: Done through PR#297, `1ffd023`.**
Its delivery checks scalar
operators, bindings/assignments, branches, known iterable/index paths, returns and
lexical interpolation references. Invalid, deferred and checked-subset states
remain distinct. Collection construction, nominal fields/patterns and inferred
generics were assigned to slice 2; core contracts, proofs and effects remain slices 3–5.
Mixed Rat/integer joins explicitly defer rather than erase rational identity.
Completion evidence: focused coverage passes 344 assertions, public CLI
integration passes 118 assertions with positioned failures/no artifacts, and
fresh compiler/stdlib fixed points preserve v9. Complete source/bytecode functional
acceptance and canonical `make check` pass (146s against the 120s target). See the
[performance evidence and disposition](../../../tests/VALIDATION_PROFILE.md#namespace-ordinary-expressions--2026-10-05).
This is internal preparation, not executable namespace acceptance.

**Slice 2: Done**, selected by the owner after PR#297.
Records and enum variants, nested arrays and contextual empty collections,
nominal fields, exhaustive variant destructuring and argument-only generic
function inference retain original declaration and lexical identities. Explicit
function type arguments remain fixed; constructor type arguments retain their
existing restriction. Same-spelled nominal types do not unify through nested
inference or patterns. Whole-body constraints prevent a shared unresolved payload
acquiring contradictory types in separate calls/statements, reject recursive
evidence and keep unknown children explicit. Inferred locals settle immediately;
unrelated deferred proofs cannot permit inference from later returns or uses.
No record-destructuring syntax is introduced; destructuring covers the existing
variant payload patterns. Empty map/set construction is covered; methods and
indirect/core contracts were assigned to slice 3. Mixed Rat/integer joins remain
deferred; guard proofs were delivered in slice 4, while effects and emission
remain later obligations. Discarded incomplete
values stay deferred. Shared-constraint errors retain their original function
span; finer multi-site diagnostic evidence remains slice 7.

Focused coverage passes 465 assertions; public CLI integration passes 145,
including positioned constructor/inference/exhaustiveness/shared-constraint
failures and absence of bytecode. The positive namespace fixture still meets the
execution gate. Fresh compiler/stdlib fixed points preserve v9; the complete
aggregate program checks source and saved-bytecode behavior. Final clean canonical
validation passes in 152s (465 namespace and 145 CLI assertions); the 120s full
and 15s unit targets remain open. Performance disposition is recorded in the
[delivery evidence](../../../tests/VALIDATION_PROFILE.md#namespace-aggregates-patterns-and-inference--2026-10-05).
PR#321 merged the core and indirect-call implementation on 2026-10-07: core
operations and methods, receiver calls, indirect callable contracts and reserved
core identity checks. PR#326 completed the slice 3 contract matrix: arity,
argument and result failures; array callback input/result/effect contracts; async
indirect-call deferral; and namespace-versus-value resolution. The expanded
compiler unit suite passed 540 assertions; no checker defect was found. Slice 3
is complete on `next`, but does not imply executable namespace acceptance. PR#326
adds no compiler/runtime execution path or product runtime cost. `make check`
passed in 120s against the 120s budget; the separate `unit-compiler` run took
about 51s against its 15s target. That validation-cost gap remains under RM#106,
with no before/after timing claim from this test addition.

**Slice 4: Guard proofs — Done through PR#327**, merged to `next` on 2026-10-07.
Branch comparisons establish lower/upper facts keyed by local binder identity; these facts prove
guarded conversions and safe `Nat` subtraction within the proven path. Declared
guard invariants and known literals provide proofs where available. Reassignment
invalidates only the written local's fact, while unavailable proofs and nested
condition obligations remain deferred. Known failing conversions reject.
Focused tests cover positive, rejected, deferred and mutation cases, including
parenthesized conditions and loop obligations. This does not complete effects,
emission or executable namespace acceptance; slice 5 is complete through PR#329,
with slices 6–8 and GI#198 still open.

The focused compiler unit suite passes 543 assertions in 0.40s with the native
toolchain already built. Canonical `make check` passes all suites: one run took
125s and the final run took 118s against the 120s target. The final 75s aggregate
unit phase is not a sample of the separate 15s focused incremental target. No
comparable before/after sample establishes a checker regression. The
[performance profile](../../../tests/VALIDATION_PROFILE.md#namespace-guard-proofs--2026-10-07)
records the evidence and next measurement; VM/runtime semantics and bytecode
format are unchanged, and previously accepted programs retain their emitted
behavior.

| Slice | Finish line and required evidence | Dependencies | Provisional remaining PRs |
| --- | --- | --- | ---: |
| 1. Ordinary expressions and control flow — Done through PR#297 | Identity-based checking covers operators, bindings/assignments, branches, loops and returns, including locals referenced within interpolation. Positive and rejected cases preserve lexical identity, evaluation/checking rules and source spans; unsupported paths remain explicit. | Merged body/signature foundations | 0 |
| 2. Aggregates, patterns and generic inference — Done through PR#298 | Records/enums, constructors, collection values, destructuring/matching and inferred generic arguments use declaration identities. Same-spelled nominal types remain distinct inside nested types and patterns; arity, exhaustiveness and inference failures remain useful. | 1; existing generic contracts | 0 |
| 3. Core and indirect-call contracts — Done through PR#321 and PR#326 | Core operations/methods, receiver calls and indirect callable values share checked contracts with direct calls. Reserved core identity cannot be forged; wrong arguments/results and ambiguous receiver/namespace uses reject correctly. | 1–2 | 0 |
| 4. Guard proofs — Done through PR#327 | Guarded conversions, arithmetic obligations and mutation invalidation use identity-aware facts through the completed body checker. Positive, rejected and unavailable proofs preserve existing soundness and exact semantics. | 1–3 | 0 |
| 5. Effects and await | Purity, callable effects, ordinary/async calls and await use the checked graph consistently, including imported/generic/indirect paths and recovered local explanation evidence. Invalid boundaries reject without implying whole-program validity. | 1–4 and callable contracts | 1–2 |
| 6. Identity emission and standalone loading | Complete the seven sequenced acceptance gates below. Keep this an implementation slice: source-map/explain/locate behavior belongs to slice 7; coordinated source/seed migration and platform acceptance belong to slice 8. | 1–5 | Re-estimate by gate; provisionally one focused PR per gate, with adjacent gates combined only when one coherent change proves both |
| 7. Diagnostics, explain and locate — Done on this delivery's merge | Consume stable module/source identities and mappings from slice 6. Preserve precise expression/type/pattern spans through checking and lowering, including declaration-signature types and explicit call type arguments; make qualified selectors unambiguous and unqualified selectors valid only when unique; reject wrong identities, ambiguous selectors and stale mappings safely. Acceptance includes direct and receiver calls with excess and missing explicit type arguments, exact mapped ranges, and stale/mismatched source-map rejection. | 1–6; preserve source identity and spans throughout | 0 |
| 8. Coordinated seed/source cutover and acceptance | Audit initial stdlib exports; migrate compiler, tests, examples and package inputs under one namespace model. Produce the namespace-capable seed, prove fresh compiler/stdlib fixed points and native/browser/installed conformance, document breaking migration, and remove obsolete flat execution and transitional code when its removal criterion is met. Own the broad source/seed cutover and cross-platform acceptance. | 1–7; bootstrap-root staging established before source cutover | 2–3 |

Slice 6 is split into sequential gates within RM#41, not separately prioritised or
weighted tasks. The existing 14% slice weight remains unchanged, and progress is
credited once against the parent slice. Each gate should be reviewable as one
focused PR; combine adjacent gates only when the code and acceptance evidence form
one coherent change.

| Gate | Bounded outcome and exit evidence |
| --- | --- |
| 6a. Callable identity emission — Done through PR#342 | Functions, generic functions, direct/indirect calls and compiler-generated helpers lower from declaration identities to collision-free symbols. Same-spelled cross-module callable fixtures pass, including imported function values invoked indirectly through `PureFn`. |
| 6b. Ordinary body emission — evidence delivered in PR#343 | Operators, bindings/assignments, branches, loops, returns and collections lower the checked body without changing evaluation order or lexical identity; positive and rejected cases remain explicit. |
| 6c. Aggregate and pattern emission — evidence delivered in PR#344 | Same-spelled cross-module `Item`, `Envelope`, `Outcome` and `Finished` identities remain distinct through record/enum/variant emission; nested generic constructors, field access and variant patterns produce exact source and saved-bytecode values. |
| 6d. Proof and effect emission — evidence delivered in PR#345 | Guard-proven conversions/arithmetic, mutation invalidation, purity, async calls and await preserve the checked identity/effect decisions through emitted code; valid and rejected boundaries are covered. The internal Unit expression is identity-checked so async `main` can be emitted and run. |
| 6e. Standalone source loading — evidence delivered in PR#346 | A complete multi-file source program loads its modules into one namespace, resolves the reserved entry identity and entry-directory root, restricts imports to local/toolchain files, confines quoted paths, and launches only the selected entry's `main`. Staged-compiler CLI tests now exercise entry-root and symlink rejection, absent artifacts, and selected-main behavior. |
| 6f. Standalone saved-bytecode loading — evidence delivered in PR#347 | A compiled multi-module artifact retains distinct imported identities and launches only the selected entry after every source file has been removed. The existing v9 format validates and runs the artifact without a format change. |
| 6g. Core loading and bootstrap staging — evidence delivered in this change | A staged root-level entry imports a compiler submodule that shares the implicit core `Option` identity; source and saved-v9 execution select only the entry. Parent-root and symlink escapes reject without publishing artifacts, and temporary staging inputs are removed. Namespace-capable seed generation, fixed-point proof and platform matrix remain slice 8. |

Slice 6 completes only when all seven gates pass together in the supported end-to-end
path. PR#331, PR#333, PR#334 and PR#337 provide earlier partial evidence; PR#342
completes 6a, PR#343–PR#345 deliver 6b–6d evidence, PR#346–PR#348 deliver 6e–6g
evidence, and this delivery adds one integrated staged-root acceptance path. The
path checks source, compiles, removes every source file, then checks and runs the
saved v9 artifact; its assertions cover identity emission, ordinary bodies,
aggregates/patterns, proof/effects and shared core identity. Existing confined-path
rejections remain covered. Slice 6 completes on merge of this evidence. This does
not enable public namespace execution. Slice 6 preserves the source identity and
spans needed by slice 7, but detailed source-map, explain and locate behavior is
slice 7. Slice 8 owns coordinated migration, namespace-capable seed fixed points,
native/browser/installed acceptance and transitional-path removal.

At the slice-5 handoff, **4–7 PRs remained provisionally**, including
implementation, tests, documentation and integration; this was a forecast, not a
commitment. That estimate predates delivery of gates 6a–6g and is superseded by
the current completion assessment below; reassess remaining PR count against
slice-6 integration, slices 7–8 and their acceptance evidence.
Splitting or combining PRs must preserve these finish lines. Each handoff records
the checklist slice, exact revision, positive/negative evidence, remaining gaps,
performance disposition and next action. No slice is complete from assertion
counts alone. Programme weights remain unchanged; P2 retains 20% weight, with
partial completion estimated below and 10 pp formally accepted across the programme.

### Namespace completion assessment

<a id="namespace-completion-assessment"></a>

Current scope baseline, updated 2026-10-11 after PR#358 merged: **100% delivered**
for RM#41, using the unchanged 20/8/12/8/7/10/14/6/15 scope weights. Slices 1–8
are complete on `next`; coordinated migration, seed fixed points, the identity-only
project route, and native/browser/installed acceptance passed. Receiver-context
inference covers unannotated `Map.put` and `Set.add`; the maintained audit passes
all 59 functional, compiler-contract, and top-level example roots. PR#358 records
completion; alpha.12 promotion and artifact verification remain RM#143 gates. See
the merged [PR#358](https://github.com/sproates/panackelty/pull/358) and validation
profile.
The integrated staged-root program exercises all seven slice-6 gates; PR#349
records source and saved-bytecode execution, identity emission, aggregate and
pattern identity, proof/effect behavior, confined loading, and shared core
identity. This replaces the prior 65% post-slice-5 estimate; it is a reasoned
allocation of implementation, tests, integration, migration and acceptance effort,
not measured
completion time, equal slice weights or credit by PR count. The denominator and
P2's 20% parent programme weight are unchanged. Keep these internal weights stable
until a documented scope/estimate revision explains its effect on the total.

| RM#41 outcome | Scope weight | Evidence/state | Completion | Earned contribution |
| --- | ---: | --- | ---: | ---: |
| Binding/signature/body foundations | 20% | Merged PR#261, PR#262, PR#275 and PR#284: graph/re-exports, tagged nominal/parameter contracts and lexical body identities | 100% | 20 pp |
| Slice 1: Ordinary expressions/control flow | 8% | Merged PR#297: checked operations, bindings, branches, loops and interpolation; explicit child obligations | 100% | 8 pp |
| Slice 2: Aggregates/patterns/generic inference | 12% | PR#298: constructor/nested identity, inference and match evidence above | 100% | 12 pp |
| Slice 3: Core/indirect-call contracts | 8% | PR#321 and PR#326: arity, arguments/results, callback contracts, async deferral, reserved identities and namespace/value resolution | 100% | 8 pp |
| Slice 4: Guard proofs | 7% | PR#327: identity-keyed path facts, conversions, arithmetic obligations and mutation invalidation | 100% | 7 pp |
| Slice 5: Effects/await | 10% | Merged PR#329: identity-based pure/ordinary/async call and await checks, imported/generic/indirect paths, recovered valid-sibling evidence | 100% | 10 pp |
| Slice 6: Identity emission/standalone loading | 14% | Seven gates integrated by merged PR#349, including staged-root source and saved-bytecode acceptance | 100% | 14 pp |
| Slice 7: Diagnostics/explain/locate | 6% | Completed through PR#351–#355: retained match-pattern, signature, body-local and call-argument spans; exact under-arity locations for direct and receiver calls; qualified/ambiguous/wrong-identity selectors, mapped ranges, malformed maps and stale source closures in `explain`/`locate` | 100% | 6 pp |
| Slice 8: Source/seed cutover and acceptance | 15% | Completed by merged PR#358: compiler/tooling API migration, standard-library exports, identity-only project route, seed fixed point, native/browser/installed acceptance and 59-root identity-only audit | 100% | 15 pp |
| **Total** | **100%** | **Foundations and slices 1–8 complete in merged PR#358** | **100%** | **20 pp** |

Foundations carry 20% for graph, resolver, public contracts and lexical identity;
aggregate/pattern/generic work is larger than ordinary expressions. Core, proofs
and effects retain 25%; emission/loading plus coordinated cutover retain 29% for
integration, platform validation and migration risk. Existing source spans and
v9 fixed points are evidence within delivered slices, not partial credit toward
later precise tooling or namespace-capable cutover. No scope is removed, and
nothing is counted both as a foundation and a remaining slice.

P2 contributes 20% × 100% = **20 pp**. Including P1's accepted 10 pp, the
programme contribution is **30 pp**. The parse-only migration
diagnostic remains an internal standalone-parser boundary; project commands
filter it and require successful identity checking and emission. PR#358 merged
after its exact-head checks passed; the namespace implementation acceptance is
complete on `next`.

The public namespace execution gate was removed by the accepted identity-based
cutover in PR#358. Deferred or unsupported body obligations still fail closed.
Preserve source identity and spans from the start, rather than rebuilding them at
slice 7. Before P3 manifests exist, bootstrap a root-level entry in an isolated
snapshot of the compiler/bytecode closure; do not permit parent-path escape from
a `src/compiler/` confinement root. Remove staging outputs after each build.
Temporary bootstrap inputs need explicit removal criteria and must not become a
second installed language mode.

PR#331, merged to `next` on 2026-10-08, exercises
same-spelled imported functions, records, enums and variants through source and
saved v9 bytecode, confirms that only the selected entry `main` launches, rejects
relative-root escapes and symlink traversal, and stages compiler builds under an
explicit `src/` bootstrap root. This is a merged checked-subset delivery, not
slice completion. Remaining slice-6 acceptance is tracked as callable, ordinary
body, aggregate/pattern, proof/effect, source-loading, saved-bytecode-loading,
and core/bootstrap gates.
Slice 7 owns precise diagnostics and tooling migration; slice 8 owns
complete seed fixed-point evidence and native, browser and installed conformance.
Website impact: no website update; this
compiler migration changes no published feature claim, and the browser seed
remains unchanged. Performance impact and current canonical timing are recorded
in the latest validation-profile entry; no causal regression claim is made from
the single full-check run.

PR#334, merged to `next` on 2026-10-08, adds two imported modules with colliding
generic `Item[T]`, `make` and `identity[T]` declarations. The complete entry
program checks, runs, compiles and runs saved bytecode with exact output. Its
canonical check passed in 195s against the unchanged 120s budget; the validation
profile records the measurement, sandboxed rerun and accepted disposition under
GI#106/RM#123. This is additional checked-subset evidence, not slice completion.

[PR#337: Nested generic payload emission](https://github.com/sproates/panackelty/pull/337)
extends that fixture through nested generic records inside same-named generic
enum payloads. Source and saved-v9 execution assert the values read back through
each module's variant pattern. This strengthens the identity-emission evidence
without enabling public namespace execution or completing slice 6. Its work
provides partial nominal/variant-emission evidence; source-map/tooling remains in
slice 7, and coordinated seed migration and platform acceptance remain in slice
8. Website impact: none; the public namespace execution gate and current
website feature claims remain unchanged. Its clean canonical check passed in
200.88s against the 120s target and the warm focused compiler check in 54.90s
against the 15s target; the validation profile records both and the limits on
causal attribution.

The 6a follow-up in [PR#342](https://github.com/sproates/panackelty/pull/342)
passes same-spelled imported `indirect_value` functions
from `left` and `right` as `PureFn` values, then invokes each through `call`.
The staged namespace fixture expects distinct results (`43` and `44`) from both
source execution and saved v9 bytecode, alongside the existing generic direct-call
and guard-helper identity cases. This is regression coverage for the existing
emitter path; it changes no compiler or VM implementation. The canonical `make
check` passes both source and saved-bytecode assertions; gate 6a's acceptance
evidence is complete on PR#342's merge. This does not complete slice 6.

The 6b ordinary-body delivery extends those same-spelled imported callable
fixtures with local bindings, mutable assignments, array collections, iteration,
arithmetic, a conditional and a returned tail value. Source and saved-v9 runs
continue to produce exactly `43` and `44`. Unit coverage also verifies that
invalid and deferred bodies fail closed at identity emission. The path was already
implemented in the checked-body adapter, so this delivery adds executable
acceptance evidence without changing compiler or VM source. Its matched
clean `make check` comparison is recorded in the validation profile; this is an
intermediate gate, not completion of slice 6 or public namespace execution.

The 6c aggregate/pattern delivery adds a direct identity assertion for the
nested generic `Envelope` records alongside the existing `Item`, `Outcome` and
`Finished` symbol checks. The fixture's source and saved-v9 executions assert
distinct exact values through nested field reads and variant patterns. This
adds acceptance evidence without compiler or VM source changes. Focused
`make check-compiler` passed in 86s and full host-access `make check` passed in
133s; both exceed unchanged budgets, and these single runs do not attribute the
overruns to the 6c test-only delivery. At that handoff, gates 6d–6g and the
public namespace execution gate remained open.

The 6e source-loading acceptance extends the staged-compiler driver to exercise
the actual command path. A multi-file source fixture verifies the entry module is
selected while imported modules' same-named `main` functions remain uncalled.
Additional CLI checks reject `..` imports and symlink traversal for check, compile
and run as applicable, and verify rejected compilation publishes no bytecode.
The loader and staged CLI path were already implemented, so PR#346 added
end-to-end acceptance tests and progress evidence without changing compiler or
VM source. The checked-in bootstrap seed still reports the namespace migration
gate; public namespace execution remains closed until coordinated slice-8
cutover. Gate 6f is merged in PR#347; gate 6g has acceptance evidence in this
change, while coordinated cutover, slice 7 tooling and complete platform
acceptance remain open.

P2 owns standalone namespace semantics, entry selection and entry-directory
root confinement, necessary initial stdlib exports and its own
bootstrap/native/browser/installed acceptance. P3 owns manifests, explicit
project selection, dependency/package roots, physical identity and race-safe
confinement hardening, and local-package consumption,
including replacement of temporary root staging with a compiler manifest. P4
owns final stdlib grouping/re-exports/prefix migration and obsolete adapter
removal. P8 independently accepts the combined clean/relocated two-application
local-package evidence after P1–P4; it is not counted again as P2 implementation.
The P2 → P3 → P4 implementation sequence, final P8 acceptance and the later conditional
[next integration workflow](next-integration-workflow--active-promotion-acceptance-remains.md#rm-135) remain unchanged.

Per-delivery performance checks and GI#106's owned comparison/remediation review
remain part of P2 delivery. [RM#134: Compiler performance investigation](groomed-development-and-engineering-work.md#rm-134)
/ GI#285 remains unscheduled; neither deep profiling nor another workstream
starts through this plan. Compiler understanding stays paused. This planning
record changes no executable behavior or published capability; no runtime
measurement or website promotion is needed for the record itself.

Historical P2 handoff before merged PR#349: slice-6 integration, precise
diagnostics/explain/locate, and coordinated source/seed migration remained. PR#349
has since completed slice 6. Current progress and remaining slice-7 work are
recorded in the [Namespace completion assessment](#namespace-completion-assessment)
above; slice 8 still owns the namespace-capable seed, source migration, fixed
points and native/browser/installed acceptance. Keep public namespace execution
gated until that acceptance. Website impact remains none: executable namespace
syntax is not a published capability and pinned claims remain accurate.

The following original assessment is historical; P1 delivered its design outcome.
The first outcome is a reviewed design with worked multi-file examples, a current
behavior audit, compatibility/migration decisions and positive/negative acceptance
cases. Cover private helpers, public entry points, aliases, selective imports,
re-exports, same-named symbols, transitive visibility and canonical module identity.
Assessment is provisionally medium / one design PR; implementation cost remains
unknown. Coordinate [package structure](reusable-modules-and-package-structure--idea.md#reusable-modules-and-package-structure--idea),
[dependency management](dependency-management--idea.md#dependency-management--idea) and #106's separate-compilation
design without requiring them to ship together. Main risks are ambiguous lookup,
accidental API exposure and breaking existing programs.

Assess first-class language support for namespaces separately from the existing
logical import paths (`stdlib/...` and `project/...`). Import path organisation
alone does not settle qualified symbol lookup or namespace semantics.

- [ ] Identify real name-collision, discoverability and API-organisation problems
      and representative programs that would benefit from namespaces
- [ ] Specify declaration and qualification syntax, namespace/module relationships,
      nesting, aliases, imports, visibility/exports, collision and shadowing rules
- [ ] Assess interactions with functions, types, generics, method syntax and
      compiler assistance; define useful unresolved/ambiguous-name diagnostics
- [ ] Estimate compiler, bytecode/runtime, bootstrap and tooling implications;
      decide compatibility and migration before selecting an implementation
- [ ] Define focused and public-CLI acceptance tests for lookup, imports,
      qualification, visibility and failures across source and installed toolchains
