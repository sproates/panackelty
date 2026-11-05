# Compiler and standard-library boundary assessment

GI#409: Compiler/stdlib boundary assessment. Evidence collected 26–27 October
2026 against `aced53db9d7c03740845532d86c6dcc58b336bfd` on macOS arm64.
This is an assessment, not a compiler, library or build-system refactor.

## Finding and recommendation

Keep the current source organisation. The compiler already has explicit file
and path adapters, and its stage-2 compilation cache already follows its actual
source closure. An application-only library edit does not inherently require
recompiling the compiler. Adding a new host primitive is a different matter:
its name, types and effects are recognised by the compiler and its implementation
and calling contract are registered in the VM.

The smallest justified next step is to assess narrower artifact dependencies
within **GI#106: Validation speed**. In particular, replace the package tool's
all-stdlib timestamp prerequisite with a validated dependency closure if baseline
measurements justify it. Preserve content-based invalidation, new/deleted imports,
implicit core, compiler/VM identity, corrupt-artifact rejection and conservative
fallbacks. Do not weaken validation or infer that a cache hit is test acceptance.
This is a recommendation for that task's existing measured-selection checkpoint,
not an additional implementation commitment or numerical target.

Leave changes to the host-operation registration model with the alpha.16 native
coupling work. Moving wrappers into a new directory would not remove those
compiler/VM contracts. No new compiler-support package, independent component
release scheme or separate compilation is justified by this evidence alone.

## Complete source dependency map

The existing dependency walker, `tests/probe_dependencies.awk`, reports **42 files**:
27 compiler modules, two bytecode modules, four shared project-model modules and
nine stdlib modules. The public `panack inventory` command independently loaded
exactly the same 42 files. Every retained source snapshot matched its checkout
file byte for byte; the inventory compiler identity matched the checked-in seed.
This is the complete loaded closure for the current compiler entry point, not a
claim that every declaration executes in every compiler command.

The entry is `src/compiler/main.panack`, which imports `driver.compiler_main`.
The driver reaches the frontend/loader/checker/emitter, bytecode codec/decoder,
explanation, inventory, coverage and source-map facilities. Shared project-model
code supplies manifest/descriptor parsing and confined dependency source loading.
It does not put package acquisition, archive extraction or Git policy into the
compiler: `src/packages/` and `packages/http/` are absent from this closure.

| Stdlib module | Import paths into the compiler closure | Purpose |
| --- | --- | --- |
| `core` | Implicitly loaded by `loader`; also `path → core` | Option/Result, core error types and source-defined owned methods. Loaded even when a program has no imports. |
| `compiler_files` | `filesystem_support → compiler_files` | Existing compiler-internal trapping text/byte file operations. |
| `compiler_paths` | `path_support → compiler_paths` | Existing compiler lexical string-path operations. |
| `bytes` | Bytecode codec/decoder; source maps/inventory; coverage session; shared package manifest | Empty buffers and bounded byte slicing used for serialization and parsing. |
| `environment` | Driver, loader, coverage, coverage session, inventory | Configuration, root paths and compiler/tool identities. |
| `path` | Driver, loader, coverage session, source maps | Typed paths for filesystem and output operations; imports core error identities. |
| `filesystem` | Driver, loader, coverage session, source maps | Typed reads/writes, metadata and directory operations; re-exports HostError. |
| `package_filesystem` | Driver, loader, project frontend, shared package project and resolved project | Root identity, overlap checks and confined reads of dependency sources; re-exports HostError. |
| `host` | Driver, coverage, coverage session, inventory; transitively filesystem/package_filesystem | Arguments, output, exit, nested bytecode execution and shared host error identity. |

The compiler-local adapters are one-line public re-exports in
`src/compiler/filesystem_support.panack` and `path_support.panack`. A named
compiler-facing boundary therefore already exists for those two services.
The compiler also legitimately uses typed filesystem/path and shared byte APIs;
forcing all of these through another layer would add interfaces without removing
real dependencies.

The full 42-file closure, relative to the repository root:

```text
src/bytecode/{codec,decoder}.panack
src/compiler/{checker,coverage,coverage_session,diagnostics,driver,emitter,
  explanations,expression_contracts,filesystem_support,lexer,loader,main,
  module_bindings,module_bodies,module_effects,module_emission,module_resolution,
  module_signatures,owned_intrinsics,parser,path_support,project_frontend,purity,
  resolver,source_inventory,source_maps,types}.panack
src/project_model/{package_manifest,package_project,resolved_project,utf8}.panack
src/stdlib/{bytes,compiler_files,compiler_paths,core,environment,filesystem,
  host,package_filesystem,path}.panack
```

Brace notation above lists files; it is not Panack import syntax. In addition to
these Panack sources, core primitives and intrinsic methods depend on compiler
schemas/lowering and VM value operations. `owned_intrinsics.panack` records owned
method identities/signatures; `module_bodies.panack` checks them. Primitive and
host name resolution, arity and effects also appear in `resolver.panack`,
`checker.panack` and `purity.panack`; the Panack bytecode decoder reuses those
contracts. This implicit semantic coupling is not an extra source import.

## Build, bootstrap, native and release boundaries

- **Compiler artifact:** stage 1 copies/checks the seed. Stage 2 invokes
  `tests/run_probe.sh`, which fingerprints the loaded closure, VM, seed, helper
  scripts and roots. The phony `probe-inputs` prerequisite invokes this check
  every time; it does not necessarily compile. A valid digest-checked artifact
  is reused and the output is copied only if its bytes differ. Unsupported
  dependency syntax falls back to a conservative broad source fingerprint.
- **Bootstrap:** stage 3 runs stage 2 over compiler source. Fixed-point validation
  compares compiler stages and stdlib-conformance artifacts. Explicit
  `bootstrap/regenerate-seed.sh` checks further stages and conformance before
  publishing a seed and digest. An ordinary library edit does not automatically
  rewrite the checked-in seed. Shipping changed compiler behaviour requires a
  suitably regenerated/validated seed because installation ships that seed, not
  an arbitrary local stage-2 artifact.
- **Native runtime:** `Makefile` compiles separate `src/vm/*.c` objects, tracks
  included headers through `.d` files and links one VM. The VM currently links
  libcurl, zlib and libcrypto regardless of the compiler entry's source imports.
  Native changes can rebuild/relink the VM and consequently invalidate the probe
  cache by VM identity, even when the compiler's emitted bytes would be unchanged.
  This is conservative tool identity, not proof of a Panack source dependency.
- **Package tool:** its own entry is `src/packages/main.panack`; it compiles with
  the seed, separately from the compiler. Its prerequisites include every stdlib
  source and every HTTP-package source. Some are legitimate dependencies, such as
  `process` for Git. Others are not: `testing_async` is in neither its source
  closure nor the compiler's. The HTTP-server artifact has similarly broad
  stdlib prerequisites and also depends on the package-tool artifact.
- **Release:** `bin/panack` reads `VERSION` at invocation and selects the bundled
  seed/VM/stdlib. Installation copies those components together. `VERSION`
  determines package naming and installed metadata; it is not a prerequisite of
  the compiler, native objects or package-tool bytecode. Release identity,
  compatibility checks and archive regeneration still matter. Bytecode format
  version 9 and intrinsic contracts are separate from the release label.

Source evidence: [Makefile](../Makefile), [probe cache](../tests/run_probe.sh),
[dependency walker](../tests/probe_dependencies.awk),
[loader](../src/compiler/loader.panack),
[owned intrinsics](../src/compiler/owned_intrinsics.panack),
[VM builtin registry](../src/vm/builtins.c), [VM verifier](../src/vm/verify.c),
[seed refresh](../bootstrap/regenerate-seed.sh), [launcher](../bin/panack).

## Five change cases

“Observed” below distinguishes real cache executions from Make dry-run plans.
Dry runs use timestamp-current stand-in outputs only to inspect prerequisites;
they are not compiled or executed as evidence of correctness.

| Change | Observed rebuild behaviour | Necessary correctness work and validation |
| --- | --- | --- |
| Application-only library: `testing_async` (also checked `process` for compiler independence) | Real compiler cache probe: zero compilation calls. A Make `-W` plan for `testing_async` schedules package-tool compilation despite its absence from that tool's closure; no native compilation/link. | Recompile affected consumers and test that library. No compiler/seed rebuild is implied by an absent dependency. The current broader validation policy remains applicable until separately changed. `process` is a legitimate package-tool dependency; its package rebuild is not evidence of waste. |
| Used helper: `compiler_paths` | Real content change invalidates the compiler cache and invokes compilation once. Make schedules package-tool compilation too. No native compile/link. | A semantic helper change must reach consuming compiler artifacts; validate compiler path behaviour, affected callers and bootstrap/release seed as appropriate. A comment-only probe establishes invalidation, not a semantic requirement to change artifact bytes. |
| Implicit core type/method | Real `core.panack` content change invalidates the compiler cache and invokes compilation once. All compilations load core; package-tool plan also rebuilds. | Recompile consumers of changed core definitions and test type/method semantics, compiler self-hosting and conformance. A source-defined helper does not inherently require C changes. Changing a primitive representation or intrinsic method contract may require compiler schemas/lowering and VM changes as well. |
| New host capability | A call to an unregistered synthetic host operation is rejected at source binding. Existing `process_run_isolated` requires compiler recognition/type/effect/arity rules plus a VM registry entry and native implementation. A Make plan marking `host_capabilities.c` changed compiles one native object, relinks the VM and schedules the package tool; stage-2 cache checks include VM identity. | Add/check all relevant compiler and VM contracts, the Panack wrapper, negative arity/type/effect cases, native resource/error/cancellation behaviour, bytecode verification and seed compatibility. Rebuilding is needed when adding a new primitive under this architecture; composing existing host operations in Panack need not change the compiler. No new host operation was implemented in this assessment. |
| Behaviour-neutral version/packaging metadata | Make `-W VERSION` schedules no native or package-tool compilation. The stage-2 cache recipe is still visited because of `probe-inputs`; VERSION is absent from its fingerprint. Package archive naming and installed VERSION change. | Repackage and validate version output, metadata, installation and archive/release integrity. No semantic compiler rebuild follows from changing the release label alone. Packaging script/configuration changes may require their own checks; this is not a blanket exemption. |

For host coupling, trace `stdlib/process.panack` → `process_run_isolated` through
`resolver.panack`, `module_bodies.panack`, `checker.panack`, `purity.panack`,
`src/vm/builtins.c` and `host_capabilities.c`. VM verification rejects calls absent
from both the builtin registry and the program, and checks registered arity.
Existing call instructions carry names and arities; adding a primitive does not
in itself prove a new bytecode opcode or format version is necessary.

## Probe results and reproduction

`make native` built the current VM without source changes. Seed SHA-256:
`aab8b0557c9525ac1e917d093e1c44452ba746fde59d880b7ea3e5c2a24509df`.
The copied-stdlib experiment compiled the actual compiler entry, not a toy
replacement. A forwarding VM wrapper counted `run SEED compile` invocations.

| Run | Actual compiler invocations | Observed whole-second elapsed time |
| --- | ---: | ---: |
| Cold cache | 1 | 40 s |
| Unchanged repeat | 0 | <1 s |
| Comment appended to unused `process` | 0 | <1 s |
| Comment appended to used `compiler_paths` | 1 | 32 s |
| Comment appended to implicit `core` | 1 | 31 s |
| Further comment appended to unused `testing_async` | 0 | Not separately timed |

All resulting compiler artifacts were byte-identical to the seed. Comments
changed source content and invalidated dependent cache entries without changing
executable semantics. These are single local observations with coarse timing,
not controlled performance benchmarks or targets for GI#106.

Run from the repository root at the evidence revision. Use a fresh evidence
directory; all modified source copies and generated outputs stay inside it.

```sh
make native
mkdir -p build/gi409
cp -R src/stdlib build/gi409/stdlib
awk -v cwd="$PWD" -v source=src/compiler/main.panack \
  -v stdlib_root=src/stdlib -v project_root="$PWD" \
  -f tests/probe_dependencies.awk > build/gi409/closure.txt
PANACKELTY_BOOTSTRAP_ROOT="$PWD" ./bin/panack inventory \
  src/compiler/main.panack -o build/gi409/compiler.pinv
```

The inventory is an actual loader-derived source snapshot. Its 42 paths and source
bytes were compared with the dependency walk and checkout using the existing
`Reader` from `scripts/source_coverage.cjs`: read the single-root
`PANACKINVENTORY1` envelope, then `PANACKINV1/local-replay-v9`, compiler blob,
entry, artifact blob and the counted source snapshots/items. Resolve `stdlib/`
names against `src/stdlib` and `project/` names against the entry's directory.
The complete reader consumed the container with no trailing bytes.

For cache probes, set `PANACKELTY_STDLIB_PATH` to the copied stdlib and
`PANACK_PROBE_CACHE=build/gi409/cache`. Set `PANACK_PROBE_VM` to an executable
forwarding shell wrapper containing:

```sh
#!/bin/sh
printf '%s\n' "$*" >> build/gi409/vm-calls.log
exec ./panack-vm "$@"
```

Clear that log before each invocation of:

```sh
sh tests/run_probe.sh --compile src/compiler/main.panack build/gi409/compiler.bc
```

Count lines beginning `run ` containing ` compile `; `check` calls are validation,
not compilation. Run cold and warm, then append a comment separately to copied
`process.panack`, `compiler_paths.panack`, `core.panack` and `testing_async.panack`,
rerunning after each. Leave the wrapper bytes unchanged throughout. Hash the
result each time. Do not edit the tracked stdlib for this experiment.

For Make plans, copy the seed into three fresh, timestamp-current files
`build/gi409/stage1.bc`, `stage2.bc` and `package-tool.bc`. These are stand-ins for
up-to-date outputs, not claims that the package tool has been built. Run:

```sh
make -n -W src/stdlib/testing_async.panack \
  STAGE1_COMPILER=build/gi409/stage1.bc \
  STAGE2_COMPILER=build/gi409/stage2.bc \
  PACKAGE_TOOL_ARTIFACT=build/gi409/package-tool.bc \
  build/gi409/stage2.bc package-tool native
```

Compare with no `-W`, and repeat for `compiler_paths.panack`, `core.panack`,
`src/vm/host_capabilities.c` and `VERSION`. Baseline and VERSION schedule zero
native/package compiles; each stdlib case schedules one package compile; the
native case additionally schedules one C compilation and one VM link. All plans
visit the stage-2 cache recipe. Do not mistake that visit for a cache miss.

Finally, checking `main(): Void { gi409_unregistered_host_operation(); }` with
the seed and current stdlib exits 1 with `module unknown-host.panack has no
binding gi409_unregistered_host_operation`, at line 1, column 16. The registry
and verifier source trace above explains why a native implementation alone would
not make that name callable from Panack.

## Options and completion

| Option | Benefit | Cost / limitation | Recommendation |
| --- | --- | --- | --- |
| Keep all arrangements unchanged | No migration or bootstrap disruption; source closure is already respected by stage-2 cache | Retains broad package-tool prerequisites and native identity invalidation | Keep source organisation, but investigate the demonstrated build overreach |
| Tighten dependency/build rules | Avoid rebuilding unrelated artifacts while retaining correctness checks | Must handle imports, core, VM/seed identity, corrupted caches, additions/deletions and uncertain graphs safely | Smallest justified follow-up; evaluate within GI#106's existing baseline and selection |
| Introduce a new compiler-support library | Could enforce a curated API if future evidence shows unstable shared interfaces | New interfaces, migration and bootstrap burden; does not remove intrinsic registries or VM coupling; adapters already exist | Not justified as an alpha.15 refactor by these findings |

The assessment accounts for the full loaded library closure and all five change
cases, separating measured results from inferred correctness requirements. It
makes no general claim about historical rebuild incidents, which may have used
different revisions, cold caches, changed VM identities or conservative fallback
paths. No unresolved source dependency remains in the assessed entry's closure.

Acceptance is this evidence-backed assessment and recommendation. Optimisations
and native-coupling changes remain separate work. No product code, compiler seed,
VM ABI, test policy or package release model changes. Informational documentation
validation is appropriate; broad product validation is not acceptance evidence
for a source/build assessment. The website's planned library-boundary assessment
claim remains accurate; no immediate website edit is required.

### Inventory comparison command

After generating `closure.txt` and `compiler.pinv` with the commands above, this
uses the existing container reader and validates snapshot contents as well as
path equality:

```sh
node <<'JS'
const fs = require('fs'), path = require('path');
const {Reader} = require('./scripts/source_coverage.cjs');
const r = new Reader(fs.readFileSync('build/gi409/compiler.pinv'));
r.magic('PANACKINVENTORY1\n');
if (r.u32() !== 1) throw Error('Expected one root');
const inv = new Reader(r.blob()); r.end();
inv.magic('PANACKINV1\nlocal-replay-v9\n');
if (!inv.blob().equals(fs.readFileSync('bootstrap/compiler-v9.bc')))
  throw Error('Compiler identity differs');
inv.text(); inv.blob();
const names = [];
for (let n = inv.count(256); n--; ) {
  const name = inv.text(), source = inv.text();
  const file = name.startsWith('stdlib/')
    ? path.resolve('src/stdlib', name.slice(7))
    : path.resolve('src/compiler', name.slice(8));
  if (source !== fs.readFileSync(file, 'utf8')) throw Error('Snapshot differs: ' + name);
  names.push(file);
  for (let count = inv.count(100000); count--; ) {
    inv.text(); inv.text(); inv.text();
    for (let i = 0; i < 6; i++) inv.u32();
  }
}
inv.end();
const expected = fs.readFileSync('build/gi409/closure.txt', 'utf8').trim().split('\n').sort();
if (JSON.stringify(expected) !== JSON.stringify(names.sort())) throw Error('Closure differs');
console.log('Matched source files: ' + names.length);
JS
```

The comparison is for this trusted locally generated assessment artifact, not a
new untrusted-sidecar reader or a production tool.
