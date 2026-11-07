# Compiler preparation and incremental reuse

Assessment: 7 November 2026, against core commit `a7a939a8`.
Related work: [RM#28: Incremental and modular builds](roadmap/records/independent-website-publishing.md#rm-28)
and [RM#157: Performance review](roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-157).

The owner wants to avoid repeated compilation before undertaking larger language
changes. This record defines the first implementation slice, its delivery
evidence and subsequent reuse boundaries. The second slice is an opt-in
[unchanged-compilation trial](COMPILER_UNCHANGED_REUSE_TRIAL.md); module-level
body checking after edits is now validated through an explicit
[in-memory checked-module snapshot](COMPILER_MODULE_REUSE_TRIAL.md).
[One-shot persistent reuse](COMPILER_PERSISTENT_REUSE.md) now makes that boundary
available through `panack compile --incremental` and `panack check --incremental`.
These validated changes are included in this integration into `next`.

## Original path assessed before the first slice

`bin/panack` invokes the hosted compiler. For ordinary source commands,
`driver.prepare_project` calls `loader.load_project`, then
`project_frontend.prepare_project_program`. Manifest commands resolve packages
first and use the corresponding dependency-aware preparation path.

The loader reads and parses the reachable modules, builds their binding graph,
checks signatures and guards, checks function bodies, and checks effects across
the collected bodies. For a valid project it calls `module_emission_program`
and stores the resulting executable `Program` in `LoadedProject.program`.

The frontend then builds a binding graph and calls `module_emission_program`
again. This is a repeated conversion of checked declarations to executable
declarations, not evidence that every type check runs twice. Finally the bytecode
emitter walks the program's functions. Even ordinary `check` currently invokes
`compile_program` before reporting success.

Relevant implementation:

- [Loader](../src/compiler/loader.panack): `load_project_mode_with_configuration`.
- [Frontend](../src/compiler/project_frontend.panack): both preparation functions.
- [Driver](../src/compiler/driver.panack): source and resolved-project commands.
- [Module emission](../src/compiler/module_emission.panack): conversion and symbol naming.
- [Body checking](../src/compiler/module_bodies.panack): `BoundFunctionBody` retains checked bodies and evidence.
- [Effect checking](../src/compiler/module_effects.panack): `module_local_functions` computes permissions across bodies.

There are already caches outside this ordinary compiler path:
`tests/run_probe.sh` reuses compiled test artifacts using input fingerprints;
`scripts/source_coverage_cache.cjs` reuses authenticated coverage preparations.
Neither is general incremental compiler checking. Preserve their behaviour;
do not replace them with another test cache under this work.

## Three edit cases

| Case | Current ordinary compiler path | Required eventual behaviour |
| --- | --- | --- |
| No changes | Loads, checks and emits the project again | Reuse a successful result after validating all relevant inputs; execute programs afresh |
| Function implementation changes | Rechecks and emits the loaded project | Recheck the changed body and any consumers of changed semantic facts; reuse unaffected work |
| Signature, type or constraint changes | Rechecks and emits the loaded project | Invalidate affected consumers transitively, including proofs, effects and explanation evidence |

An unchanged signature is insufficient proof that callers are unaffected.
`module_local_functions` derives some effect permissions from bodies and their
calls. Guard definitions also participate in checking. An initial module cache
must conservatively invalidate dependants on any module change until more precise
dependency tracking is proved correct.

Emitted names incorporate module graph indexes. Persisting final per-module
bytecode without accounting for graph ordering would be unsafe. Prefer checked
module results before global naming; retain whole-program emission initially.
Source positions and evidence mode are inputs too: moving text must not reuse
old locations, and ordinary compilation must not masquerade as an explanation
result containing retained evidence.

## First implementation slice: prepare a project once

Status: implemented, validated locally and included in this integration into `next`.

Make the loader's successful prepared program reusable within the same command.
Retain an explicit association with the preparation context: entry identity,
project and standard-library roots, package identity/dependencies, and mode.
The frontend may return that program only when the requested context matches
and the identity/effect checks and diagnostics permit success. An unprepared,
invalid or differently contextualised project must retain the current error or
fresh-preparation path. Do not simply return `loaded.program`: on failure that
field may contain the original assembled declarations.

Implementation stages:

1. Add focused counting/timing observations to establish duplicate lowering for
   source and manifest commands. Include compiler self-compilation and a small
   multi-module example. Keep timing out of ordinary output.
2. Represent successful preparation explicitly using existing compiler types
   where possible. Reuse it through the frontend. Audit inventory, coverage,
   source-map and explanation consumers for context differences.
3. Prove equivalence and measure the same workloads with candidate and baseline.
   Remove temporary instrumentation or retain only a deliberate opt-in facility.

Acceptance:

- A matching successful project lowers once, rather than twice, per preparation.
- Baseline and candidate produce identical bytecode; diagnostics, exit status,
  source maps, inventories and retained explanations remain equivalent.
- Invalid programs, missing entry functions, package confinement failures and
  alternate preparation contexts cannot acquire a false successful result.
- Source edits between separate invocations remain visible: this slice has no
  persistent cache and no invalidation shortcut.
- Run affected loader/frontend/tooling tests during development, then one final
  required full local check after batching changes. Do not start development CI.
- Report measured time and memory effects without promising a percentage. If
  this conversion is cheap, say so; eliminating it does not solve total build time.

No bytecode-format change, VM change, daemon, disk cache or language change is
needed for this slice. Removing bytecode emission from `check` is a separate
candidate, not an unexamined part of this change. Expected delivery: one product PR.

### Implementation and observed results

`LoadedProject.preparation` records the context only after successful lowering.
The frontend reuses `program` when that context matches, identity and effect
checking succeeded, and no non-staging diagnostics remain. Otherwise it retains
the previous preparation path. Loading mode and evidence stay with the immutable
loaded project; there is no cross-invocation or cross-mode cache. Root spellings
must match exactly; a harmless spelling difference can miss reuse, not broaden it.

The existing test-only compiler observer, without new instrumentation, recorded
two `module_emission_program` calls for the baseline and one for the candidate
on both compiler self-compilation and the resolved HTTP-server package project.
Both pairs emitted identical bytecode. Sentinel-based unit tests independently
distinguish reuse from lowering and exercise every context field, incomplete
checks, diagnostic precedence, missing input and missing main, as well as plain,
explained and inventory loading. This replaces the proposed new counter code.

Alternating baseline/candidate timing pairs used the same normal VM and source
tree. This task ran no builds or profiling alongside those timings. Baseline compiler is the saved
alpha.15 seed at the start of this slice; candidate contains this slice.

| Workload | Pairs | Baseline median | Candidate median | Less time |
| --- | ---: | ---: | ---: | ---: |
| Stdlib example | 5 | 0.080832 s | 0.078079 s | 3.41% |
| Compiler self-compilation | 3 | 14.205127 s | 13.777691 s | 3.01% |

Compiler pairs, baseline/candidate seconds: 14.205127/14.185008,
14.323235/13.777018, 14.026048/13.777691. This is a modest local improvement,
not a portable guarantee or a full-validation speed claim. Timed compiler output
matched SHA-256 `1c6aada26cef66b2f5c23bc918b73226cff4ee2551a2cf7710c0eb48a03c8a35`
in every run; stdlib output matched
`823a5f93fe2886a44b680cb09c1d634bdf6aa8fc0f5cdff57510cfe743eb8a0d`.

A separate macOS `/usr/bin/time -l` pair observed peak resident memory of
385,220,608 bytes baseline and 352,616,448 bytes candidate. That is about 31 MiB
less in this pair; it is not a repeated memory benchmark.

Differential checks on the multi-module indexed-lookup fixture matched successful
check, failed check (missing main), run and explain status/stdout/stderr, plus
compiled bytecode and source-map bytes. Inventory payloads matched after checking
and excluding their intentionally different embedded compiler bytes. Existing
inventory tests continue to cover the full compiler-identity contract.

All 35 focused preparation tests passed. Native oracle artifact contracts passed
with the candidate. Bootstrap regeneration reached the same compiler hash through
stages 2, 3 and 4 and preserved the stdlib artifact. The refreshed seed and digest
are included; bytecode remains version 9.

Final local `make check` passed (exit 0, 518 seconds), including unit, functional,
bootstrap, packaging, relocated release smoke and quick-start checks. The existing
120-second aggregate timing warning is advisory. This run duration is not a
controlled comparison with earlier full-validation runs.

The first full-check attempt stopped in packaged HTTP smoke because the restricted
execution environment could not allocate a localhost port. The same port allocator
and isolated smoke passed with host access; the complete check then passed with
that access. No product or test weakening was needed for this environment failure.

Local scratch evidence is in `build/compiler-preparation-reuse/`. It includes
timings, observer traces, comparison scripts and validation logs; these are not
new product dependencies. The owner subsequently requested integration of the compiler-reuse batch into
`next`, with CI starting on integration. The globally installed toolchain is unchanged.

## Subsequent slices

Slice 1 below is now implemented as the linked opt-in trial. Fresh package-tool
validation precedes the compiler: the compiler fingerprints the resolved
descriptor, rather than interpreting raw manifests or lockfiles itself. Fresh
source discovery resolves the current reachable graph before every lookup.
The checked-module stage has a validated in-memory body-reuse boundary, detailed
in the linked trial. The subsequent persistent implementation serves ordinary
one-shot CLI invocations. Narrower invalidation remains the next implementation
step, followed by a separate hosted cache-reuse trial.

1. **Reuse unchanged successful compilations across invocations.** Start with
   ordinary check/compile/run and a bounded local cache, not serialized checker
   internals. Reuse bytecode, never execution outcomes. Key on exact source bytes,
   resolution inputs and relevant directory membership, toolchain identity,
   options, roots and package descriptor/lock inputs. Validate current package
   confinement before accepting a hit. Do not rely on timestamps alone. Recheck
   inputs before publication; partial, corrupt or unavailable entries fall back
   to fresh work. Keep tooling modes fresh until their complete outputs can be
   replayed correctly. Design the trusted storage/publication boundary before
   implementing it, drawing on the existing cache tests without importing a
   Node dependency into the product.
2. **Reuse unchanged checked modules after an edit.** Persist parsed/checked
   results only after defining their versioned representation and semantic
   dependencies. Begin with conservative transitive module invalidation;
   recompute global effect closure and final naming/emission. Test additions,
   removals, import changes, cycles, guard/effect changes, and source relocation.
3. **Narrow invalidation to declarations/functions where measurements justify it.**
   Track actual signature, guard and effect dependencies. Prove that a private
   body edit skips unaffected work without stale diagnostics or evidence.

Each cache slice needs a fresh-build comparison matrix, observed reuse counts,
cold and warm timings, an explicit bypass, and failure/corruption cases. Measure
ordinary local compiler use first; hosted cache persistence is not a prerequisite
for establishing a useful local compiler improvement. These later slices need
their detailed contracts before implementation; this assessment does not claim
the entire incremental compiler is ready to build or estimate its total effort.
