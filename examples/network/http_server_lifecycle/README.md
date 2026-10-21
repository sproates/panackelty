# Own the HTTP listener lifecycle

Alpha.14 development example. Run from the matching `next` source checkout:

```sh
./bin/panack run --project examples/network/http_server_lifecycle/panack.json 8082
```

The program starts and immediately closes a loopback listener, then calls the
idempotent `destroy()` method. Expected output is `Server closed` followed by
`Server destroyed`. This is a lifecycle example for tests and embedding applications,
not a server to visit in a browser. An optional argument selects the port.

See [the HTTP playbook](../../../docs/guides/HTTP_PLAYBOOK.md#own-the-listener-lifecycle)
for the distinction between graceful close, immediate cancellation and Ctrl-C.
