# HTTP server reference (alpha.14 development)

This page documents the routed server API exported by `http/server` in the
alpha.14 development branch. The packaged HTTP client remains a separate API;
see the [HTTP package guide](../../packages/http/README.md). For directory
serving, use the [static HTTP server tool](../../tools/http-server/README.md).

## Import

```panack
import http/server as http
```

The constructors and `ServerError` are exported by `http/server`. `Request`,
`Header`, and `ResponseWriter` are re-exported by the same module.

## Create and start a server

```panack
http.create_server(port: Nat): Server
http.create_bound_server(address: Str, port: Nat): Server
```

`create_server` binds to `0.0.0.0` (all local IPv4 interfaces).
`create_bound_server` accepts a dotted-decimal IPv4 address; use
`"127.0.0.1"` to accept connections from the local machine only. The port must
be in `1..65535`. Invalid addresses and ports are reported when `listen()` is
awaited. Hostnames and IPv6 addresses are not supported by these constructors.

Register routes before starting the listener. Then await `listen()`; it returns
when the socket is ready, while the active listener keeps the native process
alive:

```panack
async home(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
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

The complete [ordinary runnable example](../../examples/network/http_server/README.md)
also accepts a port argument, registers GET and POST routes, and reports startup
errors. For explicit per-server configuration, use a contextual server below.

## Route registration

Each handler is a named async function. Registration methods return `Unit`.
Handlers return `Result[Unit,Str]` and receive the request and response writer:

```panack
async handler(request: http.Request, response: http.ResponseWriter): Result[Unit,Str]
```

| Method | Signature | Behavior |
| --- | --- | --- |
| `get` | `get(path: Str, handler: AsyncFn[Request,ResponseWriter,Result[Unit,Str]]): Unit` | Registers an exact GET route. |
| `post` | `post(path: Str, handler: AsyncFn[Request,ResponseWriter,Result[Unit,Str]]): Unit` | Registers an exact POST route. |
| `put` | `put(path: Str, handler: AsyncFn[Request,ResponseWriter,Result[Unit,Str]]): Unit` | Registers an exact PUT route. |
| `delete` | `delete(path: Str, handler: AsyncFn[Request,ResponseWriter,Result[Unit,Str]]): Unit` | Registers an exact DELETE route. |
| `fallback` | `fallback(handler: AsyncFn[Request,ResponseWriter,Result[Unit,Str]]): Unit` | Registers one fallback for supported methods whose path has no exact route. |

Paths must start with `/` and be valid URI references. Query and fragment
components are validated but ignored during matching. Matching is
case-sensitive and uses the raw path: percent escapes, repeated slashes and dot
segments are not decoded or normalized. A repeated method/path pair, invalid
route, or second fallback records a setup error; the original route remains in
place. `listen()` returns that error without binding. Registering after listening
starts is a runtime error.

Only GET, POST, PUT and DELETE reach handlers. Unsupported methods receive 405;
for paths with registered routes, the `Allow` header lists those methods in
canonical order. A supported method with no route on a path that has other
methods also receives 405. An unmatched path uses the fallback when present, or
receives 404 otherwise. Unsupported methods never reach the fallback.

## Contextual servers

```panack
http.create_contextual_server[T](port: Nat, context: T): ContextualServer[T]
http.create_bound_contextual_server[T](address: Str, port: Nat, context: T): ContextualServer[T]
```

These constructors have the same bind behavior as the ordinary constructors
and retain one typed context for the server's lifetime. Every contextual
handler receives that value first:

```panack
async health(context: AppConfig, request: http.Request,
    response: http.ResponseWriter): Result[Unit,Str] {
  response.send(context.greeting)
  Ok(())
}
```

`ContextualServer[T]` has the same `get`, `post`, `put`, `delete`, `fallback`,
`listen`, `close` and `destroy` methods as `Server`. Its route handlers have
type `AsyncFn[T,Request,ResponseWriter,Result[Unit,Str]]`. Context is explicit
and typed; handlers are named function references and do not capture closures.
See the [contextual runnable example](../../examples/network/http_contextual_server/README.md).

## Listener methods and errors

`ServerError` is a public record with one field:

```panack
record ServerError { message: Str }
```

The message is for people and is not a stable machine-readable error code.
`listen`, `close`, and `destroy` are async and return
`Result[Unit,ServerError]`:

| Method | Signature | Behavior |
| --- | --- | --- |
| `listen` | `async listen(): Result[Unit,ServerError]` | Binds and resolves when ready. Setup, address, port and listener failures are returned as `Error`. |
| `close` | `async close(): Result[Unit,ServerError]` | Stops new requests and allows active work to drain for up to one second before cancellation. |
| `destroy` | `async destroy(): Result[Unit,ServerError]` | Stops new requests and cancels active work immediately. |

Close and destroy are idempotent. A listener that has closed or been destroyed
cannot be restarted. Normal process exit and termination signals perform bounded
cleanup. Handler `Error(message)`, a handler that returns without sending, and
recoverable request-scoped runtime faults are logged and produce a generic 500
unless the response has already been sent. The diagnostic is not returned as a
`ServerError` and is not exposed in the response. A failed request does not stop
healthy requests.

## Fixed server limits

The API has no tuning parameters. The implementation accepts at most 16 open
connections and admits at most 4 requests at once. Request and response bodies
are limited to 1 MiB each; request headers and response headers are limited to
16 KiB each. A request has a 10-second accept-to-completion deadline. Capacity
overflow receives 503, oversized request bodies receive 413, and oversized
request headers receive 431. Invalid or oversized application responses become
a generic 500 if no response has been committed. Request bodies are buffered
before handlers run. Each connection carries one response and then closes.

These bounds, response preservation, fault handling and lifecycle behavior are
covered by the HTTP package tests. There is no TLS server, WebSocket, HTTP/2,
streaming response, keep-alive, route parameter, middleware or closure-capture
support in this API.

## Runnable examples

Both complete examples use the checked-in HTTP package and matching toolchain
version (`0.1.0-alpha.13` in the current development checkout). Run each
command from the repository root:

```sh
./bin/panack run --project examples/network/http_server/panack.json 8080
./bin/panack run --project examples/network/http_contextual_server/panack.json 8081
```

They bind to loopback and accept Ctrl-C for shutdown. Their READMEs include
requests to try, expected responses, source and manifest links, and the
corresponding reference link. The integration suite runs the checked-in examples
from source and saved bytecode.

## Evidence and scope

This page follows the exported declarations in
[`packages/http/src/server.panack`](../../packages/http/src/server.panack),
route policy in [`routing.panack`](../../packages/http/src/routing.panack),
and runtime limits in [`tcp_server.c`](../../src/vm/tcp_server.c). Behavioral
claims are checked by [`tests/http_package.sh`](../../tests/http_package.sh),
[`http_state.c`](../../tests/unit/vm/http_state.c), and the HTTP package tests
described in the [test guide](../../tests/README.md).
This is the first Server reference page for review; it does not extend the
format to other HTTP types or the wider standard library.
