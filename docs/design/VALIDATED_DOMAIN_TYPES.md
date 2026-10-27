# Validated domain types: alpha.15 contract

This consolidates GI#435 so implementation can follow one contract. The owner
accepted the six constraint forms, explicit guard binding, validation result,
multiline conditions, grouping, and failure ordering. The final syntax proposal
uses the existing `len()` spelling and adds two named string predicates. This
record supplies the remaining concrete error schema, Unicode pin and acceptance
cases as the final design proposal for review. It does not claim implementation.

[Roadmap scope](../roadmap/records/language-direction-and-differentiation--exploration.md#rm-153)
remains the release decision record. If older discussion conflicts with this
consolidation, use the explicit decisions here once the design is approved.
Do not reopen settled decisions during implementation unless evidence reveals a
specific correctness problem or material scope change.

## Existing behaviour and the correction to the alternatives decision

The current specification already includes `||` in numeric guards.
`evaluate_nat_guard` and `prove_guard_bounds` in `src/compiler/checker.panack`
handle numeric disjunctions. This is bounded existing support, not a general
proof engine. Preserve accepted numeric programs and their supported proofs
when migrating to explicit bindings. Do not reject numeric `||`, remove it,
or claim to deliver it for the first time in alpha.16.

Alpha.16 covers extending alternatives to the new string, collection and
record-relationship constraints, including static proof and runtime diagnostics.
That extension remains a scheduled outcome, not an unscheduled backlog item.
Alpha.15 combines the new constraints with `&&` and parentheses. No arbitrary
user predicate evaluator, new arithmetic guard fragment or general theorem prover
is selected. Preserve existing numeric guard capabilities beyond these new forms.

## Declarations and methods

```panackelty
type NonEmptyText = Str where text => text != ""

type Label = Str where text =>
  (text.len() >= 1 && text.len() <= 80)
  && text.is_non_blank()
  && text.has_no_control_characters()

type SmallNames = [Str] where names =>
  names.len() >= 1 && names.len() <= 20

record Range {
  start: Int,
  end: Int
}

type OrderedRange = Range where range => range.start <= range.end
```

These are target declarations, not examples runnable on the current compiler.
The binding is programmer-chosen and local to the guard. Parentheses only group
expressions; line breaks after `=>` and before `&&` continue the condition.
A following declaration ends it. Migration covers source, examples, fixtures and
bootstrap together; no implicit `value` compatibility mode remains afterwards.

| Form | Contract |
| --- | --- |
| `text != ""` or `text.len() >= 1` | Non-empty text; equivalent proof facts. |
| `text.len() >= minimum`, `text.len() <= maximum` | Inclusive, non-negative compile-time constant bounds in Unicode scalar values. Either bound or both. |
| `text.is_non_blank()` | New pure `Str` method returning `Bool`. At least one scalar outside the pinned whitespace set. Empty text returns false. |
| `text.has_no_control_characters()` | New pure `Str` method returning `Bool`. No U+0000–U+001F or U+007F–U+009F. Empty text returns true. |
| `items.len() >= minimum`, `items.len() <= maximum` | Existing array count operation; add pure `len(): Nat` methods to Map and Set using the same bound rules. Map counts entries; Set counts distinct elements. |
| `record.left < record.right`, `record.left <= record.right` | Two direct fields of the same record with the same underlying ordered numeric type. No nested traversal, arithmetic or separate-argument relationships. |

Use `len()`, not the previously illustrated `length()`. The current string API
counts code points and rejects surrogate code points in valid UTF-8, giving the
agreed scalar-value semantics. Arrays already expose `len()`. Map and Set currently lack a public count method
in the owned-intrinsic registry; their new `len(): Nat` methods are an explicit
prerequisite within this scope, available outside guards as well. Bytes retains
its separate existing `length()` API and is not a selected guard family.
The two new predicates are ordinary usable methods outside guards as well.
They must have one shared definition used by runtime validation and static proof;
do not maintain divergent compiler and library character classifications.

Reject contradictory length bounds at declaration with both bounds and locations.
Accept redundant compatible constraints; report each failed written conjunct in
its declared order rather than silently changing the declaration. This preserves
source attribution even when two conditions express the same requirement.

Reuse through values already carrying compatible guarded guarantees remains
supported. This contract adds no type-inheritance syntax, arbitrary predicate
helpers, or new guarded-base declaration composition mechanism.

## Unicode data

Pin non-blank classification to Unicode **17.0.0**, property `White_Space` from
[PropList.txt](https://www.unicode.org/Public/17.0.0/ucd/PropList.txt).
The 25 scalars are U+0009–000D, U+0020, U+0085, U+00A0, U+1680, U+2000–200A,
U+2028, U+2029, U+202F, U+205F and U+3000. Vendor this small table with its source,
version and applicable Unicode licence notice; no runtime download or host locale
lookup. No additional Unicode library is required for this bounded classification.

Whitespace and controls are distinct. U+00A0 is whitespace but not an excluded
control; a tab is both. U+200B is neither in these sets. Non-blank does not promise
visible text. No normalisation, trimming or grapheme counting occurs. A Unicode
version upgrade is a deliberate semantic change with tests and release notes.

## Construction, updates and validation

Keep ordinary guarded construction proof-only: `Label(input)` succeeds only when
the compiler can establish every requirement. A missing proof is a compile-time
error explaining which fact is missing and pointing to explicit validation.
Do not silently insert runtime checks or rely on a mutable variable's old value.

`validate[Label](input)` returns `Result[Label, ValidationError]`, with
`Ok(guarded_value)` or `Error(details)`. The earlier chat's closing `>` was a typo;
type arguments use square brackets. Evaluate input once, preserve it unchanged,
and never print automatically. Compiler-supported `validate` is import-free;
public error types can be imported from `stdlib/validation`. Wrong base types and
unsupported target declarations are compile-time errors, not invalid user data.

Validation targets the declared base type. It is not a JSON decoder or a coercing
record mapper: record arguments already obey their field types. Existing guarded
fields remain valid. Field error locations apply to validation failures attributed
to a field; this design does not invent a parallel untyped record representation.
Cross-field failures identify both relevant fields.

Copying, passing through calls, generics and module boundaries, and storage retain
proved guarantees. Operations producing changed strings or collections return
ordinary base values unless the checker proves the resulting constraints. A size
constraint never implies arbitrary index validity or key membership.

For record updates, construct a complete ordinary base record, preserving all
unchanged fields explicitly, then prove or validate the replacement. For example:

```panackelty
candidate = Range(new_start, previous.end)
result = validate[OrderedRange](candidate)
```

No new record-update syntax or intermediate invalid guarded record is introduced.
An unchanged typed field retains its own guarantees. A replacement assigned to a
guarded variable must first establish the whole target invariant.

## Structured errors and rendering

Proposed public records and enums in `stdlib/validation`:

```panackelty
record ValidationError {
  target_type: Str,
  failures: [ValidationFailure]
}

record ValidationFailure {
  target_type: Str,
  locations: [ValidationLocation],
  constraint: ValidationConstraint,
  facts: ValidationFacts
}

enum ValidationLocation { WholeValue, Field(Str) }
enum ValidationCountUnit { UnicodeScalars, ArrayElements, MapEntries, SetElements }
enum ValidationRelation { LessThan, LessThanOrEqual }
enum ValidationConstraint {
  MinimumCount(ValidationCountUnit, Nat),
  MaximumCount(ValidationCountUnit, Nat),
  NonBlank,
  NoControlCharacters,
  FieldRelation(ValidationRelation),
  NumericPredicate(Str)
}
enum ValidationFacts {
  ActualCount(Nat),
  BlankText,
  ControlCharacter(Nat, Nat),
  RelationNotSatisfied,
  PredicateNotSatisfied
}
```

These declarations specify the public data shape; their compiler/library wiring
is implementation work. `target_type` uses the compiler's unambiguous type identity.
`Field` stores a declared field name, never input text. A relationship lists its
left and right field locations in expression order. `ControlCharacter` stores
first offending zero-based scalar index and scalar number, not the original text.
One failure per failed written constraint; do not emit a failure per character.
`NumericPredicate` stores the declared expression with the explicit binding,
not substituted input values. Existing numeric alternatives remain a single
failed alternative requirement; do not render their branches as mandatory clauses.

Count constraints pair with `ActualCount`; non-blank pairs with `BlankText`;
control exclusion with `ControlCharacter`; relationships with `RelationNotSatisfied`;
numeric predicates with `PredicateNotSatisfied`. The implementation must maintain
these pairings and never create empty failures for a rejected value. Numeric
constraints can be explained without retaining the potentially sensitive input.

Emit all failed conjuncts, left to right through parentheses. Preserve field
failure declaration order before cross-field constraints. Optimisation may share
scans and count calculations, but must not suppress failures or reorder them.
Success carries no failure data. Diagnostic metadata must survive saved bytecode.

Provide the pure owned method `ValidationError.render(): Str`, implemented in
Panack. It returns plain text without ANSI escapes or terminal-width-dependent
wrapping. No trailing newline; the caller chooses where to display or log it.
Always name the target type and explain each failed requirement. Use singular
and plural units correctly. Never print the original input by default.

Required examples, with separate paragraphs for each failure:

> Cannot validate the value as Label.
>
> Label requires at least 1 Unicode scalar value, but the string is empty.
>
> Label requires at least one character that is not Unicode whitespace, but the string is empty.

The earlier agreed one-failure empty-string example applies to a type declaring
only the 1–80 length bounds. This Label also declares non-blank, so both fail.
The maximum-length and control predicates pass for empty text.

> Cannot validate the value as SmallNames.
>
> SmallNames allows at most 20 array elements, but the array contains 21.

> Cannot validate the value as OrderedRange.
>
> OrderedRange requires field 'start' to be less than or equal to field 'end', but that relationship is not satisfied.

> Cannot validate the value as Label.
>
> Label does not allow control characters, but U+0009 occurs at character position 2 (counting Unicode scalar values from 1).

Compile-time failures use the existing source-aware diagnostic framework. They
must distinguish an unsupported condition, contradictory declaration, known
invalid value and missing proof. Show the relevant declaration and use site,
explain the cause, and offer `validate[Type](input)` only where runtime validation
is applicable. Do not direct users to validation for an unsupported predicate.

## Finite acceptance and implementation sequence

No implementation is authorised by this planning record. When scheduled:

1. Migrate explicit bindings while preserving existing numeric proofs, including
   accepted and rejected `||` cases; test parentheses, multiline parsing, name
   scope, diagnostics and bootstrap. No new arbitrary expression support.
2. Add the shared text predicates, Map/Set count methods, string and collection
   guard forms and proof facts. Verify static acceptance, static rejection and unknown-input missing
   proof for each form; preserve scalar/count semantics across host platforms.
3. Add explicit runtime validation, the error schema and renderer. Exercise every
   constraint, multi-failure ordering, sensitive-input omission, numeric guards,
   public CLI and saved bytecode. Demonstrate both successful and failed results.
4. Add record relationships and replacement validation, guarantee preservation and
   loss across transformations, copying, reassignment, calls, generics and modules.
   Integrate GI#134 proof explanations and GI#173 source attribution once.
5. Finish complete examples/transcripts and installed acceptance, specification,
   reference, migration guidance and comparable validation-cost measurements.

Each slice includes meaningful tests and applicable canonical product validation;
these are proposed delivery boundaries, not permission to defer tests to the end.
Expect roughly five implementation PRs, revised only for concrete dependencies.

Boundary cases: empty, minimum, maximum and one outside each bound; composed and
decomposed accents; supplementary scalars; every whitespace-table boundary;
U+001F/U+0020, U+007E/U+007F, U+009F/U+00A0; U+200B; empty/oversized arrays,
maps and sets; map replacement and duplicate set insertion; ordered, equal and
reversed record fields; failed constraints before and after a passing conjunct.
Preserve existing valid numeric programs after the mechanical binding migration.

Document measured runtime costs without claiming validation is free. Evidence
must include workload, data size, platform and revision. The design itself changes
no runtime or performance behaviour. The website's broad planned domain-type
claim remains accurate; detailed documentation follows implementation and release.
