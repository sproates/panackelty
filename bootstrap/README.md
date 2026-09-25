# Bootstrap seed

`compiler-v8.bc` is the audited stage-1 compiler seed for bytecode format 8.
Its SHA-256 is recorded in [`compiler-v8.bc.sha256`](compiler-v8.bc.sha256).
It was originally produced by the transitional Python compiler; the self-hosted
compiler reproduces those same bytes. The seed includes Path, Duration, Instant,
and the existing non-value `Void` argument rule. The format remains version 8.

The seed passes through the bounded native loader and verifier like every other
bytecode artifact. Normal bootstrap uses it to produce stage 2, then stage 3.
`make bootstrap-check` requires identical stage-2/stage-3 compiler artifacts and
stage-1/stage-2/stage-3 standard-library artifacts. It also tests a complete seed
refresh in a temporary directory with Python absent from `PATH`.

## Refreshing the seed

From the repository root, run:

```sh
make regenerate-seed
make check
```

Refresh requires a C11 toolchain, Make, POSIX shell utilities and `sha256sum`
(Linux) or `shasum` (macOS). It never invokes Python or the transitional compiler.
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
transition needs an explicit, reviewed bridge; there is no Python fallback.
