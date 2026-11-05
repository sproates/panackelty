# Validating configuration and request data

The [dispatch example](../../examples/validated_dispatch.panack) puts checks at
two boundaries. Configuration supplies the first and last retry delays. Each
request supplies a label. Once these values have passed their checks, the
dispatch function accepts a typed configuration and a `Label`.

Run it from the root of the extracted toolchain:

```sh
./bin/panack run examples/validated_dispatch.panack
```

The queue list is known when the program is compiled. Panack checks that
`Queues(["support"])` has between one and four entries. The retry window
needs a comparison of the supplied values, so `configure` calls
`validate[OrderedWindow](RetryWindow(first,last))`. Configuration succeeds
when the first delay is less than or equal to the last.

The request handler takes a `Str` and matches on `validate[Label](raw_label)`.
The `Ok` arm passes a `Label` to dispatch. The `Error` arm calls
`problem.render()` and decides to print the result. An empty request label
fails two requirements. The maximum-length and control-character rules pass.

The [complete expected output](../../tests/functional/expected/examples/validated_dispatch.stdout)
includes the accepted request, both explanations for the empty label, and the
rejected retry configuration. The same example runs from saved bytecode:

```sh
./bin/panack compile examples/validated_dispatch.panack -o dispatch.bc
./bin/panack run dispatch.bc
```

## Migrating an existing guard

Alpha.14 used an implicit candidate named `value`. In alpha.15, name the
candidate immediately after `where`. The mechanical change keeps the expression
and its meaning:

```diff
-type Port = Nat where value >= 1 && value <= 65535
+type Port = Nat where value => value >= 1 && value <= 65535
```

Run `panack check` on the program entry point after migrating the declarations.
Existing numeric alternatives using `||` retain their meaning. Combine the
new string, collection and record requirements with `&&`.

Use proof-only construction for a known value. Use `validate[Type](input)`
when the input has the correct base type and the invariant depends on its
contents. For a replacement record, construct the complete candidate and
validate that record. Keep the old guarded record until the result is `Ok`.

The example widens `config.queues` to `[Str]` before indexing. A copy can
preserve its guarded type, or you can explicitly request the ordinary base
type for an operation. A bound on size alone does not prove an arbitrary
index is in range.

## Reading an error in code

Import `ValidationError` from `stdlib/validation` to write an explicit error
type annotation. In a match arm Panack infers the type of `problem`.

| Member | Meaning |
| --- | --- |
| `problem.target_type` | Identity of the declared guarded type, including its module. |
| `problem.failures` | One `ValidationFailure` per failed written requirement, in declaration order. |
| `failure.locations` | `WholeValue` for a value rule, or two `Field` names in comparison order for a relationship. |
| `failure.constraint` | The declared requirement, including the bound, unit or relation where applicable. |
| `failure.facts` | The observed count, blank text, first control character, unsatisfied relation or unsatisfied numeric predicate. |
| `problem.render()` | Plain text with a heading and a paragraph per failure. The caller controls display and the final newline. |

A control-character fact carries a zero-based Unicode scalar index and the
scalar number. The rendered message counts character positions from one.
Neither form includes the original string. For all supported guard forms and
proof rules, see the [language reference](../reference/SPEC.md#guarded-types).
