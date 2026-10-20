# Routed HTTP server

This alpha.14 development example serves GET `/` and echoes POST `/echo` using
named async handlers and response writers.

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
For directory serving, use the [static HTTP tool](../../../tools/http-server/README.md).
