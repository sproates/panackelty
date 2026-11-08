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

## Binding-validation reuse trial, 8 November 2026

The preceding profile found repeated duplicate/reserved-name checks during name
resolution. Collected `ModuleBindings` now retain the diagnostics with the exact
declarations and imports that produced them. Each lookup compares those snapshots
before reuse. Raw bindings validate normally, and altered names, imports or source
positions force fresh validation. The original diagnostic algorithm and order are
unchanged. No lookup result, disk format, bytecode format or VM implementation is
changed.

The snapshot keeps references to the immutable input arrays plus the diagnostics;
it does not intentionally clone declaration/import contents. Structural equality
still walks the inputs, so this is not constant-time validation. The measured
saving comes from avoiding repeated interpreted validation and temporary name-set
construction. Memory use was not separately measured; retained diagnostics live
with their module bindings for the compilation.

Three alternating ordinary-VM fresh builds compiled the same candidate sources:

| Pair | Original | Candidate |
| --- | ---: | ---: |
| 1 | 13.320s | 9.198s |
| 2 | 13.034s | 9.152s |
| 3 | 13.116s | 9.323s |

Median time fell from 13.116s to 9.198s (29.9%). Every output matched byte for byte.
These are local compiler self-builds, not a universal or hosted speedup claim.

Three alternating edited-build pairs changed `module_magic` in a copied source
workspace. Cache directories were outside that workspace; each compiler had its
own cache, and both reused 46/49 modules after each edit:

| Pair | Original edited | Candidate edited |
| --- | ---: | ---: |
| 1 | 11.041s | 9.761s |
| 2 | 12.497s | 9.633s |
| 3 | 12.132s | 10.880s |

Median edited time fell from 12.132s to 9.761s (19.5%). All paired outputs matched.
Cold cache population took 17.975s/13.768s in one observation. No other builds
ran concurrently with timings. Initial harness runs omitted cache-directory
creation and fell back to fresh compilation; they are excluded from these edited
measurements. The corrected harness requires the expected module-hit trace.

The candidate passes 1,456 module-binding assertions, including eight new checks
for ordered diagnostics, changed declarations/imports, recovery, moved positions
and direct resolution of invalid bindings. Six negative functional programs retain
exact exit status, stdout and stderr; native artifact contracts pass. Bootstrap
stages 2, 3 and 4 agree on
`8119c5b34934dc7832bc770f8ada2720b32adfdeea3499c566c31705f9f783c2`;
the standard-library artifact is unchanged. The production coverage inventory
adds only the snapshot record to declaration exclusions (51 files, 485 exclusions);
all 77 coverage-policy tests pass. Final `make check` passed in 611s (unit 377s,
bootstrap 77s in parallel), including functional, packaging and release-smoke
checks. This duration is acceptance evidence, not a matched suite-speed comparison.
The bounded optimisation is included in this integration.
No website change is needed; RM#157/GI#106 remain open.

## Post-optimisation profile, 8 November 2026

Baseline: merged `a4c15a313a3bcb2c5ac643aded541be54e51e66e` (PR #686).
The compiler, normal VM and cache formats are unchanged. A separate observer VM
was built from the existing watchdog hook, selecting coarse phase boundaries.
The copied 49-module source workspace, compiler identity and source edits were
the same for fresh and incremental execution. Each VM used its own cache outside
the source root. Cache directories were created explicitly; each edited run
required 46 module hits. No other builds were launched concurrently.

Normal-VM timings alternated execution order. The first three pairs included a
large variation, so two additional pairs were collected rather than declaring
incremental compilation slower from the initial results:

| Pair | Edited incremental | Fresh | Order |
| --- | ---: | ---: | --- |
| 1 | 10.827s | 10.277s | Edited, fresh |
| 2 | 10.793s | 10.209s | Fresh, edited |
| 3 | 12.922s | 16.691s | Edited, fresh |
| 4 | 10.893s | 10.967s | Fresh, edited |
| 5 | 10.797s | 10.713s | Edited, fresh |

Medians are 10.827s incremental and 10.713s fresh, about 1.1% apart. Three pairs
favoured fresh work and two favoured reuse. This workload shows no clear edited
speed advantage after the compiler optimisations; it does not establish a
universal cache regression. The cause of timing variation was not determined.
One cold incremental build took 15.165s; an unchanged complete-artifact hit took
2.834s. Every fresh/edited pair matched byte for byte, as did cold/unchanged and
normal/observer outputs for corresponding inputs.

The observer ran cold, unchanged, edited and fresh modes once each. Its edited
invocation took 11.033s. Balanced enter/return events were checked and selected
nested timings subtracted, so categories below do not double-count:

| Edited-build phase | Observed time | Calls |
| --- | ---: | ---: |
| Decode saved checked bodies | 4.494s | 46 |
| Lex source | 1.670s | 49 |
| Check effects | 0.940s | 50 |
| Parse source | 0.780s | 49 |
| Lower, emit and serialize bytecode | 1.383s | one of each |
| Check signatures | 0.486s | 49 |
| Encode changed bodies | 0.166s | 3 |
| Check changed bodies | 0.146s | 3 |
| Construct/revalidate module keys | 0.140s | 2 |
| Cached-body loading outside decoding | 0.026s | 1 |

Remaining time includes driver, loading, cache handling and process overhead.
These are instrumented phase observations, not exact uninstrumented costs.
The fresh observer invocation also varied upward (15.909s, including 8.273s
checking bodies), so it must not be presented as a matched speedup estimate.
Cold observed encoding took 5.176s. Unchanged execution spent 2.455s lexing/parsing
before its complete-artifact hit. Decoding's 4.494s wall time closely matched
4.489s process CPU time; it is computation, not primarily waiting for disk I/O.

**Next recommendation:** a bounded checked-body decoder trial. Profile its reader
and record reconstruction costs, then reduce intermediate work while retaining
malformed-input rejection, bounds/depth checks, large-Nat handling and fresh-output
equivalence. Keep the current cache and bytecode formats. Adopt only a measured
end-to-end improvement on the ordinary VM; a reduction in call count alone is
insufficient. The earlier SourcePos specialisation remains in place, so this is
further investigation, not a proposal to repeat that change.

This outranks function-level reuse: changed-body checking costs only 0.146s here.
Lex/parse avoidance is a useful later candidate, especially for unchanged builds,
but requires a separate source-validation/reuse contract. Signature checking is
no longer a leading cost. No compiler change, default-policy change or new cache
is selected by this report; existing incremental behaviour remains available.

All 16 compiler invocations succeeded with the required artifact and cache-hit
comparisons. This measurement-only delivery requires documentation/link checks,
not a compiler rebuild or full validation run. It completes the reassessment,
not RM#157/GI#106, and has no website impact.

## Span and body-node decoder trial, 8 November 2026

A more detailed observer on baseline `6242eeff4343fe60b25a97bee8fc5c2002a98473`
found 118,087 source-span decodes (236,174 positions), 91,988 declaration-identity
decodes and 84,707 body-node decodes in the 46 reused modules. Selected exclusive
observer costs included 0.853s span scaffolding, 1.721s position reconstruction,
1.548s body-node decoding and 1.610s bound-body scaffolding. Fine-grained tracing
raised total decoding to 8.134s; these figures locate repeated work and are not
normal-VM timings. Recursive inclusive totals must not be added together.

Two bounded specialisations retain the cache and bytecode formats:

- Decode the two ordinary fixed-width positions of a span together, constructing
  one reader/result pair. The generator checks the expected schemas. Large Nat
  coordinates, truncation and invalid dictionary indexes retain the general path.
- Validate a body-node tag once and select its payload with a balanced comparison
  tree instead of a linear chain of up to 18 cases. Payload decoding, depth limits
  and rejection of out-of-range tags are unchanged.

The span-only trial produced small paired savings: 10.482/10.324s,
10.129/9.981s and 10.254/10.177s (original/candidate edited builds). Rather than
refreshing the seed for that alone, the related tag specialisation was trialled
in the same batch. Three alternating ordinary-VM pairs on identical combined
candidate sources measured:

| Pair | Original edited | Candidate edited | Candidate fresh |
| --- | ---: | ---: | ---: |
| 1 | 10.709s | 10.363s | 10.330s |
| 2 | 10.289s | 10.260s | 9.685s |
| 3 | 10.620s | 10.206s | 9.831s |

Both compilers reused 46/49 modules. Each edited pair matched its fresh output;
cold artifacts matched too. No other builds ran concurrently. Median edited time
fell from 10.620s to 10.260s (3.4%); individual savings ranged from 0.029s to 0.415s.
This is a modest local self-build improvement, not a universal gain. Cold cache
population was 14.425s/14.604s in single observations; no cold-build saving is
claimed. The combined experiment does not isolate the tag change's contribution.

The focused module-binding suite passes 1,471 assertions, including 96 body-cache
checks. Added cases cover truncation through both positions, invalid end-file
indexes, large coordinates only in the end position, the last valid body-node tag
and invalid tags. Native artifact contracts and bootstrap fixed points pass;
standard-library bytes are unchanged. Bootstrap stages 2, 3 and 4 agree on
`832e623e1ab83e96efc1f4af7180f3566d3f09f83f8a440dc4baf1d89440665d`.
All 77 coverage-policy tests pass. The coverage-policy review updates only
positions/identifiers of existing declaration exclusions, not executable coverage.
Final `make check` passed in 560s (unit 347s and bootstrap 79s in parallel),
including functional, packaging, release smoke and quick start. This is acceptance
evidence, not a controlled suite-speed comparison. The batch is included in this
integration.

### Larger reuse question

The current cache reconstructs a rich checked-body tree, including repeated source
spans, identities and typed structure. The owner raised the possibility of a larger
breakthrough, and the profile supports investigating representation/reconstruction
cost rather than assuming more small reader specialisations are sufficient.
Saving less data or retaining a form closer to what emission consumes are
hypotheses, not selected designs. Any later assessment must inventory the actual
emission/effect consumers, preserve validation and diagnostics, and compare total
fresh, cold, unchanged and edited costs. Format changes require a separate bounded
proposal; this trial does not authorise or implement one. Function-level reuse
still addresses little of this particular workload's measured cost.

This batch does not complete RM#157/GI#106 and has no website impact.

## Retained-data assessment, 8 November 2026

Baseline: `efa9a293d96f8bd958c4c95209ab83c11a6c3f51`, after the span/tag batch.
The question is whether substantially less data can be saved/restored, rather
than continuing to optimise individual reads. This assessment changes no product
code or format and does not claim a measured runtime improvement.

### What actually consumes the cache

`check_project_sources_with_reuse` in `loader.panack` installs cached
`BoundFunctionBody` values, checks their status and owner identity, and passes them
to fresh global effect checking and module emission. Signatures and guards still
run fresh. The persisted schema already omits explanation evidence, constraints,
operator/type-check histories, deferred checks and return-type evidence; removing
those again would save nothing. Strings are already dictionary encoded.

| Retained information | Current ordinary-mode consumer | Assessment |
| --- | --- | --- |
| Node shape, children, literals and call targets | `module_emission_expression_with_parameters`, `module_emission_block`; effect traversal | Needed to reproduce code and call/await rules. |
| Spans, block/statement and pattern positions | Located expressions, block origins, effect diagnostics | Do not remove wholesale. Positions affect correctness and diagnostic equivalence, not just explain output. |
| Function signature and binding identity | Loader ownership/coverage checks, effect context, emitted parameter/result types and callable names | Keep; repeated occurrences are candidates for shared references. |
| `BoundBody.type` | Indirect-call effect classification and non-Unit awaited-result rejection | Not generally disposable. Most ordinary nodes may not need the full type, but selective omission needs a context-aware contract. |
| `BodyBinding` annotation, mutable flag and local name/owner | Emitted binding and type spelling | Needed; local names alone are insufficient for every binding. |
| `BoundFunctionBody.parameters` | No read found on the cached loader/effects/emission path; emission takes parameter names from fresh declarations | Candidate omission, not yet implemented. |
| `BodyCall` generic type-argument payload | Both ordinary emission and effect checking bind but do not use it | Candidate omission, not yet implemented. |
| Local identity paths and repeated owner/type structures | Plain emission mostly selects name/owner; other compiler modes use richer identities | Prefer sharing first; broad deletion would couple the cache to a new reduced-body contract. |

The reader/writer boundary, not every use of `BoundFunctionBody` elsewhere in the
compiler, is the relevant scope: explain/source tooling bypasses this plain cache.
Nevertheless, future consumers must be reviewed before narrowing that contract.

### Inventory of real payloads

A fresh incremental compiler self-build generated 49 module payloads. An offline
schema-driven inventory verified every module payload SHA-256, dictionary index,
tag and buffer boundary, and consumed each payload exactly. It follows the
explicit body-cache schema and current intrinsic variants; it is investigation
tooling, not a replacement product decoder. Total payload size was **9,522,928
bytes (9.08 MiB)**, including 241,358 bytes of existing string dictionaries.

Counts below are across all 49 modules; distinct counts are summed **within each
module**, so the opportunity does not require a cross-module cache or shared
lifetime. Byte totals for nested types overlap and must not be added.

| Structure | Occurrences | Distinct within modules |
| --- | ---: | ---: |
| Signature/type description | 123,405 | 1,705 |
| Declaration identity | 94,354 | 3,337 |
| Declaration binding | 13,784 | 2,833 |
| Local identity | 40,244 | 11,436 |
| Source position | 244,636 | 142,920 |
| Source span | 122,318 | 91,976 |

The two straightforward omission candidates above occupy **245,172 bytes (2.6%)**
combined. They are not the large opportunity on their own. `BoundBody.type`
occupies 1,912,156 bytes, but some of it is required by current effect checks.
Full source spans occupy 3,914,176 bytes; dropping them would erase required
information. Repetition of immutable structures is the stronger first target.

### Model shared values, not missing information

A second pass models per-module typed dictionaries with four-byte references.
Each distinct typed value is defined once, child references are included, and
repeated parents do not charge their children again. The model includes one
count word per table and preserves the existing string dictionary. It compares
complete payload sizes, avoiding double-counting nested identities and types.
It is a size model, not an implemented codec or timing forecast; additional
framing/validation choices could change the exact result.

| Model | Payload bytes | Reduction |
| --- | ---: | ---: |
| Current | 9,522,928 | — |
| Share `SignatureType` values | 8,149,660 | 14.4% |
| Also share declaration identities/bindings and local identities | 6,521,712 | 31.5% |
| Also share source positions | 6,135,820 | 35.6% |

This preserves information while avoiding repeated reconstruction. A decoded type
such as `Nat` can be reused by many nodes instead of repeatedly allocating the
same type record and argument list. The model's 1,705 distinct type values versus
123,405 occurrences is a strong reason to trial sharing, but fewer bytes/objects
do not establish a speedup: dictionary construction and lookups also cost work.
No memory or ordinary-VM timing benefit is claimed yet.

### Recommendation and bounded first trial

**Yes, substantially less representation is plausible without discarding semantic
information. Trial per-module type sharing first**, then identities only if the
end-to-end results justify it. This is a larger reduction in repeated work than
another fixed-field reader specialisation, while leaving downstream compiler
records and their consumers intact. It is a proposed one-PR experimental slice,
not a selected implementation or a promised breakthrough.

The trial contract should be:

- Store unique immutable types once and rebuild each once; ordinary body records
  receive shared values with identical structural contents.
- Explicitly version the disposable module-cache representation and reject old,
  truncated, out-of-range, cyclic/forward-invalid, excessive-count and excessive-
  depth data. Missing/incompatible cache state falls back to fresh compilation.
- Preserve source/tool/transitive-dependency keys, global effect checking,
  diagnostics, positions, and source/saved-bytecode results. No VM bytecode change.
- Measure fresh, cold, unchanged and genuinely edited builds on the normal VM,
  including encoding cost and memory where practical. Verify actual cache hits
  and compare fresh outputs. Reject a net regression or a merely smaller file.
- Batch any justified identity-sharing extension before one final full validation,
  rather than paying for a separate full suite after every tiny experiment.

A more ambitious boundary would cache lowered per-module code plus a compact
summary of call/effect obligations, avoiding checked-body reconstruction entirely.
That requires a real effect-summary contract, diagnostics/source-origin retention,
and composition checks. Emitted names currently incorporate graph module indexes;
the current cache keys already include graph order/membership, so conservative
invalidation could protect them. More granular reuse across graph changes would
need stable symbols or relocation. It is not simply a matter of saving today's
bytecode per file. This route has a higher potential ceiling but materially larger
scope; investigate it only if the smaller sharing trial leaves substantial cost.

The assessment completes the requested investigation, not RM#157/GI#106. No
compiler rebuild, seed refresh, full validation or website change is required;
documentation/link checks cover this delivery. No cache redesign was implemented.
