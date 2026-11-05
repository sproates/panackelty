# Alpha.15 validated domain types acceptance

The alpha.15 development toolchain at core revision `4551a1d` was packaged
on 5 November 2026 on macOS 26.5 arm64. The new example was included in the
archive, which was then extracted, relocated to a path containing a space, and
run from an unrelated working directory with the bootstrap and standard-library
environment overrides unset. The development package still identifies itself
as alpha.14, bytecode 9, pending release versioning.

The installed CLI checked and ran the [dispatch example](../examples/validated_dispatch.panack),
compiled it with a source map, and executed the saved bytecode. Both executions
matched the [expected output](../tests/functional/expected/examples/validated_dispatch.stdout).
`explain` on `handle_request` identified the runtime checks. A reverse lookup
on `w.first <= w.last` located the comparison in `configure`.

The existing stable alpha.14 installation ran the old `Port` declaration in the
[migration guide](guides/VALIDATED_DOMAIN_TYPES.md). The relocated development
installation ran the migrated declaration. Both printed 443.

The installed compiler also rejected proof-only construction and returned a
structured runtime failure for each of these inputs:

| Requirement | Rejected input |
| --- | --- |
| Non-empty string | Empty text |
| Maximum two Unicode scalars | Three ASCII letters |
| Non-blank | One U+3000 IDEOGRAPHIC SPACE |
| No control characters | A tab between two letters |
| Collection count | Empty array with minimum one, repeated map key with minimum two, two distinct set members with maximum one |
| Field relationship | First delay 4, last delay 3, required first ≤ last |

The [functional fixture](../tests/functional/cases/domain_constraints/main.panack)
covers positive and negative runtime outcomes, replacement and multi-rule order.
The [compiler contracts](../tests/runner/compiler_suites/contracts.panack)
assert the public fields, scalar boundaries, and omission of sensitive input.
The [CLI regression suite](../tests/functional/cases/cli_explanations/main.panack)
distinguishes known invalid values, missing proof and unsupported declarations.

## Follow-up to the acceptance audit

The original acceptance used an explicit base-array conversion to index the
queue. The follow-up exercises direct indexing in the packaged example.
The functional regressions cover a guarded/base conditional, literal arrays of
guarded elements and nested arrays. Invalid values identify the offending
expression and its guard. A base-typed array with unknown contents requires
element-level proof. Both static control exclusion and runtime validation now
use the same first-control lookup in the core library. Its scalar offset and
code are checked at the C0/C1 boundaries and after a supplementary character.

On macOS arm64 the follow-up compiler (seed `2124cd1c`) and the original
`fc5e66a` installation compiled the same 10,000-iteration control-exclusion
workload. Inputs alternated between `"Ticket 42"` and `"𐐷\tTicket"`; both
executables reported 5,000 failures. On the same VM, five alternating trials
after warm-up took medians of 173 ms and 218 ms respectively. The timing
includes input selection, checking and summing failure counts.

## Cost of proof and explicit validation

Each program summed one expression over 50,000 iterations and printed one
total. The installed compiler produced saved bytecode once per program. After
one warm-up per artifact, five trials ran in rotating order using the installed
VM on the same machine. Times include VM process startup and the loop. They
exclude compilation, error rendering and terminal interaction.

| Expression in the loop | Total | Median wall time | Range |
| --- | ---: | ---: | ---: |
| Plain `"Ticket 42".len()` | 450,000 | 36.2 ms | 32.6–43.9 ms |
| Proved `Label("Ticket 42").len()` | 450,000 | 37.3 ms | 31.5–39.1 ms |
| Validate `"Ticket 42"`, read accepted length | 450,000 | 1,668.5 ms | 1,547.1–1,755.3 ms |
| Validate `""`, read number of failures | 100,000 | 561.1 ms | 487.9–656.4 ms |

The plain and statically proved programs produced byte-for-byte identical
executables. Explicit validation of the nine-scalar input took about 33 µs per
iteration for these four rules. The empty-string case has fewer scalars to
inspect and constructs two failures. It measures a different workload.

To reproduce, use the four-rule `Label` declaration from the dispatch example
and this loop. Replace `EXPRESSION` with a table entry for the plain and proved
cases. For either runtime case, match on `validate[Label](input)` using
`Ok(v) => v.len()` and `Error(e) => e.failures.len()`.

```panackelty
main(): Void {
  mut total: Nat = 0
  for i in 0..50000 { total = total + (EXPRESSION) }
  print(total)
}
```
