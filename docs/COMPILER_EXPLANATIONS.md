# Compiler explanations: subtraction and effect boundaries

`panack explain SOURCE.panack --function NAME` checks the complete project and
reports retained natural-number subtraction and effect-boundary evidence for one function. It reads
source; it does not execute the program, emit bytecode, write files, or consume a
source-map sidecar. The command is implemented in unreleased core; existing
published/browser versions do not acquire it automatically.

For staged multi-module projects, qualify a query with the portable module
identity, for example `--function project/library.panack:lookup`. A bare name
continues to work when it selects one loaded function. When multiple modules
define that name, the query reports ambiguity; a module path that does not own
the requested function is rejected. Explain and locate use the same selector
rules, so retained evidence cannot silently come from a same-named function in
another module.

For example:

```panack
pure remaining(n: Nat): Nat {
  if n >= 2 { n - 2 } else { 0 }
}
main(): Void { print(remaining(5)); }
```

`panack explain example.panack --function remaining` reports the `n - 2`
obligation, right constant `2`, retained lower bound `2`, and the true branch of
`n >= 2`, with the original source ranges of both expressions. If the subtraction
is changed to `n - 3`, the same bound is reported as insufficient. A write that
invalidates a guard also discards its origin; a subsequent fresh guard supplies
new evidence. False branches are labelled explicitly.

## Meaning of the output

- `program: accepted` means the normal resolver, type checker and purity checks
  accepted the entire captured project. `program: rejected` means they did not.
  A local proof can still be reported in a rejected program; it does not override
  other diagnostics, purity failures, or return-type errors.
- `subtraction: proved` means the checker's actual Nat underflow obligation passed
  using retained constants or a name's lower bound against a right constant.
- `subtraction: unproved` means the checker could not discharge that obligation.
  It is not a demonstrated runtime failure. The query does not invent missing
  assumptions, propose a minimal repair, or run a separate prover.
- `subtraction: unavailable` covers other numeric domains, invalid operands, or
  no recorded subtraction in the requested function. Parsing/resolution failures
  can prevent checking entirely. Missing source positions are also explicit.
- Type and purity failures retain ordinary diagnostics on stderr and exit status
  1. Accepted projects exit 0, including a valid function with no supported
  explanation. Invalid arguments or a missing function in a valid project exit 1.

Coordinates are one-based lines/Unicode code-point columns and half-open ranges
in the captured source text. Paths use `project/`, `stdlib/`, or the source-map
identifier rules for external paths. Control characters in paths and excerpts
are escaped. Source excerpts are read from the loaded snapshot, not reread to
reconstruct proofs. As with ordinary compilation, this is not an atomic snapshot
of a concurrently edited source tree.

## Effect-boundary explanations

The same command also reports the effect pass's actual decisions for calls,
`await` context/operands and rejected discarded non-Unit await results. For example,
`pure report(): Void { print(1); }` reports a rejected call boundary at `print(1)`,
with pure context, ordinary callee effect and the existing purity rejection reason.
An ordinary function calling `print` reports an allowed local boundary.

- `effect boundary: allowed` means that particular boundary passed its existing
  effect rules. It does not imply that argument expressions, the surrounding await,
  another function, or the complete program passed. Each nested boundary is checked
  separately; use `program: accepted/rejected` for whole-project validity.
- `effect boundary: rejected` reports each violation retained at that boundary by
  the actual pass. The renderer does not parse diagnostics or recheck effects.
- Calls show the enclosing mode (`pure`, `ordinary`, `async`), callee classification,
  whether directly awaited, and whether classification came from a named declaration,
  a callable type, or a builtin/constructor contract. An ordinary declaration stays
  ordinary even when its body currently does nothing. A `PureFn` widened to `Fn`
  is classified by its declared callable type, not the original function body.
- Async calls require await in an async context; async contexts reject blocking
  ordinary calls. Await context and operand checks are separate boundaries.
- After a function body type error, the query still checks local effect boundaries
  in separately type-valid functions. Eligibility includes the complete body and
  return type check; it never recovers individual expressions inside an invalid
  function. Loading, parsing, resolution or type errors in signatures or type declarations
  prevent recovery throughout the project, including earlier valid functions.
- `effects: unavailable` distinguishes a global frontend failure, an invalid
  selected function, and a checked function with no call/await boundary.
- Recovery is explanation-only. Original diagnostics on stderr, their order, exit
  status and ordinary compilation's type-first behavior are unchanged. Recovered
  effect violations appear in the local explanation, not as additional stderr
  diagnostics. The project remains rejected. An allowed call to a valid declaration
  does not establish that the callee body type-checks or that a transitive call is safe.

Both branches and loop bodies are checked statically. Output does not say a branch
ran, infer transitive effects, identify a dynamic callable implementation, or follow
callee bodies to find an ultimate I/O operation. Generic/imported definitions keep
their own source positions and are checked once. Type-declaration guard diagnostics
still run when effects are checked, but their evidence is outside this function
query. A type-valid guard with an effect violation does not disable local function
evidence. After unrelated body type errors, that guard violation is not added to
stderr: the original type-first diagnostics are preserved, and the project remains
rejected. There is no separate
capability taxonomy such as Network or Filesystem in this output.

## Boundaries and cost

This first query covers binary Nat subtraction in function definitions. Generic
functions are checked once at their definition; output does not claim call-site
specialisation or runtime path execution. Imported definitions keep their own
source locations. A query can include multiple subtraction obligations in checker
traversal order, including nested subtractions and both branches.

The proof rules and acceptance semantics are unchanged. Variable-to-variable
relations, compound guard inference and arithmetic constant folding are not added.
The original checker may reject mathematically safe expressions outside its
supported proof rules. Constant values are retained, but declaration/initializer
provenance chains are not; the operation range identifies the operands being
checked. Type-declaration guards are not part of this function query. General type explanations, transitive effect explanations, compilation provenance, suggested requirements, change
predictions and runtime value derivations remain separate programme work.

Only the explicit query retains subtraction/effect evidence and lower-bound origins. Ordinary
checking carries empty evidence/availability collections and omits guard origins. Effect decisions
share the same traversal and predicates in both modes; opt-in records retain local
reasons and source spans, with no extra type analysis pass. After body errors only the query runs the
filtered effect traversal; ordinary compilation skips it. No evidence
is serialized into bytecode, and the bytecode format remains v9. The query checks
the whole loaded project before filtering output to the function. It uses bounded
source loading (1 MiB per file; 8 MiB and 256 files per closure); the existing
source-map loader reports failures for exceeded bounds. This is a source-size
bound, not a hard guarantee on compiler time, memory, or output size. Plain text
is the current presentation, not a versioned machine-readable protocol.

Acceptance coverage lives in `tests/runner/compiler_explanations_unit.panack` and
`tests/functional/cases/cli_explanations`, plus the existing mutation/checker,
bootstrap and public-CLI suites. Measurements and limitations are recorded in
[the validation profile](../tests/VALIDATION_PROFILE.md). This bounded delivery
completes neither #134's whole types/effects/proof scope nor programme #180.
