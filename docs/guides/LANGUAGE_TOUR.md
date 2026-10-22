# Language tour

## Exact fractions and Unit

```panackelty
main(): Void {
  third = 1/3
  ten = third * 30
  print(ten.nat())  // 10
  print((1/8).dec()) // 0.125
  success: Unit = ()
  print(success)   // ()
}
```

Integer `/` returns `Rat`;
`a.divide(b)` retains truncating natural division. Exact conversions trap if
the value cannot be represented: `(1/3).nat()` and `(1/3).dec()` fail rather than
round. `Unit` is a first-class value for APIs such as `Result[Unit,Str]`.
See [the specification](../reference/SPEC.md#rational-arithmetic-and-exact-conversions).


## Typed paths and elapsed time

Panackelty includes opaque `Path`, exact signed nanosecond
`Duration`, and monotonic `Instant` values. Import `stdlib/path` and
`stdlib/time`. Path construction checks text or native bytes; lexical operations
preserve spelling and do not access the filesystem. Durations support exact
arithmetic and checked fractional conversions. `time.now()` is effectful
and returns a `Result`; arithmetic on existing clock readings is pure.

Typed filesystem and process APIs are available through `stdlib/filesystem`
and `stdlib/process`. They accept `Path` values and return structured errors.
Process execution supports byte streams, working directories, environment
overrides, output limits, and exact-duration timeouts; `host.sleep` provides
checked sleep. See the [host API contract](../reference/SPEC.md#typed-filesystem-process-and-sleep-apis)
and [runnable example](../../tests/functional/cases/host_capabilities/main.panack).

The [API specification](../reference/SPEC.md#paths-and-monotonic-time) and
[executable example](../../tests/functional/cases/host_types/main.panack) show their
contracts and use.


## A quick language tour

Every linked program is included in the download archive and exercised from
both source and compiled bytecode by the functional suite. From a source
checkout, run an example with `./bin/panack run examples/NAME.panack`. From the root
of an extracted archive, use `./bin/panack run examples/NAME.panack`.

| Feature | Runnable example | Specification |
| --- | --- | --- |
| Compact functions and conditional values | [`fizzbuzz.panack`](../../examples/fizzbuzz.panack) | [Declarations, functions, and `Void`](../reference/SPEC.md#declarations-functions-and-void) |
| Arbitrary-precision integers and exact decimals | [`decimal.panack`](../../examples/decimal.panack) and [`euler003.panack`](../../examples/euler003.panack) | [Values and numeric semantics](../reference/SPEC.md#values-and-numeric-semantics) |
| Guarded domain types | [`guards.panack`](../../examples/guards.panack) | [Guarded types](../reference/SPEC.md#guarded-types) |
| Pure functions with local loops and mutation | [`euler001_iterative.panack`](../../examples/euler001_iterative.panack) | [Effects and purity](../reference/SPEC.md#effects-and-purity) and [ranges, arrays, and loops](../reference/SPEC.md#ranges-arrays-and-loops) |
| Unicode strings, indexing, and interpolation | [`strings.panack`](../../examples/strings.panack) | [Strings](../reference/SPEC.md#strings) |
| Records, generic tagged unions, and exhaustive matching | [`lexer_foundation.panack`](../../examples/lexer_foundation.panack) and [`option_result.panack`](../../examples/option_result.panack) | [Records and tagged unions](../reference/SPEC.md#records-and-tagged-unions) |
| Named callable values and higher-order arrays | [`callables.panack`](../../examples/callables.panack) | [Effects and purity](../reference/SPEC.md#effects-and-purity) |
| Persistent arrays, maps, sets, and byte buffers | [`collections_and_bytes.panack`](../../examples/collections_and_bytes.panack) | [Persistent collections and bytes](../reference/SPEC.md#persistent-collections-and-bytes) |
| Logical modules and the bundled standard library | [`option_result.panack`](../../examples/option_result.panack) | [Modules](../reference/SPEC.md#modules) and [standard library](../reference/SPEC.md#standard-library) |
| One verified bytecode execution model | Any example above | [Compilation and the Panackelty VM](../reference/SPEC.md#compilation-and-the-panackelty-vm) |

`else` is optional for a conditional used only for control flow. A conditional
that produces a value remains exhaustive and requires both branches.

The examples deliberately stay small enough to modify. The
[`examples` guide](../../examples/README.md) continues with complete algorithms,
including palindrome detection, memoized Fibonacci, and Project Euler 1–5.

### Interpolate a local value

Interpolation accepts a local scalar name, such as `${first}`. If the value comes
from an expression or field access, bind it to a local first:

```panackelty
pure describe(name: Str, score: Nat): Str {
  first: Str = name[0]
  "${first}: ${name} scored ${score}"
}
```

For example, `${name[0]}` is not an interpolation form; `first = name[0]`
followed by `${first}` is. See the complete [strings example](../../examples/strings.panack).


## Language highlights

### Local type inference

Local annotations can be omitted when the initializer determines the type:

```panackelty
main(): Void {
  name = "Ada"
  mut greeting = "Hello, ${name}"
  greeting = "Welcome, ${name}"
  mut balance: Int = 0
  balance = -1
  print(greeting)
}
```

A new plain name declares an immutable binding; assigning to a visible name
requires `mut`. Types stay fixed, shadowing is prohibited, and empty values such
as `[]` need annotations when the initializer cannot determine their element
type. Function signatures remain explicit. See the [binding rules](../reference/SPEC.md#declarations-functions-and-void).

### Generic functions

Functions can work with different types while
retaining compile-time checks:

```panackelty
pure identity[T](value: T): T { value }

main(): Void {
  print(identity(42))
  print(identity[Str](../../"hello"))
}
```

Calls infer type arguments from their inputs, or accept an explicit complete
list. Generic bodies are checked even when unused; unconstrained `T` does not
permit arithmetic. See the [generic function rules](../reference/SPEC.md#generic-source-functions)
and [runnable example](../../examples/generic_functions.panack) for library helpers,
empty collections, and recursion.

### Exact decimal arithmetic

Financial and measurement code should not inherit a rounding surprise merely
because it used a decimal literal:

```panackelty
pure total_with_tax(subtotal: Dec, rate: Dec): Dec {
  subtotal + subtotal * rate
}

main(): Void {
  print(total_with_tax(19.99, 0.20))
}
```

`Dec` addition, subtraction, multiplication, and remainder are exact. Division
is accepted when the result has a finite decimal expansion; otherwise the VM
reports that an explicit rounding operation is needed.

### Domain rules in the type system

Guarded types turn familiar values into distinct types with compiler-checked
invariants:

```panackelty
type Port = Nat where value >= 1 && value <= 65535

pure describe_port(port: Port): Str {
  "listening on ${port}"
}

main(): Void {
  http: Port = 8080
  print(describe_port(http))
}
```

The compiler only accepts a conversion when it can prove the target guard from
the value or its guarded source type. It does not hide a runtime check inside
otherwise pure code.

### Explicit effects, practical algorithms

Pure code cannot read files, inspect the environment, or print. Local mutation
is allowed, so straightforward iterative code stays straightforward:

```panackelty
pure sum(values: [Nat]): Nat {
  mut total: Nat = 0

  for value in values {
    total = total + value
  }

  total
}

main(): Void {
  print(sum([10, 20, 12]))
}
```

Receiver-first method syntax keeps common operations readable without changing
their functional semantics. It is ordinary call syntax with the receiver
supplied as argument 1, so persistent updates still return new values:

```panackelty
pure extend(values: [Nat], value: Nat): [Nat] {
  values.append(value)
}

main(): Void {
  mut values: Map[Str,Nat] = map()
  values = values.put("answer", 42)
  if values.has("answer") {
    print(values.get("answer"))
  }
  print("Aλ🙂".reverse())
}
```

Explicit function references support checked higher-order code without hidden
captures. Callable types retain purity, and persistent array transformations
infer their result from the callback signature:

```panackelty
pure square(value: Nat): Nat { value * value }
pure total(sum: Nat, value: Nat): Nat { sum + value }

main(): Void {
  operation: PureFn[Nat,Nat] = @square
  squares: [Nat] = [1, 2, 3, 4].map(operation)
  print(squares)
  print(squares.reduce(0, @total))
}
```

### Exhaustive data modelling

Tagged unions and exhaustive matching make success and failure part of a
function's signature:

```panackelty
pure safe_divide(numerator: Nat, denominator: Nat): Result[Nat,Str] {
  if denominator == 0 {
    Error("division by zero")
  } else {
    Ok(numerator.divide(denominator))
  }
}

pure describe(result: Result[Nat,Str]): Str {
  match result {
    Ok(value) => "result: ${value}",
    Error(message) => "error: ${message}"
  }
}
```

The compiler rejects a `match` that misses or repeats a variant.
