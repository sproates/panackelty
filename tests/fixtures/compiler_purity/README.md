# Fixed purity source cases

These 10 source files preserve the exact former Python oracle inputs,
including deliberately invalid programs. They are not standalone examples.
`tests/runner/compiler_purity_unit.panack` imports the real compiler module,
requires exactly `ok` for success, and asserts fixed diagnostic substrings for
failures. Its 1 loaded-module cases remain inline.

The live Python differential comparison is retired. The original oracle passed
before retirement; fixed expectations are not generated from the implementation
under test. Names retain the original method and expanded ordinal. When changing
a case, review its expected outcome independently and update the native probe,
`tests/COVERAGE.md` and the migration inventory together. Missing inputs fail.
Both `make unit` and `make check-compiler` run the probe.

See `tests/ORACLE_REPLACEMENT.md` for retirement evidence and remaining gates.
