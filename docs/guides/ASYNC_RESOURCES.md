# Working with async resources

This guide and its examples require the **native alpha.16 toolchain**.
They use named functions and pass resource handles explicitly.

## Choose the resource API when input arrives in pieces

For a small buffer already in memory, `sha256.hash(data)` is a pure one-shot call.
For input arriving in pieces, use a SHA-256 context through
`stdlib/digest/sha256/async`. The operation names stay `create`, `update`, `finish`
and `close`.

Run [the complete example](../../examples/async_digest.panack):

```sh
./bin/panack run examples/async_digest.panack
```

It updates one context with `a`, then `bc`, and compares the result with the
one-shot hash of `abc`:

```text
Chunked SHA-256 matches one-shot: true
```

Each update accepts at most 64 KiB. Split larger input before updating. One-shot
hashing accepts at most 16 MiB. The context is an opaque `Sha256Context`, not a
number a caller can manufacture.

`finish` consumes the context on success **and on failure**. Do not call `close`
after `finish`. If an earlier operation fails, the example attempts `close` and
preserves the original error; a cleanup failure is reported separately. Some
native digest failures already invalidate the context, so a subsequent close may
report that it is invalid. Cancellation follows the runtime's ownership rules;
ordinary error branches are not a general cancellation handler.

## Bound decompression before accepting input

Run [the gzip example](../../examples/async_gzip.panack):

```sh
./bin/panack run examples/async_gzip.panack
```

```text
hello
```

The compressed greeting is embedded so the example needs no download or setup.
The decoder has a 1 KiB total output budget. This is a small-message recipe,
not an unbounded file-decompression utility.

A `GzipChunk` reports three things:

- `consumed`: how many input bytes this step consumed;
- `data`: the decompressed bytes produced by this step;
- `finished`: whether decoding has completed.

One step need not consume all the input. Advance by `consumed`, retain the
unconsumed suffix and keep decoding until `finished`. Passing `finish = true`
means this buffer contains the final input, not that the decoder must complete
in one call. Each input step is limited to 64 KiB; each output step produces at
most 64 KiB. For input arriving from a file or socket, supply bounded chunks and
mark only the final input as final.

The example joins output chunks because its entire output is bounded to 1 KiB.
For larger workloads, process or write each chunk instead of repeatedly growing
a single buffer. Decode text only once a complete text buffer is available;
UTF-8 characters can cross output-chunk boundaries.

A gzip decoder still needs `close` after successful completion, malformed or
incomplete input, or an exhausted output budget. The example closes it before
returning either the decoded bytes or the original failure. If cleanup also
fails, it preserves both explanations. This differs from consuming a SHA-256
context with `finish`.

## Resource ownership is part of the API

Copying an opaque handle creates an alias, not a new resource. Closing or consuming
one alias invalidates the others. Handles cannot be transferred to another
invocation. Closing releases native state, but does not replenish per-invocation
creation quotas: 65,536 SHA-256 contexts and 64 gzip decoders, shared with legacy
interfaces. Do not use an unbounded create/close loop as a resource pool.

Async operations yield between bounded work steps. They do not imply that every
operation runs on a new thread, or that CPU work is free. The
[domain-error contract](../design/STDLIB_DOMAIN_ERRORS.md#sha-256-and-gzip-resources)
documents failure categories and lifecycle limits.

## Using async APIs inside an HTTP handler

The [healthcheck service](../../examples/network/http_uptime/README.md) is the
worked example of this integration. Its immutable context holds the start
`Instant` and a service name. Each `/health` request awaits `time.now()` and
computes `now.difference(context.started)`. The output is elapsed nanoseconds,
not a calendar timestamp. Clock failure produces an explanatory HTTP 503 response.

The context does not need mutable capture to preserve the start value. This
example does not count requests: shared mutable state is separate work.

For network data, the high-level HTTP client is usually easier than raw stream
management. `http/client/async` owns its underlying stream lifecycle and retains
detailed transport errors. Use raw `network/http_stream/async` only when the
application needs to handle request/response bytes and explicit stream ownership.
