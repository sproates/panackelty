# Panackelty

**A small, expressive programming language for dependable terminal tools and
exact numerical work.**

[Website](https://panackelty.com) · [Specification](SPEC.md) ·
[Releases](../../releases) · [Contributing](CONTRIBUTING.md)

Panackelty combines arbitrary-precision numbers, checked domain types, explicit
effects, and a portable bytecode VM. Its syntax stays compact enough for a quick
script while its compiler catches the mistakes that become expensive when a
program grows.

<p align="center">
  <img src="docs/assets/panackelty.jpg" width="800" alt="A home-cooked plate of panackelty with beef, potatoes, carrots, and peas in gravy">
</p>
<p align="center">
  <sub>
    A home-cooked plate of panackelty, the North East English dish behind the
    name. Photograph by the project author,
    <a href="docs/assets/README.md">licensed under CC BY 4.0</a>.
  </sub>
</p>

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
`quotient(a, b)` retains truncating natural division. Exact conversions trap if
the value cannot be represented: `(1/3).nat()` and `(1/3).dec()` fail rather than
round. `Unit` is a first-class value for APIs such as `Result[Unit,Str]`.
See [the specification](SPEC.md#rational-arithmetic-and-exact-conversions).

## Typed paths and elapsed time

Panackelty includes opaque `Path`, exact signed nanosecond
`Duration`, and monotonic `Instant` values. Import `stdlib/path` and
`stdlib/time`. Path construction checks text or native bytes; lexical operations
preserve spelling and do not access the filesystem. Durations support exact
arithmetic and checked fractional conversions. `instant_now()` is effectful
and returns a `Result`; arithmetic on existing clock readings is pure.

Typed filesystem and process APIs are available through `stdlib/filesystem`
and `stdlib/process`. They accept `Path` values and return structured errors.
Process execution supports byte streams, working directories, environment
overrides, output limits, and exact-duration timeouts; `host_sleep` provides
checked sleep. See the [host API contract](SPEC.md#typed-filesystem-process-and-sleep-apis)
and [runnable example](tests/functional/cases/host_capabilities/main.panack).

The [API specification](SPEC.md#paths-and-monotonic-time) and
[executable example](tests/functional/cases/host_types/main.panack) show their
contracts and use.

## Start writing Panackelty

The developer preview is designed to be downloaded and run directly. The
download contains everything needed to check, compile, and run programs: the
`panack` command, native VM, self-hosted compiler, and standard library.

Developer-preview archives for `0.1.0-alpha.9` are available from the
[GitHub release](https://github.com/sproates/panackelty/releases/tag/v0.1.0-alpha.9).
The [release policy](RELEASE_POLICY.md) defines the preview's support and
compatibility boundaries.

### System requirements

The initial preview supports:

- Linux on x86-64
- macOS on Apple silicon (arm64)
- A terminal and `tar` for unpacking the download
- `sha256sum` on Linux or `shasum` on macOS for verifying the download

Download both the archive matching the operating system and processor and its
adjacent `.sha256` file from the Releases page:

| System | Archive |
| --- | --- |
| Linux x86-64 | `panackelty-0.1.0-alpha.9-linux-x86_64.tar.gz` |
| macOS arm64 | `panackelty-0.1.0-alpha.9-macos-arm64.tar.gz` |

Windows and other architectures are not part of the initial preview.

### Install a downloaded release

In the directory containing both downloaded files, verify the archive. On
Linux, run:

```sh
sha256sum -c panackelty-0.1.0-alpha.9-linux-x86_64.tar.gz.sha256
```

On macOS, run:

```sh
shasum -a 256 -c panackelty-0.1.0-alpha.9-macos-arm64.tar.gz.sha256
```

The command must report the archive as `OK`. Then unpack the matching archive;
the macOS name is shown here:

```sh
tar -xzf panackelty-0.1.0-alpha.9-macos-arm64.tar.gz
./panackelty/bin/panack --version
```

The version command prints `panack 0.1.0-alpha.9 (bytecode 8)`. The Linux
archive follows the same layout and uses `linux-x86_64` in its name. To make
`panack` available in future terminal sessions, keep the whole extracted
directory together and link its command into a directory on `PATH`:

```sh
mkdir -p "$HOME/.local/opt" "$HOME/.local/bin"
mv panackelty "$HOME/.local/opt/panackelty"
ln -s "$HOME/.local/opt/panackelty/bin/panack" "$HOME/.local/bin/panack"
export PATH="$HOME/.local/bin:$PATH"
```

Add the final `export` command to the shell's startup file if `~/.local/bin` is
not already on `PATH`. The complete toolchain remains under
`~/.local/opt/panackelty`.

### Write and run a first program

Save this as `hello.panack`:

<!-- quick-start-program-begin -->
```panackelty
pure greeting(name: Str, answer: Nat): Str {
  "Hello, ${name}. The answer is ${answer}."
}

main(): Void {
  print(greeting("Ada", 42))
}
```
<!-- quick-start-program-end -->

Check the program, run its source, compile it to `hello.bc`, and run the saved
bytecode:

```sh
panack --version
panack check hello.panack
panack run hello.panack
panack compile hello.panack
panack run hello.bc
```

Primary lexer, parser, name, and type failures identify their owning source as
`file:line:column`, including when the error is in an imported module.
Positioned errors also show the source line and a caret. Tabs expand to
four-column stops; Unicode and control characters appear as `\u{hex}` escapes
to keep the caret aligned.

The complete output is:

<!-- quick-start-output-begin -->
```text
panack 0.1.0-alpha.9 (bytecode 8)
ok
Hello, Ada. The answer is 42.
wrote hello.bc
Hello, Ada. The answer is 42.
```
<!-- quick-start-output-end -->

`panack hello.panack` is shorthand for `panack run hello.panack`. Functions need
no `fn` keyword, and the final expression in a non-`Void` function is its return
value.

### Upgrade or remove Panackelty

To upgrade, download and verify the new archive, extract it in a working
directory, and replace the installed directory while keeping the command link:

```sh
mv panackelty "$HOME/.local/opt/panackelty.new"
mv "$HOME/.local/opt/panackelty" "$HOME/.local/opt/panackelty.old"
mv "$HOME/.local/opt/panackelty.new" "$HOME/.local/opt/panackelty"
panack --version
rm -rf "$HOME/.local/opt/panackelty.old"
```

Do not begin if either temporary path already exists. If the new version does
not run correctly, move `panackelty.old` back before deleting it. To remove
Panackelty completely:

```sh
rm "$HOME/.local/bin/panack"
rm -rf "$HOME/.local/opt/panackelty"
```

## What's in the name?

[Panackelty](https://en.wikipedia.org/wiki/Panackelty) is a traditional North
East English dish, particularly associated with Sunderland and County Durham.
It slowly brings simple ingredients—usually meat, potatoes, onions, and other
root vegetables—together in one pan. **Panack** is the short form, and the name
of this project's command-line tool.

The name fits the language: Panackelty is built from a deliberately small set of
ingredients that work well together, and everything ends up in one dependable
runtime.

## Why Panackelty?

- **Numbers mean what they say.** `Nat` and `Int` are arbitrary precision, and
  `Dec` uses exact base-10 arithmetic rather than binary floating point.
- **Make invalid values harder to express.** Guarded types attach checked domain
  rules—such as a valid port range—to ordinary scalar values.
- **See side effects at a glance.** `pure` functions cannot quietly perform I/O
  or call effectful code; local loops and mutation are still available when
  they make an algorithm clearer.
- **Model real programs directly.** Records, generic tagged unions, exhaustive
  matching, named callable values, persistent collections, Unicode strings,
  and modules are built in.
- **Ship one execution model.** Source always compiles to versioned bytecode,
  which is verified before the Panackelty VM runs it.
- **Trust the bootstrap story.** The public compiler is written in Panackelty,
  and the project checks that successive compiler builds are byte-for-byte
  identical.

Panackelty is experimental, but it is already capable of compiling itself and
running complete command-line programs.

`else` is optional for a conditional used only for control flow. A conditional
that produces a value remains exhaustive and requires both branches.

## A quick language tour

Every linked program is included in the download archive and exercised from
both source and compiled bytecode by the functional suite. From a source
checkout, run an example with `./panack run examples/NAME.panack`. From the root
of an extracted archive, use `./bin/panack run examples/NAME.panack`.

| Feature | Runnable example | Specification |
| --- | --- | --- |
| Compact functions and conditional values | [`fizzbuzz.panack`](examples/fizzbuzz.panack) | [Declarations, functions, and `Void`](SPEC.md#declarations-functions-and-void) |
| Arbitrary-precision integers and exact decimals | [`decimal.panack`](examples/decimal.panack) and [`euler003.panack`](examples/euler003.panack) | [Values and numeric semantics](SPEC.md#values-and-numeric-semantics) |
| Guarded domain types | [`guards.panack`](examples/guards.panack) | [Guarded types](SPEC.md#guarded-types) |
| Pure functions with local loops and mutation | [`euler001_iterative.panack`](examples/euler001_iterative.panack) | [Effects and purity](SPEC.md#effects-and-purity) and [ranges, arrays, and loops](SPEC.md#ranges-arrays-and-loops) |
| Unicode strings, indexing, and interpolation | [`strings.panack`](examples/strings.panack) | [Strings](SPEC.md#strings) |
| Records, generic tagged unions, and exhaustive matching | [`lexer_foundation.panack`](examples/lexer_foundation.panack) and [`option_result.panack`](examples/option_result.panack) | [Records and tagged unions](SPEC.md#records-and-tagged-unions) |
| Named callable values and higher-order arrays | [`callables.panack`](examples/callables.panack) | [Effects and purity](SPEC.md#effects-and-purity) |
| Persistent arrays, maps, sets, and byte buffers | [`collections_and_bytes.panack`](examples/collections_and_bytes.panack) | [Persistent collections and bytes](SPEC.md#persistent-collections-and-bytes) |
| Logical modules and the bundled standard library | [`option_result.panack`](examples/option_result.panack) | [Modules](SPEC.md#modules) and [standard library](SPEC.md#standard-library) |
| One verified bytecode execution model | Any example above | [Compilation and the Panackelty VM](SPEC.md#compilation-and-the-panackelty-vm) |

The examples deliberately stay small enough to modify. The
[`examples` guide](examples/README.md) continues with complete algorithms,
including palindrome detection, memoized Fibonacci, and Project Euler 1–5.

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
type. Function signatures remain explicit. See the [binding rules](SPEC.md#declarations-functions-and-void).

### Generic functions

Functions can work with different types while
retaining compile-time checks:

```panackelty
pure identity[T](value: T): T { value }

main(): Void {
  print(identity(42))
  print(identity[Str]("hello"))
}
```

Calls infer type arguments from their inputs, or accept an explicit complete
list. Generic bodies are checked even when unused; unconstrained `T` does not
permit arithmetic. See the [generic function rules](SPEC.md#generic-source-functions)
and [runnable example](examples/generic_functions.panack) for library helpers,
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

The compiler only accepts a conversion when it can prove the guard. It does not
hide a runtime check inside otherwise pure code.

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
enum Result[T, E] { Ok(T), Error(E) }

pure safe_divide(numerator: Nat, denominator: Nat): Result[Nat,Str] {
  if denominator == 0 {
    Error("division by zero")
  } else {
    Ok(numerator / denominator)
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

## Use the toolchain

Panack source files use the `.panack` extension. Compiled programs use `.bc`.

```sh
# Type-check without producing bytecode
panack check hello.panack

# Compile beside the source, or choose an output path
panack compile hello.panack
panack compile hello.panack -o hello-release.bc

# Run source or verified bytecode
panack run hello.panack
panack run hello.bc

# Inspect the generated instructions
panack disasm hello.panack
panack disasm hello.bc
```

Arguments after the input path are passed to the program.

## How it works

Every source program takes the same route to execution:

```text
program.panack  ->  compiler and type checker  ->  bytecode  ->  Panackelty VM
program.bc      ->  bytecode verifier          ->  bytecode  ->  Panackelty VM
```

The loader treats bytecode as untrusted input. Before execution it validates the
format version, function and purity metadata, operands, calls, arities, and
control-flow targets.

The standard library is explicit: use `import stdlib/prelude` for the complete
surface, or logical imports such as `import stdlib/option`,
`import stdlib/result`, and `import stdlib/text`. These names work from a source
checkout and an installed toolchain without exposing its directory layout. See the
[standard-library guide](src/stdlib/README.md) for the available APIs.
The separately imported `stdlib/testing` module provides pure assertions and
ordered result reporting for Panackelty test programs.
`stdlib/testing_files` adds sorted fixture-directory discovery and explicitly
owned temporary workspaces for test isolation.
`stdlib/testing_commands` compares bounded child-process results and expected
host failures without requiring a shell or decoding binary output.
During development, `make functional` uses the Panackelty-hosted runner for
twenty-five selected success cases, twenty examples, and forty-one failure
cases, plus a self-hosted compiler check and the runner smoke case from source
and bytecode.

## Explore further

- [`examples/`](examples) is the single home for runnable example code. Its
  [guide](examples/README.md) covers Project Euler 1–5, palindrome testing,
  FizzBuzz, recursive memoized and iterative Fibonacci, exact decimals, guarded
  types, strings, tagged unions, callable values, persistent collections, and
  byte buffers.
- [The language specification](SPEC.md) defines syntax, types, effects, runtime
  semantics, and deliberate limitations.
- [The architecture guide](ARCHITECTURE.md) explains the compiler, bytecode,
  verifier, VM, and repository layout.
- [The VM execution guide](docs/VM_GUIDE.md) follows arithmetic, function calls,
  branches, and loops through real disassembly and step-by-step stack traces.
- [The self-hosting guide](SELF_HOSTING.md) follows the bootstrap chain and its
  reproducibility guarantees.
- [The testing guide](tests/README.md) describes the test suites and how to run them.
- [The roadmap](ROADMAP.md) tracks upcoming language and engineering work.
- [The release policy](RELEASE_POLICY.md) defines preview stability, supported
  systems, compatibility, and support lifetime.
- [The changelog](CHANGELOG.md) records notable release changes and known
  preview limitations.
- [The security policy](SECURITY.md) explains how to report vulnerabilities.
- [The contribution guide](CONTRIBUTING.md) explains how to open the structured
  **Bug report** form and covers development expectations.
- Panackelty is available under the [MIT License](LICENSE).

## Build Panackelty itself

This workflow is for contributors and people who want to build or inspect the
toolchain itself. Normal Panackelty programs should use a downloaded release.

A source build requires a POSIX-like Linux or macOS environment, a C11 compiler,
and `make`, plus standard POSIX utilities and a SHA-256 utility. The complete
development suite additionally requires Git for release-tag safety tests.
`make policy` rejects source-tree
interpreter dependencies; `make check-no-interpreter` runs a clean full check,
native conformance and packaging with only explicitly allowed tools in `PATH`.

The checkout directory may contain spaces and parentheses. The native VM builds
with `-O2` by default. Standard `CC`, `CPPFLAGS`, `CFLAGS`, `LDFLAGS`, and
`LDLIBS` overrides are supported; for a debug build, run `make clean` followed
by `make CFLAGS="-O0 -g"`. Clean before changing flags because existing binaries
do not automatically rebuild when command-line flags change.

From the repository root:

```sh
make                  # build the native VM as ./panack-vm
./panack --help       # use the toolchain from the checkout
make check            # run all tests and the reproducible-bootstrap proof
make check-compiler   # focused compiler checks with public-CLI coverage
make check-bytecode   # focused bytecode and artifact checks
make check-vm         # focused VM and host-boundary checks
make native-fault     # allocation and host failure/cleanup contracts
make native-sanitize  # address/undefined-behaviour checks (also run in CI)
make native-coverage  # LLVM line/branch report and HTML under build/coverage
```


Direct bytecode/verification coverage runs in
`tests/runner/bytecode_unit.panack`, `tests/runner/bytecode_native_unit.panack`
and the native C verifier contracts in `tests/unit/vm/native_modules.c`.
These share fixed version-8 and malformed artifact vectors and compare exact
canonical artifacts and disassemblies. Fixture provenance and wire-format
expectations are documented in
`tests/fixtures/bytecode/contract_cases/README.md`.

The compiler check includes the complete direct Panackelty compiler assertions,
fixed independent expectations and public CLI fixtures. The same
assertions also run in `make unit`, whose timer includes the shell harness,
Panackelty probes and native C contracts.

The public compiler is written in Panackelty. To run it from source as an
ordinary Panackelty program:

```sh
./panack run src/compiler/main.panack -- compile examples/euler001.panack -o build/euler001.bc
```

Internal test probes reuse compiled bytecode when their source and toolchain
contents match. Every check still executes the tests. `make clean` removes the
cache; see [the testing guide](tests/README.md) for the reuse and isolation rules.

Build and test the distribution with:

```sh
make package
```

This command completes the native conformance and reproducible-bootstrap gates,
builds the final archive, and smoke-tests that exact archive from a fresh
directory with only runtime tools available in `PATH`. It also
writes the archive's `.sha256` checksum and executes the packaged README's
quick start, upgrade, and removal procedures. Run
`make release-smoke` to rebuild and exercise only the archive gate.

The release archive is written to
`build/panackelty-VERSION-OS-ARCH.tar.gz`, with fields derived from `VERSION` and
the build host. Verify its adjacent checksum with `sha256sum -c FILE.sha256` on
Linux or `shasum -a 256 -c FILE.sha256` on macOS. The archive expands into one
relocatable `panackelty/` directory with
`bin`, `libexec`, and `share` subdirectories plus top-level `README.md` and
`LICENSE` files. The directory may be moved after extraction; `bin/panack`
discovers the VM, compiler, version metadata, and standard library relative to
its own location.

Install a source build under a chosen prefix with
`make install PREFIX=/desired/prefix`; `DESTDIR` is also supported for staged
installation. This conventional installation interface remains separate from
the download-friendly archive layout.

The full validation suite includes focused compiler and VM tests, black-box
program tests through `panack`, and a fixed-point bootstrap proof. The three
component checks retain representative public-command evidence while targeting
the 15-second incremental budget. The former stage-0 implementation and
compatibility facade are removed. `make regenerate-seed` now verifies the recorded seed
digest, builds fresh self-hosted stages, and checks compiler/library identity
before replacing the seed. See [the refresh procedure](bootstrap/README.md).

Continuous integration runs on pull-request updates and pushes to `main`.
New commits cancel older runs for the same pull request. For implementation, mixed or uncertain changes, the test job runs `make check`
once; the focused targets above remain available for local work. Changes limited
to the explicit informational-document allowlist receive quick document/local-link
checks, without compiler builds, packaging, sanitizers or coverage. The existing
check names remain present and reject routing failures. See
[change-aware CI](tests/README.md#change-aware-ci) and use `make docs` locally
for those informational edits.
Opt-in [detailed validation profiling](tests/README.md#detailed-validation-profiling)
separates native builds, harness groups, probes and bootstrap costs.

Continuous integration builds this package independently on Ubuntu 22.04
x86-64 and macOS 14 arm64. For full validation, both jobs run `make check-no-interpreter`, covering
a clean full check and packaging with only allowlisted commands visible.
Each successful matrix job retains the exact archive,
its SHA-256 checksum, and a provenance record naming the source commit and
runner image. These routine CI jobs never publish a release.

Pushing an existing tag whose name exactly matches `v` plus `VERSION` starts the
separate release workflow. It reruns `make check`, rebuilds and smoke-tests both
platform archives, verifies the collected checksums and provenance, and only
then publishes a GitHub prerelease. Validation and package jobs are read-only;
repository write permission is scoped to the final publication job.

Project-wide contribution and validation expectations are documented in
[AGENTS.md](AGENTS.md).

The remaining direct compiler contracts now run in
`tests/runner/compiler_contracts_unit.panack` (201 assertions) and
`tests/runner/compiler_integration_unit.panack` (51 assertions), under both
`make unit` and `make check-compiler`. They cover emitter instructions, diagnostic
rendering and source snapshots, loader/imports and driver commands, generics,
inference, types and host boundaries. The probes use fixed expectations;
their provenance is recorded in
`tests/ORACLE_REPLACEMENT.md`. Seed regeneration uses verified self-hosted stages.


Direct VM execution and loader contracts run in `tests/runner/vm_unit.panack`
against the portable corpus in `tests/fixtures/vm_contracts`. Its 174 assertions
include native module, bigint and allocation-failure wrappers; header isolation
runs in `tests/native_headers.sh`. `make native-vm-contracts` runs this group,
and `make unit`, `make check-vm`, sanitizer and coverage gates include it.
The VM corpus checks 61 fixed execution contracts, including 21
per-artifact C return-kind assertions. Fixed independent arithmetic expectations
and builtin signatures run in `make native-oracle-contracts`.

Direct host, runtime and standard-library assertions run in
`tests/runner/host_runtime_unit.panack` with reviewed source and malformed
bytecode fixtures in `tests/fixtures/host_runtime`. The probe asserts native
process, file, path, environment and timing contracts and exact testing-library
reports. Direct C host checks and forced failures run under instrumentation.
The [fixture guide](tests/fixtures/host_runtime/README.md) describes the direct
native checks, fixed oracle fixtures and bootstrap cross-checks. Functional
source and bytecode cases verify public behaviour on both supported platforms.

Run development harness checks with `make harness`.
They cover repository/CI policy, packaging, timing, corrupt seeds and fixture-runner
failure propagation on both supported platforms.
