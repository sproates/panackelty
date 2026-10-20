# Network examples

These examples use the native toolchain. Install [alpha.13](../../docs/guides/INSTALLATION.md)
for the TCP examples; the routed HTTP examples require the alpha.14 development branch. Run them from the root of the extracted archive with `./bin/panack`.
The same command path works in a built source checkout.

| Example | What it demonstrates |
| --- | --- |
| [TCP exchange](tcp_exchange.panack) | A bounded request/response exchange; requires a listening peer |
| [Finite TCP server](tcp_serve.panack) | Explicit client counts, concurrency and deadlines |
| [Routed HTTP server](http_server/README.md) | GET and POST handlers with response writers (alpha.14 development) |
| [Contextual HTTP server](http_contextual_server/README.md) | Shared typed configuration, query access and fallback routing (alpha.14 development) |

For ordinary directory serving use [panack-http-server](../../tools/http-server/README.md).
For application HTTP APIs and limitations, read the [HTTP package guide](../../packages/http/README.md).
