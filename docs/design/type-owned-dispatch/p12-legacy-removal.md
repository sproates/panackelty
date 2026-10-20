# P12: Remove legacy receiver dispatch

GI#525 P12 follows [P11](p11-remaining-callers.md). Completion takes effect on
integration after canonical validation and exact-head CI. Receiver calls remain
`->` calls; the owned-dot bridge and final syntax migration are P13/P14.

## Removed boundary

The parser no longer rewrites receiver dots to globals. Module resolution rejects
value-dot calls with an arrow migration diagnostic while preserving namespace
calls and record fields. A same-named global cannot rescue an absent owned method.
The ValueMethodCall fallback and special get/put/add/has dispatcher are removed.
Map and Set select their own typed operations. Callable/map/reduce backend
operations retain private intrinsic identities without source-name dispatch.

All 32 compatibility exports in the [P10 register](p10-stdlib-callers.md)
are removed, including the wrapper-only text, numeric, option and result modules.
Former primitive globals are unavailable to applications. Constructors, print,
read_line and write_file retain their explicitly classified ordinary surfaces.
Host and transport operations are reached through explicit service namespaces.
Compiler string-path/file plumbing uses compiler_paths/compiler_files services;
TCP, HTTP transport and deterministic async testing have explicit services too.
Internal primitive grants require the toolchain package, exact module and exact
declaration identity. An application file with a matching basename gains nothing.

Project acceptance and emission now use checked declaration identities throughout.
Removing the old parallel checker exposed checks that must belong here: Bool and
pure guarded declarations, duplicate record fields, preservation of child
argument errors, and mutation-sensitive Nat proof invalidation. Writes invalidate
only the affected lexical local; argument order, condition writes and loop writes
cannot reuse stale bounds. A fresh guard can establish a new valid bound.
Subtraction explanation evidence comes from the same proof decision as acceptance.
Valid sibling effect explanations survive an invalid function body; invalid
declarations still gate global effect evidence.

## Coverage and compatibility

The [P11 test register](p11-legacy-test-boundary.md) is retired as an acceptance
boundary. Parser tests preserve raw dot syntax for resolution. Low-level backend
fixtures explicitly exercise internal operations through a test-only adapter;
the adapter does not restore receiver sugar. Public rejection coverage checks
removed globals/wrappers, fallback attempts, namespace collisions and spoofed
primitive grants. Successful owned calls execute from source and saved bytecode.
Existing functional diagnostic goldens reflect identity-based source locations.
No VM opcode or bytecode version changes. The VM retains its four `$method_*`
bytecode service names solely for existing saved artifacts; neither the source
resolver nor emitter can select them. Native registry expectations remain intact.
Logical-import and stdlib artifact goldens remove obsolete wrappers and unused
callees. Disassembly comparison confirms every surviving stdlib function body
(including main) is unchanged after imported-module renumbering; old/new outputs
match. Installed-file and coverage inventories replace the four removed modules
with the four explicit service modules. No surviving coverage input is excluded.

## Validation

Seed regeneration verifies identical compiler artifacts across stages 2/3/4:
`f48268ec4d845dac4f8094c3b14a0c093c6a22a3c6caf7464339169df7c0910d`.
All three stdlib stages reproduce
`4f2904808fe9b6be4633eb54a4022d4216ab4eab2dd5b196677f9155ebaaaba3`
and exact conformance output. Focused binding, public CLI explanation and rational
trap checks pass (1,257, 218 and 27 assertions respectively).
All 133 public integration, 26 resolver, 48 source-map and 95 host-runtime
assertions pass. Retained read_line/write_file APIs have complete identity
contracts, with terminal/file round-trip and rejection coverage.
Native artifact contracts, all 13 focused coverage manifest/policy tests,
68 source-coverage experiment assertions and 65 provenance assertions pass.
Final canonical `make check` passes in 510 seconds: unit 365 seconds, bootstrap
208 seconds concurrently, functional 101 seconds, release smoke 8 seconds and
quick start 1 second. This includes all 1,257 binding and 532 functional runner
assertions, the 70-root source audit and source/saved-bytecode network fixtures.
The complete diff was self-reviewed. Exact-head CI is required before integration.

## Impact and handoff

This is a compiler/source compatibility change for the unreleased alpha.14 line.
Previously compiled bytecode retains its VM contract. The website's released
alpha.13 examples need no intermediate-arrow rewrite; alpha.14 publication must
use the final P14 syntax and P15 consumer acceptance. Existing GI#525 website
follow-up remains responsible for that cutover.

The final check took 510 seconds versus P11's reported 535 seconds; these are
local validation runs with differing cache state, not controlled benchmarks.
The change removes a parallel acceptance traversal but adds identity guard
validation and mutation scans. No runtime or compiler speedup is claimed.
Historical timing warnings remain informational under the existing policy;
no timeout or coverage threshold was weakened. P13 begins only after this packet's acceptance.

## Programme status on acceptance

The unchanged stage weights yield 90%; GI#525 remains open.

| Stage / packets | Status | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | Done | 20% | 100% | 20 |
| Built-ins: P06, P08 | Done | 20% | 100% | 20 |
| Stdlib: P10 | Done | 15% | 100% | 15 |
| Compiler/source: P09, P11 | Done | 20% | 100% | 20 |
| Legacy removal: P12 | Done on integration | 10% | 100% | 10 |
| Dot cutover: P13–P14 | Unstarted | 5% | 0% | 0 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| **Total** | | **100%** | | **90%** |
