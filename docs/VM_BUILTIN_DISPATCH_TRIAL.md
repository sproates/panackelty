# VM builtin dispatch trial

7 November 2026. Local trial on `feature/rm162-real-program-fixes`, against
`7a0fa13`, with the previous six VM optimisations adopted.

## Change

The builtin registry classifies each entry at C build time. Ordinary entries use
one default classification; 24 special names retain distinct classifications.
The execution dispatcher compares these classifications rather than repeatedly
comparing builtin names. No runtime initialisation or mutable cache is needed.

The existing order and bodies of special handling remain intact, including
capability checks, awaited work, HTTP callbacks, process exit, embedded host
forwarding and coverage gaps. Pure and locally owned state operations retain
their existing host boundary. Direct and indirect calls both reach this dispatcher.
Names remain available for diagnostics, external host callbacks and existing
implementation-family dispatch. This trial does not remove every name comparison
from the VM or change how unresolved names are looked up.

A fresh baseline sample confirmed repeated strcmp work inside execution_builtin.
It is qualitative profiling evidence, not the timing comparison below.

The Builtin registry entry remains 24 bytes on this arm64 build: the classification
uses existing alignment space. No new heap allocation, per-instruction storage,
bytecode version or public API is introduced. Other platform layouts were not
measured.

## Validation

- Candidate VM and native tests build with the existing strict warning flags.
- Full native module suite passed, including resumable, async, TCP/HTTP,
  filesystem/console, coverage and host-policy contracts.
- New behavior tests cover pure versus host-forwarded ordinary builtins for
  direct/indirect calls, with and without prepared verification metadata.
- Native fault suite passed all 2,133 injected allocation failures.
- Five compiled and saved-bytecode fixtures passed exact expected output:
  collections, owned_intrinsics, callables, records_and_enums and async_await.
- Self-review checked that removing the classification field restores the old
  registry exactly, and replacing classification comparisons with their original
  names restores the old execution dispatcher exactly. No arity, purity, call
  implementation or control-flow changes were mixed into the trial.

## Measurements

Alternating execution order, five stdlib pairs and three compiler pairs, using
identical saved compiler bytes and input sources. Wall time includes startup,
decoding, verification, compilation and output. No concurrent builds or tests
ran during the timing comparison.

| Workload | Baseline median | Candidate median | Less time |
| --- | ---: | ---: | ---: |
| Stdlib imports | 0.283986 s | 0.277083 s | 2.43% |
| Compiler | 22.074287 s | 20.190343 s | 8.53% |

Compiler pairs (baseline / candidate, seconds):
22.074287 / 20.190343; 22.072860 / 20.180798; 22.087613 / 20.673218.
All three favour the candidate. This is a local small-sample result, not a
portable performance guarantee or a full-validation speedup claim. The short
stdlib measurement is noisier and less useful for predicting larger workloads.

Every output hash matched:

- Stdlib: `868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686`.
- Compiler: `9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.

## Status

Adopted in the normal checkout VM after owner approval. `make native` passed,
followed by full local `make check` (exit 0, 662 seconds): unit, functional,
bootstrap, packaging, release smoke and quick-start checks passed. Bootstrap
completed in 165 seconds. The total was 29 seconds below the preceding 691-second
run, but this is not a controlled whole-suite comparison. Historical timing
budget warnings remain advisory.

No push, PR or CI run. The stable globally installed release is unchanged.

No website, language or CLI documentation changes are required: externally
observable behavior is unchanged. VM component documentation records the internal
registry classification; release status and coverage records link this evidence.
