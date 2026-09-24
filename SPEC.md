# Panackelty language charter (draft 0.2)

## Promise

Panackelty makes small terminal programs and exact numerical algorithms pleasant to
write, while letting the compiler enforce domain invariants and isolate effects.

Source code compiles to Panackelty bytecode. The bytecode format and VM are part of
the language, rather than an incidental implementation detail.

## Values and numeric semantics

| Type | Meaning |
| --- | --- |
| `Nat` | Arbitrary-precision integer greater than or equal to zero |
| `Int` | Arbitrary-precision signed integer |
| `Dec` | Arbitrary-precision base-10 decimal; never binary floating point |
| `Rat` | Exact rational with a signed arbitrary-precision numerator and positive denominator |
| `Unit` | Singleton value written `()` |
| `Str` | Unicode text |
| `Bool` | `true` or `false` |
| `Void` | A function return marker indicating that no value is returned |

Integer literals are inferred as `Nat` when non-negative and `Int` when
negative. A literal containing a decimal point is `Dec`. `Dec` stores an
arbitrarily large base-10 coefficient and scale. Addition, subtraction,
multiplication, and remainder are exact. Division is exact when it has a finite
decimal expansion and otherwise reports that an explicit rounding operation is
needed; rounding modes are not yet part of the language. Exact division removes
redundant fractional trailing zeros from its result (for example, `1.00 / 2.0`
prints `0.5`, and `0.00 / 2.0` prints `0`).

`Nat` subtraction is accepted only when the checker can prove that the result
is non-negative. This first implementation recognizes constants and simple
guard facts. Use `Int` when subtraction may legitimately cross zero.

### Rational arithmetic and exact conversions

Division `/` of `Nat` and/or `Int` operands produces `Rat`, including when the
result is integral. Rational `+`, `-`, `*`, `/`, unary negation, equality, and
ordering are exact. Binary operations accept `Rat` mixed with `Nat` or `Int`;
arithmetic results remain `Rat`. `Rat` does not mix implicitly with `Dec` and
does not support `%`. Other integer arithmetic retains its existing types.
There is no implicit assignment conversion from an integer to `Rat`; use `n/1`.

Every rational is reduced by the greatest common divisor, with a positive
denominator; zero is `0/1`. Display always uses `numerator/denominator`, including
`10/1`. Division by zero traps. Intermediate integers may grow without a fixed
precision limit, so rational arithmetic is not a constant-cost operation.

```panackelty
third = 1/3
ten = third * 30  // Rat, normalized to 10/1
x = ten.nat()     // Nat, exactly 10
half = (1/2).dec() // Dec, exactly 0.5
```

`value.nat()` (also `nat(value)`) accepts `Rat` and returns `Nat` only when the
value is integral and non-negative; otherwise it traps without truncating.
`value.dec()` (also `dec(value)`) accepts `Rat` and returns its exact finite
`Dec` representation, or traps when the reduced denominator contains factors
other than two and five. Rounded decimal conversion and checked `Result`
conversion APIs remain deferred. These operations are pure despite being partial.

`quotient(a, b)` accepts two `Nat` values and returns their truncated natural
quotient; it traps for a zero divisor. This explicit operation replaces former
uses of integer `/` for byte packing and integer algorithms. `%` remains the
integer remainder operation (with the divisor's sign for signed operands).
`Dec / Dec` retains its existing exact-decimal semantics.

### First-class `Unit`

`Unit` has one value, `()`, distinct from the internal no-result `Void` sentinel.
It may be stored, passed, returned, used in generic arguments, records, enums,
arrays, maps, sets, and callbacks. For example, `Result[Unit,Str]` can contain
`Ok(())`. Equality of two Unit values is true; ordering and arithmetic are invalid.
A function returning `Unit` must explicitly produce a Unit value; an empty body
still produces `Void`. Neither `Rat` nor `Unit` is currently a guarded-type base.

## Paths and monotonic time

`Path`, `Duration`, and `Instant` are opaque, immutable, non-generic value types.
They have no source constructors, public fields, numeric casts, or bytecode
constant tags. The names cannot be redeclared. Values can be stored in records,
arrays, and generic types, passed to callbacks, and compared with `==`/`!=`.
They are not scalar Map keys or Set elements. Arithmetic and ordering use the
named functions below; infix arithmetic and ordering are not overloaded.

### Native paths

Import `stdlib/path` for `PathError`, `Result`, and `Option`. The initial native
representation is POSIX bytes on the supported Linux and macOS targets. Text
construction encodes UTF-8; native construction preserves arbitrary bytes.
Both reject empty input (`EmptyPath`) and embedded NUL (`PathContainsNul`).
Neither construction nor any lexical operation below accesses the filesystem.
A `Path` does not establish existence, access permissions, canonical identity,
or containment within a directory.

| Function | Result and behavior |
| --- | --- |
| `path_from_text(Str)` | `Result[Path,PathError]`; checked UTF-8 construction |
| `path_from_native(Bytes)` | `Result[Path,PathError]`; checked native-byte construction |
| `path_current()` | `Path` containing `.`; does not capture the working directory |
| `path_native_bytes(Path)` | Exact native `Bytes` |
| `path_to_text(Path)` | `Result[Str,PathError]`; `PathNotUtf8` on invalid UTF-8 |
| `path_display(Path)` | ASCII `Str`; escapes backslash, controls, and non-ASCII bytes as lowercase `\xhh` |
| `path_absolute(Path)` | `Bool`; whether the first byte is `/` |
| `path_append(Path, Path)` | `Result[Path,PathError]`; rejects an absolute right operand with `AbsolutePathAppend` |
| `path_directory(Path)` | Lexical parent `Path`; a bare component has parent `.` and a root is its own parent |
| `path_filename(Path)` | `Option[Path]`; last nonempty component, or `None` for a root |

Appending inserts `/` only when the left operand does not already end with it.
No operation collapses `.` or `..`, resolves symbolic links, changes case, or
silently makes a relative path absolute. Parent and filename extraction ignore
trailing separators and retain leading root separators (including `//`). Dot
components remain literal components. Equality compares native bytes exactly:
`a` and `./a` are unequal even when they refer to the same file. Display is for
humans, not a serialization or shell-escaping format; printing an unconverted
Path produces `<Path>`.

The existing Str-based `path_parent`, `path_join`, `path_suffix`,
`path_with_suffix`, `path_is_absolute`, `path_resolve`, and `file_exists` retain
their bootstrap-compatible contracts. In particular, the old `path_join`
normalizes its result; the typed `path_append` never does. Typed filesystem queries and I/O are specified below. Non-POSIX
representations remain follow-up work.

### Exact durations

Import `stdlib/time`. A `Duration` stores a signed arbitrary-precision integer
number of nanoseconds. Negative durations are valid, including differences
between an expired deadline and the current instant. Sleep and process timeout APIs validate nonnegative values and host limits
at their boundaries, as specified below.

| Function | Result and behavior |
| --- | --- |
| `duration_nanoseconds(Int)` | Exact `Duration` |
| `duration_milliseconds(Int)` / `duration_seconds(Int)` | Exact scaled `Duration` |
| `duration_ticks(Duration)` | Signed `Int` nanoseconds |
| `duration_from_seconds(Rat)` | `Result[Duration,DurationError]`; rejects fractional nanoseconds |
| `duration_as_seconds(Duration)` | Exact `Rat`, without rounding |
| `duration_add(Duration, Duration)` / `duration_subtract(Duration, Duration)` | Exact `Duration` |
| `duration_scale(Duration, Int)` | Exact `Duration` |
| `duration_divide(Duration, Int)` | `Result[Duration,DurationError]`; requires an exact integer number of nanoseconds |
| `duration_ratio(Duration, Duration)` | `Result[Rat,DurationError]`; exact dimensionless ratio |
| `duration_before(Duration, Duration)` | `Bool`; signed less-than comparison |

`DurationError` is `FractionalNanosecond` or `ZeroDurationDivisor`.
Construction from seconds accepts an explicitly rational expression such as
`3/2`; decimals do not implicitly convert to rationals. The type has no fixed
integer overflow boundary, subject to available memory. Printing a duration
produces its signed nanosecond count followed by `ns`. Nanosecond representation
does not promise nanosecond clock resolution or accuracy. Rounded conversions
are not provided; use exact rational seconds or explicitly calculate integer
nanoseconds.

### Monotonic instants

`instant_now(): Result[Instant,ClockError]` reads the host's `CLOCK_MONOTONIC`.
It is effectful, including when used through other functions. Host read failure
returns `Error(ClockUnavailable())`. All other operations here are pure:

| Function | Result and behavior |
| --- | --- |
| `instant_add(Instant, Duration)` | A shifted `Instant`, including for negative durations |
| `instant_difference(Instant, Instant)` | Signed `Duration`: first operand minus second |
| `instant_before(Instant, Instant)` | `Bool`; strict less-than comparison |

All instants obtainable within one program execution share one monotonic clock
domain. Repeated reads may be equal; later reads do not precede earlier reads.
Clock readings are independent of calendar-clock corrections. Whether time
spent in system suspension is counted follows the host's monotonic-clock
contract and is not a portable guarantee. Deadlines shifted before the clock's
origin remain valid arithmetic values; they do not change that origin.

There is no public epoch, tick accessor, wall-clock conversion, integer
constructor, serialization, or cross-execution comparison. Printing an instant
produces `<Instant>`. The bytecode container remains version 8: new calls use
existing verified `CALL` instructions, and older VMs reject unknown calls.
Runtime operand checks reject forged records and wrong types even when bytecode
bypasses source checking.

## Declarations, functions, and `Void`

A function declaration starts with its name. There is no `fn` keyword. An
optional `pure` modifier precedes the name, and a colon introduces the return
type:

```panackelty
pure increment(value: Nat): Nat {
  value + 1
}

main(): Void {
  answer: Nat = increment(41)
  mut label: Str = "answer"
  label = "${label} ${answer}"
  print(label)
}
```

A call may use receiver-first method syntax. `receiver.name(arguments)` is
defined as `name(receiver, arguments)`: the receiver becomes argument 1, and
the remaining arguments retain their order. The callable is resolved in the
same module-wide namespace as an ordinary call, then the receiver participates
in the usual argument checking, generic inference, and purity checking.

```panackelty
pure increment(value: Nat, amount: Nat): Nat { value + amount }

main(): Void {
  values: [Nat] = [40.increment(2)].append(43)
  print(values.len())
}
```

Method syntax does not declare members or introduce overloads. A dot-name
without parentheses remains record-field access, while a dot-name followed by
parentheses is always a method call: `box.value` reads a field, but
`box.value()` looks up the global callable `value` and supplies `box` as its
first argument. Unknown callables and receiver type mismatches are diagnosed
exactly as for the corresponding ordinary call; receiver mismatches therefore
refer to argument 1. Postfix field access, indexing, and method calls may be
chained from left to right. Because constructor calls share the global callable
namespace, the same lowering applies to them, although normal arity and type
rules still govern whether such a call is useful.

Four collection names are method-only exceptions to global lookup. `put` and
`get` require a `Map[K,V]` receiver, `add` requires a `Set[T]` receiver, and
`has` accepts either a map/key pair or a set/element pair. These names are
resolved from the receiver type and do not reserve the corresponding global
function names: a program may still declare an unrelated function named
`add`, but `value.add(item)` always selects the collection method. Their
internal bytecode call names are not source identifiers.

Local type annotations may be omitted. `name = value` declares a new immutable
local when no local or parameter with that name is visible; otherwise it assigns
to the existing binding. `mut name = value` declares a mutable inferred local.
`name: Type = value` and `mut name: Type = value` always declare explicitly typed
locals. There is no `let` keyword or `:=` operator.

The initializer determines an inferred binding's fixed type: non-negative integer
literals use `Nat`, negation of an integer uses `Int`, decimal literals use `Dec`,
and other expressions use their existing checked type. A call uses its declared
return type; copying a guarded value preserves its domain type, and copying a
callable preserves its `PureFn` or `Fn` signature. A literal satisfying a guard
does not automatically acquire that domain type. Mutable locals do not retain
initializer constants as proof of their future value.

```panackelty
main(): Void {
  name = "Ada"
  mut ready = false
  ready = true
  mut balance: Int = 0
  balance = -1
  print(name)
}
```

Each inferred initializer must determine a complete non-`Void` value type before
the next statement. `[]`, `None()`, `map()`, `set()`, or nested constructions such
as `Some([])` need annotations when their type arguments remain unresolved.
Evidence inside the same expression can resolve these arguments, including
`[[], ["Ada"]]`, `if ready { None() } else { Some("Ada") }`, and
`map().put("Ada", true)`. Compatible array elements and branches combine nested
type evidence independently of their order. Incompatible element or branch
types remain errors. Expected-type handling already supported for annotated
bindings, calls, and returns is unchanged. Later assignments, uses, and enclosing
return types do not supply missing evidence for an inferred local declaration.

Assignments require a mutable binding and a value compatible with its fixed type;
they never change its type or mutability. Explicit annotations remain necessary
on function parameters, function returns, and record fields.

Declarations are visible only after their initializer, through the rest of their
enclosing block and nested blocks. A declaration cannot reuse a visible local or
parameter name, even in an inner block. This also applies to loop variables and
pattern bindings. Sibling blocks may reuse names that are not visible in one
another. Plain `=` inside a nested block assigns to a visible outer binding;
it does not shadow it. Function parameters, loop variables, and pattern bindings
remain immutable. Bindings introduced in a branch or loop do not escape it.
A misspelled assignment to an unknown name declares a new immutable local;
unused-binding warnings are not implemented yet.

`Void` is valid only as a function return type and is not a first-class source
value: it cannot be used for parameters, bindings, collection elements, or
arguments. A `Void` function may fall through its closing brace without a final
expression. Every non-`Void` function must end with a value compatible with its
declared return type. Empty parentheses produce the first-class `Unit` value.

Within a block, a physical line break terminates a complete declaration,
assignment, or standalone expression. A semicolon remains an explicit separator
for multiple statements on one line. A final expression immediately before the
closing brace is the block's value; a newline before `}` does not terminate it.
An explicit trailing semicolon discards that final expression's value, so an
empty block or a block containing only terminated statements has no value.

Line breaks are ignored inside parentheses and brackets and whenever the
surrounding tokens require continuation: after or before a binary operator,
after a comma or dot, and before `else`, `where`, or `in`. An opening delimiter
on a new line starts a new expression; keep a call's opening parenthesis or an
index's opening bracket on the preceding line. Blank lines and comments do not
create extra statements. Consequently,
both layouts below form one expression:

```panackelty
total: Nat = left +
  right

same: Nat = left
  + right
```

A declaration or assignment immediately before `}` needs no separator.
Adjacent statements on the same physical line remain invalid unless separated
by `;`; this keeps statement boundaries deterministic without indentation-based
parsing.

An `if` used as a value has two block branches. Both branches must produce
compatible values:

```panackelty
pure label(ready: Bool): Str {
  if ready { "ready" } else { "waiting" }
}
```

The `else` branch may be omitted when the conditional is used for control flow.
An `if` without `else` has type `Void`, and any tail value produced by its body
is discarded. It therefore cannot initialize a binding, become an argument, or
provide the result of a non-`Void` function. Only the selected body executes.

```panackelty
main(): Void {
  if file_exists("settings.txt") {
    print("settings found")
  }
}
```

## Generic source functions

Functions may declare type parameters after their name. The compiler checks the
body once with those parameters treated as abstract types, including functions
that are never called. Type parameters are scoped to their function's signature
and body; duplicate names and names that conflict with existing types are
rejected. `main` cannot have type parameters.

```panackelty
pure identity[T](value: T): T { value }

pure choose[T](values: [T], fallback: T): T {
  if values.len() == 0 { fallback } else { values[0] }
}

main(): Void {
  number: Nat = identity(42)
  text: Str = identity[Str]("hello")
  fallback: Str = choose([], "empty")
  empty: [Nat] = identity[[Nat]]([])
}
```

A call either supplies every type argument in declaration order or infers them
from its value arguments. Inference gathers evidence from all arguments,
including nested arrays, records, enums, and callable signatures, before
checking each argument against the substituted parameter type. It uses the
existing type-join rules, including `Nat`/`Int` widening and incomplete empty
collection or constructor evidence. Incompatible evidence is rejected even if
later arguments would otherwise hide the conflict. Explicit type arguments are
fixed and are never widened by argument inference.

Every type parameter must be determined. Return annotations, later assignments,
and later uses do not provide inference evidence. A return-only parameter, or
one supported only by an empty array or payload-free constructor, therefore
requires explicit type arguments. For example, `identity([])` is ambiguous,
even in a binding annotated `[Nat]`; `identity[[Nat]]([])` is valid. Ordinary
argument assignability and guarded-type proofs still apply after substitution.
`Void` is not a valid type argument or value argument.

Receiver-first calls may supply type arguments as `value.identity[Str]()`,
which means `identity[Str](value)`. Brackets immediately after a call name are
parsed as a type-argument list when their matching closing bracket is followed
by `(`; ordinary array indexing remains unchanged. Explicit type arguments
currently apply only to generic user functions, not built-ins or constructors.

Unconstrained parameters support operations justified by their declared shape:
a `T` can be returned, stored, compared for equality, or passed to another generic
function; a `[T]` can be indexed or iterated. Numeric arithmetic, ordering, field
access, and other operations needing a more specific type reject an abstract
`T`. A concrete shape such as `Box[T]` permits its declared field access.
Purity remains part of the function and callable contracts; type arguments do
not permit an effectful call from a pure body.

Generic functions may recurse and call other generic functions. They compile to
one ordinary function body with type arguments erased; tagged runtime values
and the existing version-8 call instructions supply execution. No specialisation,
new opcode, or bytecode version is required. Taking a generic function reference
with `@name` is deferred; use a non-generic wrapper when a concrete callback is
needed. Constraints, traits, higher-rank polymorphism, partial type arguments,
and inference from the expected return type are also deferred.

## Guarded types

```panackelty
type Positive = Int where value > 0
type Port = Nat where value >= 1 && value <= 65535
```

A guarded type is a nominal refinement of a primitive scalar. `value` denotes
the candidate value. The initial decidable guard language contains literals,
`value`, comparisons, `&&`, `||`, and arithmetic.

The checker accepts a conversion only when it can prove the guard from a
literal or from facts introduced by a surrounding `if`. It does not silently
insert a runtime check. A future `check<T>(expression): Result<T, GuardError>`
operation will be the explicit boundary for untrusted values.

This separation is important: arbitrary user code in guards would make type
checking non-terminating. Guards therefore remain total, pure, and within a
documented decidable fragment.

## Effects and purity

Functions are effectful unless declared `pure`:

```panackelty
pure twice(n: Nat): Nat { n * 2 }
ask(): Str { read_line() }
```

The effect is part of the checked function signature. A pure function may call
only pure functions and cannot invoke terminal I/O, file I/O, or clock-reading built-ins.
Arguments are values, so a pure function may receive text previously read by an
effectful caller and compute with it; it cannot itself perform or conceal I/O.

Local mutation is not an externally observable effect. A pure function may use
`mut` bindings, assignment, `while`, and `for`; it still cannot perform I/O or
call an impure function from inside those constructs. Function parameters and
bindings without `mut` are immutable.

Function names become first-class callable values only through an explicit
reference expression, `@name`. A declared function with parameters `A, B` and
result `R` has type `PureFn[A,B,R]` when declared `pure`, otherwise
`Fn[A,B,R]`. The final type argument is always the result, so a zero-argument
function uses `PureFn[R]` or `Fn[R]`. `PureFn` is assignable to the corresponding
`Fn`, but not conversely. Constructors and built-ins are not referenceable in
this initial model, and callable values do not capture local state.

A callable is invoked with `callback.call(arguments)`. Argument and result
types come directly from its callable type. Calling an `Fn` is an effect, while
calling a `PureFn` is permitted in pure code. These rules make effect checking
independent of the eventual runtime target and keep named-reference inference
deterministic.

Built-ins:

- `print(value): Void` — effectful
- `read_line(): Str` — effectful
- `read_file(path: Str): Str` — effectful
- `write_file(path: Str, contents: Str): Void` — effectful
- `read_bytes(path: Str): Bytes` — effectful
- `write_bytes(path: Str, contents: Bytes): Void` — effectful
- `len(text: Str): Nat` — pure
- `command_args(): [Str]` — effectful snapshot of program arguments
- `environment_has(name: Str): Bool` and `environment_get(name: Str): Str` —
  effectful access to the VM's environment snapshot; `environment_get` traps
  when the key is absent
- `eprint(value): Void` and `process_exit(code: Nat): Void` — effectful process boundary
- `path_resolve`, `file_exists` — effectful filesystem queries
- `path_parent`, `path_join`, `path_suffix`, `path_with_suffix`, and
  `path_is_absolute` — pure lexical path operations
- `run_bytecode(data: Bytes): Void` — effectful verified nested VM execution
- `run_bytecode_args(data: Bytes, arguments: [Str]): Void` — effectful verified
  nested execution with an explicit argument snapshot

The terminal, file, environment, argument, process, and nested-execution calls
form the stable VM-to-host ABI specified in
[`src/runtime/ABI.md`](src/runtime/ABI.md). Collection, text, byte, conversion,
and lexical path operations are deterministic VM primitives rather than host
services.

Text reads require valid UTF-8, while binary reads preserve every byte. Missing,
denied, or otherwise unusable filesystem paths produce an `I/O error`. Paths
containing an embedded NUL cannot be represented by the native host boundary
and trap before any filesystem operation rather than being truncated.

## Standard library

The canonical library is an explicit module graph imported as
`stdlib/prelude`; no module is imported implicitly. It defines
`Option[T]` with `None`/`Some` and `Result[T,E]` with `Ok`/`Error`. Portable
source wrappers provide the `text_*`, `bytes_*`, and checked
`environment(name): Option[Str]` APIs listed in `src/stdlib/README.md`.

Persistent array, map, and set operations and the `path_*` operations retain
their compiler-known polymorphic signatures. They are part of the standard
library surface: storage operations still execute as VM primitives, while array
`map` and `reduce` retain their compiler lowering. Source generics provide
`option_value_or[T]`, `result_value_or[T,E]`, and `array_first[T]` without new
primitives. Host access remains limited to the ABI calls identified above.

`len` also accepts arrays and byte buffers.

## Strings

`Str` values are Unicode text. They concatenate with `+`, and interpolation uses
`${name}` for a local scalar variable:

```panackelty
pure greeting(name: Str, attempts: Nat): Str {
  "Hello, ${name}; attempt ${attempts}"
}
```

Interpolation accepts `Nat`, `Int`, `Dec`, `Rat`, `Unit`, `Str`, `Bool`, and guarded scalar
types. It is pure string construction, not I/O. Indexing uses a `Nat` and returns
a one-character `Str`. Currently, “character” means a Unicode code point,
not a user-perceived grapheme cluster; both indexing and `len` use that same
definition. `text.len()`, `text.slice(start, end)`,
`text.starts_with(prefix)`, and `text.reverse()` are pure receiver-first
spellings of their built-ins. `reverse` operates on Unicode code points, so it
preserves each code point while reversing their order; it does not attempt
grapheme-cluster segmentation. The VM traps on an out-of-bounds string index.

## Ranges, arrays, and loops

Ranges are half-open and currently use natural-number bounds:

```panackelty
for value in 0..10 {
  // visits 0 through 9
}
```

Range expressions may be bound to inferred locals for later iteration.

Arrays are immutable, homogeneous values. Their type is written `[T]`. Indexes
are `Nat`, and the VM traps with a useful message if an index is out of bounds.
The compiler infers non-empty array literals. An empty literal `[]` is accepted
when an assignment or return context supplies its element type.

```panackelty
pure sum(values: [Nat]): Nat {
  mut total: Nat = 0
  for value in values {
    total = total + value
  }
  total
}
```

`while` conditions must be `Bool`. Facts established by the condition are
available while checking the body, so guarded operations such as decrementing a
`Nat` inside `while cursor > 0` can be proven safe.

## Records and tagged unions

Records are nominal product types. Construction is positional in declaration
order, while access is by field name:

```panackelty
record SourcePos { offset: Nat, line: Nat, column: Nat }

pure next_column(position: SourcePos): Nat {
  position.column + 1
}
```

Enums are nominal tagged unions. Every variant is a pure constructor, including
zero-payload variants, which are called with `()`:

```panackelty
enum OptionNat { None, Some(Nat) }

pure value_or(option: OptionNat, fallback: Nat): Nat {
  match option {
    Some(value) => value,
    None() => fallback
  }
}
```

Match expressions must cover every variant exactly once. Payload bindings are
immutable and receive their declared types. All arms must produce compatible
types. Record construction, variant construction, field access, and matching
are pure operations and therefore remain available inside pure functions.

Records and enums may declare type parameters:

```panackelty
record Pair[A, B] { first: A, second: B }
enum Option[T] { None, Some(T) }
enum Result[T, E] { Ok(T), Error(E) }
```

Constructor arguments infer type parameters. A constructor with no evidence,
such as `None()`, carries an unresolved type parameter that must be resolved by
assignment, a function return type, or another branch. Generic representations
are erased to their nominal record or enum representation in VM bytecode.

## Persistent collections and bytes

An empty array receives its element type from context:

```panackelty
mut tokens: [Token] = []
tokens = tokens.append(token)
```

`append` and `concat` return new arrays and are pure; existing arrays are not
modified. `Map[K,V]` provides `map.put(key, value)`, `map.has(key)`, and
`map.get(key)`. `Set[T]` provides `set.add(value)` and `set.has(value)`. Updates
return new collections rather than mutating their receivers. Map keys and set
elements are currently restricted to scalar types, including `Rat` and `Unit`. `get` traps when a key is
missing, so callers should use `has` until generic optional lookup is added.

The original `map_put`, `map_has`, `map_get`, `set_add`, and `set_has`
free-function spellings remain supported as a deliberate bootstrap and source
compatibility layer. The concise names are available only after a dot, avoiding
new conflicts in the global function namespace. Array `append` and `concat`
retain both equivalent free and receiver-first spellings under the uniform
call-lowering rule.

Arrays also provide pure higher-order operations. `values.map(callback)`
requires `callback: PureFn[T,U]` for `values: Array[T]` and returns a new
`Array[U]`. `values.reduce(initial, callback)` requires
`callback: PureFn[A,T,A]` and returns the accumulator type `A`. The callback
signature determines the mapped result and confirms the accumulator without
implicit coercion. Both operations traverse from left to right, evaluate the
receiver, initial value, and callback once, and leave the original array
unchanged. Effectful callbacks are rejected.

`Bytes` is an immutable byte buffer. Pure operations include `bytes`,
`byte_append`, `bytes_concat`, `byte_len`, `byte_get`, `utf8_encode`, and
`utf8_decode`. `read_bytes` and `write_bytes` are effectful. These operations are
the foundation for moving the bytecode serializer into Panackelty.

Lexer-oriented pure string operations include `slice`, `starts_with`,
`starts_with_at`, `is_digit`, `is_letter`, `is_whitespace`, and
`nat_from_str`. These deliberately recognize the ASCII lexical classes used by
Panackelty source: digits `0`–`9`, letters `A`–`Z`/`a`–`z`, and space, tab,
carriage return, and newline. Other Unicode code points remain valid `Str`
content but do not belong to these lexer classes.

Boolean `&&` and `||` short-circuit. This is semantically significant for safe
bounds checks in lexers and parsers.

## Modules

Modules support quoted file-relative imports and logical imports:

```panackelty
import "token.panack"
import project/shared/diagnostic
import stdlib/option
```

Quoted paths without a reserved logical prefix are resolved relative to the
importing file and must end in `.panack`. Logical imports use slash-separated
identifier segments. Their canonical spelling is unquoted and extensionless;
quoted logical paths and a terminal `.panack` suffix are accepted for explicit
compatibility. `project/` is rooted at the directory containing the entry
source file, while `stdlib/` is rooted at the standard library bundled with the
active toolchain. These namespaces are reserved, so a project file cannot
shadow a standard-library module and resolution does not use a search-path
precedence rule.

Empty segments, `.` or `..`, non-identifier segments, other suffixes, and
absolute file imports are rejected. Resolved paths are canonical module
identities: importing the same file through extensionless and suffixed logical
spellings still loads it once. All imports are checked for cycles and duplicate
declarations. The current module system combines declarations into one program
namespace; visibility, selective imports, third-party packages, and configurable
project roots are still pending.

## Compilation and the Panackelty VM

Panackelty source files use the `.panack` extension. Saved bytecode artifacts
use `.bc`.

Execution begins at `main()`. The VM is the only execution engine: `panack run`
compiles `.panack` source to bytecode in memory before starting the VM. `panack
compile program.panack` persists the same bytecode as `program.bc`, and `panack run
program.bc` loads, verifies, and executes that artifact directly.

The compiler emits stack instructions with a named function table and purity
metadata. Calls use isolated frames containing locals and an operand stack.
Values retain runtime tags, and VM arithmetic checks `Nat` underflow even after
static checking as a bytecode-safety measure.

The serialized format begins with the `PANACKBC` magic header and a numeric format
version. Version 8 uses the compact typed binary payload introduced by version
5, containing function
signatures, purity flags, and instruction streams; `main` is the implicit entry
point. The VM uses
an internal `Void` sentinel to keep call and return mechanics uniform, but it is
not exposed as a source value. Before execution the bytecode verifier rejects
unknown opcodes, malformed constants and operands, invalid control-flow targets,
calls with incorrect arity, missing functions, and pure functions that call
impure functions.

Version 8 retains the instruction, tagged-value, isolated-call-frame,
control-flow, and trap semantics introduced by version 4, the binary encoding
introduced by version 5 and the method/string additions from version 6. It adds
verified indirect callable invocation for explicit named references, as
specified by `src/bytecode/FORMAT.md`.
Structurally valid bytecode that reaches invalid dynamic state must trap rather
than expose a host implementation exception.

The format also has versioned resource limits for artifact size, table and
instruction counts, names, text, numeric encodings, and operand collections.
These protect the untrusted loader and verifier without changing
the source language's arbitrary-precision numeric model.

Within one bytecode version, serialization is canonical: function records are
ordered by ascending Unicode function name and all nested sequences retain
deterministic compiler traversal order. Identical source and dependency inputs
must therefore produce byte-identical artifacts, and loading then reserializing
a canonical artifact must preserve its bytes.

`panack disasm` accepts either `.panack` or `.bc` and prints the VM instruction stream.
The format is versioned but not yet declared stable across Panackelty releases.

The self-hosted compiler implements the same `check`, `compile`, `run`, and
`disasm` command behavior. Its project loader resolves file-relative,
project-root, and toolchain-standard-library imports, canonicalizes module
identities, loads each module once, and rejects missing modules, invalid paths
or suffixes, cycles, and duplicate declarations before emission.

## Diagnostics

Primary lexer, parser, name-resolution, and type-checking failures are rendered
as `file:line:column: message`. Lines and columns are one-based, and the file is
the canonical path of the source module that owns the failing token or
expression. This location is preserved when the failure originates in an
imported module.

For a valid position in a loaded source snapshot, the header is followed by the
numbered source line and a single caret under the reported character. Columns
in the header count Unicode codepoints. Excerpts expand tabs to four-column
stops, double backslashes, and display non-ASCII and control characters as
lowercase `\u{hex}` escapes, keeping caret alignment independent of terminal
Unicode width. CR immediately before LF is omitted; a bare CR is escaped.
A position one character past the line points after its last character,
including an empty line. Missing source or an invalid line/column retains the
header alone. Rendering uses the text read by the loader, without rereading files.

Secondary labels, stable error codes, and automated fixes are not yet included. Loader, entry-point,
and declaration-wide failures may remain message-only when there is no single
source expression to identify.

## Deliberately postponed

- Mutable array elements and growable collections
- Module visibility, selective imports, and package management
- Explicit checked construction from untrusted data
- Generic constraints, traits, and higher-rank polymorphism
- A backwards-compatibility guarantee for bytecode versions
- Concurrency

The immediate proving ground is a sequence of Project Euler solutions. Features
should be added when those programs demonstrate a concrete need.

## Typed filesystem, process, and sleep APIs

Import `stdlib/filesystem`, `stdlib/process`, or `stdlib/time`; `stdlib/prelude`
includes all three. These APIs use the existing opaque `Path` and `Duration`
values. Host calls are effectful, including metadata, enumeration, and sleep.
Only `host_decode_utf8` is pure.

Failures return `Result[T,HostError]`, where
`HostError { operation: Str, code: Str, native_code: Nat }` identifies the service,
a portable category, and POSIX errno (zero for language-defined failures).
Categories are `not_found`, `permission_denied`, `already_exists`, `not_directory`,
`is_directory`, `not_empty`, `symlink_loop`, and fallback `io_error`; validation
and resource failures additionally use `out_of_range`, `negative_duration`,
`invalid_utf8`, `invalid_argument`, `invalid_environment`, `not_regular_file`,
`output_limit`, `timeout`, `launch_failed`, `clock_unavailable`, or `out_of_memory`.
Native error numbers are diagnostic details, not portable branching contracts.
Malformed bytecode operands still trap; allocation failure may trap rather than
allocate an error value.

### Filesystem operations

| Function | Result |
| --- | --- |
| `fs_read(path: Path, limit: Nat)` | `Result[Bytes,HostError]` |
| `fs_write(path: Path, contents: Bytes)` | `Result[Unit,HostError]` |
| `fs_metadata(path: Path)` | `Result[FileMetadata,HostError]` |
| `fs_list(path: Path)` | `Result[[Path],HostError]` |
| `fs_create_directory(path: Path)` | `Result[Unit,HostError]` |
| `fs_remove_file(path: Path)` | `Result[Unit,HostError]` |
| `fs_remove_directory(path: Path)` | `Result[Unit,HostError]` |
| `fs_temp_file(parent: Path)` | `Result[Path,HostError]` |
| `fs_temp_directory(parent: Path)` | `Result[Path,HostError]` |
| `host_decode_utf8(contents: Bytes)` | `Result[Str,HostError]` |

Reads and writes accept regular files, follow symlinks, and preserve bytes.
Reads fail without partial data when their explicit byte limit is exceeded.
Writes create or truncate a file and may leave partial contents on failure;
they promise neither atomic replacement nor durable storage. New ordinary file
and directory permissions are 0666 and 0777 filtered by the caller's umask.
File data and read limits are capped at 16 MiB. Special files are rejected after
opening with nonblocking flags; these calls are not a general stream API.

`FileMetadata { kind: FileKind, size: Nat }` describes the final directory entry
without following a final symlink. `FileKind` is `RegularFile`, `Directory`,
`SymbolicLink`, or `OtherFile`. Size is the host's nonnegative byte-size field,
not a recursive directory total. As with POSIX `lstat`, trailing separators and
intermediate symlinks follow host path-resolution rules.

Enumeration returns immediate relative child names, excluding `.` and `..`, in
unsigned native-byte lexicographic order. Join a returned name with its parent
using `path_append`. Names are never decoded implicitly; arbitrary native bytes
round-trip where the host filesystem permits them. Enumeration is capped at
65,536 entries and 16 MiB of names and fails without a partial list. Concurrent
filesystem changes can affect results; enumeration is not a snapshot.

Directory creation creates one level and fails if the entry exists. File removal
unlinks a file or symlink; directory removal requires an empty directory. Neither
recursively deletes entries. Temporary creation atomically chooses a unique name
under the explicit parent, with permissions 0600 for files and 0700 for
directories, subject to umask. The caller owns the returned path and must remove
it explicitly. There is no automatic cleanup or lifetime tracking.

Paths do not establish containment. Relative paths use the invocation's working
directory; `..`, symlink traversal, and races remain POSIX host behavior. Recursive
operations and secure descriptor-relative traversal remain separate follow-ups.

### Process execution

```panack
process_run(
  executable: Path, arguments: [Str], stdin: Bytes, working_directory: Path,
  environment: [Str], timeout: Duration, output_limit: Nat
): Result[ProcessOutput,HostError]
```

`ProcessOutput { exit_code: Nat, signal: Nat, stdout: Bytes, stderr: Bytes }`
represents completed execution. A normal exit has `signal == 0`, including a
nonzero exit code. Signal termination has a nonzero signal number and
`exit_code == 0`. Failure before execution returns `launch_failed`, with errno;
timeout and output exhaustion return their own error categories, without partial
output. There is no implicit UTF-8 decoding: use `host_decode_utf8` explicitly.

The executable is an exact path, resolved relative to the requested child
working directory when relative. There is no PATH search or implicit shell.
Arguments exclude argv[0], are UTF-8 encoded, and reject NUL. Explicitly invoking
`/bin/sh` opts into that shell's parsing. Environment entries use `NAME=value`
and override the invocation's captured environment only in the child; empty
names, missing `=`, duplicate names, and NUL are rejected. Empty values are
allowed. Environment deletion and native-byte argument construction are deferred.

The parent concurrently writes stdin and drains stdout and stderr, closing stdin
when exhausted or when the child closes it. The output limit applies to the sum
of both streams; no ordering between streams is promised. Input and combined
output limits are capped at 16 MiB; argument and environment lists at 4096 entries
each. OS argument-size limits can still cause `launch_failed`.

Timeout covers launch and stream collection using `CLOCK_MONOTONIC`. A zero
timeout fails before launching. At timeout or another collection failure, the VM
closes its streams, requests SIGKILL for the child's process group and direct
child, and reaps the direct child. The oracle falls back to direct-child killing
where a sandbox forbids group signalling. Descendants that escape the process
group are not contained; this is not a process sandbox. Descendants holding
output streams open remain subject to the timeout. Deadlines are scheduling
bounds, not hard real-time guarantees; host launch, termination, and cleanup can
add latency.

### Sleep and timing bounds

`host_sleep(duration: Duration): Result[Unit,HostError]` waits for a nonnegative
duration, retries interrupted sleeps, and accepts zero. Sleep and process timeout
accept at most 31,536,000 seconds (365 days), expressed as exact nanoseconds;
negative or larger values return `negative_duration` or `out_of_range` before any
sleep or process launch. Duration arithmetic itself remains arbitrary precision.
System-suspension accounting follows the host clock and sleep implementations;
no portable across-suspension deadline promise is made.
