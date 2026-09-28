# Developer and AI-agent adoption: architectural assessment

Assessment date: 2026-09-29. Source baseline: `cf834bf68a26c7d43a1db58b2e95a5cd430bcbf6`
(PR #84). Work record: [issue #85](https://github.com/sproates/panackelty/issues/85).
[ROADMAP.md](../ROADMAP.md) owns current state and priority; this is supporting
evidence and a proposal awaiting user review, not an implementation commitment.

## Conclusion and scope

Panackelty has a credible compiler/runtime foundation, but it is not yet a broadly
connected application platform. The largest verified blockers are host/platform
integration, application lifecycle/concurrency, and reusable modules/packages.
Developer and agent assistance can deliver smaller improvements without waiting
for all of those foundations. No evidence yet establishes agent preference,
comparative productivity, mainstream adoption or performance superiority.

Keep the VM, exact numerics and purity boundary as current design constraints.
A VM hosted by another platform is consistent with them; replacing the VM with
a new execution engine would require a separately agreed architectural change.
Do not equate general-purpose ambition with building every toolkit from scratch.

This assessment combines source/specification review, four small native CLI
probes, repository metadata and existing validation evidence. It is not a
security audit, load test, cross-platform prototype or completed AI benchmark.
Absence claims concern this repository and its documented public interfaces.
Efforts are relative: **S** is a bounded PR/investigation, **M** crosses several
components, **L** is likely a sequence of PRs. Estimates include relevant tests,
docs, integration and maintenance; discovery effort is separate from delivery.

## Existing strengths worth preserving

- Self-hosted compiler and byte-identical bootstrap evidence, exact numerics,
  checked purity, tagged unions, records, generics and named callable references:
  [SPEC.md](../SPEC.md), [SELF_HOSTING.md](../SELF_HOSTING.md).
- Verified versioned bytecode, explicit VM ownership contracts, native sanitizers,
  failure injection, compiler probes and public-CLI tests; public C coverage exists:
  [test guide](../tests/README.md), [value model](../src/vm/VALUE_MODEL.md).
  Missing `.panack` measurement does not mean missing `.panack` tests.
- Typed filesystem/process services, bounded subprocess execution, exact time
  values and explicit host errors: [host ABI](../src/runtime/ABI.md) and
  [standard library](../src/stdlib/README.md).
- Installable preview archives and exact-artifact gates for Linux x86-64 and
  macOS arm64, with a documented preview compatibility policy:
  [RELEASE_POLICY.md](../RELEASE_POLICY.md). Broader distribution is a gap;
  release discipline itself is not absent.

## Evidence register

| ID | Finding and classification | Evidence and implication |
| --- | --- | --- |
| E1 | Host extension/embedding interface: verified omission | [builtins.c](../src/vm/builtins.c) has a fixed registry; [vm.h](../src/vm/vm.h) exposes an internal invocation struct and synchronous `execute`, not a documented embedder API. [host.c](../src/vm/host.c) calls native `exit` for `process_exit`. Existing host services are useful but cannot be assumed safe to expose unchanged inside a GUI or browser host. |
| E2 | Concurrency/lifecycle: deliberate limit plus architectural gap | [SPEC effects](../SPEC.md#effects-and-purity) permit named callable references without captured state; concurrency is postponed. [execute.c](../src/vm/execute.c) uses native recursive calls and a run-to-completion instruction loop. [value model](../src/vm/VALUE_MODEL.md#ownership-and-reclamation) uses non-atomic counts for a single-threaded invocation. No public suspension, cancellation or signal-handler API was found. Internal subprocess `poll` is not language-level async support. |
| E3 | Namespace/package composition: verified limitation | [SPEC modules](../SPEC.md#modules), [loader](../src/compiler/loader.panack) and stdlib README describe one combined declaration namespace. Visibility, selective imports, third-party packages and configurable roots are pending. Two imported files declaring `answer` fail with a duplicate declaration. |
| E4 | Agent/compiler assistance: mixed strengths and omissions | [Diagnostic](../src/compiler/types.panack) carries message/position; [driver](../src/compiler/driver.panack) renders text for check/compile/run/disasm. Caret locations, inference and missing-match-case feedback exist. Stable diagnostic codes, machine-readable diagnostics and type/effect queries are absent from the documented CLI. The typo/unused probes below demonstrate bounded opportunities, not proof of every possible diagnostic deficiency. |
| E5 | Runtime observability and source measurement: verified gap | [emitter FunctionCode](../src/compiler/emitter.panack) has no instruction-to-source map; [bytecode format](../src/bytecode/FORMAT.md) is strict/versioned. The bounds-trap probe lacks a source location or stack. `.panack` coverage remains unmeasured. Mapping may serve both, but tracing and coverage collection have different acceptance criteria. |
| E6 | Application libraries/platforms: verified repository omissions | The [stdlib inventory](../src/stdlib/README.md) and builtin registry contain no networking/HTTP/TLS, database, graphics/audio, GUI or JavaScript interop API. No browser/mobile build target or supported Windows release is documented. JSON files in test fixtures do not constitute a JSON application library. External-process delegation is possible but is not equivalent to integrated platform support. |
| E7 | Long-running resource behavior: needs investigation | Bytecode decode limits and process time/output limits exist. The VM invocation has no visible execution-fuel or allocation-budget field. Native recursive calls, whole-file operations and fatal memory exhaustion merit bounded stress/embedding review. Purity and bytecode verification do not restrict an effectful program's host authority. No exploit or newly proven memory-safety defect is claimed. |
| E8 | Scaling/numerics: evidence of costs, unknown application impact | [collection builtins](../src/vm/builtins_collections.c) scan maps/sets linearly and rebuild map entries on update. Persistent array append already has an optimisation. [SPEC numerics](../SPEC.md#values-and-numeric-semantics) are exact, without binary floats or rounding modes. Larger datasets and graphics/audio conversions need measurement/design; these facts do not establish unacceptable game or server performance. |
| E9 | Onboarding/tooling: partial foundation | Tour/examples, source/install instructions and test libraries exist. No repository implementation of a formatter, LSP, debugger, package resolver or project-init command was found. CLI/API discoverability and a complete small application tutorial can be assessed before a full IDE toolchain. |
| E10 | Maintainability/documentation: observed drift | The compiler loader nests path/loading/error work; checker/parser combine substantial responsibilities. This warrants focused review, not a rewrite based on size. SPEC's promise/Euler-first selection text predates the broader ambition; SECURITY's public-repository wording is stale; parts of [coverage matrix](../tests/COVERAGE.md) retain old test descriptions. PR #84 established conventions and planning ownership. |
| E11 | Repository security/tooling: partial inventory | Read-only GitHub metadata on the assessment date shows a public repo, Issues/Projects enabled and main protected with `test` and Linux/macOS package checks. Ruleset list is empty; classic protection exists. `security_and_analysis` was not returned; full protection administration is unavailable to this integration. Hosted scanner/private-reporting configuration is unknown, not proven disabled. No dedicated scanner/update workflow is present among the tracked workflows. |
| E12 | Adoption/differentiation: untested hypothesis | The repo demonstrates self-hosting and examples, not independent developer retention or agent preference. Reliable AI-assisted delivery and useful compiler assistance are candidate advantages. No agent comparison or free-choice study was run in this assessment. |

### CLI probe reproduction

After `make`, create each source in a temporary directory and run the indicated
command with `./panack`. These are diagnostic observations, not benchmark trials.
All four were rerun against the baseline native VM and bundled compiler.

| Probe | Source | Command and observed result |
| --- | --- | --- |
| Typo | `main(): Void { greeting = "hello"; print(greting) }` | `check`: exit 1, `unknown name greting` plus file/line/caret; no `greeting` suggestion |
| Unused binding | `main(): Void { unused = 42; print("hello") }` | `check`: exit 0, `ok`; no unused-binding guidance |
| Runtime location | `pure pick(values: [Nat], index: Nat): Nat { values[index] }` followed by `main(): Void { print(pick([1], 9)) }` | `run`: exit 1, `error: VM trap: index is out of bounds`; no file/line/stack |
| Composition | Import `one.panack` containing `pure answer(): Nat { 1 }` and `two.panack` containing `pure answer(): Nat { 2 }`; main prints `answer()` | `check`: exit 1, `error: top-level name answer is already declared` |

## Application paths and prerequisite decisions

| Use | Useful foundation today | What prevents a comfortable end-to-end workflow |
| --- | --- | --- |
| Scripts/data tools | CLI, files, bytes, process API, exact arithmetic | Discoverability, reusable dependencies, data formats, robust errors and distribution |
| Daemons/services | Executable VM, filesystem, process/time APIs | Graceful shutdown, bounded long-running execution, service observability; networking and concurrency for responsive servers |
| Web/application servers | Records/unions and deterministic core logic | HTTP/TLS/networking, async/backpressure/cancellation, storage integrations and operational lifecycle |
| Desktop GUI | Portable core and named callbacks | Toolkit bridge, event dispatch, callback state/ownership, host-controlled lifetime and application packaging |
| Games | Pure logic, collections and exact values | Graphics/audio/input bridge, frame pacing, performance evidence and explicit numeric conversions |
| Browser | C VM and portable bytecode are potential starting points | Host-service replacement, JS/DOM bindings, async events, resource controls and browser build/debugging workflow |
| Mobile | Potential embedded core logic | Platform/toolkit bridge, lifecycle/suspension, packaging/toolchain and device validation; not just another CPU build |

Three shared decisions have high enabling value:

1. **Host-controlled execution:** embedder lifetime, typed service registration,
   failure/exit behavior, permitted host capabilities and callback ownership.
   Start by designing/prototyping one bounded host call, not an unrestricted FFI.
2. **Execution/event model:** evaluate cooperative VM suspension or host-driven
   invocations before adopting threads. Define re-entry, cancellation, resource
   cleanup and data sharing; do not make reference counts atomic and call it done.
   Closure support is a candidate convenience, not automatically necessary for
   a first callback API (explicit state is an alternative).
3. **Composition model:** namespaces/exports before standard-library migration;
   dependency identity and reproducibility before a package registry. A local
   vendored dependency experiment can precede hosted package infrastructure.

A browser VM feasibility study should compare required host adaptations and
artifact/debugging costs without selecting a backend here. GUI toolkit, mobile
framework and networking library choices need their own supported-platform,
licensing, security and maintenance review. No outside technology is endorsed
by this repository-only assessment.

## Candidate comparison — proposed, not scheduled implementation

Confidence below concerns the problem evidence; delivery estimates remain provisional.
Delay risk is developer friction or architectural rework unless otherwise stated.

| Candidate / evidence | Value and smallest useful outcome with acceptance | Effort, dependencies, risk and maintenance |
| --- | --- | --- |
| Agent delivery pilot (E12) | Establish where agents fail, then compare fixes; reproducible runs and a maintenance task, including unsuccessful results | S–M; task/budget review first; high confidence in evidence gap, unknown benefit. Maintain versioned briefs/results rather than a broad benchmark service. |
| Structured diagnostics (E4) | Let tools identify errors reliably: stable codes, file/range and machine-readable output for a bounded diagnostic family, preserving human output | M; no source map needed for frontend errors. High confidence; schema compatibility and escaping/Unicode need tests. Machine-readable output does not require shipping a full JSON parser library. |
| Name suggestions / API discovery (E4,E9) | One small resolver suggestion slice, or a discoverable API query; tests for irrelevant/ambiguous names and imported code | S suggestions, M queries; select using pilot failures. High confidence in omission, usefulness unmeasured. False suggestions and ongoing ranking rules are the main costs. |
| Host/embedding contract (E1,E2,E7) | Design plus bounded native harness: invoke core logic twice, return failures/exits to host, demonstrate owned host-call values and cleanup | S–M discovery, L full support; high enabling value for server/GUI/browser/mobile. Medium feasibility confidence; ABI, re-entry and security authority need explicit contracts. |
| Event/cancellation model (E2,E7) | Compare host-driven callbacks and resumable VM execution with explicit state; show cancellation/cleanup in a bounded prototype before platform integration | M discovery, L delivery; depends on host ownership decisions. High confidence in gap, medium solution confidence. Scheduler, shutdown and race semantics create enduring complexity. |
| Namespaces/exports (E3) | Two libraries may use the same private helper; defined imports, qualification, visibility and failures | M design, L delivery including bootstrap. High confidence; prerequisite for stdlib migration and scalable dependencies. Compatibility and diagnostics matter more than syntax alone. |
| Dependency/project workflow (E3,E9) | One reproducible local third-party dependency with explicit identity/root and useful missing/conflicting-version errors | M discovery, L resolver/registry; depends on module design. Medium confidence in chosen scope; supply-chain policy and version support add maintenance. Defer hosted registry. |
| Source mapping, runtime diagnosis and coverage (E5) | First decide sidecar vs versioned metadata; validate cross-file trap locations and mapping stability. Then separate PRs for stack diagnostics and trustworthy coverage denominators/aggregation | S–M design, L combined delivery; high confidence. Preserve deterministic bytecode and bootstrap, never silently change v8. Metadata/debug data lifetime and report accuracy add maintenance. |
| Application library slice (E6) | A complete data tool may justify JSON; a service milestone may justify networking. Specify malformed input, resource bounds, errors and full application acceptance | M JSON estimate, L network/TLS/service stack; scope confidence low until pilot. Prefer reviewed library bridges where suitable; third-party security updates remain project work. |
| Resource/authority contract (E7) | Document trusted-code boundaries and specify bounded invocation/cancellation/allocation behavior for an embedding or service proof | S assessment, M–L implementation; high confidence in unbounded dimensions, no proven exploit. Must precede claims of safe untrusted execution; test cleanup and nested runs. |
| Workload performance (E8) | Measure scaling at named input sizes, compile/run time, memory and collection/numeric hotspots; optimise only demonstrated bottlenecks | S profiling, M–L fixes; no arbitrary throughput promise. Keep exact semantics; approximate numerics would need explicit types/conversions and separate approval. |
| Human onboarding/formatter/editor (E9) | One complete small app guide plus discoverable commands; test it with an unfamiliar developer. Formatter/editor work follows observed friction | S guide, M formatter, L mature editor tooling. Medium value confidence; syntax evolution and editor integration create maintenance. Avoid making LSP a prerequisite for CLI assistance. |
| Focused readability and test evidence (E10,E5) | Audit one compiler responsibility, reconcile its real tests, then behavior-preserving refactor with bootstrap and relevant CLI cases | S audit, M component changes. High confidence in documentation drift; refactor need must be demonstrated. Source coverage informs risk but need not block every well-tested small change. |
| Repository settings/security docs (E11,E10) | Finish an access-aware inventory, verify private-reporting instructions and propose a minimal set of applicable checks with triage ownership | S assessment; actual configuration effort unknown. No settings changes here. Do not equate a C scanner with analysis of custom language semantics; account for alert noise/CI cost. |

## Proposed ordering and trade-offs

**Now:** review this assessment, agree pilot tasks/budget and reconcile the small
set of stale product/security/test statements in scoped follow-up work. The
assessment issue stays open until the report and completion summary are merged.
This report records the mismatch instead of silently changing language contracts.

**Next:** run the bounded pilot. Use its failures to choose one main delivery
initiative; structured diagnostics plus a narrowly justified assistance slice is
the leading small candidate, not an already approved bundle. A host/embedding
contract investigation is the leading enabling candidate. It can be scheduled
after the pilot without committing to a GUI, browser or server implementation.
If unsupported integration dominates results, give that foundation priority over
polishing name suggestions. Security defects discovered meanwhile take precedence
according to actual risk.

**Later:** namespaces then stdlib organisation/dependency workflow; source mapping
and its separate diagnostics/coverage outcomes; event integration and one chosen
platform/application milestone. Relative order depends on pilot and design evidence.
Keep application performance, readability and test quality visible throughout;
none is satisfied by a coverage percentage alone.

**Defer with triggers:** REPL until interactive exploration repeatedly blocks users;
more CI optimisation until measured feedback delay justifies it; full LSP/package
registry until basic project workflows create demand; broad game/mobile/browser
backends until a host contract and representative app validate cost. This defers
implementation, not the general-purpose ambition. Avoid simultaneously building
all targets or unrelated differentiators from the older roadmap.

## Proposed experiment for issue #86 — not run

[Issue #86](https://github.com/sproates/panackelty/issues/86) tracks the subsequent
experiment. Agent competence and agent choice are different questions.

### Delivery arm

Propose three small briefs, Panackelty versus Python initially, two independent
runs per language/task (12 runs). A later TypeScript/Node.js comparison follows
only if this pilot is informative. Use the same model version, tool permissions,
clean fixture, documentation access and acceptance harness per pair; disclose
training familiarity rather than claiming equal familiarity. Give Panackelty a
concise language/library entry point without task-specific solutions.

1. **File inventory CLI:** list immediate files in a supplied fixture, emit a
   deterministic summary and report inaccessible inputs. Follow-up: filter by
   suffix. Acceptance includes spaces, Unicode, empty input, byte counts and
   predictable exit/output behavior; stay within the current filesystem surface.
2. **Exact ledger summary:** parse a specified simple delimited format (explicitly
   not full CSV), aggregate amounts by account, reject malformed data and apply
   a stated rounding rule only at the reporting boundary. Follow-up: add refunds.
   Exact rational expectations prevent a benchmark that silently excuses numeric
   errors in either language; do not tune the task to guarantee a Panackelty win.
3. **Small JSON HTTP service:** read/write an in-memory record with validated
   input and specified error responses. Follow-up: a query filter. This probes a
   currently unsupported application path; missing JSON/networking must count as
   a blocked task, not be excluded. Do not build a runtime or delegate the whole
   service to another language within the trial to hide that result.

Proposed per-run cap: 15 minutes including the maintenance change, 12,000 model
tokens and 40 tool calls; stop at the first cap. Maximum model-token envelope is
144,000 plus evaluation, at most three aggregate agent-hours. These are proposed
bounds, not cost quotes or an instruction to launch concurrent agents. Confirm
available execution tooling, price/cost ceiling and the final budget before runs.
Use one model initially; label two repeats exploratory, not statistically decisive.

Predefine public requirements and withheld edge-case acceptance tests. Record
initial and changed-task pass rates, setup failures, repairs, intervention,
elapsed time and cost where observable; separate input/output tokens if exposed.
Review naming, structure and change readability using the same rubric. Archive
briefs, toolchain/model versions, commands, dependency versions, outputs and patches.
Score completion quality first; compare speed/cost only alongside correctness.
Use temporary test data and bounded local services; do not expose test servers or
provide access to unrelated user files. Do not add Python to repository CI.

### Choice and adoption arm

In separate fresh sessions, offer the same task with no mandated language, list
available toolchains symmetrically and record choice plus stated reasons. Choice
under an explicit Panackelty instruction is compliance, not adoption. Familiarity,
installation effort and missing dependencies may dominate; report those reasons.
Scope and budget this arm separately after the delivery pilot, rather than
expanding the initial 12 runs automatically.

Neither arm substitutes for independent human use. A subsequent volunteer should
install a release, finish and modify a small app without maintainer intervention;
observe blockers and willingness to return. Do not equate agent preference on
three tasks with mainstream language adoption.

## Review decisions requested

- Accept or amend the proposed pilot briefs and resource ceiling.
- Confirm the proposed small delivery candidate and enabling investigation remain
  conditional on evidence, rather than declaring a feature-first roadmap now.
- Select the first complete application milestone after the pilot. Broad platform
  ambition remains intact; no toolkit, backend or concurrency model is chosen here.
