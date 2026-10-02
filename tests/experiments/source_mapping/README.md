# U1: bounded source-mapping experiment

Run `make source-mapping-experiment` from the repository root with a C toolchain,
Node 24 or later, and the existing compiler seed. The target is separate from
`make check` and interpreter-free validation. Check runs it after the existing
isolated compiler suite on Linux and macOS. The original U1 delivery changed no
production source, CLI or compiler seed.
U2 now supplies retained frontend spans and a refreshed v9 seed; this test target
still adds no production tracing ABI or bytecode-format extension.

## Question and result

Can an actual failing VM instruction be attributed to an exact source range in
local, imported and generic code without changing ordinary execution?

Yes, for the deliberately bounded function-tail expression `items[index]`.
`compiler.panack` uses the real loader, checker, emitter and serializer. It
identifies the loaded declaration and its actual emitted function, checks the
terminal `INDEX_GET` opcode and records its function-local instruction index.
The original U1 frontend retained a receiver position but no complete indexing
range, so the initial probe re-lexed and parsed the expression to recover its end.
The first U2 slice replaces that workaround: the probe now reads the actual
`SourceSpan` retained on the loaded tail expression. Unsupported emission shapes
remain unmapped. This probe is still not a general instruction/source mapping
algorithm and must not be extended by guessing nearby instructions.

`trap_probe.c` compiles the real VM dispatcher into a test executable. It saves
the current function and PC immediately before each single-instruction advance,
because trapping clears the execution frames. The Program still owns the saved
function. Execution uses the same decoder, verifier and dispatcher as the native
VM. There is no alternative interpreter or production tracing ABI.

The test's expected file, function, PC, range, line and column are literal values
specified independently of the mapping code. Imported declarations resolve to
the imported file. A generic function is called with both Nat and Str before the
Str call traps: attribution identifies the shared erased body, not a distinct
specialisation or type-argument trace. A Unicode prefix checks code-point rather
than UTF-8 byte offsets. Generated instructions and unsupported expressions have
no attribution. An earlier indexing trap cannot borrow its function's mapped
tail range. The public CLI reproduces the same trap from source and saved bytes.

## Representation decision

Proceed towards U2 with an optional, deterministic sidecar, subject to agreeing
its production scope. It preserves the current v9 executable bytes and verifier,
can be omitted without changing execution, and can be validated separately.
The experiment emits identical bytecode to the public CLI and deterministic
relative-path metadata across repeated builds and relocated source roots.

The competing embedded candidate appends a marker and the same metadata to v9.
The existing decoder correctly rejects it. This is a compatibility/size probe,
not an implemented new bytecode version: an embedded representation needs an
explicit format version, loader/verifier contract, seed migration and tests.
Embedding would bind transport of code and map together, but still would not
prove the map's source claims. No measured benefit justifies that migration for
this first consumer. Retain the embedded option if future consumers require it.

The experimental envelope records a schema version, offset encoding, producer
bytecode digest, executable digest, source hashes, entries and an integrity
checksum. Lookups require a matching instruction boundary/opcode, valid bounded
ranges, unique keys and unchanged source snapshots. Relative paths reject
traversal and symlinks. Missing, oversized, malformed, stale or mismatched maps
return unavailable. The prototype hashes the entire declared fixture corpus,
not the loader's exact dependency closure; this deliberately over-invalidates.
It contains no absolute paths or source text, but relative filenames and hashes
can still reveal information. It is not a privacy guarantee.

**Integrity is not authenticity.** The envelope checksum catches accidental
valid-range edits, but a malicious producer can rewrite a range and recompute
it. An executable counterexample preserves that finding. Trust must come from
the producer/artifact channel, not a self-asserted compiler hash. U2 must define
that boundary, capture the exact loaded source snapshots (including libraries),
and address read/compile/lookup races. These fixtures do not establish safety
for arbitrary untrusted metadata, concurrent filesystem mutation or adversarial
resource exhaustion. No production consumer accepts this format.

## Measurements and limitations

The runner prints every raw timing sample and metadata size. Compilation compares
the same compiler probe with mapping disabled/enabled. Runtime compares bulk and
single-step execution of the same test executable over 1,000 successful calls
followed by a trap; both include process startup. Metadata serialisation and
lookup are measured separately. Disassembly parsing and building the probe are
outside those timing samples. These small warm measurements are feasibility
observations, not production overhead budgets or performance guarantees.
See [the recorded run](../../VALIDATION_PROFILE.md#u1-source-mapping-experiment-2026-10-02).
U1 introduced no production overhead because it changed no production sources
or artifacts; that small benchmark did not prove tracing was free. The U2 span
retention costs are measured separately in the validation profile.

U1 does not establish realistic-program usefulness for the whole programme.
It provides a tested attribution boundary and identifies what U2 must replace:
function/PC identity, dependency closure and trusted metadata distribution.
The first U2 delivery has replaced source-span recovery with retained spans. Runtime retention and positive non-impact evidence for
other workstreams remain open under U0.

## Follow-on scope estimates

| Delivery | First useful outcome | Provisional size / PRs |
| --- | --- | --- |
| U2 production mapping | Retain spans through parsing/lowering/emission; deterministic optional mapping and public-CLI fallback tests; agree producer trust and exact source closure | Medium–large / 2–3 |
| #134, U3 | First guarded-subtraction explanation from retained checker facts and source origins | Medium / 1–2; full types/effects/proof scope still unestimated |
| #173, U4 | Connect mapped emission with checking/lowering reasons and explicit generated origins | Medium–large / 2–3 after U2; mapping alone is insufficient |
| #174, U5 | One bounded inferred requirement validated by actual recompilation | Medium / 1–2 after U3; general solver excluded |
| #175, U6 | Bounded dependency change with positive unaffected evidence | Large / 2–4 after U0 dependency investigation |
| #172, U7 | One opt-in computation derivation with bounded retention | Large / 2–4 after U0 retention investigation |
| #136 / #131 | First source-aware trap / bounded source-coverage consumer | Each medium / 1–2 after U2; separate unscheduled work |

These are revised planning estimates, not scope commitments or whole-workstream
completion estimates. Agree representative workloads and overhead budgets before
production acceptance. The experiment should remain until production tests
replace its decision evidence; do not keep competing metadata implementations
once U2 supersedes it.
