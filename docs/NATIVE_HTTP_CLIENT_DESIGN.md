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
verification failure is returned as `TlsFailure`. The native adapter requires
libcurl's asynchronous DNS feature and returns a bounded error when the linked
library lacks it. The VM owns native operations on one thread; libcurl is
initialized once on that thread before HTTP operations begin. WASI/browser
execution is unavailable.

Native builds require libcurl development headers and libraries plus
`curl-config` on `PATH` (or an explicitly configured `HTTP_CURL_CONFIG`).
`make native` checks that dependency at build time. The native VM dynamically
links libcurl, so the host must also provide its runtime library even when a
program does not use HTTP. On the tested Ubuntu 22.04 target, native builds use
`libcurl4-openssl-dev` and installed toolchains need `libcurl4`; macOS uses its
system libcurl. These are host prerequisites: Panackelty archives do not bundle
libcurl or its transitive libraries, and the Ubuntu package version is not
pinned. This is an initial backend choice for alpha.13, not a commitment to make
libcurl part of the language API; future evidence may justify replacing the
adapter.

## Dependency evidence snapshot

Observed on 2026-10-14; these are environment snapshots, not version pins.
`curl-config` reports build-time information, while `curl_version_info()`
reports the library actually loaded at runtime ([upstream documentation](https://curl.se/libcurl/using/curl-config.html),
[runtime API](https://curl.se/libcurl/c/curl_version_info.html)).

| Environment | Verified libcurl evidence | Native dependency boundary |
| --- | --- | --- |
| Local macOS 26.5, arm64 | Runtime probe: libcurl 8.7.1; Secure Transport and LibreSSL 3.3.6; zlib 1.2.12; `AsynchDNS`, `SSL` and `threadsafe` features. `/usr/bin/curl-config --configure`: `--enable-threaded-resolver --with-gssapi --with-ssl=/usr/local/libressl --with-secure-transport`. | `panack-vm` links `/usr/lib/libcurl.4.dylib` and `/usr/lib/libSystem.B.dylib`. The system libcurl also links Apple Security, CoreFoundation and SystemConfiguration, nghttp2, zlib and libSystem; crypto, SSL, LDAP and Kerberos are weak-linked. These are OS-provided, not vendored by Panackelty. |
| GitHub Actions `ubuntu-22.04` P5 smoke | The [#452 run](https://github.com/sproates/panackelty/actions/runs/38507500172) installed `libcurl4-openssl-dev:amd64` version `7.81.0-1ubuntu1.29`; this establishes the tested development package and OpenSSL flavour. The smoke passed. | Ubuntu's [matching Jammy `libcurl4` package record](https://packages.ubuntu.com/jammy-updates/amd64/libcurl4) lists runtime dependencies on Brotli, glibc, GSS-API/Kerberos, IDN2, LDAP, nghttp2, public-suffix data, RTMP, SSH, OpenSSL, zstd and zlib. The project does not vendor these libraries. |

The curl project identifies libcurl's license as SPDX `curl`, a permissive
license inspired by MIT/X but not identical to MIT ([license text and identifier](https://curl.se/docs/copyright.html)). The current executable links the host's dynamic libcurl; archives do not bundle it or its transitive libraries.

The local macOS HTTP and installed-toolchain checks establish operation with
that host library. The Ubuntu CI smoke establishes a native build with its
installed development package, but does not run the HTTP fixture or installed
archive on Linux. These results do not establish a self-contained or
version-locked native package. A release artifact must have its supported host
library prerequisite made explicit and checked on each target. No complete
per-transitive-package license inventory has been produced yet.

The Linux job currently installs from the live Jammy repository rather than a
locked package snapshot. Its recorded package version is evidence for this run,
not a reproducible pin. Keep dependency pinning, per-platform license inventory
and clean-host archive acceptance as explicit follow-up evidence; do not infer
them from a successful compile.

The deterministic local fixture in `tests/http_client_package.sh` exercises
source and saved-bytecode execution, normal and non-2xx responses, repeated
headers, informational and chunked responses, close framing, malformed and
oversized bodies, invalid URLs, deadlines, trusted HTTPS, host-name mismatch
and rejection of an untrusted certificate. The fixture creates its own short
lived certificate and uses `CURL_CA_BUNDLE` only for the trusted-certificate
cases. `tls-late` holds a real server TLS flight past the configured deadline
and verifies timeout for both source and saved-bytecode execution before the
fixture attempts to deliver the late flight. This does not exercise a resolver
that completes after cancellation. The final case installs the package into a
temporary prefix and checks source and saved-bytecode execution through the
installed CLI.
