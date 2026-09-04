# Panackelty

**A small, expressive programming language for dependable terminal tools and
exact numerical work.**

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

## Start writing Panackelty

The developer preview is designed to be downloaded and run directly. Writing,
checking, compiling, and running Panackelty programs does not require Python,
`make`, a C compiler, or a copy of this repository. The download contains the
`panack` command, native VM, self-hosted compiler, and standard library.

Developer-preview archives are not published yet. The first release will be
`0.1.0-alpha.1`; until its archives appear on the [Releases page](../../releases),
use the build-from-source workflow below. The [release policy](RELEASE_POLICY.md)
defines the preview's support and compatibility boundaries.

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
| Linux x86-64 | `panackelty-0.1.0-alpha.1-linux-x86_64.tar.gz` |
| macOS arm64 | `panackelty-0.1.0-alpha.1-macos-arm64.tar.gz` |

Windows and other architectures are not part of the initial preview.

### Install a downloaded release

In the directory containing both downloaded files, verify the archive. On
Linux, run:

```sh
sha256sum -c panackelty-0.1.0-alpha.1-linux-x86_64.tar.gz.sha256
```

On macOS, run:

```sh
shasum -a 256 -c panackelty-0.1.0-alpha.1-macos-arm64.tar.gz.sha256
```

The command must report the archive as `OK`. Then unpack the matching archive;
the macOS name is shown here:

```sh
tar -xzf panackelty-0.1.0-alpha.1-macos-arm64.tar.gz
./panackelty/bin/panack --version
```

The version command prints `panack 0.1.0-alpha.1 (bytecode 7)`. The Linux
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

The complete output is:

<!-- quick-start-output-begin -->
```text
panack 0.1.0-alpha.1 (bytecode 7)
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
running complete command-line programs without Python.

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
- [The self-hosting guide](SELF_HOSTING.md) follows the bootstrap chain and its
  reproducibility guarantees.
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
and `make`. The complete development suite additionally requires Python 3.12;
Python implements the transitional test oracle and harness, not the toolchain
shipped to users.

From the repository root:

```sh
make                  # build the native VM as ./panack-vm
./panack --help       # use the toolchain from the checkout
make check            # run all tests and the reproducible-bootstrap proof
make check-compiler   # focused compiler checks with public-CLI coverage
make check-bytecode   # focused bytecode and artifact checks
make check-vm         # focused VM and host-boundary checks
```

The public compiler is written in Panackelty. To run it from source as an
ordinary Panackelty program:

```sh
./panack run src/compiler/main.panack -- compile examples/euler001.panack -o build/euler001.bc
```

Build and test the Python-free distribution path with:

```sh
make package PYTHON=false
```

This command completes the native conformance and reproducible-bootstrap gates,
builds the final archive, and smoke-tests that exact archive from a fresh
directory with Python, `make`, and a C compiler absent from `PATH`. It also
writes the archive's `.sha256` checksum and executes the packaged README's
quick start, upgrade, and removal procedures. Run
`make release-smoke PYTHON=false` to rebuild and exercise only the archive gate.

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
the 15-second incremental budget. The Python stage-0 implementation remains
only as a development oracle and for deliberate seed regeneration; the public
toolchain runs without it.

Continuous integration builds this package independently on Ubuntu 22.04
x86-64 and macOS 14 arm64. Each successful matrix job retains the exact archive,
its SHA-256 checksum, and a provenance record naming the source commit and
runner image. These routine CI jobs never publish a release.

Pushing an existing tag whose name exactly matches `v` plus `VERSION` starts the
separate release workflow. It reruns `make check`, rebuilds and smoke-tests both
platform archives, verifies the collected checksums and provenance, and only
then publishes a GitHub prerelease. Validation and package jobs are read-only;
repository write permission is scoped to the final publication job.

Project-wide contribution and validation expectations are documented in
[AGENTS.md](AGENTS.md).
