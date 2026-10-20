# P11: Compiler and remaining owned callers

GI#525 P11 follows [P10](p10-stdlib-callers.md) and uses P09's accepted
arrow-capable seed. Completion takes effect on integration after canonical
validation and exact-head CI.

## Delivered boundary

Compiler and bytecode implementation callers now use owned receiver operations.
The migration covers collections, text, bytes, paths, time and Option/Result
helpers while preserving ordinary functions, constructors and host services.
Packages, tools, examples, test harnesses, source-coverage helpers and maintained
functional programs use the same boundary. Executable shell fixtures, generated
host/coverage/package programs and active language-tour examples are updated.
Unused private namespace imports are removed; re-exports and explicit import-fixture
coverage remain intact. The TCP empty-request generator selects its initializer
directly instead of relying on a replacement tied to the old source spelling.

The compiler's syntax/resolution/checker/emitter implementations are retained.
This packet changes their own callers, not the compatibility language they still
implement. Intrinsic names in compiler tables, VM instructions and diagnostics
are implementation data and are not renamed into source method calls.

Arbitrary free-function receiver sugar in positive examples is replaced with
explicit calls, including generic identity and local functions deliberately
sharing an owned method's name. Empty generic receivers and Result values use
typed bindings. Existing named callbacks retain their signatures and effects;
the P01 reference audit identifies no disappearing receiver-function references
requiring a new adapter in this corpus. Bound-method references are not added.

## Retained test boundary and P12 handoff

P11 does not remove coverage of still-supported legacy semantics. The exact
[legacy test boundary](p11-legacy-test-boundary.md) lists 55 standalone inputs
with 95 conservative receiver candidates, plus quoted compiler input/expected-text
hosts. Those tests exercise the old parser/checker/lowering and exact diagnostics.
Their host programs are migrated; P12 must replace their assertions with owned
semantics or old-syntax rejection as it deletes the legacy implementation.
The register distinguishes these intentional inputs from maintained application
callers and from primitive implementation names.

The 32 compatibility source wrappers remain exactly as listed in P10. P12 owns
wrapper/global-alias removal, receiver-to-global lowering deletion, special
get/put dispatch removal and rejection of old receiver dots. Namespace dots stay
valid. P13/P14's owned-dot bridge and source cutover are separate work.

## Validation

The migrated compiler passes source checking. Seed regeneration builds stages
2/3/4 with matching compiler SHA-256:
`72dd4ea10fb83fb1de19ee18bbd6d2e59de41fa2c21a4b45ff57b4c9bc436fb8`.
The input was the saved P09/P10 seed
`c4f6b8b2b3c567a0400353eff7ba4f9f5373caa22027d6f3b4f42bb1f8f1bdfd`.
All stages reproduce unchanged stdlib bytecode
`27af27f328cd4b4bb55c4f190ceff1cf55cfd522a0c54093ecb12df2953c41f8`
and exact output. Native oracle artifact contracts pass without golden changes.
Instruction/source mapping passes all 34 assertions, including imported generic
origins. The CLI package test passes all 57 assertions with the refreshed compiler.
The runtime probe target passes, including 140 bytecode assertions. Exact
pattern diagnostics retain their original line/column contracts.


Final canonical `make check` passes in 535 seconds: unit 384 seconds (including
all 1,249 binding assertions), bootstrap 268 seconds running concurrently,
functional 107 seconds (all 532 assertions), release smoke 8 seconds and quick
start 1 second. This run uses warm build/probe caches after focused validation;
it is not a controlled clean-build performance comparison. Review
covers source argument order, receiver evaluation, imported namespace boundaries,
generic owner evidence, retained diagnostics and the exact regenerated seed.
Self-review confirms unchanged function signatures and literal values in all 35
changed production compiler/bytecode/package/tool files; only caller syntax and
unused namespace imports change there. No timeout, coverage gate or assertion
count is reduced.

## Impact and documentation

VM instructions, bytecode format, algorithms, numeric semantics and host behavior
are unchanged. Migrated callers bypass forwarding functions; the self-hosted
compiler artifact changes from 938,926 to 933,568 bytes accordingly. Full-suite timing is comparable validation
evidence, not a claim of runtime speed improvement. Historical timing budgets
remain informational under GI#106.

The roadmap, self-hosting notes, compiler README, coverage documentation and
language tour describe the migration boundary. SPEC retains legacy behavior until
P12 removes it and records the owned-caller checkpoint. Architecture and root
README remain accurate: execution and stable-release setup are unchanged.
Website impact: no new currently released claim; the alpha.14 methods article
already describes the ongoing migration. No website publication is needed here.

## Programme status on acceptance

Weights remain unchanged. P11 completes the compiler/source stage, adding its
remaining 18 programme points: estimated completion rises from 62% to 80%.
P12 is next, and GI#525 remains open.

| Stage / packets | Status | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | Done | 20% | 100% | 20 |
| Built-ins: P06, P08 | Done | 20% | 100% | 20 |
| Stdlib: P10 | Done | 15% | 100% | 15 |
| Compiler/source: P09, P11 | Done | 20% | 100% | 20 |
| Legacy removal: P12 | Next; unstarted | 10% | 0% | 0 |
| Dot cutover: P13–P14 | Unstarted | 5% | 0% | 0 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| Total | | 100% | | 80% |
