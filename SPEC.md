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
| `Str` | Unicode text |
| `Bool` | `true` or `false` |
| `Void` | A function return marker indicating that no value is returned |

Integer literals are inferred as `Nat` when non-negative and `Int` when
negative. A literal containing a decimal point is `Dec`. `Dec` stores an
arbitrarily large base-10 coefficient and scale. Addition, subtraction,
multiplication, and remainder are exact. Division is exact when it has a finite
decimal expansion and otherwise reports that an explicit rounding operation is
needed; rounding modes are not yet part of the language.

`Nat` subtraction is accepted only when the checker can prove that the result
is non-negative. This first implementation recognizes constants and simple
guard facts. Use `Int` when subtraction may legitimately cross zero.

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

An immutable local declaration has the form `name: Type = value`. A mutable
local adds the `mut` modifier, as in `mut name: Type = value`. Subsequent
assignment uses `name = value`. There is no `let` keyword.

`Void` is valid only as a function return type and is not a first-class source
value: it cannot be used for parameters, bindings, collection elements, or
arguments. A `Void` function may fall through its closing brace without a final
expression. Every non-`Void` function must end with a value compatible with its
declared return type. Empty parentheses are not a value.

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
only pure functions and cannot invoke terminal or file I/O built-ins.
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
library surface but execute as VM primitives because generic functions cannot
yet express those APIs in source. Host access remains limited to the ABI calls
identified above.

`len` also accepts arrays and byte buffers.

## Strings

`Str` values are Unicode text. They concatenate with `+`, and interpolation uses
`${name}` for a local scalar variable:

```panackelty
pure greeting(name: Str, attempts: Nat): Str {
  "Hello, ${name}; attempt ${attempts}"
}
```

Interpolation accepts `Nat`, `Int`, `Dec`, `Str`, `Bool`, and guarded scalar
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
elements are currently restricted to scalar types. `get` traps when a key is
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
version. Version 7 uses the compact typed binary payload introduced by version
5, containing function
signatures, purity flags, and instruction streams; `main` is the implicit entry
point. The VM uses
an internal `Void` sentinel to keep call and return mechanics uniform, but it is
not exposed as a source value. Before execution the bytecode verifier rejects
unknown opcodes, malformed constants and operands, invalid control-flow targets,
calls with incorrect arity, missing functions, and pure functions that call
impure functions.

Version 7 retains the instruction, tagged-value, isolated-call-frame,
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

The developer-preview diagnostic contract does not yet include source excerpts,
secondary labels, stable error codes, or automated fixes. Loader, entry-point,
and declaration-wide failures may remain message-only when there is no single
source expression to identify.

## Deliberately postponed

- Mutable array elements and growable collections
- Generic functions and explicit type arguments
- Module visibility, selective imports, and package management
- Explicit checked construction from untrusted data
- Parametric polymorphism and traits
- A backwards-compatibility guarantee for bytecode versions
- Concurrency

The immediate proving ground is a sequence of Project Euler solutions. Features
should be added when those programs demonstrate a concrete need.
