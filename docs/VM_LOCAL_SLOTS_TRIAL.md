# VM local-variable slots trial

7 November 2026. Owner-approved local experiment on
`feature/rm162-real-program-fixes`, against `cbce9fe`. The baseline already
includes adopted direct-call links and direct concatenation allocation.

## Approach and boundary

Verification gathers each function's parameter and local names, sorts a temporary
name table, and assigns one slot per distinct name. Parameters keep their original
argument positions. LOAD, STORE, ITER_INIT and both ITER_NEXT names receive slot
numbers. The temporary name table is freed before execution.

Slots share the instruction's existing derived call-link storage: local-access
and call instructions use different union members. Nothing is serialized. A
program's slots become usable only after successful verification of the whole
program; failure invalidates both execution caches. Re-verification rebuilds the
slots. As with direct-call links, verified programs must remain immutable while
executing.

A verified call frame allocates a zeroed array of value references. Reads borrow
references; replacement releases the previous value; frame destruction releases
all retained values. Unassigned slots remain NULL and preserve the existing
uninitialised-local trap, including assignments skipped by control flow. Slots
remain in the frame across async suspension. Unverified native test programs keep
the existing name-based execution path.

The owner approved adoption after reviewing this experiment. The normal checkout
VM is rebuilt with indexed locals and full local validation passed on 7 November
2026 (`make check`, exit 0, 690 seconds).
No CI, PR, merge, installed release, compiler source or bytecode format change.
This includes unit, functional, bootstrap, package/install, release-smoke and
quick-start checks. The compiler bootstrap hash remains unchanged.

## Profile

A five-second sample of the existing VM compiling the compiler captured 4,261
main-thread samples. `local_get` and `local_put` were visible hotspots, alongside
string comparison and allocation/free work. Their self samples were 181 and 83;
these exclude time in their callees and do not establish a predicted speedup.

## Acceptance evidence

- Native module suite passed, including verified async suspension/resumption.
- New native checks cover parameter order differing from alphabetical order,
  reassignment and retained aliases, branch-skipped assignment, verification
  failure invalidation and successful re-verification after editing instructions.
- Native fault suite passed: 2,092 injected allocation failures, including the
  temporary slot preparation and indexed-frame construction/cleanup.
- Complete-program comparisons and benchmark results are recorded below.

The website and public language/API contract are unaffected. The experiment is
tracked under RM#157: Performance review; it does not complete that wider review.

## Compilation measurements

Separate native VMs built with the same `cc -O2` flags. Baseline and candidate
alternate order on successive pairs; no other builds or tests ran concurrently.
Measurements include VM startup, decoding, verification/preparation, compilation
and writing output. Source paths were the same for both executables.

| Workload | Pairs | Baseline median | Candidate median | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Compile 28 stdlib imports | 5 | 0.390646s | 0.297229s | 23.91% |
| Compile compiler | 3 | 28.169218s | 23.132269s | 17.88% |

Raw elapsed seconds in pair order:

- Stdlib baseline: 1.263767, 0.390646, 0.371368, 0.384448, 0.403367.
- Stdlib candidate: 0.606451, 0.287230, 0.300690, 0.276408, 0.297229.
- Compiler baseline: 28.651471, 27.818655, 28.169218.
- Compiler candidate: 23.132269, 23.714594, 22.984095.

The slower first stdlib pair is retained, not discarded. These are local results,
not a CI or whole-validation speedup claim. All stdlib output hashes were
`868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686`;
all compiler outputs matched the existing bootstrap hash
`9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.

The candidate compiled and ran saved bytecode for `collections`,
`owned_intrinsics`, `callables`, `records_and_enums` and `async_await`, matching
each fixture's expected stdout exactly. This includes deep recursion (20,000
calls), collection iteration, enum bindings and nested async suspension. The
installed CLI and normal checkout VM were not replaced for these trial checks.

## Memory trade-off

On this arm64 build, Instruction remains 120 bytes and Program remains 24 bytes.
Function grows from 48 to 56 bytes. Frames gain a slots pointer and mode flag
(16 bytes including alignment); indexed local storage uses one pointer per
possible local rather than a name/value pair plus copied name per assigned local.
This reserves slots even for branches not executed, trading some potential
unused slots for fewer allocations and name copies.

The compiler bytecode has 894 functions, 9,668 total distinct local slots across
those functions, and a maximum of 145 slots in one function (1,160 bytes per
invocation's slots on this target). Its largest explicit temporary name table
has 1,111 entries (17,776 bytes), freed after that function is prepared. These
figures describe the compiler workload, not universal limits on user programs.

A separate sequential `/usr/bin/time -l` compiler-compilation pair measured
391,692,288 bytes maximum resident set size for the baseline and 387,579,904 for
the candidate. Peak memory footprint was 382,730,912 versus 378,765,960 bytes.
Both outputs retained the bootstrap hash. One memory pair supports no observed
peak-memory regression on this workload; it is not a universal memory guarantee.

## Reproduction and disposition

Build the baseline revision and candidate into separate directories using
`make BUILD_DIR=build/locals-trial-baseline build/locals-trial-baseline/vm/panack-vm`
and the analogous candidate path, with identical flags. Run each VM with
`run bootstrap/compiler-v9.bc compile SOURCE -o OUTPUT`, setting
`PANACKELTY_STDLIB_PATH` to this checkout's `src/stdlib` and
`PANACKELTY_BOOTSTRAP_ROOT` to this checkout. Use `src/compiler/main.panack` for
the compiler workload and a source importing all 28 stdlib modules under unique
aliases for the stdlib workload. Alternate five stdlib pairs and three compiler
pairs, retaining all samples and comparing output hashes. Run measurements alone.

The owner approved adoption: the trial improves both measured compilation
workloads, preserves bytecode and passes focused and full local correctness
checks. This adds to the two previously adopted optimisations.

The adoption `make check` took 690 seconds versus 820 seconds for the preceding
concatenation adoption, a 130-second reduction (15.85%). Bootstrap took 174
seconds versus 209 seconds. These are single sequential full-suite runs, not a
controlled repeated benchmark or a hosted-CI performance guarantee. Historical
timing-budget warnings remain advisory. No tests or checks were removed.
