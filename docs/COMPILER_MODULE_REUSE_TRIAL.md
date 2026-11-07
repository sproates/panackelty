# Checked-module reuse trial

7 November 2026. Implemented and validated locally; integration pending under
[RM#28: Incremental and modular builds](roadmap/records/independent-website-publishing.md#rm-28).
This follows the [whole-compilation cache](COMPILER_UNCHANGED_REUSE_TRIAL.md).

## First boundary: retained compiler values

This stage proves reuse after edits using an explicitly retained, successful
in-memory compiler snapshot. It does not yet persist checked modules, add a
compiler daemon, or speed separate ordinary CLI invocations after edits. The
whole-compilation disk cache keeps its existing behaviour.

`check_project_sources_reusing` takes newly loaded sources and an optional
`ProjectCheckSnapshot` returned by an earlier call in the same compiler process.
It returns the new snapshot and the module identities actually reused/rechecked.
The existing `check_project_sources` entry point supplies no prior snapshot.
Callers must retain compiler-produced snapshots, not manufacture or deserialize
records as trusted checking results. Keeping one snapshot is sufficient; it
retains the prior source graph and checked bodies until the caller releases it.

This avoids inventing an AST/checker serialization format before establishing
correct reuse boundaries. Persistence or a session-serving interface remains a
subsequent delivery, not an implicit part of this trial.

## Invalidation contract

- Source discovery, parsing and confinement run afresh. Never reuse filesystem
  discovery or assume timestamps prove identity.
- Require the same preparation context: entry and root identities, package
  confinement, dependencies and main-function requirement. Failed, partially
  checked and explanation-mode snapshots cannot seed reuse.
- Initially require the same ordered module identities. Added, removed,
  relocated or reordered modules fall back to checking all bodies.
- Compare exact source paths, source bytes and resolved import edges per module.
  Recheck changed modules and their transitive import consumers, even when their
  signatures appear unchanged. Diamonds converge by a fixed-point traversal.
- Core is an implicit dependency. Any core change invalidates all bodies.
  Domain declarations contribute to shared initial checking state; any changed
  domain declaration also invalidates all bodies, conservatively including
  changed source positions in those declarations.
- Recompute every declaration signature and guard. If those checks fail, do not
  reuse bodies during diagnostic recovery.
- Combine reused bodies with freshly checked bodies, then recompute global effect
  closure, final naming/lowering and bytecode emission. Never reuse module-index
  dependent emitted names or earlier success as a whole-project verdict.
- Keep ordinary diagnostics and bytecode identical to a fresh check. Explanation,
  source-map, inventory and coverage entry points continue using fresh checking.

## Acceptance

Compare complete checked results and bytecode with fresh work for cold and warm
checks, leaf and entry edits, transitive/diamond consumers, signature and effect
rejections, guards, core, context changes, module additions/removals, import
changes/cycles, missing sources and recovery from failure. Observe actual body
checking, not only a claimed cache-hit count. Measure a representative edited
compiler project and check ordinary fresh-compilation overhead. Run focused
cases during development and one final local validation after the edits settle.
No development CI is requested. This is not completion of granular incremental
compilation or a claim about hosted cache persistence.

## Local evidence

All 54 focused comparisons passed. They compare complete checked results, not
only success/failure; successful cases also compare serialized bytecode. The
suite is registered with the normal compiler module-bindings tests.

Three alternating fresh/reuse pairs used an isolated copy of the compiler's
47-module source graph. Each process first obtained a successful snapshot, then
changed the entry module's body to add a print after the compiler call. Timing
starts with loading the edited project and includes checking and bytecode
emission/serialization; initial snapshot construction is outside that interval.
Neither timings nor the isolated edit change the installed compiler.

| Edited compiler build | Median | Body modules reused | Body modules rechecked |
| --- | ---: | ---: | ---: |
| Fresh | 15.480125 s | 0 | 47 |
| Retained snapshot | 8.269575 s | 46 | 1 |

That is 46.58% less time for this entry edit, not a prediction for every edit.
Changes with more transitive consumers will reuse less. Reuse times were
8.269575, 7.997960 and 8.366689 seconds. Every edited artifact matched SHA-256
`ff5b38c81b78d4bc77c78b6afd3988746b24d464dc969ae0f2a304724e905887`.

A separate three-pair ordinary fresh self-compilation comparison measured
14.345221 s baseline and 14.350636 s candidate medians, a 0.04% difference.
There is no meaningful fresh-build regression demonstrated by this small sample.
Both compilers emitted identical bytecode. No other build or profiling ran
alongside these measurements. Peak memory has not been benchmarked: retained
snapshots keep the previous parsed graph and checked bodies alive, so this trial
does not claim free or reduced memory use.

The test-only observer recorded 48 body-check calls: 47 for the initial snapshot
and one for the edited entry module. It also recorded 94 signature-check calls
and two module lowerings, confirming those phases still run for both checks.
In this single observer run, fresh signature checks took about 3.92s and
lexing/parsing about 2.02s. Body checking of the changed entry took under 1ms;
global effect checks and emission/serialization still ran. These inclusive
phase observations are not a second timing benchmark. Signature reuse is a
measured follow-up candidate, requiring its own invalidation/diagnostic tests.
Native oracle contracts passed. Seed regeneration converged across stages 2–4
at SHA-256 `b0cdc321d2e5f2b5401bd4c78d0f0b74ffd3cc4708f777ac76dbea584b88aea7`;
the stdlib artifact remained unchanged. Full local `make check` passed (exit 0,
563 seconds), including unit, functional, bootstrap, relocated release smoke
and quick-start checks. The legacy 120-second aggregate warning is advisory;
this run is not a controlled full-suite speed comparison. Documentation checks
and self-review passed. No CI or public publication was started.

Local scratch
evidence lives in `build/compiler-module-reuse/`. No website changes are needed:
this is an internal compiler-library trial, not a new published CLI capability.
