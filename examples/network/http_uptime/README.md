# HTTP healthcheck with uptime

This example needs the alpha.16 development toolchain containing the async
stdlib modules. It does not run on the released alpha.15 toolchain.

From the repository root:

```sh
./bin/panack run --project examples/network/http_uptime/panack.json 8082
```

In another terminal:

```sh
curl -i http://127.0.0.1:8082/health
```

The body is `uptime_ns=` followed by the elapsed nanoseconds since server setup,
for example `uptime_ns=1234567890`. Repeating the request measures uptime again.
The clock is monotonic: changing the machine's calendar time does not change
this duration. Nanosecond representation does not imply nanosecond accuracy.

The optional port defaults to 8082. The server binds to loopback; Ctrl-C stops
it. Set `PANACK_SERVICE_NAME` before starting to choose the `X-Service` response
header (the default is `uptime-example`). Environment reads use the invocation
snapshot, not a mutable configuration store.

The [program](src/main.panack) imports `stdlib/time/async` and awaits `time.now()`
inside the handler. A typed immutable context carries the start instant. No
mutable capture or global state is needed. A clock failure at startup reports
an error and exits; a clock failure during a healthcheck returns HTTP 503 with
an explanation instead of inventing an uptime value.

This measures time since setup, not time since machine boot, and does not count
requests. Shared mutable state is separate work.
