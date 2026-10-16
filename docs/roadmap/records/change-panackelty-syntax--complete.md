[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Change Panackelty syntax — complete

<a id="rm-83"></a>

**RM#83: Syntax simplification**.

The accepted syntax removes redundant declaration keywords, uses a colon for
function return types, and distinguishes no-return functions with `Void`.

| Concern | Legacy syntax | Current syntax |
| --- | --- | --- |
| Function | `fn answer(): Nat` | `answer(): Nat` |
| Pure function | `pure fn answer(): Nat` | `pure answer(): Nat` |
| Immutable binding | `let answer: Nat = 42;` | `answer: Nat = 42` |
| Mutable binding | `let mut total: Nat = 0;` | `mut total: Nat = 0` |
| No returned value | `main(): Unit { () }` | `main(): Void {}` |
| Result failure | `Err(message)` | `Error(message)` |

- [x] Record the goals and non-goals of the syntax change
- [x] Write representative before-and-after examples
- [x] Draft and specify the revised lexical and grammar rules
- [x] Resolve declaration ambiguity by retaining mandatory type annotations
- [x] Adopt a clean break and reject the legacy syntax
- [x] Update `docs/reference/SPEC.md` with the accepted syntax and `Void` semantics
- [x] Update the bootstrap lexer, parser, checker, compiler, VM, and tests
- [x] Apply the same syntax to the Panackelty-hosted compiler sources
- [x] Update every example and user-facing command snippet
- [x] Advance the bytecode version for the `Void` value-tag change
