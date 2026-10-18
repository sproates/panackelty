# P01: dispatch inventory and compatibility audit

[GI#525: Type-owned dispatch migration](https://github.com/sproates/panackelty/issues/525)

Status: P01 inventory complete, prepared for integration. This is a documentation and evidence delivery; P02-P15 remain unimplemented. Baseline: `5327c2b6789e47801a03e663b007fd2a6f9da0c9`, the merged PoC on `next`. The remote `next` baseline was checked before branching. The original 82-declaration inventory was verified against this tree.

## Result

Every tracked Panack source under compiler/stdlib, packages, tools, examples and tests is accounted for. The real parser and project-aware body binder provide source spans, namespace/value classification, targets and type evidence. Package manifests and original declaration identities are retained, including the two packages containing a `service.panack` module. Regex was not used to decide method dispatch.

| Evidence | Count | Interpretation |
| --- | ---: | --- |
| Tracked Panack source files | 615 | Complete specified source corpus |
| Successfully parsed files | 605 | AST traversal performed |
| Intentionally malformed lexer/parser fixtures | 10 | Individually recorded with rejection diagnostics |
| Unique parsed call/reference sites | 24,125 | Includes direct calls, namespace calls, receiver calls and named references |
| Sites matching bound body nodes | 24,099 | Selection evidence, not necessarily successful calls in negative fixtures |
| Guard calls resolved from local declarations | 2 | Guard expressions are checked outside function-body traversal |
| Rejected calls / pruned child calls in negative fixtures | 24 | Explicitly classified, no owner invented |
| Legacy receiver calls | 1,915 | Includes attempted calls in negative tests |
| Namespace calls | 4,585 | Distinct from receiver sugar |
| Owned PoC calls | 35 | All in tests; no production migration claimed |
| Named references | 72 | Included for callback-adapter migration |
| Unbound dot nodes in rejection fixtures | 4 | No checked call target retained |
| Remaining direct calls | 17,514 | Includes constructors and invalid-call fixtures |
| Built-in spellings with dispositions | 100 | Complete resolver.is_builtin list |
| Standard-library source functions | 82 | Existing signatures and proposed destinations verified |
| Quoted/generated/documented source fragments | 318 | Separately parsed and classified; no claim of executing dynamic generators |

All compiler, stdlib, package, tool and example call sites have checked targets. The 26 non-body-bound sites are all test inputs and are explained individually. The typed ledger does not infer legal APIs from erroneous receivers in negative tests.

## Deliverables

- [API register](p01-api-register.md): every built-in spelling, retained-service rationale, source-method mapping, signature rules and deletion packet.
- [Call-site index](p01-call-sites.md): complete source ledger split into compiler/stdlib, packages, tools, examples and tests, with individual exceptions.
- [Generated/documented source register](p01-source-templates.md): generator/example locations, method spellings, parser status and fragment digest.
- [Reproduction procedure and probe](p01-reproduction.md): the temporary real-compiler audit tools and integrity checks.
- [Stdlib source inventory](dispatch-stdlib-inventory.md): all 82 declarations and their existing signatures.

The ledger assigns each caller a disposition and delivery/removal packet. Broad source migration remains P10/P11 and legacy deletion remains P12. Temporary migration wrappers do not exist yet; the old global names and source wrappers in the register are the exact surfaces to retire when their callers have moved.

## Findings and decisions

### Correct the proposed API domains

The original handoff overgeneralised two operations. Current checked dispatch supports array concat and text slice; it does not establish text concat or array slice. Both entries have been corrected rather than expanding the language. `len` also accepts Bytes, whose calls converge on Bytes.length under the existing canonical byte-method naming decision.

Rat owns the existing nat/dec conversions. Do not create methods on all numeric types simply because an erroneous fixture retains a core conversion target. Current guarded-value checks still apply.

### Preserve inference when ownership changes

Array.concat computes an element-type join through `body_join`, including compatible Nat/Int inputs. Its intrinsic declaration needs that result solver; an ordinary fixed `[T] -> [T]` signature would narrow current behaviour. This fits D5's declaration-associated intrinsic implementation and does not require source-level dependent types or overloading.

An Array/Map/Set with hole type arguments still has a known owner family. Bind that owner and keep its argument constraints; do not infer the owner from the method spelling or reject all empty containers.

Explicit generic arguments on old core source functions may describe the owner. In particular, empty `first[Str]()` and `sort_by[Nat](...)` callers need typed receiver bindings when those parameters move to an impl. The API register identifies their exact source sites and embedded fixture counterparts. Apply the same rule to explicit type arguments on namespace Option/Result wrappers: owner arguments move to receiver construction/annotations, not to the method's own binder list.

### Preserve service boundaries

`environment.get` remains an explicit namespace function. Str-based compiler path/file adapters remain explicit module functions with their existing trapping behaviour. They do not become foreign Str implementations. Public low-level aliases used only to back such wrappers are retired after callers migrate; necessary host primitives become internal declaration-associated operations. Explicit printing/input and constructors remain deliberate services/constructors without receiver fallback.

No method overload or foreign-implementation exception is needed to resolve the current corpus. Source functions with no receiver role remain ordinary functions, as D6 already specifies.

### Move nominal identities with their implementations

Moving Path/time receiver implementations into core also requires PathError, DurationError and ClockError identity migration. The compiler currently hard-codes their defining modules in `body_intrinsic_type`, and constructs PathError directly in `body_check_path_core_call`. P08 must update these identity producers, preserve re-exports and test pattern/signature identity through both routes. Merely moving the source enum would be incomplete.

HostError, FileMetadata, FileKind and ProcessOutput stay with retained service APIs. Do not move unrelated types into implicit core or expose every new core declaration as an unqualified user global.

### Cover every visitor when replacing PoC markers

P02's explicit owned-call metadata affects exhaustive Expr visitors in parser, checker, emitter, module_bodies, module_emission, module_resolution, purity, resolver and source_inventory. P12's marker removal also reaches module_effects and explanations. Lexer/parser changes alone do not remove legacy semantics. Update source maps, source inventories and explanation evidence alongside normal compilation.

### Migrate deliberate free-function receiver examples

These are the exact current non-core free-function receiver sites. Their method-like spelling does not give the target ownership. Keep explicit function coverage while removing sugar; preserve the old missing-method or purity rejection where that is the test's purpose.

| Source | Current receiver call | Migration disposition |
| --- | --- | --- |
| `examples/generic_functions.panack:16` | `identity` | Use the explicit ordinary function call in positive examples/tests; add or retain a separate rejection for old receiver sugar. Do not implement the built-in owner from this module. |
| `tests/fixtures/compiler_checker/checks_method_calls_as_receiver_first_calls-1.panack:6` | `total` | Move the locally owned Pair operation into impl Pair for positive owned-dispatch coverage; keep legacy lowering rejection separately. |
| `tests/fixtures/compiler_contracts/emitter/emits_method_calls_as_receiver_first_calls-1.panack:4` | `increment` | Use the explicit ordinary function call in positive examples/tests; add or retain a separate rejection for old receiver sugar. Do not implement the built-in owner from this module. |
| `tests/fixtures/compiler_contracts/generic_functions/inference_explicit_arguments_and_lexical_type_scope-1.panack:14` | `identity` | Use the explicit ordinary function call in positive examples/tests; add or retain a separate rejection for old receiver sugar. Do not implement the built-in owner from this module. |
| `tests/fixtures/compiler_contracts/syntax/method_calls_lower_to_calls_with_the_receiver_first-1.panack:10` | `total` | Move the locally owned Pair operation into impl Pair for positive owned-dispatch coverage; keep legacy lowering rejection separately. |
| `tests/fixtures/compiler_purity/rejects_calls_to_impure_user_functions-2.panack:3` | `output` | Preserve the impure-function rejection with an explicit call; separately reject unsupported receiver sugar. |
| `tests/functional/cases/core_methods/main.panack:18` | `twice` | Use the explicit ordinary function call in positive examples/tests; add or retain a separate rejection for old receiver sugar. Do not implement the built-in owner from this module. |
| `tests/functional/cases/owned_imports/src/main.panack:37` | `apple` | Keep ordinary global calls explicit; owned calls must select the impl; replace the old dot-sugar assertion with rejection at P12 and owned-dot evidence at P14. |
| `tests/functional/cases/owned_imports/src/main.panack:38` | `banana` | Keep ordinary global calls explicit; owned calls must select the impl; replace the old dot-sugar assertion with rejection at P12 and owned-dot evidence at P14. |
| `tests/functional/cases/owned_method_dispatch/main.panack:36` | `apple` | Keep ordinary global calls explicit; owned calls must select the impl; replace the old dot-sugar assertion with rejection at P12 and owned-dot evidence at P14. |
| `tests/functional/cases/owned_method_dispatch/main.panack:38` | `impl` | Keep ordinary global calls explicit; owned calls must select the impl; replace the old dot-sugar assertion with rejection at P12 and owned-dot evidence at P14. |

### HTTP readiness remains a later acceptance gate

P03's numeric Server fixture must coexist with Map.get/put; it is not an HTTP implementation. P07 must exercise GI#518's actual handler types: `AsyncFn[Request,ResponseWriter,Result[Unit,Str]]` and `AsyncFn[T,Request,ResponseWriter,Result[Unit,Str]]` for ContextualServer[T], with Unit-returning registration. The current inventory does not prove those future owned signatures execute. There is no reason to change Map's signature or missing-key semantics to enable Server.get/put.

## Remaining decisions and next packet

No unresolved owner collision or required scope extension blocks P02. The corrections above preserve the agreed migration rules and current checked behaviour. The existing stop conditions still apply if implementation reveals new evidence, especially during core error-type moves or async handler integration.

P02 is the next eligible packet after P01 acceptance: introduce explicit ownership metadata for the existing nongeneric record PoC, preserve its current tests and import identities, and update all visitors. Do not jump to generic/built-in migration or HTTP networking. P03 then provides the small server/Map coexistence test.

The source-template register is intentionally finite evidence of checked-in generators and examples. It does not enumerate every runtime string a generator can produce. Migrate the generator and its expected diagnostics/source spans together and use its existing test suite to verify expanded output. Fragments and negative templates are obligations, not silently accepted programs.

## Validation and limitations

The temporary probe compiled with the audited compiler seed. Parser and binding passes exited successfully; package-aware and corrected-root passes completed with empty VM stderr. Loader diagnostics from malformed or semantically invalid tests were retained and reviewed. The known temporary namespace parser gate was not mistaken for a language limitation.

Self-checks verified distinct identities for a namespace call, an arbitrary free-function receiver call, an imported owned call and a Map.get call. Ledger uniqueness and coverage checks require one row for every parsed span, no unaccounted production/example sites, all 100 built-ins and all 82 stdlib declarations. Plan documentation and local links are validated with `make docs`; `git diff --check` covers whitespace. No product source, seed, runtime or executable test changed. Full make check is not claimed for this documentation-only delivery, and no proposed P03 fixture is claimed to have run.

Performance impact: none at runtime. The inventory probe is temporary tooling recorded as documentation, not a new compiler pass or build dependency. Its repeated module loads are audit work, not a proposed production design.

Website impact: no change to released syntax or public runtime claims; this is an unreleased migration inventory. The final syntax cutover's website follow-up remains P15.

## Programme status

The existing eight-stage weight baseline is unchanged. On acceptance of this P01 delivery, stage 1 earns 5 percentage points; no implementation stage receives credit for planning or for the already merged PoC.

| Stage | Weight | Completion on P01 acceptance | Earned contribution |
| --- | ---: | ---: | ---: |
| Inventory and ownership (P01) | 5% | 100% | 5 points |
| General method model | 20% | 0% | 0 points |
| Built-in owned implementations | 20% | 0% | 0 points |
| Stdlib migration | 15% | 0% | 0 points |
| Compiler and source migration | 20% | 0% | 0 points |
| Legacy removal | 10% | 0% | 0 points |
| Dot cutover | 5% | 0% | 0 points |
| Release and ecosystem acceptance | 5% | 0% | 0 points |
| Total | 100% | | 5 points |

Packet status: P01 delivered by this documentation change; P02, P03, P04, P05, P06, P07, P08, P09, P10, P11, P12, P13, P14 and P15 have not started. GI#525 stays open. This status takes effect when the delivery is accepted and integrated, not merely because a draft exists.
