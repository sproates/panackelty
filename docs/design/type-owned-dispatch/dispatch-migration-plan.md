# Type-owned dispatch migration plan

Current packet evidence: [P01 inventory](p01-inventory.md) and
[P02 explicit ownership metadata](p02-owned-metadata.md), and
[P03 Server/Map coexistence](p03-server-map.md). The tables below
preserve the original proposed baseline; the delivery reports record earned progress.

[P01 inventory and compatibility audit](p01-inventory.md) records the checked source baseline, complete call-site register and corrections to the proposed API mapping.

Status: proposed implementation sequence, updated 17 October 2026.

## Implementation handoff pack

Read this overview first, then use these companion files for execution:

1. [Implementation decisions, task packets and bootstrap runbook](dispatch-implementation-handoff.md).
2. [Concrete acceptance fixtures and expected results](dispatch-acceptance-fixtures.md).
3. [Standard-library declaration inventory](dispatch-stdlib-inventory.md).

The implementation handoff supplies proposed decisions where this overview previously left choices open. The fixture sources are specifications, not tests claimed to have run. P01 is an explicit bounded discovery gate for the complete semantic call-site inventory; it must finish before broad migration. Use P01 through P03 as the representative implementation trial before proceeding with the remaining programme.


Starting evidence: [PR#524: Type-owned method dispatch PoC](https://github.com/sproates/panackelty/pull/524). The owner approved merging the PoC into `next` on 17 October 2026 as the migration starting point. [GI#525: Type-owned dispatch migration](https://github.com/sproates/panackelty/issues/525) tracks the follow-on work. Merging the PoC does not implement or complete this migration.

## Required outcome

All receiver calls resolve through methods owned by the receiver's type. Migrate the standard library, compiler and maintained callers using arrow syntax, remove legacy receiver sugar and its supporting global APIs, then replace arrow syntax with dot syntax.

This must enable the HTTP server work to use `server.get(...)` and `server.put(...)` without interference from `Map.get` or `Map.put`. These names must have no compiler-wide collection meaning. A server method and a map method may have different parameters, return types and effects, and each must be checked against its own declaration.

The dispatch migration supplies that language capability. It does not implement the HTTP server, choose its route-registration API, or redesign its runtime.

## Scope and invariants

- Select a method using the receiver's resolved type and the method name, with generic substitution where applicable.
- Preserve original owner identity through imports, aliases and re-exports. Importing another module must not change which method wins.
- Pass the actual receiver as `self`, evaluating it once and preserving evaluation order.
- Enforce the selected method's visibility, argument types, return type, purity and asynchronous contract.
- Reject missing methods without looking for a same-name global function.
- Remove global receiver APIs, compatibility aliases and compiler machinery used by the current sugar.
- Retain legitimate constructors, entry points and operations without a receiver. Inventory these explicitly rather than mechanically converting every first parameter into `self`.
- Retain necessary VM primitives internally. Associate primitive lowering with resolved declarations, never with an arbitrary source method spelling.
- Preserve field access and namespace-qualified calls throughout the migration.
- Keep traits, inheritance, arbitrary foreign-type extensions and runtime dynamic dispatch outside this programme unless evidence establishes a separate requirement.

Direct-call code generation with `self` as an argument remains valid. The language guarantee comes from resolving an owned method before emission, rather than finding an unrelated global function.

## HTTP server acceptance contract

Introduce a small server-shaped fixture early, alongside collections. It is a language regression fixture, not a networking implementation. Use signatures representative of the HTTP work once those signatures are established; do not invent a production server API to satisfy the test.

During migration, exercise `server->get(...)`, `server->put(...)`, `routes->get(...)` and `routes->put(...)` in the same project. After the final cutover, run the same scenarios with dot syntax.

The fixture must establish all of the following:

1. An imported server type owns `get` and `put`; neither method needs a separate import.
2. Two server instances receive their own `self` values.
3. Server and Map calls coexist, including a server implementation that uses a Map internally.
4. Server signatures can differ from Map signatures. Wrong server arguments produce a server-method diagnostic, without applying Map key/value rules.
5. Map lookup and persistent update semantics remain unchanged.
6. Same-name global functions and methods on unrelated types cannot redirect either call.
7. Namespace aliases, selectively renamed imports and re-exports retain the original server owner.
8. A type lacking `get` or `put` receives a missing-method error, even when globals with those names exist.
9. An effectful receiver expression is evaluated once. Method effects and callback effects are checked from the selected declaration.
10. Any asynchronous handler signature required by the HTTP work is covered before claiming that specific integration is ready.

The HTTP work can use arrow calls once its required signatures and this fixture pass. Its final `server.get` and `server.put` spelling depends on the dot cutover. Do not add an early server-specific dot exception.

## Stage 1: Inventory APIs and decide ownership

Create a register of legacy declarations and call sites across compiler, stdlib, tests, examples, tools and packaged sources. Record the replacement owner, signature, effects, implementation mechanism, callers and removal condition for each API.

The current compiler has several distinct mechanisms:

| Mechanism | Examples | Replacement |
| --- | --- | --- |
| Dot-to-global receiver fallback | `value.operation()` becomes `operation(value)` | Lookup on the receiver type |
| Special method-name mappings | `get`, `put`, `has`, `add` | Ordinary owned declarations |
| Compiler-known global receiver operations | `len`, `append`, `map_get`, `set_add` | Owned methods with source or intrinsic implementations |
| Special core source identities | `core_first`, `core_ends_with`, `core_sort_by` | Normal owned method declarations |
| Special higher-order lowering | `call`, `map`, `reduce` | Owned signatures with declaration-associated lowering |

Initial code locations include `src/compiler/parser.panack`, `module_resolution.panack`, `module_bodies.panack`, `checker.panack`, `resolver.panack`, `module_emission.panack` and `emitter.panack`. Source implementations begin in `src/stdlib/core.panack` and the individual stdlib modules.

Classify functions by their meaning. Operations on a parser state have a plausible owner; constructors and operations combining independent inputs may remain module functions. Resolve ownership for built-in and opaque types before migrating their APIs.

**Exit gate:** every legacy receiver API and exception has a replacement and removal condition. Every retained global API has an explicit reason. HTTP server collision tests are specified.

## Stage 2: Generalise the method model

The PoC supports nongeneric records. Extend it to the type families actually required by the inventory:

- Generic records and enums, including `Option[T]` and `Result[T, E]`.
- Arrays, Maps and Sets with their existing inference rules.
- Primitive and opaque built-in types.
- Generic methods, including methods whose result element type differs from the receiver's.
- Pure, impure and asynchronous methods where required.
- Callable values, preserving their effect distinctions.
- Aliases and refined types, with explicit method-identity rules.

Promote method ownership into explicit compiler metadata rather than indefinitely extending the PoC's synthetic-name encoding. Define generic implementation syntax and substitution rules in the specification before relying on them in library code.

Keep user implementations with their defining type's module. Give built-in types designated core or stdlib ownership. Reject duplicate or conflicting implementations deterministically. Method availability must not depend on import order.

Add the server-shaped regression fixture at this stage. Prove that its `get` and `put` are independent of Map methods before broad migration begins.

**Exit gate:** the required type families, imports, visibility, effects and generic cases work through arrow dispatch, with positive and negative tests. State any unsupported HTTP signature explicitly.

## Stage 3: Provide owned built-in implementations

Introduce owned signatures for text, numeric, byte, collection and callable operations. Attach necessary opcode or specialised lowering to the selected declaration's identity.

Map owns its `get` and `put` signatures. The compiler must not decide that any method named `get` means map lookup. The same principle applies to `has`, `add`, `map`, `reduce` and `call`.

Move source implementations such as `first`, `ends_with` and `sort_by` into ordinary owned declarations. Preserve collection persistence, missing-key results, bounds behaviour, exact numeric semantics, callback purity and evaluation order.

Temporary legacy paths may coexist to keep current callers buildable, but every one must be registered for removal. New arrow calls must never depend on those fallback paths.

**Exit gate:** replacement methods pass behavioural tests, including server and Map calls in one project and deliberately conflicting global names.

## Stage 4: Migrate the standard library

Use dependency order:

1. Core text, numeric, byte and collection operations.
2. Option and Result.
3. Path and time.
4. Filesystem, process, environment and asynchronous APIs.
5. Testing helpers and service-support code.

For each module, move receiver operations to their agreed owners, convert internal calls to arrow dispatch, update imports and exports, and update examples and tests. Preserve errors and effect contracts.

Do not create artificial receiver objects merely to remove a function. In particular, a filesystem operation accepting a Path does not automatically belong to Path. Apply the ownership decisions from stage 1.

Where unmigrated compiler code needs an old exported function temporarily, record that wrapper and its removal dependency. Do not leave compatibility exports as an undocumented permanent surface.

**Exit gate:** stdlib receiver APIs and internal receiver calls use owned dispatch. Remaining compatibility exports are exhaustively listed.

## Stage 5: Migrate the compiler and maintained executable sources

Follow the compiler's actual dependency graph, starting with shared data structures and utilities, then lexer/parser, module resolution/signatures, checking/body binding, emission and entry points. Migrate runners, probes, examples and tools as their dependencies become available.

Use resolved information for mechanical edits. A text replacement cannot reliably distinguish namespace calls, field access, receiver calls and global functions with the same name.

Bootstrap transition:

1. Validate a compiler that understands the expanded owned-method model while compiler sources still use syntax the current seed accepts.
2. Refresh the audited seed through the existing process.
3. Migrate compiler sources to arrow dispatch in buildable slices.
4. Rebuild the self-hosting stages and verify the required fixed point.

Each product slice must pass the canonical checks. Do not leave a checkout that requires an unavailable intermediate compiler to rebuild.

**Exit gate:** compiler, stdlib and maintained executable sources use owned dispatch for receiver operations. A clean checkout can rebuild them from the checked-in seed.

## Stage 6: Remove legacy dispatch while arrow remains

Remove dot-to-global fallback, obsolete global receiver APIs, compatibility wrappers, parser name mappings, legacy `$method_*` dispatch markers, special core-function remapping, and obsolete built-in global signatures and reserved-name rules.

Remove checker, resolver and emitter branches that exist solely for the old paths. Update imports and exports and remove dead implementations. Necessary runtime operations remain behind the new declarations.

Audit both project-aware and legacy frontend routes. Share the new semantics or explicitly retire an obsolete route; no entry point may silently retain receiver sugar.

Add rejection tests for removed global receiver APIs and old dot receiver calls. Keep namespace-qualified calls and field access working. Retain historical examples only when clearly labelled as history or negative fixtures.

Repeat the HTTP fixture with Map and server methods together. Confirm that no spelling-based `get` or `put` dispatch remains in parser, checking, resolution or emission.

**Exit gate:** the project builds and passes with legacy machinery physically removed. Working receiver calls still use arrow syntax. A same-name global function cannot rescue a missing method.

## Stage 7: Change owned calls from arrow to dot

Only begin once stage 6 passes. Define the final forms:

| Form | Meaning |
| --- | --- |
| `value.method(...)` | Type-owned method dispatch |
| `module.function(...)` | Namespace-qualified function call |
| `value.field` | Field access |

None may fall back to unrelated globals. Specify callable-field invocation explicitly. Test namespace shadowing, chains, fields used as receivers and imported types.

Second bootstrap transition:

1. Build a bridge compiler accepting arrow and owned-dot calls through the same method resolver, with legacy sugar already absent.
2. Refresh the seed.
3. Convert maintained sources, fixtures and examples to dot.
4. Rebuild and verify self-hosting.
5. Remove arrow-call support and temporary conversion machinery.

Run the complete HTTP acceptance contract with `server.get(...)`, `server.put(...)`, `routes.get(...)` and `routes.put(...)`. Do not restore collection-name exceptions to make any case pass.

**Exit gate:** dot is the sole receiver-call syntax, arrow calls are rejected, and a clean checkout rebuilds successfully.

## Stage 8: Release and ecosystem acceptance

Update the specification, compiler and stdlib READMEs, architecture and self-hosting documentation, quick starts, diagnostics, examples and release notes.

Check packaged compiler/stdlib sources, the browser distribution, website examples and other maintained consumers against the matching toolchain version. Record breaking compatibility requirements explicitly. Do not publish examples against a released compiler that cannot parse them.

Required verification includes:

- Original Fruit output: `10`, `11`, `20`, `21`.
- Imported and re-exported ownership, distinct instances and same-name methods.
- The HTTP server/Map acceptance contract above.
- Generic inference, visibility, effects, callbacks and evaluation order.
- Collection and numeric behaviour.
- Public CLI source execution, project builds and saved bytecode.
- Canonical `make check` and self-hosting fixed-point verification.
- A final source audit for obsolete dispatch branches, globals and wrappers.

**Exit gate:** the shipped language, maintained sources and documentation use one dispatch model. Actual HTTP server functionality remains separately tracked; language fixtures do not establish that a network server has been implemented.

## Delivery and progress baseline

Keep PR#524 focused. Deliver the migration in independently validated PRs; stages may need multiple PRs where type support or source migration is substantial. Do not combine syntax cutover with unresolved legacy removal.

| Stage | Provisional weight | Status | Accepted completion | Earned contribution |
| --- | ---: | --- | ---: | ---: |
| 1. Inventory and ownership | 5% | Proposed | 0% | 0 points |
| 2. General method model | 20% | Bounded PoC evidence only | 0% | 0 points |
| 3. Built-in owned implementations | 20% | Proposed | 0% | 0 points |
| 4. Stdlib migration | 15% | Proposed | 0% | 0 points |
| 5. Compiler and source migration | 20% | Proposed | 0% | 0 points |
| 6. Legacy removal | 10% | Proposed | 0% | 0 points |
| 7. Dot cutover | 5% | Proposed | 0% | 0 points |
| 8. Release and ecosystem acceptance | 5% | Proposed | 0% | 0 points |
| Total | 100% | Proposed programme | | 0 points |

Weights estimate relative implementation, integration and verification scope, not elapsed time. Keep them stable unless scope changes explicitly. Generic/built-in ownership and compiler migration carry the largest uncertainty. The HTTP collision requirement is included in these stages rather than counted again as a separate outcome.

The first follow-up should produce the API inventory and ownership decisions, with the server/Map regression fixture specified as an early gate. This establishes exactly which global surfaces disappear and prevents HTTP support being deferred until the end of the migration.
