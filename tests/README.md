# Tests

Panackelty has five validation paths:

- `unit` tests exercise implementation internals directly through the Python
  compiler, checker, bytecode, verifier, VM, and runtime interfaces. Small
  source snippets may be used to isolate internal behavior and failures.
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

Set `VALIDATION_TIMINGS_FILE` to append tab-separated phase, duration, budget,
and exit-status records. CI publishes those records in the workflow summary and
retains them as a per-run artifact so timing regressions remain visible.

The specification-to-test map and prioritized coverage backlog live in
[`COVERAGE.md`](COVERAGE.md). Update it when a language promise or its automated
evidence changes.

Unit tests are grouped by implementation subsystem under `tests/unit/compiler`,
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
Independent success and failure cases run concurrently, using up to four
workers by default. Set `PANACK_TEST_JOBS` to a positive integer to tune that
concurrency.

Self-hosted parser tests compile one command-argument-driven driver per parser
entry point and reuse its verified code across cases. Each assertion still
executes the real Panackelty-hosted parser while avoiding repeated compilation
of an unchanged compiler module graph.

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
