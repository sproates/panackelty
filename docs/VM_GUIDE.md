# Following a program through the Panackelty VM

This guide shows what the compiler emits and how the VM executes it. Start with
an arithmetic expression, then follow a function call and a loop. Each example
is a runnable program in `examples/`, with expected output checked by the
functional suite from both source and compiled bytecode.

The guide describes bytecode version 8. [FORMAT.md](../src/bytecode/FORMAT.md)
is the authoritative instruction and binary-format contract;
[VALUE_MODEL.md](../src/vm/VALUE_MODEL.md) describes native memory ownership.

## From source to instructions

The compiler parses and checks a source program, then emits instructions for
each function. The VM verifies the bytecode before executing `main`, which takes
no parameters. Source execution takes the same compilation route as a saved
`.bc` file; there is no separate source interpreter.

```mermaid
flowchart LR
    S[Panackelty source] --> C[Compiler and type checker]
    C --> B[Version 8 bytecode]
    B --> V[Verifier]
    V --> M[VM begins at main]
    M --> H[Runtime and host services]
```

From the repository root:

```sh
./panack disasm examples/vm_arithmetic.panack
./panack compile examples/vm_arithmetic.panack -o /tmp/vm_arithmetic.bc
./panack disasm /tmp/vm_arithmetic.bc
./panack run /tmp/vm_arithmetic.bc
```

The two disassembly commands show the same instructions. `disasm` is an
inspection command, not an assembler: the text it prints isn't source input to
`panack run`. In a release archive, use `./bin/panack` instead of `./panack`.

## Reading the disassembly

The public CLI prints pipe-separated lines:

```text
FUNCTION|main|impure|
0|CONST|Nat:2
1|CONST|Nat:3
2|BINARY|+
```

A function header gives its name, purity, and parameter names. `main` has no
parameters, so its last field is empty. Instruction lines give a zero-based
instruction number, opcode, and any operands. `CONST|Nat:2` pushes a tagged
natural number; `BINARY|+` adds the two values at the top of the stack.

Instruction numbers restart at zero in each function. Jump operands refer to
these numbers within the same function. They aren't byte offsets: instructions
have different encoded lengths.

The binary representation starts with `PANACKBC` followed by a zero byte, the
big-endian version `00 08`, and the function table. Each instruction has a
one-byte opcode followed by its encoded operands. For example:

| Display | Instruction bytes (hex) | Meaning |
| --- | --- | --- |
| `CONST\|Nat:2` | `00 00 00 01 02` | CONST opcode, Nat tag, two-byte magnitude length 1, magnitude 2 |
| `BINARY\|+` | `05 00` | BINARY opcode, operator code 0 |
| `JUMP_FALSE\|6` | `12 00 00 00 06` | Conditional jump to instruction 6 |
| `RETURN` | `14` | Return the top value |

These are instruction fragments, not complete runnable files. Function names,
parameter names, purity flags, and instruction counts belong to the surrounding
binary structure. Only scalar constants are encoded directly; instructions and
built-ins construct collections, records, variants, and other composite values.

## The state the VM keeps

Each active function call owns a frame:

```text
Call frames (oldest first)
┌────────────────────────────────────────────┐
│ main: suspended at its next instruction     │
│ locals: its own named values                │
│ operand stack: its own intermediate values  │
└────────────────────────────────────────────┘
┌────────────────────────────────────────────┐
│ called function: currently executing        │
│ PC: next instruction number                │
│ locals: parameters plus local bindings      │
│ operand stack: bottom → top                 │
└────────────────────────────────────────────┘
```

The program counter (PC) identifies the next instruction. The VM advances it
before executing that instruction, so a call leaves the caller ready to resume
after the call. A jump replaces the next PC with its target.

The operand stack holds intermediate results. In `[Nat:2, Nat:3]`, `Nat:3` is
on top. `BINARY` pops the right operand, then the left, and pushes one result.
This matters for subtraction and division. `STORE` removes the top value and
assigns it to a named local; `LOAD` pushes the local's value onto the stack.
Locals and the operand stack are separate.

A user-function call removes its arguments from the caller's stack and creates
a frame with those arguments bound to parameters in source order. The new
frame starts at PC 0 with an empty operand stack. `RETURN` removes the callee
frame and puts its result on the caller's stack. Returning from `main` finishes
the program.

Built-ins such as `print` run immediately rather than creating a user-code
frame. Every call produces one internal result, including the `Void` sentinel
for operations with no source-level result. `Void` keeps call and return stack
behaviour uniform; source code cannot store it as an ordinary value. A `POP`
may discard it, or `RETURN` may use it to finish a `Void` function.

## Instruction families

The top of the stack is on the right. `…` means unchanged values below the
operands. The full reference includes encoded operand widths and dynamic
failure conditions.

| Instructions | Effect |
| --- | --- |
| `CONST`, `LOAD` | Push a constant or local value. |
| `STORE`, `POP` | Remove the top value; STORE retains it in a local. |
| `UNARY`, `BINARY` | Replace one or two operands with the operation's result. |
| `MAKE_RANGE` | `…, start, end → …, range`; end is exclusive. |
| `MAKE_ARRAY` | Consume the given number of values and preserve their order. |
| `INDEX_GET` | `…, collection, index → …, item`; supports strings, bytes, and arrays. |
| `INTERPOLATE` | Interleave popped values with the instruction's fixed text parts. |
| `MAKE_RECORD`, `FIELD_GET` | Construct ordered named fields, or replace a record with a field value. |
| `MAKE_VARIANT` | Construct a tagged-union value from ordered payload values. |
| `MATCH_VARIANT` | Pop an enum; push its payload on success, or jump on mismatch. |
| `MATCH_FAIL` | Trap because no variant matched. |
| `ITER_INIT` | Pop an iterable and store an internal iterator in a local. |
| `ITER_NEXT` | Assign the next item to a local, or jump when exhausted; no operand-stack change. |
| `CALL` | Consume arguments and invoke the named built-in or user function. |
| `CALL_VALUE` | Consume a callable followed by its arguments, validate the target, then call it. |
| `JUMP_FALSE` | Pop a Boolean; jump only when false. |
| `JUMP` | Set the PC to the target unconditionally. |
| `RETURN` | Pop a result and return it to the caller, or finish main. |

Source `&&` and `||` use jumps so the right operand can be skipped. They do not
use the eager Boolean `BINARY` operations. There is no dedicated print opcode:
output goes through `CALL|print|1`. Map/set methods also use internal built-in
call targets, while array `map` and `reduce` lower to iteration and indirect
calls. Source `@name` callable references currently lower to private string
targets; this representation doesn't make arbitrary source strings callable.

## Worked executions

The listings below are actual public-CLI disassembly. The tables were captured
at instruction boundaries in the transitional Python reference VM using the
same compiled files, and their output was compared with the native C VM.
They are recorded walkthroughs, not a new tracing command or a live debugger.

Each row shows the state **after** the named instruction. Stacks read from
bottom to top. The PC column identifies what executes next. Values retain their
tags so text, natural numbers, Booleans, and internal values remain distinct.
The iterator's cursor is hidden, but the item local shows each yielded value.

### Arithmetic and local variables

The compiler emits the addition before multiplication. The stack holds intermediate results until instruction 5 stores the answer; instruction 6 loads it again for printing.

Source: [`vm_arithmetic.panack`](../examples/vm_arithmetic.panack).

```panackelty
main(): Void {
  answer = (2 + 3) * 4
  print(answer)
}
```

```text
FUNCTION|main|impure|
0|CONST|Nat:2
1|CONST|Nat:3
2|BINARY|+
3|CONST|Nat:4
4|BINARY|*
5|STORE|answer
6|LOAD|answer
7|CALL|print|1
8|RETURN
```

| Step | Just executed | Next PC | Active stack | Active locals | Output so far |
| ---: | --- | --- | --- | --- | --- |
| 0 | `Start` | `main:0` | `[]` | (empty) | (empty) |
| 1 | `main · 0\|CONST\|Nat:2` | `main:1` | `[Nat:2]` | (empty) | (empty) |
| 2 | `main · 1\|CONST\|Nat:3` | `main:2` | `[Nat:2, Nat:3]` | (empty) | (empty) |
| 3 | `main · 2\|BINARY\|+` | `main:3` | `[Nat:5]` | (empty) | (empty) |
| 4 | `main · 3\|CONST\|Nat:4` | `main:4` | `[Nat:5, Nat:4]` | (empty) | (empty) |
| 5 | `main · 4\|BINARY\|*` | `main:5` | `[Nat:20]` | (empty) | (empty) |
| 6 | `main · 5\|STORE\|answer` | `main:6` | `[]` | `answer=Nat:20` | (empty) |
| 7 | `main · 6\|LOAD\|answer` | `main:7` | `[Nat:20]` | `answer=Nat:20` | (empty) |
| 8 | `main · 7\|CALL\|print\|1` | `main:8` | `[Void]` | `answer=Nat:20` | `20` |
| 9 | `main · 8\|RETURN` | `finished` | `no frame` | (empty) | `20` |

Output:

```text
20
```

### A conditional inside a function call

The caller passes Nat:7 to classify. Its comparison yields false, so instruction 3 jumps to instruction 6. The large branch is skipped. Returning Str:"small" resumes main at instruction 2.

Source: [`vm_branch_call.panack`](../examples/vm_branch_call.panack).

```panackelty
pure classify(value: Nat): Str {
  if value >= 10 {
    "large"
  } else {
    "small"
  }
}

main(): Void {
  print(classify(7))
}
```

```text
FUNCTION|classify|pure|value
0|LOAD|value
1|CONST|Nat:10
2|BINARY|>=
3|JUMP_FALSE|6
4|CONST|Str:large
5|JUMP|7
6|CONST|Str:small
7|RETURN
FUNCTION|main|impure|
0|CONST|Nat:7
1|CALL|classify|1
2|CALL|print|1
3|RETURN
```

At entry to `classify`, `main` is suspended at PC 2 with an empty stack. The callee has `value=Nat:7` and its own empty stack. The table shows the active frame; suspended frames retain their state.

| Step | Just executed | Next PC | Active stack | Active locals | Output so far |
| ---: | --- | --- | --- | --- | --- |
| 0 | `Start` | `main:0` | `[]` | (empty) | (empty) |
| 1 | `main · 0\|CONST\|Nat:7` | `main:1` | `[Nat:7]` | (empty) | (empty) |
| 2 | `main · 1\|CALL\|classify\|1` | `classify:0` | `[]` | `value=Nat:7` | (empty) |
| 3 | `classify · 0\|LOAD\|value` | `classify:1` | `[Nat:7]` | `value=Nat:7` | (empty) |
| 4 | `classify · 1\|CONST\|Nat:10` | `classify:2` | `[Nat:7, Nat:10]` | `value=Nat:7` | (empty) |
| 5 | `classify · 2\|BINARY\|>=` | `classify:3` | `[Bool:false]` | `value=Nat:7` | (empty) |
| 6 | `classify · 3\|JUMP_FALSE\|6` | `classify:6` | `[]` | `value=Nat:7` | (empty) |
| 7 | `classify · 6\|CONST\|Str:small` | `classify:7` | `[Str:"small"]` | `value=Nat:7` | (empty) |
| 8 | `classify · 7\|RETURN` | `main:2` | `[Str:"small"]` | (empty) | (empty) |
| 9 | `main · 2\|CALL\|print\|1` | `main:3` | `[Void]` | (empty) | `small` |
| 10 | `main · 3\|RETURN` | `finished` | `no frame` | (empty) | `small` |

Output:

```text
small
```

### A loop with an accumulator

The range 1..4 yields 1, 2, and 3. ITER_NEXT writes each item into a local. The body updates total and jumps back to instruction 6. A fourth ITER_NEXT finds the iterator exhausted and jumps to instruction 14.

Source: [`vm_loop.panack`](../examples/vm_loop.panack).

```panackelty
main(): Void {
  mut total: Nat = 0
  for item in 1..4 {
    total = total + item
  }
  print(total)
}
```

```text
FUNCTION|main|impure|
0|CONST|Nat:0
1|STORE|total
2|CONST|Nat:1
3|CONST|Nat:4
4|MAKE_RANGE
5|ITER_INIT|$iter0
6|ITER_NEXT|$iter0|item|14
7|LOAD|total
8|LOAD|item
9|BINARY|+
10|STORE|total
11|CONST|Void
12|POP
13|JUMP|6
14|LOAD|total
15|CALL|print|1
16|RETURN
```

| Step | Just executed | Next PC | Active stack | Active locals | Output so far |
| ---: | --- | --- | --- | --- | --- |
| 0 | `Start` | `main:0` | `[]` | (empty) | (empty) |
| 1 | `main · 0\|CONST\|Nat:0` | `main:1` | `[Nat:0]` | (empty) | (empty) |
| 2 | `main · 1\|STORE\|total` | `main:2` | `[]` | `total=Nat:0` | (empty) |
| 3 | `main · 2\|CONST\|Nat:1` | `main:3` | `[Nat:1]` | `total=Nat:0` | (empty) |
| 4 | `main · 3\|CONST\|Nat:4` | `main:4` | `[Nat:1, Nat:4]` | `total=Nat:0` | (empty) |
| 5 | `main · 4\|MAKE_RANGE` | `main:5` | `[Range[1, 4)]` | `total=Nat:0` | (empty) |
| 6 | `main · 5\|ITER_INIT\|$iter0` | `main:6` | `[]` | `total=Nat:0, $iter0=Iterator` | (empty) |
| 7 | `main · 6\|ITER_NEXT\|$iter0\|item\|14` | `main:7` | `[]` | `total=Nat:0, $iter0=Iterator, item=Nat:1` | (empty) |
| 8 | `main · 7\|LOAD\|total` | `main:8` | `[Nat:0]` | `total=Nat:0, $iter0=Iterator, item=Nat:1` | (empty) |
| 9 | `main · 8\|LOAD\|item` | `main:9` | `[Nat:0, Nat:1]` | `total=Nat:0, $iter0=Iterator, item=Nat:1` | (empty) |
| 10 | `main · 9\|BINARY\|+` | `main:10` | `[Nat:1]` | `total=Nat:0, $iter0=Iterator, item=Nat:1` | (empty) |
| 11 | `main · 10\|STORE\|total` | `main:11` | `[]` | `total=Nat:1, $iter0=Iterator, item=Nat:1` | (empty) |
| 12 | `main · 11\|CONST\|Void` | `main:12` | `[Void]` | `total=Nat:1, $iter0=Iterator, item=Nat:1` | (empty) |
| 13 | `main · 12\|POP` | `main:13` | `[]` | `total=Nat:1, $iter0=Iterator, item=Nat:1` | (empty) |
| 14 | `main · 13\|JUMP\|6` | `main:6` | `[]` | `total=Nat:1, $iter0=Iterator, item=Nat:1` | (empty) |
| 15 | `main · 6\|ITER_NEXT\|$iter0\|item\|14` | `main:7` | `[]` | `total=Nat:1, $iter0=Iterator, item=Nat:2` | (empty) |
| 16 | `main · 7\|LOAD\|total` | `main:8` | `[Nat:1]` | `total=Nat:1, $iter0=Iterator, item=Nat:2` | (empty) |
| 17 | `main · 8\|LOAD\|item` | `main:9` | `[Nat:1, Nat:2]` | `total=Nat:1, $iter0=Iterator, item=Nat:2` | (empty) |
| 18 | `main · 9\|BINARY\|+` | `main:10` | `[Nat:3]` | `total=Nat:1, $iter0=Iterator, item=Nat:2` | (empty) |
| 19 | `main · 10\|STORE\|total` | `main:11` | `[]` | `total=Nat:3, $iter0=Iterator, item=Nat:2` | (empty) |
| 20 | `main · 11\|CONST\|Void` | `main:12` | `[Void]` | `total=Nat:3, $iter0=Iterator, item=Nat:2` | (empty) |
| 21 | `main · 12\|POP` | `main:13` | `[]` | `total=Nat:3, $iter0=Iterator, item=Nat:2` | (empty) |
| 22 | `main · 13\|JUMP\|6` | `main:6` | `[]` | `total=Nat:3, $iter0=Iterator, item=Nat:2` | (empty) |
| 23 | `main · 6\|ITER_NEXT\|$iter0\|item\|14` | `main:7` | `[]` | `total=Nat:3, $iter0=Iterator, item=Nat:3` | (empty) |
| 24 | `main · 7\|LOAD\|total` | `main:8` | `[Nat:3]` | `total=Nat:3, $iter0=Iterator, item=Nat:3` | (empty) |
| 25 | `main · 8\|LOAD\|item` | `main:9` | `[Nat:3, Nat:3]` | `total=Nat:3, $iter0=Iterator, item=Nat:3` | (empty) |
| 26 | `main · 9\|BINARY\|+` | `main:10` | `[Nat:6]` | `total=Nat:3, $iter0=Iterator, item=Nat:3` | (empty) |
| 27 | `main · 10\|STORE\|total` | `main:11` | `[]` | `total=Nat:6, $iter0=Iterator, item=Nat:3` | (empty) |
| 28 | `main · 11\|CONST\|Void` | `main:12` | `[Void]` | `total=Nat:6, $iter0=Iterator, item=Nat:3` | (empty) |
| 29 | `main · 12\|POP` | `main:13` | `[]` | `total=Nat:6, $iter0=Iterator, item=Nat:3` | (empty) |
| 30 | `main · 13\|JUMP\|6` | `main:6` | `[]` | `total=Nat:6, $iter0=Iterator, item=Nat:3` | (empty) |
| 31 | `main · 6\|ITER_NEXT\|$iter0\|item\|14` | `main:14` | `[]` | `total=Nat:6, $iter0=Iterator, item=Nat:3` | (empty) |
| 32 | `main · 14\|LOAD\|total` | `main:15` | `[Nat:6]` | `total=Nat:6, $iter0=Iterator, item=Nat:3` | (empty) |
| 33 | `main · 15\|CALL\|print\|1` | `main:16` | `[Void]` | `total=Nat:6, $iter0=Iterator, item=Nat:3` | `6` |
| 34 | `main · 16\|RETURN` | `finished` | `no frame` | (empty) | `6` |

Output:

```text
6
```

Instructions 11 and 12 push and discard the loop body's `Void` result. They are real compiler output. The iterator and final item remain in the frame locals until the function returns; the reference VM does not remove them at the source block boundary.

## What verification guarantees

The loader checks the header, version, bounded lengths, constant encodings,
and instruction forms. Semantic verification then checks the entry point,
function identities, jump targets, named callees, argument counts, and purity
edges. A pure function cannot directly call an effectful built-in such as
`print`.

Verification does not perform complete stack-shape or source-type analysis.
Execution still checks operand-stack underflow, uninitialised locals, dynamic
operand types, bounds, missing map keys, and numeric failures. Indirect calls
recheck target existence, arity, and purity at runtime. Malformed bytecode
therefore cannot rely on having passed through the source checker.

For example, `Dec` division with a non-terminating decimal result traps rather
than silently rounding. A negative `Nat` result also traps. These checks
preserve the language's arithmetic rules even for forged instruction streams.
Resource exhaustion and process termination sit outside the language trap model.
Bytecode verification is not an operating-system sandbox: effectful programs
can use the host services available to their process.

## How values live in memory

The native VM uses reference-counted tagged values. `Bool` stores a flag in
its value object; `Unit` and `Void` have distinct tags and no payload. `Rat` owns
a normalized pair of arbitrary-precision integers.
Loading a value doesn't require a deep copy of its contents. Persistent updates
create a new container and retain its elements, leaving the original intact.

Assigning a mutable local replaces that local's value; it does not mutate an
existing collection. Frames own their stacks and locals, and returning from a
function releases those references while transferring the result to the caller.
The current value graph cannot contain cycles, so reference counting can reclaim
it without a tracing garbage collector. See the
[native value model](../src/vm/VALUE_MODEL.md) for ownership and allocation rules.

## Continue exploring

Use `./panack disasm examples/callables.panack` to see `CALL_VALUE` and the
lowering of array `map` and `reduce`. Inspect `examples/option_result.panack`
for variant construction and matching, or `examples/collections_and_bytes.panack`
for persistent collection operations. These programs share the same VM; their
higher-level syntax is expressed through the instruction families above.

To reproduce the examples' checks, run `make check` from the repository root.
The functional harness discovers the three `vm_*.panack` programs and compares
source and bytecode execution against their files under
`tests/functional/expected/examples/`. The walkthrough tables are explanatory
snapshots and must be reviewed if compiler lowering or disassembly changes.
