# Bootstrap seed

`compiler-v8.bc` is the audited stage-1 compiler seed for bytecode format 8.
It was produced from `src/compiler/main.panack` by the transitional stage-0
compiler. The portable C11 VM verifies the seed before execution, then uses it
to produce stage 2; stage 2 produces stage 3. `make bootstrap-check` requires
the stage-2 and stage-3 compiler and standard-library conformance artifacts to
be byte-identical.

The seed is a release input, not a trusted executable in the host process. It
passes through the same bounded native loader and verifier as every other
bytecode artifact. Maintainers can deliberately refresh it after compiler or
bytecode changes with `make regenerate-seed`, then review its changed digest
and prove the new fixed point before committing it.

Current SHA-256:

```text
6d5f1cb5ebccb8023b399e1a02e465726cbaf49083c194f402146ceb264aed6a  compiler-v8.bc
```

The current seed includes the Path, Duration, and Instant builtin signatures.
It is refreshed because the complete prelude imports the new time module; the
previous seed cannot check that module. The bytecode format remains version 8.

The compiler-unit migration refresh also enforces the existing non-value `Void`
argument rule (including nested `print`), matching the bootstrap checker.
This refresh uses the existing stage-0 process; replacing that process with a
Python-free seed workflow remains separate work. Fixed-point bootstrap and
release checks validate the refreshed seed.
