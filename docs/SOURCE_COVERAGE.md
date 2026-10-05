# Single-execution source coverage

SC3 / RM#138 adds opt-in observations for one native execution. It uses the
[SC2 inventory](SOURCE_INVENTORY.md), unchanged bytecode v9 and the shared VM
dispatcher. It does not establish a test-suite coverage percentage.

```sh
./panack compile example.panack -o example.bc
./panack inventory example.panack -o example.pinv
./panack coverage-run example.bc example.pinv example.pcov
./panack coverage-report example.panack --inventory example.pinv --coverage example.pcov
```

The raw destination must not exist. Program arguments follow the raw destination.
Collection preserves program output and traps. A trapped execution still produces
a terminal record containing its observed prefix; the report labels terminal `2`
(trapped), compared with `1` (returned). Embedded explicit exit is terminal `3`.
Native `process_exit`, killed processes and crashes can leave incomplete output;
a missing footer is unavailable collection, never zero coverage.

The report reproduces the artifact and **single-root** inventory using local
source and the installed compiler. It compares exact identities before decoding
counters. Compiler, source, artifact or inventory changes invalidate the record.
No paths from a foreign record are opened. The runner treats the inventory as an
opaque identity blob; only the reporter establishes that it matches the artifact.
These are local execution records, not cryptographically authenticated evidence.

## Observations

Tab-separated item rows contain portable source name, SC2 structural ID, kind,
detail, state, attempted count and completed count. Line rows contain source name,
`line/N`, `line`, `anchor`, state and a Boolean reach count. Report order is stable.

- Function attempts count successful frame entry; completion counts returns.
  Unused emitted functions are explicitly observed with zero counts.
- Expression attempts count successful incoming edges from outside the
  compiler-associated expression interval (plus function entry at PC zero).
  An internal loop jumping to the same PC cannot invent repeated outer entries.
  Completion counts successful control-flow edges leaving that interval at its
  end, including jumps. Reaching a shared destination from another arm cannot
  falsely complete an untaken expression. Calls complete only after successful
  return or successful delivery of an asynchronous result.
- Statements use SC2's first-expression anchor. Their completion is completion of
  that anchor, not a claim that an entire loop or enclosing statement returned.
- A line is reached if any observed source expression/statement anchor on its
  starting line was attempted. Multiline spans do not imply every line ran.
  Function headers, punctuation and excluded declarations do not fabricate lines.
- `if` records true/false; `&&` and `||` record evaluate-right/skip-right; loops
  record body/exit; matches record each selected source arm. Failed match tests
  and compiler-generated loops are not additional source decisions.
- Match-decision attempts/completions describe the subject expression. Arm rows
  describe selection, separately from whether the arm body completed.
- Erased generic instantiations share their source identity; recursive entries
  accumulate. Generated method-map/reduce loops preserve original child source
  probes without creating source branches.
- `excluded`, `not-emitted` and `unavailable` are explicit states. In particular,
  pruned core functions are not presented as successfully collected zero hits.
  Missing or ambiguous source associations remain unavailable, including zero-hit
  line rows whose other anchors are unavailable; do not include
  them as zeroes or publish a percentage from this report.

## Collection boundaries and limits

Each verified instruction has four unsigned 64-bit counters: attempted,
completed, conditional fallthrough and conditional branch. Actual branch choice
is recorded even when the two destinations coincide. Function entries are
separate counters. Suspension and zero-budget advances do not redispatch an
instruction; successful asynchronous completion is credited once.

Collection is opt-in with at most 262,144 instruction cells and a 16 MiB raw
record. Counter storage is 32 bytes per instruction plus 8 bytes plus `sizeof(size_t)` per
function and a small collector header; frame bookkeeping adds three machine
words on a 64-bit build. Disabled execution allocates no collector or source
metadata. Allocation failure before execution fails collection explicitly.
Counter overflow saturates and marks the record unusable without changing the
program's semantics. Tests inject a small counter ceiling and allocation faults.

Collectors belong to one execution. Reuse, nested bytecode/process execution and
child task/server contexts mark an observation gap. Child VMs detach inherited
collectors, preventing counters from different contexts corrupting each other.
The reporter rejects gap/overflow records. SC4 will register and aggregate these
contexts, distinguish missing/truncated/killed children and handle multiple
roots. SC5 owns suite baselines and publication; SC6 owns initial gap closure.

## Raw format, version 1

All integers are unsigned big-endian. Layout:

1. ASCII `PANACKCOV1\n` (11 bytes).
2. Artifact length (u32), exact artifact bytes; inventory length (u32), exact
   outer SC2 inventory bytes.
3. Function count (u32), in **serialized artifact order** (sorted function names).
4. Per function: entry count (u64), instruction count (u32), then four u64
   counters per instruction: attempted, completed, fallthrough, branched.
5. ASCII `END\n`, terminal byte (0 incomplete, 1 returned, 2 trapped, 3 exit),
   flags byte (bit 0 observation gap, bit 1 overflow).

The reporter derives the entire expected layout from local compilation before
reading fixed-width counters. It rejects extra/truncated bytes, incorrect counts,
identity mismatch, unknown/partial terminal status, flags, completion greater
than attempts and inconsistent conditional edge totals. This format is a bounded
single-execution contract, not a suite manifest or general trace format.
