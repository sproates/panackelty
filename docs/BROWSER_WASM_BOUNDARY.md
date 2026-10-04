# Browser/WASM repository boundary investigation

This is the historical investigation snapshot from before repository separation.
For the resulting ownership and delivery contract, see
[architecture](../ARCHITECTURE.md#browser-ownership-and-website-delivery) and the
[roadmap](../ROADMAP.md#browserwasm-boundary-investigation--2026-10-01).

## Decision

**Recommendation: split the browser playground into a separate repository, but not by copying the current source-tree integration unchanged. Perform one small core-boundary preparation first, then migrate.**

The browser product is operationally and conceptually downstream of Panackelty core, but its current build consumes private repository layout and development snapshots. A direct split today would replace monorepo coupling with fragile cross-repository source copying. Keeping it permanently in core would preserve unnecessary browser tooling and CI on core changes.

This recommendation deliberately tests the user's preferred split against the evidence rather than assuming it.

## Evidence from current implementation

The playground is optional: its own README states that the native CLI, compiler seed, bytecode v9 and canonical `make check` do not require WASI or browser tooling.

The browser build nevertheless reaches deeply into the core checkout:

- `src/playground/build.mjs` compiles nearly every `src/vm/*.c` file directly with WASI SDK Clang, replacing only native `host_capabilities.c` with the browser adapter.
- It copies `bootstrap/compiler-v9.bc` directly.
- It serializes every `src/stdlib/*.panack` source into the browser artifact.
- The website repository assembles its pages with a pinned browser release and runs the browser integration suite.
- Browser runtime compatibility therefore currently follows source-tree shape and the exact development VM implementation, not only the documented bytecode contract.

The bytecode contract is substantially stronger than this coupling suggests. `src/bytecode/FORMAT.md` explicitly versions executable encoding/semantics and requires a version increment for incompatible changes. However, additive native intrinsics can be added within v9 and older runtimes may reject them. A downstream browser runtime therefore needs an explicit core release/compatibility identity in addition to the numeric bytecode version.

## Measured CI cost

Recent hosted job timing shows the original diagnosis needs refinement.

For PR #150 (run 36785380904):
- WASI SDK installation: ~5 s.
- locked JavaScript dependency setup: ~2 s.
- deterministic Wasm build: ~4 s.
- native comparison/contracts: ~5 s.
- Playwright browser-engine installation: **~4 m 38 s**.
- actual Chromium/Firefox/WebKit tests: **~1 m 41 s**.
- browser job total after setup: ~6 m 42 s.

A nearby run (36781512371) spent **~7 m 51 s** installing browser engines and ~1 m 41 s testing. Thus the expensive repeated provisioning is predominantly Playwright browser installation, not WASI SDK acquisition.

PR #150 changed only process documentation yet both Pages and Playground browser jobs ran for about 6–7 minutes because the current conservative `full` route reaches browser consumers. This is real feedback latency even though browser correctness was unaffected.

## Alternatives considered

### 1. Keep browser/WASM permanently in core

Advantages:
- atomic changes across VM/compiler/stdlib/browser;
- no cross-repository version coordination;
- browser tests immediately expose changes that affect the browser profile.

Disadvantages:
- browser-specific Node/Playwright/WASI workflows remain part of core repository operation;
- conservative core changes can trigger multiple real-browser suites;
- the core repository owns website application code and browser deployment concerns;
- the current integration makes source-tree layout part of the browser build interface.

This is the lowest migration risk but the weakest responsibility boundary.

### 2. Split immediately using current source-tree inputs

Advantages:
- removes browser CI from routine core PRs quickly;
- browser deployment and dependencies get clear ownership.

Disadvantages:
- the new repository would need to clone/pin Panackelty source and know private paths such as `src/vm`, `bootstrap/compiler-v9.bc` and `src/stdlib`;
- compatibility would effectively be “works with this commit SHA”, not a supported dependency contract;
- changes to internal VM file layout could break the downstream build despite no bytecode contract change.

**Rejected.** This hides coupling rather than removing it.

### 3. Establish a small consumable core boundary, then split

Advantages:
- browser repository can depend on an intentional versioned core artifact/interface;
- core CI can validate that artifact without installing browsers;
- browser CI owns browser engines, UI and deployment;
- compatibility updates become deliberate rather than every-core-commit events;
- periodic/release-triggered downstream checks can detect drift without blocking every core PR.

Cost:
- requires a bounded preparation step and migration;
- cross-repository changes need explicit compatibility/version management.

**Recommended.**

## Required core boundary before migration

Do not create a new stable C embedding ABI merely for this split. The smallest useful dependency should package the inputs the browser already needs while keeping their provenance explicit:

1. a versioned browser-runtime source or build-input bundle for the VM implementation and public headers needed by the WASI build, excluding native-only host adapters;
2. the matching compiler seed artifact;
3. the matching standard-library sources;
4. bytecode version plus a **core compatibility/release identity** that changes when additive runtime intrinsics or browser-relevant semantics change;
5. checksums/provenance and the existing bytecode contract.

The exact packaging mechanism should reuse release/build infrastructure where possible. It need not promise that VM C internals are a public third-party ABI; it is a versioned build input consumed as a unit.

Core tests should prove the bundle is complete and internally matched. They should not require Playwright.

## Proposed ownership after split

### Core repository

Own:
- language/specification and bytecode contract;
- compiler and bootstrap seed;
- native/reference VM implementation;
- standard library;
- versioned browser-runtime dependency bundle and compatibility metadata;
- conformance fixtures sufficient for downstream consumers;
- release publication of those inputs.

Do not routinely own:
- Playwright/browser installation;
- browser UI/worker/controller;
- browser-specific host adapter;
- browser deployment/Pages workflow.

### Browser repository

Own:
- WASI compilation of the pinned core runtime bundle;
- browser host capability adapter;
- JavaScript WASI shim;
- worker/runtime/controller/UI and examples;
- Playwright tests across supported engines;
- browser/playground deployment.

Pin an immutable core release/artifact and checksum. Update deliberately.

## Cross-repository validation model

Core changes continue to run native/unit/functional/bootstrap and bytecode-contract tests. A core release or explicit browser-compatibility candidate can trigger or request a downstream compatibility run, but routine unrelated core PRs do not install browsers.

The browser repository runs its complete browser suite when its own code or pinned core dependency changes. A scheduled compatibility check may test the latest core development revision as an early-warning signal; failures there are advisory until the browser dependency is deliberately updated.

A bytecode-version change, compatibility-identity change, compiler-seed/stdlib release or browser-relevant VM change should be visible in the dependency update. The browser repository must not silently track `main`.

## Website consideration

The playground currently shares the Panackelty website and GitHub Pages artifact. Repository separation does not require moving the entire website. Prefer a deployment boundary in which the browser repository publishes a versioned playground artifact that the website deployment consumes, or hosts the playground independently under a stable route/domain arrangement. The migration must preserve `/playground/` URLs or explicitly redirect them.

Coverage publication is a separate core concern and should not become coupled to browser builds.

## Migration sequence and estimate

### PR 1 — core dependency boundary (small–medium)
Create and test the versioned browser-runtime input bundle/manifest and document compatibility semantics. No browser-engine installation is required to prove bundle completeness.

### Separate repository migration (medium, likely 1–2 PRs across repositories)
Create the browser repository, consume the pinned bundle, move browser-specific source/tests/workflows, and prove equivalent browser behavior. Establish deployment integration.

### Core cleanup PR (small)
Remove migrated browser source/workflows/dependencies from core, retain website/coverage publication in their independent repositories, update docs and CI routing.

Provisionally **2–4 reviewed PRs total across the two repositories after this investigation**, depending on whether repository bootstrap and deployment migration are combined.

## Interim optimisation

Because migration is not instantaneous, cache or preinstall Playwright browser binaries and ensure change-aware routing avoids browser jobs for genuinely unrelated changes where safely identifiable. Do not invest in elaborate WASI caching: measured WASI setup is only seconds.

Any interim routing change must remain conservative for VM, compiler seed, stdlib, playground, shared website assets and workflow changes until the split is complete.

## Decision tests

The recommendation would change to “retain in core” if:
- atomic compiler/VM/browser changes are frequent enough that versioned dependency updates become the dominant maintenance cost;
- a complete downstream input cannot be packaged without exposing unstable internal implementation in an unmanageable way; or
- deployment constraints require browser code to remain inseparable from core publication.

Current evidence does not establish any of those conditions. The existing bytecode contract, independent VM audit, optional browser toolchain and small explicit browser host adapter all support a downstream-consumer model.

## Conclusion

The evidence supports the user's architectural instinct **with one qualification**: split the browser/WASM product from Panackelty core, but first turn its current implicit source-tree dependencies into one explicit, versioned core dependency. This yields a real component boundary rather than a second repository that still depends on core internals by path and commit SHA.
