# P13: Owned-dot bridge and seed

GI#525 P13 follows [P12](p12-legacy-removal.md), starting from
`a239bc22e935c73ebb24e9886432ef366cf5117e`. Completion takes effect on integration
after canonical validation and exact-head CI. P14 remains a separate packet.

## Behavior

Both `receiver.method(...)` and `receiver->method(...)` enter the same owned-call
binder. Dots first distinguish namespaces/enum qualification from lexical values;
a value selects a method by its checked owner identity. No ordinary-function
fallback or name-based collection dispatch is restored. Receivers and arguments
are bound and evaluated once, in the same order for both spellings. Explicit
method type arguments, visibility, inference, purity and async contracts retain
the existing checks and emission identities.

Lexical values shadow namespace aliases, including selectively imported namespace
re-exports. Selected declarations and generic type binders retain import collision
checks. A failed namespace lookup cannot select a value method; a missing method
cannot select a global or namespace function. Field access stays separate:
`holder.call(...)` selects an owned method, while `(holder.call).call(...)` invokes
a callable field. Mixed arrow/dot chains are accepted by this bridge.

Compiler and library implementation sources remain in arrow syntax. The saved
bridge compiler must be verified before P14 converts those sources and removes
arrow support. The runtime, bytecode version and VM compatibility aliases are
unchanged. No HTTP networking implementation or release publication is included.

## Acceptance evidence

The focused binding suite passes all 1,265 assertions; the public bridge and
legacy-removal scripts pass. All 30 source-coverage reader/manifest/policy/release
gate tests and documentation links pass. Final canonical `make check` passes in
556 seconds, including all 532 functional runner assertions, the 70-root source
audit, network fixtures, bootstrap, package relocation and release smoke. Unit
validation took 401 seconds, bootstrap 216 seconds concurrently, release smoke
8 seconds and quick start 1 second. The full diff was self-reviewed; exact-head
CI is required before integration.
The public bridge test uses temporary dot-converted
copies of 12 existing owned-dispatch acceptance fixtures, retaining their exact
expected outputs. It runs source and saved bytecode, including generic/imported
owners, Server/Map coexistence, callable and async contracts, core methods and
receiver evaluation order. Dedicated programs cover namespace shadowing,
namespace re-exports, mixed chains, callable-field disambiguation, privacy,
guard scope, retained import collisions and missing-method/global rejection.
Binding tests additionally compare accepted and rejected generic/callable cases
under both spellings. Existing arrow fixtures remain in canonical validation.

The starting P12 seed is
`f48268ec4d845dac4f8094c3b14a0c093c6a22a3c6caf7464339169df7c0910d`.
Audited intermediate refreshes produced `1e9cf0bb3317babe1a76138c0be58479b67539677ef1442900998bb5f041c80a`
and `4f27153bb29d468c689097a0c225d0f7e1a079729fbd25ae2b9d7421dd425cb2`
while namespace scope rules were completed. The final source rebuilds identical
stages 2/3/4 with SHA-256
`8853d78f508e23b6ce16f5f88fcf013c653bea4e61280f5e1d33f59400de9023`.
Each refresh preserves the standard-library fixed point
`4f2904808fe9b6be4633eb54a4022d4216ab4eab2dd5b196677f9155ebaaaba3`
and exact expected output. Seed bytes and digest are reviewed together.

## Impact and next step

Runtime operations remain unchanged; dots select the same checked call targets
as arrows. Compiler work adds namespace/value classification for dot calls and
resolves import identity when checking lexical collisions. No runtime or compiler
speedup is claimed. The final check took 556 seconds versus P12's reported 510 seconds. This run
rebuilt native test tools and reused focused-probe caches; the two runs have
different cache/load states and do not establish a compiler regression. Existing
GI#106 validation-latency reassessment remains open, with historical budgets
informational and no timeout or coverage gate weakened.

Website: the released alpha.13 examples retain their versioned contract. The
existing GI#525 website follow-up remains pending final P14 syntax and P15
consumer acceptance before alpha.14 promotion; P13 does not publish an
intermediate syntax change to the website.

## Programme status on acceptance

Estimated completion is 92% under unchanged stage weights. The bridge and audited
seed earn 40% of the five-point dot-cutover stage (two points); the broader source
conversion, arrow removal and acceptance remain P14. This is scope-based partial
credit, not a count of completed packets. GI#525 remains open; P14 is next.

| Stage / packets | Status | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | Done | 20% | 100% | 20 |
| Built-ins: P06, P08 | Done | 20% | 100% | 20 |
| Stdlib: P10 | Done | 15% | 100% | 15 |
| Compiler/source: P09, P11 | Done | 20% | 100% | 20 |
| Legacy removal: P12 | Done | 10% | 100% | 10 |
| Dot cutover: P13–P14 | P13 done on integration; P14 unstarted | 5% | 40% | 2 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| **Total** | | **100%** | | **92%** |
