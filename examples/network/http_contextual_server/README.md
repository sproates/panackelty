# Contextual routed HTTP server

This alpha.14 development example shares one typed `AppContext` between named
handlers. No closure capture or untyped configuration is needed.

It requires the matching Panackelty `0.1.0-alpha.13` development toolchain and
the HTTP package at `packages/http`.

```sh
./bin/panack run --project examples/network/http_contextual_server/panack.json 8081
curl http://127.0.0.1:8081/
curl 'http://127.0.0.1:8081/health?probe=1'
curl -i http://127.0.0.1:8081/missing
```

Run curl in another terminal. The first response is `Hello`; `/health` returns
`OK: Hello` and echoes the raw query in `X-Query`. The fallback returns 404.
The optional port defaults to 8081. Binding is local-only, startup failures exit
with status 1, and Ctrl-C stops the server.

See [the source](src/main.panack), [manifest](panack.json), and
[HTTP package guide](../../../packages/http/README.md).
The [HTTP server reference](../../../docs/reference/http-server.md) documents
the contextual constructor, handler type, routes and lifecycle contract.
