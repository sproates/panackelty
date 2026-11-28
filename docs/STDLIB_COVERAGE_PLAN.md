# Standard-library test coverage

28 November 2026. The owner selected stdlib coverage as the first alpha.17 task.
The owner authorised execution on 28 November. SCV1 is in progress;
[the inventory and findings](STDLIB_COVERAGE_INVENTORY.md) record current evidence. It is a
bounded improvement to existing coverage, separate from the broader coverage
architecture review. No separate issue has been created.

## What the current report tells us

The successful source collection [46160227297](https://github.com/sproates/panackelty/actions/runs/46160227297)
measured commit `72d7d1cd0607f78e598c988649a244cd8528f435`.
The manifest includes 18 of the 69 source modules under `src/stdlib`; 51 are
explicitly unmeasured. Declaration-only modules need classification, not invented
execution percentages.

For the measured subset, 87 of 345 executable source lines were observed,
120 were not hit by this corpus, and 138 were unavailable. That yields the
published 25.22–65.22% range. Functions are 41.44–65.77%; branches 13.25–46.18%.
These are not whole-stdlib coverage figures. Nor does a zero in this corpus
establish that no other test exercises the behaviour.

Of the 138 unavailable lines, 97 are in core.panack and 29 in testing_commands.panack.
Their causes must be established before choosing a remedy. The collector currently
runs 14 unit roots, 13 functional cases, three compiler executions and a scope
fixture. Dedicated ordinary/async stdlib, State, filesystem, console, process,
HTTP, digest and gzip tests exist outside this corpus.

## Scope and completion

Account for all 69 modules in the released alpha.16 stdlib, including their
public contracts, shared helpers and retained compatibility adapters. Measure
executable source where supported and associate native-backed operations with
the relevant VM tests. Source and native metrics remain separate. Pure type and
re-export declarations are not runtime test targets, although their public
import/type contracts still need evidence.

For each public operation, identify applicable success, boundary and error cases;
ordinary/async equivalence; and resource ownership or cancellation requirements.
Reuse existing assertions before adding tests. Every discovered gap must end with
an asserting test or a specific reviewed reason it is outside this task. Do not
silently defer difficult gaps and declare completion.

The owner clarified that this task does not target a percentage. Completion is
judged against the behavioural inventory, with branch gaps and unavailable
measurements visible. Expanding measured scope may initially lower the percentage.

## Delivery slices

Weights are provisional effort allocations, totalling 100. SCV1 is in progress, with an early SCV2 measurement correction under focused
validation. All slices still have zero accepted delivery credit. They are not time estimates.

| Slice | Weight | Work and exit evidence |
| --- | ---: | --- |
| SCV1: Establish the test and measurement baseline | 15 | A finite matrix of all 69 modules and public operations, mapped to existing asserting tests, missing cases, native backing and measurement status. Explain the 138 unavailable lines by cause. Distinguish unmeasured, unavailable, observed zero and declaration-only items. Identify collector limitations before promising their repair. |
| SCV2: Measure the tests already present | 30 | Register suitable existing tests in bounded stdlib collection batches, preserving assertions, child-process observations and source identity. Adapt temporary generated fixtures where required without creating duplicate test implementations. Demonstrate one ordinary, async, resource and State case end to end before migrating the remainder. Retain explicit unsupported cases. Provide a focused stdlib collection path that does not execute compiler self-compilation, with complete-batch integrity checks and safe report aggregation. |
| SCV3: Fill the behavioural gaps | 40 | Add tests from the SCV1 matrix, updating it as each gap closes. Batch pure/data/error-rendering cases first, then host resources and async/State lifecycle cases. Verify outputs and effects, failure details, bounds, cleanup and ordinary/async parity where applicable. Use deterministic peers, temporary resources and existing fault injection. Test cancellation, queued State operations and resource reuse where those are contractual risks. Add native checks for native behaviour rather than claiming Panack wrapper coverage proves it. |
| SCV4: Verify and publish the result | 15 | Publish per-module before/after coverage with denominator changes separated from added execution. Explain every remaining unavailable or unmeasured executable area. Meet the accepted contract matrix, pass applicable final regression checks, run the expanded collection on GitHub and verify public report provenance. Update the manifest, coverage documentation, release summary and completion record. |

SCV1 precedes SCV2; SCV2 establishes useful measurements before SCV3 claims gains.
SCV3 is delivered in coherent batches, not one PR per function. Expect roughly
four to six PRs for the whole task, depending on whether resource tests need a
separate integration batch. This is a substantial test/measurement task, not a
quick percentage adjustment. SCV1 must refine the effort estimate with evidence.

## Test priorities

- Data and pure helpers: empty input, boundaries, invalid UTF-8, validation and
  error rendering, path and byte operations, known digest/compression vectors.
- Filesystem and process: missing paths, permissions where reliably reproducible,
  bounded input/output, invalid arguments, nonzero exit, timeout, confinement and
  failed publication. Assert the failure's meaning and retained filesystem state.
- Console, time and environment: EOF, malformed input, failed output, missing
  values and duration boundaries. Avoid wall-clock timing assertions that depend
  on runner speed; use established deterministic hooks where available.
- Networking and streamed resources: independent loopback peers, partial data,
  limits, failures, cancellation and cleanup. No dependence on public services.
- State and async operations: retained values, update semantics, contention,
  cancellation and subsequent progress. Keep compile-time rejection tests alongside
  runtime tests; rejected code is not expected to produce runtime coverage.

These are risk categories, not a claim that the listed behaviours lack tests.
The inventory determines what actually needs adding.

## Validation and boundaries

During development, run the affected asserting tests and focused stdlib collection.
Preserve source and saved-bytecode checks where applicable. Batch related edits
before the final required validation; do not run the entire compiler coverage
corpus for each test addition. Changes to shared harness logic still require its
applicable regression and final validation checks. Record collection time and
compare like-for-like scope; do not set a new arbitrary performance target.

The expected work is tests, fixture integration, reporting and documentation.
Do not assume a compiler rebuild, bytecode change or general coverage redesign is
necessary. If missing measurements require substantial compiler/VM work, show the
specific blocker and smallest viable remedy before expanding scope. Likewise,
a newly discovered product defect needs its own regression evidence and a scoped
fix decision; it must not quietly turn this task into a language redesign.

Cached preparation may be reused only under the existing identity checks. Test
execution and counters must be fresh. Never turn unavailable values into zero,
remove difficult modules to improve the percentage, or weaken assertions.
The scope inventory must detect new stdlib modules so future additions cannot
silently escape classification.

There is no website article or product-page change in this planning step. SCV4
updates the existing coverage site; any wider editorial work is separate.
