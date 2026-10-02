# Compiler explanations: first U3 query

`panack explain SOURCE.panack --function NAME` checks the complete project and
reports retained natural-number subtraction evidence for one function. It reads
source; it does not execute the program, emit bytecode, write files, or consume a
source-map sidecar. The command is implemented in unreleased core; existing
published/browser versions do not acquire it automatically.

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
checked. Type-declaration guards are not part of this function query. Type/effect
explanations, general compilation provenance, suggested requirements, change
predictions and runtime value derivations remain separate programme work.

Only the explicit query retains evidence and lower-bound origins. Ordinary
checking carries empty evidence collections and omits guard origins. No evidence
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
