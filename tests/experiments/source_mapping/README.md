# Native source-map acceptance evidence

Run `make source-mapping-experiment` with the native toolchain and Node 24+.
The target compiles `trap_probe.c`, which observes the actual VM dispatcher,
and tests the production `compile --source-map` and `locate` commands. It runs
in the Linux/macOS compiler CI jobs, separately from interpreter-free native
`make check`. Canonical self-hosted unit and CLI tests cover the production
contract without Node.

## Independently checked attribution

The observer records the function and PC immediately before advancing one real
VM instruction. On a trap, it retains the function identity owned by the Program;
execution frames may already have been cleared. It uses the actual decoder,
verifier and dispatcher, with no alternative interpreter or production trace ABI.

Literal test expectations cover local, imported, erased generic, nested,
conditional and callback indexes. A Unicode prefix establishes code-point rather
than UTF-8 offsets. Binding initialisers and earlier indexes retain their own
range instead of borrowing a later tail expression. Source execution and saved
bytecode produce the same trap, and mapped artifacts match ordinary compilation.

The test constructs malformed maps and coherent lies about instruction indices,
source indices, paths, ranges, line/column coordinates, lowering flags and counts.
Duplicate records and valid-range changes are rejected by local reproduction.
Missing/stale/oversized/invalid-UTF-8 inputs, wrong artifacts, implicit-core
changes, unknown functions and unmapped PCs all return unavailable. Unchanged
explicit-source symlinks are supported; unrelated neighbouring files are outside
the exact loaded closure. Repeated and relocated builds retain identical pairs.

## Decision and trust boundary

U1 selected an optional sidecar to preserve executable v9 bytes. An appended
metadata candidate is still tested and rejected by the existing v9 verifier;
embedding metadata would need an explicit format migration with a demonstrated
benefit. No such migration is part of U2.

The U1 checksum envelope exposed a coherent-forgery counterexample: checksums do
not authenticate source attribution. U2 replaces that envelope with exact local
reproduction of bytecode, source snapshots and emitter entries. Production never
parses foreign map fields. The full [source-map contract](../../../docs/SOURCE_MAPS.md)
records the trust boundary, disclosure of complete source text, closure limits,
lookup cost and filesystem assumptions. The test-only JSON implementation and
compiler adapter have been removed; the JavaScript decoder in `run.cjs` exists
solely to construct adversarial mutations of known-good output.

## Measurements and limits

The current runner reports mapped/ordinary compilation samples, one validated
lookup per timing sample, and artifact/sidecar sizes. These include process
startup, source loading and compiler work. The
[validation profile](../../VALIDATION_PROFILE.md) retains historical U1 runtime
bulk/single-step and JavaScript serialisation measurements, but the current
target does not reproduce those retired implementation timings. There is no
production runtime tracing in this delivery.

The self-hosted compiler is also measured as a larger, multi-module corpus.
These observations establish practical costs for this foundation; they do not
satisfy the whole programme's realistic-program usefulness gate. Runtime
retention, checker explanations and positive non-impact evidence remain open.

## Follow-on scope estimates

| Delivery | First useful outcome | Provisional size / PRs |
| --- | --- | --- |
| U2 production mapping | Implemented across frontend, emission and validated sidecar deliveries; acceptance on final delivery merge | 3 PRs |
| #134, U3 | First guarded-subtraction explanation from retained checker facts and source origins | Medium / 1–2; full types/effects/proof scope still unestimated |
| #173, U4 | Connect mapped emission with checking/lowering reasons and explicit generated origins | Medium–large / 2–3 after U2; mapping alone is insufficient |
| #174, U5 | One bounded inferred requirement validated by actual recompilation | Medium / 1–2 after U3; general solver excluded |
| #175, U6 | Bounded dependency change with positive unaffected evidence | Large / 2–4 after U0 dependency investigation |
| #172, U7 | One opt-in computation derivation with bounded retention | Large / 2–4 after U0 retention investigation |
| #136 / #131 | First source-aware trap / bounded source-coverage consumer | Each medium / 1–2 after U2; separate unscheduled work |

These are planning estimates rather than whole-workstream completion claims.
Agree representative workloads and budgets before the later feature acceptance
and programme-wide evaluation.
