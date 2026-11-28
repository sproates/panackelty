# Request count and uptime: retained closure

Requires the alpha.16 development toolchain; released alpha.15 does not support State.
From the repository root:

```sh
./bin/panack run --project examples/network/http_state_closure/panack.json 8082
```

Then request `http://127.0.0.1:8082/status`, for example with
`curl -i http://127.0.0.1:8082/status`. The optional port defaults to 8082;
the server binds to loopback. Ctrl-C stops it.

Each response reports the committed request count and monotonic uptime in seconds.
The exact duration can be fractional; it is not the calendar time. The count is
shared across handler calls and resets when the server restarts. Only this route
is counted. Requests rejected before reaching the handler are not counted.

The pure update callback increments the current value exactly once. Concurrent
requests cannot overwrite each other’s increment. A clock failure after commit
returns HTTP 503 and still counts that request. A startup clock failure prints an
explanation and exits. Cancellation before commit does not count; cancellation or
response failure after commit does not undo the increment. Keep read-modify-write
logic inside `update`; reading a value and replacing it later is not atomic.

The anonymous handler retains the start instant and cell. Its request and response writer are parameters, not captured resources. The captured binding is immutable; mutation is explicit through State.

See the [equivalent contextual version](../http_state_contextual/README.md).
