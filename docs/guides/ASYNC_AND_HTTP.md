# Async and HTTP

Native alpha.13 supports `async` functions, explicit `await` and `AsyncFn`
references. Async operations suspend through the VM's bounded scheduler. General
source-level task spawning and resource scopes remain future work.

## Choose the right interface

| Need | Start here |
| --- | --- |
| Serve a folder | [Static HTTP server tool](../../tools/http-server/README.md) |
| Fetch an HTTP/HTTPS URL | [HTTP client package](../../packages/http/README.md#client) |
| Write a custom HTTP service | [Routed HTTP server API](../../packages/http/README.md#server) |
| Work directly with TCP | [Network examples](../../examples/network/README.md) and [TCP contract](../reference/SPEC.md#native-tcp-exchange-development-toolchain) |
| Read confined files asynchronously | [Filesystem contract](../reference/SPEC.md#asynchronous-confined-filesystem-reads) |

The native HTTP client uses the host's libcurl. On Ubuntu 22.04, installations
need `libcurl4`; source builds additionally need `libcurl4-openssl-dev` and
`curl-config`. macOS uses its system libcurl.

## Service lifecycle

The alpha.14 development branch exposes `http/server`: create a bound server,
register named handlers and await `.listen()`. Successful listening returns after
binding; the native runtime keeps the process alive. `.close()` drains for up to
one second, `.destroy()` cancels immediately, and SIGINT/SIGTERM perform bounded
shutdown. `stdlib/async_host` supplies arguments, console output and exit.

Use the [ordinary](../../examples/network/http_server/README.md) or
[contextual](../../examples/network/http_contextual_server/README.md) example.
The generic [static HTTP package](../../packages/static_http/README.md) uses the
same framework with confined filesystem reads. It retains loopback binding by
default. The unbound convenience constructors listen on all IPv4 interfaces;
choose a bound constructor when access should remain local.

Finite TCP examples retain their existing transport contracts. The previous
finite HTTP and `http/service` APIs have been replaced on the development branch.

Native networking and filesystem services are host capabilities. Do not assume
that another host provides them simply because it runs the same language.
