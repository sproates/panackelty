# P02: Explicit ownership metadata

GI#525 P02 replaces the bounded PoC's spelling-based method selection with explicit
syntax and canonical ownership. Baseline: `a4d9296d2c2df948d3922ee9b6cd8f9257b2464b`.
This packet becomes complete when its PR is merged after final-head checks.

## Implementation

- `OwnedCallExpr` retains the receiver, method spelling, arguments and type
  arguments. Existing located-expression wrappers retain source and type spans.
- `OwnedMethodSyntax` retains owner and method names independently of the internal
  function name. The existing type spans retain the implicit receiver's owner span.
- Declaration collection creates `OwnedMethodIdentity` using the original
  `DeclarationIdentity`, including package and module. Each `ModuleBindings` has
  an owner/method index separate from ordinary function/import lookup.
- Checked signatures retain the binding, visibility, effects and receiver slot.
  Body checking selects by canonical ownership, binds the receiver once and passes
  that result into the shared argument checker. Checked calls retain the selected
  binding through existing direct-call emission.
- All expression visitors, source inventory and exhaustive test helpers handle
  owned calls explicitly. The project route uses metadata, not string prefixes.
- Diagnostics use `Fruit->apple`; methods are unavailable through ordinary
  function lookup, including attempts to select their internal emission names.

The `$owned$Record$method` function identity remains an internal declaration,
signature and emission bridge, as permitted by P02. It is not a lookup key for
receiver dispatch. The `$owned_call$` marker and its special resolution branch are
gone. This is an explicit temporary bridge, not a claim that P12 cleanup is done.

## Scope and acceptance

Existing dot sugar and field access remain unchanged. Existing limitations also
remain: earlier local nongeneric records, synchronous nongeneric methods, no
foreign implementations. General owner syntax, generic substitution, built-in
methods and async contracts belong to later packets. No HTTP runtime or Map
behaviour changes are included.

Unit checks cover syntax metadata, a separate method index, canonical imported
ownership, private-method rejection, ordinary-lookup exclusion and no cross-owner
fallback. Existing functional programs cover 10/11/20/21, same-name global
functions, two owners, aliases and re-exports. The dispatch fixture additionally
asserts receiver-before-argument order and single evaluation through source and
saved-bytecode execution. Existing negative fixtures retain duplicate, purity,
argument, privacy and public-signature checks with readable method diagnostics.

Focused probes passed: parser 208 assertions and module bindings 857 assertions.
The first full run exposed four diagnostic fixtures still executed by the old
checked-in seed (eight check/compile mismatches). No assertion was weakened.
The seed was refreshed through `make regenerate-seed`, verifying compiler stages
2/3/4 and identical stdlib bytecode and expected output. This refresh delivers
P02 to the public CLI; it does not earn credit for P09's later expanded-arrow gate.

- Input seed: `e11bfef47d3e2b69469b5d99647b825e9585a4d9be86e5435762a26129ff8ba4`.
- P02 seed, stages 2/3/4: `6a9eba9e4610bc11b02a1229df20f89b6ba745be2b867ca0c95138311e7ded21`.
- Unchanged stdlib: `eea94d9ea68fcafaa4a68d1eabb926add89f1fc8f3e2d9b6b5877353609c052b`.

Final `make check` passed (exit 0): 487 seconds overall, unit 336 seconds,
bootstrap 266 seconds in parallel, functional 108 seconds with 400 assertions,
release smoke 9 seconds and quick start 1 second. Parser 208 and module-binding
857 assertions also passed within the full run. The printed historical timing
budget warnings are not current acceptance gates under GI#106. This run is not a
controlled before/after performance comparison. `make docs` and `git diff --check`
passed. Hosted checks must still pass for the exact PR head before integration.

Runtime performance: unchanged direct-call instructions and receiver evaluation
count; no runtime registry or lookup was added. Compiler binding now reuses the
checked receiver and indexes methods. Whole-suite timings are validation evidence,
not a controlled compiler-performance benchmark.

Documentation reviewed: specification, architecture, self-hosting, compiler notes,
coverage and roadmap. README commands and current-release claims remain accurate.
Website: no impact on alpha.13 claims; this remains unreleased compiler work.

## Programme handoff

P01 and P02 are complete on acceptance of this delivery. P03 is the next eligible
packet: the bounded Server/Map coexistence fixture. P04 also depends on P02, but
P03 remains the representative trial before broader migration. P03-P15 have not
started; GI#525 remains open.

The eight-stage weights are unchanged. P02 supplies the ownership foundation,
assessed as 10% of the general-method stage; the substantial generic, built-in and
effect work remains. This is an estimated scope contribution, not a claim that
stage 2's acceptance gate is complete.

| Stage | Weight | Completion on acceptance | Earned contribution |
| --- | ---: | ---: | ---: |
| Inventory and ownership (P01) | 5% | 100% | 5 points |
| General method model (P02 foundation; P03/P04/P05/P07 remain) | 20% | 10% | 2 points |
| Built-in owned implementations | 20% | 0% | 0 points |
| Stdlib migration | 15% | 0% | 0 points |
| Compiler and source migration | 20% | 0% | 0 points |
| Legacy removal | 10% | 0% | 0 points |
| Dot cutover | 5% | 0% | 0 points |
| Release and ecosystem acceptance | 5% | 0% | 0 points |
| Total | 100% | | 7 points |

Packet register: P01 inventory complete; P02 metadata complete on acceptance;
P03 collision fixture, P04 generics, P05 core registry, P06 intrinsics, P07 effects,
P08 core sources, P09 seed checkpoint, P10 stdlib, P11 callers, P12 deletion,
P13 dot bridge, P14 dot conversion and P15 release acceptance remain unstarted.
