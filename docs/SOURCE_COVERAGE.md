# Source coverage collection

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
The single-execution reporter rejects gap/overflow records. Use the SC4 session
commands below to register and aggregate supported contexts and multiple roots. SC5 owns suite baselines and publication; SC6 owns initial gap closure.

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

## Complete execution sessions (SC4 / RM#139)

On native Linux and macOS, `coverage-session` registers an execution tree. The
first artifact/inventory pair is the root; subsequent pairs declare every other
eligible artifact that nested VMs or subprocesses may execute. Every inventory
is a single-root SC2 inventory. Generate them with the same installed compiler.

```sh
./panack compile parent.panack -o parent.bc
./panack inventory parent.panack -o parent.pinv
./panack compile child.panack -o child.bc
./panack inventory child.panack -o child.pinv
./panack coverage-session run-1 parent.bc parent.pinv child.bc child.pinv -- child.bc
./panack coverage-session run-2 parent.bc parent.pinv child.bc child.pinv -- child.bc
./panack coverage-aggregate --source parent.panack parent.pinv \
  --source child.panack child.pinv --session run-1 --session run-2
```

The directory must not exist. Arguments after `--` go to the root program.
Repeated sessions must have exactly the same ordered identity registry. The
report reproduces **every** registered source/artifact/inventory locally before
reading observations. It adds counters per artifact, then derives line reach
from the resulting source observations. Different artifact roots remain separate
report groups; shared imported sources are not a combined suite denominator.
Unused registered roots retain emitted zero counts, exclusions and not-emitted
states. Unavailable source associations stay unavailable. No suite percentage is
published; that remains SC5.

Each admitted execution has a unique hierarchical ID. Admission writes an
expected ticket before execution; the collector exclusively claims it and later
writes a record bound to the session's random 128-bit identity and execution ID.
Parent records declare their child counts. A locked shared budget records total
admissions and reserved record bytes. Aggregation checks the whole tree, exact
file membership, budget, identities, terminal records and counter invariants.
It never deduplicates silently or treats failed collection as zero.

- Nested `run_bytecode` / `run_bytecode_args`, including compiler invocations,
  match exact artifact bytes against the registry. Unknown artifacts still run
  in-process with ordinary semantics, but make collection unavailable.
- Native task sessions and TCP server handlers own separate collectors. Reused
  server slots get new IDs. Async suspension/resumption stays inside the same
  execution and does not add entries or duplicate instruction completion.
- `process_run` reserves a child ticket and supplies the internal
  `PANACK_COVERAGE_TICKET` environment value. A compatible `panack-vm` process,
  including one reached through the `panack` exec launcher, must claim it. The
  artifact must be registered. Register the compiler artifact/inventory too when
  a subprocess invokes a compiler command. An unregistered native subprocess
  fails registration explicitly; it does not silently run without collection.
- Shells/tools that do not participate, multiple VM processes launched behind one
  ticket, `check`-only subprocesses, detached descendants and external runtimes
  cannot establish complete collection. Their missing or duplicate claim blocks
  a complete report. There is no implicit exclusion for an external process.
- Traps with a terminal record retain attempted prefixes. Cancellation, native
  `process_exit`, crashes, timeouts and killed collectors can leave partial
  records or missing root closure. These block a complete report. SC3 raw reports
  remain available for isolated executions; they cannot substitute for tickets.

`PANACK_COVERAGE_ACTIVE=1` is exposed by session VMs to harness code, including
nested VMs. The fixture runner rejects `PANACK_TEST_RUNNER_REPORT` transcript
reuse while this marker is active. Clear transcript reuse when collecting a
fresh runner execution. Compilation caches may reuse exact bytecode, but every
execution still needs a fresh ticket. Copying a prior raw record into a new
session fails its session identity; listing the same session twice also fails.
This is local evidence, not cryptographic authentication against deliberate
rewriting. Session directories must remain private and unchanged during reading.

Bounds: 16 distinct artifact identities, 16 MiB registry, 4,096 executions across
the entire process tree, 256 MiB reserved record data per session, 16 MiB per
wrapped record and IDs shorter than 192 bytes. The reporter accepts at most 256
explicit sessions. Budget exhaustion or allocation/I/O failure leaves collection
unavailable. Normal execution without a session allocates no registry or files.
Session collection is not supported on WASI; the single-execution collector API
is unchanged. Native embedding callers keep the parent run alive until its task
or server contexts have been destroyed, and close each owned run exactly once.

### Session directory format, version 1

Integers are unsigned big-endian u32 unless the embedded SC3 record specifies
otherwise. Names are derived locally from validated numeric execution IDs.

| File | Contents |
|---|---|
| `registry` | `PANACKSESSION1\n`, 16-byte random identity, root count, then length-prefixed exact artifact and inventory bytes for each root |
| `budget` | admitted execution count, reserved wrapped-record bytes |
| `expect-ID` | `root\n`, `nested\n`, `task\n`, `server\n` or `process\n` |
| `claim-ID` | `claimed\n`, exclusively created once |
| `raw-ID` | `PANACKEXEC1\n`, session identity, length-prefixed ID, artifact index, declared child count, complete SC3 raw record |
| `closed` | `closed\n`, written only after root teardown and record writing |

The root ID is `0`; children are consecutively numbered `PARENT.1` onward.
Missing any expected member, deleting an entire child, adding surplus records,
copying records between sessions, overflow, inconsistent budgets, mixed registry
identities and partial terminal states all prevent a complete report. Reporting
is read-only and emits no source rows on validation failure.
