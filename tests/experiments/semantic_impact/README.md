# Bounded semantic-impact experiment

Reproduce with `make semantic-impact-experiment` (native toolchain and Node 24+).
Linux/macOS compiler CI runs this target separately from the interpreter-free
`make check`. It exports actual internal checker records, predicts before making
changed snapshots, then checks those snapshots through both the checker and
public `panack check`. Temporary source files are removed on success or failure.
This is one feasibility experiment for [GI#175: Semantic change prediction](https://github.com/sproates/panackelty/issues/175)
and [RM#97: Shared programme investigation](../../../ROADMAP.md#rm-97), not a
production predictor, public CLI, proof graph or completion of GI#175.

## Method and results

`baseline.panack` contains `safe_subtract → remaining → checkout` and an ordinary
`audit` caller. It executes with output `3`, `6`. `evidence.panack` uses
`load_project_explained`; the same production checker supplies `SubtractionEvidence`
and `EffectEvidence`. Its private tab-separated transport includes decision,
bound, selected guard/truth, operand types/constants and expression, or declared
callee classification, context and actual boundary violation count. It does not
parse diagnostics or infer evidence from an empty diagnostic list.

The runner captures the baseline, validates the narrow fixture assumptions and
emits `PRE-CHANGE predictions` before constructing any changed source. Predictions
are derived from retained facts and the existing two local rules: a Nat lower
bound must cover the right constant, and a pure context cannot call an ordinary
declaration. These small test-only projections are deliberately not a second
general checker. Their assumptions are checked; unsupported cases say `unknown`.
Actual modified checking is the independent validation, not the prediction input.

| Proposed edit | Prediction and observed result |
| --- | --- |
| Leaf guard `n >= 2` → `n >= 1` | Leaf `n - 2` becomes unproved; actual retained lower bound is 1, public check rejects. |
| Same guard edit, checkout `n - 5` | Local obligation remains proved by its unchanged Nat parameter, local true-branch `n >= 5`, lower bound 5 and right constant 5. All these fields are positively re-established in the rejected project. |
| Leaf `pure` → ordinary | `remaining → safe_subtract` rejects; ordinary `audit → safe_subtract` remains an allowed local boundary with actual ordinary context/callee evidence. |
| Same effect edit, checkout | The local subtraction proof remains established. The call to `remaining` still uses its unchanged pure declaration; it is not evidence that `remaining` or the whole project is valid. |
| **Additional conditional edit:** widen `remaining` to ordinary too | Predicted from the original checkout boundary: rejection moves outward to `checkout → remaining`; the now-ordinary remaining boundary permits its ordinary callee. This is a two-edit cascade, not automatic inference from the leaf edit. |

The non-impact argument is confined to checkout's **individual underflow
obligation**. The runner requires its exact closed fixture body, the recorded
operand/guard facts, and one of two permitted edits strictly outside that body.
The operand is checkout's parameter, not `balance` returned by the changed call.
Changes to the local guard or a body using `balance - 5` return unknown. Matching
after-check records confirms this scoped argument; equal output, unchanged source,
call-graph disconnection or absent diagnostics alone would not establish it.
No function behavior, execution path, return bound or whole-program validity is
claimed preserved. Source coordinates are deliberately not cross-revision IDs:
within this fixed fixture, function plus unique expression selects the obligation.

**Negative finding:** no transitive return-bound/proof consequence is expressible
from these records. Nat return types do not export guards or a relation between
input and result. Effect classifications use immediate declarations rather than
inferred transitive effects. Reachable callers are textual dependencies, and a
rejected project is compilation fallout; neither means every caller lost a proof.
The conditional effect cascade supplies useful dependency evidence but does **not**
satisfy GI#175's original one-change transitive acceptance criterion.

## Counterexamples and limits

- An inner `n >= 2` replaces an outer `n >= 3` as the retained bound. Weakening the
  outer guard has an unmatched origin (prediction unknown); actual checking still
  proves subtraction from the inner guard. Weakening the inner guard to 1 loses
  the proof even though the outer condition is mathematically sufficient. The
  checker does not recover alternative proofs; one selected origin is not a
  complete minimal dependency graph.
- Assignment `n = 0` between guard and subtraction discards the bound. The
  unchanged guard text is insufficient; absent valid evidence returns unknown.
- Guard/proof rejection skips the **entire** effect pass. The experiment asserts
  `effects_checked=false` and no effect records, and reports unavailable rather
  than preserving effects by silence. A separate conservative recovery slice
  could check type-valid functions after unrelated body errors, with valid global
  signatures and per-function availability. Removing the loader gate alone would
  both trust erroneous inferred/callable types and risk overwriting type diagnostics.
- Constant proofs can depend on initializer values without retained initializer
  provenance. Callable types, generic capabilities, imported aliases, recursion,
  loops, type-declaration guards, exhaustiveness and multi-cause edits are outside
  this predictor's closed fixture domain. Missing or unsupported evidence cannot
  be classified as preserved. This transport is not a supported external format.

| Category | Existing evidence / smallest missing machinery |
| --- | --- |
| Guard/Nat proof | Selected local lower-bound origin, operands and result exist; alternative assumptions, initializer chains and interprocedural contracts do not. |
| Effects | Immediate declaration/type boundaries exist; transitive classification is not inferred. A declared-mode change has a local semantic consequence. |
| Parameter/result types | Used by checking; no general retained constraint/dependency explanation exported here. No Nat-to-Int cascade claimed. |
| Generic capabilities | Needs retained capability obligations and substitutions; not assessed by this fixture. |
| Enum/exhaustiveness | Needs retained match/variant coverage obligations; not an effect/guard edge merely because all have source spans. |

Shared source identity, obligation identity and explicit availability can be
reused. A universal proof graph is not justified by this evidence.

## Cost and recommendation

Measured on Linux x86-64, Node 24.19.0, native `-O2`, compiler baseline `2957915`.
Fixture SHA-256: `4c10a346859a4f89ffd9521febbd2f18d0c5b47a0b3db8bbb5d45bebb7e59762`.
The 316-byte source loads standard-library core definitions too: **3 subtraction
records, 31 effect records, 3,407 transport bytes**, plus a 180-byte prediction
object. These are serialized test transport sizes, not heap/RSS measurements or
a proposed stable metadata representation. The fixture itself has two subtraction
obligations; the extra one belongs to the loaded closure.

Five warm process samples in milliseconds, excluding probe compilation:

| Mode | Samples | Median |
| --- | --- | --- |
| Public ordinary check | 46.597, 47.894, 48.136, 47.660, 47.029 | 47.660 |
| Retained evidence/export | 49.065, 48.227, 49.731, 47.500, 47.898 | 48.227 |

The observed difference is 0.567ms (~1.2%); it includes process startup, loading,
formatting and output. The binaries have different drivers. It does not isolate
retention cost, bound memory, demonstrate scalability or establish zero overhead.
The target rebuilds the probe when compiler/source inputs change and reports fresh
samples; timing is informational, never a brittle acceptance threshold. Ordinary
builds gain no analysis pass or metadata because no production sources change.

**Recommendation:** retain this experiment and do not ship a general prediction
command yet. The smallest independently testable production slice would be opt-in
declaration-effect dependency records with exact source/declaration identity,
availability and rule inputs, then a narrowly scoped pure-to-ordinary query that
reports direct incompatible/permitted boundaries and explicitly unknown transitive
behavior. Medium / one PR for evidence capture, another separately authorized
query slice if warranted; source identity and diagnostic parity tests are required.
Do not bundle effect recovery, general type explanations, new contracts or an
automatic repair cascade into that slice. GI#175 and programme GI#180 remain open.

Website impact: none; this adds reproducible research tests and a report, no
language semantics, released command, downloadable artifact or advertised feature.
