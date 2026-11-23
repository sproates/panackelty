# HTTP client with detailed failures

This example requires the **alpha.16 development toolchain** and the matching
HTTP package. It does not work with alpha.15's package exports.

From the repository root:

```sh
./bin/panack run --project examples/network/http_diagnostic_client/panack.json https://example.com/
```

The program takes exactly one URL and prints the response status and body byte
count. Its explicit limits are a five-second timeout, 16 KiB of headers and
64 KiB of body. It does not print the response body or assume it is UTF-8.
Network access and a reachable server are required for this command; tests use
an independent loopback fixture instead of a public website.

The [source](src/main.panack) imports `http/client/async as http` and awaits
`get_with_limits`. The package dependency is in [panack.json](panack.json), with
a checked-in [lockfile](panack.lock.json). For an installed archive, the same
relative dependency locates that archive's `packages/http` directory.

## Responses and failures are different

An HTTP 404 is a valid response. The example prints `HTTP 404; ... body bytes.`
and exits 1 because this application's policy treats statuses from 400 upwards
as unsuccessful. That exit policy is separate from transport validation.

An invalid URL, DNS failure, connection failure, TLS failure, timeout or rejected
response returns `HttpClientError`. The example sends `problem.render()` to
stderr and exits 1. The error retains its HTTP category and, when available,
the transport error and endpoint context. The renderer uses that detail rather
than guessing whether a TLS failure was caused by a certificate, hostname or
another issue. Do not parse the rendered prose for control flow: inspect the
typed category when an application needs different behaviour for a particular
failure.

A reproducible failure that makes no network request is:

```sh
./bin/panack run --project examples/network/http_diagnostic_client/panack.json http://user@localhost/
```

```text
Cannot make the HTTP request: HTTP URL authority is empty or contains userinfo.
```

Certificates and hostnames are verified. There is no advice here to bypass TLS
verification. The existing `http/client` export remains available with its old
error contract; using the new export in one program does not migrate other
consumers automatically.

With no URL or more than one argument, the program prints usage and exits 2.
See the [stdlib reference](../../../docs/reference/STANDARD_LIBRARY.md) and
[migration guide](../../../docs/guides/STDLIB_MIGRATION.md) for related APIs.
