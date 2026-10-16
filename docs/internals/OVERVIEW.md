# How Panackelty works

## How it works

Every source program takes the same route to execution:

```text
program.panack  ->  compiler and type checker  ->  bytecode  ->  Panackelty VM
program.bc      ->  bytecode verifier          ->  bytecode  ->  Panackelty VM
```

The CLI remains synchronous. An experimental
[internal VM execution API](../../src/vm/README.md#internal-resumable-execution) supports
instruction-budgeted pause/resume. A bounded internal task experiment adds scoped
joins, cancellation and virtual deadlines using fake pending operations. These
experiments do not add source-language tasks, networking or `async/await`.

The loader treats bytecode as untrusted input. Before execution it validates the
format version, function and purity metadata, operands, calls, arities, and
control-flow targets.

`Option[T]`, `Result[T,E]` and their constructors are available without imports.
Text and collection operations use methods: `"hello.panack".ends_with(".panack")`,
`values.first()` and `values.sort_by(@comparator)`. Sorting is stable and requires
a pure comparator. Other library APIs remain explicit: import a module such as
`stdlib/path`, or `stdlib/prelude` for the combined surface. Logical imports work
from a checkout and an installed toolchain. See the
[standard-library guide](../../src/stdlib/README.md) and
[collections example](../../examples/collections_and_bytes.panack).
File-based namespaces are supported for the checked language subset described in
the [module contract](../reference/SPEC.md#modules). Use `project/` imports for files under the
entry source root and `stdlib/` for the active toolchain; aliases, selected
imports, qualified names and explicit public re-exports define module boundaries.
The compiler emits only fully checked bodies, keeps same-named declarations in
different modules distinct, and launches only the entry module's `main`. Imports
cannot escape the entry root. Namespaces shipped in alpha.12 and are supported in the current alpha.13 release. See the
[namespace examples](../../examples/namespaces) and [namespace progress](../roadmap/records/language-namespaces--idea.md#rm-41).

The separately imported `stdlib/testing` module provides pure assertions and
ordered result reporting for Panackelty test programs.
`stdlib/testing_files` adds sorted fixture-directory discovery and explicitly
owned temporary workspaces for test isolation.
`stdlib/testing_commands` compares bounded child-process results and expected
host failures without requiring a shell or decoding binary output.
During development, `make functional` uses the Panackelty-hosted runner for
the selected success cases, examples and failure cases, plus a self-hosted compiler check and the runner smoke case from source
and bytecode.
