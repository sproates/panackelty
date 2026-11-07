# VM string metadata reuse trial

7 November 2026. Local experiment on `feature/rm162-real-program-fixes`, against
`d633489`, with all three previous VM optimisations already adopted.

## Change and scope

String addition copies bytes into final storage and combines the input strings'
cached metadata: add Unicode code-point counts and AND the ASCII flags. It no
longer scans the result to calculate information already known about its inputs.
This is code-point counting, not grapheme counting or Unicode normalisation.

Raw-byte string constructors still calculate metadata by scanning. Both paths
share the allocation/copy helper, including overflow checks and a trailing NUL.
The specialised constructor borrows valid string values, never modifies inputs,
and returns an independent owned result. Allocation count, value layout and
bytecode format are unchanged. No additional cache or retained storage is added.
String interpolation and other raw-byte producers retain their existing path.

This is a candidate trial, not adoption in the normal checkout VM. No CI, PR,
merge, installed-release change or website change. Full product validation and
normal-VM rebuilding remain for adoption after trial acceptance.

## Correctness

- Native module suite passed.
- 49 input pairs compare string addition against the existing scan: empty,
  ASCII, accented letters, Greek, emoji, combining characters and embedded NUL.
  Repeated concatenation checks metadata propagation and same-value operands;
  byte contents, trailing NUL, input reference counts and result independence
  are asserted.
- Native fault suite passed: 2,094 injected allocation failures, including both
  allocations in the cached-metadata constructor and unchanged input ownership.
- `collections`, `owned_intrinsics`, `callables`, `records_and_enums` and
  `async_await` compiled and ran saved bytecode with exact expected stdout.
- Source review found metadata writes in the constructors only; ordinary string
  values are not mutated behind the cached character counts or ASCII flags.

## Measurement method

Separate baseline and candidate native VMs use identical `cc -O2` flags. Five
stdlib-compilation pairs and three compiler-compilation pairs alternate execution
order, comparing output hashes. Compilation includes decoding, verification and
output writing. No builds or other tests run alongside the measurements.

The opt-in [concatenation benchmark](../tests/benchmarks/vm_concat.c) additionally
accepts a byte length as its second argument. For string addition, it checks
length, code-point count and ASCII metadata while allocating and releasing
200,000 results. Compare 8-, 32-, 128- and 4,096-byte operands with five alternating
pairs each. Link the same benchmark source against each VM's objects excluding
`main.o`, using the VM's native link libraries. This isolates the operation;
compiler measurements establish whether the improvement matters to that workload.

The website and language/API contract are unaffected. This trial contributes to
RM#157: Performance review without completing the wider review.

## Results

| Compilation workload | Pairs | Baseline median | Candidate median | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| 28 stdlib imports | 5 | 0.298563s | 0.294447s | 1.38% |
| Compiler source | 3 | 24.475423s | 23.741077s | 3.00% |

These small compiler differences do not establish a substantial build-time gain.
All outputs matched: stdlib SHA-256
`868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686`,
and compiler/bootstrap SHA-256
`9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.

| Bytes per string operand | Baseline median | Candidate median | Time reduction |
| ---: | ---: | ---: | ---: |
| 8 | 0.008284s | 0.007451s | 10.06% |
| 32 | 0.010939s | 0.007103s | 35.07% |
| 128 | 0.025548s | 0.006980s | 72.68% |
| 4096 | 0.433610s | 0.025690s | 94.08% |

Each operation timing covers 200,000 concatenations with five alternating pairs;
all output checksums matched. Short-string timings are only milliseconds and
especially sensitive to scheduling noise. The large-string result demonstrates
removing the extra scan; it is not a claim that complete programs become 94%
faster. Copying the actual bytes remains necessary and scales with result size.

Recommendation: retain this bounded candidate for adoption, primarily for its
string-operation improvement. It adds no per-value metadata or retained memory.
The normal checkout VM remains on the three previously adopted optimisations;
full `make check` remains pending adoption approval. No CI was triggered.
