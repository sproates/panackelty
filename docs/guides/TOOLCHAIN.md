# Using the toolchain

## Use the toolchain

Panack source files use the `.panack` extension. Compiled programs use `.bc`.

```sh
# Type-check without producing bytecode
panack check hello.panack

# Compile beside the source, or choose an output path
panack compile hello.panack
panack compile hello.panack -o hello-release.bc

# Run source or verified bytecode
panack run hello.panack
panack run hello.bc

# Inspect the generated instructions
panack disasm hello.panack
panack disasm hello.bc
```

Arguments after the input path are passed to the program.

### Explain checked types, guarded conversions, subtraction and effects

```sh
panack explain example.panack --function remaining
```

The explanation query shows inferred local types and their checked initializer
evidence, proof-bearing conversions into guarded types, checked array
`map`/`reduce` callback contracts, and why a `Nat` subtraction is proved or
unproved, using the checker's actual constants or guard bounds and their source
locations. It also explains how array elements, conditional results and match
arms combine into a checked result type, and local
call and await effect boundaries, including why pure or
async code rejects a call, and reports whole-program acceptance separately. Unproved does not
mean a runtime failure was demonstrated; unsupported explanations remain explicit.
A body type error in another function need not hide valid local effect evidence;
global declaration type errors still make it unavailable. Original diagnostics and
whole-program rejection are preserved. The command does not execute your program. See the
[query contract and example](COMPILER_EXPLANATIONS.md) for scope and limits.

### Locate an instruction in its source

Compile with an optional map, then validate a function/instruction location:

```sh
panack compile hello.panack -o hello-mapped.bc --source-map hello.pmap
panack locate hello-mapped.bc --source hello.panack --source-map hello.pmap --function main --instruction 0
```

Use fresh output filenames. Lookup recompiles the exact source closure and
accepts the pair only if both artifact and map match; stale or missing evidence
returns `source-map: unavailable`. Execution and bytecode v9 are unchanged.
Sidecars contain complete source text, including imports and comments. Read the
[trust, privacy and compatibility contract](SOURCE_MAPS.md) before sharing
one. Automatic source-aware runtime errors remain future work.
