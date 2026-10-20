# GI#518 contract snapshot

Retrieved and updated with owner-approved bind-address clarification on 20 October 2026.
Source: [GI#518](https://github.com/sproates/panackelty/issues/518). This is the agreed
issue contract, not a claim of implementation. Compare the live issue before each
packet; owner-approved issue changes take precedence over this snapshot.

## Goal

Provide a simple, reusable routed HTTP server API in the existing `packages/http` package. Application code should create a server on a port, register method/path handlers that receive request and response objects, and start listening without configuring transport workers or connection limits for the common case.

This is a follow-up to [GI#237: HTTP server package](https://github.com/sproates/panackelty/issues/237), which delivered the bounded low-level server and service primitives. It does not reopen or replace that completed acceptance record.

## Public API contract

### Create a server

```panack
server = http.create_server(port)
```

`create_server(port: Nat): Server` creates an opaque, stateful server handle. The public API has no options record or limit-tuning arguments. The simple constructors bind to all local IPv4 interfaces; separate explicit-address constructors are defined below. The implementation uses these fixed defaults:

- Bind to all local IPv4 interfaces (`0.0.0.0`) so a server can be reached from the local network.
- Handle up to four requests concurrently; if all four request slots are occupied, immediately return HTTP 503 for a new request, close its connection, and do not queue it or invoke a handler. Keep at most sixteen connections open; reject additional connections with HTTP 503.
- Limit request and response bodies to 1 MiB each, measured in bytes, and the complete serialized request head (request line, header fields, and terminating CRLFs) and the complete serialized response header block to 16 KiB each. Reject oversized request bodies with 413 and request headers with 431. If an application response body or response header block exceeds its limit before anything is committed, log the failure and send a generic 500 response; never send a partial response.
- Apply a ten-second deadline from accepting a connection through handler completion and response completion. At the deadline, cancel the handler. If it has not called `send`/`send_bytes`, send 504 when the connection still permits it. If it has already completed a valid send, flush that buffered response rather than replacing it with 504. Log any handler failure after send; it must not replace the response.
- Drain active work for up to one second during graceful shutdown, then cancel remaining work.

These are fixed implementation defaults for the first version, not public configuration. `port` must be in 1–65535. Invalid ports and bind/listener failures are returned by `listen()` as `Error(ServerError)`.

### Create a contextual server

Some applications need per-server data in their handlers, such as the static preview tool's filesystem root identity and serving configuration. Provide a separate typed constructor:

```panack
server = http.create_contextual_server(8080, app_context)
```

Its generic signature is `create_contextual_server[T](port: Nat, context: T): ContextualServer[T]`. The type `T` is inferred from the supplied context value. The server stores that context using existing Panack value semantics and supplies it as the first argument to every handler invocation. This is explicit typed context passing; it does not use closures or add new mutation, aliasing, or general mutable-record semantics.

A `ContextualServer[T]` has the same route-registration methods, fixed transport defaults, setup-error behavior, lifecycle, request/response behavior, and HTTP semantics as `Server`. Its `get`, `post`, `put`, `delete`, and `fallback` methods take named handlers of type `AsyncFn[T,Request,ResponseWriter,Result[Unit,Str]]`. Every handler registered on one contextual server uses the same `T` and receives the same server context value. The context is fixed when the server is created; there is no per-route context or API to replace it. The ordinary `create_server(port)` and its two-argument handler type remain available and unchanged.

Use the contextual form when a named handler needs explicit server data. Do not force callers to wrap handlers in closures or expose the context through a public mutable route table.

### Choose an explicit bind address

Owner-approved clarification, 20 October 2026: preserve a simple all-interface constructor while allowing applications to choose an IPv4 bind address explicitly.

- `create_bound_server(address: Str, port: Nat): Server`
- `create_bound_contextual_server[T](address: Str, port: Nat, context: T): ContextualServer[T]`

These have the same handler, registration, error and lifecycle contracts as the corresponding simple constructors. The address must be a dotted-decimal IPv4 literal with four decimal octets in 0–255; leading zeroes are rejected except for the octet `0`. Hostnames, IPv6, whitespace and abbreviated forms are not accepted in this first version. Invalid addresses are reported by `listen()` as `Error(ServerError)` before binding, just like invalid ports. A syntactically valid address that cannot be bound produces a listener error.

`create_server(port)` and `create_contextual_server(port, context)` retain `0.0.0.0` as their bind address. Explicit `127.0.0.1` accepts local-machine connections only; `0.0.0.0` listens on all local IPv4 interfaces. This choice does not introduce public transport limits or an options record. All other fixed defaults remain identical.

The static preview tool uses `create_bound_contextual_server(address, port, static_context)`, retains its default `127.0.0.1`, and retains `--host` for an explicit IPv4 bind address. Documentation must explain how to deliberately allow LAN access. The generic package must not contain preview-specific defaults or logic.

### Register handlers

Provide these methods with the same shape:

```panack
server.get(path, @handler)
server.post(path, @handler)
server.put(path, @handler)
server.delete(path, @handler)
server.fallback(@handler)
```

Each exact-route method takes a route string beginning with `/` and a named async handler reference. It must be a syntactically valid URI reference: optional query and fragment components are allowed, but spaces and any other characters not valid in their URI component must be percent-encoded; malformed URI syntax is a setup error. Route-registration methods return `Unit`; they do not return `Result` and require no per-call error handling. Invalid paths, duplicate method/path pairs, and a second fallback are recorded as the server handle’s setup error. A duplicate never replaces the original handler. `listen()` reports the setup error as `Error(ServerError)` and does not bind. Route registration is valid only before listening begins; attempting to register after listen begins or after shutdown raises a runtime trap and leaves the route table unchanged.

The only supported methods are GET, POST, PUT, and DELETE. HEAD, PATCH, OPTIONS, TRACE, CONNECT, and extension methods are unsupported and are rejected before any handler runs. For an unsupported method on a path with explicit routes, return 405 with an `Allow` header listing only that path’s registered methods in canonical order: GET, POST, PUT, DELETE. If no explicit route path matches, reject an unsupported method with 405 and `Allow: GET, POST, PUT, DELETE`; unsupported methods never reach the fallback. For one of the four supported methods that has no handler on a path with other registered methods, return 405 with the same path-specific `Allow` behavior. The fallback is called only for supported methods when no route path matches; it does not mask 405. Without a fallback, an unmatched path returns 404.

Route matching uses only the route string's path component, case-sensitively, against the raw request path. Any query or fragment included in the registered route string is validated but ignored for matching; it does not constrain the request query. A route such as `/search?mode=brief#top` therefore matches requests to `/search` with any query. A fragment is never present in an HTTP request. The query is exposed separately on `Request`. Routing does not percent-decode, collapse repeated slashes, or normalize dot segments. Duplicate method/path registration is determined by the path component, so registering `/search` and `/search?mode=brief` for the same method is a duplicate.

### Request and handler types

`Request` is an immutable per-request value with:

- `method: Str` — one of the supported methods GET, POST, PUT, or DELETE. Unsupported methods are rejected before a handler is invoked.
- `path: Str` — raw path, excluding query; `/` when absent.
- `query: Str` — raw query without the leading `?`; empty when absent. It is not percent-decoded or parsed into key/value pairs.
- `headers: [Header]` — original header order and repeated fields retained. Each `Header` has `name: Str` and `value: Str` fields, matching the package’s existing public header type.
- `body: Bytes` — complete bounded request body.

`request.header(name): Option[Str]` returns the first matching header, and `request.header_values(name): [Str]` returns every matching value. Header-name comparison is ASCII case-insensitive.

The ordinary `Server` accepts named async handlers of type `AsyncFn[Request,ResponseWriter,Result[Unit,Str]]`. A `ContextualServer[T]` accepts named async handlers of type `AsyncFn[T,Request,ResponseWriter,Result[Unit,Str]]`, with its stored context supplied first on every invocation. No inline function expressions or closures are supported; handlers cannot capture route-registration locals. Do not add closures or lambda syntax solely for this API. The existing language syntax for the simple pattern was already validated in the earlier design discussion: a function returns an opaque server handle, then the caller registers routes through repeated `server.get(...)` calls. Treat that as established behavior. Keep the direct registration form in Example 1; do not add language syntax or general mutable-record semantics solely for this API.

### Response writer

`ResponseWriter` is an opaque, mutable handle scoped to one request. It starts with status 200 and no application headers or body. Methods are effectful and return `Unit`:

- `response.status(code: Nat)` sets the status; valid values are 200–599.
- `response.header(name: Str, value: Str)` replaces any existing field of that name, case-insensitively.
- `response.add_header(name: Str, value: Str)` appends another field, preserving repeated fields such as Set-Cookie.
- `response.send(body: Str)` sets a text body; absent an explicit Content-Type, use `text/plain; charset=utf-8`.
- `response.send_bytes(body: Bytes)` sets a byte body; absent an explicit Content-Type, use `application/octet-stream`.

The server generates Content-Length for ordinary responses and `Connection: close`. Applications cannot set or append Content-Length, Transfer-Encoding, or Connection. Header names/values are validated against HTTP injection. An invalid status code or response header name/value raises a recoverable request-scoped runtime trap; before response bytes are committed this is logged and becomes a generic 500, and after a completed send it is logged without replacing the response. Status 204 and 304 cannot have a body. `response.send("")` completes an empty response for either status and emits no body bytes or Content-Length; a non-empty body with either status is an invalid response and becomes a generic 500 before bytes are committed. The complete serialized response header block, including the status line and generated fields, is limited to 16 KiB.

`send`/`send_bytes` completes the response exactly once; the server normally writes the buffered response after the handler finishes. If the handler returns `Ok(())` without sending, the server sends a generic 500. A second send or a response mutation after send traps as a response-state error; it never writes a second response. If the handler fails or reaches the request deadline after a valid send, flush the buffered response and log the failure; do not replace it. If the client has disconnected, the response cannot be replaced.

### Errors

`ServerError` is the public error object returned by `listen()`, `close()`, and `destroy()`. It also reports setup errors recorded by route registration when `listen()` is called:

```panack
pub record ServerError {
  message: Str
}
```

Callers inspect the human-readable message as `error.message`. The message describes the failure; its exact wording is not a stable machine-readable code. `listen`, `close`, and `destroy` return `Result[Unit,ServerError]`. Route-registration calls return `Unit`; setup failures include invalid route, duplicate method/path, and duplicate fallback, and are surfaced by `listen()` before binding. Listener failures include invalid port, invalid lifecycle operation, bind conflict, and listener failure.

If a handler returns `Error(message)` before the response is complete, the server logs the diagnostic to stderr, sends a generic 500 without exposing the message, and continues serving. Recoverable Panackelty runtime traps raised in a handler before response completion (for example, invalid indexing or checked arithmetic failure) are isolated to that request, logged, and converted to a generic 500. A later request must still succeed. An invalid response before bytes are committed also becomes a generic 500. A failure after a completed send is logged but cannot replace the response.

This requires a per-request fault boundary in the server/runtime integration; current TCP service behavior propagates handler traps to the full execution. VM integrity faults, allocation failure, process termination, and failures that make safe recovery impossible are outside this recovery promise.

### Lifecycle

The handle moves through Created, Listening, Closing, Closed, and Destroyed states.

- `await server.listen()` binds and resolves once ready; it does not wait for the server's whole lifetime. A listening handle is an active runtime resource and keeps the program alive after `main` returns.
- `await server.close()` stops new admissions, lets active handlers and response writes finish for the fixed one-second drain period, then cancels remaining work and releases resources. It is idempotent. Before listen it closes the handle without binding; after shutdown it succeeds.
- `await server.destroy()` stops admission and cancels active handlers immediately, without the drain period; it resolves once resources are released. It is idempotent and escalates an in-progress close.
- Listening after close/destroy fails. Normal program exit and termination signals close listening servers gracefully, subject to the bounded drain. Fatal VM/host failures or forced process termination may prevent graceful cleanup.

A connection handles one request and closes after its response. Keep-alive and pipelining are not supported.

### HTTP protocol baseline

The routed server uses the existing HTTP parser contract: HTTP/1.1 only; exactly one non-empty `Host` header; a request body may be framed by one `Content-Length` or `Transfer-Encoding: chunked`. A request with neither framing header has an empty body when no bytes follow the request head; bytes after an unframed request head are rejected as a 400 error. Malformed requests, duplicate `Content-Length`, and conflicting `Content-Length` plus `Transfer-Encoding` are rejected with 400. Unsupported transfer encodings receive 501, and unsupported HTTP versions receive 505. The parser enforces the fixed limits above.

### `Expect: 100-continue`

Support one `Expect` header whose trimmed value is `100-continue`, case-insensitively. After validating the complete request head, confirming the expectation is supported, determining that the route/method can be handled, and admitting the request under the concurrency limit, send the interim response `HTTP/1.1 100 Continue\r\n\r\n` before waiting for a body that has not already arrived. This is not the final response; the handler is invoked only after the complete bounded body has been read.

If the server can determine a final rejection before reading the body, send that final response without first sending 100. This includes malformed headers, a declared Content-Length above the body limit (413), an unsupported method (405), an unmatched path without a fallback (404), or exhausted request/connection capacity (503). For chunked bodies, send 100 after admission and enforce the 1 MiB limit while reading; if it is exceeded, send 413 and do not invoke the handler. The normal ten-second request deadline includes the interim-response and body-read phases.

Any other expectation value, multiple `Expect` fields, or a comma-separated expectation list receives 417 Expectation Failed, closes the connection, and does not invoke a handler. Do not send 100 when the complete body has already arrived or the request has no body to read. Requests without an `Expect` header follow the normal path.

## Examples

Implementation erratum (2026-10-20): the example match arms use `Ok(done)`
instead of the original `Ok(())`. Panackelty constructor patterns require a
binding at that position. Example 3 also uses the existing positional record
constructor `AppContext("Hello")`, replacing unsupported brace construction.
These corrections fix example syntax without adding a language
feature or changing the server API; acceptance compiles these corrected examples
verbatim and changes only the demonstration port for isolated socket runs.

All examples use the API names and semantics above. They are implementation targets, not claims that these APIs already exist.

### 1. Register several routes and listen

The exact direct route-registration sequence is a required acceptance example; it must not require reassignment between calls or nested result handling for each registration. Registration methods return `Unit`; setup errors are returned by `listen()` as `Error(ServerError)` before binding.

```panack
import http/server as http

async home(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("Hello")
  Ok(())
}

async health(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("OK")
  Ok(())
}

async create_message(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.status(201)
  response.header("Content-Type", "text/plain; charset=utf-8")
  response.send("Created")
  Ok(())
}

async about(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("About this service")
  Ok(())
}

async main(): Unit {
  server = http.create_server(8080)
  server.get("/", @home)
  server.get("/health", @health)
  server.post("/messages", @create_message)
  server.get("/about", @about)
  match await server.listen() {
    Ok(done) => (),
    Error(error) => {
      startup_error = error.message
      ()
    }
  }
  ()
}
```

### 2. Read the path, raw query, headers, and body; set status and headers

For `/search?q=panack%20language`, `request.path` is `/search` and `request.query` is `q=panack%20language`. Query decoding and key/value parsing are left to application code in the initial version.

```panack
async search(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  if request.path == "/search" {
    response.status(200)
    response.header("X-Received-Query", request.query)
    match request.header("accept") {
      None() => response.send("query=" + request.query),
      Some(value) => response.send("accept=" + value + "; query=" + request.query)
    }
    Ok(())
  } else {
    response.status(404)
    response.send("Not found")
    Ok(())
  }
}
```

Use `request.header_values("set-cookie")` or another name to inspect repeated request fields. Use `response.add_header` when a response needs repeated fields.

### Static site preview integration

The dependency runs in one direction only: the static site preview tool may use this generic HTTP package, but `packages/http` has zero knowledge of the static file server. The HTTP package must not import, name, or contain static-server-specific routes, handlers, defaults, or behavior. The preview tool owns composition: it creates a contextual server with its existing static-serving context, then registers its own named static-file handler as the fallback after application routes:

```panack
server = http.create_bound_contextual_server(address, port, static_context)
server.get("/health", @health)
server.fallback(@serve_static)
```

Here `static_context` is the preview tool's existing static-serving context (including its configured root and filesystem identity), and `serve_static` is a named handler owned by the preview tool with the contextual handler signature. The implementation adapts static serving to write through `ResponseWriter`; it must retain the tool’s existing path confinement, index-file, redirect, MIME-type, and size-limit behavior. The generic HTTP package only stores and passes the typed context and invokes the fallback. It does not import or inspect static types.

The fallback serves GET requests for paths without an explicit route. POST, PUT, and DELETE to a path served only by the static fallback return 405 with `Allow: GET`; an explicit route with a method mismatch returns 405 before the fallback runs.

### 3. Use a typed server context

This example shows that one contextual server stores a value of type `AppContext` and passes it to every registered named handler. It uses the same routing and response API as the ordinary server.

```panack
import http/server as http

record AppContext {
  greeting: Str
}

async home(context: AppContext, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send(context.greeting)
  Ok(())
}

async health(context: AppContext, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("OK: " + context.greeting)
  Ok(())
}

async main(): Unit {
  context = AppContext("Hello")
  server = http.create_contextual_server(8080, context)
  server.get("/", @home)
  server.get("/health", @health)
  match await server.listen() {
    Ok(done) => (),
    Error(error) => {
      startup_error = error.message
      ()
    }
  }
  ()
}
```

Both handlers receive the same typed context as their first argument. The server does not capture local values in closures, and this example does not require the ordinary `create_server` form or its two-argument handlers to change.

### 4. Exercise the supported and unsupported methods

Register and exercise GET, POST, PUT, and DELETE handlers. Requests using HEAD, PATCH, OPTIONS, TRACE, CONNECT, or another unsupported method against a registered path receive 405 with an `Allow` header listing that path’s registered methods in canonical order. No implicit HEAD or OPTIONS route is created.

### 5. Handler errors and runtime traps

This example shows an ordinary application error returned through the handler's `Result`. `message_store.create` is illustrative application code; the HTTP server contract does not prescribe a storage API.

```panack
async create_message(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  match await message_store.create(request.body) {
    Error(problem) => Error(problem),
    Ok(message) => {
      response.status(201)
      response.send(message)
      Ok(())
    }
  }
}
```

If `message_store.create` returns `Error(problem)`, the handler returns that error before sending a response. The server logs the diagnostic, sends a generic 500 without exposing `problem`, and keeps serving later requests. On success the handler sends a 201 response and returns `Ok(())`.

The next example shows the separate runtime-trap path:

```panack
async echo_first_byte(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  first = request.body[0] // An empty body triggers a checked runtime trap.
  response.send_bytes(request.body)
  Ok(())
}
```

With an empty request body the client receives a generic 500, the diagnostic is written to stderr, and a subsequent request to a healthy route still succeeds. Returning `Error("internal detail")` before sending has the same client-visible 500 behavior; the detail is logged but not sent to the client.

### 6. Register PUT and DELETE routes

```panack
async replace_message(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("Replaced")
  Ok(())
}

async delete_message(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.status(200)
  response.send("Deleted")
  Ok(())
}

async main(): Unit {
  server = http.create_server(8080)
  server.put("/messages/current", @replace_message)
  server.delete("/messages/current", @delete_message)
  match await server.listen() {
    Ok(done) => (),
    Error(error) => {
      startup_error = error.message
      ()
    }
  }
  ()
}
```

This uses the same fixed defaults as `create_server(port)`; the simple constructors use 0.0.0.0; callers choose another IPv4 bind address only through the explicit-address constructors. Concurrency, size limits, deadlines and drain time remain fixed.

### 7. Graceful and forced shutdown

```panack
async graceful_close(server: http.Server): Result[Unit,http.ServerError] {
  await server.close()
}

async force_destroy(server: http.Server): Result[Unit,http.ServerError] {
  await server.destroy()
}
```

After listening, call `await graceful_close(server)` to stop admission and drain active requests. Call `await force_destroy(server)` only when active requests must be cancelled immediately. Each helper returns the operation's `Result`, so its caller can match the success or inspect `error.message`. Normal program exit and termination signals use bounded graceful shutdown automatically.

## Non-goals for the first version

Path parameters, wildcard route patterns, middleware, cookies, streaming responses, TLS, HTTP/2, and a general web framework. Do not add general TCP server functionality as part of this package API change.

## Acceptance

- Document and test both explicit-address constructors alongside the simple constructors. Verify default all-interface binding for the simple forms, explicit loopback binding, invalid IPv4 syntax and unavailable-address errors through `listen()`, and unchanged fixed resource limits. The preview tool retains loopback by default and its explicit `--host` option. No test may rely on an externally reachable network; use deterministic local interface checks.

- The exact route-registration sequence in Example 1 compiles and runs. All four registrations occur on the same opaque handle without reassigning `server`; the example binds and matches the non-Unit result of `await server.listen()`, which completes after the listener is ready.
- Both `create_server(port)` and `create_contextual_server(port, context)` are documented public constructors. The exact Example 1 remains unchanged and compiles/runs with two-argument handlers; Example 3 compiles/runs with one server-wide typed context passed as the first argument to every named handler. Tests verify multiple named handler references with the same concrete context type are stored and dispatched without closures or type erasure. Both server forms have identical route methods, defaults, setup errors, protocol behavior, and lifecycle semantics.
- Add a user-facing `packages/http/README.md` that is the authoritative package guide, readable from the package directory on GitHub without requiring readers to consult this issue or internal implementation docs. It must clearly explain when to use the HTTP package and show a complete minimal server from creation through route registration and `listen()`; document both `create_server(port)` and `create_contextual_server(port, context)`, their handler signatures, and how the typed context reaches handlers; document all supported methods and fallback/matching rules; describe every `Request` field and header accessor; document `ResponseWriter` status, header, send, and send-bytes behavior; explain setup and handler errors, runtime-trap recovery, fixed protocol/resource defaults and limits, and listen/close/destroy lifecycle behavior. Include practical source examples for ordinary and contextual servers, request/query/header handling, responses and errors, and shutdown. Ensure examples use only the public API and match this ticket's behavior. Link the README from the repository's appropriate package/documentation index, and keep implementation-only transport details out of the user guide unless needed to explain observable behavior.
- The package README documents the public constructor, request/response fields, four route methods plus fallback, response methods, `ServerError.message`, and lifecycle operations. It describes fixed first-version defaults without exposing tuning options. Examples 1–7 are adapted into executable package examples/tests where applicable.
- Tests register and exercise every supported route helper: GET, POST, PUT, and DELETE. Verify unsupported methods receive 405 with `Allow` containing only registered methods in canonical order; there is no implicit HEAD or OPTIONS behavior.
- Functional tests verify request path/query/header/body access; bodyless requests without framing and rejection of bytes following an unframed request head; exact raw-path matching; route URI validation (including query/fragment acceptance, malformed escapes, raw spaces/control characters, and duplicate path components); fallback precedence; 404/405 behavior; duplicate/invalid route and duplicate-fallback failures returned by `listen()` as `ServerError` before binding, with a non-empty human-readable `message`; late registration traps without changing the route table; status, header replacement/appending, text/byte responses; default Content-Type and generated framing; and one-request-per-connection behavior.
- Fixed implementation bounds are covered at their exact boundaries: the complete request-head and response-header-block limits are 16 KiB; request and response body limits are 1 MiB in bytes. Tests cover values at the limit and over it, request-header overflow -> 431, request-body overflow -> 413, and oversized response body/header block -> generic 500 with no partial response. Tests separately saturate all four request slots and all sixteen open connections; each rejected request/connection gets 503, no request is queued, and no handler runs for rejected work. A request that exceeds the fixed deadline before sending returns 504 when possible; a request that has already sent and then stalls is cancelled and its buffered response is flushed. Graceful close drains for at most one second. None of these values is user-configurable in the first-version API.
- Functional tests verify missing send -> 500, duplicate send/mutation-after-send errors, returned handler errors, invalid status/header input -> logged generic 500 before commit, per-request runtime-trap-to-500 recovery, continued service after a trapped request, and no second response after a completed send. Wire-level tests verify Content-Length is omitted for 204 and 304. Wire-level tests verify the 100 Continue interim response arrives before a delayed body, the handler runs only after the complete body is read, no interim response is sent when no body remains to read, early final rejections do not send 100 first, oversized chunked bodies return 413 after the interim response, and unsupported expectations return 417 without invoking a handler.
- Lifecycle tests cover listen success/failure, close/destroy before listen, repeated close/destroy, listen after shutdown, graceful drain and timeout, forced destroy, and resource release on normal exit/signals.
- The static directory preview tool composes this public server API with its own fallback handler; `packages/http` has no static-server dependency or special case. End-to-end tests cover serving an index and nested asset, MIME type, redirect, path traversal rejection, size limits, and 405 behavior for non-GET fallback requests while confirming explicit-route precedence.
- Existing low-level APIs are no longer documented/exported as public server entry points; the existing HTTP client behavior remains intact.
- Update package README, language/API reference, examples, and affected architecture/specification docs. Run canonical `make check` after the final product edit.


## Test plan and execution order

Implement and run checks from the narrowest, fastest layer outward. The examples and behaviors below are the agreed contract, not opportunities to change or simplify it. Add automated tests to the existing package/unit and functional-test routes so these checks run in CI; do not leave important cases as manual-only instructions.

### 1. Compile and public-surface checks

- Compile the exact direct-registration example in Example 1 unchanged: create one server, call `get` repeatedly and `post` once on the same handle, then await and match `listen()`. Also run it as bytecode. This catches syntax, callable-reference, opaque-handle mutation, and module-export problems before starting a server.
- Compile the contextual-server example in Example 3 as source and bytecode. Verify type inference for `T`, storage of multiple named handlers with the same context type, and invocation with the stored context as the first argument.
- Compile examples covering GET, POST, PUT, DELETE, fallback, request fields, response methods, returned `Error(message)`, and close/destroy.
- Verify the routed server API is the public `packages/http` surface, the old low-level server entry points are not publicly exported or documented, and the existing HTTP client still imports and works.
- Keep these as quick compile/contract checks; do not run the full project pipeline to discover a simple API or visibility error.

### 2. Fast package-level behavior tests

Add focused tests for behavior that does not require sockets:

- Verify the contextual route table is typed as one `ContextualServer[T]`: multiple named handlers accept the same `T`, receive the configured context, and dispatch correctly. Verify ordinary `Server` handlers retain the two-argument signature. Do not add type erasure or closure capture.
- Split the raw request target into path and query; preserve percent escapes, repeated slashes, dot segments, and case exactly. Verify absent path/query behavior.
- Preserve header order and repeated fields; verify `header` returns the first case-insensitive match and `header_values` returns all matches.
- Test route matching, method/path duplicates (including duplicate path components with different query/fragment suffixes), valid and malformed route URI references, duplicate fallback, original-handler retention after a duplicate, fallback precedence, 404/405 selection, and canonical `Allow` values. Verify registered query/fragment suffixes are ignored for matching.
- Test response defaults and state transitions: status range, replacing versus appending headers, invalid names/values, reserved framing headers, default text/byte content types, send-once, mutation after send, missing send, and 204/304 body rules.
- Test response serialization and body/header byte counts at exactly each limit and one byte over. Verify generated framing and that 204/304 omit Content-Length.
- Test the public setup-error contract: registration methods return `Unit`; `listen()` reports a recorded setup error as a non-empty `ServerError.message` before binding. Verify a duplicate does not replace the first handler.

### 3. Focused real-server and wire-protocol tests

Use a real compiled server and a controllable client/HTTP fixture. Exercise both source execution and compiled bytecode, and assert status, headers, body bytes, whether the handler ran, and whether the connection closed. Include:

- Each supported verb and its route; unsupported HEAD, PATCH, OPTIONS, TRACE, CONNECT, and extension methods; method mismatch on a known path; unmatched path with and without fallback; explicit route taking precedence over fallback.
- Request path/query, repeated headers, body reads, response status, replaced/appended headers, text and byte bodies, and generated Content-Length/Connection behavior.
- HTTP/1.1 Host and framing rules: valid bodyless request without framing, Content-Length, chunked body, malformed request, missing/duplicate Host, duplicate Content-Length, conflicting Content-Length and Transfer-Encoding, unsupported transfer encoding/version, and bytes following an unframed request head. Assert the specified 400, 501, or 505 responses and that rejected requests never invoke handlers.
- `Expect: 100-continue` as a raw two-phase exchange: delay the body until the interim response is observed; confirm the handler waits for the complete body. Cover no-body/already-arrived-body cases, every early final rejection (400/404/405/413/503/417 as applicable) without a preceding 100, chunked overflow after 100, and unsupported/multiple expectations.
- Handler `Error(message)`, a recoverable runtime trap, invalid response input, missing send, and response-state errors. Confirm a generic 500 is sent only when no response has been committed, diagnostics stay out of the client response, and a subsequent healthy request succeeds.
- Exact request/response head and body limits, including request 431/413 and oversized response generic 500 with no partial response. Construct boundaries in bytes, including multibyte text where relevant.
- The fixed ten-second request deadline, including a slow body and a handler that does not finish. If the handler has not sent, confirm 504 when possible; if it has sent and then stalls, confirm the buffered response is flushed and the handler is cancelled. Verify safe connection cleanup in both cases.

Raw-socket cases should use explicit synchronization (for example, wait for a handler-start signal before releasing it), not arbitrary sleeps. Keep deadline tests isolated so one timeout does not stall unrelated cases.

### 4. Lifecycle, admission, and resource tests

Use controlled in-flight handlers to make admission and shutdown outcomes deterministic:

- Successful listen resolves when ready and the server continues to keep the program alive after `main` returns. Verify invalid port and bind conflict return `Error(ServerError)` without a listening socket.
- Exercise close and destroy before listen, repeated close/destroy, listen after shutdown, close during active work, destroy during active work, and normal exit/signal cleanup. Check that resources and ports are released.
- Hold four admitted handlers open and verify the next request receives 503 immediately, is not queued, and never invokes a handler. Separately hold sixteen connections open and verify additional connections are rejected with 503.
- For graceful close, verify admission stops, active work can finish within the one-second drain, and remaining work is cancelled after the bound. For destroy, verify active work is cancelled without waiting for the drain.
- Assert completion, cancellation, and handler-call counts as well as wire responses; do not infer them from logs alone.

### 5. Static-preview composition and regression tests

- Run the existing static HTTP integration coverage against the preview tool after it is changed to create `ContextualServer[static_http.Context]` and register its static handler as a fallback. Verify index and nested assets, MIME type, redirects, path confinement/traversal rejection, and existing size limits.
- Verify an explicit application route wins over the static fallback, and POST/PUT/DELETE to fallback-only paths return 405 with `Allow: GET`.
- Keep a dependency/visibility check that `packages/http` contains no static-server import, route, or special case. The preview tool owns all static behavior.
- Run existing HTTP client tests unchanged to catch accidental regressions to the package's client API.

### 6. Required validation gate

Run the focused package/unit tests and the HTTP package and static-preview functional tests first; fix failures before starting broad validation. Then run canonical `make check` once after the final product edit, from the repository root. Verify the exact PR head has all required hosted checks green before merging. A passing local focused test or an earlier commit's CI run does not replace validation of the final head.
## Implementation rule for unexpected gaps
The public API and acceptance criteria above are the agreed contract. If implementation reveals a gap, inconsistency, ambiguity, or other unexpected condition that is not explicitly covered by this ticket, stop the affected work and ask the issue owner for clarification. Do not guess, silently choose behavior, or expand the contract. Continue only after the owner clarifies the intended behavior, then update this ticket before relying on that decision.

## Open implementation detail

Implement the stateful opaque server and response handles and route table without exposing transport worker/context plumbing to ordinary application code. Keep route registration ergonomic: registration methods return `Unit`, setup failures are reported by `listen()` before binding, and late registration traps without changing the route table. Named callable references are sufficient; do not add closures or general mutable-record semantics solely for this API. Verify package/module visibility so the old low-level API is internal while the routed API remains public. Establish the per-request recovery boundary without turning fatal VM/host failures into unsafe continuation, and ensure resources are released safely on every lifecycle path.

## Relationship to existing work

- [GI#237: HTTP server package](https://github.com/sproates/panackelty/issues/237) — completed low-level transport/server foundation.
- [GI#162: Serve local website previews with a Panackelty-written server](https://github.com/sproates/panackelty/issues/162) — site preview is a consumer/use case, not the general HTTP API definition.
- This ticket records scope only; it does not select roadmap priority or authorize implementation by itself.
