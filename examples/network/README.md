# Network examples

These examples use the native toolchain. Install [alpha.13](../../docs/guides/INSTALLATION.md)
and run them from the root of the extracted archive with `./bin/panack`.
The same command path works in a built source checkout.

| Example | What it demonstrates |
| --- | --- |
| [TCP exchange](tcp_exchange.panack) | A bounded request/response exchange; requires a listening peer |
| [Finite TCP server](tcp_serve.panack) | Explicit client counts, concurrency and deadlines |
| [Finite HTTP session](http_server/README.md) | One HTTP request, response and session completion |

For ordinary directory serving use [panack-http-server](../../tools/http-server/README.md).
For application HTTP APIs and limitations, read the [HTTP package guide](../../packages/http/README.md).
