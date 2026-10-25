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
dependencies needs no lock.

### Package diagnostics on next

Package failures explain what happened, why the operation cannot proceed and
what to check or do next. They include an error-family code, the project,
operation, failed phase and available dependency context. Transitive failures
show the declaring chain. Failures go to stderr with status 1; successful command
output is unchanged.

A missing or corrupt cache normally calls for `deps restore`, which preserves
pins. An integrity mismatch is different: investigate the source or recover the
original content before deliberately accepting an update. Git failures include
exit status and the requested identity, but do not expose raw stderr or pretend
an ambiguous failure proves a particular server problem. See the
[verified diagnostic examples](../design/PACKAGE_DIAGNOSTIC_EXAMPLES.md) for real
messages and tested recovery commands.

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

### Git dependencies on next

Declare a dependency with an HTTPS repository URL:

```json
"utilities": {
  "git": {
    "url": "https://example.org/utilities.git",
    "ref": "v1.2.0",
    "subdir": "packages/utilities"
  }
}
```

`ref` may be a branch, tag or full commit ID. Omit it to select the remote
HEAD during explicit lock/update. Omit `subdir` for a package at the repository
root. Abbreviated object IDs and revision expressions are rejected; disambiguate
branch/tag names with `refs/heads/...` or `refs/tags/...`.

Install Git 2.29 or newer on PATH for initial acquisition and cold restore.
Git runs with private configuration and no credential helpers, hooks or filters.
Use a direct HTTPS URL without credentials, query or fragment; Git redirects
are rejected. TLS verification stays enabled. `CURL_CA_BUNDLE`, when set by the
caller, supplies the trusted certificate bundle for both archive and Git transport.

The lock pins a full commit. Moving a branch or tag changes nothing until
`deps update`. Restore fetches only that commit; an unavailable commit fails.
Verified warm and offline commands do not invoke Git or access the network.

Packages from one repository commit share a snapshot under
`.panack_modules/.sources/<source-key>`. Relative sibling dependencies may stay
within that snapshot; they cannot read outside it. Shared packages in a diamond
resolve to one physical root. If a selective update would give the same package
two different roots, it fails without changing the lock; update the affected
root aliases together. A package's imports cannot follow links into a sibling's
private files.

Snapshots are limited to 16,384 entries and 256 MiB expanded content. The fetched
Git pack has no pre-fetch byte quota. Cross-source acceptance and final installed
workflow acceptance remain subsequent programme work.
