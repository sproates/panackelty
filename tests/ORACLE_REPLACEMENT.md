# Python differential oracle replacement

The live Python compiler/VM comparisons are retired. Repository-wide removal
is also complete; the final 21 implementation-only safeguards retired with their
implementation. See [the removal audit](PYTHON_REMOVAL.md).
Development-harness tests now use shell/native checks. Language semantics, bytecode version 8
and the checked compiler seed are unchanged.

## Evidence and independence

Before retirement, all 25 selected oracle/bootstrap-numeric methods passed on
`8337aac` (32.84 seconds locally). Inputs and expectations already fixed by the
previous migrations are unchanged. New arithmetic expectations were calculated
with Python integers and `fractions.Fraction`, independently of the native VM,
and frozen as data. The seeds, exact counts, input formulae and artifact origins
are recorded in `fixtures/oracle_contracts/README.md` and `inventory.json`.
The current seed was also confirmed byte-identical to the stage-0 compiler
artifact before retirement. Wrong expected arithmetic output is rejected; decimal
normalization preserves integers beyond binary64 precision and rejects malformed
output. The native command wrapper rejects timeout, nonzero status and stderr.

Nothing in the native test path generates its expected result from the compiler
or VM being tested. Updating a golden requires independent review of the source,
expected semantics and bytecode, not merely accepting current program output.

## Retired comparisons and replacements

| Former oracle | Preserved evidence |
| --- | --- |
| Checker (31 sources), purity (10), generics (39), host types (27), local inference (59), rational/unit types (17) | Same 183 source inputs; fixed success and diagnostic assertions in the checker, purity and compiler-contract probes. Accepted inputs must return `ok`; negative expectations cannot pass merely because two compilers agree. |
| Emitter (10 cases) | Unchanged fixed `.disasm` listings in `compiler_contracts/emitter`, checked on the native VM. |
| Driver (3 module graphs) | Existing direct-driver/public-command byte equality plus new fixed stage-0 artifact bytes for basic, relative and logical imports. |
| Standard library byte identity | Fixed stage-0 artifact, native verification and execution; bootstrap now requires stage 1 = stage 2 = stage 3 for this program. Compiler stage 2 = stage 3 fixed point remains mandatory. |
| Codec (10 source variants, 5 special artifacts, 57 rejected artifacts) | Existing fixed byte arrays, canonical round trips, exact disassembly and diagnostic expectations on native/self-hosted paths. The extra disassembly-to-bootstrap comparison has its own frozen listing. |
| VM observations (61) | All stdout expectations were audited byte-for-byte against the native corpus. All 40 traps retain native exact status/stderr. Every one of the 21 successful artifacts additionally executes through a C assertion that main returns `V_VOID`, since the public CLI discards that value. Python-specific wording/None representation is archival metadata, not a native contract. |
| Native program outputs | `native_oracle_contracts.sh` compiles and runs every former functional `main.panack` and example against its existing fixed stdout, requiring successful verification/execution and empty stderr. Same selected VM under instrumentation. |
| Native compiler execution/loading | Verified seed runs check/compile/run; Euler 001 output and bytes are fixed. Compiler fixed-point and standard-library loading remain in canonical bootstrap validation. |
| Rational arithmetic | All 24 original seeded operand pairs × four operators: 96 exact reduced results, with original wide signed operands. |
| BigInt properties | All 460 boundary/seeded signed operand pairs: 1,800 add/subtract/multiply/truncating-divide-and-remainder observations, excluding zero divisors exactly as before. |
| Decimal properties | All 104 original boundary/seeded operand/exponent pairs × four operations plus six exact divisions: 422 independent expectations, including scales through ±4096. Decimal output is normalized with string operations only; no floating-point conversion or rounding. |
| Builtin registry | All 82 original names, arities and purity flags in fixed data. The same C probe also requires each handler to exist. |
| Host capabilities and host types | Existing exact conformance output runs in both functional source/bytecode paths and the native oracle corpus. |
| Two differing compiler diagnostics | Native probes retain their established wording. Bootstrap-only wording safeguards retired with their implementation; no live cross-implementation comparison is needed. |

`make unit` and `make check-vm` include `native-oracle-contracts` as well as
`native-vm-contracts`. Sanitizer and LLVM coverage builds invoke the same targets
with their selected instrumented VM and module binaries. The instrumentation
entry points first build the ordinary public CLI needed by nested CLI fixtures;
the instrumented binaries remain isolated. Standalone coverage after cleanup
exposed and now verifies this dependency. `make check-compiler`
also runs the focused `native-oracle-artifacts` group. No assertion moved out
of canonical `make check` to meet a timing budget. The general compiler/codec
probes retain their existing unit and focused-component target wiring.

The compiler-harness cache helper and its test were retired with their last
consumers. They tested the removed Python test infrastructure, not language
semantics. The unused rational source helper is also removed.

## Final implementation safeguards — retired

- `bytecode/test_verifier.py`: Python-only in-memory object shapes, mutable limits,
  function signatures and purity checks. Wire/native representable cases already
  have independent C/Panackelty evidence; these retired with the stage-0 tool.
- `bytecode/test_serialization.py` and `test_vectors.py`: stage-0 limits, source-build
  verifier invocation, Python ADT/minimal-Void self-checks and CLI hash-seed isolation.
- `vm/test_numeric.py`: stage-0 rejects `1.0 / 3.0` during compilation. The native
  frontend accepts it and the VM traps when executed; its runtime rejection is
  already in the VM corpus. This phase difference is not claimed as equivalence.
- `compiler/test_bootstrap_diagnostics.py`: stage-0 callable/import wording.
Seed corruption, fixture-runner failure injection, repository layout, packaging,
timing and build contracts have moved to `unit/harness`. The complete 52-method
audit is in [HARNESS_MIGRATION.md](HARNESS_MIGRATION.md).

These were implementation-only safeguards, not live differential dependencies.
They are now retired with the stage-0 implementation. Native replacements and
final repository checks are recorded in [PYTHON_REMOVAL.md](PYTHON_REMOVAL.md).
