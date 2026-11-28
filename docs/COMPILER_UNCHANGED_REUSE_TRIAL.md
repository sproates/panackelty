# Unchanged-compilation reuse trial

7 November 2026. Implemented and validated locally on the development branch; integration
is pending. This continues [RM#28: Incremental and modular builds](roadmap/records/independent-website-publishing.md#rm-28)
after [reusing preparation within a command](COMPILER_REUSE_ASSESSMENT.md).

## What this does

An opt-in local cache reuses successful bytecode for ordinary `check`, `compile`
and `run`. Every invocation still discovers, reads and parses the current source
closure, checks source confinement and resolves current package inputs. A hit
skips semantic checking, lowering and bytecode emission. `run` always executes
the bytecode afresh with the current arguments and environment.

This is whole-compilation reuse when nothing relevant changed. Editing one
reachable source causes a fresh compilation of the whole project. It is not
module-level incremental checking. Explain, inventory, source-map, coverage,
disassembly and library-only checks remain fresh. No VM or bytecode-format
change is involved; the implementation is Panackelty using existing digest and
bounded-filesystem APIs.

## Opt in and bypass

With the trial toolchain, create a private directory outside the source tree,
stdlib and dependency trees, then pass its absolute path:

```sh
mkdir -m 700 "$HOME/.panack-compile-trial"
PANACKELTY_COMPILE_CACHE_DIR="$HOME/.panack-compile-trial" \
PANACKELTY_COMPILE_CACHE_TRACE=1 panack check /absolute/project/main.panack
```

Repeat the command to observe `compile cache: hit`. Leaving the variable unset
or empty bypasses reuse. Trace output is optional and goes to stderr; normal
program output is unchanged. Removing the directory's `compilation-v1.cache`
file forces a cold compilation. The global installed toolchain is not changed
by this trial.

The launcher supplies the actual compiler and VM paths and explicitly enables
its compiler child with the internal `--local-compile-cache` argument. Merely
inheriting environment variables does not enable nested/direct VM invocations
of a compiler. Direct internal invocation requires the flag and accurate
`PANACKELTY_COMPILER_PATH` and `PANACKELTY_VM_PATH` identities.

## Identity, validity and storage

The SHA-256 key includes exact reachable source paths and UTF-8 source snapshots
in load order, entry/project/stdlib roots, the complete resolved-project
descriptor when applicable, and compiler/VM paths and bytes. Timestamps are not
identity. Current source discovery runs before lookup, so changed import
resolution, missing files and confinement failures cannot be hidden by a hit.

The independent package tool still validates manifests, locks and packages on
every manifest command before invoking the compiler. Raw lock and manifest
parsing stays in that tool; the compiler fingerprints its resolved descriptor.
A stale lock is rejected before a compiler cache hit can be accepted.

Only successful compilations are stored. Before publication, source snapshots
are reread and the toolchain/key are checked again. Publication uses the existing
atomic-write API. Concurrent writers may replace one another's entry; the key
makes that a subsequent miss rather than a mismatched hit. The cache header
contains a format tag, input digest and artifact digest. Truncated, oversized,
corrupt, unavailable or unwritable entries fall back to fresh compilation.

The directory must already exist and be outside all input roots. Reads do not
follow symbolic links. There is one replaceable entry, bounded to 16 MiB including
its header. Source eligibility is bounded to 8 MiB and 256 modules, and each
toolchain file to 16 MiB. Exceeding a reuse bound disables reuse for that input;
it does not impose a new language or compilation limit. Different projects can
share the directory safely but evict one another, so this is deliberately a
small trial rather than a cache-management product.

This is trusted private local storage, not an authenticated artifact-download
protocol. An artifact digest detects accidental corruption, not malicious
replacement by someone who can rewrite both digest and artifact. Do not share
or import cache entries. Toolchain files are assumed stable for an invocation,
as with the existing local launcher. Runtime bytecode verification remains in
the VM; `check` relies on the trusted previous successful compilation.

## Evidence

The CLI regression suite covers cold/warm byte identity, fresh execution with
changed arguments, the explicit launcher gate, changed VM identity, changed
compiler bytes at the same path, source edits with preserved timestamps,
signature errors, missing/symlinked dependencies, corruption/truncation/size
limits, bypass, rejected project-local cache storage, publication failure,
concurrent publication, and fresh manifest/lock validation with a local package.

Alternating measurements used the same normal VM and source tree, with five
stdlib repetitions and three compiler repetitions per mode. Baseline is the
previous preparation-reuse slice; trial timings precede a final unused-import
and formatting cleanup. No builds or profiling ran alongside the measurements.

| Workload | Baseline | Trial bypassed | Cold | Warm | Warm saving vs baseline |
| --- | ---: | ---: | ---: | ---: | ---: |
| Stdlib example | 0.079131 s | 0.080780 s | 0.096177 s | 0.060509 s | 23.53% |
| Compiler self-compilation | 14.341385 s | 14.309989 s | 15.044921 s | 2.338506 s | 83.69% |

These are medians, not portable guarantees. Cold self-compilation is about 5%
slower due to fingerprinting, rereading and publication. The small example has
proportionally greater cold overhead. The trial remains disabled by default.
Peak memory has not been benchmarked for this slice; warm runs still retain
the freshly parsed source graph. Compiler warm times were 2.461308, 2.338506 and 2.316267 seconds. All four modes
emitted byte-identical artifacts for each workload. Final local `make check` passed (exit 0, 572 seconds), including unit,
functional, bootstrap, packaged release/relocation and quick-start checks. The
legacy 120-second aggregate timing warning remains advisory. This is not a
controlled full-suite performance comparison.

The existing test-only observer recorded 10 lex/parse calls in both cold and
warm stdlib runs. Cold work included 10 signature checks, 10 body checks and one
module-lowering/bytecode-emission pass; the warm run recorded none of these
semantic or emission phases. Both artifacts matched. Native oracle contracts
passed, and seed regeneration converged across stages 2–4. The final compiler
SHA-256 is `9779ed496d77d02bd021a50ac6dd3d1d299f4ea13d009b6b47033e53b165fd3a`,
identical to the measured candidate despite the final source-only cleanup.
The stdlib artifact retains SHA-256
`823a5f93fe2886a44b680cb09c1d634bdf6aa8fc0f5cdff57510cfe743eb8a0d`.

There is no claim that this speeds edited builds or the full validation suite;
those workflows already have separate caches and are not enabled by this trial.
Local evidence is retained in `build/compiler-unchanged-reuse/` (timings,
observer traces and validation logs). No CI or public publication was started.

No website update is needed: this is an opt-in development experiment, not a
change to published alpha.15 capabilities.
