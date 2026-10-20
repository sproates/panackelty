# Ordered task packets

All paths are repository-relative. P01–P15 are complete on the final acceptance
PR's merge. Their original work/exit criteria below remain the scope record;
the [acceptance register](acceptance-register.md) records delivered evidence.
Alpha.14 publication is separate from implementation acceptance.

P01–P02 are a feasibility gate. Internal prototypes must be labelled and removed
or integrated before their packet closes. No test stub may stand in for a required
runtime guarantee. For each packet, inspect the matching rows in the source map,
add tests to the existing native/package/functional harnesses, and update affected
documents. If a packet exceeds a reviewable change, split it with the same acceptance
IDs and keep the packet incomplete until all sub-outcomes are accepted.

The owner's 20 October execution update permits continuous work and merging
self-reviewed, exact-head green PRs into `next`. See the
[integration permission](implementation-plan.md#integration-permission-and-execution-scope).
Packets remain acceptance checkpoints; they need not each create a separate PR.

## P01: Contract, caller inventory and decision register

**Prerequisites:** None. **Weight:** 4%. **Estimate:** Small; 1 documentation PR.

### Work

Inspect the source map and live issue. List every HTTP server/service export, caller, test generator, packaged artifact, CLI option and shared client type. Record current compiler/seed revision. Resolve G3–G6 and conflicting G8 options with the owner before their dependent work. Extract issue Examples 1 and 3 verbatim into planned fixtures, without claiming they run.

### Exit gate

An inventory table gives each old API a replacement, consumer, test and removal packet. Every acceptance row has an owner packet and intended executable fixture. Each unresolved decision has exact competing interpretations and a blocked packet. No product edits.

**Acceptance:** A01–A03; all gates inventoried.

## P02: Typed handle and runtime feasibility trial

**Prerequisites:** P01. **Weight:** 6%. **Estimate:** Medium/high uncertainty; 1–2 PRs.

### Work

Inspect value retention, checked built-in owner registration, host waits, execution cancellation and TCP service ownership. Prove a package-owned Server and ContextualServer[T] can store multiple concretely typed named callbacks and preserve one context value without closures/type erasure. Prototype request-writer lifetime and readiness as separate from service lifetime. Design structured trap classification and cancellation cleanup. Record operation/verification and bootstrap implications.

### Exit gate

A reviewed representation/ownership table covers every retained value and released resource. Compile the direct registration shape, same-owner Map coexistence and two distinct contexts; reject wrong context/callback/effect types. A bounded native prototype proves aliasing, stale-handle rejection and request teardown. If this needs unagreed language scope, stop and ask; do not build the rest on an unverified abstraction.

**Acceptance:** A01–A03, A27; G1/G2 resolved.

## P03: Opaque server ownership and constructors

**Prerequisites:** P02. **Weight:** 8%. **Estimate:** Medium; 1–2 PRs.

### Work

Implement the agreed handle representation and all four constructors. Preserve ordinary/contextual callback signatures and context value semantics. Store port/address validation failures for listen. Implement Created state, sticky setup-error storage and safe alias retention. Include stale generation, invalid handle and allocation failure checks at native boundaries.

### Exit gate

Independent server instances do not share routes/context. Aliases refer to one handle. Invalid address/port construction does not bind or trap; listen reports ServerError. Wrong T or ordinary/contextual callback mixtures fail compilation. Native fault tests prove no double release or unsafe continuation.

**Acceptance:** A01–A03, A27.

## P04: Public Request value and header access

**Prerequisites:** P03. **Weight:** 4%. **Estimate:** Small/medium; 1 PR.

### Work

Introduce required immutable method/path/query/headers/body view. Keep parser/client nominal types compatible through explicit adapters where needed; inventory old Request(target) callers before changing shared types. Implement first/all case-insensitive header access preserving order. Keep raw escapes, repeated slashes, case and dot segments unchanged.

### Exit gate

Pure package tests cover empty query, absent path, percent escapes, repeated header names/case and exact Bytes. Client checks remain passing. Do not percent-decode or parse query pairs. No sockets needed for this packet.

**Acceptance:** A04–A05.

## P05: ResponseWriter validation and buffering

**Prerequisites:** P03. **Weight:** 8%. **Estimate:** Medium; 1–2 PRs.

### Work

Implement request-scoped writer ownership, status/header operations and send-once state. Validate header tokens/values and reserved framing fields; compute byte counts including generated headers/status line. Snapshot a valid complete response atomically. Add text/byte defaults, repeated fields and bodyless status rules. Define failure outputs using the request fault category from P02.

### Exit gate

Pure/native tests cover status 199/200/599/600, CR/LF injection, framing overrides, repeated Set-Cookie, replacement, valid first send, second send, mutation after send, no send, exact limits and stale writer use. A failed oversized send produces no partial bytes. Completed data remains available for the recovery packet.

**Acceptance:** A10–A14, A27.

## P06: Route registry and deterministic dispatch

**Prerequisites:** P03–P05. **Weight:** 6%. **Estimate:** Medium; 1 PR.

### Work

Implement get/post/put/delete/fallback for both handle forms, returning Unit. Validate full route URI reference, store only its raw path as matching key. Record invalid/duplicate registrations without replacing handlers. Dispatch supported/unsupported methods, path-specific Allow and fallback according to the decision table. Trap registration after listening begins without mutation.

### Exit gate

Package tests cover all methods, all duplicate/query/fragment cases, exact matching, canonical Allow ordering and fallback precedence. Both contextual handlers receive the same T. Same-name Map methods coexist. Do not introduce path parameters or public route tables.

**Acceptance:** A06–A09.

## P07: Request-head admission and bounded body framing

**Prerequisites:** P04, P06; G4/G5 resolved. **Weight:** 7%. **Estimate:** Medium; 1–2 PRs.

### Work

Separate complete-head validation and route decision from body consumption. Preserve HTTP/1.1 Host/framing rules. Enforce head/body byte bounds while reading, including chunked decoding and fragmented input. Reserve request capacity before waiting for admitted bodies; keep connection and request accounting separate. Integrate early final rejection without invoking handlers.

### Exit gate

Raw fixture covers 400/404/405/413/431/501/505, Content-Length and chunked success, unframed body boundary per owner decision, exact limits and arbitrary input fragmentation. Handler counts remain zero for rejections. Capacity is released on every parse/error/disconnect path.

**Acceptance:** A15–A17, A20.

## P08: Expect handshake and interim-response sequencing

**Prerequisites:** P07. **Weight:** 7%. **Estimate:** Medium; 1 PR.

### Work

Implement one trimmed case-insensitive 100-continue expectation. Head validation, routing and admission precede the interim response. Send it only when a body remains unread. Unsupported/multiple/comma-list expectations produce 417. Continue bounded chunk decoding after interim response and never invoke the handler early.

### Exit gate

A synchronised raw client waits for exact interim bytes before sending a body, then observes one final response. Cover every early final status without 100, already-buffered/no-body cases, chunk overflow after 100, split head/body arrivals and the shared deadline. Test both server forms.

**Acceptance:** A18–A19.

## P09: Per-request failure isolation and completion

**Prerequisites:** P02, P05, P07–P08. **Weight:** 12%. **Estimate:** Large; 1–2 PRs.

### Work

Implement the structured request fault boundary and generic 500 translation. Preserve completed sends on returned errors/traps; log diagnostics only to stderr. Release failed child resources without destroying sibling work. Retain fatal VM/host failure propagation. Integrate single final flush and disconnect handling.

### Exit gate

One trapped request is followed by a successful request in the same process. Exercise index/arithmetic traps, returned Error, invalid response, missing send, duplicate send and failure after send. Native fault injection confirms OOM/integrity failure is not recovered unsafely. No second final response, diagnostic leakage or stale completion.

**Acceptance:** A12–A14, A21, A27.

## P10: Readiness, deadlines and bounded admission

**Prerequisites:** P09; G3/G4/G6 resolved. **Weight:** 10%. **Estimate:** Large; 1–2 PRs.

### Work

Implement listen readiness and service-resource liveness after main returns. Enforce one ten-second accept-to-completion deadline. Separate four admitted requests from sixteen connections; reject excess according to clarified G4. Integrate no-send 504 and completed-send preservation under clarified G3. Add deterministic counters/fixtures for cancellation and slot release.

### Exit gate

Wire tests hold four handlers and separately sixteen incomplete connections, verify immediate 503/no queue/no handler, then show recovery. Listen resolves only after bind succeeds. Slow-head/body/handler/write cases use one deadline. Check no service dies merely because main returned.

**Acceptance:** A20, A22–A23.

## P11: Close, destroy and termination cleanup

**Prerequisites:** P10. **Weight:** 8%. **Estimate:** Medium/large; 1 PR.

### Work

Implement Created/Listening/Closing/Closed/Destroyed transitions, idempotence and destroy escalation. Stop admission first; drain at most one second, then cancel remaining work. Apply clarified normal-exit/signal semantics. Release sockets, retained context/callbacks, waits and registry slots once.

### Exit gate

Lifecycle matrix passes before/after listen, during active writes/handlers, repeated calls and close/destroy races. Port reuse proves release. Signal tests observe bounded cleanup and exit; forced kill is not claimed graceful. Cancellation tests detect late delivery, use-after-free and double release.

**Acceptance:** A23–A24, A27.

## P12: Static-preview composition

**Prerequisites:** P11; G8 resolved. **Weight:** 6%. **Estimate:** Medium; 1 PR.

### Work

Adapt static package context/handler to the public contextual writer API. The tool creates the explicit-address contextual server and registers the static handler as fallback. Preserve loopback default, --host, filesystem identity, confinement, MIME, redirects, index and compatible size/header policy. Put GET-only fallback 405 logic in static code.

### Exit gate

Serve index/nested assets, reject traversal, check MIME/redirect/size boundaries, POST/PUT/DELETE Allow: GET, explicit-route precedence and bind options. Inspect dependency graph: HTTP never imports static types or filesystem policy. Installed preview behaves the same at its documented surface except agreed changes.

**Acceptance:** A25.

## P13: Remove obsolete public server APIs

**Prerequisites:** P12. **Weight:** 4%. **Estimate:** Medium; 1 PR.

### Work

Migrate all P01 callers and generated test sources. Remove low-level server/service exports from the package manifest and documentation. Keep internal implementation only where needed and preserve separately supported TCP APIs and saved-artifact contracts. Audit Request/Response types shared with client before deleting anything.

### Exit gate

Negative public-import tests reject removed entry points. Generic package API works from an external project. HTTP client tests pass unchanged. Every P01 old export has removal evidence or an owner-approved retention reason; no temporary public wrapper remains by accident.

**Acceptance:** A26–A27.

## P14: Public guide and executable examples

**Prerequisites:** P13. **Weight:** 4%. **Estimate:** Small/medium; 1 PR.

### Work

Write authoritative packages/http/README.md with all constructors, handler signatures, methods, fields, errors, lifecycle, fixed limits and binding explanation. Convert issue Examples 1–7 into runnable projects/tests; preserve exact Example 1. Replace illustrative application storage with a documented fixture only where the issue permits adaptation. Update package/docs indices, reference, architecture, async/HTTP guides and preview help.

### Exit gate

Each complete guide example compiles/runs source and bytecode through public exports. Both server forms and shutdown are demonstrated. Explain LAN opt-in for preview, generic client-visible errors and absence of tuning. Inventory preflight and package checks cover additions; no stale arrow syntax or internal APIs.

**Acceptance:** A01–A03, A28.

## P15: Installed consumer and final acceptance

**Prerequisites:** P14. **Weight:** 6%. **Estimate:** Medium; 1 PR.

### Work

Run every acceptance row on final source and bytecode, relocated installed examples and preview, plus client regression. Audit exports/dependencies and lifetime leaks. Verify compiler/library fixed points if changed; run full make check and exact-head hosted checks. Record performance evidence, limitations, issue/roadmap completion and release follow-up.

### Exit gate

Every A-row has test path, command, asserted result and passed revision. No unresolved gates. Full final checks pass; installed consumers survive relocation and intended source-independent bytecode execution. Close GI#518 only after integration and acceptance; do not claim alpha.14 publication or browser socket support.

**Acceptance:** A01–A28.

## Delivery evidence file

Create `pNN-acceptance.md` in this directory for each accepted packet. Include the
baseline/head SHA, exact changed surface, acceptance IDs, commands/results,
performance impact, unresolved gates, retained compatibility and the next packet.
Update this plan's progress table without changing weights silently. A failed test
or pending CI remains pending evidence. Do not write “done” based on source review.
