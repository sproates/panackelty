# Build and develop Panackelty

## Build Panackelty itself

This workflow is for contributors and people who want to build or inspect the
toolchain itself. Normal Panackelty programs should use a downloaded release.
Native development requires neither WASI SDK nor JavaScript packages.

A source build requires a POSIX-like Linux or macOS environment, a C11 compiler,
and `make`, plus standard POSIX utilities and a SHA-256 utility. The complete
development suite additionally requires Git for release-tag safety tests.
`make policy` rejects source-tree
interpreter dependencies; `make check-no-interpreter` runs a clean full check,
native conformance and packaging with only explicitly allowed tools in `PATH`.

The checkout directory may contain spaces and parentheses. The native VM builds
with `-O2` by default. Standard `CC`, `CPPFLAGS`, `CFLAGS`, `LDFLAGS`, and
`LDLIBS` overrides are supported; for a debug build, run `make clean` followed
by `make CFLAGS="-O0 -g"`. Clean before changing flags because existing binaries
do not automatically rebuild when command-line flags change.

From the repository root:

```sh
make                  # build the native VM as ./panack-vm
./bin/panack --help       # use the toolchain from the checkout
make check            # run all tests and the reproducible-bootstrap proof
make check-compiler   # focused compiler checks with public-CLI coverage
make check-bytecode   # focused bytecode and artifact checks
make check-vm         # focused VM and host-boundary checks
make native-fault     # allocation and host failure/cleanup contracts
make native-sanitize  # address/undefined-behaviour checks (also run in CI)
make native-coverage  # LLVM line/branch report and HTML under build/coverage
```


For reproducible local build/validation measurements, see the
[bounded baseline harness](../../tests/README.md#reproducible-build-and-validation-baseline).
It runs in a disposable checkout and retains exact candidate inputs and repeated
clean, warm and dependency-edit observations outside the source tree.
<!-- validation:coverage-publication-copy-begin -->

Browse the [public native VM coverage report](https://sproates.github.io/panackelty-coverage/)
for LLVM line/branch results and source navigation without downloading an
artifact. The landing page identifies the coverage commit, archive date and
successful validation run. This measures the native C VM corpus, not the
self-hosted compiler or whole-language coverage. The separate
[`panackelty-coverage`](https://github.com/sproates/panackelty-coverage) publisher
publishes only when manually dispatched during release preparation. It does not
poll on a schedule or publish on pushes. Documentation-only updates leave the
previous report in place; failed validation or publication leaves the last
published report visible. See [publication details](../../tests/README.md#public-coverage-publication)
and the [release gate](../releases/RELEASE_POLICY.md#release-gate).

<!-- validation:coverage-publication-copy-end -->
Direct bytecode/verification coverage runs in
`tests/runner/bytecode_unit.panack`, `tests/runner/bytecode_native_unit.panack`
and the native C verifier contracts in `tests/unit/vm/native_modules.c`.
These share fixed version-9 and malformed artifact vectors and compare exact
canonical artifacts and disassemblies. Fixture provenance and wire-format
expectations are documented in
`tests/fixtures/bytecode/contract_cases/README.md`.

The compiler check includes the complete direct Panackelty compiler assertions,
fixed independent expectations and public CLI fixtures. The same
assertions also run in `make unit`, whose timer includes the shell harness,
Panackelty probes and native C contracts.

The public compiler is written in Panackelty. To run it from source as an
ordinary Panackelty program:

```sh
./bin/panack run src/compiler/main.panack -- compile examples/euler001.panack -o build/euler001.bc
```

Internal test probes reuse compiled bytecode when their source and toolchain
contents match. Every check still executes the tests. `make clean` removes the
cache; see [the testing guide](../../tests/README.md) for the reuse and isolation rules.

Build and test the distribution with:

```sh
make package
```

This command completes the native conformance and reproducible-bootstrap gates,
builds the final archive, and smoke-tests that exact archive from a fresh
directory with only runtime tools available in `PATH`. It also
writes the archive's `.sha256` checksum and executes the packaged README's
quick start, upgrade, and removal procedures. Run
`make release-smoke` to rebuild and exercise only the archive gate.

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
the 15-second incremental budget. The former stage-0 implementation and
compatibility facade are removed. `make regenerate-seed` now verifies the recorded seed
digest, builds fresh self-hosted stages, and checks compiler/library identity
before replacing the seed. See [the refresh procedure](../../bootstrap/README.md).

Continuous integration runs on pull-request updates and pushes to `main`.
New commits cancel older runs for the same pull request. For implementation, mixed or uncertain changes, CI partitions the canonical
check into compiler/harness, runtime/functional and bootstrap jobs, alongside
independent sanitizer and coverage jobs. The focused targets above remain
available for local work; `make check` still runs the complete local suite. Changes limited
to the explicit informational-document allowlist receive quick document/local-link
checks, without compiler builds, packaging, sanitizers or coverage. The existing
check names remain present and reject routing failures. See
[change-aware CI](../../tests/README.md#change-aware-ci). Run
`bash scripts/validate_change.sh --plan origin/main` to inspect the shared
selection, or replace `--plan` with `--run` to execute its local route. Reviewed
process documents also use the informational route. Shared, mixed and unknown
changes retain full validation. Website changes validate independently in their
own repository. CI additionally executes platform and instrumentation gates.
Opt-in [detailed validation profiling](../../tests/README.md#detailed-validation-profiling)
separates native builds, harness groups, probes and bootstrap costs.

Continuous integration builds this package independently on Ubuntu 22.04
x86-64 and macOS 14 arm64. Each platform runs five clean, isolated suites via
`make check-no-interpreter CI_SUITE=…`: compiler/harness, runtime/functional,
bootstrap, source conformance and bytecode conformance/packaging. Together they cover the complete
check and package proofs with only allowlisted commands visible. The stable
package checks require every suite on both platforms to succeed.
The bytecode conformance job retains the exact archive,
its SHA-256 checksum, and a provenance record naming the source commit and
runner image. These routine CI jobs never publish a release.

Pushing an existing tag whose name exactly matches `v` plus `VERSION` starts the
separate release workflow. It reruns `make check`, rebuilds and smoke-tests both
platform archives, verifies the collected checksums and provenance, and only
then publishes a GitHub prerelease. Validation and package jobs are read-only;
repository write permission is scoped to the final publication job.

Project-wide contribution and validation expectations are documented in
[CONTRIBUTING.md](../../CONTRIBUTING.md).

The remaining direct compiler contracts now run in
`tests/runner/compiler_contracts_unit.panack` (201 assertions) and
`tests/runner/compiler_integration_unit.panack` (133 assertions), under both
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
The [fixture guide](../../tests/fixtures/host_runtime/README.md) describes the direct
native checks, fixed oracle fixtures and bootstrap cross-checks. Functional
source and bytecode cases verify public behaviour on both supported platforms.

Run development harness checks with `make harness`.
They cover repository/CI policy, packaging, timing, corrupt seeds and fixture-runner
failure propagation on both supported platforms.

### Source coverage inventory

`panack inventory SOURCE.panack -o FILE` records eligible source functions,
expressions, statements and decisions without running the program. Add
`--include OTHER.panack` for files outside the import closure. Validate with
`panack inventory-check SOURCE.panack --inventory FILE` and the same roots.
This is an inventory, not measured coverage; see [the contract](../reference/SOURCE_INVENTORY.md).
