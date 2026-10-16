# Async and HTTP

Native alpha.13 supports `async` functions, explicit `await` and `AsyncFn`
references. Async operations suspend through the VM's bounded scheduler. General
source-level task spawning and resource scopes remain future work.

## Choose the right interface

| Need | Start here |
| --- | --- |
| Serve a folder | [Static HTTP server tool](../../tools/http-server/README.md) |
| Fetch an HTTP/HTTPS URL | [HTTP client package](../../packages/http/README.md#client) |
| Write a custom HTTP service | [HTTP server and service APIs](../../packages/http/README.md#server) |
| Work directly with TCP | [Network examples](../../examples/network/README.md) and [TCP contract](../reference/SPEC.md#native-tcp-exchange-development-toolchain) |
| Read confined files asynchronously | [Filesystem contract](../reference/SPEC.md#asynchronous-confined-filesystem-reads) |

The native HTTP client uses the host's libcurl. On Ubuntu 22.04, installations
need `libcurl4`; source builds additionally need `libcurl4-openssl-dev` and
`curl-config`. macOS uses its system libcurl.

## Service lifecycle

`http/service` supplies typed configuration, post-bind readiness and
SIGINT/SIGTERM draining. `stdlib/async_host` provides arguments, bounded console
output and exit for async tools. The generic [static HTTP package](../../packages/static_http/README.md)
combines the service with asynchronous confined reads; website build and preview
configuration remain the caller's responsibility.

Finite TCP/HTTP examples stop after a configured number of clients. They are
useful for protocol and regression work, but do not represent a persistent
application service. See the [continuous service contract](../reference/SPEC.md#continuous-native-services-development-toolchain)
for shutdown, cancellation and ownership guarantees.

Native networking and filesystem services are host capabilities. Do not assume
that another host provides them simply because it runs the same language.
