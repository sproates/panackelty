# HTTP Server reference

Signatures and behaviour are checked against
the package source and integration tests linked in [Source and test evidence](#source-and-test-evidence).

Use [`http/server`](../../packages/http/README.md) for routed handlers. The
packaged HTTP client is a separate API. For directory serving, see the
[static HTTP server tool](../../tools/http-server/README.md).

New to this API? Start with [the HTTP playbook](../guides/HTTP_PLAYBOOK.md).
It includes complete programs, setup steps, requests to try and expected results.

## API index

### Constructors

See the [worked playbook guide](../guides/HTTP_PLAYBOOK.md#your-first-server).

- [`create_server`](#create_server)
- [`create_bound_server`](#create_bound_server)
- [`create_contextual_server`](#create_contextual_server)
- [`create_bound_contextual_server`](#create_bound_contextual_server)

### `Server` route methods

- [`Server.get`](#serverget)
- [`Server.post`](#serverpost)
- [`Server.put`](#serverput)
- [`Server.delete`](#serverdelete)
- [`Server.fallback`](#serverfallback)

### `ContextualServer` route methods

- [`ContextualServer.get`](#contextualserverget)
- [`ContextualServer.post`](#contextualserverpost)
- [`ContextualServer.put`](#contextualserverput)
- [`ContextualServer.delete`](#contextualserverdelete)
- [`ContextualServer.fallback`](#contextualserverfallback)

### Lifecycle methods

See the [worked playbook guide](../guides/HTTP_PLAYBOOK.md#own-the-listener-lifecycle).

- [`Server.listen`](#serverlisten) · [`ContextualServer.listen`](#contextualserverlisten)
- [`Server.close`](#serverclose) · [`ContextualServer.close`](#contextualserverclose)
- [`Server.destroy`](#serverdestroy) · [`ContextualServer.destroy`](#contextualserverdestroy)

### Behaviour and examples

- [Handler inputs and responses](#handler-inputs-and-responses)
- [Routing and errors](#routing-and-errors)
- [Fixed limits](#fixed-limits)
- [Runnable programs](#runnable-programs)
- [Source and test evidence](#source-and-test-evidence)

## Import and first server

```panack
import http/server as http
```

The module exports the constructors and `ServerError`. It also re-exports
`Request`, `Header`, and `ResponseWriter`.

```panack
async home(request: http.Request,
    response: http.ResponseWriter): Result[Unit,Str] {
  response.send("Hello")
  Ok(())
}

async main(): Unit {
  server = http.create_bound_server("127.0.0.1", 8080)
  server.get("/", @home)
  match await server.listen() {
    Ok(done) => (),
    Error(problem) => { message = problem.message; () }
  }
}
```

Create the handle and register routes before awaiting `listen()`. The listener
binds when `listen()` runs and resolves when the socket is ready. An active
listener keeps the native process alive. The [ordinary runnable program](#runnable-programs)
also accepts a port argument, registers GET and POST routes, and reports
startup errors.

## Constructors

All constructors are synchronous and return a server handle. They do not open
a socket; `listen()` performs the bind and reports address or port errors.
Ports must be in `1..65535`. Addresses must be dotted-decimal IPv4 literals;
hostnames and IPv6 addresses are unsupported.

### `create_server`

```panack
create_server(
  port: Nat
): Server
```

**Parameters**

- `port`: Port from 1 through 65535.

**Returns:** A new `Server` configured to bind to `0.0.0.0`, all local IPv4
interfaces. **Async:** No; binding happens when `await server.listen()` runs.
An invalid port is returned by `listen()` as `Error(ServerError)`.

### `create_bound_server`

```panack
create_bound_server(
  address: Str,
  port: Nat
): Server
```

**Parameters**

- `address`: Dotted-decimal IPv4 bind address. Use `"127.0.0.1"` for
  connections from the local machine only.
- `port`: Port from 1 through 65535.

**Returns:** A new `Server` for that address. **Async:** No; binding happens
when `await server.listen()` runs. Invalid address and port values are
returned by `listen()` as `Error(ServerError)`.

### `create_contextual_server`

```panack
create_contextual_server[T](
  port: Nat,
  context: T
): ContextualServer[T]
```

**Parameters**

- `port`: Port from 1 through 65535.
- `context`: Typed value retained for this server; its type determines `T`.

**Returns:** A `ContextualServer[T]` configured to bind to `0.0.0.0`.
**Async:** No; binding happens when `await server.listen()` runs. Port errors
are returned by `listen()` as `Error(ServerError)`.

Each route handler receives the stored context as its first argument. All
handlers on one contextual server use the same `T`; handlers are named async
function references and do not capture closures.

### `create_bound_contextual_server`

```panack
create_bound_contextual_server[T](
  address: Str,
  port: Nat,
  context: T
): ContextualServer[T]
```

**Parameters**

- `address`: Dotted-decimal IPv4 bind address. `"127.0.0.1"` keeps the
  listener local; `"0.0.0.0"` listens on all local IPv4 interfaces.
- `port`: Port from 1 through 65535.
- `context`: Typed value retained for this server; its type determines `T`.

**Returns:** A contextual server configured for the specified address.
**Async:** No; binding happens when `await server.listen()` runs. Invalid
address and port values are returned by `listen()` as `Error(ServerError)`.
Hostnames and IPv6 are unsupported.

## Ordinary route methods

See the [worked playbook guide](../guides/HTTP_PLAYBOOK.md#handle-requests-and-responses).

Register routes before starting the listener. Each handler is a named async
function with this signature:

```panack
async handler(request: Request,
    response: ResponseWriter): Result[Unit,Str]
```

Registration is synchronous and returns `Unit`; the handler runs
asynchronously for a matching request.

### `Server.get`

```panack
get(
  path: Str,
  handler: AsyncFn[
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `path`: Exact URI-reference path beginning with `/`.
- `handler`: Named async handler; receives the parsed `Request` and
  request-scoped `ResponseWriter`.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for a matching GET request. **Errors:** Invalid
or duplicate routes are reported by `listen()`; registering after listening
begins traps. A supported method on an unknown path invokes the fallback if registered, or
returns 404 otherwise. A known path registered only for other methods returns
405 with an `Allow` header, without invoking fallback. Matching is case-sensitive and performs no decoding or
normalization.

### `Server.post`

```panack
post(
  path: Str,
  handler: AsyncFn[
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `path`: Exact URI-reference path beginning with `/`.
- `handler`: Named async handler; receives the parsed `Request`, including
  the complete bounded request body on `request.body`.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for a matching POST request. **Errors:** Invalid
or duplicate routes are reported by `listen()`; registering after listening
begins traps. The complete request body is buffered before the handler runs.

### `Server.put`

```panack
put(
  path: Str,
  handler: AsyncFn[
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `path`: Exact URI-reference path beginning with `/`.
- `handler`: Named async handler with the ordinary handler signature.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for a matching PUT request. **Errors:** Invalid
or duplicate routes are reported by `listen()`; registering after listening
begins traps. A path registered for another method receives 405 and an
`Allow` header.

### `Server.delete`

```panack
delete(
  path: Str,
  handler: AsyncFn[
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `path`: Exact URI-reference path beginning with `/`.
- `handler`: Named async handler with the ordinary handler signature.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for a matching DELETE request. **Errors:** Invalid
or duplicate routes are reported by `listen()`; registering after listening
begins traps. A path registered for another method receives 405 and an
`Allow` header.

### `Server.fallback`

```panack
fallback(
  handler: AsyncFn[
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `handler`: Named async handler called for otherwise unmatched supported
  methods and paths.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for an unmatched supported request. **Errors:** A
second fallback is reported by `listen()`; registering after listening begins
traps. Unsupported methods and 405 cases do not reach the fallback.

## Contextual route methods

See the [worked playbook guide](../guides/HTTP_PLAYBOOK.md#share-application-configuration).

Contextual routes use the same matching and registration rules as ordinary
routes. The stored context is passed to the handler first:

```panack
async handler(context: AppConfig, request: Request,
    response: ResponseWriter): Result[Unit,Str]
```

The handler type is
`AsyncFn[T,Request,ResponseWriter,Result[Unit,Str]]`.

### `ContextualServer.get`

```panack
get(
  path: Str,
  handler: AsyncFn[
    T,
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `path`: Exact URI-reference path beginning with `/`.
- `handler`: Named async contextual handler; receives `(context, request,
  response)`.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for a matching GET request. **Errors:** Invalid
or duplicate routes are reported by `listen()`; late registration traps.

### `ContextualServer.post`

```panack
post(
  path: Str,
  handler: AsyncFn[
    T,
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `path`: Exact URI-reference path beginning with `/`.
- `handler`: Named async contextual handler; it receives the shared context
  first and the complete bounded request body on `request.body`.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for a matching POST request. **Errors:** Invalid
or duplicate routes are reported by `listen()`; late registration traps. The
complete request body is buffered before the handler runs.

### `ContextualServer.put`

```panack
put(
  path: Str,
  handler: AsyncFn[
    T,
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `path`: Exact URI-reference path beginning with `/`.
- `handler`: Named async contextual handler; receives `(context, request,
  response)`.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for a matching PUT request. **Errors:** Invalid
or duplicate routes are reported by `listen()`; late registration traps.

### `ContextualServer.delete`

```panack
delete(
  path: Str,
  handler: AsyncFn[
    T,
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `path`: Exact URI-reference path beginning with `/`.
- `handler`: Named async contextual handler; receives `(context, request,
  response)`.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for a matching DELETE request. **Errors:** Invalid
or duplicate routes are reported by `listen()`; late registration traps.

### `ContextualServer.fallback`

```panack
fallback(
  handler: AsyncFn[
    T,
    Request,
    ResponseWriter,
    Result[Unit, Str]
  ]
): Unit
```

- `handler`: Named async contextual handler for unmatched supported methods
  and paths; receives `(context, request, response)`.

**Returns / async:** Returns `Unit`; registration is synchronous and the
handler runs asynchronously for an unmatched supported request. **Errors:** A
second fallback is reported by `listen()`; unsupported methods and 405 cases
do not reach it.

## Lifecycle methods

`ServerError` is a public record with one field:

```panack
record ServerError { message: Str }
```

Its message is for people, not a stable machine-readable error code. All
lifecycle methods below are async and return
`Result[Unit,ServerError]`; await them and handle both result cases.

### `Server.listen`

```panack
async listen(): Result[Unit,ServerError]
```

**Returns:** `Ok(())` when the listener is bound, or `Error(ServerError)` if
route setup, address, port, or listener setup fails. **Lifecycle:** Resolves
when ready, not when the server stops. An active listener keeps the native
process alive. A closed or destroyed server cannot be restarted.

### `ContextualServer.listen`

```panack
async listen(): Result[Unit,ServerError]
```

**Returns:** The same result as `Server.listen`; recorded route, address and
port errors and listener failures are returned as `Error(ServerError)`.
**Lifecycle:** Resolves when ready. An active listener keeps the native process
alive, and a stopped server cannot be restarted.

### `Server.close`

```panack
async close(): Result[Unit,ServerError]
```

**Returns:** `Ok(())` when shutdown completes, or `Error(ServerError)` on an
operation failure. **Lifecycle:** Stops new requests and allows active work to
drain for up to one second, then cancels remaining work and releases
resources. Idempotent. Normal process exit and termination signals also use
bounded cleanup.

### `ContextualServer.close`

```panack
async close(): Result[Unit,ServerError]
```

**Returns:** `Ok(())` when shutdown completes, or `Error(ServerError)` on an
operation failure. **Lifecycle:** Stops new requests, drains active
contextual handlers and response writes for up to one second, then cancels
remaining work and releases resources. Idempotent. Normal process exit and
termination signals also use bounded cleanup.

### `Server.destroy`

```panack
async destroy(): Result[Unit,ServerError]
```

**Returns:** `Ok(())` when resources are released, or `Error(ServerError)` on
an operation failure. **Lifecycle:** Stops new requests and cancels active
work immediately without a graceful drain. Idempotent; a destroyed server
cannot be restarted.

### `ContextualServer.destroy`

```panack
async destroy(): Result[Unit,ServerError]
```

**Returns:** `Ok(())` when resources are released, or `Error(ServerError)` on
an operation failure. **Lifecycle:** Stops new requests and immediately
cancels active contextual handlers. Idempotent; a destroyed server cannot be
restarted.

## Handler inputs and responses

### Request fields

- `method`: The HTTP method.
- `path`: The raw request path.
- `query`: The raw query string without the leading question mark.
- `headers`: Headers in request order. `request.header(name)` returns the
  first case-insensitive match; `header_values(name)` preserves repeated
  values.
- `body`: The buffered request body bytes.

### ResponseWriter

`ResponseWriter` starts with status 200. A handler can:

- Set the status with `status`.
- Replace a header with `header`, or append a repeated header with
  `add_header`.
- Finish the response once with `send` or `send_bytes`.

Text responses default to UTF-8 plain text; byte responses default to
`application/octet-stream`. The server supplies framing headers and closes
each connection after one response.

```panack
async echo(request: http.Request,
    response: http.ResponseWriter): Result[Unit,Str] {
  response.send_bytes(request.body)
  Ok(())
}
```

## Routing and errors

See the [worked playbook guide](../guides/HTTP_PLAYBOOK.md#routes-and-fallbacks).

Paths must begin with `/` and be valid URI references. Query and fragment
components are validated but ignored during matching. Matching is
case-sensitive and uses the raw path: percent escapes, repeated slashes and
dot segments are not decoded or normalized. A repeated method/path pair, an
invalid route, or a second fallback records a setup error; the original route
remains in place. `listen()` reports setup errors without binding. Registering
routes after listening starts is a runtime error.

Only GET, POST, PUT and DELETE reach handlers. Unsupported methods receive
405; for paths with registered routes, `Allow` lists supported methods in
canonical order. A supported method with no exact route on a path that has
other methods also receives 405. An unmatched path invokes the fallback when
present, or returns 404 otherwise. Unsupported methods never reach fallback.

A handler returning `Error(message)`, returning without sending, or trapping
with a recoverable request-scoped fault is logged and produces a generic 500
unless the response has already been sent. The diagnostic is not returned as
a `ServerError` or exposed in the response. A failed request does not stop
healthy requests.

## Fixed limits

The API has no tuning parameters. The implementation enforces these bounds:

- At most 16 open connections and 4 admitted requests at once.
- Request and response headers are limited to 16 KiB each.
- Request and response bodies are limited to 1 MiB each.
- A request has a 10-second accept-to-completion deadline.
- `close()` drains active work for up to one second before cancellation.

Request bodies are buffered before handlers run. Capacity overflow receives
503, oversized request bodies receive 413, and oversized request headers
receive 431. Invalid or oversized application responses become a generic 500
if no response has been committed.

This API has no TLS server, WebSocket, HTTP/2, streaming response, keep-alive,
route parameters, middleware, or closure-capture support.

## Runnable programs

Both examples are complete Panackelty packages with manifests and use the
matching toolchain version. Run from the repository root; both bind to loopback and accept
Ctrl-C for shutdown:

```sh
./bin/panack run --project examples/network/http_server/panack.json 8080
./bin/panack run --project examples/network/http_contextual_server/panack.json 8081
```

The [ordinary example README](../../examples/network/http_server/README.md)
shows GET and POST routes, requests to try, expected responses, and links to
the source and manifest. The
[contextual example README](../../examples/network/http_contextual_server/README.md)
demonstrates typed shared configuration and fallback routing. The integration
suite runs both checked-in examples from source and saved bytecode.

## Source and test evidence

This reference follows the exported declarations in
[`packages/http/src/server.panack`](../../packages/http/src/server.panack),
route policy in
[`routing.panack`](../../packages/http/src/routing.panack), and runtime limits
in [`tcp_server.c`](../../src/vm/tcp_server.c). Behaviour is checked by
[`tests/http_package.sh`](../../tests/http_package.sh),
[`http_state.c`](../../tests/unit/vm/http_state.c), and the HTTP package tests
described in the [test guide](../../tests/README.md).

This is the first Server reference page for review. It does not extend the
format to other HTTP types or the wider standard library.
