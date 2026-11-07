# VM profile reassessment after nine optimisation batches

7 November 2026, current normal checkout VM at source revision 28a70a7.
Read-only profiling; no VM implementation, rebuild, full validation or CI.

## Collection

macOS sample, 1 ms interval, requested 20 seconds on one compiler compilation;
the process completed during collection. The main thread supplied 14,737 samples.
Three separate stdlib-import compilations supplied 145, 149 and 144 samples.
The stdlib workload is too brief for precise ranking; repeat samples provide
only a qualitative cross-check. Sampling is statistical, not allocation counting
or a controlled timing comparison. This does not predict percentage speedups.

Both workloads used the existing saved compiler and normal checkout VM. Their
output hashes remained 9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d
(compiler) and 868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686
(stdlib imports).

Raw local reports: /tmp/reassessment-compiler-profile.txt and
/tmp/reassessment-stdlib-profile-{1,2,3}.txt. These are temporary local evidence,
not portable repository dependencies. Scratch input was removed after profiling.

## Findings

- The constant-construction call subtree has 1,646 samples (11.17%), dominated by
  value_data allocation/copying, with smaller numeric construction costs. The
  sampled return address was checked against disassembly: it follows the call
  to constant_value. This includes descendants, not only that function's own
  instructions. Later release costs are elsewhere in the profile.
- value_equal itself has 1,834 top-of-stack samples (12.44%), with substantial
  callers in collection lookup/update paths. This does not establish which
  collection algorithm or threshold should change; updates still scan entries.
- strcmp and its two named stubs total 1,323 top-of-stack samples (8.98%).
  Callers include internal collection/text builtin dispatch as well as other
  runtime comparisons. Our previous classification removed execution-level
  special-case routing comparisons, not all implementation-family dispatch.
- Allocation/free, release, operand stack operations and the general execution
  loop remain visible. These categories span multiple operations and must not
  be added to the inclusive constant subtree as independent costs.
- The short stdlib samples also show allocation/free, execution and name
  comparison costs; they do not support precise percentages or a separate rank.

## Recommendation

Keep immutable literal reuse as the next proposed trial, measuring retained
memory and cleanup as well as timing. It has a directly identified repeated
construction cost without requiring a collection representation change.
Internal collection/text builtin dispatch is a strong alternative with lower
retained-memory risk. Collection lookup/update optimisation deserves targeted
measurement before choosing an algorithm. Frame-buffer reuse remains a later
candidate because its ownership and retention trade-offs are more involved.

No implementation is started by this reassessment. All nine adopted optimisation
batches retain their prior validation status. No website or public API changes.
