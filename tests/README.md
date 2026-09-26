# Tests

[Python-free test architecture and migration inventory](PYTHON_MIGRATION.md)
maps each existing test area to its replacement and records the parity gates.
All twenty former Python functional methods now have Panackelty-hosted
replacement evidence. The runner (`runner/main.panack`) checks twenty-five
selected success cases, twenty examples, and forty-one expected failures.
`make functional` also checks the self-hosted compiler driver and exercises
`runner_smoke` from source and saved bytecode. The transitional implementation and its implementation-only tests are retired.

Panackelty has six core validation paths, plus the repository policy and
isolated-environment proof:

- `unit` tests exercise implementation internals directly. Panackelty probes
  cover compiler, bytecode, VM, host and standard-library behavior.
  Small source snippets isolate internal behavior and failures.
- `harness` checks repository/CI policy, distribution, timing, seed rejection
  and fixture-runner failures using shell and native Panackelty subprocesses.
  Run `make harness`; see [HARNESS_MIGRATION.md](HARNESS_MIGRATION.md).
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
The unit phase times `unit-impl`, including the native harness and Panackelty probes, and propagates any failure.

For incremental work, use `make check-compiler`, `make check-bytecode`, or
`make check-vm`. Each runs the owning unit-test subtree plus representative
public-CLI workflows, and each has a 15-second budget when the native toolchain
is already built. `make unit` remains a quick way to run every internal test.

The complete workflow builds the stage-2 self-hosted compiler once. Functional
compiler-driver checks and the compiler program's compiled-output case reuse
that verified artifact, and the later bootstrap phase extends it to stage 3 for
the byte-identical fixed-point proof. Other programs are still compiled through
the public CLI before their bytecode output is checked. The ordinary proof runs in
the bootstrap phase. That phase also exercises a fresh seed refresh on a
temporary copy with a Python-free `PATH`, independently of cached stages.
`seed_refresh.sh` uses a fake VM for fast unit/focused-compiler failure injection:
bad digests, hash failures, verifier/compiler failures, stage mismatches, runtime
and expected-output failures, signals, locks, symlinks, concurrent edits,
successful publication and an unchanged fixed point. See `bootstrap/README.md`
for the refresh contract.
The compiler fixture allows 90 seconds for source execution and compilation,
which can build the complete compiler. Ordinary fixture commands retain 20
seconds; phase timing warnings remain independent of command timeouts.
`make functional` captures one successful full runner report and passes its
temporary file to `runner_smoke` in both source and bytecode mode. Each mode
compares the exact report; a missing or changed report fails. Standalone runs
of `runner_smoke` still invoke the complete runner themselves.

CI runs one main test job per pull-request revision and again after a merge to
`main`. It does not repeat focused developer targets before the full suite.
Superseded PR runs are cancelled. Both platform package jobs additionally run
`make check-no-interpreter`, proving the full workflow from a clean build with
an allowlisted command environment. This includes bootstrap, native conformance
and exact-archive gates. The default native build uses `-O2`;
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
use it directly.
`stdlib/testing_files` exposes sorted immediate fixture directories and
temporary workspaces; `functional/cases/testing_fixtures` exercises their
creation, enumeration, and explicit cleanup.
`stdlib/testing_commands` supplies byte-exact process and host-error
assertions; `functional/cases/testing_commands` verifies those via the public
CLI. Make orchestrates shell, C and Panackelty checks.


Direct bytecode/verification coverage runs in
`tests/runner/bytecode_unit.panack`, `tests/runner/bytecode_native_unit.panack`
and the native C verifier contracts in `tests/unit/vm/native_modules.c`.
These share fixed version-8 and malformed artifact vectors and compare exact
canonical artifacts and disassemblies. Live Python differential comparisons are
retired together with bootstrap-only object/limit safeguards, recorded in
`tests/fixtures/bytecode/contract_cases/README.md`.

The former implementation-only unit packages and their support helper are
retired. Add new behavioral tests to the owning native/Panackelty probe;
`make unit` includes all these checks.

`tests/unit/harness/distribution.sh` covers both conventional staged
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

`tests/unit/harness/layout.sh` locks the packaging workflow to the two supported
runner/architecture pairs, the Python-free package command, checksum and
provenance uploads, and the absence of tag or release triggers. It separately
requires the tag workflow to match `VERSION`, depend on complete validation and
both matrix packages, recheck downloaded assets, and confine write permission
to the final prerelease publication job. It also fixes the project website's
complete static file set and ensures its Pages workflow validates pull requests
but grants deployment permissions only after a change reaches `main`.

Structurally valid artifacts that forge dynamically unsafe states live in
`tests/fixtures/vm_contracts`. The native Panackelty probe executes the reviewed bytes and requires fixed
status/streams; C probes also check all 21 successful return kinds.

Portable file-I/O source programs and dynamic host assertions now live in
`tests/functional/cases/cli_environment_files` and `tests/runner/host_runtime_unit.panack`.
They check identical text/binary round trips, disk bytes, missing and denied
paths, invalid UTF-8 content and embedded-NUL rejection through the native CLI.

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
for success and preserving diagnostic substrings for failures. Live Python
comparisons are retired; these fixed expectations remain unchanged. Read `fixtures/compiler_checker/README.md` when adding
a checker case. `runner/compiler_purity_unit.panack` follows the same pattern
for ten fixed source fixtures and one cross-module contract. Its maintenance contract is in
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

The live Python compiler/VM oracle and its unused harness are retired.
`make native-oracle-contracts` checks fixed independent arithmetic, signatures,
artifacts and the full program corpus. Unit, focused VM, sanitizer and coverage
gates include it. See `ORACLE_REPLACEMENT.md` for the complete audit and retained
bootstrap-specific checks.
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

The remaining direct compiler contracts now run in
`tests/runner/compiler_contracts_unit.panack` (201 assertions) and
`tests/runner/compiler_integration_unit.panack` (51 assertions), under both
`make unit` and `make check-compiler`. They cover emitter instructions, diagnostic
rendering and source snapshots, loader/imports and driver commands, generics,
inference, types and host boundaries. Fixed expectations now replace the Python
differential oracle; the case mapping is in `tests/ORACLE_REPLACEMENT.md`.
The transitional implementation and its 21 implementation-only safeguards are retired;
seed regeneration now uses verified self-hosted stages.


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
The [migration inventory](fixtures/host_runtime/README.md) maps all 37
former methods: 31 migrated to direct native evidence and the final six replaced
by fixed oracle fixtures and native/bootstrap cross-checks. Functional source and bytecode cases still verify
public behaviour on both supported platforms.

`make policy` checks the source tree for forbidden interpreter dependencies and
runs adversarial policy controls. It is part of `make check`. The final retirement
audit and isolated-environment proof are in [PYTHON_REMOVAL.md](PYTHON_REMOVAL.md).

Manual release request controls run in `unit/harness/release.sh`: confirmed main
commit and version, canonical tag requests, rejected events/branches/inputs, and
missing, empty or duplicate release notes. The workflow keeps publication behind
full validation and both platform packages, with exact source provenance.

Release publication controls use disposable local Git repositories and a stubbed
GitHub CLI boundary to verify real annotated-tag creation, same-tag retries, and
rejection of lightweight or wrong-commit tags before publication. Development
validation therefore requires Git; downloaded toolchains are unaffected.

## Detailed validation profiling

Set `VALIDATION_PROFILE_FILE` to an **absolute** path outside `build/` to
append opt-in, headerless TSV observations. Columns are run context, label,
parent label (`-` at the root), elapsed wall-clock seconds, and exit status.
`VALIDATION_PROFILE_RUN` names the experiment. Labels contain no tabs/newlines.
The wrapper preserves command arguments, stdout/stderr and exit status; failure
to append a report warns without changing the command result. With profiling
unset it directly executes the command. Budget warnings and their existing
`VALIDATION_TIMINGS_FILE` format remain unchanged.

```sh
profile_dir=$(mktemp -d)
export VALIDATION_PROFILE_FILE="$profile_dir/profile.tsv"
export VALIDATION_TIMINGS_FILE="$profile_dir/budgets.tsv"
make clean
VALIDATION_PROFILE_RUN=clean sh tests/profile_command.sh clean/check make check
for component in compiler bytecode vm; do
  VALIDATION_PROFILE_RUN="warm-$component" \
    sh tests/profile_command.sh "warm/check-$component" make "check-$component"
done
```

Record the source commit, host/runner image, compiler version, build flags and
whether native prerequisites already existed with each experiment. Do not run
other builds concurrently. Repeat measurements only when noise or a regression
needs investigation. A clean check includes builds; the focused baseline starts
with ordinary native prerequisites built. Parent rows include their children:
**do not sum nested rows**. One-second resolution is intended to find large
costs, not benchmark tiny operations. Probe rows include their source compilation
and execution; harness groups include their subprocesses. Fixture rebuilds can
appear more than once under different parents. Observations include profiling
overhead, and are not CPU measurements or end-to-end GitHub workflow duration.

Both Check packaging jobs archive the clean-check and subsequent package profile,
including failure rows, separately from budget records. Expected negative-control
commands may have nonzero rows inside a successful harness group. `Validation profile`
collects warm compiler/bytecode/VM measurements on both supported platforms when
the profiler changes or by manual dispatch. It does not run on ordinary code or
documentation PRs. Existing required checks, sanitizers and release gates remain
in place. Findings and next investigations live in
[`VALIDATION_PROFILE.md`](VALIDATION_PROFILE.md).


## Change-aware CI

`make docs` checks informational documentation locally without building native
tools. `make ci-check` exercises change classification, routing failures and
link-checking fixtures. The latter is also part of `make check` through the
policy stage and runs before CI selects a route.

| Changes | Check workflow |
| --- | --- |
| Only `ROADMAP.md`, `ARCHITECTURE.md`, `SELF_HOSTING.md`, `tests/README.md`, `tests/COVERAGE.md`, `tests/VALIDATION_PROFILE.md` as regular non-executable files | Document checks and local file links; no builds, packaging, sanitizer or coverage work |
| README, specification, packaged inputs, instructions, workflows, code, other paths or mixed changes | Full existing validation and both platform packages |
| Missing revisions/history or empty/unknown diff | Full validation |
| Classification or document checking fails/cancels | Existing named checks fail; no false successful skip |

The classifier compares the complete PR diff from its merge base, not only the
last commit; main pushes compare the prior commit to the pushed head. It inspects
both sides of additions, deletions, renames and mode changes. Symlinks and
executable Markdown never select the fast path. `scripts/ci_docs.sh` is the
single allowlist; expanding it requires evidence that the document is neither
an executable fixture nor a packaged/behavioral input.

The fast path checks changed whitespace, empty/NUL-containing documents,
conflict markers, and local inline/image/reference link destinations outside
code fences. It also checks incoming links to allowlisted files, catching broken
references after deletion/rename. Remote URLs and heading fragments are not
validated; this is deliberately not a general Markdown renderer or network
crawler. Content still needs human review.

The existing `test`, `Package (linux-x86_64)` and `Package (macos-arm64)` results
remain stable. On docs changes the package-named jobs run only short guards on
Ubuntu and explicitly report that full validation is not applicable; they do
not claim a platform build took place. They verify that classification and docs
checks succeeded. On full changes they retain their original platforms and all
gates. No branch-protection settings need changing. Workflow-wide path filters
are avoided so required check results are never left pending due to filtering.
Release validation and the separate profiling/Pages workflows retain their
existing triggers and gates.
