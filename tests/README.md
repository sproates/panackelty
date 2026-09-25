# Tests

[Python-free test architecture and migration inventory](PYTHON_MIGRATION.md)
maps each existing test area to its replacement and records the parity gates.
All twenty former Python functional methods now have Panackelty-hosted
replacement evidence. The runner (`runner/main.panack`) checks twenty-five
selected success cases, twenty examples, and forty-one expected failures.
`make functional` also checks the self-hosted compiler driver and exercises
`runner_smoke` from source and saved bytecode. The Python unit tests remain.

Panackelty has five validation paths:

- `unit` tests exercise implementation internals directly. Panackelty probes
  cover the lexer, parser, resolver, type checker, and purity; the remaining compiler, bytecode,
  verifier, VM, and runtime tests use the Python harness during migration.
  Small source snippets isolate internal behavior and failures.
- `functional` tests treat the `panack` command as a black box. They compile or
  run complete `.panack` programs on the VM, capture their output, and compare it
  with the expected observable behavior.
- `native-check` uses only the native VM, compiler seed, and POSIX tools. It
  proves the stage-2/stage-3 fixed point and runs the success, failure, and
  malformed-artifact conformance corpus without Python.
- `release-smoke` extracts and relocates the final download archive, then uses
  only that toolchain from a fresh working directory and a runtime-only `PATH`.
- `quick-start` verifies the packaged README's download checksum, installation,
  first program and exact transcript, upgrade, and removal instructions.

Run a level independently with `make unit`, `make functional`,
`make native-check`, `make release-smoke`, or `make quick-start`. Run the full
development validation with `make check`.
Each command prints its wall-clock duration and budget; an over-budget run also
prints a warning without hiding the underlying test result.
The unit phase times `unit-impl`, including both the Python tests and the
Panackelty probes, and propagates a failure from either suite.

For incremental work, use `make check-compiler`, `make check-bytecode`, or
`make check-vm`. Each runs the owning unit-test subtree plus representative
public-CLI workflows, and each has a 15-second budget when the native toolchain
is already built. `make unit` remains a quick way to run every internal test.

The complete workflow builds the stage-2 self-hosted compiler once. Functional
compiler-driver checks and the compiler program's compiled-output case reuse
that verified artifact, and the later bootstrap phase extends it to stage 3 for
the byte-identical fixed-point proof. Other programs are still compiled through
the public CLI before their bytecode output is checked. The proof runs only in
the bootstrap phase, not again as a unit test.
The compiler fixture allows 90 seconds for source execution and compilation,
which can build the complete compiler. Ordinary fixture commands retain 20
seconds; phase timing warnings remain independent of command timeouts.
`make functional` captures one successful full runner report and passes its
temporary file to `runner_smoke` in both source and bytecode mode. Each mode
compares the exact report; a missing or changed report fails. Standalone runs
of `runner_smoke` still invoke the complete runner themselves.

CI runs `make check` once per pull-request revision and again after a merge to
`main`. It does not repeat the focused developer targets before the full suite.
Superseded PR runs are cancelled. Both platform package jobs still run bootstrap,
native conformance, and exact-archive gates. The default native build uses `-O2`;
functional and native process tests exercise the same optimised VM. Native tests
use the production build graph rather than maintaining separate source lists.
`make native-sanitize` additionally runs native module, loader, and execution
coverage against an isolated instrumented build.

Set `VALIDATION_TIMINGS_FILE` to append tab-separated phase, duration, budget,
and exit-status records. CI publishes those records in the workflow summary and
retains them as a per-run artifact so timing regressions remain visible.

The specification-to-test map and prioritized coverage backlog live in
[`COVERAGE.md`](COVERAGE.md). Update it when a language promise or its automated
evidence changes.

`stdlib/testing` supplies pure structured assertions and an explicit reporter
for new Panackelty-hosted tests. Its initial end-to-end case is
`functional/cases/testing_library`; the lexer, parser, resolver, type-checker, and purity unit probes now also
use it directly. The remaining Python unit harness retains its own discovery.
`stdlib/testing_files` exposes sorted immediate fixture directories and
temporary workspaces; `functional/cases/testing_fixtures` exercises their
creation, enumeration, and explicit cleanup.
`stdlib/testing_commands` supplies byte-exact process and host-error
assertions; `functional/cases/testing_commands` verifies those via the public
CLI. Make orchestrates the Python and Panackelty checks during migration.

Remaining Python unit tests are grouped by subsystem under `tests/unit/compiler`,
`tests/unit/bytecode`, and `tests/unit/vm`. Shared compilation and VM-output
helpers live in `tests/unit/support.py`. Add a focused module to the owning
subsystem instead of growing a single catch-all test file; `make unit`
discovers the package tree recursively.

`tests/unit/test_native_distribution.py` covers both conventional staged
installation and the download archive. Its archive test builds the packaging
layout without Python, validates the complete file set, relocates the extracted
directory, and runs a standard-library program through `bin/panack`. Its
checksum test independently verifies the digest emitted by `package-checksum`.
`tests/release_archive_smoke.sh` is the separate release gate: it starts from
the `.tar.gz`, creates all program inputs outside the checkout, and covers help,
version, checking, source and bytecode execution, arguments, bundled standard
library imports, and malformed-bytecode rejection without development tools.
`tests/quick_start.sh` extracts its program and expected transcript from the
README inside the final archive, installs that archive under an isolated home,
runs the documented commands with development tools absent from `PATH`, then
exercises the replacement-style upgrade and complete removal procedures.

`tests/unit/test_layout.py` locks the packaging workflow to the two supported
runner/architecture pairs, the Python-free package command, checksum and
provenance uploads, and the absence of tag or release triggers. It separately
requires the tag workflow to match `VERSION`, depend on complete validation and
both matrix packages, recheck downloaded assets, and confine write permission
to the final prerelease publication job. It also fixes the project website's
complete static file set and ensures its Pages workflow validates pull requests
but grants deployment permissions only after a change reaches `main`.

Structurally valid artifacts that forge dynamically unsafe states are defined
once in `tests/unit/forged_runtime.py`. The bytecode contract tests execute that
corpus on the Python oracle, and the native VM tests serialize and execute the
same instructions, so both implementations must trap on the same conditions.

Shared file-I/O source builders live in `tests/unit/file_io_cases.py`. Oracle,
native, and public-CLI tests use them for identical text/binary round trips and
missing, denied, invalid-content, and unrepresentable-path failures.

Each test-only functional case has its own directory under
`tests/functional/cases` containing `main.panack` and `expected.stdout`. Supporting
modules live beside `main.panack`. A case may use `source.path` instead of `main.panack`
to test a program elsewhere in the repository. The harness discovers these
directories automatically, so adding a case does not require Python changes.
The Panackelty functional runner currently executes cases sequentially.

`runner/compiler_parser_unit.panack` imports the parser directly, compiles once,
and checks 192 fixed expectations for expressions, blocks, types, and programs.
Its 38 groups preserve the former Python method names, including all expanded
subtests and exact malformed-input diagnostics. `runner/compiler_lexer_unit.panack`
similarly covers the eight direct lexer contracts.
`runner/compiler_resolver_unit.panack` covers 26 exact source and module-graph
expectations in 12 groups, including diagnostic paths and positions. All run in `make unit` and
`make check-compiler`; each reports failures and exits nonzero on a mismatch.
The checker probe, `runner/compiler_checker_unit.panack`, also runs in both
targets. It checks 31 source fixtures and three module graphs, requiring `ok`
for success and preserving diagnostic substrings for failures. The Python
checker test retains only differential acceptance comparisons on the same
31 source fixtures. Read `fixtures/compiler_checker/README.md` when adding
a checker case. `runner/compiler_purity_unit.panack` follows the same pattern
for ten shared source fixtures and one cross-module contract, retaining all ten
Python differential comparisons. Its maintenance contract is in
`fixtures/compiler_purity/README.md`. Both probes require `ok` for success
and preserve the original diagnostic substrings on failure. Their old-to-new
assertion mapping is in `PYTHON_MIGRATION.md`.

Expected CLI failures live under `tests/functional/failures`. Each failure has
its own directory containing `main.panack` and `expected.stderr`; the harness runs
it through both `panack check` and `panack compile`, requires exit status 1 with
exact diagnostics and no stdout, and verifies that compilation leaves no
bytecode artifact. Case-directory paths in diagnostics are normalized to
`<case>` so import failures remain portable across checkout locations.

User-facing programs under `examples` are also discovered automatically. Their
expected output lives under `tests/functional/expected/examples` with the same
stem and a `.stdout` extension, keeping every documented example executable.
The release archive includes the complete directory so links in its language
tour resolve to the same programs validated by this harness.

The retained checker and purity oracles and self-hosted emitter, driver, and bytecode tests
use `CompilerHarnessTestCase` to compile each parameterised probe once per class.
Test inputs travel through command arguments or temporary files instead of being
embedded into a newly compiled probe each time. Every invocation constructs a
fresh VM; output and environment are isolated even after a failed invocation.
Harnesses are rebuilt in each test process, with no persistent artifact cache.
The `string_boundaries` functional case covers ASCII and mixed-width Unicode,
combining characters, empty strings, NULs, and derived string values through
source execution, saved bytecode, and native conformance. Native unit tests
retain out-of-range indexing and invalid slicing failures.

Native hardening also includes `make native-fault` (test-only allocation and
syscall injection) and `make native-coverage` (Clang/LLVM line and branch reports
under `build/coverage`). Both fault sweeps and arithmetic/mutation properties are
included in ordinary VM tests. CI additionally runs the sanitizer corpus and
uploads the HTML coverage report. CI explicitly installs matching Clang and LLVM
packages; local tool overrides are `LLVM_CC`, `LLVM_COV` and `LLVM_PROFDATA`.
No fault-injection controls enter production.
