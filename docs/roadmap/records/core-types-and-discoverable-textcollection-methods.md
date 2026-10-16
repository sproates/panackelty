[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Core types and discoverable text/collection methods

<a id="rm-40"></a>

**RM#40: Core types and methods** · [GI#116: Core types and methods](https://github.com/sproates/panackelty/issues/116).

Work record: [issue #116](https://github.com/sproates/panackelty/issues/116).
State: Done. Completed in PR #118, merged and deployed on 2026-09-30.
Canonical checks, browser suites and live import-free core/method execution passed;
issue #116 is closed. The cached-example defect discovered afterwards is tracked
separately below. The following scope records the accepted implementation.
Recorded and groomed on 2026-09-30 following hands-on website playground feedback.
The user selected implementation on 2026-09-30 as one cohesive PR covering core
types, text/collection methods and migration. The user approved the merge.

`Option[T]` and `Result[T,E]` should be usable without stdlib imports, and
ordinary text/collection operations should be discoverable as methods on values
of the appropriate type. Existing import-free `.starts_with()` and `.reverse()`
make the imported `text_ends_with` helper inconsistent. The proposed equivalent
is `print("hello.panack".ends_with(".panack"))`, preserving the literal suffix.
Most current dot calls still resolve global functions; the proposal must settle
receiver-type lookup rather than merely shorten prefixed function names.

- [x] Define minimal implicit availability for Option/Result and their
      constructors, including canonical definitions, removal of obsolete imports and name
      collisions; decide helper exposure separately from core types
- [x] Inventory text and collection APIs and select concise, type-appropriate
      methods, beginning with `Str.ends_with`; cover literals, variables and
      chaining, global receiver-first calls, fields, generics, purity and useful
      wrong-receiver/unknown-method diagnostics
- [x] Apply the agreed breaking preview migration for existing helpers,
      imports and user names; coordinate with the namespace proposals below
      without assuming full namespaces must ship first
- [x] Deliver independently scoped slices with meaningful compiler and public-CLI
      tests, source/saved-bytecode parity, installed-package/bootstrap checks,
      browser integration and updated language/library examples and contracts

Value: less import ceremony and a more consistent API for developers. Estimated M for core availability and M–L for methods and migration,
including tests, docs and integration; one cohesive implementation PR, as selected by the user.
Resolve the bounded design decisions within the relevant PR rather than requiring
a separate design report up front. Confidence in the friction is high; lookup rules and
migration cost need assessment. Risks are name capture, duplicate definitions,
inference/effect regressions and differences between native and browser builds.
Delay prolongs learning friction rather than a known correctness defect.

Acceptance includes import-free construction and matching of both core types,
migration of existing callers, name-resolution failures, unchanged purity and
persistent collection semantics, suffix edge cases and real playground usage.
Implementation must pass `make check` and relevant browser checks. Keep one
coherent contract across the compiler, stdlib, packages and browser assets.
Full namespaces, classes, inheritance, dynamic dispatch and general user-defined
extension methods are outside this proposal. A minimal core prelude is a candidate
mechanism; implicitly importing the entire stdlib is not the proposed outcome.

### Historical scope and recommendation before PR #118

This comparison records the recommendation before PR #118 shipped. The public
playground had made basic API inconsistencies visible to new users. The rationale
was to reduce migration cost before more libraries and examples depended on the
earlier spellings; this was a reasoned expectation, not a measured adoption result.

| Candidate | Value and trade-off | Size / estimated PRs |
| --- | --- | --- |
| Core types and standard methods (#116) | Recommended Next: direct user feedback, immediate benefit in ordinary programs, and existing method machinery to build on | M–L overall / 1 |
| Dependency-aware probe reuse (#106), excluding separate compilation | Strongest alternative: slow validation affects every change, but savings from narrower invalidation remain unmeasured | M / 1–2 |
| Native TCP/timer feasibility | Enables network applications, but carries greater lifecycle uncertainty and does not resolve basic API friction | M–L investigation / 1–2 |

Choose caching first if measurements show iteration cost obstructs this work;
choose transport feasibility first if a concrete network application becomes the
immediate objective. Full namespaces and separate compilation remain broader
follow-ups, not prerequisites for this bounded initiative.

Delivery scope, combined in one implementation PR:

1. **Core availability (M component).** Provide `Option[T]`, `Result[T,E]`, `None`,
   `Some`, `Ok` and `Error` by default. Prefer one source-defined core loaded once
   through the existing loader; settle behaviour when the stdlib root is missing
   or overridden. Remove obsolete core imports from maintained sources. Conflicting user
   declarations should produce an explicit diagnostic rather than silent
   shadowing. Keep value-or helpers and host/testing modules outside the implicit
   public surface. Verify annotations, inference and pattern matching without
   imports, nested imports and collision failures. Preserve existing inference
   limits; this slice does not promise inference for an unconstrained `None()`.
2. **Text methods (M component).** Make `len`, `slice`, `starts_with`,
   `starts_with_at`, `ends_with`, `reverse`, `is_digit`, `is_letter`,
   `is_whitespace` and `parse_nat` available on `Str` without imports. Preserve
   existing Unicode/code-point, ASCII classification and parse-failure contracts.
   Use receiver-type lookup for standard method names; an unrelated global
   function must not capture a standard method call. Preserve current user
   receiver-first calls for names outside the standard method set. Define the
   reserved-name boundary and wrong-receiver behaviour explicitly, including
   names shared by supported types such as `len`. Reuse library algorithms and
   existing runtime primitives where possible; a new VM opcode is not assumed.
3. **Collection methods and migration (M component).** Provide import-free array
   `first()` and `sort_by(comparator)` alongside `len`, `append`, `concat`, `map`
   and `reduce`; retain Map `put`/`has`/`get` and Set `add`/`has`, and make their
   existing operations consistent with the same receiver lookup rules. Map/Set
   length is not currently supported and is not added in this change.
   Preserve stable sorting, pure callback requirements and immutable updates.
   Keep current missing-key behaviour; safe optional Map lookup is separate.
   Inventory Bytes explicitly and defer new byte-buffer method names to a
   follow-up rather than leaving its coverage ambiguous. Migrate public examples
   and playground lessons to the preferred spellings.

Migration decision, 2026-09-30: the user reports no external Panackelty authors,
so preserving obsolete imports and prefixed functions is not a delivery
requirement. Prefer a clean breaking preview migration. Update compiler and
stdlib sources, tests, examples, packages and playground lessons together in
each affected slice; remove superseded public wrappers and redundant import
modules once their remaining responsibilities have been accounted for. Do not
remove useful helper behaviour merely because its old module also defined a
now-implicit type. Record source/API changes in the changelog under the existing
preview release policy; this is not a compatibility-preserving patch release.

Retain an old spelling only where the reproducible bootstrap demonstrably needs
it, with the exact dependency, limited scope and removal condition documented.
Do not assume a historic compiler requires public aliases: prove the requirement
and prefer isolated bootstrap staging. Acceptance includes a reference audit for
obsolete calls/imports and a passing bootstrap after migration. Unrelated stdlib
imports and general user-defined receiver-first functions remain in scope only
where affected by the new lookup contract. New method names must not become new
unqualified global functions. Do not promise editor completion in
this item: discoverability means a consistent documented type API and relevant
diagnostics. General extension methods, new namespaces and Option/Result helper
methods remain separate scope.

Technical evidence: `loader.panack` already deduplicates resolved module paths;
`parser.panack` marks selected collection methods for checker resolution, but
ordinary dot calls currently lose their method identity. The implementation must
retain enough identity for the proposed lookup rules. The browser runtime
already supplies a stdlib root and runs the same compiler bytecode, so implement
this in the shared compiler/library path, without browser-only source rewriting.
An implicit prelude must not leak extra helper declarations simply because the
current option/result source files contain them alongside their enums.

The implementation PR needs unit and public-CLI positive/negative cases, canonical
`make check`, bootstrap/package evidence and rebuilt playground assets with
relevant browser tests. Compare compilation latency and asset size before and
after implicit loading; investigate material regressions rather than assuming
all-module loading is free. Final acceptance includes running import-free
Option/Result examples, the exact suffix example above and array first/sort
examples in the published playground after separately approved merge/deployment.
Implementation and live acceptance are complete. The release version remains unchanged during implementation; prepare the next
alpha (currently expected `0.1.0-alpha.10`) and breaking-change notes in a separate
release PR. The bytecode format stays v9.
