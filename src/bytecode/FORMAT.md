# Panackelty bytecode contract

This document freezes the executable semantics shared by Panackelty compilers,
verifiers, and VMs. Version 4 introduced the semantics below with a transitional
JSON payload. Version 5 retained those semantics and replaced only the artifact
encoding with the compact binary layout specified here. Version 6 retained that
layout and added internal collection-method call targets plus Unicode
code-point string reversal. Version 7 adds indirect callable invocation.
Changing executable behavior or encoding requires another bytecode version
increment.

## Version 7 binary layout

Every artifact starts with the nine bytes `PANACKBC\0`, followed by an unsigned
16-bit big-endian version. Version 7 then contains one payload with no padding
or trailing bytes:

```text
u16 version = 7
u16 function_count
function[function_count]

function = name, u8 flags, u8 parameter_count,
           name[parameter_count], u32 instruction_count,
           instruction[instruction_count]
```

Functions are sorted by ascending Unicode function name. Bit zero of `flags`
is the purity flag; every other bit must be zero. `main` is the implicit entry
point and is not repeated in the payload.

The primitive encodings are:

| Form | Encoding |
| --- | --- |
| `u8`, `u16`, `u32` | Unsigned big-endian integer of the stated width. |
| `i16` | Two's-complement signed big-endian integer. |
| `name` | `u16` UTF-8 byte length, then exactly those bytes. |
| `text` | `u32` UTF-8 byte length, then exactly those bytes. |
| natural magnitude | `u16` byte length followed by a minimal unsigned big-endian magnitude; zero has length zero and nonzero values cannot start with `00`. |
| signed integer | Sign `u8` (`0` positive, `1` negative), then a natural magnitude; negative zero is invalid. |
| decimal | Sign `u8`, exponent `i16`, digit count `u16`, then packed binary-coded decimal digits, high nibble first. Digit count is nonzero; an odd final low nibble is `f`; every other nibble is `0` through `9`. |

Decimal fields encode the sign, coefficient digits, and base-10 exponent from
the abstract `Dec` value directly, preserving exact scale. All integers use
their one minimal representation. These canonical rules make it impossible for
two valid byte sequences to encode the same function table.

Each instruction begins with its one-byte opcode, followed by the operand form
in this table:

| Code | Instruction | Encoded operand |
| ---: | --- | --- |
| `00` | `CONST` | Constant tag and tag-specific data described below. |
| `01` | `LOAD` | `name` |
| `02` | `STORE` | `name` |
| `03` | `POP` | none |
| `04` | `UNARY` | `u8`: `0` is `-`, `1` is `!` |
| `05` | `BINARY` | `u8`: `+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `<=`, `>`, `>=`, `&&`, `||` are numbered `0` through `12` in that order. |
| `06` | `MAKE_RANGE` | none |
| `07` | `MAKE_ARRAY` | item count `u16` |
| `08` | `INDEX_GET` | none |
| `09` | `INTERPOLATE` | part count `u16`, then that many `text` values |
| `0a` | `ITER_INIT` | iterator-local `name` |
| `0b` | `ITER_NEXT` | iterator `name`, item-local `name`, end target `u32` |
| `0c` | `MAKE_RECORD` | type `name`, field count `u16`, then field `name` values |
| `0d` | `FIELD_GET` | `name` |
| `0e` | `MAKE_VARIANT` | enum `name`, variant `name`, payload count `u16` |
| `0f` | `MATCH_VARIANT` | variant `name`, failure target `u32` |
| `10` | `MATCH_FAIL` | none |
| `11` | `CALL` | callee `name`, arity `u8` |
| `12` | `JUMP_FALSE` | target `u32` |
| `13` | `JUMP` | target `u32` |
| `14` | `RETURN` | none |
| `15` | `CALL_VALUE` | arity `u8` |

The `CONST` tags are `0` `Nat`, `1` `Int`, `2` `Dec`, `3` `Str`, `4` `Bool`,
and `5` `Void`. `Nat` uses a natural magnitude, `Int` a signed integer, `Dec`
the decimal form, `Str` a `text`, `Bool` one `u8` equal to zero or one, and
`Void` has no data.

Decoders reject truncation, invalid UTF-8, unknown tags or opcodes, reserved
flag bits, non-minimal numbers, invalid decimal nibbles, count or length limit
violations, and trailing bytes before passing the decoded table to the semantic
verifier.

## Execution model

A program is a uniquely named function table with `main` as its entry point.
`main` takes no parameters. Each function records its ordered parameter names,
purity flag, and instruction sequence. Instruction offsets are zero-based.

Every active call owns an isolated frame containing:

- its function;
- a program counter pointing to the next instruction;
- a local map initially populated by pairing parameters with arguments;
- an initially empty operand stack.

Calling a user function suspends the caller and pushes a new frame. `RETURN`
removes the callee frame and pushes its result onto the caller's operand stack.
Returning from `main` terminates execution with that value. Locals never alias
another frame's local map. Persistent collection built-ins return new values
instead of mutating inputs.

Unless stated otherwise, operands are popped last-in-first-out, instructions
advance the program counter before executing, and a produced value is pushed on
the current operand stack. Multi-value operations preserve source order: if
`a` is pushed before `b`, a two-argument operation receives `(a, b)`.

## Values

The VM uses tagged values:

| Tag | Meaning |
| --- | --- |
| `Nat` | Arbitrary-precision integer greater than or equal to zero |
| `Int` | Arbitrary-precision signed integer |
| `Dec` | Finite arbitrary-precision base-10 decimal; division must terminate exactly |
| `Str` | Unicode text indexed and measured by code point |
| `Bool` | `true` or `false` |
| `Void` | Internal no-result sentinel; not a source-storable value |
| `Range` | Half-open pair of natural bounds `[start, end)` |
| `Array[T]` | Ordered persistent sequence of tagged values |
| record name | Ordered named fields for that nominal record |
| enum name | Variant name plus ordered payload values |
| `Map[K,V]` | Persistent insertion-ordered key/value entries |
| `Set[T]` | Persistent insertion-ordered unique values |
| `Bytes` | Immutable byte sequence |
| `Iterator` | Frame-local iteration state; never serialized |

Only `Nat`, `Int`, `Dec`, `Str`, `Bool`, and `Void` occur in `CONST` records.
Composite values are constructed by instructions or built-ins.

Integer division truncates toward zero. Integer remainder follows the divisor's
sign. Decimal addition, subtraction, multiplication, and remainder are exact;
decimal division traps when its exact result has a non-terminating base-10
expansion. `Nat` arithmetic traps if it would produce a negative result.

## Instructions

Stack effects use `...` for the unchanged lower stack and put the top at the
right.

| Instruction | Operand | Stack effect and behavior |
| --- | --- | --- |
| `CONST` | `(tag, value)` | `... -> ..., value`; push a verified scalar constant. |
| `LOAD` | local name | `... -> ..., value`; read an initialized local. |
| `STORE` | local name | `..., value -> ...`; replace or create the frame local. |
| `POP` | none | `..., value -> ...`; discard one value. |
| `UNARY` | `-` or `!` | `..., value -> ..., result`; numeric negation or Boolean negation. |
| `BINARY` | binary operator | `..., left, right -> ..., result`; apply the specified arithmetic, comparison, or Boolean operation. |
| `MAKE_RANGE` | none | `..., start, end -> ..., range`; construct `[start, end)`. |
| `MAKE_ARRAY` | item count | `..., item1..itemN -> ..., array`; preserve item order. |
| `INDEX_GET` | none | `..., collection, index -> ..., item`; index `Str`, `Bytes`, or an array. |
| `INTERPOLATE` | ordered text parts | Pop `len(parts)-1` values and interleave their string forms between the parts. |
| `MAKE_RECORD` | `(type, fields)` | Pop one value per ordered field and construct a nominal record. |
| `FIELD_GET` | field name | `..., record -> ..., field-value`. |
| `MAKE_VARIANT` | `(enum, variant, count)` | Pop `count` payload values in source order and construct the enum value. |
| `MATCH_VARIANT` | `(variant, failure)` | Pop an enum. On a match, push its payload values in declaration order; otherwise jump to `failure`. |
| `MATCH_FAIL` | none | Trap because no enum arm matched. |
| `ITER_INIT` | iterator local | `..., iterable -> ...`; store an iterator for a range, array, or bytes value. |
| `ITER_NEXT` | `(iterator, local, end)` | Store the next item in `local`; if exhausted, jump to `end`. |
| `CALL` | `(callee, arity)` | Pop ordered arguments. Built-ins execute immediately; user calls push an isolated frame. Both push one tagged result, including `Void`. |
| `CALL_VALUE` | arity | `..., callable, arg1..argN -> ..., result`; dynamically resolve the callable's internal target, validate existence, arity, and the caller/callee purity edge, then invoke it. |
| `JUMP_FALSE` | target | Pop a Boolean and jump when false; otherwise continue. |
| `JUMP` | target | Continue at the target offset. |
| `RETURN` | none | Pop the frame result, remove the frame, and deliver the result to its caller. |

All jump targets refer to instructions in the same function. Short-circuit
Boolean behavior is expressed with jumps rather than eager `BINARY` operations.

Version 6 added four pure internal collection call targets emitted only from
typed method syntax: `$method_put` and `$method_get` require Map receivers,
`$method_add` requires a Set receiver, and `$method_has` dispatches between Map
and Set using the runtime value tag. It also adds the pure `reverse(Str) -> Str`
built-in, which reverses Unicode code points while preserving each code point's
UTF-8 byte sequence. Source checking establishes the collection families and
argument types; the VM retains dynamic checks for forged artifacts.

Version 7 adds `CALL_VALUE`. Source-level `@name` references are represented as
opaque callable values by the language even though the current bytecode lowering
uses a private string target. Source checking supplies the `Fn[...]` or
`PureFn[...]` signature. Because the verifier does not perform stack-shape type
analysis, the VM revalidates the indirect target, arity, and purity at execution
time. Array `map` and `reduce` are compiler lowerings to `ITER_*`, persistent
array construction, and `CALL_VALUE`; they add no privileged runtime primitive.

## Verification and traps

Artifacts are untrusted. Before execution, the verifier checks the entry point,
unique function identities, signatures, instruction and operand shapes, scalar
constant tags, in-function control-flow targets, call existence and arity, and
purity edges. Verification is necessary but does not assume source-level typing.

Execution traps instead of exposing host-language failures for invalid dynamic
state, including operand-stack underflow, uninitialized locals, incompatible
runtime operands, invalid field or index access, invalid matches, missing
returns, `Nat` underflow, division failure, invalid UTF-8, invalid byte values,
and missing map keys. I/O failures are reported as Panackelty errors. Resource
exhaustion and process termination are outside the language-level trap model.

Trap wording may gain context without a version change, but the condition that
causes a trap is part of the version-7 contract.

## Resource limits

Version-7 loaders and verifiers enforce these limits before execution. They
apply equally to serialized artifacts and in-memory function tables produced by
a compiler.

| Resource | Maximum |
| --- | ---: |
| Complete artifact | 16 MiB |
| Functions | 4,096 |
| Parameters per function | 255 |
| Instructions per function | 1,000,000 |
| Instructions per program | 4,000,000 |
| UTF-8 bytes in a name | 1,024 |
| UTF-8 bytes in one string constant or interpolation part | 1 MiB |
| Items in one encoded operand collection | 65,535 |
| Digits in an integer or decimal coefficient | 4,096 |
| Absolute decimal exponent | 4,096 |

The artifact-size check occurs before payload decoding. Length and count fields
are checked before allocating or iterating their contents. These are validation
limits, not source-language fixed-width numeric semantics: arithmetic remains
arbitrary precision within the resources admitted by an artifact.

Tightening a limit requires a bytecode version change. Implementations may
reject execution earlier for unavailable process memory, which remains outside
the language-level trap model.
