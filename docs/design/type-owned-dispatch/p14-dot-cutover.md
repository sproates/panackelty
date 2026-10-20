# P14: Dot-only method calls

GI#525 P14 follows [P13](p13-owned-dot-bridge.md) at
`cc67a1a70b9bb9a5d03889d5b1e4196eeb66d6cd`. Completion takes effect on integration
following canonical validation and exact-head CI. P15 remains separate.

## Cutover

Maintained compiler, library, package, tool, example and test receiver calls use
owned dots. Generated Panackelty source fragments are migrated with their callers.
The parser rejects `receiver->method(...)` with an explicit instruction to use
dots. The lexer recognizes that obsolete token only to report the rejection.
`OwnedCallExpr` remains an internal normalization node after dot classification;
it has no independently accepted source spelling. There is one owned-method
binder, with unchanged ownership, visibility, inference and effect checks.

Namespace calls and fields keep their meanings. Missing methods never fall back
to globals, and callable fields require explicit invocation. The temporary P13
fixture converter is removed; maintained fixtures now exercise dots directly.
Diagnostic expectations retain the same errors and update method spellings and
caret positions for the shorter separator. Historical packet reports, planning
inventories and non-code dependency/conversion arrows retain their meanings.

No VM operation or bytecode-version change is required. Saved bytecode keeps its
existing runtime contract. No release, browser promotion, website deployment or
HTTP framework implementation is included.

## Validation

Focused validation passes: 210 parser, 36 source-span, 1,260 binding and
333 public failure-fixture assertions; dot source/saved-bytecode acceptance and
arrow rejection; all 30 coverage-policy checks. Final canonical `make check`
passes in 459 seconds, including all 532 functional runner assertions, the
70-root source audit, network fixtures, bootstrap, package relocation and release
smoke. Unit validation took 307 seconds, bootstrap 213 seconds concurrently,
release smoke 8 seconds and quick start 1 second. The accepted P13 input seed is
`8853d78f508e23b6ce16f5f88fcf013c653bea4e61280f5e1d33f59400de9023`.
The first dot-only compiler refresh reproduces all stages as
`53e54b0fa15187352beb50918c92a904177f5b042a1c0e3cfc17fa2cb9b26ec9`;
standard-library bytecode remains
`4f2904808fe9b6be4633eb54a4022d4216ab4eab2dd5b196677f9155ebaaaba3`.
The final refresh includes dot spelling in ownership diagnostics and reproduces
stages 2/3/4 as
`d9ebb4911470894ffb9cca33f14ab07f8f855f981da4f36afcaf9376179b7fa5`,
with the same standard-library bytecode and exact expected output.

Diff review compares source against mechanical receiver-separator replacement:
179 Panackelty files have no other change. Nine additional files contain the
reviewed parser rejection, diagnostic spellings or test expectations. Six stale
binding expectations were corrected after the first focused run: the raw-dot
frontend rejection, four qualified-syntax gate counts and one type-token column.
Public failure-fixture checks confirm migrated diagnostic text and carets.
The first canonical run then identified an integration pattern column and two
CLI explanation spans still reflecting the longer separator. Direct reproduction
confirmed unchanged errors at columns shifted left by one. Corrected checks retain
all diagnostic and source-location assertions; all 218 explanation assertions pass.
The coverage and provenance experiments pass 68 and 65 assertions respectively
with migrated generated source. The exact diff was self-reviewed; applicable
exact-head CI must pass before integration.

## Impact and handoff

Calls retain the same checked targets and runtime operations. Compiler source
uses the P13 dot classification path and no longer parses receiver arrows.
The final check took 459 seconds versus P13's reported 556 seconds; the rerun
reused compilation caches from focused checks and the earlier failed run. These
are different cache/load states, not evidence of a compiler speedup. Existing GI#106 latency
reassessment remains open; historical timing thresholds are informational.
No timeout, assertion or coverage threshold is weakened.

The website follow-up register now records final dot syntax, awaiting P15's
matching consumer acceptance before alpha.14 promotion. Released alpha.13
examples retain their stated version contract. P15 must verify packaged/browser
consumers and perform the final deletion audit before GI#525 closes.

## Programme status on acceptance

The unchanged weights yield 95%. P14 completes the dot-cutover stage; P15 retains
the final five points and GI#525 stays open.

| Stage / packets | Status | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | Done | 20% | 100% | 20 |
| Built-ins: P06, P08 | Done | 20% | 100% | 20 |
| Stdlib: P10 | Done | 15% | 100% | 15 |
| Compiler/source: P09, P11 | Done | 20% | 100% | 20 |
| Legacy removal: P12 | Done | 10% | 100% | 10 |
| Dot cutover: P13–P14 | Done on integration | 5% | 100% | 5 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| **Total** | | **100%** | | **95%** |
