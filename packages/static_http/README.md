# Static HTTP service

A generic read-only directory server built on `http/service` and
`stdlib/async_filesystem`. Website builds and preview assembly belong to callers.
This package ships in the native alpha.13 release.

Use the executable in [`tools/http-server`](../../tools/http-server/README.md) for
ordinary directory serving. Applications can depend on this package, import
`static_http/service`, and call `await serve(config, @ready)`.

`Config(root, address, port, limits, headers)` takes an opaque `Path`, numeric IPv4
address, port, `ServiceLimits` and extra response headers. `ServiceLimits` is the
HTTP service's concurrency, request bytes, response bytes, client timeout and drain
record. The default tool uses 4 clients, 16 KiB requests, 1 MiB serialized responses,
a 10-second client timeout and 1-second shutdown grace. Response headers must leave
at least 512 bytes in the response budget. Framing, Content-Type, Location and Allow
headers belong to the server and cannot be overridden. Invalid names/values fail
before binding. The readiness callback receives `TcpServiceReady` after root
validation and bind, before admission, and returns `Result[Unit,Str]`.

## File and URL policy

- GET and HEAD serve regular files; other methods return 405 with `Allow: GET, HEAD`.
- Directories use `index.html`; a missing slash redirects with 308 and preserves
  the query. Empty directories and missing files return 404. There is no listing
  or implicit single-page-application fallback.
- Decode URL paths once as UTF-8. Query strings never become filenames. Reject
  malformed escapes, NUL/control bytes, traversal, repeated separators and encoded
  slashes/backslashes. A double-encoded percent sequence remains a literal filename.
- Resolve beneath the opened root using the existing confined resolver. Relative
  links that remain inside are allowed; absolute links, escapes and loops are denied.
  Pin root device/inode at startup and reject data from a replacement root.
- Read per request so file edits are visible. Concurrent edits have no snapshot
  guarantee. HEAD performs the same bounded read as GET but sends no body.
- Known web assets receive MIME types; unknown extensions use
  `application/octet-stream`. The complete serialized response must fit the limit,
  including headers, so the maximum file size is smaller than the response budget.
  Oversized files return 503 and a concise diagnostic; no partial 200 is emitted.
- Missing/inaccessible paths return 404. Malformed paths return 400. Unexpected
  read errors return 500 with a diagnostic; resource exhaustion or a replaced root
  returns 503. Diagnostics do not include host paths.

Configured headers also appear on connection-generated errors, including malformed
requests and unsupported methods. Transport failures that cannot produce an HTTP
response close the connection. Writes, directory listings, ranges, compression,
TLS, watch/rebuild and automatic reload are outside this tool's scope.

Asynchronous reads and diagnostics can be logically cancelled at a client deadline
or shutdown. A blocked host operation may retain its worker slot until it finishes;
it cannot resume a destroyed request. See the [runtime contracts](../../docs/reference/SPEC.md).
