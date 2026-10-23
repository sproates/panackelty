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

## Locked local dependencies on next

Projects with dependencies require `panack.lock.json`. Create it explicitly:

```sh
panack deps lock --project panack.json
panack check --project panack.json
panack run --project panack.json
```

Local directories are used in place; edits to their source files are live.
Local `.tar.gz` dependencies are verified and extracted under `.panack_modules`.
Changed manifests or archive bytes require `panack deps update --project
panack.json`. Add root dependency aliases after the manifest path to restrict an
update to those dependencies and their required subgraphs. Changes outside that
selection are refused. Commit the lock with the manifest; exclude the generated
`.panack_modules` directory from version control.

`panack deps restore --project panack.json --offline` verifies the locked local
archives and rebuilds missing or corrupt materialization without rewriting the
lock. Ordinary check, compile and run commands verify the existing lock and cache;
they never extract archives or repair a cache implicitly. A project without
dependencies needs no lock. Git acquisition is not yet
implemented; its syntax remains reserved by the package contract.

### HTTPS archives on next

Use an HTTPS archive declaration such as
`"utilities": {"archive":"https://example.org/utilities.tar.gz"}`. The archive
must contain `panack.json` at its root and obey the same extraction limits as a
local archive. URLs and redirect targets cannot contain credentials, queries or
fragments. Up to five HTTPS redirects are allowed; the lock retains the original
URL and compressed SHA-256.

Initial `deps lock` downloads and verifies the package. Later `deps lock` and
ordinary project commands only verify existing state. `deps restore` reuses a
verified cache or downloads the pinned bytes when repair is needed. Changed
remote bytes fail without changing the lock. Use `deps update [ALIAS...]` to
accept a deliberate change; unselected remote dependencies retain their pins.
`deps restore --offline` requires verified remote entries already in the cache.
It can still restore local archives from their declared paths.
