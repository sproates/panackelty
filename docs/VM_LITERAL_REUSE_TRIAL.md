# VM execution-scoped literal reuse trial

7 November 2026. Trial against d46d367 on feature/rm162-real-program-fixes,
with the previous nine optimisation batches adopted.

## Design and lifetime

Successful verification assigns a slot to each Nat, Int, Decimal and string
constant instruction. The index reuses the instruction's existing derived-data
union. Each VMExecution lazily allocates a zeroed pointer table on its first
eligible constant. Each literal Value is constructed only on first use of that
instruction and retained for subsequent uses in the same execution. Equal
literals in different instructions are not deduplicated.

The cache owns one reference; each executed constant instruction produces another.
Synchronous completion and resumable destruction release the cache, including
cancellation and error paths. Returned values outlive cache cleanup through their
own references. Separate executions do not share mutable cache state. Suspended
executions keep their cache until destroyed. A long-lived execution can therefore
retain all literals it has touched until it ends.

Verification disables old cache metadata before any work and publishes it only
on success. Re-verification assigns fresh slots. The existing contract forbids
mutating/re-verifying a Program while executions use it. Unverified native test
programs keep the original construction path. Boolean/Unit/Void sharing is
unchanged. Failed table or literal allocations produce the existing OOM trap.
No wire-format change or new opcode is needed.

## Validation

- Strict-warning candidate VM and native builds passed.
- Full native suite passed. New tests compare cached and uncached results for
  numeric literals, Decimal negative zero/scale, empty strings, Unicode and
  embedded NUL strings. Repeated calls reuse an instruction's literal; separate
  executions produce independent caches. Results retain correct ownership after
  cleanup. Tests also cover partial-execution cancellation and re-verification,
  including failure invalidating the cache metadata.
- Allocation-failure sweeps cover pointer table, string and all three numeric
  construction paths, completion and cancellation. Full fault suite passed
  2,096 injected failures with no leaks.
- Five compiled/saved-bytecode fixtures passed exact expected output:
  collections, owned_intrinsics, callables, records_and_enums and async_await.
- All timed and memory-measured compiler outputs matched the baseline hash.

## Timing

Five stdlib pairs and three compiler pairs, alternating order with identical
saved compiler and sources. Wall time includes startup, decode/verification,
compilation and output. No concurrent builds/tests or memory measurement.

| Workload | Baseline median | Candidate median | Less time |
| --- | ---: | ---: | ---: |
| Stdlib imports | 0.231796 s | 0.199718 s | 13.84% |
| Compiler | 17.436340 s | 13.929345 s | 20.11% |

Compiler pairs (baseline / candidate, seconds):
17.436340 / 13.929345; 17.365631 / 13.736196; 17.530950 / 14.022768.
These local samples favour the candidate consistently, but are not a portable
speed guarantee or a measurement of complete validation throughput.

Output hashes:

- Stdlib: 868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686.
- Compiler: 9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d.

## Memory and limits

On arm64, Instruction remains 120 bytes and Value remains 64 bytes. Program grows
from 24 to 32 bytes; execution state gains a pointer and count (16 bytes).

The saved compiler has 8,269 eligible literal instructions: 5,682 strings and
2,587 numeric literals. Its per-execution table is 66,152 bytes. Constructing all
eligible literals would hold approximately 616,256 bytes of Value objects and
payload, excluding allocator overhead. Only touched literals are constructed,
but the full pointer table is allocated even if a small portion of the program
runs. Concurrent executions each pay their own cache cost; it is not globally
shared or bounded by an additional eviction policy.

One separate /usr/bin/time -l compiler pair measured peak resident memory:
378,798,080 bytes baseline and 377,995,264 bytes candidate (both about 361 MiB).
This shows no material peak increase in that workload; the small reduction is
not evidence of a memory saving. It does not establish memory behavior for a
large literal-heavy program or many concurrent short handlers. Those remain
important limits when generalising this trial's performance result.

## Status

Adopted in the normal checkout VM after owner approval, with the retention
trade-off above explicit. make native passed, followed by full local make check
(exit 0, 593 seconds): unit, functional, bootstrap, packaging, release smoke and
quick-start checks passed. Bootstrap completed in 122 seconds and compiler
bytecode remained unchanged. The total was 22 seconds below the preceding
615-second run; this is an observed run-to-run comparison, not a controlled
whole-suite benchmark. Historical timing-budget warnings remain advisory.

No push, PR or CI run. The stable globally installed release is unchanged.
Ownership/component documentation and release/coverage records are updated.
No website, language or CLI update is needed because observable semantics and
bytecode are unchanged.
