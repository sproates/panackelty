# HTTP Client reference

**API stability during alpha:** APIs may change between alpha releases without
advance notice. This reference follows the latest release; earlier alpha versions
aren't maintained. Versioned documentation starts with the first non-alpha release.

Use `http/client` to make HTTP or HTTPS GET requests from a native Panackelty
program. The client checks TLS certificates and hostnames, limits response sizes
and returns errors as values. It isn't available in the browser playground.

For incoming requests, see the [Server Reference](http-server.md) and
[Server Playbook](../guides/HTTP_PLAYBOOK.md).

## Import and dependency

```panack
import http/client as client
```

Add an `http` dependency pointing to the release package's `packages/http`
directory. Dependency paths are relative to your application's `panack.json`.
The client re-exports `Header`, `Response` and `HttpError` from `http/types`.

## get

```panack
async get(url: Str): Result[Response,HttpError]
```

Send one GET request and wait for the response. Call it with `await` from an
async function. `url` must be a supported absolute HTTP or HTTPS URL.

The default timeout is 10 seconds, with up to 64 KiB of response headers and
1 MiB of response body. A valid response returns `Ok(Response)`, including HTTP
errors such as 404 or 500. Check `response.status` to decide what those mean for
your application. Network, URL and protocol failures return `Error(HttpError)`.

## get_with_limits

```panack
async get_with_limits(url: Str, limits: ClientLimits): Result[Response,HttpError]
```

Make the same GET request with your own timeout and response limits. All three
limits must be positive and within the maximum values below; otherwise the call
returns `Error(InvalidLimits())` before making a request.

```panack
client.ClientLimits(5000, 16384, 65536)
```

This allows five seconds, 16 KiB of headers and 64 KiB of body data. The timeout
covers the transport operation, including DNS, connection, TLS and data transfer.
A limit or timeout failure returns an error rather than a partial response.

## ClientLimits

```panack
record ClientLimits {
  timeout_ms: Nat,
  header_limit: Nat,
  body_limit: Nat
}
```

- `timeout_ms`: Timeout in milliseconds. The default is 10000 (10 seconds),
  and the maximum is 60000 (60 seconds).
- `header_limit`: Maximum response header size in bytes. The default and maximum
  are both 65536 (64 KiB).
- `body_limit`: Maximum response body size in bytes. The default and maximum
  are both 1048576 (1 MiB).

## Response and Header

```panack
record Header {
  name: Str,
  value: Str
}

record Response {
  status: Nat,
  headers: [Header],
  body: Bytes
}
```

`status` is the HTTP status code. `headers` keeps the received order and repeated
fields, so don't assume that a header name occurs only once. `body` contains the
response bytes after HTTP framing has been removed. Decode it as UTF-8 only when
you know the response contains UTF-8 text. These records have no client-specific
methods; read their fields directly.

## Shared Request record

`http/types` also exports this plain data record:

```panack
record Request {
  method: Str,
  target: Str,
  headers: [Header],
  body: Bytes
}
```

`method` is the HTTP method, `target` is the request target, and `headers` and
`body` hold the request data. The GET facade builds requests internally and
doesn't accept this record as an argument. This is a different type from
`http/server.Request`, which is the request passed to routed server handlers
and has the accessors described in the Server Reference.

## Supported URLs and HTTP behaviour

Use lowercase `http://` or `https://`, followed by an ASCII DNS name or dotted
IPv4 address. An optional port must be between 1 and 65535. The default ports are
80 and 443. An omitted path becomes `/`; a query string is allowed.

URLs are limited to 8192 bytes. IPv6 addresses, credentials in the URL, fragments,
spaces, control characters and other schemes aren't supported. Encode non-ASCII
characters in paths and query strings before passing the URL.

The client sends HTTP/1.1 GET with `Accept-Encoding: identity` and
`Connection: close`. It handles fixed-length, chunked and connection-close
response bodies. It doesn't follow redirects, decompress content, reuse
connections or provide cookies, proxies, authentication helpers, custom request
headers or request bodies. A redirect is returned as an ordinary response.

HTTPS uses the platform libcurl trust configuration and verifies both the
certificate chain and hostname. `CURL_CA_BUNDLE` can select a PEM CA bundle;
there is no option to disable verification. Linux release users need the libcurl
runtime library; macOS uses the system library. No C build tools are needed to
use an extracted release package.

## HttpError

The shared `HttpError` enum is available through `http/client` and `http/types`.
Its string payloads explain the failure; don't depend on their exact wording.

- `InvalidUrl(Str)`: The URL is malformed or uses an unsupported form.
- `InvalidLimits`: A supplied limit is zero or exceeds the supported maximum.
- `UnsupportedHost`: The hostname or address isn't supported, including IPv6.
- `DnsFailure`: The hostname couldn't be resolved.
- `ConnectFailure`: A connection couldn't be established.
- `TlsFailure`: TLS negotiation or certificate verification failed.
- `Timeout`: The operation exceeded its deadline.
- `ProtocolError(Str)`: The response is malformed, incomplete or ambiguously framed.
- `LimitExceeded(Str)`: A response exceeded a size limit.
- `UnsupportedVersion(Str)`: The HTTP version isn't supported.
- `UnsupportedTransferEncoding`: The response uses an unsupported transfer encoding.
- `InvalidResponse`: The response couldn't be represented as a valid result.
- `TransportFailure(Str)`: Another transport failure occurred.
- `Cancelled`: Shared enum variant; the GET facade doesn't expose a cancellation method.
- `UnsupportedMethod(Str)`: Shared protocol variant; the GET facade doesn't accept a method argument.
- `InvalidRequest`: Shared protocol variant; the GET facade constructs the request itself.

Import variants explicitly when matching a particular failure, for example
`import http/client::{HttpError.Timeout}`. Handle other failures too. Task
cancellation is governed by the async runtime; the presence of `Cancelled` in the
shared enum doesn't promise that cancelling a task returns a normal result.

## Try a complete client

Start the server from the [playbook](../guides/HTTP_PLAYBOOK.md#your-first-server).
Create a directory called `http-client-example` beside your extracted `panackelty`
directory, then save this as `http-client-example/src/main.panack`:

```panack
import http/client as client
import stdlib/async_host as host

async main(): Unit {
  match await client.get("http://127.0.0.1:8080/") {
    Ok(response) => {
      status = response.status
      ignored = await host.stdout("Status: ${status}")
      body_written = await host.stdout(response.body.decode_utf8())
      ()
    },
    Error(problem) => {
      ignored = await host.stderr("The HTTP request failed")
      await host.exit(1)
    }
  }
}
```

Save this as `http-client-example/panack.json`:

```json
{
  "schema": 1,
  "package": "examples.http_client",
  "version": "0.1.0",
  "language": "modules-v1",
  "toolchain": "0.1.0-alpha.14",
  "source": "src",
  "entry": "main.panack",
  "exports": {},
  "dependencies": {
    "http": { "path": "../panackelty/packages/http" }
  }
}
```

From the directory containing both folders, run:

```sh
./panackelty/bin/panack run --project http-client-example/panack.json
```

The output starts with `Status: 200`, followed by `hello from Panackelty HTTP server`.
The body already ends with a newline, so printing it adds a blank line. To try
custom limits, replace `client.get(...)` with
`client.get_with_limits("http://127.0.0.1:8080/", client.ClientLimits(5000, 16384, 65536))`.
