# VM shared Unit and Void trial

7 November 2026. Owner-requested combined trial on
`feature/rm162-real-program-fixes`, against `ac2a704`, with eight preceding
VM optimisations adopted.

## Change

Unit and Void each have a separate immutable static Value, using the existing
SIZE_MAX immortal-reference marker introduced for Booleans. value_unit() is the
explicit shared Unit constructor; production Unit construction sites now use it.
value_void() returns the shared Void marker. Generic value_new remains a heap
allocator, including for native ownership tests requiring real reference counts.

The distinct V_UNIT and V_VOID tags, equality rules, rendering and return checks
are unchanged. Unit still renders as () and Void as void. Callers continue to
balance ownership through retain/release; neither shared object may be mutated.
Both constructors cannot fail. Containers and result variants still allocate
normally and their allocation failures remain tested.

The change adds two static Values (128 bytes on this arm64 layout), no additional
retain/release branch beyond the Boolean mechanism, no per-object growth and no
bytecode change. It includes HTTP, console, filesystem, digest/gzip and host
success-result construction so those paths use the same Unit representation.

## Verification

- Strict-warning candidate VM and native test builds passed.
- Full native suite passed, covering async, host, network and return behavior.
- Added checks preserve Unit/Void distinction, equality with separately allocated
  values, normal heap reference counts, repeated shared construction and container
  retention/destruction.
- Fault checks prove Unit/Void construction and retain/release make no allocation
  attempt with the first allocation configured to fail.
- Native fault suite passed 2,044 injected failures, down from 2,056 because the
  removed allocations no longer create failure points. No assertions were removed.
- Five compiled/saved-bytecode fixtures passed exact output: collections,
  owned_intrinsics, callables, records_and_enums and async_await.
- All timed compilations produced identical baseline/candidate bytecode.

## Measurements

Five stdlib pairs and three compiler pairs, alternating order, identical saved
compiler and sources. Wall time includes startup, decode/verification, compilation
and output. No concurrent builds or tests during the comparison.

| Workload | Baseline median | Candidate median | Less time |
| --- | ---: | ---: | ---: |
| Stdlib imports | 0.246909 s | 0.234117 s | 5.18% |
| Compiler | 18.766589 s | 17.241106 s | 8.13% |

Compiler pairs (baseline / candidate, seconds):
18.065090 / 17.051557; 18.766589 / 17.241106; 19.679324 / 18.078070.
All three favour the candidate, but both versions vary across rounds. The median
percentage is a small local sample, not a portable guarantee or a whole-suite
speedup claim. Unit and Void were measured together; their separate contributions
have not been established.

Output SHA-256:

- Stdlib: `868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686`.
- Compiler: `9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.

## Status

Focused trial checks passed. Recommend adoption followed by one full local
validation. The normal checkout VM remains unchanged; full `make check` awaits
adoption. No push, PR or CI run.

Value ownership documentation and release/coverage records reflect the trial.
No website, language or CLI change is needed: observable semantics are unchanged.
