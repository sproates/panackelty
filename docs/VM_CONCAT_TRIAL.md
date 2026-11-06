# VM concatenation trial

7 November 2026. Local experiment on `feature/rm162-real-program-fixes`,
against `bc6c880`, which already includes the adopted direct-call lookup cache.
No bytecode, compiler, bootstrap, public API or language semantics change.

## Change

String addition, byte append/concatenation and array concatenation construct the
result directly in its final storage. This removes one temporary allocation,
copy and free per operation. String Unicode character counting still scans the
result; array elements retain their existing reference-counted ownership.
Length overflow is rejected before allocating or reading inputs.

The normal checkout VM has **not** been replaced by this candidate. This trial
uses separate baseline/candidate build directories. It is not yet a completed
product change: focused verification passed, but final `make check` and adoption
remain outstanding. No CI, PR, integration merge or website change.

## Measurements

Both VMs built with the same `cc -O2` flags on the same Mac. Each microbenchmark
performs 200,000 operations, releasing every result, with identical output checksums.
Five alternating baseline/candidate pairs; table shows medians, not best runs.
Inputs: two 4,096-byte ASCII strings or byte buffers; one 4,096-byte buffer plus
a byte for append; two 512-element arrays for array concatenation.

| Workload | Baseline seconds | Candidate seconds | Time reduction |
| --- | ---: | ---: | ---: |
| String addition | 0.424360 | 0.408903 | 3.64% |
| Byte concatenation | 0.042494 | 0.023995 | 43.53% |
| Byte append | 0.028205 | 0.017175 | 39.11% |
| Array concatenation | 0.645904 | 0.532840 | 17.50% |
| Compile 28 stdlib imports (five pairs) | 0.396922 | 0.394572 | 0.59% |
| Compile compiler (one pair) | 26.024568 | 26.212680 | -0.72% |

The compiler timings do **not** establish a speedup. These operation-level gains
depend on input size and workload; they do not predict whole-validation savings.
The string result is small and needs caution. Byte and array results support
keeping this as a candidate for adoption.

Both compiler outputs matched the baseline exactly, including the compiler's
bootstrap hash `9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.

## Verification

- Native module contracts passed, including new empty-result, Unicode, embedded
  NUL, length-overflow and repeated/shared array-element ownership assertions.
- Native fault contracts passed: 2,082 injected allocation failures, including
  every new constructor allocation and post-failure ownership checks.
- `collections` and `owned_intrinsics` functional fixtures compiled with the
  candidate VM, then their saved bytecode ran with exact expected stdout. This
  directly exercised the VM; the installed CLI was not replaced for the trial.
- Reviewed the source diff and ran `git diff --check`.

## Repeat the operation benchmark

The standalone source is [vm_concat.c](../tests/benchmarks/vm_concat.c). It is
opt-in, not an addition to routine CI. Build each VM into a separate directory
with `make BUILD_DIR=build/concat-trial-baseline build/concat-trial-baseline/vm/panack-vm`
(and the corresponding candidate path). Baseline objects must come from the
baseline revision, before source edits. Link the benchmark with that directory's
VM objects excluding `main.o`, using the same native libraries and flags as the
VM link command. Run the resulting binary with `string`, `bytes`, `append` and
`array`, alternating binaries for five pairs. Do not benchmark alongside builds
or other tests. The benchmark measures the operations directly, without bytecode
dispatch overhead; end-to-end compiler measurements include that overhead.

Existing architecture, source ownership and website claims remain accurate.
This experiment changes allocation mechanics only; broader map/set indexing and
string metadata optimisations are outside its scope.
