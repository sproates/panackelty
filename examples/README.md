# Examples

This directory is the single home for user-facing Panackelty example programs.
Run any example from a repository checkout with:

```sh
./panack run examples/fizzbuzz.panack
```

The same directory is included in release archives. From the extracted
`panackelty/` directory, run it with:

```sh
./bin/panack run examples/fizzbuzz.panack
```

Every `.panack` file here is exercised by the functional suite from both source
and compiled bytecode. Expected output is kept separately under
`tests/functional/expected/examples` as test data rather than example code.

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

- `decimal.panack` — exact decimal arithmetic
- `guards.panack` — guarded domain types
- `callables.panack` — named pure function values and array `map`/`reduce`
- `collections_and_bytes.panack` — persistent arrays, maps, sets, and byte buffers
- `lexer_foundation.panack` — records, enums, strings, and lexer-style scanning
- `option_result.panack` — generic tagged unions and exhaustive matching
- `strings.panack` — Unicode indexing and interpolation
