[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Real async TCP from Panackelty

<a id="rm-36"></a>

**RM#36: Finite TCP client** · [GI#126: Finite TCP client](https://github.com/sproates/panackelty/issues/126).

Work record: [issue #126](https://github.com/sproates/panackelty/issues/126).
State: Done. PR #127 merged on 2026-09-30 after all 23 hosted checks passed.
Canonical validation passed in 117s, with native sanitizer, independent-peer
source/bytecode tests, bootstrap and browser rejection evidence in the PR.
This functionality is included in the published alpha.11 native release; it is
not part of alpha.10.
Issue #126 was closed during the subsequent grooming pass.
Selected on 2026-09-30 after deployment
reliability, explicitly ahead of build-cache work and a C-only networking spike.
The user wants real source programs to benefit from async/await.

Bounded scope: `await tcp_exchange` owns one numeric-IPv4 request/response
connection, half-closes after sending, reads to EOF, and returns bytes/error.
Bound memory and total time; use nonblocking sockets with owner-thread polling
and close on every outcome. Native embedded execution is opt-in; WASI remains
without raw networking. No source handles, spawning, listening, DNS, TLS or HTTP
framing. This is an additive development feature, not an alpha.10 release claim.

Medium–large, one PR including compiler/decoder effects, seed refresh, native
backend, source/bytecode tests, fault/sanitizer evidence and documentation.
Acceptance requires partial/binary/empty I/O, response limits, timeout/refusal,
independent progress, cancellation cleanup, explicit host restrictions,
canonical `make check` and Linux/macOS/browser CI. The finite server subsequently
shipped below; broader server capabilities and build-cache improvements remain
independently scoped follow-ups.
