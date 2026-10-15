# Compiler explanations: expression and argument types, guarded conversions, subtraction and effects

`panack explain SOURCE.panack --function NAME` checks the complete project and
reports inferred local types with their checked initializer types, the checked
type of an expression at a source location, function return checks, direct,
core and indirect call argument and arity checks, native TCP/HTTP transport
argument requirements, explicit type-argument counts,
the `len` input-type requirement, selected text/byte/numeric/collection/path,
environment, time and filesystem builtin argument checks, and annotated local
initializer and assignment value checks, plus retained array `map` and `reduce`
callback contracts, unary/binary operator type decisions, indexing and
iteration and field access type checks, Boolean condition requirements for
`if` and `while`, and `match` variant membership. It also reports retained proof-bearing guarded
conversions, natural-number subtraction and effect-boundary evidence, and
array/conditional/match/generic parameter type joins for one function. It reads
source; it does not
execute the program, emit bytecode, write files, or consume a source-map sidecar. The
command is implemented in unreleased core; existing published/browser versions
do not acquire it automatically.

## Completion target for GI #134

The owner-defined target is comprehensive function-checking coverage: `panack
explain` must report every relevant type, effect and proof decision the checker
makes while checking a function. For each decision it should expose the actual
accepted/rejected outcome, supporting checker facts and source location. If
checking cannot reach a decision, it should report `unavailable` with the reason.
If the checker did make a decision but the explanation does not cover it yet,
that is an implementation gap, not an acceptable unavailable result. No relevant
decision should be silently omitted.

The query families documented below are delivered partial coverage, not a claim
that the target is complete. Before selecting further implementation slices,
inventory the relevant checker decisions and map each one to its explanation
output and regression coverage. This target concerns decisions made while
checking functions; it does not by itself expand ordinary diagnostic wording,
change language semantics, or add parse/resolution or compilation-provenance
explanations.

## Function-checking decision inventory

The normal project loader calls the module-body checker and the effect checker;
Explain asks those same passes to retain evidence. The table maps those active
decision families to the current output and known gaps. “Covered” means there is
an explanation regression for accepted, rejected, and/or unavailable outcomes
where that outcome is reachable; it does not mean every special case in the
family is complete.

| Checker decision family | Checker source site | Explain evidence and regression source | Remaining coverage gap |
| --- | --- | --- | --- |
| Local types and function results | `bind_body_block` records annotated initializers and assignments; `body_incomplete_local` classifies unresolved locals; function result compatibility is retained while checking the body. | `BindingTypeEvidence`, `AssignmentTypeEvidence`, and `ReturnTypeEvidence`; compiler-module and CLI cases in `tests/runner/compiler_module_bindings_unit.panack` and `tests/functional/cases/cli_explanations/main.panack`. | The local result is reported, but a complete account of how constraints, generic substitution, and later solutions produced that type is not retained. Deferred and incomplete-result reasons need broader source attribution. `Void` rejection for unannotated bindings has no dedicated evidence record. |
| Call arguments and arity | `body_call`, `body_call_arity_evidence`, `body_explicit_type_argument_arity_evidence`, and the `body_check_*_core_call` helpers in `module_bodies.panack`. | `CallArgumentEvidence` and type-check evidence rendered by `explanations.panack`; focused module tests and the CLI fixture cover direct, constructor, indirect, core, TCP/HTTP, and selected builtin families. | The call sites are mapped, but coverage is representative rather than exhaustive across every builtin and every rejection/deferred path. Generic substitutions are visible in expected argument types; their complete inference trail is not. |
| Explicit type-argument counts | `body_call`, `body_check_special_core_call`, and `body_retain_rejected_type_argument_arity`. | Type-argument-arity evidence; module and CLI regressions cover generic functions, non-generic calls, and generic record/enum constructors. | Resolution failures are outside this body-checking scope. The complete set of accepted type-argument-bearing targets still needs fixture-by-fixture confirmation. |
| Generic inference and constraints | `body_infer`, `body_unify`, `body_apply_solutions`, `body_finish_constraints`, `body_solve`, and `body_call`. | Selected repeated-parameter joins use `TypeJoinEvidence`; inferred locals and substituted call arguments have output. Regressions live in the module-binding unit suite and CLI explanation fixture. | First-source inference, failed or deferred inference reasons, constraint application, and every hole/unification decision are not retained as a complete trace. This is a confirmed implementation gap. |
| Type joins and result types | `body_join`, `check_body_array`, `body_check_collection_core_call`, `check_body_conditional`, `bind_body_match`, and `body_infer`. | `TypeJoinEvidence` reports array, concatenation, conditional, match, and selected repeated-generic joins. Module and CLI regressions exercise accepted, rejected, and unavailable cases. | Other constraints that refine an expression result or alter an inferred type have not yet been mapped to evidence; join-site reachability is not exhaustive. |
| Operators and expression requirements | `check_body_operation`, `check_body_index`, `check_body_field`, `body_iteration_item`, `check_body_conditional`, and `bind_body_match`. | `OperatorTypeEvidence` and `BodyTypeCheckEvidence`; regressions cover operands/results, index receiver and position, fields, loop iterables, branch conditions, and match rules. | Special core operations are listed separately below. Deferred decisions need source-specific reasons wherever the checker has facts that explain the deferral. |
| Specialized core calls and callbacks | `body_check_special_core_call` dispatches to the text, bytes, path, filesystem, time, environment, TCP/HTTP, collection, rational-conversion, and typed-intrinsic checkers. `body_check_collection_core_call` checks callback effect, arity, parameter, and result contracts. | Argument and callback evidence is rendered by `explanations.panack`; `tests/runner/compiler_module_bindings_unit.panack`, `tests/runner/compiler_explanations_unit.panack`, and the CLI fixture cover selected families. | `body_check_typed_intrinsic` retains argument evidence for `fs_read`, `fs_read_beneath`, `fs_write`, `fs_metadata`, `fs_list`, `fs_directories_overlap`, `fs_create_directory`, `fs_remove_file`, `fs_remove_directory`, `fs_temp_file`, and `fs_temp_directory`. Decisions for `run_bytecode_args`, `process_exit`, `process_run`, `host_sleep`, `host_decode_utf8`, `read_file`, `file_exists`, `read_bytes`, and `write_bytes` lack it. Collection receiver-shape rejection and every callback constraint path also need case-by-case coverage. |
| Match rules and payload bindings | `bind_body_match` checks subject, variant membership, duplicate arms, payload binding count/type, exhaustiveness, and arm-result joins. | `BodyTypeCheckEvidence` and `TypeJoinEvidence`; compiler-module and CLI regressions include generic payload substitution and unresolved subject types. | Name-resolution failures are outside scope. Deferred/malformed body patterns and every variant/payload decision branch still need exhaustive outcome mapping. |
| Proof obligations | `body_guard_conversion`, `body_proves_nat_subtraction`, and the condition-fact helpers in `module_bodies.panack`. | `GuardConversionEvidence` and subtraction proof records rendered by `explanations.panack`; proof unit tests and CLI cases cover guarded conversions and Nat subtraction facts. | The proof-producing source paths need a complete audit to ensure no type/proof obligation is embedded in a specialized operation or generic constraint without its own explanation evidence. |
| Effects | `module_effect_call`, `module_effect_expression`, and `check_module_effects` in `module_effects.panack`. | `EffectEvidence`; compiler explanation unit and CLI regressions cover call/await boundaries, invalid-body availability, and representative effects. | Declaration-level effect decisions and effects carried through inferred callable/generic types are not yet mapped exhaustively to evidence and source facts. An unresolved indirect-call effect retains no boundary record, so output can incorrectly say no boundary was retained even though the checker saw a call. |

The source-site inventory exposed checker decisions that currently have no
dedicated explanation evidence:

- `bind_body_string` accepts, rejects, or defers interpolation based on the
  local's scalar or guarded type.
- `bind_body_block` rejects storing `Void` in an unannotated binding, and
  `check_body_array` rejects a `Void` element. Array-join evidence does not
  explain the separate `Void` rejection.
- `body_check_collection_core_call` and `body_check_map_set_call` reject
  incompatible collection receivers. Callback evidence does not explain an
  invalid array/map/set receiver.
- `body_check_typed_intrinsic` checks argument types for process, host and
  legacy file operations, but currently retains argument evidence only for the
  enumerated `fs_*` subset in the table above.
- `module_effect_call` sees indirect calls with unresolved callable effects but
  does not retain an unavailable effect-boundary record. The renderer can then
  report that no boundary was retained, which loses the reason this call's
  effect is unknown.

These are confirmed gaps because the checker already makes and diagnoses the
decision; they must not be represented as acceptable “unavailable” outcomes.
Separately, several evidence renderers use a generic unavailable reason instead
of the specific unresolved fact, and generic inference retains only selected
joins rather than the complete hole/constraint history.

This inventory maps the active function-body type/effect decision families to
their checker sites, explanation record families, regression suites, and known
evidence gaps. “Covered” remains local to the listed cases; it does not claim
every branch is complete. The gaps are implementation work, not accepted
omissions. Parse, name-resolution, signature-declaration, and
compilation-provenance questions are recorded separately because they are
outside this issue's function-body decision scope. Each implementation slice
must add evidence at the decision site, preserve ordinary checking behavior,
and cover its reachable outcomes.

For staged multi-module projects, qualify a query with the portable module
identity, for example `--function project/library.panack:lookup`. A bare name
continues to work when it selects one loaded function. When multiple modules
define that name, the query reports ambiguity; a module path that does not own
the requested function is rejected. Explain and locate use the same selector
rules, so retained evidence cannot silently come from a same-named function in
another module.

For example:

```panack
pure remaining(n: Nat): Nat {
  if n >= 2 { n - 2 } else { 0 }
}
main(): Void { print(remaining(5)); }
```

`panack explain example.panack --function remaining` reports the `n - 2`
obligation, right constant `2`, retained lower bound `2`, and the true branch of
`n >= 2`, with the original source ranges of both expressions. If the subtraction
is changed to `n - 3`, the same bound is reported as insufficient. A write that
invalidates a guard also discards its origin; a subsequent fresh guard supplies
new evidence. False branches are labelled explicitly.

## Meaning of the output

- `program: accepted` means the normal resolver, type checker and purity checks
  accepted the entire captured project. `program: rejected` means they did not.
  A local proof can still be reported in a rejected program; it does not override
  other diagnostics, purity failures, or return-type errors.
- `subtraction: proved` means the checker's actual Nat underflow obligation passed
  using retained constants or a name's lower bound against a right constant.
- `subtraction: unproved` means the checker could not discharge that obligation.
  It is not a demonstrated runtime failure. The query does not invent missing
  assumptions, propose a minimal repair, or run a separate prover.
- `subtraction: unavailable` covers other numeric domains, invalid operands, or
  no recorded subtraction in the requested function. Parsing/resolution failures
  can prevent checking entirely. Missing source positions are also explicit.
- Type and purity failures retain ordinary diagnostics on stderr and exit status
  1. Accepted projects exit 0, including a valid function with no supported
  explanation. Invalid arguments or a missing function in a valid project exit 1.

## Inferred local types

For locals without an annotation, `inferred locals` reports the checked
initializer expression and its source range, the initializer's checked type, and
the resulting local type. For example, `copy = identity("hello")` reports the
generic call at its source location and the inferred `Str` type. Annotated locals
are omitted. If checking could not determine a complete type, the result is
`inferred type: unavailable`; the query does not invent a type or change inference
rules. The output describes the checked definition, not a runtime value or a
call-site specialisation.

## Function return types

The function explanation reports the check of its final result against the
declared return type. `return type: accepted` or `rejected` comes from the
existing checker decision and includes the result expression's checked type,
declared type and source range. `return type: unavailable` marks a deferred
compatibility check. If the checker rejects an incomplete result, the boundary
can be `rejected` while `actual type` is `unavailable`; the query does not fill
in an unresolved type. The normal diagnostic and exit status remain unchanged.

## Direct function-call arguments and arity

For each checked direct function or record/enum constructor call in the selected
function, the explanation reports each argument's checked type against the
corresponding parameter or payload type.
Accepted, rejected and unavailable outcomes come from the existing checker
decision and include the argument expression's source range. The same applies
to checked arguments for the text helpers `is_digit`, `is_letter`,
`is_whitespace`, `starts_with`, `starts_with_at`, `reverse` and `slice`, and the
byte helpers `byte_append`, `bytes_concat`, `byte_len`, `byte_get`,
`utf8_encode` and `utf8_decode`, and the numeric helpers `nat`, `dec`,
`nat_from_str` and `quotient`. The collection checks cover append items, map
keys/values, set items and membership keys for `append`, `map_put`, `map_get`,
`map_has`, `set_add`, `set_has` and `has`, including their receiver-method
forms. Path checks cover the typed arguments to `path_from_text`,
`path_from_native`, `path_append`, `path_to_text`, `path_native_bytes`,
`path_display`, `path_absolute`, `path_directory`, `path_filename`,
`path_join`, `path_suffix`, `path_parent`, `path_is_absolute`,
`path_with_suffix` and `path_resolve`. Other generic collection-call joins,
callback contracts for other operations, and other builtins remain outside
these slices. The environment checks cover the
string key arguments to `environment_get` and `environment_has`. Diagnostics and
exit status remain unchanged. Time checks cover arguments to `duration_nanoseconds`,
`duration_ticks`, `duration_from_seconds`, `instant_add`, `instant_difference` and
`instant_before`. Direct calls to resolved function declarations and record/enum
constructors also report whether the supplied argument count matches the
declared inputs, with the call source range, expected and supplied counts, and
the existing accepted or rejected outcome. Core builtins, indirect calls and
explicit type-argument counts remain uncovered call-arity decisions.

## Array callback contracts

For receiver calls to `[T].map(callback)` and `[T].reduce(initial, callback)`,
the explanation reports the callback's checked callable type, expected parameter
types, result type and accepted, rejected or unavailable outcome. It reflects
the existing checker requirements: callbacks must be pure and have the required
arity; each supplied element or accumulator must be accepted by the matching
callback parameter; `map` callbacks must return a value; and `reduce` callback
results must be compatible with the initial accumulator type. The source
location identifies the callback expression. Original diagnostics and type
acceptance remain unchanged; ordinary checking retains no callback explanation
evidence.
The filesystem I/O checks cover the `Path` and byte-limit inputs to `fs_read`
and the `Path` and `Bytes` inputs to `fs_write`. Rooted filesystem read checks
cover the root and relative `Path` inputs and the `Nat` limit to
`fs_read_beneath`. Filesystem query checks cover
the `Path` inputs to `fs_metadata`, `fs_list`, `fs_directories_overlap`,
`fs_create_directory`, `fs_remove_file`, `fs_remove_directory`, `fs_temp_file`
and `fs_temp_directory`.

## Annotated local initializers

For each explicitly annotated local in the selected function, the explanation
reports the initializer's checked type against the annotation, with the existing
accepted, rejected or unavailable decision and the initializer's source range.
Inferred locals remain in the inferred-local section. Assignment checks are not
included here; diagnostics and exit status remain unchanged.

## Assignment values

For each assignment to an existing local, the explanation reports the assigned
value's checked type against the local's known type, with accepted, rejected or
unavailable and the value's source range. This describes type compatibility
only; a separate immutability error can still reject the assignment. Diagnostics
and exit status remain unchanged.

## Checked type joins

The query reports the checker's existing type-join decisions when it combines
array literal element types, the element types of the arrays passed to `concat`,
the two results of an `if` expression, successive match arm results, or repeated
arguments that constrain one generic function type parameter. Each entry
identifies the join context and source location, both input types, the resulting
type when determined, and whether the join was accepted, rejected or
unavailable. For generic calls, only repeated constraints
that reach the checker's join operation are shown; this is not a complete trace
of generic inference. An unavailable join means the checker could not derive a
complete result from the available types; it does not imply that the types are
incompatible. Rejected joins keep the original diagnostic and exit status. This
query does not add coercions or change type inference.

## Unary and binary operator types

For checked unary and binary expressions, the query reports the operator,
operand types, resulting expression type, source range, and the existing
operator type decision as accepted, rejected, or unavailable. This reports the
type-checking decision only. In particular, an accepted `Nat` subtraction
operator type does not mean its separate underflow proof succeeded; use the
subtraction explanation for that proof. Ordinary diagnostics and program
acceptance remain unchanged, and ordinary checking retains no operator
explanation evidence.

## Checked type requirements

For an indexed expression, the query reports two independent checks: whether
the receiver is a supported indexable type (`Str`, `Bytes` or `Array[T]`), and
whether the position has type `Nat`. The receiver check includes the resulting
element type when available. For a `for` loop, it reports the iterable type,
the accepted iterable forms (`Range`, `Bytes` or `Array[T]`), and the resulting
item type when available. For field access, it reports whether the receiver
is a record containing the requested field and the resulting field type when
available. For `if` and `while`, it reports whether the condition has the
required `Bool` type. For each `match` arm, it reports whether the
resolved variant belongs to the matched enum. It also reports whether each arm
is unique, whether its pattern binds the variant's required payload count, and
whether every enum variant is covered. Exhaustiveness results name any
missing variants. The enum-subject decision is separate; when the subject is
not an enum, exhaustiveness is reported as unavailable because there is no
variant set to compare. These decisions include the subject type when
available, the rule checked, their source range, and an accepted, rejected or
unavailable status. These are local checked-body results;
`program: accepted/rejected` remains the whole-project result. Original
diagnostics and ordinary checking are unchanged, and ordinary checking retains
no checked type requirement or match decision evidence.
For a generic enum payload binding, the explanation shows the matched enum type,
the variant payload assigned to the binding, and the substituted binding type.
Its source range is the full match-arm pattern, because the parsed pattern does
not retain a separate range for each binding name.

Add `--type-at LINE COLUMN` to query the checked expression covering a
one-based line and Unicode code-point column in the selected function's source
file. If expressions overlap, the query reports the smallest source range, so a
cursor inside a call argument selects that argument while a cursor on the call
selects the call. Source ranges remain half-open. A location outside a checked
expression, or a function whose body did not complete type checking, reports
`checked expression type: unavailable`; the query does not infer from diagnostics
or reconstruct missing constraints. The ordinary function explanation is still
included in the output.

Coordinates are one-based lines/Unicode code-point columns and half-open ranges
in the captured source text. Paths use `project/`, `stdlib/`, or the source-map
identifier rules for external paths. Control characters in paths and excerpts
are escaped. Source excerpts are read from the loaded snapshot, not reread to
reconstruct proofs. As with ordinary compilation, this is not an atomic snapshot
of a concurrently edited source tree.

## Effect-boundary explanations

The same command also reports the effect pass's actual decisions for calls,
`await` context/operands and rejected discarded non-Unit await results. For example,
`pure report(): Void { print(1); }` reports a rejected call boundary at `print(1)`,
with pure context, ordinary callee effect and the existing purity rejection reason.
An ordinary function calling `print` reports an allowed local boundary.

- `effect boundary: allowed` means that particular boundary passed its existing
  effect rules. It does not imply that argument expressions, the surrounding await,
  another function, or the complete program passed. Each nested boundary is checked
  separately; use `program: accepted/rejected` for whole-project validity.
- `effect boundary: rejected` reports each violation retained at that boundary by
  the actual pass. The renderer does not parse diagnostics or recheck effects.
- Calls show the enclosing mode (`pure`, `ordinary`, `async`), callee classification,
  whether directly awaited, and whether classification came from a named declaration,
  a callable type, or a builtin/constructor contract. An ordinary declaration stays
  ordinary even when its body currently does nothing. A `PureFn` widened to `Fn`
  is classified by its declared callable type, not the original function body.
- Async calls require await in an async context; async contexts reject blocking
  ordinary calls. Await context and operand checks are separate boundaries.
- After a function body type error, the query still checks local effect boundaries
  in separately type-valid functions. Eligibility includes the complete body and
  return type check; it never recovers individual expressions inside an invalid
  function. Loading, parsing, resolution or type errors in signatures or type declarations
  prevent recovery throughout the project, including earlier valid functions.
- `effects: unavailable` distinguishes a global frontend failure, an invalid
  selected function, and a checked function with no call/await boundary.
- Recovery is explanation-only. Original diagnostics on stderr, their order, exit
  status and ordinary compilation's type-first behavior are unchanged. Recovered
  effect violations appear in the local explanation, not as additional stderr
  diagnostics. The project remains rejected. An allowed call to a valid declaration
  does not establish that the callee body type-checks or that a transitive call is safe.

Both branches and loop bodies are checked statically. Output does not say a branch
ran, infer transitive effects, identify a dynamic callable implementation, or follow
callee bodies to find an ultimate I/O operation. Generic/imported definitions keep
their own source positions and are checked once. Type-declaration guard diagnostics
still run when effects are checked, but their evidence is outside this function
query. A type-valid guard with an effect violation does not disable local function
evidence. After unrelated body type errors, that guard violation is not added to
stderr: the original type-first diagnostics are preserved, and the project remains
rejected. There is no separate
capability taxonomy such as Network or Filesystem in this output.

## Guarded-conversion explanations

The query reports conversions into a guarded type when the checker consults
value facts to decide whether the target guard is proved. For example,
`positive: Positive = value` inside `if value > 0` reports the retained local
bound and the target guard. A conversion from one guarded type to another also
reports the source invariant used to prove the target. Output includes the
source range, source and target types, available bounds, and whether the proof
passed, failed, or could not be decided. An unproved conversion does not prove a
runtime failure, and the normal diagnostic remains unchanged.

This slice covers proof-bearing conversions into guarded targets, including
core values and guarded source invariants. The checker accepts a guarded source
for a compatible guarded target only when its invariant proves the target
guard; this also permits conversions that previously failed, such as
`Positive` to `NonNegative`. Stricter target guards still require sufficient
facts. Widening a guarded value to its unguarded base type is not a
proof-bearing conversion and is omitted. The query does not infer new facts.
Guard-conversion evidence is retained only by the explicit `explain` query;
ordinary project checking keeps the evidence collections empty.

## Boundaries and cost

This first query covers binary Nat subtraction in function definitions. Generic
functions are checked once at their definition; output does not claim call-site
specialisation or runtime path execution. Imported definitions keep their own
source locations. A query can include multiple subtraction obligations in checker
traversal order, including nested subtractions and both branches.

The checker adds the narrow guarded-source conversion described above;
explanation mode records that same checker decision. Other proof rules and
acceptance semantics are unchanged. Variable-to-variable relations, compound
guard inference and arithmetic constant folding are not added.
The original checker may reject mathematically safe expressions outside its
supported proof rules. Constant values are retained, but declaration/initializer
provenance chains are not; the operation range identifies the operands being
checked. Type-declaration guards are not part of this function query. General type explanations, transitive effect explanations, compilation provenance, suggested requirements, change
predictions and runtime value derivations remain separate programme work.

Only the explicit query retains subtraction/effect evidence, lower-bound origins,
and guarded-conversion decisions. Ordinary checking carries empty
evidence/availability collections and omits guard origins. Effect decisions
share the same traversal and predicates in both modes; opt-in records retain local
reasons and source spans, with no extra type analysis pass. After body errors only the query runs the
filtered effect traversal; ordinary compilation skips it. No evidence
is serialized into bytecode, and the bytecode format remains v9. The query checks
the whole loaded project before filtering output to the function. It uses bounded
source loading (1 MiB per file; 8 MiB and 256 files per closure); the existing
source-map loader reports failures for exceeded bounds. This is a source-size
bound, not a hard guarantee on compiler time, memory, or output size. Plain text
is the current presentation, not a versioned machine-readable protocol.

## Query turnaround baseline

On macOS 26.5 arm64, five end-to-end CLI queries against a two-module project
with an imported generic function and a guarded conversion took 0.05s, 0.04s,
0.04s, 0.05s and 0.04s (`real` time). Each sample launched a new VM process and
included project loading, checking and rendering; the filesystem cache was warm
after the first query. The median was 0.04s and the observed range was 0.04–0.05s.
Use 0.25s per query on this fixture and host as the initial useful turnaround
target, giving five times the observed maximum for source-size and host variance.
This is a local comparison target, not a CI timing gate or a build-performance
investigation.

Acceptance coverage lives in `tests/runner/compiler_explanations_unit.panack`,
`tests/runner/compiler_module_bindings_unit.panack` and
`tests/functional/cases/cli_explanations`, plus the existing mutation/checker,
bootstrap and public-CLI suites. Measurements and limitations are recorded in
[the validation profile](../tests/VALIDATION_PROFILE.md). This bounded delivery
completes neither #134's whole types/effects/proof scope nor programme #180.
