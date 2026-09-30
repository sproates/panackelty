# Bootstrap seed

`compiler-v9.bc` is the stage-1 compiler seed for bytecode format 9.
Its SHA-256 is recorded in [`compiler-v9.bc.sha256`](compiler-v9.bc.sha256).
The historical v8 seed compiled the updated compiler in the old accepted source
subset. A temporary native decoder bridge ran that compiler to emit v9; repeated
v9 compiler builds converged. The bridge and v8 seed have been removed. This seed
includes async declarations, awaited calls, AsyncFn effects and typed fake reads.
No Python compiler or new development runtime was used for this migration.

The seed passes through the bounded native loader and verifier like every other
bytecode artifact. Normal bootstrap uses it to produce stage 2, then stage 3.
Stage 2 uses the content-checked probe cache described in
[the testing guide](../tests/README.md); its output is verified on each use.
The independent seed-refresh transaction below always builds fresh stages.
`make bootstrap-check` requires identical stage-2/stage-3 compiler artifacts and
stage-1/stage-2/stage-3 standard-library artifacts. It also tests a complete seed
refresh in a temporary directory with an allowlisted `PATH`.

## Refreshing the seed

From the repository root, run:

```sh
make regenerate-seed
make check
```

Refresh requires a C11 toolchain, Make, POSIX shell utilities and `sha256sum`
(Linux) or `shasum` (macOS). The existing seed compiles each successive stage.
`SEED_COMPILER` may select another seed; `SEED_DIGEST` defaults to that path plus
`.sha256`. The two files must share a directory. The digest file must contain
exactly one SHA-256 line with two spaces and the seed's basename, followed by a
newline, as in the checked-in manifest.

`regenerate-seed.sh` takes a per-seed lock and snapshots the seed and digest.
It verifies the recorded digest before asking the native VM to verify or run
that snapshot. It uses fresh, isolated artifacts, ignoring cached `build/`
outputs. The input compiles stage 2; stage 2 compiles stage 3; stage 3 compiles
stage 4. Each artifact must pass the native verifier, and all three compiler
artifacts must be byte-identical. Each then compiles the standard-library
conformance program. Those artifacts and their runtime output must agree, and
the output must match the checked-in `expected.stdout`.

Only after these checks, and a check that neither input file changed during the
run, does refresh replace the seed and its digest. It prints the input, compiler,
and conformance SHA-256 values for review. An unchanged seed is left untouched.
Review any binary and digest change together with the compiler source changes;
commit them only after the full validation passes. Do not regenerate independent
oracle goldens merely to match a new compiler.

Each file is published with a rename on the same filesystem. The pair is not
an atomic filesystem transaction: interruption between the two renames leaves
a digest mismatch and a subsequent refresh fails closed. Restore both files
from the last reviewed revision before retrying. Ordinary failures and handled
signals remove staging files and the lock; a forced termination may leave
`<seed>.refresh-lock`. Inspect it and ensure no refresh is running before removing
it. Restore inputs rather than blessing an unexpected digest mismatch.

This process requires a seed that can compile the current compiler source and
a native VM that can load its bytecode. A future incompatible format or language
transition needs an explicit, reviewed bridge from the existing seed.

## Implicit core transition

The current v9 seed loads the canonical Option/Result core and standard methods.
The previous v9 seed compiled a temporary source copy with an explicit core
import in compiler/types.panack; that bridge compiled the final import-free
compiler. The final seed is verified through the ordinary compiler and stdlib
fixed-point checks. No bridge source or compatibility aliases are retained.
