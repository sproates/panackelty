# Async and HTTP

Native alpha.15 supports `async` functions, explicit `await` and `AsyncFn`
references. Async operations suspend through the VM's bounded scheduler. General
source-level task spawning and resource scopes remain future work.

## Alpha.16 development APIs

On the development branch, async handlers can read the clock through
`stdlib/time/async`, use filesystem and subprocess APIs through their
`/async` modules, and handle domain-specific errors. Pure helpers still need
no await. The [stdlib reference](../reference/STANDARD_LIBRARY.md) lists the
modules and limits; the [migration guide](STDLIB_MIGRATION.md) explains changes
from the compatibility interfaces. These additions require the development
compiler, VM and stdlib together.

Try [async timing](../../examples/async_timing.panack),
[a temporary note with cleanup](../../examples/async_temporary_note.panack), or
[a bounded subprocess](../../examples/async_pipe_input.panack). The
[resource walkthrough](ASYNC_RESOURCES.md) adds SHA-256 and gzip examples with
explicit cleanup. The [uptime service](../../examples/network/http_uptime/README.md)
shows async clock access in a real HTTP handler.

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

The released server API exposes `http/server`: create a bound server,
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
finite HTTP and `http/service` APIs were replaced in alpha.14.

Native networking and filesystem services are host capabilities. Do not assume
that another host provides them simply because it runs the same language.
