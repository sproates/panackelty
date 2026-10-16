# HTTP package

Native **0.1.0-alpha.13** includes `panackelty.http` version `0.1.0`.
It supplies an HTTP/HTTPS client and HTTP server primitives with explicit resource
limits. To serve files, start with the [static HTTP server tool](../../tools/http-server/README.md).

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
| `http/server` | Finite sessions and connection handlers | [Server](src/server.panack) |
| `http/service` | Continuous service with readiness and shutdown callbacks | [Service](src/service.panack) |

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

### Server

`http/server.serve` is a **finite session**, with a client count, concurrency,
request/response byte limits, client/admission deadlines and shutdown grace.
It supports GET, HEAD and POST; HEAD omits body bytes while retaining
representation headers. The [finite-session fixture](../../examples/network/http_server/README.md)
accepts one client and stops. Its assertion-style failure handling is intended
for regression evidence, not as an application template.

`http/service.serve` remains available until stopped. It requires a named worker,
a typed `ConnectionContext`, `ServiceLimits`, and a post-bind readiness callback.
The caller currently supplies the connection wrapper and error handling; this is
an explicit low-level interface rather than a one-call web application API.
The [static service implementation](../static_http/src/service.panack) is a real
consumer. The [service contract](../../docs/reference/SPEC.md#continuous-native-services-development-toolchain)
documents lifecycle, cancellation and resource ownership.

The server is plain HTTP on numeric IPv4, with bounded request handling. Do not
infer TLS termination, arbitrary streaming protocols or a production web framework
from client HTTPS support.
