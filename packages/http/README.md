# HTTP package

Native **0.1.0-alpha.13** includes `panackelty.http` version `0.1.0`.
The alpha.14 development branch replaces the transport-oriented server API with
a routed HTTP server framework. The HTTP/HTTPS client remains unchanged. To serve files, start with the [static HTTP server tool](../../tools/http-server/README.md).

## Add the dependency

In your application's `panack.json`, give `dependencies.http.path` the relative
path from that manifest to this package. In an extracted release the package is
at `panackelty/packages/http`. Keep the complete package directory; the manifest
and exported modules are required. The application and package must use the
matching `0.1.0-alpha.13` toolchain.

For example, an application beside the extracted `panackelty/` directory uses:

```json
{"schema":1,"package":"my.http_app","version":"0.1.0","language":"modules-v1","toolchain":"0.1.0-alpha.13","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../panackelty/packages/http"}}}
```

Run it with `panack run --project path/to/app/panack.json`. There is no package
registry lookup or implicit dependency download.

## Exported modules

| Import | Purpose | API source |
| --- | --- | --- |
| `http/types` | `Header`, `Request`, `Response`, typed `HttpError` | [Types](src/types.panack) |
| `http/client` | `get(url)` and `get_with_limits(url, limits)` | [Client](src/client.panack) |
| `http/server` | Routed servers, requests and response writers | [Server](src/server.panack) |

### Client

Import `http/client` with an alias and call `await client.get(url)` inside an
async function. It returns `Result[Response,HttpError]`; handle both cases.
`Response` contains the status, headers and body bytes. `ClientLimits` sets
`timeout_ms`, `header_limit` and `body_limit` for `get_with_limits`.

The client supports bounded HTTP/1.1 GET and verified HTTPS. The native runtime
uses libcurl; Linux needs its distribution's runtime library (`libcurl4` on
Ubuntu 22.04). TLS verification is enabled. See the
[client contract](../../docs/design/NATIVE_HTTP_CLIENT_DESIGN.md) for URL,
redirect, framing and size restrictions.

### Server (alpha.14 development branch)

See the complete [HTTP server reference](../../docs/reference/http-server.md)
for constructor and method signatures, routing, handler errors, lifecycle and
fixed limits. The runnable ordinary and contextual programs link back to that
reference.

Import `http/server as http`. Create a server, register named async handlers, then
`await server.listen()`. Successful listening means the socket is bound; the
listener keeps the native process alive after `main` returns.

```panackelty
import http/server as http
async home(request: http.Request, response: http.ResponseWriter): Result[Unit,Str] {
  response.send("Hello")
  Ok(())
}
async main(): Unit {
  server = http.create_bound_server("127.0.0.1", 8080)
  server.get("/", @home)
  match await server.listen() {
    Ok(done) => (),
    Error(problem) => ()
  }
}
```

The [runnable example](../../examples/network/http_server/README.md) also reports
startup errors and accepts a port argument. For shared typed configuration, use
`create_bound_contextual_server(address, port, context)` and handlers shaped as
`AsyncFn[T,Request,ResponseWriter,Result[Unit,Str]]`; see the
[contextual example](../../examples/network/http_contextual_server/README.md).
`create_server(port)` and `create_contextual_server(port, context)` bind
`0.0.0.0`, accepting traffic on every IPv4 interface. Use the bound constructors
with `127.0.0.1` for local-only services.

Register exact paths with `.get`, `.post`, `.put`, `.delete`, or set `.fallback`.
There are no route parameters, middleware or captured closures. Registration is
completed before listening; invalid or duplicate registrations make setup fail.
GET, POST, PUT and DELETE are supported. HEAD and other methods receive 405 and
do not invoke handlers. HEAD errors carry no body. Missing routes receive 404.

`Request` contains `method`, raw `path`, raw `query` (without `?`), ordered
`headers` and buffered body bytes. Paths are not URL-decoded. `.header(name)`
returns the first matching value; `.header_values(name)` preserves duplicates.
Header lookup is ASCII case-insensitive.

`ResponseWriter` provides `.status`, `.header`, `.add_header`, `.send` and
`.send_bytes`. Status defaults to 200. Header replacement is case-insensitive;
`add_header` preserves duplicates. Text defaults to UTF-8 plain text and bytes to
`application/octet-stream`. Header names are ASCII tokens; values permit printable
ASCII and tabs under the package policy. The framework owns framing headers. The first send
commits the response; later mutations cannot replace it. Status 204 and 304 must
have empty bodies. Returning without sending, returning `Error`, or a recoverable
request fault produces a bounded 500 unless a response was already committed.
A failed request does not terminate healthy requests.

The fixed bounds are 16 accepted connections, four admitted requests, 16 KiB of
request headers, 1 MiB request and response bodies, 16 KiB response headers, and a
10-second accept-to-completion deadline. Content-Length and chunked bodies are
buffered before the handler runs; valid `Expect: 100-continue` is handled by the
framework. Each connection carries one response and closes. There is no keep-alive,
TLS server, WebSocket, streaming response or HTTP/2 support.

`await server.close()` stops admission and drains existing requests for up to one
second; `await server.destroy()` cancels immediately. Native SIGINT/SIGTERM and
explicit process exit perform bounded cleanup. A closed server cannot be restarted.
The old `http/service` export, finite HTTP sessions, connection contexts and public
serializer helpers have been removed. Low-level TCP APIs remain separate.
