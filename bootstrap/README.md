# Bootstrap seed

`compiler-v7.bc` is the audited stage-1 compiler seed for bytecode format 7.
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
4217eeea781f9a1da5d113f4f7df31c6646cc6d83fb1d7e6bd4b3afa98747b70  compiler-v7.bc
```
