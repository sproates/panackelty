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

## Decoder trial, 8 November 2026

The owner selected this follow-up. Source positions have a file dictionary index
and three coordinates. Their usual representation is four fixed-width words.
The trial decodes those words after one bounds check and constructs one reader,
rather than allocating an intermediate reader for every field. The generated
schema is asserted before emitting this specialisation. Large Nat coordinates
retain the general decoder, and invalid indexes and truncated words still fail.
The encoded format, checksums, identity keys, depth limits and bytecode version
are unchanged. No cache bounds were relaxed.

The isolated test decodes the same 49 module payloads, containing 1,076 function
bodies, in separate VM invocations. Three pairs alternate old/new order.

| Pair | Original decoder | Candidate decoder |
| --- | ---: | ---: |
| 1 | 4.654s | 4.257s |
| 2 | 4.962s | 4.202s |
| 3 | 4.833s | 4.251s |

Median: 4.833s to 4.251s, a 12.0% reduction. These timings include file reads
and VM startup. An initial incorrectly placed cache produced no module payloads;
those empty-workload timings were discarded and the harness now requires both
nonempty payloads and matching positive decoded-body counts.

For the complete build comparison, the original seed at `814c81d` and candidate
compile the same copied compiler sources using separate private caches. Each
pair changes `module_magic` again, alternates compiler order, and compares both
incremental artifacts with fresh compilation. There were no concurrent builds.

| Pair | Original edited build | Candidate edited build | Fresh candidate |
| --- | ---: | ---: | ---: |
| 1 | 16.640s | 16.793s | 19.197s |
| 2 | 15.790s | 15.074s | 19.282s |
| 3 | 16.352s | 15.316s | 18.307s |

Median edited time: 16.352s to 15.316s, a 6.3% reduction. Two pairs improved;
the first was 0.153s slower, so this is a modest local result, not a universal
speedup or hosted-pipeline claim. All pairs reused 46/49 modules and emitted
identical bytecode to their fresh comparison. Single cold observations were
23.747s/23.187s and unchanged observations 2.762s/2.822s (original/candidate);
neither path is the target of this optimisation.

Disposition: retain the bounded decoder improvement. Focused codec tests passed 81 assertions; all 24 incremental CLI
groups and native-oracle artifact compatibility passed. The 77 coverage-policy
checks passed; only the declaration-position exclusion digest changes, with no
new exclusions or corpus changes. Bootstrap stages 2, 3 and 4 agree on
`79a4717360930b7c77ea06dcfd758629adb24f73b61090db9471e19d56dafa10`;
the standard-library artifact is unchanged. Final `make check` passed in 617s
(unit 388s, bootstrap 136s in parallel), including functional and release smoke
checks. This is an acceptance observation, not a matched suite-speed comparison. No website update is needed for this internal optimisation, and this
does not complete RM#28/GI#106 or RM#157.

## Local development workflow, 8 November 2026

The owner selected adoption in routine local candidate builds. `make compiler`
now uses the existing complete-artifact probe cache first, then enables checked
module reuse on a miss. It writes `build/compiler/candidate.bc`;
`make compiler-fresh` produces the same candidate without either cache.
The contributor and development guides select this as the ordinary edit/build
workflow. It does not replace the installed compiler or checked-in seed.

This is deliberately bounded: existing normal probes, full validation, bootstrap
proofs and hosted jobs retain their commands. No automatic CI cache adoption or
change to the public CLI default is included. Module caches use the existing
compiler identity and safety rules, live outside source trees, and are separated
by checkout and entry point. Cache creation failure falls back to fresh checking.
Requested trace output is visible; unrelated compiler diagnostics still fail.

### Workflow measurements

A copied source tree used the actual new Make targets with the existing VM
(`make -o native` excluded rebuilding that copied executable). Three real edits
changed `module_magic`, alternating fresh/incremental order. Each edited build
reused 46/49 modules and matched the fresh candidate byte for byte.

| Pair | Incremental candidate build | Fresh candidate build |
| --- | ---: | ---: |
| 1 | 14.191s | 15.907s |
| 2 | 13.814s | 15.860s |
| 3 | 13.638s | 15.847s |

Median edited build: 13.814s versus 15.860s fresh, a 12.9% saving for this
workload. The single unchanged observation was 0.136s: this preserves the
existing complete-artifact fast path, rather than claiming a new unchanged-build
speedup. The initial cold observation took 21.031s, reflecting cache population
overhead. These local observations are not whole-suite or hosted speedups.

The final checkout's `make compiler` also succeeded, matched the unchanged
bootstrap seed byte for byte, and checked the Euler example using the candidate.
The probe harness passes its existing cases and five new groups covering
opt-in behaviour, artifact hits, edits, cache-directory fallback, trace handling,
default cache location and fresh bootstrap command routing. Policy checks pass.
Final `make check` passed in 567s (unit 370s, bootstrap 122s in parallel),
including functional, packaging and release smoke checks. This is validation
evidence, not a matched whole-suite performance comparison. No compiler/VM source, bootstrap artifact,
bytecode format or website claim changes; RM#28/GI#106 and RM#157 remain open.

## Signature reachability trial, 8 November 2026

The owner selected a bounded trial of fresh signature checking. A separate
observer VM timed nested calls inside `check_module_signatures`. The original
profile used the current seed at `a828b04`; the candidate used the same VM
instrumentation after the change. Each emitted its expected self-compiled
artifact. Instrumentation is local only and does not enter the packaged VM.

The public-reachability helper answers whether a declaration has any public
route. It previously continued resolving selective re-exports after a local
public declaration or earlier route had already proved the answer. Two
short-circuit conditions stop resolving subsequent imports/selectors once that
Boolean answer is true. Module graph validation still checks all imports; no
signature, diagnostic, visibility rule, cache or bytecode format is removed.

| Diagnostic observation | Original | Candidate |
| --- | ---: | ---: |
| Signature checking, 49 modules | 4.322s | 1.666s |
| Public reachability, inclusive | 3.395s | 0.726s |
| Path-resolution calls within signatures | 31,189 | 9,722 |

Nested timings are not additive. These are single diagnostic observations,
used to locate the waste; the ordinary-VM trials below establish build impact.

### Paired builds

Both compilers compiled identical copied sources with separate private caches.
Each pair made a real edit to `module_magic` and alternated compiler order.
Edited builds reused 46/49 modules; all four outputs within each pair matched
byte for byte. No other builds ran concurrently.

| Pair | Original edited | Candidate edited | Original fresh | Candidate fresh |
| --- | ---: | ---: | ---: | ---: |
| 1 | 13.332s | 10.824s | 15.655s | 13.416s |
| 2 | 13.507s | 11.016s | 17.145s | 15.180s |
| 3 | 14.052s | 11.281s | 17.780s | 13.377s |

Median edited time fell from 13.507s to 11.016s (18.4%). Median fresh time
fell from 17.145s to 13.416s (21.8%); fresh timings varied more, although all
three pairs improved. Single cold incremental observations were 20.213s and
17.950s. This is one compiler workload, not a universal or hosted speedup.

Disposition: retain the two short-circuit conditions. The module-binding suite
passed 1,448 assertions, including eight new export-order/privacy checks. Six
negative functional programs produced identical exit status, stdout and stderr
with both compilers. Native-oracle artifact contracts passed, and the production
coverage inventory retains its existing exclusion digest. Bootstrap stages 2, 3
and 4 agree on `6e7a2f72e256b5e7d9f923b907f4c6c6ebeabe86596a257b69cb4a4313ba104d`;
the standard-library artifact is unchanged. Final `make check` passed in 565s
(unit 353s, bootstrap 101s in parallel), including functional and release smoke
checks. This suite duration is an acceptance observation, not a matched speedup
comparison. No website update is needed for this
internal optimisation; it does not complete RM#157 or GI#106.

## Name-lookup traversal trial — not adopted, 8 November 2026

A follow-up trial stopped walking remaining imports once ordinary name lookup
had selected a declaration or import. Duplicate-binding checks stayed before
selection. A separate observer VM counted and timed name resolution, import
selector access and binding diagnostics on the compiler self-build. The packaged
VM and seed were never changed.

Selector calls inside name resolution fell from 255,824 to 67,030. Instrumented
signature checking measured 2.203s/2.100s and body checking 8.578s/8.754s
(original/candidate). The trace contains many small calls and adds overhead;
these observations locate work, not establish a speedup.

Three alternating ordinary-VM pairs compiled identical sources without caches:

| Pair | Original | Candidate |
| --- | ---: | ---: |
| 1 | 12.928s | 13.299s |
| 2 | 13.426s | 13.170s |
| 3 | 13.155s | 13.029s |

The medians, 13.155s and 13.170s, are effectively even. Every artifact matched.
The candidate passed 1,453 module-binding assertions, including five temporary
cases covering local selection, the last import's edge index, missing names and
late binding conflicts. The production change and trial-only tests were removed;
no seed refresh or product change is retained. This documentation-only outcome
uses documentation/link checks, not another full product validation.

The profile identifies a stronger next candidate: name resolution called
`module_binding_diagnostics` 29,556 times across a 49-module compilation,
spending 5.829s inclusive in the observer run. Source inspection confirms that
each call rebuilds a name set and checks all declarations/imports again. The
trial deliberately preserved those checks. A future bounded design should
avoid repeating validation of an unchanged binding set within one compilation,
while preserving duplicate detection, diagnostic ordering and direct resolver
behaviour on invalid graphs. No such design or implementation is adopted here.

This completes the rejected trial, not RM#157/GI#106. There is no website impact.
