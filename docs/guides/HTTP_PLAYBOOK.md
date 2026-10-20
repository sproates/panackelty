# HTTP server playbook

Build a local server, try real requests, and then adapt the handlers to your application.
The [API reference](../reference/http-server.md) lists signatures, return values and limits.

## Your first server

### Get the matching toolchain

Use Linux or macOS with Git, Make, a C compiler and libcurl development headers
(including `curl-config`). On Ubuntu, install `build-essential`, `git`, `curl`
and `libcurl4-openssl-dev`. On macOS, install the Xcode command-line tools; check
that `cc`, `make` and `curl-config` are available in your terminal.

In a new working directory:

```sh
git clone --branch next https://github.com/sproates/panackelty.git
cd panackelty
make
```

All commands below run from this checkout's root. If you already have a matching
checkout, use that instead. Use `./bin/panack`, so a different installed release
cannot accidentally run the examples.

### Run it before changing anything

```sh
./bin/panack run --project examples/network/http_server/panack.json 8080
```

The terminal prints `Listening on http://127.0.0.1:8080/` and stays running.
In a second terminal:

```sh
curl http://127.0.0.1:8080/
```

Expected body:

```text
hello from Panackelty HTTP server
```

Stop with Ctrl-C in the server terminal. If port 8080 is occupied, replace it
with 8083 in both commands. A startup error is printed on stderr and the program
exits with status 1. This server listens on `127.0.0.1`: only your machine can
connect. Explicitly choose `0.0.0.0` if you intend to accept connections through
other local network interfaces; it is a listening address, not a browser URL.

### The complete program

The file is `examples/network/http_server/src/main.panack`. This single example
also supplies the routes used in the next two guides. You can read every line here:

```panack
import http/server as http
import stdlib/async_host as host

async home(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("hello from Panackelty HTTP server\n")
  Ok(())
}

async echo(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send_bytes(request.body)
  Ok(())
}

async inspect_request(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  caller = match request.header("X-Caller") {
    Some(value) => value,
    None() => "anonymous"
  }
  response.header("X-Query", request.query)
  response.send("Caller: " + caller)
  Ok(())
}

async not_found(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.status(404)
  response.send("No route for " + request.path)
  Ok(())
}

async main(): Unit {
  arguments = await host.arguments()
  port = if arguments.len() == 0 { 8080 } else { arguments[0].parse_nat() }
  server = http.create_bound_server("127.0.0.1", port)
  server.get("/", @home)
  server.post("/echo", @echo)
  server.put("/echo", @echo)
  server.delete("/echo", @echo)
  server.get("/inspect", @inspect_request)
  server.fallback(@not_found)
  match await server.listen() {
    Ok(done) => { ignored = await host.stdout("Listening on http://127.0.0.1:${port}/"); () },
    Error(problem) => { ignored = await host.stderr(problem.message); await host.exit(1) }
  }
}
```

`import http/server as http` names the HTTP module. `@home` passes a named
function as a handler; the server calls it when a matching request arrives.
`response.send(...)` finishes the response. `Ok(())` reports successful handler
completion: `()` is the Unit value, not the HTTP response body.

Register the routes first, then `await server.listen()`. That call resolves once
the listener is ready; it does not wait until the server stops. The active
listener keeps the process alive after `main` returns.

### The project manifest

The existing file is `examples/network/http_server/panack.json`:

```json
{"schema":1,"package":"examples.http_server","version":"0.1.0","language":"modules-v1","toolchain":"0.1.0-alpha.13","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
```

`source` and `entry` locate the program. The `http` dependency resolves relative
to this manifest to the checkout's `packages/http` directory. If you move the
project, update that relative dependency path. Copying just `main.panack` into
an arbitrary directory is not sufficient to resolve the package import.

## Handle requests and responses

Start the ordinary program from the first guide. Its `/echo` handler receives
`request.body` as bytes and sends the same bytes back:

```sh
curl -i --data 'Hello, Ada' http://127.0.0.1:8080/echo
```

Expect status `200 OK`, content type `application/octet-stream`, and body
`Hello, Ada`. `send_bytes` preserves bytes; `send` accepts a `Str` and defaults
to `text/plain; charset=utf-8`. Bodies are fully buffered, not streamed.

The `/inspect` handler reads an optional header and the raw query string:

```sh
curl -i -H 'X-Caller: guide' 'http://127.0.0.1:8080/inspect?name=Ada'
```

Expect status `200 OK`, header `X-Query: name=Ada`, and body `Caller: guide`.
Without `X-Caller`, the body is `Caller: anonymous`. Header lookup is
case-insensitive and returns `Option[Str]`, so the handler handles `Some` and
`None`. `request.query` excludes the `?` and is not a decoded parameter map.
For repeated request headers, `request.header_values(name)` returns all matches.

Set status and headers before calling `send` or `send_bytes`. Use
`response.header(name, value)` to replace a header and `response.add_header(name,
value)` to append one. Send exactly once. The server supplies `Content-Length`,
`Transfer-Encoding` and `Connection`; applications must not set these framing
headers. A handler that returns without sending produces a generic 500.

## Routes and fallbacks

The ordinary program registers POST, PUT and DELETE on `/echo`. All three use
the same byte-echo handler; this demonstrates dispatch, **not persistence or a
resource store**. The HTTP verb does not automatically update or delete data.

```sh
curl -i -X PUT --data 'replacement' http://127.0.0.1:8080/echo
curl -i -X DELETE http://127.0.0.1:8080/echo
curl -i http://127.0.0.1:8080/missing
curl -i http://127.0.0.1:8080/echo
```

- PUT returns 200 with body `replacement`.
- DELETE returns 200 with an empty body.
- GET `/missing` invokes `not_found`, which sets 404 and sends `No route for /missing`.
- GET `/echo` returns 405 and `Allow: POST, PUT, DELETE`. The path exists, but GET is not registered. The fallback is not called.

A fallback is an application handler, so it must choose its own status; registering
one does not automatically make its response a 404. Without a fallback, an unknown
path receives the server's default 404. Unsupported methods such as PATCH receive
405 and never enter a fallback.

Paths are exact and case-sensitive. `/echo/` differs from `/echo`. Query strings
do not affect route selection; no wildcard or `:id` route parameters are supported.
Registering the same method/path twice is a setup error reported by `listen()`.

## Share application configuration

Use a contextual server when several handlers need application values. For
example, a preview service needs a greeting and a service name. Put both in one
record; one context argument can contain multiple settings.

The context is supplied by your application. The `Request` is supplied for each
incoming request. Keeping these separate avoids relying on globals for configuration.

Run the complete example:

```sh
./bin/panack run --project examples/network/http_contextual_server/panack.json 8081
curl -i 'http://127.0.0.1:8081/health?probe=1'
```

Run curl in a second terminal. Expect status 200, `X-Service: preview`,
`X-Query: probe=1`, and body `OK: Hello`. GET `/` returns `Hello`.
Stop the server with Ctrl-C.

The file is `examples/network/http_contextual_server/src/main.panack`:

```panack
import http/server as http
import stdlib/async_host as host

record AppContext {
  greeting: Str,
  service_name: Str
}

async home(context: AppContext, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send(context.greeting)
  Ok(())
}

async health(context: AppContext, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.header("X-Service", context.service_name)
  response.header("X-Query", request.query)
  response.send("OK: " + context.greeting)
  Ok(())
}

async fallback(context: AppContext, request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.status(404)
  response.send("No route for " + request.path)
  Ok(())
}

async main(): Unit {
  arguments = await host.arguments()
  port = if arguments.len() == 0 { 8081 } else { arguments[0].parse_nat() }
  server = http.create_bound_contextual_server("127.0.0.1", port, AppContext("Hello", "preview"))
  server.get("/", @home)
  server.get("/health", @health)
  server.fallback(@fallback)
  match await server.listen() {
    Ok(done) => { ignored = await host.stdout("Listening on http://127.0.0.1:${port}/"); () },
    Error(problem) => { ignored = await host.stderr(problem.message); await host.exit(1) }
  }
}
```

Its manifest, `examples/network/http_contextual_server/panack.json`:

```json
{"schema":1,"package":"examples.http_contextual_server","version":"0.1.0","language":"modules-v1","toolchain":"0.1.0-alpha.13","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
```

Every registered handler receives `AppContext` first, then the request and
response writer. The compiler infers the context type from the value passed to
`create_bound_contextual_server`; all handlers on that server must accept it.
The explicit address keeps this example on loopback. The shorter
`create_contextual_server(port, context)` uses all local IPv4 interfaces.

This example uses configuration values. It does not demonstrate a mutable shared
store or a concurrent database pool; passing context alone does not establish a
synchronization policy for those use cases.

## Own the listener lifecycle

For a normal service, keep the listener running and use Ctrl-C to stop it. For a
test or an embedding application that owns the listener lifetime, explicitly
await shutdown when its work is done:

- `close()` stops accepting new work and allows active handlers and writes up to one second to finish, then cancels what remains.
- `destroy()` cancels active work immediately. Use it when graceful draining is not appropriate.
- Both are idempotent. A stopped handle cannot restart; create a new server instead.

Here is a complete lifecycle demonstration. It starts and immediately stops a
server, as a test might do. It deliberately does not wait for browser requests.
The final `destroy()` demonstrates an idempotent call after `close()`; ordinary
applications normally choose one shutdown method.

`examples/network/http_server_lifecycle/src/main.panack`:

```panack
import http/server as http
import stdlib/async_host as host

async home(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("Hello")
  Ok(())
}

async main(): Unit {
  arguments = await host.arguments()
  port = if arguments.len() == 0 { 8082 } else { arguments[0].parse_nat() }
  server = http.create_bound_server("127.0.0.1", port)
  server.get("/", @home)

  match await server.listen() {
    Error(problem) => {
      ignored = await host.stderr(problem.message)
      await host.exit(1)
    },
    Ok(done) => ()
  }

  // A test or embedding application can do its work here, then stop the listener.
  // This example stops immediately; it is not a long-running web service.
  match await server.close() {
    Error(problem) => {
      ignored = await host.stderr(problem.message)
      await host.exit(1)
    },
    Ok(done) => { ignored = await host.stdout("Server closed"); () }
  }

  // Safe after close too: destroy is idempotent and releases any remaining work.
  match await server.destroy() {
    Error(problem) => {
      ignored = await host.stderr(problem.message)
      await host.exit(1)
    },
    Ok(done) => { ignored = await host.stdout("Server destroyed"); () }
  }
}
```

`examples/network/http_server_lifecycle/panack.json`:

```json
{
  "schema": 1,
  "package": "examples.http_server_lifecycle",
  "version": "0.1.0",
  "language": "modules-v1",
  "toolchain": "0.1.0-alpha.13",
  "source": "src",
  "entry": "main.panack",
  "exports": {},
  "dependencies": {
    "http": {
      "path": "../../../packages/http"
    }
  }
}
```

```sh
./bin/panack run --project examples/network/http_server_lifecycle/panack.json 8082
```

Expected output, followed by process exit:

```text
Server closed
Server destroyed
```

Use the same result-handling pattern with `await server.destroy()` instead of
`close()` when immediate cancellation is required. The contextual server has
the same lifecycle methods and return types. A failure to start or shut down
returns `Error(ServerError)`; inspect its `message`, rather than matching exact
error text.

## Before building a larger service

This first version has fixed limits: 16 open connections, 4 admitted requests,
16 KiB request and response headers, 1 MiB request and response bodies, and a
10-second accept-to-completion deadline. It closes each connection after one
response. It has no TLS server, WebSocket, middleware, streaming responses or
HTTP/2 support. See the [reference](../reference/http-server.md#fixed-limits)
for errors and bounds before adapting these examples to production traffic.
