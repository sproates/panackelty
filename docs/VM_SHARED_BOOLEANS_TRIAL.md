# VM shared Boolean trial

7 November 2026. Local trial on `feature/rm162-real-program-fixes`, against
`5b79fc8`, with the preceding seven VM optimisations adopted.

## Representation and ownership

Boolean construction returns one of two static immutable Values. Their refs field
uses SIZE_MAX as an immortal marker: retain and release neither modify nor free
these objects. Callers still balance logical ownership through retain/release.
Boolean construction cannot fail and needs no dynamic allocation or initialisation.
Heap-owned Booleans created with value_new retain normal reference counting.

The two static Values cost 128 bytes on this arm64 Value layout. There is no
per-value layout expansion, bytecode change or language-visible identity change.
The trade-off is one immortal-marker check in retain/release for all Values.
Native clients must not mutate the shared Booleans. Existing VM operations read
Boolean payloads; mutable resources and heap ownership remain unchanged. This
adds no general thread-safety guarantee for other Values.

The fresh baseline profile still showed allocation work beneath value_bool.
The comparison below tests the net effect, including the extra ownership checks.

## Validation

- Strict-warning candidate VM and native test builds passed.
- Full native suite passed. New checks exercise repeated shared construction,
  container retention/release, equality with a heap-owned Boolean, and normal
  heap reference counts. Async checks cover waiting, completion and cancellation
  after the caller drops its ownership of a shared Boolean.
- Existing tests that assert exact Boolean reference counts use explicit
  heap-owned Boolean inputs, preserving their original lifetime assertions.
- Fault injection explicitly confirms shared Boolean construction/retain/release
  attempt no allocations even when the first allocation is set to fail.
- Native fault suite passed 2,056 injected allocation failures, down from 2,133
  because Boolean allocations have been removed. No failure assertions were
  removed or disabled.
- Five compiled/saved-bytecode fixtures produced exact expected output:
  collections, owned_intrinsics, callables, records_and_enums and async_await.

## Measurements

Alternating baseline/candidate order; five stdlib pairs and three compiler pairs.
Identical saved compiler and source inputs, including startup/decode/verification
and output. No builds or tests ran concurrently with the timing comparison.

| Workload | Baseline median | Candidate median | Less time |
| --- | ---: | ---: | ---: |
| Stdlib imports | 0.275114 s | 0.245465 s | 10.78% |
| Compiler | 20.457201 s | 18.035861 s | 11.84% |

Compiler pairs (baseline / candidate, seconds):
20.336210 / 17.619902; 20.457201 / 18.186072; 21.559995 / 18.035861.
All three favour the candidate. These small local samples do not establish a
portable percentage, nor a speedup for the complete validation suite. Workloads
that seldom construct Booleans could benefit less or pay the marker-check cost.

Every compiled output hash matched:

- Stdlib: `868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686`.
- Compiler: `9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.

## Status

Adopted in the normal checkout VM after owner approval. `make native` passed,
followed by full local `make check` (exit 0, 619 seconds): unit, functional,
bootstrap, packaging, release smoke and quick-start checks passed. Bootstrap
completed in 146 seconds and the compiler bytecode hash remained unchanged.
The total was 43 seconds below the preceding 662-second run; this is an observed
run-to-run comparison, not a controlled whole-suite benchmark. Historical timing
budget warnings remain advisory.

No push, PR or CI run. The stable globally installed release is unchanged.

Value ownership documentation and release/coverage records are updated. No
website or language/CLI documentation changes are required: observable program
semantics and bytecode are unchanged.
