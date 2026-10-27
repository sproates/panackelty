# GI#106: shared compiler test preparation

27 October 2026. The owner approved this bounded improvement and its targets
following [the baseline](VALIDATION_SPEED_BASELINE.md). Implementation is merged into next in
[PR#637](https://github.com/sproates/panackelty/pull/637), measured at
`97f19bfd289d225d62b943996812f20cfeffcf66`.
**Measured targets are met; final owner acceptance remains outstanding.**
This does not close the wider GI#106 umbrella issue.

## What changed

Nine heavy compiler suite bodies moved into importable test modules. Their bytes
match the originals after reversing the `main` to `pub run_suite` declaration
rename. Existing standalone entry paths remain as wrappers. Make prepares one
shared artifact, then the existing bounded worker pool selects and executes
one suite per fresh VM process. Each target retains its previous suite selection.
Lightweight probes remain separate.

No production compiler, standard-library, VM or package-policy code changed.
No timeout was increased and no required merge, nightly or release validation
was removed. This is shared test preparation, not separate compilation for user
programs or the alpha.16 native-coupling work.

All nine real suites passed in the pilot, totalling 1,838 assertions. Checker
and source-inventory shared/standalone output matched exactly. The real
dispatcher, VM and cache also passed small-fixture regressions for:

- Every suite twice, and ordered once-only worker-pool execution.
- Missing and unknown selection, with status 2 and no suite execution.
- An injected assertion failure after cached success, followed by restored success.
- Direct suite and transitive dependency edits invalidating the artifact.
- Corrupt bytecode and incomplete digest publication forcing recompilation.
- A missing suite failing instead of using stale bytecode, then recovering.

Existing cache regressions continue to cover compiler/VM identity, failed
compilation and inputs edited during compilation. The new regression group added about three seconds to
the local full harness. Policy and documentation checks passed.

## Five-workload replay

Same machine, seed, command sequence, equivalent source edits, worker count and
cache protocol as the baseline. Each command passed. Profile outputs were outside
the checkout; temporary source edits were restored byte-for-byte. No competing
local benchmark ran. These are paired operational samples on an active desktop,
not a statistical guarantee for every run.

| Workload | Before | After | Change / result |
| --- | ---: | ---: | --- |
| Clean full, two workers | 1245.71s | 1045.17s | 16.10% faster; 15% target met |
| Unchanged full repeat | 821.77s | 847.04s | 3.08% slower; within 5% guard |
| Small compiler edit | 534.43s | 427.30s | 20.05% faster; 20% target met |
| Compiler unchanged repeat | 300.48s | 299.91s | Essentially unchanged |
| Small package-tool edit | 53.10s | 53.51s | 0.77% slower; within guard; no compiler rebuild |
| Package unchanged repeat | 31.30s | 32.04s | 2.36% slower; within guard |
| Documentation median, three runs | 4.25s | 4.18s | No compilation; below 5s |

Documentation samples after the change were 4.20s, 4.14s and 4.18s. Clean and warm
full checks execute the complete unit and functional suites. The unit phase fell
from 887s to 688s on clean; the functional phase remained 240s. The warm unit
phase was 512s versus 501s, and warm HTTP-client checks were also several seconds
slower. These slower observations are retained rather than hidden by the clean
result.

The compiler-edit result only just clears its target. Calculating the agreed
20% reduction from the unrounded 534.43s baseline gives **427.544s**. The earlier
whole-second 427s figure was over-rounded; this report corrects that arithmetic
rather than changing the agreed percentage. A 427.30s observation is not evidence
of substantial spare headroom or a guaranteed maximum on a busy machine.

### Compilation and execution evidence

| Workload | Real cached-probe compilations before / after | Inclusive compilation seconds before / after |
| --- | ---: | ---: |
| Clean full | 42 / 35 | 445 / 243 |
| Unchanged full | 1 / 1 | 34 / 34 |
| Compiler edit | 27 / 19 | 425 / 179 |
| Compiler repeat | 0 / 0 | 0 / 0 |
| Package edit | 5 / 5 | 18 / 18 |
| Package repeat | 0 / 0 | 0 / 0 |

Counts exclude the synthetic compiler used by the cache harness. They are not
a census of every compiler invocation: direct CLI compilations, native C builds,
later bootstrap stages and private handoff profiles are separate. The one warm
full compilation is intentional isolated bootstrap preparation. Package checks
retain their deliberate edited-package rebuild proof and never rebuild the
compiler or seed.

Shared preparation took 48s in both the initial pilot and the full clean run.
Existing suite assertions still execute on every selection. Profile parents
include children and concurrent work overlaps; do not add these numbers as total
wall time or label every `probe-run` second pure assertion work.

## Hosted comparison

Both platforms passed [candidate profile run 40427251177](https://github.com/sproates/panackelty/actions/runs/40427251177),
using the same image versions and two-worker workflow as
[baseline run 40418980260](https://github.com/sproates/panackelty/actions/runs/40418980260).
These are representative focused checks, not hosted replays of all five local
workloads. Native preparation precedes measurement; component order and cache
sequence are unchanged.

| Platform / check | Before initial / cached | After initial / cached |
| --- | ---: | ---: |
| macOS compiler | 975s / 493s | 732s / 485s |
| Linux compiler | 983s / 480s | 621s / 379s |
| macOS bytecode | 70s / 52s | 66s / 48s |
| Linux bytecode | 71s / 49s | 51s / 36s |
| macOS VM | 327s / 326s | 342s / 340s |
| Linux VM | 415s / 414s | 336s / 336s |

Initial compiler checks improved 24.9% on macOS and 36.8% on Linux, exceeding
the 10% target. The unchanged Linux VM path also ran substantially faster, so
machine variability contributes to that platform's result; the entire observed
reduction must not be attributed to this change. macOS VM times increased by
less than 5%. Cached compiler checks did not regress on either platform.

Images match: macOS `20260831.0302.1` with Apple clang 15.0.0; Ubuntu
`20261023.322.3` with GCC 11.4.0. Candidate dispatch was
`2026-10-27T00:34:19Z`; both jobs started three seconds later. macOS completed
at `01:08:11Z`, Linux at `01:04:08Z`. Queueing is separate from measurement.
Artifacts preserve full profiles and environment metadata.

## Ordinary four-worker guard

Both runs passed: baseline **409.37s**, candidate **378.22s**, a **7.61% reduction**.
Baseline `e17ad6c` and candidate `97f19bf` were selected sequentially in the same
checkout, with native/package prerequisites prepared outside the timer and
separate initially empty probe caches. Command:

```sh
make -j1 VALIDATION_JOBS=4 PANACK_PROBE_CACHE="$cache" check-compiler
```

This guards the default worker setting separately from the agreed two-worker
measurements. It is not interchangeable with a clean full check.

## Reproduction, delivery and acceptance

Follow the exact five-workload edits and order in the baseline. Candidate raw
logs, patches, profiles and environment are retained on the measurement host at
`.panackelty-ops/gi106-shared-2026-10-27/`, outside the repository. The four-worker
script records both revisions, uses separate caches, restores the candidate
branch and preserves unrelated files. All task-generated build/cache artifacts
are removed after measurement; raw evidence is retained.

PR#637 merged the implementation into next at
`a5fdb74186d4340f2ebb00aa8c7f84fb4e974150`. This report delivers the comparison and
acceptance record; it introduces no runtime or validation behavior. Required PR
checks must be green at the exact merged head. The milestone is **awaiting owner
acceptance**, not complete. GI#106 remains open, and the release's known weighted
contribution remains 35/90 until acceptance is recorded. RM#154: Faster coverage
remains next in the agreed order after this checkpoint. No website claim changes
are required; the public compiler and package behavior is unchanged.
