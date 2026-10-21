# HTTP server playbook

This playbook walks through a working HTTP server, from sending your first
request to sharing settings between handlers and shutting the server down.
Each example includes the complete code, so you can read it here or try it with
your alpha.14 release package. For individual methods and their options, see the
[API reference](../reference/http-server.md).

## Your first server

### Open your alpha.14 release package

Use the alpha.14 release package for your operating system and processor. Unpack
it and open a terminal in the extracted `panackelty` directory. The package
includes the compiler, runtime, standard library, HTTP package and the examples
used in this playbook. You do not need Git, Make, a C compiler or Xcode tools.

Check that you are using the supplied compiler:

```sh
./bin/panack --version
```

The version should be `0.1.0-alpha.14`. All commands below run from this extracted
directory, which contains `bin`, `examples` and `packages`. Using `./bin/panack`
keeps the compiler and bundled packages together, even if another version is
installed on your machine. Use `curl` in a second terminal to try the requests.

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

When you've finished, press Ctrl-C in the terminal running the server. If
another program is already using port 8080, use 8083 in both the server command
and the curl request. If the server can't start, it writes an error message to
stderr and exits with status 1.

The example listens on `127.0.0.1`, so you can only connect from the same machine.
To try it from another device on your network, change the address passed to
`create_bound_server` to `0.0.0.0`. Then connect using the server machine's network
IP address and port, for example `http://192.168.1.10:8080/`. Replace that example
IP address with your machine's address; don't put `0.0.0.0` in your browser.

### The complete program

Here's the complete server program. The next two sections explain how its routes
handle requests, responses and fallbacks.

If you'd like to edit and run it, you'll find this example in your extracted
release package at `examples/network/http_server/src/main.panack`.

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

The import lets us refer to the HTTP module as `http`. In `server.get("/", @home)`,
`@home` tells the server which function to call when a GET request arrives at `/`.
That function sends the response with `response.send(...)`, then returns `Ok(())`
to report that it finished successfully. The `()` inside `Ok` is the Unit value;
the response body is the text passed to `send`.

Once the routes are registered, `await server.listen()` starts the listener and
returns when it's ready to accept connections. The server continues running
after `main` finishes, so it can keep answering requests.

### The project manifest

The example also includes a `panack.json` file, which tells Panackelty where to
find the program and its dependencies. Here's the manifest from
`examples/network/http_server/panack.json`:

```json
{
  "schema": 1,
  "package": "examples.http_server",
  "version": "0.1.0",
  "language": "modules-v1",
  "toolchain": "0.1.0-alpha.14",
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

Together, `source` and `entry` point to `src/main.panack`. The `http` dependency
points to the HTTP package bundled with the release. Its path is relative to
`panack.json`, so if you copy the example to another directory, copy the manifest
too and update the dependency path to match the HTTP package's location.

## Handle requests and responses

The first server includes an `/echo` route that sends back whatever you put in
the request body. To try it, leave that server running and send this request
from a second terminal:

```sh
curl -i --data 'Hello, Ada' http://127.0.0.1:8080/echo
```

You'll get a `200 OK` response with `Hello, Ada` as its body and
`application/octet-stream` as its content type. The handler uses `send_bytes`
to return the bytes unchanged. For text, use `send`, which takes a `Str` and
uses `text/plain; charset=utf-8` unless you set another content type.

The server holds each request and response body in memory in full. It doesn't
stream the body in chunks.

The `/inspect` route shows how to read a header and the query string. This
request supplies an `X-Caller` header and puts `name=Ada` after the `?` in the URL:

```sh
curl -i -H 'X-Caller: guide' 'http://127.0.0.1:8080/inspect?name=Ada'
```

The response has status `200 OK`, an `X-Query: name=Ada` header and the body
`Caller: guide`. Leave out `X-Caller` and the body becomes `Caller: anonymous`.

A header may be missing, so `request.header(...)` returns `Option[Str]`. The
handler uses `Some` for a supplied value and `None` for the default greeting.
Header names aren't case-sensitive. If a request contains the same header more
than once, use `request.header_values(name)` to get all its values.

`request.query` contains the text after the `?`, without the `?` itself. It
doesn't split that text into named parameters or decode escaped characters.

Set the response status and headers before you call `send` or `send_bytes`.
`response.header(name, value)` replaces any existing value for that header;
`response.add_header(name, value)` adds another value without replacing it.

Each handler must send one response. If it returns without sending anything,
the server returns a generic 500 error. Leave `Content-Length`,
`Transfer-Encoding` and `Connection` to the server; these headers control how
the response is delivered and mustn't be set by your handler.

## Routes and fallbacks

A route matches both an HTTP method and a path. In the first example, POST, PUT
and DELETE requests to `/echo` all call the same handler, which sends back the
request body. Using PUT or DELETE doesn't change any data by itself: your
handler decides what each request does.

With the first server running, try these requests in another terminal:

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

The fallback decides what to send when no route matches the path. In this
example, `not_found` explicitly sets status 404. Registering a fallback doesn't
set that status for you. If you don't register one, the server sends its own 404
response for an unknown path. Unsupported methods, such as PATCH, receive 405
without calling the fallback.

Paths must match exactly, including capitalisation and trailing slashes, so
`/echo/` is a different path from `/echo`. The query string doesn't affect which
route is chosen. Wildcards and parameters such as `:id` aren't supported.
If you register the same method and path twice, `listen()` reports the error
when you start the server.

## Share application configuration

Handlers often need settings that belong to the application, such as a greeting
or a service name. A contextual server lets you pass those settings to every
handler. You can put several values in a record and pass that record as the
context, so you aren't limited to one setting.

The example below uses an `AppContext` record with a greeting and a service name.
Each handler receives this context alongside the incoming request and the response
writer. There's no need to store the settings in global variables.

To try the bundled example, start it with the following command, then run the
curl request in a second terminal:

```sh
./bin/panack run --project examples/network/http_contextual_server/panack.json 8081
curl -i 'http://127.0.0.1:8081/health?probe=1'
```

The curl request shows the response headers followed by the body:

```http
HTTP/1.1 200 OK
X-Service: preview
X-Query: probe=1
Content-Type: text/plain; charset=utf-8
Content-Length: 9
Connection: close

OK: Hello
```

`X-Service` comes from the application's context, while `X-Query` contains the
query string from this request. A GET request to `/` returns `Hello`. Press
Ctrl-C in the server terminal when you've finished.

Here's the full program. To edit it, look for
`examples/network/http_contextual_server/src/main.panack` in your release package:

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

The accompanying manifest is shown below. It's included at
`examples/network/http_contextual_server/panack.json`:

```json
{
  "schema": 1,
  "package": "examples.http_contextual_server",
  "version": "0.1.0",
  "language": "modules-v1",
  "toolchain": "0.1.0-alpha.14",
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

Every handler on this server takes `AppContext` as its first argument, followed
by the request and response writer. The compiler works out the context type from
the value passed to `create_bound_contextual_server` and checks that the handlers
accept it.

The address `127.0.0.1` keeps this example accessible only from the same machine.
You can also use `create_contextual_server(port, context)`, which listens on all
local IPv4 interfaces.

These settings stay unchanged while the server runs. If you need handlers to
update shared data or use a database connection pool, you'll also need to decide
how concurrent access is managed. Passing a context doesn't handle that for you.

## Own the listener lifecycle

For the examples above, Ctrl-C is enough to stop the server. Sometimes your
program needs to stop a server itself, perhaps because a test has finished or
a larger application is shutting down. There are two methods for this:

- `close()` stops accepting new work and gives running handlers and response writes up to one second to finish. It then cancels anything still running.
- `destroy()` cancels active work immediately, without waiting for it to finish.

You can call either method again after the server has stopped. These repeated
calls are safe, but they don't restart the server. To listen again, create a
new server.

The following program starts a server and immediately closes it, as you might
do in a test. It won't stay running for you to visit in a browser. It also calls
`destroy()` after `close()` to show that calling it on a stopped server is safe;
in your own application, you'd normally choose one or the other.

The full code is below. You'll also find it in the release package at
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

Here's the accompanying manifest, included at
`examples/network/http_server_lifecycle/panack.json`:

```json
{
  "schema": 1,
  "package": "examples.http_server_lifecycle",
  "version": "0.1.0",
  "language": "modules-v1",
  "toolchain": "0.1.0-alpha.14",
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

The program prints these messages and then exits:

```text
Server closed
Server destroyed
```

If you need to stop immediately, use `await server.destroy()` and handle its
result in the same way as `close()`. Contextual servers use these same methods
and return types.

If starting or stopping the server fails, the result is `Error(ServerError)`.
Use its `message` to explain the problem to the user, but don't make your
program's behaviour depend on the exact wording of that message.

## Serve a directory

If you want to serve HTML, stylesheets or other files, the release includes
`panack-http-server`. It uses the same routed server framework, with a static
file handler that keeps requests inside the directory you choose.

From your extracted release directory, create a small page and start the tool:

```sh
mkdir -p demo-site
printf '<h1>Hello from Panackelty</h1>\n' > demo-site/index.html
./bin/panack-http-server --root ./demo-site --port 8084 --no-cache --noindex
```

Once it prints `Serving http://127.0.0.1:8084/`, open that address in a browser.
You'll see “Hello from Panackelty”. You can also request the page from another
terminal:

```sh
curl http://127.0.0.1:8084/
```

```html
<h1>Hello from Panackelty</h1>
```

Changes to the file appear on the next request. Press Ctrl-C in the server
terminal to stop it. The tool waits up to one second for active requests to
finish before cancelling any that remain.

The default address is `127.0.0.1`. Add `--host 0.0.0.0` to accept connections
from other devices, using your machine's network address to connect.
`--no-cache` adds `Cache-Control: no-store`; `--noindex` asks search engines not
to index the pages. Neither option is an access control.

The tool serves regular files with appropriate content types and uses
`index.html` for directories. It doesn't list directory contents. Missing files
return 404, and paths that escape the chosen root are rejected. GET is supported;
other methods don't serve file content. Responses, including their headers, must
fit within 1 MiB. Larger files return 503. There is no TLS, compression, range
support or automatic browser reload.

### Use static serving in your application

For a program that needs to start its own directory server, add a `static_http`
dependency pointing to `packages/static_http` in the release and import
`static_http/service`. Its `serve` function registers a contextual fallback on
the routed HTTP server. The command above is the ready-to-run version of this
integration.

```panack
async serve(
  configuration: Config,
  ready: AsyncFn[Endpoint,Result[Unit,Str]]
): Result[Unit,Str]
```

`Config` contains `root: Path`, `address: Str`, `port: Nat`,
`request_body_limit: Nat`, `response_limit: Nat` and `headers: [Header]`.
`Endpoint` contains the bound `address: Str` and `port: Nat`.
The ready callback runs after the root has been validated and the socket bound;
returning an error stops the server. A successful call returns after readiness,
while the listener keeps the process running. The public function doesn't return
a server handle.

The tool uses a 16 KiB request-body limit and a 1 MiB response limit. Application
limits must fit within the framework's 1 MiB body limits; the response limit must
be greater than 512 bytes and leave room for headers. Extra headers cannot
override framing headers, `Content-Type`, `Location` or `Allow`. Invalid
configuration returns an error before the listener starts.

## Before building a larger service

Before using these examples in a larger service, check that the server's limits
fit what you need. It allows up to 16 open connections and handles up to four
requests at a time. Request and response headers are limited to 16 KiB each,
and request and response bodies to 1 MiB each.

Each connection has ten seconds from being accepted to completing its response,
and closes after that one response. The server doesn't yet support TLS,
WebSocket, middleware, streaming responses or HTTP/2.

The [reference](../reference/http-server.md#fixed-limits) explains these limits
and the errors you'll receive when they're exceeded.
