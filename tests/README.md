# Tests

The [website repository](https://github.com/sproates/panackelty-website) owns assembly, previews, published-release
acceptance and deployed-byte checks. [Panackelty Browser](https://github.com/sproates/panackelty-browser)
owns browser/runtime integration tests. Core retains native and runtime-bundle
contracts; `make check` requires neither Node, Playwright nor WASI.

The suites combine Panackelty probes, native C tests, shell harness checks,
and public CLI tests. The runner (`runner/main.panack`) checks twenty-five
selected success cases, twenty-three examples, and forty-one expected failures.
`make functional` also checks the self-hosted compiler driver and exercises
`runner_smoke` from source and saved bytecode.

`make semantic-impact-experiment` runs the bounded compiler-evidence prediction
experiment with Node 24+. It is a separate Linux/macOS compiler CI step; native
`make check` remains interpreter-free. See the
[experiment report](experiments/semantic_impact/README.md) for scope and reproduction.

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
  malformed-artifact conformance corpus.
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
is already built. `make unit` runs every internal test.

`tests/run_probe.sh` caches compiled probe bytecode under `build/probes` (or
`BUILD_DIR/probes`). Its key covers the selected VM, compiler seed, helper,
source paths and contents, and all Panackelty sources under `src`, `tests`,
`examples` and the selected standard library. Added/deleted files and edits with
restored timestamps invalidate reuse. Cached bytecode has a checked digest;
failed compilation and inputs changed during compilation are never published.
Every invocation executes the probe and its assertions. Instrumented VMs have
separate keys and build directories. `make clean` removes all cached bytecode.
The compiler integration driver and snapshot helper use the same cache; fixture
programs still exercise the public CLI. The shell harness compiles its runner
once per group and reuses it across isolated failure-injection scenarios.
Compiler probes schedule integration and contract checks first to avoid a long
final worker tail. Make-driven validation uses four workers by default; set
`VALIDATION_JOBS=1` for serial execution or choose a limit from 1 to 32. Setup
and fixture mutation remain serial. The worker pool buffers stdout/stderr and prints
results in the requested order, waits for every probe, and propagates failures.

The complete workflow builds the stage-2 self-hosted compiler once, before the
native oracle corpus. Oracle and functional compiler-driver checks and the
compiler program's compiled-output case reuse
that verified artifact, and the later bootstrap phase extends it to stage 3 for
the byte-identical fixed-point proof. Other programs are still compiled through
the public CLI before their bytecode output is checked. The ordinary proof runs in
the bootstrap phase. That phase also exercises a fresh seed refresh on a
temporary copy with an allowlisted `PATH`, independently of cached stages.
`seed_refresh.sh` uses a fake VM for fast unit/focused-compiler failure injection:
bad digests, hash failures, verifier/compiler failures, stage mismatches, runtime
and expected-output failures, signals, locks, symlinks, concurrent edits,
successful publication and an unchanged fixed point. See `bootstrap/README.md`
for the refresh contract.
The compiler fixture allows 90 seconds for source execution and compilation,
which can build the complete compiler. Ordinary fixture commands retain 20
seconds; phase timing warnings remain independent of command timeouts.
`make check` creates a fresh temporary report session. The oracle corpus runs
`runner_smoke` on its selected VM; that smoke program executes the full fixture
runner through the public CLI and captures its actual report only after checking
exit status, signal, stdout and stderr. The later functional phase reuses this
report and compares its exact bytes through both source and bytecode smoke
execution. A missing or changed report fails. The session is removed on success
or failure, and ambient report overrides are cleared at entry. Test results
are never reused between check invocations. The nested fresh-run command has a
180-second bound for cross-platform variance; it still requires the complete,
byte-exact 364-case report. Standalone `make functional` and
`runner_smoke` still execute the full runner themselves; standalone oracle,
sanitizer and coverage targets retain their own complete corpus execution.

CI runs a validation matrix per pull-request revision and again after a merge
to `main`. It does not repeat focused developer targets before the full suite.
Superseded PR runs are cancelled. Both packaging platforms run five clean
`make check-no-interpreter CI_SUITE=…` partitions with an allowlisted command
environment. The default command without `CI_SUITE` still performs the complete
standalone proof. CI uses these shared targets:

| Suite | Shared coverage |
| --- | --- |
| `compiler` | Policy, full harness, report-capture and seed-failure controls, all compiler probes |
| `runtime` | Native VM/oracle contracts, host/bytecode probes, complete functional phase |
| `bootstrap` | Fixed-point bootstrap, independent seed refresh, archive smoke and quick start |
| `conformance-source` | Every native source conformance program |
| `conformance-bytecode` | Every native compile/bytecode conformance program, negative cases and CLI checks, archive smoke and quick start; uploads archive |

After each platform's `conformance-bytecode` partition, CI separately runs
`sh tests/release_install_readme.sh` with network access. It downloads the pinned
public release by executing the current README's platform block, then checks the
README program/transcript and optional home-directory installation in a temporary
workspace. This tests released user instructions on Ubuntu x86_64 and macOS arm64;
it is separate from the offline `make check` and freshly built archive gates.

Ubuntu validation runs the first three suites, three sanitizer partitions
(`sanitize-vm`, `sanitize-oracle`, `sanitize-runner`), and independent coverage.
These jobs use Ubuntu 24.04. Coverage uses its preinstalled, explicitly versioned
Clang, llvm-cov and llvm-profdata 18 tools, verifies their availability, and fails
if any is missing. It does not install packages or restore cached coverage.
The sanitizer partitions execute VM contracts, oracle fixtures/programs excluding
the nested functional runner, and that runner respectively. `SANITIZE_SUITE`
selects a partition; its default `all` preserves the complete standalone proof.
The coverage target calls the same complete sequence. Partition regression
checks compare operation multiplicity and inject compilation, verification,
execution, stderr and output failures. Each packaging platform runs all five
ordinary suites. The canonical `make unit` calls the same `unit-harness`, `unit-runtime` and
`unit-compiler` targets, and `make check` remains the complete local command.
Distribution builds own a copied VM in their temporary checkout and assert that
the shared VM inode and bytes remain unchanged, so concurrent compiler commands
cannot observe a relink.

Canonical unit validation overlaps harness and compiler suites after native setup,
then runs the runtime suite only if both succeed. The default four-worker budget
assigns one worker to the harness and three to compiler probes; one worker retains
serial execution and stops at the first failure. CI compiler validation sets three
workers explicitly: one for the harness and two for compiler probes. The macOS
matrix retains five jobs to avoid a sixth job waiting for a runner.
Native conformance also uses bounded workers (four by default). Each program
keeps its source/compile/bytecode assertions together in an isolated temporary
directory; NUL-delimited arguments preserve paths containing spaces. Negative
fixtures and CLI/archive checks still run after every program succeeds.
Bootstrap overlaps the ordinary fixed-point check with the isolated native
seed-refresh proof. They read the same immutable sources and seed but build
separate stages; no generated compiler stage or proof result is shared between them.
Both must succeed before quick-start or conformance gates proceed.
Runtime validation overlaps the native corpus with host/bytecode probes using
the configured worker limit after building shared native prerequisites. Only
the native corpus captures the functional runner report; probe artifacts are
separate. A one-worker setting retains serial execution. Failure controls use a
FIFO rendezvous to prove both branches execute concurrently and propagate either failure.
The stable required checks aggregate all applicable jobs, including failures
and cancellation. A package artifact is usable only with successful package
gates for its revision; upload alone does not certify the entire matrix.

Native compilation stays local to each job, avoiding an artifact-transfer
dependency. Bootstrap and instrumented builds remain isolated. No persistent
cache is needed for the CI duration target. The default native build uses `-O2`;
functional and native process tests exercise the same optimised VM. Native tests
use the production build graph rather than maintaining separate source lists.
`make native-sanitize` additionally runs native module, loader, and execution
coverage against an isolated instrumented build.

Set `VALIDATION_TIMINGS_FILE` to append tab-separated phase, duration, budget,
and exit-status records. CI publishes those records in the workflow summary and
retains them as a per-run artifact so timing regressions remain visible.

### Public coverage publication

[Public native VM coverage](https://sproates.github.io/panackelty-coverage/)
is hosted independently by
[`sproates/panackelty-coverage`](https://github.com/sproates/panackelty-coverage).
Core Check still generates and uploads `native-coverage-<run-id>` artifacts with
90-day retention. The separate publisher uses its built-in GitHub token to read
public artifacts; it needs no new credential. It checks about every 15 minutes
and supports manual dispatch. GitHub may delay schedules and disables scheduled
workflows in inactive public repositories after 60 days; maintainers should check
that repository's Actions status if the public report stops advancing.

Only completed successful trusted main-push Check reports are eligible. The
publisher validates source ancestry, artifact identity and report navigation,
serializes deployments, rechecks selection before publishing and prevents rollback
behind the live report. Its landing page and `provenance.json` identify the core
source, Check run, artifact and archive date. It verifies every deployed report
file. Missing/expired reports, API errors and failed validation leave the last
published report visible; investigate that repository's workflow and rerun a full
core Check if a fresh artifact is needed. Publishing does not execute core code.

Website publication never selects, downloads or verifies those report bytes.
The website retains `/coverage/` and `/coverage/html/` as links to the new report;
old deep LLVM source URLs are not mirrored. The homepage and documentation link
directly to the independent host. Review previews use the same landing pages,
which explicitly say the report may describe a different source revision.

### Website publication

The [website repository](https://github.com/sproates/panackelty-website) owns website tests and publishing. Core Check
does not wait for browser or website CI. Coverage publication remains independent.

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
These share fixed version-9 and malformed artifact vectors and compare exact
canonical artifacts and disassemblies. Fixture provenance and wire-format
expectations are documented in
`tests/fixtures/bytecode/contract_cases/README.md`.

Add new behavioral tests to the owning native/Panackelty probe;
`make unit` includes all these checks.

`unit/vm/resumable.c`, linked into the native module contracts, checks exact
instruction-budget boundaries, direct/indirect calls, interleaved independent
VMs, array/byte/range iterators, deep frame growth, terminal outcomes, fake-host
re-entry and suspended cleanup. The native allocation-failure suite sweeps
resumable creation, deep completion, traps and destruction while suspended.
The public `callables` fixture also executes 20,000 recursive calls through
source and saved bytecode. Sanitizer and coverage gates include these contracts.
`unit/vm/tasks.c` checks parent/child joins, failure propagation, cancellation
around every wait/delivery transition, inherited virtual deadlines, fairness,
queue saturation/retry, stale generations and cross-session identities. Three
verified corpus programs replay through pending print acknowledgements with the
original stdout expectations. Allocation sweeps include session admission,
repeated wait/delivery, cancellation, host failure and destruction while pending.
The fixed fake host has no OS producer; native cancellation is not yet tested.
`fixtures/execution` holds fixed performance inputs; the optional native module
command `resume FILE.bc BUDGET` captures `print` with a fake host to measure
instruction-budget overhead. Reproduction and limitations are in
[VALIDATION_PROFILE.md](VALIDATION_PROFILE.md).

`runner/stdlib_unit.panack` directly checks stable generic sorting, immutable
inputs, large exact values and literal Unicode suffix boundaries. It runs in
`unit-runtime-probes`; the checker probe rejects effectful/nonboolean sorting
callbacks. The collections functional case and example exercise the public
CLI in source and saved-bytecode modes.

`tests/unit/harness/distribution.sh` covers both conventional staged
installation and the download archive. Its archive test builds the packaging
layout, validates the complete file set, relocates the extracted
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
runner/architecture pairs, the package command, checksum and
provenance uploads, and the absence of tag or release triggers. It separately
requires the tag workflow to match `VERSION`, depend on complete validation and
both matrix packages, recheck downloaded assets, and confine write permission
to the final prerelease publication job. Website layout/publication checks live
in the website repository.

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
directories automatically, so adding a case does not require harness changes.
The Panackelty functional runner currently executes cases sequentially.

`runner/compiler_parser_unit.panack` imports the parser directly and checks
192 fixed expectations for expressions, blocks, types, and programs.
Its 38 groups cover valid input and exact malformed-input diagnostics.
`runner/compiler_lexer_unit.panack`
similarly covers the eight direct lexer contracts.
`runner/compiler_resolver_unit.panack` covers 26 exact source and module-graph
expectations in 12 groups, including diagnostic paths and positions. All run in `make unit` and
`make check-compiler`; each reports failures and exits nonzero on a mismatch.
The checker probe, `runner/compiler_checker_unit.panack`, also runs in both
targets. It checks 31 source fixtures, three module graphs and three sorting
callback contracts, requiring `ok`
for success and preserving diagnostic substrings for failures.
Read `fixtures/compiler_checker/README.md` when adding
a checker case. `runner/compiler_purity_unit.panack` follows the same pattern
for ten fixed source fixtures and one cross-module contract. Its maintenance contract is in
`fixtures/compiler_purity/README.md`. Both probes require `ok` for success
and check specific diagnostic substrings on failure.

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
inference, types and host boundaries. The probes use fixed expectations;
their provenance is recorded in
`tests/ORACLE_REPLACEMENT.md`. Seed regeneration uses verified self-hosted stages.


Direct VM execution and loader contracts run in `tests/runner/vm_unit.panack`
against the portable corpus in `tests/fixtures/vm_contracts`. Its 177 assertions
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
The [fixture guide](fixtures/host_runtime/README.md) describes direct native
evidence, fixed oracle fixtures and bootstrap cross-checks. Functional source
and bytecode cases verify public behaviour on both supported platforms.

`make policy` checks the source tree for forbidden interpreter dependencies and
runs adversarial policy controls. It is part of `make check`.
`make check-no-interpreter` repeats full validation, native conformance and
packaging with an allowlisted command environment.

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
whether native prerequisites and probe caches already existed with each experiment. Do not run
other builds concurrently. Repeat measurements only when noise or a regression
needs investigation. A clean check includes builds; the focused baseline starts
after the full check has populated ordinary native outputs and probe caches.
Parent rows include their children, and worker observations can overlap:
**do not sum nested rows**. One-second resolution is intended to find large
costs, not benchmark tiny operations. Probe rows include their source compilation
and execution; harness groups include their subprocesses. Fixture rebuilds can
appear more than once under different parents. Observations include profiling
overhead, and are not CPU measurements or end-to-end GitHub workflow duration.

Every Check packaging suite archives its clean suite profile,
including failure rows, separately from budget records. Expected negative-control
commands may have nonzero rows inside a successful harness group. `Validation profile`
collects an initial focused run and an immediate cached repeat for each
compiler/bytecode/VM target on both supported platforms when the profiler or
probe helpers change, or by manual dispatch. The initial run starts with native
prerequisites built; later components can reuse helpers built by earlier ones. It does not run on ordinary code or
documentation PRs. Existing required checks, sanitizers and release gates remain
in place. Findings and next investigations live in
[`VALIDATION_PROFILE.md`](VALIDATION_PROFILE.md).


## Change-aware CI

Use the same selector locally and in CI:

```sh
bash scripts/validate_change.sh --plan origin/main
bash scripts/validate_change.sh --run origin/main
```

The plan reports `route`, affected `components` and required `checks`. Local
selection compares the merge base with the working tree and index and includes
untracked, non-ignored files. The run checks branch/index/worktree whitespace,
then executes `make docs` alone for `docs`, `make docs` and canonical
`make check` for `full`. CI runs the same route with its additional platform,
sanitizer and coverage gates. No route caches test outcomes.
Use a different base ref when appropriate. Invalid local refs fail visibly;
missing CI history and empty diffs conservatively select full validation.
`make docs` remains available for a focused document check; it does not itself
classify changes. `make ci-check` exercises selection and execution regressions
without building the compiler, and remains part of `make check` through policy.

| Changes | Check workflow |
| --- | --- |
| `README.md` changes confined to the marked positioning-copy region before `## Exact fractions and Unit` | Document, local-link and whitespace checks; quick-start, examples, installation and packaged README content remain full-validation inputs |
| Only `ROADMAP.md`, `ARCHITECTURE.md`, `SELF_HOSTING.md`, `tests/README.md`, `tests/COVERAGE.md`, `tests/VALIDATION_PROFILE.md` | Document, local-link and whitespace checks |
| Only `CONTRIBUTING.md`, `docs/ROADMAP_PROCESS.md`, `.github/pull_request_template.md`, optionally mixed with the preceding row | Same document route; these instructions are reviewed prose, not consumed by build recipes, package installation or executable fixture extraction |
| Any other `README.md` change, or a mixture of positioning copy with any full-validation input | Full native validation and both platform packages |
| Any component, shared contract, unlisted path or mixture with code | Full native validation and both platform packages |
| Missing revisions/history or empty/unknown diff | Full validation |
| Classification or document checking fails/cancels | Existing named checks fail; no false successful skip |

All document inputs must be regular non-executable files on both sides.
README.md stays package-owned except when comparing the complete file outside
the bounded positioning-copy markers leaves it identical. Before those markers
exist on a historical base, the classifier uses the preamble through the fixed
`## Exact fractions and Unit` boundary; malformed or moved boundaries fall back
to full validation. The local selector checks both index and working-tree
versions so an unsafe staged edit cannot be hidden by an unstaged reversal. All
other inputs retain full validation. The classifier compares the complete PR
diff from its merge base. Renames expand into old-path deletion/new-path
addition; symlinks, executable Markdown and unmerged local indexes cannot use
the fast path. `scripts/ci_docs.sh` owns the explicit document allowlist and
positioning boundary. Other README content remains a packaged input, `SPEC.md`
defines executable behavior, and workflow files control execution; those retain
full checks.

### Component dependencies and retained coupling

`scripts/validation_components.sh` records core component ownership.

| Input owner | Dependencies and consumers that must retain evidence |
| --- | --- |
| Compiler | Compiler/harness probes, emitted bytecode, source/bytecode functional cases, self-hosting fixed point, installed toolchain |
| Bytecode contract | Compiler emitter, native verifier/VM, malformed fixtures, conformance, embedded/Wasm execution, package compatibility |
| VM/runtime and TCP | Native modules, fault tests, host probes, source/bytecode functional cases including real TCP peers, sanitizer and coverage runs; compiler itself runs on VM |
| Standard library | Compiler and runtime probes, bootstrap library identity, source examples, installed and browser libraries |
| Examples | Functional expected outputs, source/bytecode conformance, packaged examples |
| Package inputs | Installed documents/version, native toolchain, quick-start extraction, both supported platform archives |
| Shared or unknown | Conservative union of all consumers, including Makefile, seed, specifications, selectors and workflows |

The native integration envelope remains full for every non-document component. Existing focused `check-compiler`, `check-bytecode` and `check-vm`
targets help during development but do not replace acceptance checks. The
launcher builds the native VM, Panackelty probes execute on it, and packaging and
bootstrap combine components; the earlier independent VM audit does not prove
safe omission of those integrations. Separate compilation and dependency-aware
artifact reuse remain later work under #106.

Core Check uses the conservative shared selector. No core workflow builds Wasm
or provisions browser engines; browser and website repositories validate their
explicit dependency updates independently.

The fast path checks changed whitespace, empty/NUL-containing documents,
conflict markers, and local inline/image/reference link destinations outside
code fences. It also checks incoming links to allowlisted files, catching broken
references after deletion/rename. Remote URLs and heading fragments are not
validated; this is deliberately not a general Markdown renderer or network
crawler. Content still needs human review.

The existing `test`, `Package (linux-x86_64)` and `Package (macos-arm64)` results
remain stable as short, two-minute-bounded result gates. On docs changes the package-named jobs run only short guards on
Ubuntu and explicitly report that full validation is not applicable; they do
not claim a platform build took place. They verify that classification and docs
checks succeeded and full work was skipped. On full changes they require the
separate cancellable execution jobs to succeed; package gates require the whole
platform matrix. Those execution jobs retain their original platforms and all
gates. Only short result jobs use `always()`: putting it on an expensive job
would keep superseded builds running after cancellation. Per-PR cancellation
uses the `validation-...` concurrency group to separate this rollout from older
unconditional jobs. No branch-protection settings need changing. Workflow-wide path filters
are avoided so required check results are never left pending due to filtering.
Release validation and the separate profiling workflows retain their
existing triggers and gates.

## Native async TCP

`tests/tcp.sh` is part of `make functional` and hosted runtime validation. It
runs complete source and saved-bytecode programs through `panack` against the
independent C loopback fixture `tests/tcp_server.c`. It verifies exact binary
response content (including fragmented transfers and NUL), empty/exact-limit
responses, limit errors, stalled-peer timeout, refusal, and source type/effect
rejection. The refusal fixture releases an ephemeral loopback port before the
attempt; tests do not use a fixed service port or external network.

`tests/unit/vm/tcp.c` runs with native module, sanitizer and coverage suites:
concurrent independent fast/stalled executions, capability denial, external
completion rejection, timeout, cancellation, invalid inputs and forged call
metadata. Allocation sweeps include socket tracking and typed completion cleanup.
The network example lives under `examples/network` because it requires a peer;
`tests/tcp.sh` executes that exact example with its ephemeral port substituted.
`tests/fixtures/tcp/legacy-name.hex` was compiled by the alpha.10 seed from
`pure tcp_exchange(): Nat { 7 }` and `main(): Void { print(tcp_exchange()) }`.
The native bytecode suite checks, runs (expecting `7`) and disassembles it, proving
the new reserved intrinsic does not shadow old saved user functions.
All networking fixtures bind only loopback and own their cleanup.


Finite TCP server coverage uses `unit/vm/tcp_server.c` for owner/host contracts,
`unit/vm/native_faults.c` for allocation and descriptor cleanup, and
`tcp_serve.sh` with independent `tcp_client.c` peers for public source and saved
bytecode. These run through canonical unit/functional targets. Loopback binding
must be permitted. Playground runtime tests assert explicit WASI rejection.

## Website validation ownership

Prepared browser environments and publication timings belong to the [website repository](https://github.com/sproates/panackelty-website).
Historical core measurements remain in the validation profile as dated evidence.

`make source-mapping-experiment` runs the native-PC source-map acceptance tests, requiring
Node 24+ in addition to the native toolchain. It is separate from interpreter-free
`make check` and runs after the isolated compiler CI suite on Linux and macOS.
See [the experiment](experiments/source_mapping/README.md) for its exact scope,
trust boundary and printed size/timing evidence.

`make source-coverage-experiment` runs the bounded GI#131 feasibility harness
with the same Node/native requirements and isolated compiler CI placement.
It verifies exact instruction attempts, function entries, expression-start-line
reach, identity failures and incomplete collection, then prints cost samples.
See its [decision and denominator limits](experiments/source_coverage/README.md).
This is neither a production collection command nor a suite coverage baseline.


`compiler_source_spans_unit.panack` checks frontend expression spans with fixed
Unicode code-point/line/column expectations and nested range traces. It runs with
the native compiler unit suite; no external interpreter is required. Public CLI
location regressions live in the compiler integration suite. These checks prepare
U2 instruction mapping but do not establish a runtime source-map contract.


The U2 `compiler_instruction_sources_unit.panack` suite checks actual emitted
instruction indices and source ranges, lowered/unavailable attribution, optional
map/plain byte identity and original imported/generic locations. It runs under
`make unit` and `make check-compiler`. `make source-mapping-experiment` additionally
checks validated public CLI mappings against real native VM traps. The former
test-only sidecar and compiler adapter are removed.

`compiler_source_maps_unit.panack` and `compiler_source_maps_cli.panack` run in
both `make unit` and `make check-compiler`. They exercise the sidecar contract,
bounds and the public CLI on complete programs, including execution, exact
lookup output, stale closure, corrupt maps and alias-safe output failures.
The CLI suite also compiles a staged two-module project with same-named
declarations, requiring identity-qualified lookup and rejecting ambiguous bare
names and mismatched module identities. It runs those CLI cases through the
freshly prepared stage-2 compiler so they test this source tree's driver rather
than an older checked-in seed.
They need no Python or Node interpreter. The additional native observer suite
constructs binary forgeries and checks actual runtime PCs; its decoder is only
an adversarial test helper, never a production lookup implementation.

### U3 subtraction explanation acceptance

`tests/runner/compiler_explanations_unit.panack` checks independently expected
constant/guard proof rules, false and nested branches, guard-origin replacement,
mutation invalidation/refresh, loop-carried bounds, unsupported domains and
generic definitions. It compares diagnostic arrays with ordinary checking and
verifies ordinary checking retains no evidence. It runs in `make unit` and
`make check-compiler`.

The `cli_explanations` functional case runs complete projects through `./panack`,
asserting imported generic source locations, accepted/rejected/unavailable
results, ordinary source/bytecode behavior, absence of execution, whole-program
purity failure, parser/resolver failures and argument errors. The canonical
functional runner executes the case from source and compiled bytecode; its
runner-smoke expectations include it. Full acceptance remains `make check`.

### Indexed declaration lookup acceptance

`compiler_contracts_unit.panack` checks fixed lookup results across duplicate
and missing declarations, independent namespaces, interleaved imports and merged
module indexes. `compiler_integration_unit.panack` executes the imported
`fixtures/compiler_contracts/indexed_lookup` project with generic constructors,
guarded values and forward function references. These run in both ordinary
compiler validation and canonical `make check`; existing source-map and
explanation tests continue to cover attribution and proof evidence.

## U3 effect explanation acceptance

The existing explanation unit probe adds fixed effect expectations for declarations,
callable parameters/bindings/fields, widening, async/await, nested calls, branches,
loops, generic definitions, match payloads, source ranges and unlocated ASTs.
Retention-on/off diagnostics and empty ordinary evidence are compared. Public CLI
coverage adds imported generic rejection, allowed ordinary/async calls, callable
classification, discarded await and explicit unavailability after frontend errors.
No tests are removed; existing subtraction cases remain. The contract is
[documented here](../docs/COMPILER_EXPLANATIONS.md).

Validation: 196 explanation unit assertions, 36 explanation CLI assertions and
343 functional cases pass through canonical `make check` (150s). Existing
compiler-contract fixtures also preserve exact baseline output/status/artifacts
across 227 cases. Performance and fixed-point evidence are in
[the validation profile](VALIDATION_PROFILE.md#u3-local-effect-explanations--2026-10-03).

### Runtime provenance experiment

`make runtime-provenance-experiment` requires Node 24+ and the native toolchain.
It runs real-VM derivation, bounded-history, unavailable-evidence and public-CLI
assertions plus CPU/RSS observations. The retention unit checks also run in
`make check`; the complete experiment runs in Linux/macOS compiler CI. See its
[scope, privacy policy and findings](experiments/runtime_provenance/README.md).
No production runtime tracing command or alternate interpreter is added.

### Staged namespace resolution

`compiler_module_bindings_unit.panack` checks raw qualified syntax and original
cross-module declaration identities through selective aliases, namespace/enum
re-exports and repeated aliases. Negative cases cover private or hidden transitive
bindings, wrong binding kinds, conflicting lexical imports, missing/duplicate/cyclic
graphs and namespace values. A layered diamond checks one completed visit per
module. The existing diagnostic functional case runs qualified default-import,
type, function-reference and pattern sources through `panack check`, `compile`
and `run`, asserting the migration diagnostic and absence of bytecode output.
These tests exercise an internal, fail-closed resolution slice; checked namespace
execution and coordinated source migration remain pending.

## Reproducible build and validation baseline

Run the bounded baseline from the checkout root, with no other builds or trials
running on the host:

```sh
sh scripts/build_baseline.sh /tmp/panack-build-baseline macos-arm64-local 3 all
```

The output directory must be new and outside the checkout. Use a neutral host
label, without a login, hostname or personal identity. The default three repeats
are bounded to 1–5. `all` observes three clean `make check` runs, three unchanged
warm `make check-compiler` runs, then three runs of each small probe scenario:
unchanged, unrelated source edit, direct imported dependency edit and transitive
imported dependency edit. `probes` runs only the probe matrix after native setup;
it does not establish full/focused validation performance. Every trial is
sequential; normal validation retains its two internal workers.

The harness clones into an owned disposable directory and overlays the candidate,
including tracked changes/deletions and untracked non-ignored files. It retains
`source.tar`, `source.sha256` and the base commit/tree so the exact candidate can
be reconstructed even before publication. The archive includes the harness;
the probe fixture is generated afterward from that pinned script, with a relative
input manifest per observation. Ignored local configuration is excluded. Keep the
external evidence private until reviewed: raw source archives and logs can contain
local development content. The original checkout and its caches are not modified.

`environment.txt` records neutral host/OS/hardware, compiler and Make versions,
fixed `-O2`, serial Make dispatch, two validation workers and seed/VM/stage-2 hashes.
Inherited build/test overrides and budget overrides are cleared, retaining the
repository targets of 120/15/75/60 seconds for full/focused/functional/bootstrap.
The native-tool allowlist retains available `sysctl`/`sw_vers` host metadata;
platform commands absent from a restricted PATH report `unavailable`.
Project caches are removed before each clean full check; OS page caches are
uncontrolled. Warm focused runs reuse the final clean run's artifacts. Each probe
trial starts with a copy of the same successfully primed baseline cache at the same
path. Unrelated edits preserve output `42`; direct/transitive edits must output
`43`. All probes execute; cached test results are never used. The existing broad
source fingerprint and all compiler/seed/VM invalidation controls remain intact.

`samples.tsv` records external POSIX `time -p` elapsed seconds in the C locale,
status and completed
instrumented probe-build/probe-run/native-build command counts. Native build
commands include both compilation and linking; they are not translation-unit
counts. `profile.tsv` preserves existing whole-second inclusive component rows.
Summed probe compilation/execution seconds are aggregate observed worker time,
not a wall-clock decomposition: concurrent and nested observations overlap.
Within the standalone tiny probe, compile and execution labels are separate, but
whole-second resolution can report zero. Neither probe timings nor aggregate unit
timings are a substitute for the 15-second focused target. Apply 120 seconds only
to clean full validation and 15 seconds to the measured focused compiler check.

`summary.tsv` reports every scenario's sample count/minimum/median/maximum.
Individual logs, stderr, timing, workload `.status` and `.timer-status` files
remain available, including failed runs. The timed shell records the workload
status independently so a timer or wrapper cannot replace the workload result.
The timer is resolved to an absolute executable path; implicit shell exit is
disabled only while capturing its explicitly checked status.
A failed workload retains its original status; a failed timer also prevents a
successful sample even if the workload passed.
A `COMPLETE` marker appears only after every command and exact probe output passes;
failure stops the sequence with a nonzero status. Disposable work is removed on
completion or interruption. Results measure one host/revision, not a universal
budget guarantee or evidence of a before/after speedup.

`tests/build_baseline.sh`, included in canonical policy validation, uses a tiny
fixture repository, fake toolchain and the real compiled-probe cache driver to
check isolation, candidate snapshot fidelity, actual cache-hit/miss counts,
output checking, failure propagation, repeat bounds and overwrite rejection.
The timing regression runs through the restricted native-tool PATH, verifies
every sample invokes its external timer, preserves workload failure 9 when a
mock timer returns 1, rejects timer failure after workload success, and also
exercises dash when available.
The native-tool allowlist resolves the executable from PATH because some shells
report a `time` keyword rather than an executable from `command -v`.
These regression tests do not substitute for real-toolchain baseline trials.

### Source inventory contracts

The compiler/unit routes include `runner/compiler_source_inventory_unit.panack`
and `runner/compiler_source_inventory_cli.panack`: literal original-syntax
expectations plus public command, identity, failure and bytecode-equivalence tests.
See [source inventory](../docs/SOURCE_INVENTORY.md).
