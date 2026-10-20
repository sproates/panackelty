# Examples

This directory is the single home for user-facing Panackelty example programs.
Run any example from a repository checkout with:

```sh
./bin/panack run examples/fizzbuzz.panack
```

The same directory is included in release archives. From the extracted
`panackelty/` directory, run it with:

```sh
./bin/panack run examples/fizzbuzz.panack
```

Every runnable example entry is exercised by both functional runners from
source and compiled bytecode. Namespace support modules are compiled and run
through their example entry. Expected output is kept separately under
`tests/functional/expected/examples` as test data rather than example code.

## VM walkthroughs

The [VM execution guide](../docs/guides/VM_GUIDE.md) includes disassembly and recorded
execution states for these small programs:

| Program | Demonstrates |
| --- | --- |
| `vm_arithmetic.panack` | Operand order, intermediate stack values, and local storage |
| `vm_branch_call.panack` | An isolated call frame, comparison, conditional jump, and return |
| `vm_loop.panack` | A half-open range, iterator state, accumulator, and backward jump |

## Algorithms

| Program | Demonstrates |
| --- | --- |
| `palindrome.panack` | Two-pointer string comparison without reversing the string |
| `fizzbuzz.panack` | Conditional logic and a half-open range |
| `fibonacci.panack` | Recursive Fibonacci with a persistent memo map and an iterative solution |

The palindrome test compares Unicode code points exactly. It does not remove
spaces, punctuation, or differences in letter case before comparing.

## Project Euler

| Program | Problem and approach |
| --- | --- |
| `euler001.panack` | Multiples of 3 or 5, recursively |
| `euler001_iterative.panack` | Multiples of 3 or 5, with a loop |
| `euler002.panack` | Sum even Fibonacci terms below four million |
| `euler003.panack` | Largest prime factor by trial division |
| `euler004.panack` | Largest palindromic product of two 3-digit numbers |
| `euler005.panack` | Smallest multiple using greatest and least common divisors |

## Language features

The other programs focus on individual language features:

- `generic_functions.panack` — generic calls, library helpers, explicit empty-array
  types, receiver-first syntax, and recursion
- `decimal.panack` — exact decimal arithmetic
- `guards.panack` — guarded domain types
- `callables.panack` — named pure function values and array `map`/`reduce`
- `collections_and_bytes.panack` — persistent arrays, maps, sets, byte buffers, stable sorting and suffix filtering
- `lexer_foundation.panack` — records, enums, strings, and lexer-style scanning
- `option_result.panack` — generic tagged unions and exhaustive matching
- `strings.panack` — Unicode indexing and interpolation

## Type-owned methods (alpha.14)

These examples require the unreleased `next` toolchain; published alpha.13 does
not support these `impl` declarations. Build from source before running them.

| Program | Demonstrates |
| --- | --- |
| [owned_methods.panack](owned_methods.panack) | An Inventory with its own `get`, `put` and `add` methods, Map operations inside those methods, chaining and preservation of previous values |
| [generic_methods.panack](generic_methods.panack) | A generic Box, receiver and method type inference, pure callbacks through `.call()`, and mapping from a number to text |

```sh
./bin/panack run examples/owned_methods.panack
./bin/panack run examples/generic_methods.panack
```

`Inventory.put` and `Map.put` share a spelling but select different declarations
from their receiver types. `self` is the current receiver. Generic methods use
the same dot syntax; `Box.map` infers its result type from the callback.
See the [migration guide](../docs/guides/METHOD_MIGRATION.md) for the final syntax
and removed compatibility APIs.

## Namespaces

These examples use the currently checked namespace subset and are exercised
from source and saved bytecode by the functional suite:

| Program | Demonstrates |
| --- | --- |
| `namespaces/trip_planner/main.panack` | A multi-file itinerary that combines route, weather, and budget modules through aliases and selected imports; only the entry module starts the program |
| `namespaces/twin_sensors/main.panack` | Two modules with distinct, same-named `Reading` record types and qualified type references |
| `namespaces/public_api/main.panack` | A consumer using a public API facade that re-exports a declaration backed by a private helper |

Each namespace example is self-contained in its directory. Run the trip planner
from the repository root with `./bin/panack run examples/namespaces/trip_planner/main.panack`;
use the corresponding `main.panack` path for the other examples.

## Networking

See the [network examples guide](network/README.md) for TCP exchanges and finite
HTTP sessions. For serving a directory, use the [static HTTP server](../tools/http-server/README.md).
The one-client HTTP example is a regression fixture, not a long-running application template.

## Adding an example

Add a matching `.stdout` file under `tests/functional/expected/examples/`, then
update the example count in `tests/runner/main.panack` and the ordered example
list and transcript total in `tests/functional/cases/runner_smoke/main.panack`.
Run `sh tests/fixture_inventory.sh` for immediate bookkeeping feedback before
`make check`. See the [preflight contract](../tests/README.md#fixture-inventory-preflight).
