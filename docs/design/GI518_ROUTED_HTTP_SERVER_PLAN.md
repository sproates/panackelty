# GI#518: routed HTTP server implementation plan

Status: **proposed implementation plan; GI#518 is the acceptance contract**. The routed API is not implemented yet. This plan records the agreed sequence for work on `next`; it does not authorize starting implementation or change the ticket's scope.

## Agreed API

Keep `packages/http` as the package. Its public server surface will be the routed API, while the existing low-level server/service machinery becomes an implementation detail. Preserve the existing HTTP client API and behavior.

The ordinary form remains:

```panack
server = http.create_server(8080)
server.get("/", @home)
server.get("/health", @health)
server.post("/messages", @create_message)
await server.listen()
```

Its named async handlers receive `(Request, ResponseWriter)` and return `Result[Unit,Str]`.

For handlers that need explicit per-server data, add the separate typed form:

```panack
server = http.create_contextual_server(8080, app_context)
server.get("/", @home_with_context)
server.fallback(@static_files)
await server.listen()
```

The result is `ContextualServer[T]`; its named handlers receive `(T, Request, ResponseWriter)` and return `Result[Unit,Str]`. `T` is inferred from the context value. All handlers on that server share the same context type and stored value. This is explicit typed context passing, not closure capture, a public mutable route table, or a new language-level mutation/aliasing feature. Both server forms otherwise share the route, response, fixed-default, error, protocol, and lifecycle contract in [GI#518](https://github.com/sproates/panackelty/issues/518).

The static preview tool owns its `static_http.Context`, its named static handler, and its fallback composition. The generic HTTP package stores and passes the context without importing or knowing about static serving.

## Repository starting points

- `packages/http/panack.json` currently exports `types`, `client`, `server`, and `service`.
- `packages/http/src/server.panack` and `service.panack` contain the low-level server/service surface; `client.panack` is an existing public client that must keep working.
- `packages/static_http/src/service.panack` currently owns `Context` and `respond(context, request)`, and uses the low-level HTTP API.
- `tools/http-server` is the consumer that assembles static serving configuration.
- `tests/http_package.sh` and `tests/static_http.sh` exercise the existing HTTP package and static tool.
- `packages/http/README.md` currently describes the low-level API. GI#518 requires it to become the complete, user-facing guide for the routed API and retained client, and requires removal of obsolete public-server guidance.

Before implementation, inspect current callers and exports again on `next`; this list is a planning snapshot, not a substitute for checking the exact working tree.

## Delivery sequence

### 1. Establish API and package visibility

1. Re-read GI#518 and this plan against the current `next` tree.
2. Inventory imports, manifest exports, docs, examples, tests, tools, and any website references to `http/server`, `http/service`, and low-level symbols.
3. Map each low-level dependency to an internal implementation path and update consumers in the same change. Keep the HTTP client public and regression-tested. Do not leave old server/service entry points documented as public.
4. Confirm the ordinary direct registration syntax and the contextual handler signature with the existing compiler before building the full server.

A small generic Panack prototype has already shown that a generic record can store named typed async handler references with a shared context and dispatch them with that context. It passed source checking, source execution, bytecode compilation, and bytecode execution. This proves the basic typed-storage pattern only; package visibility, route mutation, networking, and fault isolation still need implementation tests.

### 2. Implement the opaque handles and routing core

1. Add the opaque `Server` and `ContextualServer[T]` public handles and constructor functions.
2. Use the existing language's supported stateful-handle behavior for repeated `server.get(...)` calls. Preserve the exact simple registration example in GI#518; do not require reassignment or a `Result` match per registration.
3. Store route path components, methods, named handler references, and at most one fallback. Keep contextual route handlers typed with the same `T`; do not erase types or capture locals in closures.
4. Validate route URI references and setup errors at registration, retaining the first handler on duplicates. Return setup failures from `listen()` before binding.
5. Implement method/path selection, path-only matching, query/fragment behavior, fallback precedence, 404, and 405/`Allow` behavior as specified in the issue.

Start with compile-level and package-level tests that do not open sockets. These should expose API/type/export errors quickly.

### 3. Adapt HTTP requests and implement `ResponseWriter`

1. Reuse the existing parser and transport. Map a parsed request to the immutable public `Request` fields and header accessors without changing the parser's documented wire behavior beyond GI#518's contract.
2. Add an opaque per-request writer with status, replace-header, append-header, text-send, and byte-send operations.
3. Validate status, header syntax, reserved framing fields, body/status combinations, send-once state, generated headers, and the specified byte limits before committing bytes.
4. Add focused serializer/parser tests for exact limit boundaries and one-byte-over cases before broad protocol tests.

### 4. Implement admission, lifecycle, and failure isolation

1. Integrate the fixed request/connection admission limits, deadlines, 503 rejection, and bounded graceful shutdown with the existing service/transport internals.
2. Make `listen()` resolve at readiness while retaining the listening resource; implement pre-listen and repeated `close()`/`destroy()` behavior, escalation, termination cleanup, and release of sockets/resources.
3. Establish the per-request recovery boundary for returned handler errors, recoverable runtime traps, invalid response operations, timeout, and post-send failures. Preserve fatal VM/host failure limits stated in GI#518.
4. Use deterministic in-flight handlers and synchronization in tests; assert handler counts, cancellation, response bytes, and resource release rather than inferring behavior from logs.

If a compiler/runtime limitation or an uncovered lifecycle/error case conflicts with the ticket, stop that affected implementation path and ask the issue owner. Do not silently weaken the API, broaden language semantics, or invent a fallback behavior.

### 5. Integrate the static preview tool

1. Construct `ContextualServer[static_http.Context]` in the preview tool using the static configuration/context it already owns.
2. Adapt static serving to a named contextual handler that writes through `ResponseWriter`, then register it with `fallback` after any explicit application routes.
3. Keep path confinement, filesystem identity checks, index selection, redirects, MIME mapping, and size policy in the static package/tool. Add no static imports, types, defaults, or special cases to `packages/http`.
4. Preserve current preview behavior and verify explicit-route precedence and 405 responses for non-GET methods on fallback-only paths.

### 6. Replace and complete package documentation

Rewrite the existing `packages/http/README.md` as the authoritative user guide for the package. It must explain the client and new routed server APIs, both constructors and handler signatures, typed context, supported methods, fallback and matching, every request field/accessor, response operations, errors and runtime traps, fixed limits/defaults, and listen/close/destroy. Include complete, practical examples that agree with the API and tests. Remove obsolete claims presenting low-level server/service functions as public application APIs. Link the README from the package index and update the language/API reference, examples, architecture/specification docs, and other callers affected by the new public surface.

Review documentation against actual signatures and observable behavior; do not expose internal worker/context plumbing merely to describe implementation.

## Validation order

Keep feedback early and narrow:

1. **Public-surface compile checks:** compile the unchanged direct-registration example and the contextual example as source and bytecode. Check package exports, callable-reference types, context inference, and the existing HTTP client imports.
2. **Fast package tests:** exercise target parsing, route validation and selection, context dispatch, header access, response state, serializer boundaries, and setup errors without sockets.
3. **Focused source/bytecode wire tests:** exercise methods, fallback, parser/framing, `Expect: 100-continue`, limits, handler errors/traps, deadlines, and response bytes against a real compiled server.
4. **Lifecycle/admission tests:** test readiness, saturation, close/destroy, cancellation, signal/normal-exit cleanup, and resource release with controlled handlers.
5. **Static integration/regression tests:** run the static preview source/bytecode/installed flows, confinement and existing HTTP client tests.
6. **Final validation:** after the final product edit, run canonical `make check`. Require applicable hosted checks to pass for the exact PR head before merging. Do not use an earlier head's green result as evidence for later edits.

The exact cases, fixed values, expected status codes, and acceptance criteria are maintained in GI#518 and should be implemented as automated tests rather than manual-only checks.

## Completion and scope boundary

Complete the package implementation, consumer migration, user documentation, tests, and relevant architecture/reference updates together. Keep the HTTP client working. Keep the generic package independent of static serving. Do not add closures, middleware, streaming, TLS, HTTP/2, wildcard routes, configurable first-version limits, or general TCP APIs. If work reveals an inconsistency or unanticipated gap, stop and get clarification before choosing behavior, then update GI#518 first.
