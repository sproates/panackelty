# Static HTTP service

A generic read-only directory server built on `http/server` and
`stdlib/async_filesystem`. Website builds and preview assembly belong to callers.
This package ships in the native alpha.13 release.

Use the executable in [`tools/http-server`](../../tools/http-server/README.md) for
ordinary directory serving. Applications can depend on this package, import
`static_http/service`, and call `await serve(config, @ready)`.

`Config(root, address, port, request_body_limit, response_limit, headers)` takes
an opaque `Path`, numeric IPv4 address, port, application byte limits and extra
response headers. The tool uses a 16 KiB request-body limit and 1 MiB complete
serialized-response limit. Transport limits are owned by the routed framework:
16 accepted connections, four admitted requests, a 10-second deadline and
one-second drain. The tool retains its loopback default and `--host` option.

Response headers must leave at least 512 bytes in the response budget. Framing,
Content-Type, Location and Allow cannot be overridden. Invalid names/values fail
before binding. The readiness callback receives `Endpoint(address, port)` after
root validation and bind; returning `Error` destroys the server. Successful
`serve` returns after readiness while the runtime retains the listener.

## File and URL policy

- GET serves regular files. POST, PUT and DELETE return 405 with `Allow: GET`.
  HEAD and other unsupported methods are rejected by the framework before the
  static handler, with `Allow: GET, POST, PUT, DELETE`; HEAD carries no body.
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
  guarantee.
- Known web assets receive MIME types; unknown extensions use
  `application/octet-stream`. The complete serialized response must fit the limit,
  including headers, so the maximum file size is smaller than the response budget.
  Oversized files return 503 and a concise diagnostic; no partial 200 is emitted.
- Missing/inaccessible paths return 404. Malformed paths return 400. Unexpected
  read errors return 500 with a diagnostic; resource exhaustion or a replaced root
  returns 503. Diagnostics do not include host paths.

Configured headers apply to static application responses. Framework-generated
protocol errors and unsupported-method responses use framework headers. Transport failures that cannot produce an HTTP
response close the connection. Writes, directory listings, ranges, compression,
TLS, watch/rebuild and automatic reload are outside this tool's scope.

Asynchronous reads and diagnostics can be logically cancelled at a client deadline
or shutdown. A blocked host operation may retain its worker slot until it finishes;
it cannot resume a destroyed request. See the [runtime contracts](../../docs/reference/SPEC.md).
