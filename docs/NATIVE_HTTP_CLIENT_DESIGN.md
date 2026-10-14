# Native HTTP client design (P5)

P5 adds a bounded HTTP/1.1 GET client to `panackelty.http`. The package owns URL
validation, request construction, response framing and public errors. A private
native adapter supplies only DNS, TCP connect, verified TLS and bounded raw
duplex reads and writes. It uses libcurl's multi connect-only mode as the
alpha.13 proof of concept; no libcurl handles, headers, error text or HTTP
behavior are exposed to Panackelty source.

The boundary is replaceable: `client.panack` calls `transport.panack`, which
calls the reserved `$http_transport_exchange` intrinsic. The intrinsic accepts
the validated host, port, TLS flag, request bytes, raw response bound and one
operation deadline. The package parser decides the response status, headers,
body framing and typed failure. libcurl does not follow redirects, parse HTTP,
decompress content, consult proxies, or accept an unverified certificate.

The first public API is intentionally small:

- `http.get(url)` performs one GET and uses a 10-second deadline, a 64 KiB
  response-header bound and a 1 MiB response-body bound.
- `http.get_with_limits(url, limits)` allows callers to lower or raise those
  bounds within the same 60-second, 64 KiB-header and 1 MiB-body hard limits.
- Responses preserve status, header order and repeated header fields, and carry
  the response body as `Bytes`. Non-2xx status codes are ordinary responses.
- Errors distinguish malformed URLs, unsupported hosts, DNS and connect
  failures, TLS verification, deadlines, malformed protocol data and limits.

Only `http` and `https` URLs with ASCII DNS names or dotted IPv4 addresses are
accepted. IPv6, user information, fragments, control characters, non-origin
path forms and other schemes are rejected. Requests use HTTP/1.1, origin-form
targets and `Connection: close`. The response parser supports fixed-length,
chunked and connection-close bodies, preserves duplicate headers, and rejects
ambiguous or incomplete framing. Redirects, request bodies, connection reuse,
cookies, authentication helpers, proxies and general streaming are outside
this slice.

TLS verifies both the certificate chain and host name using the platform
libcurl trust configuration. `CURL_CA_BUNDLE`, when set, selects a PEM trust
bundle through libcurl's CA-file option; this makes alternate trust stores and
the local self-signed test authority work without disabling verification. A
verification failure is returned as `TlsFailure`. The native adapter also
requires libcurl's asynchronous DNS and thread-safe features and returns a
bounded error when the linked library lacks either. WASI/browser execution is
unavailable.

Native builds require libcurl development headers and libraries plus
`curl-config` on `PATH` (or an explicitly configured `HTTP_CURL_CONFIG`).
`make native` checks that dependency at build time. This is an initial backend
choice for alpha.13, not a commitment to make libcurl part of the language API;
future evidence may justify replacing the adapter.

The deterministic local fixture in `tests/http_client_package.sh` exercises
source and saved-bytecode execution, normal and non-2xx responses, repeated
headers, informational and chunked responses, close framing, malformed and
oversized bodies, invalid URLs, deadlines, trusted HTTPS, host-name mismatch
and rejection of an untrusted certificate. The fixture creates its own short
lived certificate and uses `CURL_CA_BUNDLE` only for the trusted-certificate
cases. Its final case installs the package into a temporary prefix and checks
source and saved-bytecode execution through the installed CLI.
