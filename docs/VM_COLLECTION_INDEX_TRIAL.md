# VM collection lookup index trial

7 November 2026. Local experiment on `feature/rm162-real-program-fixes`, against
`e0ca2dd`, with the previous four VM optimisations adopted. The owner requested
continued VM optimisation; async API work is outside this experiment.

## Scope and design

The fresh compiler profile showed collection comparisons, including update paths.
It did not establish repeated lookup as the dominant remaining compiler cost.
This trial targets repeated Map get/has and Set has operations. Map put and Set
add retain their existing implementation and ordering semantics.

An optional index stores positions into the original ordered entries. It uses
open addressing, a power-of-two bucket count of at least twice the entry count,
and equality checks to resolve hash collisions. Map lookup preserves the last
matching binding; Set lookup preserves the first occurrence. Rendering, equality
and iteration still inspect the unchanged ordered entries, not hash buckets.

Supported keys are Str, Bytes, Path, Nat, Int, Duration, Instant, Bool, Unit and
Void. Hashes include the value kind and the bytes/limbs used by existing equality.
Decimal, rational, compound and opaque keys use the original equality scan.
If any stored key is unsupported, that snapshot remains unindexed. In particular,
decimal values with different representations retain their existing equality.

Provisional trial thresholds are at least 32 entries and the eighth lookup.
These avoid index allocation for small or briefly queried values; they are not
language guarantees or proven optimal thresholds. An unsupported key or failed
index allocation permanently leaves that immutable snapshot on the original
scan. Allocation failure cannot turn a successful lookup into a failure.

An update creates a new collection with no index. Existing aliases and snapshots
keep their own contents and optional index. The index owns no values, and is
freed when its collection's final reference is released. This cache follows the
VM's existing single-threaded value/refcount ownership; it adds no thread-safety
claim. Bytecode, public APIs and language semantics are unchanged.

## Validation

- Native module suite and five compiled/saved-bytecode fixtures passed:
  collections, owned_intrinsics, callables, records_and_enums and async_await.
- New native checks cover deliberate collisions (key-8/key-20 in a 128-bucket
  table), equal-but-distinct keys, hits/misses, replacement order, preserved
  snapshots, set growth, equality with an unindexed copy and small collections.
- Additional targeted checks cover duplicate native map keys, strict Nat/Int
  distinction, all positions in a numeric set, equivalent decimal
  representations and compound-key fallback.
- Native fault suite passed with 2,097 injected allocation failures, including
  index allocation failure falling back to correct lookup and complete cleanup.

## Benchmarks

Baseline and candidate built with identical `cc -O2` flags. Pairs alternate
execution order; no builds or tests run alongside timings. Compilation includes
startup, decoding, verification, compilation and output writing. Medians:

| Compile workload | Pairs | Baseline | Candidate | Interpretation |
| --- | ---: | ---: | ---: | --- |
| 28 stdlib imports | 5 | 0.292442s | 0.295104s | No improvement |
| Compiler source | 3 | 21.078342s | 21.113322s | Effectively unchanged |

Every stdlib output matched SHA-256
`868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686`.
Every compiler output matched the bootstrap SHA-256
`9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.

The opt-in [lookup benchmark](../tests/benchmarks/vm_collection_lookup.c) measures
50,000 repeated membership queries, cycling through all keys and one missing
key. Index construction is included. Five alternating pairs per row:

| Collection / entries | Baseline median | Candidate median | Baseline / candidate |
| --- | ---: | ---: | ---: |
| Map / 8 | 0.003486s | 0.003242s | 1.08x |
| Map / 32 | 0.004109s | 0.002091s | 1.97x |
| Map / 256 | 0.016457s | 0.001935s | 8.50x |
| Map / 4096 | 0.219159s | 0.001887s | 116.14x |
| Set / 8 | 0.002408s | 0.002492s | 0.97x |
| Set / 32 | 0.003905s | 0.002126s | 1.84x |
| Set / 256 | 0.016951s | 0.002001s | 8.47x |
| Set / 4096 | 0.206820s | 0.002148s | 96.28x |

One-lookup-per-version workloads use 1,000 freshly constructed 256-entry
collections, including construction and cleanup. Update/read uses 1,000 map
replacements followed by one query. No indexes are built in these cases:

| Workload | Baseline median | Candidate median |
| --- | ---: | ---: |
| Map: fresh value then one query | 0.001041s | 0.001016s |
| Set: fresh value then one query | 0.000615s | 0.000618s |
| Map: replacement then one query | 0.001352s | 0.001343s |

Checksums and per-query results matched throughout. Millisecond/submillisecond
results are particularly noisy; the small-set difference is not evidence of a
material regression. Large lookup gains apply to these repeated-query workloads,
not entire programs, updates, or compiler compilation.

To reproduce, link the same benchmark source with each build's VM objects except
main.o and the normal VM libraries. Run `map|set COUNT hot|cold|update`, using
counts 8, 32, 256 and 4096 for hot mode, and 256 for cold/update modes (update is
Map only). Use separate baseline/candidate build directories and alternate order.

## Memory and status

On arm64, Value remains 64 bytes. Sequence grows from 24 to 40 bytes inside its
existing union, so this does not enlarge each Value allocation. Indexed snapshots
allocate 8 bytes plus 8 bytes per bucket: 520 bytes for 32 entries, 4,104 bytes
for 256, and 65,544 bytes for 4,096. Bucket count rounding can increase overhead;
keeping many independently indexed snapshots retains an index for each.

The owner approved adoption. The normal checkout VM is rebuilt with this change,
and full local `make check` passed on 7 November 2026 (682 seconds, exit 0).
Validation included unit, functional, bootstrap, package/install, release-smoke
and quick-start checks. The bootstrap hash remains unchanged. No CI, PR, merge,
installed-release change or website change. This contributes evidence to RM#157:
Performance review without completing its broader scope.

A separate sequential compiler memory comparison reported maximum resident set
sizes of 380,108,800 bytes (baseline) and 383,238,144 (candidate), about 0.82%
higher. Peak memory footprint was 370,688,648 versus 374,506,144 bytes. Both
outputs matched the bootstrap hash. One pair is evidence of the observed cost,
not a universal bound; many hot retained collections can use substantially more
index memory than this compiler workload.

Adopted locally for repeated scalar-key lookups, with the memory cost explicit.
It does not improve compiler compilation
in this trial. The small/one-off workload results do not show a material cost,
but timing those millisecond workloads more precisely would require longer runs.
The single full validation run is correctness evidence, not a controlled
whole-suite speedup measurement. Historical timing warnings remain advisory.
