# Finite HTTP session fixture

This is a one-client regression example for the [HTTP package](../../../packages/http/README.md),
not a continuously running web application or a recommended error-handling template.
It deliberately asserts the finite session's result and exits after one client.

From the root of a built checkout or extracted alpha.13 archive:

```sh
./bin/panack run --project examples/network/http_server/panack.json
```

In another terminal, make one request:

```sh
curl http://127.0.0.1:8080/
```

The response body is `hello from Panackelty HTTP server` followed by a newline.
The server then exits. Port 8080 must be free; the session has bounded admission
and client timeouts. A browser may make additional requests, so use the single
curl request for this fixture.

Read [the source](src/main.panack) beside [its manifest](panack.json).
For serving a directory, use the [static HTTP server](../../../tools/http-server/README.md).
For a long-running custom service, see `http/service` in the package guide.
