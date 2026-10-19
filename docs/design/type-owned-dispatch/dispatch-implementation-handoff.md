# Type-owned dispatch: implementation handoff

[P06 owned intrinsic dispatch](p06-owned-intrinsics.md) records checked collection/text/numeric operations and owned Server/Map acceptance. [P07 callable and effect contracts](p07-callable-effects.md) adds owned callbacks and async method execution; [P08 source core methods](p08-core-methods.md) records the source/identity migration; P09 is next on acceptance.

[P05 built-in ownership registry](p05-builtin-owners.md) records canonical built-in source-method ownership and spoof rejection. P06 is recorded separately above.

[P04 generic nominal owners](p04-generic-owners.md) records generic implementation and method binders, expected-result inference and isolated core-enum acceptance. P05 is recorded separately above.

[P03 Server/Map coexistence](p03-server-map.md) records the representative implementation trial.

[P02 ownership metadata delivery](p02-owned-metadata.md) records the next implementation step and its remaining bridge.

[P01 inventory and compatibility audit](p01-inventory.md) records the checked source baseline, complete call-site register and corrections to the proposed API mapping.

Status: proposed design and execution contract, 17 October 2026. Read alongside [the migration plan](dispatch-migration-plan.md). This document makes implementation choices explicit; it does not claim they exist in the compiler. The owner approved PR#524 as the starting point; [GI#525: Type-owned dispatch migration](https://github.com/sproates/panackelty/issues/525) tracks the remaining work.

Companions: [acceptance fixtures](dispatch-acceptance-fixtures.md) and [stdlib declaration inventory](dispatch-stdlib-inventory.md).

## Start here

Work on one packet at a time. Complete its focused tests and required project checks, record evidence, and commit a buildable result before starting dependent work. Do not implement all packets in one uninterrupted change.

The inspected baseline is core commit `7aa31bcf99cdd9e4946718d13063f787e78662ce` on `feature/type-method-dispatch-poc`, associated with [PR#524](https://github.com/sproates/panackelty/pull/524). Verify the current checkout rather than assuming it still matches. Read CONTRIBUTING.md first. Preserve unrelated changes. Start migration work from current `next` after PR#524 is merged; verify that the PoC and these documents are present. Follow the applicable branch and publication instructions for subsequent work.

The source paths below are relative to the core `panackelty/` checkout. This handoff and its companion documents are versioned in `docs/design/type-owned-dispatch/` on the PoC branch.

If a baseline has moved, report relevant differences and reconcile this document before changing code. Do not restore an older checkout or overwrite current work to make the instructions match.

## Decisions for implementation

These are proposed decisions for this migration. Follow them unless a concrete incompatibility is found. Record that incompatibility and request a decision rather than silently inventing another language feature.

### D1: Owner and lookup identity

A nominal owner is its original `DeclarationIdentity`, including package, module and declaration name. A built-in owner is its canonical type-family tag. Callable owners additionally include their effect tag. Never use an import alias or rendered type string as the semantic key.

Use one method per owner and source method name. Do not add overload resolution by argument count or type. Distinct owners may use the same name with completely different signatures. Reject duplicate methods across multiple implementation blocks, even when signatures differ. Multiple blocks for the same owner are allowed only in its defining module.

Lookup is independent of ordinary function lookup. An unrelated global declaration named `get`, `put`, `apple` or `call` cannot satisfy a method call.

### D2: Proposed declaration syntax

Retain existing function modifiers and type-parameter syntax inside implementation blocks. Use explicit implementation binders:

```panack
impl Fruit { pub pure apple(): Nat { self.value } }
impl[T] Box[T] { pub pure value(): T { self.item } }
impl[T] Option[T] { pub pure value_or(fallback: T): T { /* existing body */ } }
impl[T, E] Result[T, E] { /* methods */ }
impl[T] [T] { /* array methods */ }
impl[K, V] Map[K, V] { /* map methods */ }
impl[T] Set[T] { /* set methods */ }
impl Str { /* text methods */ }
```

These generic forms are proposed syntax, not runnable baseline examples. Method binders use the existing `method[U](...)` form. Implementation binders and method binders must be disjoint; duplicate names are errors. Require every implementation binder to appear in the owner type. The receiver is implicit `self`; reject an explicit parameter named `self` and assignment to `self`. Existing persistent-value behaviour remains unchanged.

Allow owner declarations anywhere in the same module; resolve ownership after collecting declarations rather than enforcing textual order in the parser. Reject implementations for imported nominal types. Reject specialised implementations such as `impl Map[Str, Nat]`; generic owners have one uniform method set. No trait bounds, extension-method import rules or inheritance are introduced.

### D3: Generic inference and effects

First bind the receiver once. Match its canonical type family or nominal identity and obtain implementation-binder substitutions from its type arguments. Then instantiate the method signature and solve method type parameters from explicit type arguments, arguments and expected result using the existing inference machinery.

Implementation parameters come from the receiver; an explicit call type-argument list supplies only method parameters. Never confuse the two lists. If the receiver remains a hole, defer selection until existing constraint solving resolves it; if still unresolved, report insufficient receiver-type information. Never guess an owner from the method name.

Preserve existing purity, guarded-value, numeric conversion and async rules. Async methods use existing async function syntax and await behaviour. The ordinary method layer must not redefine what `AsyncFn` means. Inspect its existing signature representation before binding callable methods; it does not necessarily use the same trailing-result convention as synchronous callables.

### D4: Built-in ownership and visibility

Put built-in receiver declarations in the toolchain core module initially. This includes primitive families, arrays, Map, Set, Bytes, and built-in opaque Path, Duration and Instant types. Option and Result already have core nominal declarations and their implementations belong there too.

The loader may load dependencies of core, but keep the graph acyclic. If moving a method requires an error type currently declared in a wrapper module, move that type's canonical declaration to core and re-export it from the old module. Do not create a second nominal copy. Test identity through both routes. Audit compatibility before changing exported identity; do not assume equal spelling proves compatibility.

Only the actual toolchain package's designated core module can define implementations for built-in owners. A user module named `core.panack` does not gain this right. Add a negative test with a spoofed path/package.

Methods are private by default and use `pub` for external access. Public methods must expose only accessible signature types, including implicit self. User code imports a type, not each method. Preserve original owner identity through selective aliases and re-exports.

True transparent aliases resolve to their canonical target before lookup. Nominal guarded types retain their own identity and do not silently inherit base methods in this migration. Packet P01 must inventory existing receiver usage on guarded types; add explicitly owned methods where required, reusing existing checked conversions in their bodies. Do not bypass guards or add inheritance as a shortcut.

### D5: Internal primitives

Use a closed internal implementation tag attached to a checked owned declaration, for example source body versus a typed intrinsic ID. The exact enum variant names may follow local conventions. Do not introduce user-facing intrinsic syntax or expose VM operations as ordinary global functions.

Core loading can create compiler-provided method declarations for operations that have no source body. These declarations still carry owner, name, signature, visibility and effect, and pass through the same lookup and checking as source methods. Identify their provenance through the toolchain package identity, not a filename or method name alone.

Reuse existing VM instructions and verified runtime semantics. Any special lowering for collection callbacks or callable invocation must occur only after resolving its declaration. Retain no dispatch branch equivalent to “if method name is get, use Map rules”.

### D6: Functions that remain

Keep constructors, `main`, explicit module services, and ordinary compiler algorithms with no established receiver role. Remove the ability to call any such function through receiver syntax. Keeping ordinary functions is not retaining global receiver fallback.

Retain namespace service APIs for filesystem, process, environment, host I/O, networking and testing unless their receiver is a type defined by that module and the inventory identifies a genuine receiver API. In particular, `environment.get(name)` remains a namespace function; `server.get(...)` and `map.get(...)` are separately owned methods.

No new static-method syntax, factory objects or universal service receiver is needed. Compiler algorithms already invoked only as explicit functions can remain functions. All existing receiver-sugar calls must become owned calls or explicit namespace/function calls with a documented classification. Do not hide an intended receiver API behind a new free-function wrapper merely to avoid implementing ownership.

### D7: Dot disambiguation and callable fields

After legacy removal, a lexical local binding shadows a namespace alias. A namespace receiver selects a namespace member; a value receiver selects an owned method. Failed namespace lookup does not fall back to a value method or global function, and failed value-method lookup does not fall back to a namespace.

`value.field` is field access. `value.name(...)` is method lookup, even if a callable field is also named `name`. Invoke callable fields explicitly through `(value.field).call(...)`. Test this distinction; do not auto-invoke the field as a fallback. `@function` remains an ordinary function reference. Bound-method references are outside this migration; replace any affected old `@receiver_api` use with a named adapter calling the method, preserving its declared effect.

## Compiler data flow

The following contract is required; record/enum names are suggestions rather than an instruction to duplicate existing abstractions.

| Layer | Required change | Existing locations |
| --- | --- | --- |
| Parsing | Explicit owned-call node with receiver, name, arguments, type arguments and spans; explicit impl metadata with binders/owner/method declarations | `parser.panack`, `lexer.panack` |
| Declaration collection | Separate method index keyed by canonical owner and method name; collect before body binding | `module_bindings.panack`, `module_resolution.panack` |
| Signature checking | Owner and method binders remain distinct; retain visibility, effects, result and implementation tag | `module_signatures.panack` |
| Body binding | Bind receiver once, select method, instantiate signature, check arguments/effects, record selected identity | `module_bodies.panack` |
| Emission | Emit source-owned direct call or selected intrinsic; never repeat method lookup by spelling | `module_emission.panack`, `emitter.panack` |
| Compatibility frontend | Route owned syntax through project-aware semantics even for a source-file CLI entry | driver/frontend routing and `checker.panack` |

The PoC currently manufactures `$owned$...` declarations and `$owned_call$...` calls. During P02 these may remain an internal emission bridge, but method identity and lookup must move into explicit metadata. Remove the strings after all consumers use the new representation. Do not expose them through source lookup, imports or diagnostics.

Do not globally replace `DeclarationIdentity` with a string or reuse `SignatureHole` to encode an owner. Current `SignatureTypeIdentity` already distinguishes core, nominal, parameter, callable and hole identities; extend that model consistently.

## API migration decisions

Preserve existing signatures, effects and return behaviour after removing the receiver parameter. Keep existing receiver method names wherever possible. The following establishes the default mapping; P01 expands it to every declaration/call site before broad edits.

| Current surface | Final owner and method | Notes |
| --- | --- | --- |
| `len(x)` and `x.len()` | Str.len, Array[T].len, Bytes.length | Bytes calls converge on length; no universal name switch |
| array append/concat/first/sort_by/map/reduce | array owner, same method names | Preserve concat element-type joins, callback effects and generic results; array slice is not a current API |
| text slice/starts_with/starts_with_at/reverse/ends_with and character predicates | Str, same receiver method names | Preserve code-point semantics; concat is an array API |
| `nat_from_str(s)`, `s.parse_nat()` | Str.parse_nat | Preserve existing result/error contract |
| numeric receiver conversions `.nat()` / `.dec()` | Rat.nat / Rat.dec | Preserve existing guarded-value conversions; no new numeric-family methods |
| `map_put`, `map_get`, `map_has` | Map.put/get/has | Preserve current missing-key behaviour; do not invent Option returns |
| `set_add`, `set_has` | Set.add/has | Persistent updates |
| `byte_append`, `bytes_concat`, `byte_len`, `byte_get` | Bytes.push/join/length/at | Use stdlib wrapper names; migrate old receiver spellings too |
| `utf8_encode`, text.encode_utf8 | Str.encode_utf8 | Remove old receiver/global aliases |
| `utf8_decode`, text.decode_utf8 | Bytes.decode_utf8 | Keep its current decoding contract |
| text.reversed | Str.reverse | Consolidate with existing reverse operation |
| option.value_or and result.value_or | respective owned value_or | Distinct owner identities |
| path.join/to_text/native_bytes/display/is_absolute/directory/filename | Path, same names | Keep path factories/current as module functions |
| time.ticks/add/subtract/scale/is_before/as_seconds/divide/ratio | Duration, same names | Preserve checked arithmetic and errors |
| time.add_instant/is_before_instant/difference | Instant.add/is_before/difference | Owners remove naming ambiguity |
| numeric.divide | Nat.divide | Preserve current quotient behaviour |
| callable `.call(...)` | callable owner selected by effect/signature | Compiler-provided family declaration, no new variadic source syntax |
| bytes.empty, map(), set(), path/time factories | ordinary constructors/module factories | No implicit receiver |
| host/filesystem/process/environment/network services | explicit module APIs | Migrate their internal receiver calls; no foreign-type impls |

Do not silently unify differently behaving decoding APIs. `host.decode_utf8` returns a checked host result and remains an explicit module service. The inventory must distinguish it from the existing core decoding contract.

For source compiler methods, preserve the original method spelling used at the call site unless two candidates conflict on one owner. Such a conflict is a design decision, not permission to add overloading. Internal helper functions called explicitly remain legitimate functions; first-parameter shape alone is not evidence they are methods.

## Ordered implementation packets

All packets inherit the invariants and tests above. A dependency must be accepted before its dependent packet begins. A packet can be split into smaller buildable PRs, but its gate cannot be skipped.

| Packet | Depends on | Scope and files | Required result |
| --- | --- | --- | --- |
| P01 Inventory and compatibility audit | baseline | All `.panack` sources, resolver built-ins, stdlib declarations, dot-call resolution, API docs | Complete mapping register, retained-function reasons, guarded/callback/async cases and conflicts; no product edits |
| P02 Owned metadata for existing records | P01 | parser, bindings, signatures, body binding, emission | Existing Fruit/import tests pass using explicit ownership; no broad generic additions |
| P03 Server/Map collision fixture | P02 | new functional project and binding tests | Server get/put use owned dispatch while current Map implementation coexists; expected output below |
| P04 Generic nominal owners | P02 | parser and signature/binding pipeline | Box, Option, Result; method binders, imports, private methods, duplicate diagnostics |
| P05 Built-in ownership registry | P04 | core loading, signatures, body binding | Designated toolchain ownership, Map/Set/array/Str/Bytes metadata; spoofing rejected |
| P06 Owned intrinsic dispatch | P05 | checker, body binding, emission, core declarations | Collection/text/numeric operations through owned arrow calls; P03 upgraded to owned Map calls |
| P07 Callbacks and effects | P04, P06 | callable signatures, map/reduce/call lowering, purity/async tests | Higher-order and async use follows existing effect rules; required HTTP handler signature tested |
| P08 Source core methods and identity moves | P06, P07 and a pre-conversion seed refresh | core, option/result, path/time and affected exports | First save a seed supporting all new declaration syntax while core still uses old syntax; then migrate source methods and verify error-type identity |
| P09 Arrow-capable seed checkpoint | P08 | compiler seed and digest, bootstrap evidence | Old syntax compiler/stdlib sources rebuild with expanded support, before migration uses it |
| P10 Stdlib callers and API cleanup preparation | P09 | stdlib and conformance fixtures | Migrated receiver APIs; exact temporary-wrapper list |
| P11 Compiler and remaining callers | P10 | compiler by dependency, tests/examples/tools | All receiver usage migrated; named callback adapters where needed; checks pass |
| P12 Legacy semantic removal | P11 | parser/resolver/checker/emitter, globals/wrappers | Legacy paths deleted, old receiver dots rejected, namespace dots valid |
| P13 Owned-dot bridge and seed | P12 | parser/routing, seed/digest | Arrow and owned-dot use the same resolver; old sugar remains absent |
| P14 Dot source conversion and arrow removal | P13 | maintained sources, parser, docs | Sources converted, bootstrap passes, arrow rejected |
| P15 Release/consumer verification | P14 | packaging/browser/docs/website follow-up records | Matching toolchain artefacts and examples, final deletion audit and acceptance |

P03 must not implement networking. P06 must not change collection runtime behaviour to accommodate Server. P12 must not retain fallbacks solely because a test used them; migrate a positive test or turn it into an explicit rejection test while preserving its original behavioural coverage.

## Bootstrap and validation runbook

Commands below run from the core repository root and were checked against the current Makefile and scripts. They are instructions for implementation, not commands executed while writing this handoff.

Initial evidence:

```sh
git status --short
git branch --show-current
git rev-parse HEAD
cat VERSION
```

After implementing compiler support while sources still use seed-compatible syntax:

```sh
make bootstrap
PANACK_PROBE_SEED=build/bootstrap/stage2/compiler.bc sh tests/run_probe.sh tests/runner/compiler_parser_unit.panack
PANACK_PROBE_SEED=build/bootstrap/stage2/compiler.bc sh tests/run_probe.sh tests/runner/compiler_module_bindings_unit.panack
```

Add focused purity/checker probes when those behaviours change. The helper sets the repository bootstrap root for compilation and unsets it for test execution. Do not globally export `PANACKELTY_BOOTSTRAP_ROOT` while running tests: nested fixtures must retain their own project identities.

At the start of P08, before any core/stdlib source adopts new declaration syntax, and at P09/P13 before their respective caller conversions:

```sh
make regenerate-seed
make check
git diff --check
```

Review `bootstrap/compiler-v9.bc` and `bootstrap/compiler-v9.bc.sha256` together. The current refresh script builds stages 2, 3 and 4, compares compiler bytecode, compiles and executes stdlib conformance at every stage, and compares expected output. It never uses cached bootstrap artefacts. Preserve its checks and record input/output seed hashes in the packet evidence.

Critical ordering: the seed used to start refresh must already understand the syntax of every compiler/core source it loads. Save the generic-impl-capable seed before core/stdlib declarations migrate in P08. Save the arrow-capable seed before arrow caller migration. Save the owned-dot-capable seed before dot source migration. If a change accidentally crosses that boundary, stop and restore only the packet's source conversion to the last committed checkpoint; do not replace the seed with an unaudited candidate or weaken refresh checks.

After conversion, run `make regenerate-seed` as needed to establish the new source fixed point, then canonical `make check`. Full checks are required for completed product changes. Historical timing budgets are informational under the current contributor validation policy; failures must be diagnosed by actual test results.

Public CLI fixture checks, after adding P03's project:

```sh
./bin/panack run --project tests/functional/cases/owned_server_map/panack.json
./bin/panack compile --project tests/functional/cases/owned_server_map/panack.json -o /tmp/panack-owned-server-map.bc
./bin/panack run /tmp/panack-owned-server-map.bc
```

Compare stdout exactly with the fixture's `expected.stdout`, require success exit status and no unexpected stderr. Verify current CLI help if interface changes before implementation. Remove only the temporary bytecode created by this task after checking it. Also exercise source-file entry through the project-aware compiler path. A rejection by the legacy parser is not evidence of a language limitation.

Adding functional fixtures affects maintained case lists and summary assertions. Inspect `tests/runner/main.panack`, `tests/functional/cases/runner_smoke/main.panack` and `tests/unit/harness/runner.sh`. Update expected counts from the actual discovered fixtures and assertions, not by copying stale totals from this document.

## Evidence, stop conditions and handoff

For each packet record: baseline commit, changed files, implemented decisions, tests and exit statuses, exact expected/actual behaviour, seed hashes if relevant, remaining compatibility entries and next eligible packet. Report implementation, committed, published and merged states separately.

Stop dependent work and seek a focused decision when:

- An ownership collision cannot be resolved under the single-method rule.
- A required refined-type or callable behaviour contradicts the proposed rules.
- Core dependencies would become cyclic after the specified identity moves.
- An HTTP-required callback signature needs a capability outside the current effect model.
- The fixed point fails or the seed cannot parse the sources required to refresh it.
- Removing a global also removes a legitimate non-receiver API not classified by P01.

Do not skip tests, invent a new language feature, silently preserve legacy fallback or implement HTTP networking to get past these conditions. Continue independent investigation where useful and report the specific decision needed.

The API inventory and the representative packet are intentional gates. Use P01 through P03 to verify the design and execution process before proceeding with the remaining packets.

## Copyable first-task prompt

> Read the dispatch migration plan and its three companion Markdown files supplied with this task. Follow CONTRIBUTING.md and the repository validation requirements. Implement only P01 initially: inspect the current baseline, produce the complete semantic API/call-site inventory and resolve it against the proposed ownership rules. Do not implement the migration, alter CI policy or implement HTTP networking in P01. Verify the given stdlib declaration inventory against the checkout. Report exact conflicts, missing capabilities and any decision needed; do not silently change the design. Record the next eligible packet and provide the completed register as a reviewable artefact. After P01 is accepted, P02 and P03 are the representative implementation trial before broader assignment.

For later packets, supply the same documents plus the latest inventory and evidence, name the one authorised packet, and require its dependency and exit gates. A new implementation task must not assume that completion of a prompt authorises the entire programme or a merge.
