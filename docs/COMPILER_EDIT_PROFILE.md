# Edited incremental compilation profile

Assessment: 8 November 2026, core `3e722af640d2e78f0a050bcfab28b1fe1e948672`.
This follows the successful [hosted cache experiment](COMPILER_HOSTED_CACHE_TRIAL.md)
and the owner's request to adopt reuse in a real CI workload where it adds value.

## CI adoption assessment

The current `Check` workflow routes pushes to `next` around compiler preparation
and the full platform matrix. Its Package jobs are compatibility/result gates;
their success does not mean those jobs rebuilt the packages. The bounded next
smoke runs on PR events, which the alpha.16 development workflow currently
suppresses. There is no expensive compiler preparation in that integration path
for another cache to accelerate.

Full/nightly compiler jobs do compile. `tests/run_probe.sh` already retains
complete artifacts keyed by source/dependency/tool identity, including stage-2
compiler and shared compiler-test preparation. This avoids recompilation within
a reused build tree. `tests/without_interpreter.sh` calls `make clean`, and hosted
jobs start fresh, so this does not establish cache reuse between nightly runs.
`make package-tool` also has ordinary file prerequisites; bootstrap proof paths
must remain fresh.

Cross-run nightly reuse remains a viable separate change. It should compare
retaining complete probe artifacts with retaining compiler module bodies, include
producer trust and transfer costs, and account for the default-branch scheduler.
No existing cache is removed and no claim is made that cross-run caching has no
benefit. Automatic CI cache adoption was not selected in this slice: it would
not improve the next integration path currently in use. The agreed fallback was
to profile edited incremental compilation before adding cache machinery.

## Method

A separate diagnostic VM reused `tests/compiler_watchdog/observer.h`, selecting
coarse loading, checking, cache encode/decode and emission functions. Only its
`execute.c` object was rebuilt and linked with the existing native objects.
The packaged VM, compiler sources and bootstrap artifact were unchanged.

The workload is the same 49-module self-compilation as the declaration reuse
trial, with a real edit changing `module_magic` from `PANACK-MODULE-1` to
`PANACK-MODULE-2` in a copied source tree. Normal and diagnostic VMs each used
their own empty cache and actual VM identity. Each ran cold, unchanged, edited
and fresh compilation in that order, without concurrent builds.

Phase measurements use completed enter/return pairs. Nested selected phase time
is subtracted before summing categories, avoiding double-counting. This is one
sample per mode, for locating costs rather than establishing a speedup. The
observer adds overhead; its phase durations must not be presented as exact
normal-VM costs. Output bytecode matched across VMs and within each fresh/cache
comparison. The edited output deliberately differs from the unedited output.

| Mode | Normal VM | Diagnostic VM |
| --- | ---: | ---: |
| Cold incremental | 20.365s | 21.276s |
| Unchanged incremental | 2.431s | 2.516s |
| Edited incremental | 13.795s | 14.287s |
| Fresh edited compilation | 15.777s | 16.312s |

## Where edited compilation spends time

| Phase | Diagnostic time | Calls |
| --- | ---: | ---: |
| Decode saved checked bodies | 4.544s | 46 modules |
| Check signatures | 4.240s | 49 modules |
| Lex source | 1.508s | 49 modules |
| Parse source | 0.696s | 49 modules |
| Check effects | 0.814s | 50 calls |
| Lower, emit and serialize bytecode | 1.213s | one of each |
| Check changed bodies | 0.165s | 3 modules |
| Encode changed bodies | 0.153s | 3 modules |
| Construct/revalidate module keys | 0.130s | 2 calls |

Remaining time includes source loading outside lexing/parsing, guards, cache file
handling, driver work and process overhead. The table deliberately separates
those costs from the measured pure body decoder; 4.544s is not a disk-read timing.

Fresh diagnostic compilation spent 7.082s checking all 49 modules' bodies and
4.223s checking signatures. The edited path removes most body checking but spends
4.544s reconstructing saved bodies. Cold cache encoding alone took 4.685s.
Unchanged cache hits still lex and parse all source: 2.199s of the 2.516s observed
invocation. Global effect checking remains fresh by design.

## Next bounded candidate

First trial a cheaper checked-body decoder, preserving corruption handling,
bounds, source/tool identity and byte-for-byte output. It is the largest measured
edited-build category and can be investigated without changing semantic
invalidation. Signature checking is the strongest alternative, but reusing its
results requires its own dependency contract; optimising repeated work inside
fresh signature checking may be simpler than persisting more state.

Function-level body reuse is lower priority for this workload: only 0.165s is
spent checking the changed modules. These observations do not establish the same
ordering for every user project, or promise that all decoder time can be removed.
No optimisation implementation is selected by this report alone.

The assessment changes documentation only. All eight compiler invocations
succeeded and their expected artifacts matched. Documentation/link checks are
the applicable repository validation; no full product suite or hosted performance
run is required for this report. The prior product and integration acceptance
remain valid.
