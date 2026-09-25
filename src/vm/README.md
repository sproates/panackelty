# Panackelty virtual machine

The Panackelty VM is a stack machine. Every call frame owns a program counter, local
variables, and an operand stack. Its instruction set covers constants, local
access, arithmetic, collection and algebraic-data construction, iteration,
control flow, calls, and returns.

Start with the [VM execution guide](../../docs/VM_GUIDE.md) for instruction
listings and worked programs showing the operand stack, locals, call frames,
and output at each step.

The portable C11 VM is the execution target. Independent fixed expectations
now replace live comparisons with the transitional Python VM. The latter remains
only for bootstrap-specific safeguards pending retirement. Seed regeneration
uses verified self-hosted stages on this VM.

The VM trusts neither source compilation nor bytecode files. Serialized
artifacts are verified before execution, and safety checks such as bounds
checking and `Nat` underflow remain enforced at runtime.

The frozen version-8 execution semantics and instruction stack effects live in
[`../bytecode/FORMAT.md`](../bytecode/FORMAT.md). The VM converts invalid dynamic
bytecode state into a Panackelty trap so host-language indexing, lookup, type,
and arithmetic exceptions do not cross the runtime boundary. A shared forged
runtime corpus exercises indirect-call validation, `Nat` underflow, zero
division, invalid byte and UTF-8 values, missing map keys, and single- and
multi-value stack underflow against both VM implementations.

The native representation, ownership, allocation, and reclamation rules are
specified in [`VALUE_MODEL.md`](VALUE_MODEL.md).

`main.c` is the entry point for the portable C11 seed executable. Its `check` command performs
bounded version-8 decoding and independent semantic verification; `run`
executes verified artifacts with the reference-counted value model, exact
numerics, persistent collections, UTF-8 operations, and stable host ABI. It
accepts and runs the complete compiler and standard-library artifacts and
consumes the same malformed vectors as the bootstrap loader. Build it with
`make native`. The build defaults to `CFLAGS=-O2`, retaining strict C11 and
warning checks. `CC`, `CPPFLAGS`, `CFLAGS`, `LDFLAGS`, and `LDLIBS` are
configurable. Run `make clean` before changing flags, for example before
`make native CFLAGS="-O0 -g"` for debugging. Optimisation does not change the
bytecode contract or disable runtime verification.

String construction records code-point count and whether every byte is ASCII.
Length is constant time, and ASCII indexing, slicing, and prefix-offset lookup
avoid rescanning the string. Non-ASCII offsets retain UTF-8 traversal. Every
string-producing operation uses the same constructor, including concatenation,
interpolation, slicing, reversal, decoding, and host inputs; bounds and UTF-8
validation remain in place.

Rational arithmetic uses normalized arbitrary-precision numerator/denominator
pairs. `Unit` has its own runtime tag; `.nat()` and `.dec()` perform exact checked
conversions and trap rather than discard precision.

Opaque paths and time values are implemented in `host_types.c`. Lexical paths
preserve native bytes; durations and instants use exact integer storage. Typed filesystem, process, and sleep operations are implemented separately
in `host_capabilities.c`, including descriptor ownership, concurrent stream
collection, resource limits, and structured host failures. See `VALUE_MODEL.md` and
`../../SPEC.md` for ownership, signatures, error behavior, and clock scope.

## Finding the implementation

Read `main.c` for the complete load → decode → verify → execute path. Each C file
is a separate translation unit; headers declare shared types and contracts, and
private helpers remain static. `Reader` belongs to `decode.c`; `Frame` and `Local`
belong to `execute.c`. No implementation is included through a header.

| Component | Files | Responsibility |
| --- | --- | --- |
| Program model | `program.h`, `program.c` | Decoded constants, named functions, instructions, cleanup |
| Loader | `decode.h`, `decode.c` | Bounded version-8 decoding and format checks |
| Verifier | `verify.h`, `verify.c` | Canonical order, jumps, call arity and purity |
| Values | `value.h`, `value.c` | Tagged values, constructors, equality, reference counting |
| Numerics | `numeric.h`, `numeric.c`, `bigint.h`, `bigint.c` | Exact arithmetic and checked conversions |
| Text support | `buffer.*`, `render.*`, `utf8.*` | Byte buffers, value formatting, UTF-8 traversal |
| Execution | `vm.h`, `execute.c` | Invocation context, frames, stack, locals, opcode dispatch |
| Builtins | `builtins.h`, `builtins.c`, `builtins_internal.h` | Shared arity/purity/handler registry |
| Builtin domains | `builtins_text.c`, `builtins_collections.c`, `builtins_numeric.c`, `builtins_vm.c` | Operations and nested bytecode execution |
| Host boundary | `host.*`, `host_types.*`, `host_capabilities.*` | Invocation snapshots, legacy I/O, paths, clocks and typed services |

`program.h` names every opcode with an explicit version-8 wire value. The decoder
and executor share those names. Numeric operations report through an error pointer
and do not depend on VM state. The verifier and executor use one builtin registry,
so effects, arities and implementation routing are kept together.

## Ownership and editing conventions

Constructors normally return one owned reference. Collection constructors retain
children; stack pop transfers a reference; local lookup borrows one. Header
comments identify exceptions such as the moving decimal constructor and consuming
host-result wrapper. See `VALUE_MODEL.md` for the full contract.

Use four spaces, braces for control statements, descriptive names, and blank lines
between validation, work, and cleanup. Comments should explain ownership or an
invariant rather than narrate obvious assignments. `src/vm/.clang-format` records
these conventions; run `clang-format -i src/vm/*.c src/vm/*.h` when editing C.
The formatter is a development convenience, not a build dependency.

`make native` compiles components separately and tracks header dependencies.
`make check-vm` includes direct C module contracts, independent header compilation,
registry parity with fixed independent signatures, and the existing native and public CLI corpus.
`make native-unit` runs only the C contracts. `make native-sanitize` builds isolated
AddressSanitizer/UndefinedBehaviorSanitizer binaries, runs those contracts, and
runs the native loader and execution suites against the instrumented runner.
It requires a compiler/runtime supporting those sanitizers and does not replace
`panack-vm`. Leak detection depends on host sanitizer support; reference-count
assertions check the tested ownership paths on every host.

`make native-fault` sweeps allocation failures and selected host syscall failures
in a separate test-only build. The normal VM suite additionally sweeps richer
compiled programs, including nested execution and traps with live caller values.
`make native-coverage` requires Clang and matching `llvm-cov`/`llvm-profdata`
(on macOS, Xcode command-line tools work) and writes `build/coverage/summary.txt`
and `build/coverage/html/`. CI runs sanitizers and uploads the coverage report.
These targets include seeded arithmetic properties, persistent-value lifetimes,
and deterministic mutation of every operand and constant form. They supplement,
but do not prove, memory safety; exhaustive host failures and coverage-guided
fuzzing remain follow-up work.


Direct VM execution and loader contracts run in `tests/runner/vm_unit.panack`
against the portable corpus in `tests/fixtures/vm_contracts`. Its 174 assertions
include native module, bigint and allocation-failure wrappers; header isolation
runs in `tests/native_headers.sh`. `make native-vm-contracts` runs this group,
and `make unit`, `make check-vm`, sanitizer and coverage gates include it.
The 61 former Python VM observations use fixed native contracts, including 21
per-artifact C return-kind assertions. Fixed independent arithmetic expectations
and builtin signatures run in `make native-oracle-contracts`.

Direct host, runtime and standard-library assertions run in
`tests/runner/host_runtime_unit.panack` with reviewed source and malformed
bytecode fixtures in `tests/fixtures/host_runtime`. The probe asserts native
process, file, path, environment and timing contracts and exact testing-library
reports. Direct C host checks and forced failures run under instrumentation.
The [migration inventory](../../tests/fixtures/host_runtime/README.md) maps all 37
former methods: 31 migrated to direct native evidence and the final six replaced
by fixed oracle fixtures and native/bootstrap cross-checks. Functional source and bytecode cases still verify
public behaviour on both supported platforms.
