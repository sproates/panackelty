# Namespace migration retrospective

This document records how the P2 namespace programme was approached, what
changed as implementation evidence arrived, and what the work taught us. It is
a retrospective on the compiler and standard-library migration, not a new
language specification or a claim that every future namespace or package
feature is complete. The detailed acceptance record and measurements remain in
[`tests/VALIDATION_PROFILE.md`](../tests/VALIDATION_PROFILE.md); the agreed
scope is in [`ROADMAP.md`](../ROADMAP.md#language-namespaces--idea).

## Programme shape

The work was split so identity-aware checking and execution could be established
before changing the compiler's own source graph:

1. **Ordinary expressions and control flow** established identity-based checks
   for expressions, bindings, branches, loops and returns.
2. **Aggregates, patterns and inference** extended those contracts to records,
   enum variants, collections, patterns and inferred generic arguments.
3. **Core and indirect-call contracts** aligned direct, receiver, core and
   indirect calls, including reserved core identities.
4. **Guard proofs** tied conversion and arithmetic evidence to local binder
   identities and invalidated it on reassignment.
5. **Effects and await** carried callable purity and async contracts through
   checked calls.
6. **Identity emission and standalone loading** connected checked identities
   to emitted programs, source loading and saved bytecode.
7. **Diagnostics, explain and locate** preserved useful source spans and made
   selectors safe across module-qualified runtime identities.
8. **Coordinated seed/source cutover** migrated the compiler, standard library,
   tests and examples together, refreshed the bootstrap seed, removed the flat
   compatibility route, and repeated native, browser and installed acceptance.

This sequence deliberately separated foundations from executable namespace
acceptance. Finishing an earlier slice did not imply that the compiler itself
could yet be built from namespace-only source.

## Approach and how it changed

The early slices built a checked graph incrementally and kept unsupported or
deferred cases explicit. That made each semantic boundary testable before
identity-based bytecode emission and loading depended on it. Slices 6 and 7
then established the execution and diagnostic contracts that the source
migration needed.

Slice 8 began from a useful but incomplete checkpoint: standalone projects
could compile and run through namespaces, saved v9 bytecode still worked after
source removal, and fresh compiler and standard-library stages reached matching
outputs. The compiler's own source still relied on a legacy flat combined-loader
path. A namespace-only compile exposed missing explicit bindings, enum-variant
imports and public re-export routes. An initial migration attempt was discarded
without changing that checkpoint.

The next approach first migrated compiler APIs in dependency order, then
coordinated their consumers and the standard-library prelude. The chain grew
from types, lexer, parser and resolver through checking, signatures, purity,
body/emission, codecs, project loading, source maps, explanations, coverage and
the driver. Public exports and selected imports were made explicit at each
boundary. Once the seed could handle the same identity-checked graph, the flat
project route could be removed and the complete root inventory audited.

Cutover trials were treated as probes, not acceptance. When forcing identity
emission exposed legacy-import or coverage-session failures, the guarded route
was restored while the missing checked metadata and fixture assumptions were
addressed. The final audit covered all 59 functional, compiler-contract and
example roots and became part of canonical `make check`. Seed regeneration then
verified fresh compiler stages 2/3/4 and the standard-library artifact; tests
also exercised saved-v9 execution with source files removed.

## Difficulties and responses

- **The bootstrap seed lagged the current checker.** The old seed could compile
  a function alone but deferred its identity check when it appeared in an
  imported module graph. Byte-core operations in codec and decoder bodies made
  this visible during stage-2 compiler construction. We did not weaken identity
  checks or mark the functions accepted; the seed/checker handoff and imported
  graph were migrated together.
- **Implicit prelude bindings collided with the new public module graph.**
  Publishing `stdlib/bytes` collided with an implicit binding in the old broad
  prelude. Aliasing or selectively re-exporting around the collision deferred
  identity checks, so that workaround was rejected. The prelude and its
  consumers were migrated as a coordinated unit.
- **The flat route hid missing checked metadata.** A forced-identity trial
  initially failed driver and source-map cases using quoted legacy imports, and
  later still failed coverage-session cases for generated trap and loop
  fixtures. The compatibility route stayed guarded until the imported closure
  and all audited roots passed the identity route.
- **Several fixtures encoded old identity assumptions.** Runtime observers
  report module-qualified function identities, while `locate` selects by
  source-level names. Some source-map and coverage fixtures also relied on
  quoted imports or transitive prelude imports. Fixtures were updated to use
  explicit selected APIs and to assert the correct identity form; oracle
  changes were checked against old bytecode output and disassembly.
- **Environment and hosted limits obscured product results.** Native TCP tests
  could not bind loopback in the default sandbox, so complete local acceptance
  required host networking. Hosted runner smoke then exceeded its 90-second
  limit, and a 120-second retry still failed on source and bytecode runs. The
  nested smoke bound was raised to 180 seconds only after confirming the same
  355 assertions and report remained required. Project check and bootstrap
  budgets were not raised. The local machine lacked a WASI SDK for a fresh
  browser build, so browser validation used the existing unchanged WASM
  artifact locally and the hosted browser workflow for its configured build.

## Outcome and open performance record

The Slice 8 candidate now uses selected namespace imports across compiler and
standard-library APIs, has no project-compilation flat compatibility route,
and passes the maintained 59-root identity audit. Fresh seed fixed points,
source-free saved-v9 execution, installed-package smoke, native acceptance and
browser tests passed. The browser host adapter added only the required
`fs_metadata` behavior over its preopened virtual filesystem; it does not imply
that other host capabilities are available. The exact candidate CI matrix also
passed after the runner-bound and fixture corrections. These results establish
the recorded candidate acceptance; release and merge status are tracked
separately.

Correctness acceptance did not settle the performance concern. A matched fresh
runner-smoke profile on one macOS arm64 host measured means of 30.005 seconds on
base `c2cf35d` and 55.515 seconds on Slice 8 commit `2dad547`, an 85% increase.
This is a material end-to-end smoke difference on that host, not an isolated
compiler measurement or evidence of cross-platform causation. The latest clean
host-enabled canonical `make check` passed all suites in 574 seconds, including
355 fixture assertions and the 59-root audit, while exceeding the unchanged
120-second full-check and 60-second bootstrap targets. GI#106 remains open for
the performance follow-up; its recorded next step is to isolate compiler-source
and module-loading cost and assess a bounded mitigation, before the next
compiler-heavy delivery or sooner if hosted timing breaches recur. Neither the
targets nor coverage were relaxed.

## Lessons for future compiler migrations

- Treat a bootstrap compiler as a separate implementation of the language
  contracts. A passing source-level checker fixture does not prove the seed can
  check the same imported graph.
- Make exports, imports, visibility and re-exports explicit early. Broad
  preludes and transitive imports conceal dependencies until the graph changes.
- Preserve the distinction between source names and emitted module-qualified
  identities. Diagnostics and runtime tooling can use different identity
  domains and need direct tests for each.
- Keep the old path only as a measured migration boundary. Force the new path
  in probes, inventory every failing root, and remove the fallback only when
  the complete required closure passes; do not turn failures into broad proof
  rules or silently deferred acceptance.
- Verify bootstrap fixed points, saved-bytecode behavior, installed use, and
  cross-platform behavior as separate properties. One does not stand in for
  another.
- Make environmental constraints visible in the record. Request the access
  needed for loopback tests and distinguish unavailable local toolchains from
  product failures.
- Keep performance measurements matched and bounded, retain noisy or
  incomplete observations with their caveats, and keep a material regression
  ticketed even when correctness CI is green.
- Add exhaustive audits to the canonical check once they are stable. This
  turned the one-time 59-root cutover probe into ongoing regression protection.

The main practical improvement for the next large migration is to establish
seed parity and an exhaustive closure audit before broad source edits. That
should reveal bootstrap-specific incompatibilities earlier and make each
subsequent migration checkpoint more trustworthy.
