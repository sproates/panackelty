# VM temporary call arguments trial

7 November 2026. Local trial on `feature/rm162-real-program-fixes`, against
`811336c`, with the previous five VM optimisations adopted.

## Change and ownership

Direct and indirect calls, including awaited calls, use an eight-pointer C-stack
buffer for up to eight arguments. Larger calls retain the existing heap array.
Every used slot is filled before invocation or cleanup, including partial operand
stack failures. Values needed beyond invocation are retained by the callee or
host; neither implementation keeps the temporary argument array alive afterwards.

The threshold covers 7,088 of 7,103 static call sites in the saved compiler
(99.79%). The maximum arity there is 13. This is a static inventory, not a
measurement of dynamic call frequency or proof that eight is the optimal cutoff.
The small buffer contains 64 bytes of pointers on this arm64 machine. No retained
heap cache, bytecode change, public API change or object-layout change is added.

A fresh baseline sample still showed allocation/free work prominently. That
sample does not attribute all allocator cost to call arguments.

## Validation

- Candidate builds with the existing strict warnings.
- Direct and indirect calls pass at arities 0, 1, 8, 9 and 255, checking the last
  argument and cleanup after partial stack underflow.
- Native module suite passed; native fault suite checked 2,133 injected failures.
  New sweeps cover direct/indirect calls at the inline/heap boundary, including
  underflow and allocation failure.
- Five compiled and executed fixtures passed exact expected output: collections,
  owned_intrinsics, callables, records_and_enums and async_await. These include
  deep calls and awaited/indirect execution.
- Every timed compilation produced the same bytecode hash as the baseline.

## Measurements

Alternating baseline/candidate order, five stdlib pairs and three compiler pairs,
using the same saved compiler and sources. Wall time includes decoding,
verification, compilation and output. No concurrent builds or tests.

| Workload | Baseline median | Candidate median | Less time |
| --- | ---: | ---: | ---: |
| stdlib | 0.310621 s | 0.300504 s | 3.26% |
| compiler | 21.186291 s | 20.605985 s | 2.74% |

Compiler pairs in seconds (baseline / candidate):
21.698334 / 20.446814; 21.186291 / 20.605985; 21.146131 / 20.793706.
All three favour the candidate, but the small sample does not establish a precise
portable speedup. The short stdlib workload is noisy, including a 0.604-second
first baseline run; its median is not a strong performance claim.

Stdlib SHA-256: `868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686`.
Compiler SHA-256: `9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.

## Status

Adopted in the normal checkout VM after owner approval. `make native` passed,
followed by full local `make check` (exit 0, 691 seconds): unit, functional,
bootstrap, packaging, release smoke and quick-start checks passed. Bootstrap
completed in 178 seconds and preserved the compiler hash above. Historical
timing-budget warnings remain advisory; this run is not a controlled measurement
of whole-suite speedup.

No push, PR or CI run. The stable globally installed release is unchanged.
Architecture, VM ownership documentation and coverage/status records were updated.
No language, CLI, bootstrap-format or website claim changes are required: this
is an internal allocation optimisation.
