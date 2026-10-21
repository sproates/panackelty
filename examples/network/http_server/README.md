# Routed HTTP server

This alpha.14 development example serves GET `/` and echoes POST `/echo` using
named async handlers and response writers.

It requires the matching Panackelty `0.1.0-alpha.13` development toolchain and
the HTTP package at `packages/http`. The runnable source and manifest are linked
below.

```sh
./bin/panack run --project examples/network/http_server/panack.json 8080
curl http://127.0.0.1:8080/
curl --data 'hello' http://127.0.0.1:8080/echo
```

Run the curl commands in another terminal. The server binds loopback and stays
alive until Ctrl-C. The optional port defaults to 8080. Startup errors are reported
on stderr and exit with status 1. GET `/` returns
`hello from Panackelty HTTP server` followed by a newline.

Read [the source](src/main.panack) and [manifest](panack.json), or try the
[contextual server](../http_contextual_server/README.md) for shared configuration.
See the [HTTP server reference](../../../docs/reference/http-server.md) for the
constructor, route and lifecycle contracts.
For directory serving, use the [static HTTP tool](../../../tools/http-server/README.md).

The [HTTP playbook](../../../docs/guides/HTTP_PLAYBOOK.md) includes this full
program and manifest. It also demonstrates GET `/inspect`, PUT/DELETE `/echo`,
and the 404 fallback. Echo routes demonstrate dispatch only; they do not store
or delete resources.
