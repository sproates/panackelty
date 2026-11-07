# One-shot incremental compilation

The development compiler accepts:

```sh
panack compile --incremental main.panack -o main.bc
panack check --incremental main.panack
panack compile --incremental --project panack.json -o main.bc
```

Each invocation finishes normally. No watcher or background compiler is needed.
Panack manages disposable local state under
`$XDG_CACHE_HOME/panackelty/compiler`, or `$HOME/.cache/panackelty/compiler` when
XDG_CACHE_HOME is unset. The existing `PANACKELTY_COMPILE_CACHE_DIR` override
selects an existing private directory instead. Cache storage must be outside
project, standard-library and dependency trees. Missing, unavailable, damaged
or incompatible state causes fresh compilation, not a source-program failure.
Deleting the directory is safe; it only removes potential reuse.

## What is reused

An exact whole-build match reuses successful bytecode. After edits, Panack loads
plain checked bodies for unchanged modules whose transitive dependencies still
match. Each module file is replaced atomically; an edited build only rewrites
rechecked modules. Different entry paths have separate module-file namespaces.
The whole-build shortcut currently retains one last successful build per cache
directory. Source discovery, source parsing and package/confinement validation
still happen on each invocation.

Signatures, guarded-type checks, global effects, final lowering and bytecode
emission run fresh after a module-level hit. Tooling queries retain their full
explanation/source-map path. Cached bodies deliberately omit explanation
history and are never supplied to explanation-mode checking. Program execution
is never cached.

The disk format remains conservative:

- Keys include exact toolchain bytes and paths, roots, resolved package data,
  module identities/order, source paths and source contents. Modification times
  are not evidence of unchanged input.
- A source change invalidates that module and its transitive import consumers.
- A change to implicit core, or to any import, type, record or enum declaration
  invalidates every body. Declaration text, structural index and source
  coordinates are inputs. An ordinary function-body edit in the same file can
  reuse unrelated modules when those declarations and coordinates remain unchanged.
  Moving a declaration still invalidates globally, preserving retained origins.
- A graph/order, root or toolchain change invalidates stored bodies.
- Only successful plain compilations seed the cache. Entries have content
  digests and bounded decoding. They are private trusted local compiler state,
  not a portable or downloadable build-artifact format.

Current reuse limits are inherited from the earlier local-cache trial: 256
source files and 8 MiB of source, 16 MiB toolchain inputs and 16 MiB per cache
file. Larger inputs compile normally. The body decoder bounds nesting to 512
and checks collection/string lengths and complete consumption. Cache files can
be discarded between compiler versions; there is no migration requirement.

## Implementation

`module_cache.panack` owns input keys and disk I/O. `body_cache_codec.panack`
encodes plain typed bodies, interning repeated strings and storing source
positions needed by fresh effect checks and emission. The codec is generated
from `scripts/body_cache_schema.json` and bounded support routines using
`scripts/generate_body_cache_codec.cjs`; generation
is a development operation, not a compiler/build/runtime dependency. Schema
changes require reviewing the generator, regenerating the Panack source and
running its round-trip and malformed-input tests. The compiler and VM bytecode
format remains version 9; no native capability was added.

## Validation and measurements

The initial implementation merged in [PR #678](https://github.com/sproates/panackelty/pull/678).
Integration packaging passed on Linux and macOS; the coverage policy job found
four missing source-manifest entries, corrected in [PR #679](https://github.com/sproates/panackelty/pull/679).
The corrected integration run passed. All 117 focused checks, 15 CLI groups, unchanged-program
artifact compatibility and bootstrap fixed-point checks passed. Final
`make check` passed in 551 seconds, including unit, functional, packaging,
release-smoke and quick-start checks. The first full run caught a stale help
transcript; it was corrected and verified before the passing final run. No
development CI was triggered.

The fresh-process CLI tests cover automatic storage, unchanged builds, edited
entry modules, transitive invalidation, fresh diagnostics, damaged module files
and unavailable storage, alongside existing package, confinement, toolchain,
execution and concurrent-publication tests. Codec tests compare fresh versus
restored diagnostics and bytecode for collections, records/enums, callable and
async effects, domain constraints, owned core methods and string boundaries.

The initial serializer preserved bytecode/effect results but decoded the compiler
snapshot in 14.4 seconds. Interning repeated text reduced the snapshot from
38,193,316 to 8,680,806 bytes and decoding to 4.3 seconds. The final CLI uses
separate per-module dictionaries and files. A single-pass writer experiment
was stopped after its full-workload cache construction became much slower;
the shared immutable writer design appeared to introduce excessive copying.
The two-pass dictionary encoder remains the selected implementation. Three-cycle
end-to-end measurements on the same 49-module compiler source tree:

| Build | Median elapsed |
| --- | ---: |
| Ordinary fresh compilation | 16.015 s |
| First incremental build, creating state | 20.672 s |
| Unchanged incremental build | 2.513 s |
| Entry-file edit, reusing 48 modules | 14.018 s |

The entry-file timing changes a comment, deliberately invalidating the exact
source key without changing the emitted artifact. Actual dependency/body changes
and failures are covered by the CLI tests. Every measured artifact is byte-identical.
The edited-build median is 12.5% lower than fresh; unchanged is
84.3% lower. Creating state costs 29.1% extra in this workload.
These are local measurements, not a claim about every project or hosted CI.
The added codec also enlarges the compiler's own source workload; previous
14-second self-compilation figures used a smaller source tree and should not
be presented as a matched comparison. The final full check passed in 551 seconds.
The separate [hosted cache trial](COMPILER_HOSTED_CACHE_TRIAL.md) now verifies
authenticated reuse across two jobs. Ordinary CI cache use is not wired; bootstrap
proofs remain fresh.

This is an initial persistent module-body cache, not complete fine-grained
incremental compilation. Fresh parsing and signatures, conservative declaration
invalidation, cache decoding and cold-cache construction remain costs.

No website publication is part of this change. Website claims for alpha.15
remain unchanged; an alpha.16 release guide can introduce the command after
integration and release validation.

## Narrower declaration invalidation

The follow-up fingerprints non-function declaration source ranges separately
from ordinary bodies. This removes the previous whole-file contribution for
files that contain types. Global signatures, guards and effect closure remain
fresh; changed modules and transitive import consumers are still rechecked.
Imports remain global inputs because they can change how guarded bases resolve.
This is module reuse, not individual-function reuse within the edited module.

All 24 CLI groups pass, including a real body edit alongside a record, enum and
guarded type; independent sibling reuse; changed guards, fields and variants;
declaration removal; Unicode/position changes; same-width edits before a type;
import alias changes; failed checks and recovery. Fresh and incremental bytecode
and failure diagnostics match. Native-oracle artifact checks and regenerated
bootstrap stages 2–4 agree. Final `make check` passed in 591 seconds, including unit, functional, bootstrap,
packaging, relocated release smoke and quick-start checks.

Three isolated same-tree cycles changed the `module_magic` function in
`module_cache.panack`, which also declares records. The edit changes emitted
code; each edited incremental artifact matched its fresh counterpart. Medians:
fresh 15.906s, cold incremental 20.655s, unchanged 2.447s, edited 13.860s with
46 of 49 modules reused (12.86% less time than fresh). This measures a different
edit from the earlier entry-file trial; the percentages are not cumulative.

The coverage manifest now includes all four new compiler modules. The production
parser/inventory reviewed 484 declaration exclusions over 51 eligible files:
294 imports, 157 records and 33 enums. Executable functions remain eligible.
All 77 fast policy tests pass; this is not a new coverage collection or a claim
about coverage percentages.
