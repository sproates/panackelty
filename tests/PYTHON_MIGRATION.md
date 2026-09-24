# Python-free development test architecture

This is the migration contract for removing Python from the *repository*.
The downloadable compiler, native VM, package, and release smoke path already
run without it. Until equivalent evidence is present, the Python oracle and
test harness remain required by `make check`; no test is removed merely because
a new runner exists.

## Baseline and ownership (September 2026)

The current suite has 294 discovered unit test methods (160 compiler, 43
bytecode, 72 VM, 19 other) and 20 functional test methods. The functional
runner additionally discovers 20 case directories, 20 example output files,
and 41 failure directories. The bytecode fixture directory has 26 `.hex`
artifacts spanning supported and deliberately rejected historical versions.
Methods are not a coverage metric: parameterized subtests, generated cases,
and the discovered programs exercise substantially more observations.

| Current owner | Required replacement | Evidence not to lose |
| --- | --- | --- |
| `tests/functional/test_programs.py` | Panackelty-hosted fixture runner using `stdlib/testing`, `testing_files`, and `testing_commands` | Exact stdout/stderr and status for source and saved bytecode; `check`, `compile`, `run`, `disasm`, shorthand, arguments, imports, examples, invalid source, malformed artifacts, environment and file I/O. Preserve `source.path` validation, failure diagnostic normalization, isolated outputs, and the stage-2 compiler reuse. |
| `tests/unit/compiler/` | Focused Panackelty compiler probes and public-CLI contract fixtures | Lexer, parser, resolver, checker, purity, emitter, loader, driver, diagnostics, generics, type and host boundary failures; deterministic artifacts and bootstrap-stage parity. Do not replace precise negative assertions with only success programs. |
| `tests/unit/bytecode/` and `tests/fixtures/bytecode/` | Portable golden bytecode and malformed vectors, native verifier/decoder tests, and Panackelty-hosted codec assertions | Versioned encoding, canonical round trips, verifier rejection, resource bounds, forged unsafe states, exact error categories, and repeated-compilation identity. |
| `tests/unit/vm/` and `tests/unit/vm/*.c` | Direct C module/fault/sanitizer tests plus black-box Panackelty programs on the native VM | Stack/frame/value ownership, exact numeric semantics, collection and host operations, allocation/syscall injection and cleanup, runtime traps, and native coverage. Keep the C tests; replace their Python launch/assert wrappers. |
| `tests/unit/{support,file_io_cases,forged_runtime,rational_cases}.py` | Portable declarative inputs and expected results, shared by the relevant native and Panackelty tests | Generated source and forged bytecode cases must be inventoried individually before replacing them; preserve boundary values, failure categories, and binary data rather than counting files. |
| `tests/unit/test_{layout,native_distribution,validation}.py` | POSIX packaging/CI contract checks and Panackelty-hosted assertions where appropriate | Exact archive contents and checksums, relocation and paths with spaces, CI safety triggers, clean build flags, timing warnings, and quick-start output. Keep exact-artifact shell gates independent of the source tree. |
| `panackelty.py`, `src/bootstrap/panackelty.py`, `make regenerate-seed` | Checked native/self-hosted cross-checks, portable golden artifacts, fixed-point bootstrap, and a documented staged seed refresh | Python compiler/VM differential accept/reject and trap evidence, seed integrity, stage-2/stage-3 byte identity, and a reviewable seed digest. |

`tests/COVERAGE.md` remains the specification-to-behavior map. For each migrated
area, record the old assertion or generated case and its new evidence there
before deleting the old test. Preserve intentionally uncovered cases as named
backlog items, not implicit losses.

## Runner and fixture contracts

The first parallel runner is `tests/runner/main.panack`. `make functional` runs
it after the unchanged Python harness. It discovers and selects `callables`,
`collections`, `compiler_lexer`, `hello_world`, `local_inference`, `modules`, `optional_else`,
`rational_unit`, `records_and_enums`, `semicolonless`, `string_boundaries`,
`testing_library`, and `vm_numeric_boundaries`, checking exact stdout, empty stderr,
and zero status for source execution, compilation, and saved-bytecode execution.
It reports failures through its process status and removes each artifact before
requiring an empty workspace. `functional/cases/runner_smoke` checks its exact
success report through the public CLI; `unit/compiler/test_panackelty_runner.py`
injects wrong expected output and checks both source and bytecode failures. Its
`--test-cleanup-failure <parent>` mode leaves a sentinel in a uniquely created
workspace, verifies that empty-directory removal fails, reports a nonzero exit,
then removes the sentinel and workspace. The focused test requires the supplied
parent directory to be empty afterward.
No Python assertion has been retired. Other cases, source.path, invalid inputs,
and environment isolation remain owned by the old harness.

The future Panackelty runner discovers immediate case directories in sorted
native-byte order. A case keeps the existing `main.panack` *or* `source.path`
and its expected output; examples and failures retain their current fixture
layout. It checks source and compiled-bytecode execution separately, compares
raw byte streams and exit status, checks empty stderr on success, and reports
case names in a deterministic order. Invalid source must fail the intended
public command without leaving a compiled artifact. The runner's own failure
count sets a nonzero process exit status; `make check` must propagate it.

Use unique temporary workspaces and bounded subprocesses. Track every created
file and directory, remove contents explicitly, then require successful empty
workspace removal. A failed cleanup fails the test rather than silently
discarding evidence. Reuse a verified stage-2 compiler artifact when the
existing harness does, but never bypass the fixed-point bootstrap or the
representative source and bytecode public-CLI paths. Keep `make unit` and
`make functional` separate under `make check`; retain timings and budgets.

Some current harness guarantees need design or host-capability work before a
specific case can migrate:

- `source.path` currently resolves symlinks and rejects targets outside the
  repository. Typed `Path` operations are lexical, not a containment check.
  Preserve equivalent resolution/validation or keep those cases on the old
  runner until a safe mechanism is available.
- The harness removes `PANACKELTY_STDLIB_VALUE` from inherited child environments.
  `process_run` can override but not delete variables. Provide equivalent
  isolation before replacing those cases; do not silently accept ambient state.
- The Python harness parallelizes independent programs (up to four workers).
  Preserve the functional and full-check time budgets without skipping cases;
  measure a sequential candidate and add safe parallel orchestration if needed.
- Failure fixtures normalize checkout-dependent diagnostic paths. Preserve
  exact expected diagnostics and the path-with-spaces tests without relying on
  an accidental checkout location.
- Some tests construct malformed bytecode and source probes dynamically. Move
  those to shared declarative fixtures or independently validated generators
  before retiring their Python builders.

## Migration order and removal gates

1. Establish a Panackelty runner on a representative fixed fixture subset.
   Run old and new paths together; compare the per-case observation matrix,
   including deliberate failures and cleanup errors. The selected subset and
   output-mismatch and injected nonempty-workspace failures are now covered;
   broader host-error injection still belongs to the full-corpus migration.
2. Port the full functional corpus and orchestration, resolving the contracts
   above. Retire a Python assertion only with equivalent new evidence in
   `tests/COVERAGE.md` and a green `make check` on both supported platforms.
3. Port unit coverage by compiler, bytecode, VM, and host subsystem. Keep
   targeted native C tests for internals and portable vectors for cross-runtime
   contracts. Maintain focused component checks and the full suite throughout.
4. Replace the differential Python oracle and seed-regeneration command with
   golden, native/self-hosted, and fixed-point evidence. Document a verified
   staged seed refresh; audit every former oracle-only failure case.
5. Remove the facade, bootstrap Python, harness, Make/CI Python setup and
   commands only after no consumers remain. Add a policy check against Python
   source, shebangs, and invocations. From a clean environment without Python,
   pass `make check`, native conformance, bootstrap, packaging, and release
   smoke on Linux x86-64 and macOS arm64.

Each migration PR should list the old tests/cases retired, the replacement
fixtures and assertions, the before/after behavior matrix, platform checks,
and any timing change. Keeping both implementations temporarily is a deliberate
migration aid, not a permanent second execution engine.
