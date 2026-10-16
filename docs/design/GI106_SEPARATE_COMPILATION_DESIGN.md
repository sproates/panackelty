# GI#106: separate-compilation design sketch

**Status:** design only. This document is not approval to change compiler
semantics or the bytecode format.

## Why examine it

Panackelty already has source-module boundaries and a reusable compiled-probe
cache. Those are not separate compilation. The probe cache reuses a complete
entry artifact only when the entry's static source closure, seed compiler, VM
and runner inputs match. A source edit inside that closure rebuilds the entry.
Separate compilation would cache module-level contracts and code so multiple
entry programs could reuse a module independently.

This is a plausible longer-term response to repeated frontend work, but it is a
compiler architecture change. The immediate probe-cache pilot does not prove
that separate compilation will shorten a representative clean build. Measure
that before implementation.

## Current compiler boundary

The project loader resolves the entry and recursively loads its imports plus
`core.panack`. It records `LoadedModule` values, physical source paths, source
text, binding inventories and diagnostics. It constructs a module binding graph
using package/module identities and checks cycles, visibility and imports.

For namespace projects, the loader then checks declaration signatures across
the loaded modules, binds module bodies against those signatures, checks
effects, and combines declarations into a `Program` for the existing whole-
program resolver, checker and emitter. `LoadedProject` retains module
signatures and checked bodies as well as the combined program. `ModuleSignature`
and `BoundFunctionBody` are useful design inputs, but neither is yet a stable,
serialized module artifact. The bytecode output is still one linked program.

This layout gives Panackelty a semantic graph to build on. The global
signature/body passes, spelling-based maps in later passes, final combined
program and single bytecode output mean source files cannot yet be checked and
emitted as independent reusable artifacts.

## Candidate artifact model

Use two related artifacts per module:

1. **Module interface** — the stable information an importing module needs:
   package and module identity; exported declaration names and kinds; record
   fields, enum variants and guarded types; generic binder positions and
   constraints; parameter/result types; callable effects; nominal identities;
   and public re-exports. Preserve original declaration identity through
   aliases. Do not encode identity as a source spelling alone.
2. **Module implementation** — checked bodies and code-generation material,
   including a symbol table that can be linked into today's final bytecode
   program. Keep this as an internal build artifact first; no promise of a
   separately distributable library binary is needed for the pilot.

Each artifact key should cover its source digest, interface digest, direct
dependency interface digests, package identity, compiler build and interface
format versions, target/bytecode version, and every semantic compiler option.
The module identity must distinguish toolchain modules, standalone project
modules and named/versioned package modules while remaining independent of the
checkout's absolute path.

The existing `ModuleSignature` is a starting point for interface contents. It
does not yet define a canonical serialization, a compatibility/versioning rule,
or complete identity for every type shape. Those are design work, not details to
defer to a cache implementation.

## Dependency and invalidation rules

The first correct implementation should use the loader's resolved module graph,
not an independent parser or a guessed filesystem scan.

| Change | Required work |
| --- | --- |
| Module implementation changes; exported interface is identical | Recheck and re-emit that module, then relink dependent executable output. Importers need not repeat type checking if their interface inputs remain identical. |
| Exported signature, visibility or re-export changes | Rebuild the interface and recheck/rebuild every dependent module reachable through interface edges. |
| Transitive imported interface changes | Rebuild the affected reverse dependency closure. |
| New, removed or retargeted import | Rebuild the importing module's graph and all affected reverse dependents. |
| Core, standard-library ABI, compiler, bytecode format, target or semantic flags change | Invalidate every artifact whose key includes that identity; core/toolchain changes normally invalidate the whole project closure. |
| Cache entry is missing, corrupt, incompatible or cannot be proven current | Rebuild it. Never turn a cache uncertainty into a successful check. |

The loader currently diagnoses module cycles. Separate artifacts can therefore
start with the same acyclic import rule, while signature collection remains a
two-phase operation within each dependency layer if types require it. If the
language later permits cycles, an explicit cycle contract is required before
caching them.

## Linking and compatibility

Keep the current final bytecode contract initially. A module implementation
artifact should expose stable semantic symbols, not raw numeric function slots
that change when another module is added. The linker can assign final slots,
resolve cross-module references and emit the existing bytecode once the full
entry closure is known.

This limits the first change to frontend/module reuse and linking. It avoids
requiring the VM to load several bytecode files at runtime, changing package
installation, or promising binary compatibility between releases. Source maps,
coverage identities, diagnostics and package relocation still need stable
module/declaration identities in both the intermediate artifact and final
bytecode.

## Main risks and open design questions

- **Identity preservation:** namespace imports, selective imports, public
  re-exports, nominal types and package versions must resolve to the same
  declaration identities as monolithic loading.
- **Generic and effect contracts:** generic binders, substitutions, inferred
  callable types, purity and async effects must cross the interface boundary
  without losing information that today's whole-project passes can see.
- **Visibility and diagnostics:** private declarations must not leak through an
  interface; diagnostics should retain the original source path and spans when
  consuming cached contracts.
- **Global passes:** checker, effect analysis, core reachability and emission
  still consume combined data. Identify which must become module-local and
  which remain link-time passes before splitting implementation artifacts.
- **Build graph ownership:** the probe cache hashes raw import syntax and falls
  back broadly on uncertain imports. Production builds should consume the
  compiler's resolved graph and should not duplicate import-resolution rules.
- **Disk and invalidation cost:** tiny modules may spend more time serializing,
  hashing and linking than they save. Cache size, pruning and parallel builds
  need evidence.
- **Bootstrap:** stage 0/1/2 compiler fixed points, bundled standard-library
  identity and compiler-version transitions must all produce reproducible
  interfaces.

## Bounded investigation before implementation

1. Inventory every `LoadedProject` consumer and classify its inputs as
   module-local, dependency-interface, whole-closure or runtime/link-time.
2. Define the complete interface value and stable encoding, including version
   rules, using current module identities, signature types, visibility and
   re-exports.
3. Trace one acyclic fixture through resolve, signature, body, effect and
   emission stages; specify expected keys and invalidation for implementation-
   only, interface, transitive-interface, add/remove and toolchain edits.
4. Measure a clean and warm multi-module compiler workload and separate
   frontend, emission, linking and harness time. Use this to decide if a
   prototype is worth the correctness and maintenance cost.
5. If the user chooses a prototype, first implement it behind an explicit
   compiler/build flag, compare diagnostics and bytecode execution with the
   monolithic path, then exercise bootstrap, package, source-map and coverage
   checks. Keep the monolithic path until equivalence evidence is complete.

No implementation or numeric performance acceptance target is proposed here.
GI#106 remains open for the user's review and acceptance.
