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
Bounds about a mutable local are invalidated when it may be assigned, including
writes in nested expressions and branches. Loop-carried writes invalidate
pre-loop bounds; a loop condition or a later guard can establish fresh bounds.
Within a compound expression, the checker conservatively drops incoming bounds
for any local the expression may write, even when a particular evaluation path
would preserve them. Split such computations and establish a fresh guard when
needed. The right-hand side of a direct assignment can use the valid pre-write
bound, so a guarded `n = n - 1` remains supported.

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

`value.nat()` accepts `Rat` and returns `Nat` only when the
value is integral and non-negative; otherwise it traps without truncating.
`value.dec()` accepts `Rat` and returns its exact finite
`Dec` representation, or traps when the reduced denominator contains factors
other than two and five. Rounded decimal conversion and checked `Result`
conversion APIs remain deferred. These operations are pure despite being partial.

`a.divide(b)` accepts two `Nat` values and returns their truncated natural
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

Import `stdlib/path` as a module, for example `import stdlib/path as path`.
The initial native
representation is POSIX bytes on the supported Linux and macOS targets. Text
construction encodes UTF-8; native construction preserves arbitrary bytes.
Both reject empty input (`EmptyPath`) and embedded NUL (`PathContainsNul`).
Neither construction nor any lexical operation below accesses the filesystem.
A `Path` does not establish existence, access permissions, canonical identity,
or containment within a directory.

| Function | Result and behavior |
| --- | --- |
| `path.from_text(Str)` | `Result[Path,PathError]`; checked UTF-8 construction |
| `path.from_native(Bytes)` | `Result[Path,PathError]`; checked native-byte construction |
| `path.current()` | `Path` containing `.`; does not capture the working directory |
| `value.native_bytes()` | Exact native `Bytes` |
| `value.to_text()` | `Result[Str,PathError]`; `PathNotUtf8` on invalid UTF-8 |
| `value.display()` | ASCII `Str`; escapes backslash, controls, and non-ASCII bytes as lowercase `\xhh` |
| `value.is_absolute()` | `Bool`; whether the first byte is `/` |
| `base.join(child)` | `Result[Path,PathError]`; rejects an absolute right operand with `AbsolutePathAppend` |
| `value.directory()` | Lexical parent `Path`; a bare component has parent `.` and a root is its own parent |
| `value.filename()` | `Option[Path]`; last nonempty component, or `None` for a root |

Appending inserts `/` only when the left operand does not already end with it.
No operation collapses `.` or `..`, resolves symbolic links, changes case, or
silently makes a relative path absolute. Parent and filename extraction ignore
trailing separators and retain leading root separators (including `//`). Dot
components remain literal components. Equality compares native bytes exactly:
`a` and `./a` are unequal even when they refer to the same file. Display is for
humans, not a serialization or shell-escaping format; printing an unconverted
Path produces `<Path>`.

The standard-library
`path` module exposes only the typed `Path` operations above. The older
compiler-known Str path identities remain available for bootstrap internals;
their string-based adapter functions are not re-exported by the standard
library. In particular, the legacy `path_join` intrinsic normalizes its result,
while `path.join` never does. Typed filesystem queries and I/O are specified
below. Non-POSIX representations remain follow-up work.

### Exact durations

Import `stdlib/time`. A `Duration` stores a signed arbitrary-precision integer
number of nanoseconds. Negative durations are valid, including differences
between an expired deadline and the current instant. Sleep and process timeout APIs validate nonnegative values and host limits
at their boundaries, as specified below.

| Function | Result and behavior |
| --- | --- |
| `time.from_nanoseconds(Int)` | Exact `Duration` |
| `time.milliseconds(Int)` / `time.seconds(Int)` | Exact scaled `Duration` |
| `duration.ticks()` | Signed `Int` nanoseconds |
| `time.from_seconds(Rat)` | `Result[Duration,DurationError]`; rejects fractional nanoseconds |
| `duration.as_seconds()` | Exact `Rat`, without rounding |
| `left.add(right)` / `left.subtract(right)` | Exact `Duration` |
| `duration.scale(factor)` | Exact `Duration` |
| `duration.divide(divisor)` | `Result[Duration,DurationError]`; requires an exact integer number of nanoseconds |
| `left.ratio(right)` | `Result[Rat,DurationError]`; exact dimensionless ratio |
| `left.is_before(right)` | `Bool`; signed less-than comparison |

`DurationError` is `FractionalNanosecond` or `ZeroDurationDivisor`.
Construction from seconds accepts an explicitly rational expression such as
`3/2`; decimals do not implicitly convert to rationals. The type has no fixed
integer overflow boundary, subject to available memory. Printing a duration
produces its signed nanosecond count followed by `ns`. Nanosecond representation
does not promise nanosecond clock resolution or accuracy. Rounded conversions
are not provided; use exact rational seconds or explicitly calculate integer
nanoseconds.

### Monotonic instants

`time.now(): Result[Instant,ClockError]` reads the host's `CLOCK_MONOTONIC`.
It is effectful, including when used through other functions. Host read failure
returns `Error(ClockUnavailable())`. All other operations here are pure:

| Function | Result and behavior |
| --- | --- |
| `instant.add(duration)` | A shifted `Instant`, including for negative durations |
| `left.difference(right)` | Signed `Duration`: first operand minus second |
| `left.is_before(right)` | `Bool`; strict less-than comparison |

All instants obtainable within one program execution share one monotonic clock
domain. Repeated reads may be equal; later reads do not precede earlier reads.
Clock readings are independent of calendar-clock corrections. Whether time
spent in system suspension is counted follows the host's monotonic-clock
contract and is not a portable guarantee. Deadlines shifted before the clock's
origin remain valid arithmetic values; they do not change that origin.

There is no public epoch, tick accessor, wall-clock conversion, integer
constructor, serialization, or cross-execution comparison. Printing an instant
produces `<Instant>`. These calls were introduced in version 8; version 9 retains
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

<a id="type-owned-methods-next"></a>

### Type-owned methods (alpha.14)

Methods use `receiver.method(arguments)`. Lookup selects a declaration owned by
the receiver's checked type; an ordinary function with the same name cannot
capture the call. The receiver and explicit arguments are evaluated once, in
that order. Namespace calls such as `time.now()` and record field access such as
`box.value` retain their meanings. A lexical value shadows a namespace alias;
a failed namespace lookup never becomes a method call, and a failed method lookup
never becomes a namespace or global call. Selectively imported declarations
retain their local-binding collision checks.

`value.name(...)` selects a method even when a callable field has that name.
Invoke that field explicitly with `(value.name).call(...)`. The temporary
`receiver->method(...)` syntax is rejected. Compiler, library and maintained
application sources use owned dots.

Core provides owned string, array, collection, byte, numeric, path, duration,
instant and callable operations. For example, `items.append(42).len()` and
`values.put("key", 1).get("key")` resolve through owned declarations. The old
receiver-function globals and module wrappers are removed. Use a typed receiver
when an empty collection cannot supply its element type: `items: [Str] = []`,
then `items.first()`. Generic owner arguments belong to the receiver type.

```panack
record Fruit { value: Nat }

impl Fruit {
  pure apple(): Nat { self.value }
  pure banana(): Nat { self.value + 1 }
}

main(): Void {
  first: Fruit = Fruit(10)
  second: Fruit = Fruit(20)
  print(first.apple())
  print(first.banana())
  print(second.apple())
  print(second.banana())
}
```

This prints `10`, `11`, `20`, `21` on separate lines. Each method belongs to its
record type; the receiver is supplied as immutable `self`. Arrow lookup uses the
receiver's checked nominal type and never falls back to an ordinary global
function. Different record types can own methods with the same name. The receiver
and explicit arguments are evaluated once, in that order. Ordinary argument,
return-type and purity checking still applies. Methods may call other methods
through `self.name()`.

Methods are private to their defining module unless declared `pub`. Exporting a
record and importing it is sufficient to call its public methods; no separate
method import is required. For example, `fruit.panack` can contain:

```panack
pub record Fruit { value: Nat }

impl Fruit {
  pub pure apple(): Nat { self.value }
  pub pure banana(): Nat { self.next_value() }
  pure next_value(): Nat { self.value + 1 }
}
```

A caller can import and instantiate it with `import "fruit.panack" as fruit` and
`first = fruit.Fruit(10)`, then call `first.apple()` or `first.banana()`.
`first.next_value()` is rejected outside the defining module. Public methods
may use private methods within their implementation. Public methods require a
public owner type; existing public-signature accessibility checks apply.

Namespace aliases, renamed selective imports and re-exports retain the original
nominal owner identity. Neither a same-name local type nor a same-name global
function changes owned-method lookup. Visibility is checked against the original
method declaration, including when the receiver type is re-exported.

Records and enums may own synchronous methods, including generic owners and
methods. The owner must be declared in the same module, before or after its impl.
Implementation binders are explicit and must occur exactly once as the owner's
type arguments; their names may differ from the type declaration's binders:

```panack
impl[T] Box[T] {
  pub pure value(): T { self.item }
  pub pure replace[U](item: U): Box[U] { Box(item) }
  pub pure empty[U](): [U] { [] }
}

pub record Box[T] { item: T }
```

`Box(42).replace[Str]("hello").value()` returns `"hello"`. The receiver
supplies implementation parameters; explicit call type arguments supply only
method parameters. Method parameters can also be inferred from arguments or the
expected result, as in `values: [Str] = Box(42).empty()`. Known nominal receivers
may retain argument holes until method arguments or surrounding result constraints
solve them; an unresolved
receiver or method type parameter is rejected before emission. Lookup never
chooses an owner from the method's spelling.

Implementation and method binders must be disjoint and individually unique.
Specialised impls such as `impl Box[Nat]`, unused implementation binders, explicit
`self` parameters, assignment to `self`, and duplicate methods across impl blocks
are rejected. There is no overloading. The canonical core Option and Result enums
support this declaration mechanism in their defining toolchain module; production
stdlib receiver API migration is separate work and is not yet delivered.

Only the designated toolchain core module can declare source methods for the
registered built-in families Nat, Int, Dec, Rat, Bool, Unit, Str, Bytes, Path,
Duration, Instant, Array, Map and Set. Array implementations use `impl[T] [T]`
(or `impl[T] Array[T]`); Map and Set use `impl[K,V] Map[K,V]` and `impl[T] Set[T]`.
These owners are canonical family tags, distinct from nominal declarations.
The same binder, privacy, purity and duplicate-method rules apply. A user package
or module named `core.panack` cannot grant itself built-in ownership. The compiler supplies owned intrinsic methods for Map, Set, arrays, text, bytes,
Nat/Rat and time values; the complete list and acceptance evidence are in the
[P06 report](../design/type-owned-dispatch/p06-owned-intrinsics.md). These declarations
have canonical owner identity and typed signatures. Collection and primitive
operations are pure; callable invocation preserves its receiver effect. Existing
primitive argument checks, collection behaviour and exact-conversion traps are
preserved. Source implementations cannot replace registered intrinsics. Arrays
also provide owned `map` and `reduce`, requiring pure callbacks with the existing
argument/result contracts. `PureFn`, `Fn` and `AsyncFn` each own `call`; its arity,
arguments and result are specialised from the receiver's callable signature.
`await callback.call(args)` retains the existing async boundary. Invoke a callable
field explicitly as `(value.field).call(args)`.

Core also defines source-owned `Str.ends_with`, `Array[T].first/sort_by`,
`Option[T].value_or`, `Result[T,E].value_or`, Path's join/text/native-byte/display/
absolute/directory/filename operations, and Duration's add/subtract/scale/
is_before/as_seconds/divide/ratio operations. Existing Unicode, stable sorting,
exact arithmetic, native-byte and error contracts are preserved. Path factories,
time factories and clock services remain explicit module functions.

`PathError` and `DurationError` have one canonical declaration in core. The path
and time modules re-export those types and their variants, preserving existing
imports. `ClockError` remains in time. P12 removes the legacy module wrappers
and core forwarding helpers; callers use the owned declarations directly.
See the [P08 report](../design/type-owned-dispatch/p08-core-methods.md).

This does not add implementations for imported nominal types, extension methods,
traits, inheritance or dynamic dispatch. Async methods use ordinary `async`
declarations and must be invoked with `await` under the existing effect rules. Guarded nominal types do
not inherit their underlying built-in family's methods. Method declarations have
compiler-only identities; they do not introduce ordinary callable names. The project-aware CLI is required, including its regular
source-file commands or explicit `--project` commands. Standalone parser/backend APIs do not establish executable source validity;
source acceptance requires module identity checking. Method diagnostics use the source owner and
method spelling, such as `Fruit.apple`.
These method rules apply from alpha.14; alpha.13 used the earlier receiver-call contract.

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

Any number of `else if` arms may follow the first arm, with an optional final
`else` block. Conditions must be `Bool`; they are evaluated in written order until
one is true. Only that arm executes, and subsequent conditions and bodies are
skipped. A final `else` executes only when every condition is false.

```panackelty
pure classify(value: Nat): Str {
  if value == 0 {
    "zero"
  } else if value == 1 {
    "one"
  } else if value == 2 {
    "two"
  } else {
    "many"
  }
}
```

A chain with a final `else` follows the existing compatible branch-value rules.
Without a final `else`, the whole chain has type `Void`: selected arm tail values
are evaluated and discarded, so different arm result types are permitted. If no
condition matches, no body executes. A partial chain cannot initialize a binding,
become an argument, or provide a non-`Void` function result. Existing nested
`else { if ... }` syntax remains valid; an explicitly written else block retains
its ordinary block-value rules. Newlines/comments may separate `else` and `if`.
Each arm requires braces; the final `else` binds to its enclosing chain.

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
  text: Str = identity[Str](../../"hello")
  fallback: Str = choose([], "empty")
  empty: [Nat] = identity[[Nat]](../../[])
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
even in a binding annotated `[Nat]`; `identity[[Nat]](../../[])` is valid. Ordinary
argument assignability and guarded-type proofs still apply after substitution.
`Void` is not a valid type argument or value argument.

Receiver-first calls may supply type arguments as `value.identity[Str]()`,
which means `identity[Str](../../value)`. Brackets immediately after a call name are
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
and the existing ordinary call instructions supply execution. No specialisation,
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

The checker accepts a conversion only when it can prove the target guard from
the value's literal, facts introduced by a surrounding `if`, or the invariant
of a guarded source type with a compatible primitive base. A guarded source
does not prove a stricter target unless its invariant entails that target
guard. The checker does not silently insert a runtime check. A future
`check<T>(expression): Result<T, GuardError>` operation will be the explicit
boundary for untrusted values.

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
result `R` has type `AsyncFn[A,B,R]` when declared `async`,
`PureFn[A,B,R]` when declared `pure`, otherwise `Fn[A,B,R]`. The final type argument is always the result, so a zero-argument
function uses `PureFn[R]` or `Fn[R]`. `PureFn` is assignable to the corresponding
`Fn`, but not conversely. Constructors and built-ins are not referenceable in
this initial model, and callable values do not capture local state.

A callable is invoked with `callback.call(arguments)`. Argument and result
types come directly from its callable type. Calling an `Fn` is an effect, while
calling a `PureFn` is permitted in pure code. These rules make effect checking
independent of the eventual runtime target and keep named-reference inference
deterministic.

Ordinary built-ins are `print(value): Void`, `read_line(): Str`, and
`write_file(path: Str, contents: Str): Void`, alongside collection/byte constructors.
Other host operations require explicit service imports: `host.arguments`,
`host.stderr`, `host.exit`, `host.run`, `environment.get`, and filesystem APIs.
Compiler-only trapping file/path plumbing uses `compiler_files` and
`compiler_paths`; these do not restore global aliases. Text, collection and
conversion operations use owned methods.

The terminal, file, environment, argument, process, and nested-execution calls
form the stable VM-to-host ABI specified in
[`src/runtime/ABI.md`](../../src/runtime/ABI.md). Collection, text, byte, conversion,
and lexical path operations are deterministic VM primitives rather than host
services.

Text reads require valid UTF-8, while binary reads preserve every byte. Missing,
denied, or otherwise unusable filesystem paths produce an `I/O error`. Paths
containing an embedded NUL cannot be represented by the native host boundary
and trap before any filesystem operation rather than being truncated.

## Standard library

The standard library is an explicit module graph. Core `Option[T]` and
`Result[T,E]` types, their constructors, and built-in text/collection methods
are available in every source program. The curated `stdlib/prelude` re-exports
the `byte_buffer`, `path`, `environment`, `time`,
`filesystem`, and `process` modules. Host, testing,
package-filesystem, and TCP APIs remain separate modules. Import domain modules
and use short names within them, such as `path.from_text` or
`byte_buffer.empty`; the prelude uses `byte_buffer` to preserve the core
`bytes()` constructor.

Persistent array, map, and set operations retain their compiler-known
polymorphic signatures. Storage operations still execute as VM primitives,
while array `map` and `reduce` retain their compiler lowering. Option and Result own generic `value_or` methods; array `first()` is also
a core method. Host access remains limited
to the ABI calls identified above.

`len` also accepts arrays and byte buffers.

The separately imported `stdlib/testing` module supplies pure `expect`,
`equal_str`, and `equal_nat` assertions that return `TestResult`. Its effectful
`report` prints outcomes in caller-supplied order and returns the number of
failures; it does not choose an exit status.

The separately imported `stdlib/testing_files` module discovers immediate
directory children in the host's deterministic native-byte order and returns
full `Path` values; final symlinks and files are excluded. It propagates host
failures rather than returning a partial list. Its `TestWorkspace` record owns
an atomically created temporary directory. Callers remove its contents and
explicitly remove the empty directory; nonempty removal fails. This API does
not establish containment, a filesystem snapshot, or automatic cleanup.

`stdlib/testing_commands` uses the bounded `process.run` contract. Its
`TestCommand` holds the complete invocation, and `run_command` compares exit
code, signal, and both byte streams against a `ProcessOutput`. The first
mismatch is reported as a `TestResult`; host errors also become failed tests.
`expect_command_error` instead checks a portable host-error category. A
completed nonzero exit is distinct from a launch or collection failure. Pure
helpers can check already captured outputs or results without running a
command. No implicit shell, PATH search, decoding, or sandbox is added.

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
`text.starts_with(prefix)`, and `text.reverse()` are pure owned methods. `reverse` operates on Unicode code points, so it
preserves each code point while reversing their order; it does not attempt
grapheme-cluster segmentation. The VM traps on an out-of-bounds string index.

The import-free `value.ends_with(suffix): Bool` method is pure. It tests a literal,
case-sensitive suffix on Unicode code-point boundaries. Empty suffixes always
match; suffixes longer than the value never match. No normalization, locale
collation or wildcard interpretation is performed.

## Ranges, arrays, and loops

Ranges are half-open and currently use natural-number bounds:

```panackelty
for value in 0..10 {
  // visits 0 through 9
}
```

Range expressions may be bound to inferred locals for later iteration.

The import-free array method `values.sort_by(less)` accepts
`less: PureFn[T,T,Bool]` for `values: [T]` and returns `[T]`.
It returns a stable sorted array, leaving the input unchanged. The pure
comparator must define a strict weak ordering (true means strictly before);
these ordering laws are not checked. Equivalent elements keep their original
order. Empty and singleton inputs need no comparisons; comparator traps
propagate. No implicit natural-order comparator or locale collation is supplied.

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
enum OptionNat { NoNat, SomeNat(Nat) }

pure value_or(option: OptionNat, fallback: Nat): Nat {
  match option {
    SomeNat(value) => value,
    NoNat() => fallback
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
enum Choice[T] { EmptyChoice, Chosen(T) }
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

Receiver operations use owned dot calls.
The former global primitive aliases and receiver-to-global sugar remain rejected;
namespace calls still use dots.

Arrays also provide pure higher-order operations. `values.map(callback)`
requires `callback: PureFn[T,U]` for `values: Array[T]` and returns a new
`Array[U]`. `values.reduce(initial, callback)` requires
`callback: PureFn[A,T,A]` and returns the accumulator type `A`. The callback
signature determines the mapped result and confirms the accumulator without
implicit coercion. Both operations traverse from left to right, evaluate the
receiver, initial value, and callback once, and leave the original array
unchanged. Effectful callbacks are rejected.

`Bytes` is an immutable byte buffer, constructed with `bytes()` or
`stdlib/bytes.empty()`. Its pure owned operations include `append`, `concat`,
`length`, `at`, and `decode_utf8`; `Str.encode_utf8()` produces bytes.
File reads and writes use explicit filesystem services.

Lexer-oriented pure string methods include `slice`, `starts_with`,
`starts_with_at`, `is_digit`, `is_letter`, `is_whitespace`, and `parse_nat`.
The lexical class methods recognize ASCII digits, letters, space, tab,
carriage return and newline. Other Unicode code points remain valid `Str`
content but do not belong to these lexer classes.

Boolean `&&` and `||` short-circuit. This is semantically significant for safe
bounds checks in lexers and parsers.

## Modules

`Option[T]`, `Result[T,E]` and the constructors `None`, `Some`, `Ok` and
`Error` are implicitly available in every source program and imported module.
They are defined once in the bundled `core.panack`; redeclaring their names is
an error. The loader requires a configured stdlib root containing this core,
including when `PANACKELTY_STDLIB_PATH` is overridden. The public launcher
configures the bundled root. No host, testing, or domain-module helper is
implicitly imported. Option/Result `value_or`, text operations and numeric
conversions are owned methods. The former `stdlib/option`, `stdlib/result`,
`stdlib/text` and `stdlib/numeric` wrapper-only modules have been removed.

Modules support quoted file-relative imports and logical imports:

```panackelty
import "token.panack"
import project/shared/diagnostic
import stdlib/path
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

Empty logical segments, non-identifier logical segments, other suffixes,
absolute imports and paths outside the entry root are rejected. Quoted imports
that traverse a symlink are also rejected. Resolved files load once and retain
their source identity; cycles and duplicate declarations are checked. In
namespace projects, function, type and variant identities remain distinct
across modules, and aliases, re-exports and public visibility are checked before
emission. Checked programs receive deterministic private names; only the entry
module's `main` launches. The bytecode format remains v9.

Namespace execution is enabled for the documented checked body subset only.
Bodies with deferred type or proof obligations fail closed. Source-map and
explain/locate replay, coordinated compiler/standard-library API migration,
refreshed seed fixed points, and native/browser/installed acceptance pass. Project
compilation uses checked identity emission; incomplete closures fail closed, and
the legacy flattened project route has been removed. `parse_program_complete`
remains a single-program adapter and rejects staged syntax instead of applying a
flattened name model. Declaration signatures check tagged type/callable identity
and public reachability, including private type and guard-helper leaks. Function
bodies retain declaration/local identities and check ordinary expressions and
control flow, lexical interpolation, records/enums, arrays, field access,
exhaustive variant patterns, generic calls, core/indirect contracts, guard proofs,
effects and await. Constructor and empty-collection evidence is contextual;
user-function inference uses value arguments and preserves rigid caller
parameters. Nominal identities remain distinct inside nested types and patterns.
Shared unresolved evidence cannot acquire conflicting types at different uses,
and inferred locals require complete types before later statements. Mixed Rat/
integer joins and other explicitly deferred obligations remain distinct from
checked bodies. Identity emission consumes only fully checked bodies; unsupported
or deferred constructs cannot be emitted. See the [accepted design and migrated
implementation](../design/MODULE_PACKAGE_DESIGN.md).

## Compilation and the Panackelty VM

Panackelty source files use the `.panack` extension. Saved bytecode artifacts
use `.bc`.

Execution begins at `main()`. The VM is the only execution engine: `panack run`
compiles `.panack` source to bytecode in memory before starting the VM. `panack
compile program.panack` persists the same bytecode as `program.bc`, and `panack run
program.bc` loads, verifies, and executes that artifact directly.

The CLI drives ordinary main synchronously and async main through the same VM's
suspend/complete loop. Async services include a deterministic fake read and a
bounded native TCP exchange, finite concurrent TCP servers and continuous native services. There is no
source task-spawning syntax.

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
- Package management
- Explicit checked construction from untrusted data
- Generic constraints, traits, and higher-rank polymorphism
- A backwards-compatibility guarantee for bytecode versions
- Concurrency

The immediate proving ground is a sequence of Project Euler solutions. Features
should be added when those programs demonstrate a concrete need.

## Typed filesystem, process, and sleep APIs

Import `stdlib/filesystem`, `stdlib/process`, or `stdlib/time`; the prelude
re-exports these modules. These APIs use opaque `Path` and `Duration` values.
Host calls are effectful, including metadata, enumeration, and sleep. The
portable `text.decode_utf8` operation is pure and traps on invalid input;
`host.decode_utf8` returns a checked result.

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
| `package_filesystem.read_beneath(root: Path, relative: Path, limit: Nat)` | `Result[FilesystemRootRead,HostError]` |
| `filesystem.read(path: Path, limit: Nat)` | `Result[Bytes,HostError]` |
| `filesystem.write(path: Path, contents: Bytes)` | `Result[Unit,HostError]` |
| `filesystem.metadata(path: Path)` | `Result[FileMetadata,HostError]` |
| `filesystem.list_directory(path: Path)` | `Result[[Path],HostError]` |
| `filesystem.create_directory(path: Path)` | `Result[Unit,HostError]` |
| `filesystem.remove_file(path: Path)` | `Result[Unit,HostError]` |
| `filesystem.remove_directory(path: Path)` | `Result[Unit,HostError]` |
| `filesystem.temp_file(parent: Path)` | `Result[Path,HostError]` |
| `filesystem.temp_directory(parent: Path)` | `Result[Path,HostError]` |
| `host.decode_utf8(contents: Bytes)` | `Result[Str,HostError]` |

Reads and writes accept regular files, follow symlinks, and preserve bytes.
Reads fail without partial data when their explicit byte limit is exceeded.
Writes create or truncate a file and may leave partial contents on failure;
they promise neither atomic replacement nor durable storage. New ordinary file
and directory permissions are 0666 and 0777 filtered by the caller's umask.
Synchronous file data and read limits are capped at 32 MiB. Special files are rejected after
opening with nonblocking flags; these calls are not a general stream API.

`FileMetadata { kind: FileKind, size: Nat }` describes the final directory entry
without following a final symlink. `FileKind` is `RegularFile`, `Directory`,
`SymbolicLink`, or `OtherFile`. Size is the host's nonnegative byte-size field,
not a recursive directory total. As with POSIX `lstat`, trailing separators and
intermediate symlinks follow host path-resolution rules.

Enumeration returns immediate relative child names, excluding `.` and `..`, in
unsigned native-byte lexicographic order. Join a returned name with its parent
using `path.join`. Names are never decoded implicitly; arbitrary native bytes
round-trip where the host filesystem permits them. Enumeration is capped at
65,536 entries and 16 MiB of names and fails without a partial list. Concurrent
filesystem changes can affect results; enumeration is not a snapshot.

Directory creation creates one level and fails if the entry exists. File removal
unlinks a file or symlink; directory removal requires an empty directory. Neither
recursively deletes entries. Temporary creation atomically chooses a unique name
under the explicit parent, with permissions 0600 for files and 0700 for
directories, subject to umask. The caller owns the returned path and must remove
it explicitly. There is no automatic cleanup or lifetime tracking.

Ordinary filesystem paths do not establish containment. Relative paths use the
invocation's working directory; `..`, symlink traversal, and races remain POSIX
host behavior. The package-loading helper `package_filesystem.read_beneath` opens the
root directory and resolves the requested regular file relative to that held
descriptor. Relative symlinks are followed only when their target stays under
the root. Absolute symlinks and attempts to traverse above the root return
`invalid_argument`; loops return `symlink_loop`. The relative path is limited to
4096 bytes, a 256-level directory walk counting the opened root, and 40 symlink
expansions. Each read is limited to 16 MiB and returns the physical root's
`(device, inode)` identity with its bytes, allowing a loader to reject a root that
changes between reads. Each operation confines one individual read; it does not
snapshot files that may be edited concurrently. Recursive writes and general
descriptor-relative operations remain separate follow-ups.

### Asynchronous confined filesystem reads

`stdlib/async_filesystem` is an isolated native asynchronous API. It re-exports
`FilesystemRootIdentity`, `FilesystemRootRead` and `HostError` with the same
identities as the existing synchronous modules:

```panack
await async_filesystem.root_identity(root)
await async_filesystem.read_beneath(root, relative, limit)
```

`root_identity(Path)` returns `Result[FilesystemRootIdentity,HostError]`, validating
an existing directory without requiring an index or any other file. A caller can
compare that identity with each successful confined read and reject a replaced
root. It does not keep a descriptor open between calls.

`read_beneath(Path, Path, Nat)` returns `Result[FilesystemRootRead,HostError]` and
uses the same confined resolver and symlink policy as `package_filesystem`.
Both supplied paths are limited to 4096 bytes; each read is limited to 16 MiB.
The resolver opens only regular files for data reads. Root identity and read
failures identify `fs_root_identity` and `fs_read_beneath` respectively in
`HostError.operation`. Unsupported hosts return `unsupported_host`.

Native operations run on independently owned workers. At most 32 operations can
be retained process-wide, including completed results awaiting consumption and
abandoned operations whose host reads have not returned. Exhaustion or inability
to create a worker returns `resource_limit`. Arguments are copied before dispatch;
workers retain no VM, execution, program or shared reference-counted values.
The owning thread alone receives completion and resumes bytecode.

Cancellation discards delivery immediately without joining a potentially blocked
filesystem syscall. The worker releases its files and buffers when that syscall
returns, and retains its admission slot until cleanup is finished. Thus a stuck
filesystem can exhaust the bounded worker capacity but cannot hold a cancelled
VM alive or create unlimited background operations. Process exit releases any
remaining host resources. This is logical cancellation, not a promise that the
operating system interrupts an in-progress disk read.

Embedded executions must explicitly enable filesystem service independently of
network or listener permission. Native CLI execution permits these operations;
server children inherit the parent's filesystem grant, but never its listener
grant. Existing synchronous filesystem calls remain unavailable in async bodies.

### Process execution

```panack
process.run(
  executable: Path, arguments: [Str], stdin: Bytes, working_directory: Path,
  environment: [Str], timeout: Duration, output_limit: Nat
): Result[ProcessOutput,HostError]
```

`ProcessOutput { exit_code: Nat, signal: Nat, stdout: Bytes, stderr: Bytes }`
represents completed execution. A normal exit has `signal == 0`, including a
nonzero exit code. Signal termination has a nonzero signal number and
`exit_code == 0`. Failure before execution returns `launch_failed`, with errno;
timeout and output exhaustion return their own error categories, without partial
output. There is no implicit UTF-8 decoding: use `host.decode_utf8` explicitly.

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

`host.sleep(duration: Duration): Result[Unit,HostError]` waits for a nonnegative
duration, retries interrupted sleeps, and accepts zero. Sleep and process timeout
accept at most 31,536,000 seconds (365 days), expressed as exact nanoseconds;
negative or larger values return `negative_duration` or `out_of_range` before any
sleep or process launch. Duration arithmetic itself remains arbitrary precision.
System-suspension accounting follows the host clock and sleep implementations;
no portable across-suspension deadline promise is made.

## Bounded async/await interface

An `async` declaration marks a function that may suspend. `await helper(args)`
activates that helper in the current task, preserving locals and call frames
until it returns. Arguments are evaluated once, left to right. There is no dormant
operation value. Bare async calls, awaiting ordinary calls, awaiting a non-call,
and await outside an async function are errors. `pure async` is not a declaration.

Async bodies may call pure functions, await async functions, and directly call
checked wrappers around local HTTP resource operations. These local operations
change an owned server or writer without performing host I/O. The compiler
recognises the closed native operation set and propagates this permission through
direct helpers containing only pure calls and those operations. It does not grant
permission by a method's spelling, and it does not propagate through indirect
calls. Pure functions still cannot perform these mutations.

Other ordinary impure functions (including print, files, sleep, process and clock
services) are rejected, both directly and through callable values or helper
bodies. Adding blocking I/O to a local resource helper removes its permission.
Bytecode verification independently derives the same direct-call permission;
there is no producer-supplied trusted flag or new bytecode version. Named async references
have `AsyncFn[A,...,R]` type and are invoked with `await callback.call(args)`.
There is no conversion between AsyncFn and Fn/PureFn. Async functions must return
a value; use Unit for no data. Async main takes no parameters and returns Unit.
A directly discarded non-Unit await expression must be bound, matched or returned.
This is a limited result-use check, not proof that every computed value is used.

Import `stdlib/testing_async` and use `await testing_async.read(fail)`.
It is an experimental fixed service, available only via await. The CLI suspends
then completes it with `Ok("fake bytes".encode_utf8())` for false or
`Error("fake I/O error")` for true. No file or socket is opened. Hosts may deliver
other bytes/errors within this exact schema. Wrong completion tags, variant names
or payload arities trap before source execution resumes. Runtime task cancellation
is terminal, not a source Result error; late completions cannot restart the task.

The source slice includes no spawn, scopes, task/resource values, general timer
service or stable embedding ABI. Existing ordinary programs preserve
their behavior when recompiled to v9. See the [executable contract](../../tests/functional/cases/async_await/main.panack).

### Native TCP exchange (development toolchain)

`tcp.exchange(address: Str, port: Nat, request: Bytes, response_limit: Nat,
timeout_ms: Nat): Result[Bytes,Str]` is an async service in `stdlib/tcp`. Use `await` inside an
async function. It opens one native IPv4 TCP connection, sends the request,
shuts down its write side after all request bytes are sent, reads until peer EOF,
and closes the connection. Reads may progress while the request is being sent.
The successful result is the entire response; partial data is discarded on error.
This is EOF-delimited request/response, not a general stream or HTTP client.

The address must be a numeric dotted IPv4 address without a NUL; DNS names and
IPv6 are unsupported. Port is 1–65535. Request and response limit are each at
most 1 MiB (1,048,576 bytes); zero-length requests and response limits are valid.
Timeout is 1–60,000 milliseconds for the entire operation, measured with the
host monotonic clock. A deadline observed together with readiness wins. Peers
that keep the response stream open reach the timeout even if they have sent data.
Host suspension follows the platform clock; no hard scheduling deadline is promised.

Errors are explicit `Error(Str)` values: `invalid TCP arguments`,
`invalid IPv4 address`, `TCP clock failed`, `TCP socket failed`,
`TCP connect failed`, `TCP poll failed`, `TCP write failed`,
`TCP shutdown failed`, `TCP read failed`, `TCP timed out`,
`TCP response limit exceeded`, or `TCP peer closed before request completed`.
Malformed dynamic argument kinds also return `invalid TCP arguments` before I/O.
Allocation failure retains the VM's existing trap behavior. Native error numbers
are deliberately not part of this initial API.

Linux and macOS CLI execution support the operation. WASI/browser execution
returns `Error("TCP is unavailable on this host")` for otherwise valid arguments;
it never opens a socket. Existing embedded executions deny TCP by default.
Native hosts can explicitly enable it and interleave zero-wait polls with other
executions. Destroying the execution closes its socket and releases buffers;
there are no native callbacks or producer threads left to quiesce.

This exchange operation includes no source socket handles, connection reuse,
DNS, TLS or HTTP framing. The separate server operation below supports listening. Source `await` activates work in the current execution; it does
not create parallel tasks. This operation is included in the alpha.11 native release and absent
from alpha.10. The bytecode encoding remains v9; older runtimes reject the new
reserved intrinsic `$tcp_exchange` as unknown rather than reinterpret old
bytecode. Previously saved user functions named `tcp_exchange` remain callable.
The development compiler reserves the new source builtin name.


### Native TCP server (development toolchain)

Import `stdlib/tcp` for the ordinary `TcpServerLimits` record, then call:

```panack
await tcp.serve(address, port, handler, limits)
```

The async builtin takes `Str`, `Nat`, `AsyncFn[Bytes,Result[Bytes,Str]]` and
`TcpServerLimits`, and returns `Result[[Result[Unit,Str]],Str]`. It binds a numeric
IPv4 address on port 1–65535 on Linux/macOS. The development compiler reserves
`tcp_serve`; saved v9 user functions with that name retain their behavior.
This operation is included in the alpha.11 native release and absent from
alpha.10 downloads. The pinned browser playground cannot open raw TCP sockets.

Record constructor fields, in order:

| Field | Allowed values |
| --- | --- |
| `clients` | 1–256 total admissions |
| `concurrency` | 1–32, no greater than `clients` |
| `request_limit` | 0–1,048,576 bytes per request |
| `response_limit` | 0–1,048,576 bytes per response |
| `client_timeout_ms` | 1–60,000 ms from acceptance through response completion |
| `admission_timeout_ms` | 1–60,000 ms from successful listen |
| `drain_ms` | 0–60,000 ms after admission stops |

Each connection sends one request terminated by a write-half close. The owner
reads until EOF, runs the named handler once, writes its returned bytes completely,
then closes the socket. Binary and empty data are valid; an extra request byte
fails before handler activation. An oversized response fails before sending.
There are no added framing bytes, connection handles, HTTP, DNS, TLS or IPv6.
`tcp_exchange` is a compatible client. A client that withholds EOF reaches its
deadline. The [finite echo example](../../examples/network/tcp_serve.panack) pairs with
[the client example](../../examples/network/tcp_exchange.panack).

Handlers are independent resumable executions scheduled round robin. They can
call pure functions and await cooperative functions, including outbound TCP.
They cannot invoke ordinary blocking effects or recursively start another server,
including through an indirect call. Runtime ownership needs no source `spawn`
syntax. Instruction budgets are not wall-clock preemption or a total memory cap.
I/O payload capacity is bounded by concurrent request/response limits; VM values,
transient copies and allocations performed by handlers are additional memory.

### Native TCP streaming server prerequisite

`tcp_serve` keeps its EOF-delimited contract above. `tcp_serve_stream` is an
additive development-toolchain service for protocols whose peer waits for a
response before closing its write side. It takes the same address, port and
limits, plus `AsyncFn[Context,Result[Bytes,Str]]` and a value of that same
`Context` type. The owner starts the named worker as soon as a client is accepted
and passes that value to it. The operation returns the same ordered finite-server
report as `tcp_serve`.

Only an active worker started by `tcp_serve_stream` may await
`tcp.read_chunk(max_bytes)`. The positive `Nat` limit is at most 16,384
bytes. A completion contains up to that many currently available bytes; it may
contain fewer. `Ok(empty_bytes)` means peer EOF, including on later reads after
EOF. A socket failure or cumulative per-client `request_limit` breach returns
`Error(Str)`. No bytes are automatically buffered for the worker before it asks
to read. A call outside that active child traps; the API does not expose a socket
or forgeable connection record. Existing per-client deadlines, admission,
concurrency, cancellation and bounded response writes remain owned by the server.

This primitive does not add HTTP framing. The Panackelty HTTP package owns
incremental parsing and response serialization; P6 is the selected consumer.
Its server accepts GET, HEAD and POST. HEAD preserves the corresponding
representation headers, including Content-Length, and sends no body. Handlers
supply the representation body as usual, subject to existing response limits.
Parser failures with a recognised HEAD method and handler failures also omit
response bodies. See `docs/design/MODULE_PACKAGE_DESIGN.md` for the package contract.
Native Linux/macOS support follows the server capability gate; WASI remains
unavailable. The source-level limit is checked again at runtime for saved or
forged bytecode.

Admission stops on the total-client limit, admission deadline or a host stop
request. The listener closes immediately. Existing clients drain until the
sooner of their own deadline and the drain deadline; zero grace cancels them
immediately, including the last admitted client if the admission limit triggered
stop. The operation returns only after children and sockets are released.
Execution destruction cancels immediately without user finalizers or graceful
waiting. A client deadline also cancels its nested outbound operation. Deadlines
observed before completion delivery win; cancellation cannot undo sent bytes.
The CLI's finite limits provide shutdown; no signal-driven shutdown API is added.

Outer `Ok` contains one report per accepted client, in acceptance order. A report
is `Ok(())` after a complete response write, or `Error(Str)` for a client failure.
Handler `Error` text is preserved exactly. Other client messages are `TCP client
socket failed`, `TCP request limit exceeded`, `TCP client read failed`, `TCP
response limit exceeded`, `TCP client write failed`, `TCP client timed out`, and
`TCP server drain cancelled client`. Expected client failures do not cancel others.

Outer `Error` reports invalid bounds/address or listener/clock/poll failures after
closing all clients: `invalid TCP server arguments`, `invalid IPv4 address`,
`TCP server socket failed`, `TCP server bind failed`, `TCP server listen failed`,
`TCP server accept failed`, `TCP server listener failed`, `TCP server clock failed`,
or `TCP server poll failed`. Partial reports are discarded on this path; earlier
network effects are not rolled back. Allocation failure, malformed dynamic
argument/record/callable/result shapes, and handler traps fail the execution and
cancel children. Forged bytecode is not trusted merely because a name has a source
type. Record field names/order and every natural-number conversion are checked.

WASI/browser execution returns `Error("TCP server is unavailable on this host")`
for otherwise valid arguments. Embedded listening is denied by default and needs
its own native opt-in, separate from outbound TCP. Handler children inherit the
outbound permission, never listening permission. These internal host APIs are
experimental, not a stable embedding ABI.

### Continuous native services (development toolchain)

`stdlib/tcp_service` adds a continuous mode to the existing transport. Its generic
`serve[T]` accepts an IPv4 address, port, concrete named async worker, typed context,
`TcpServiceLimits` and a named async readiness callback. Limits contain
`concurrency`, `request_limit`, `response_limit`, `client_timeout_ms` and `drain_ms`.
Concurrency is 1–32, request/response limits are 0–1,048,576 bytes, client timeout
is 1–60,000 ms, drain is 0–60,000 ms, and port is 1–65,535. Finite server APIs
retain their existing client-count and admission-deadline contracts.

The listener binds before calling readiness with `TcpServiceReady(address, port)`.
Admission starts only when readiness returns `Ok(())`; `Error(Str)` closes the
listener and fails the service. Readiness is cancellable. There is no admission
deadline or fixed total client limit. Normal stop returns
`TcpServiceSummary(accepted, succeeded, failed)`, counting completed response writes
as successes and client errors/cancellations as failures. Per-client reports are
not retained. Counters are bounded by the host `size_t`; the service fails before
its acceptance sequence can wrap. Listener failures discard the summary.

While the native CLI owns an active continuous service, SIGINT and SIGTERM request
stop: close admission, allow existing clients to finish within `drain_ms`, then
cancel remaining clients. Previous signal dispositions are restored on service
completion or execution destruction. Only one CLI service owns these handlers at
a time. Embedded services use explicit host stop and do not install signal handlers.
Drain is enforced by the existing polling/instruction-budget scheduler, not a hard
wall-clock guarantee around individual instructions or destructors.

The alpha.14 routed HTTP framework is exported by `http/server`; it replaces the
previous HTTP service/finite-session facades while leaving low-level TCP APIs
intact. Constructors return `Server` or `ContextualServer[T]`. The bound forms
accept numeric IPv4; convenience forms bind all interfaces. Registration mutates
opaque server state through `.get`, `.post`, `.put`, `.delete` and `.fallback`.
Handlers are named async references, with a typed context parameter for contextual
servers, and return `Result[Unit,Str]`. Invalid setup is sticky and reported by
`listen` before binding. Registration after startup is a request/runtime fault.

`listen` resolves after bind. The runtime retains active listeners after main
returns. `close` stops admission and drains for at most one second; `destroy`
cancels immediately. Shutdown is idempotent; a closed server cannot listen again.
CLI signals and explicit exit perform bounded cleanup. Request children cannot
forge host handles or resume cancelled host completions.

Requests expose raw path/query, ordered headers and buffered bytes. The four
supported methods use exact-path routing; unsupported methods never reach a
handler. A known path with an unsupported or unregistered method returns 405 with
its registered methods in Allow; an unknown path with an unsupported method lists
all four. A supported unknown path reaches fallback or receives 404.

Response writers own status, ordered headers and the first committed response.
Later sends/mutations fault without replacing it. Missing sends, handler Error
and recoverable request faults produce generic 500 before commitment. The limits
are sixteen accepted connections, four admitted requests, 16 KiB request/response
header blocks, 1 MiB request/response bodies and ten seconds from accept to
completion. Fixed-length and chunked bodies and bounded 100-continue negotiation
are supported. There is one final response per connection, no keep-alive or TLS.
See the [HTTP package guide](../../packages/http/README.md#server-alpha14-development-branch)
and [agreed contract](../design/routed-http-server/contract-snapshot.md) for the
complete public surface and protocol rules.

### Static directory service (development toolchain)

`packages/static_http` exports `service.Config(root, address, port, request_body_limit, response_limit, headers)`
and asynchronous `serve(config, ready)`. It composes existing HTTP and confined
filesystem operations without extending the standard library. It validates the
root and extra headers before binding, pins the root device/inode and rejects
contents returned from a replacement root. The executable under `tools/http-server`
is installed as `panack-http-server`; installation compiles its bytecode once.

GET serves regular files and `index.html`. A directory with an index and
no trailing slash redirects with 308, preserving the query. Missing indexes and
inaccessible paths return 404; no directory listing or SPA fallback is supplied.
POST, PUT and DELETE return 405 with Allow: GET. The framework rejects other
methods before the static handler; HEAD error responses omit body bytes.
Percent decoding occurs once;
paths must be UTF-8, with no traversal, encoded separators, repeated separators,
NUL or control bytes. Query strings do not become filenames. Relative symlinks
are allowed only under the existing confined resolver policy.

Extra headers cannot override framing, Content-Type, Location or Allow, and their
serialized form must leave 512 bytes of response budget. They apply to application errors and
redirects as well as files, but not framework-generated protocol errors. The full serialized GET representation must fit the
response limit, including headers; oversize files return 503 with a diagnostic.
Unexpected read failures return 500; resource exhaustion/root replacement returns
503. Per-request reads expose edits without a snapshot guarantee. Ranges, TLS,
compression, watch/rebuild and automatic reload are outside this contract.

Tool application limits are 16 KiB request bodies and 1 MiB serialized responses.
The framework permits sixteen connections and four admitted requests, with ten
seconds per connection and one second of signal-driven drain. The [tool guide](../../tools/http-server/README.md)
defines arguments, readiness and exit codes; the [package guide](../../packages/static_http/README.md)
defines library configuration and cancellation limits.

### Asynchronous command-line host operations

`stdlib/async_host` provides `await arguments(): [Str]`, `await stdout(Str)` and
`await stderr(Str)` returning `Result[Unit,HostError]`, and `await exit(Nat): Unit`.
Arguments use the invocation snapshot. Output appends one newline to the supplied
bytes, including embedded NUL bytes. Each message is limited to 16,384 bytes;
there are at most four retained output operations process-wide. Oversize output
returns `out_of_range`, exhausted capacity returns `resource_limit`, and native
write/descriptor failures return `io_error` with a native code. Closed pipes return
an error without terminating the process with SIGPIPE. Ordering across concurrent
writes and atomic whole-line output are not guaranteed.

Output workers own copied bytes and duplicate descriptors. Cancellation suppresses
result delivery immediately but cannot retract output; a blocked worker retains
its slot and descriptor until the host write finishes. No VM or program state is
borrowed by the worker. Output is unavailable on unsupported hosts.

Exit requires a status in 0–255 and does not return normally: the CLI exits, while
an embedded execution reports `VM_EXITED`. Embedded arguments/output require an
explicit console grant, independent of filesystem and network permissions. Server
request children inherit the parent's console grant; the readiness callback receives
only its explicit console grant. These host grants remain experimental embedding APIs.

### Native HTTP client (P5 proof of concept)

Import the package namespace `http/client` and call:

```panack
import http/client as http

async fetch(): Result[http.Response,http.HttpError] {
  await http.get("https://example.test/status")
}
```

`get(url)` performs one HTTP/1.1 GET and returns a typed `Response` containing
the status, ordered headers (including repeated fields) and raw `Bytes` body.
Non-2xx status codes are returned normally. `get_with_limits(url, limits)` uses
the same operation with a caller-selected timeout and response bounds. Defaults
are 10 seconds, 64 KiB of response headers and 1 MiB of body. Hard limits are
60 seconds, 64 KiB and 1 MiB respectively. The transport also caps total raw
response bytes at 16 MiB.

The package accepts HTTP and HTTPS URLs with ASCII DNS names or dotted IPv4
addresses. It rejects IPv6, user information, fragments, non-ASCII hosts,
control characters and unsupported schemes. Requests use origin-form paths and
close the connection after one response. Fixed-length, chunked and close-delimited
responses are supported. Invalid or ambiguous framing is a typed protocol error.
Redirects, request bodies, cookies, connection reuse and proxy use are not
supported.

The native runtime uses libcurl multi connect-only mode for asynchronous DNS,
connection establishment and TLS; the Panackelty package constructs and parses
HTTP bytes. Certificate-chain and hostname verification stay enabled and use
the platform trust store. `CURL_CA_BUNDLE` may select a PEM trust bundle.
Native builds need libcurl development files and `curl-config`; linked libcurl
must include asynchronous DNS and thread-safe features. WASI/browser execution
is unavailable.
This is the replaceable libcurl backend selected for the alpha.13 proof of
concept. See the [P5 design](../design/NATIVE_HTTP_CLIENT_DESIGN.md) for the package
boundary, exact limits, error categories and fixture coverage.

## Optional instruction source lookup

`panack compile SOURCE.panack -o ARTIFACT.bc --source-map MAP` emits an optional
sidecar without changing language semantics or executable bytecode v9.
`panack locate ARTIFACT.bc --source SOURCE.panack --source-map MAP --function NAME
--instruction PC` validates the pair by local recompilation and returns a direct
expression, enclosing lowered expression, or explicit unavailable result. Ranges
are half-open Unicode code-point offsets with one-based line/column coordinates.
Lookup does not execute source, and ordinary runtime diagnostics do not consult
maps. The [source-map contract](../guides/SOURCE_MAPS.md) defines identity, source
closure, compatibility, I/O bounds, output safety and source disclosure.

## Compiler explanations

`panack explain SOURCE.panack --function NAME` performs the ordinary frontend
checks with opt-in evidence retention and reports proof-bearing conversions
into guarded types, binary `Nat` subtraction obligations, and local call/await
effect boundaries from the named function. It also reports inferred local types
with their checked initializer type and source expression; annotated locals are
omitted, and incomplete types are explicitly unavailable. This query does not
change inference rules or claim runtime values or call-site specialisations.
Effect output retains existing pure/ordinary/async decisions and callable-type
classification without inferring transitive or runtime effects. Effect evidence is
unavailable for type-invalid functions. The query can retain boundaries for
separately type-valid functions after a body error; global loading, resolution,
signature or declaration type errors suppress all recovery. Ordinary compilation and
original diagnostics remain unchanged; recovered effect violations appear only in
the local explanation. An allowed boundary does not imply that a callee body or
the whole program is valid.
The command exposes constant/lower-bound proof decisions and proof-bearing
conversions into guarded targets, including source facts and unresolved
results. The checker accepts conversion from a compatible guarded source when
its invariant proves the target guard; stricter guards still require sufficient
facts. Other unsupported proof rules remain unchanged. A local
proved obligation does not imply whole-program validity; unproved does not imply
a runtime counterexample. Unsupported/invalid operands and unavailable evidence
are explicit. Generic results describe definitions, not specialisations. No user
code executes and no bytecode or files are emitted. See the
[command contract](../guides/COMPILER_EXPLANATIONS.md) for exit status, source bounds,
positions and deliberate limits. Bytecode v9 and numeric semantics are unchanged.

### Standard-library owned caller migration (next)

The standard library and its conformance programs use owned dot calls after
GI#525 migration. `TestWorkspace` owns the effectful `remove_empty()` method,
returning `Result[Unit,HostError]` and refusing nonempty directories as before.
Constructors and host services remain explicit module functions. Generic owner
arguments belong to a typed receiver binding, not to `value_or` or `sort_by`.
P12 removes the 32 receiver-function compatibility exports listed in the
[P10 register](../design/type-owned-dispatch/p10-stdlib-callers.md), along with
receiver-to-global lowering and special get/put dispatch. Pure
`Bytes.decode_utf8` still traps on invalid UTF-8; `host.decode_utf8` returns a
checked result. P11 migrated maintained callers; P12 replaces the retained
legacy acceptance tests with owned-dispatch and old-syntax rejection coverage.
Backend fixtures name internal operations explicitly and do not define the
public language. The [P12 report](../design/type-owned-dispatch/p12-legacy-removal.md)
records the boundary and validation.

## Local dependency locks on next

The separate Panack package tool implements the local-source portion of the
[package contract](../design/PACKAGE_MANAGEMENT_CONTRACT.md): explicit lock,
update (optionally restricted to root aliases), and offline restore. Project
check/compile/run require a current lock for dependencies and validate existing
archive materialization without acquiring sources. Local directory source edits
remain live; manifest and local archive changes require explicit update.
Archive and lock policy do not execute inside the compiler.

The tool uses generic native modules `stdlib/filesystem_stage`,
`stdlib/bounded_filesystem`, `stdlib/gzip` and `stdlib/digest`. Their bounded
operations return `Result[..., HostError]`; handles belong to one VM invocation,
are never reused during it, and reject use after close. SHA-256 uses OpenSSL EVP,
gzip uses zlib, and both operate on chunks of at most 64 KiB. Native filesystem
staging publishes one complete directory atomically. See the
[adapter contracts](../../src/stdlib/README.md#native-streaming-and-confined-filesystem-adapters-on-next)
for limits, cleanup and platform restrictions. These modules introduce no
arbitrary foreign-function interface or new bytecode version.
