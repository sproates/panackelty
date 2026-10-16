[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Bounded async TCP server

<a id="rm-37"></a>

**RM#37: Finite TCP server** · [GI#128: Finite TCP server](https://github.com/sproates/panackelty/issues/128).

Work record: [issue #128](https://github.com/sproates/panackelty/issues/128).
State: Done. Contract PR #129 and implementation PR #130 merged on 2026-09-30;
implementation merge commit `43c09b947ead3e62a7aa3a4cd18b98f1c64d44ee`.
All 23 hosted validation/build/browser checks passed; deployment-only jobs were
skipped for the PR. The implementation records a passing 110s `make check`, native
ASan/UBSan, 16 WASI runtime/asset tests and identical compiler/stdlib bootstrap
fixed points. The existing unit-phase budget warning remains. This completes the
selected finite server stage and allows backlog grooming to proceed. Issue #128
can close when this repository completion summary merges. No release is included.

The [server contract](../../internals/ARCHITECTURE.md#bounded-tcp-server-contract--proposed-implementation)
defines an awaited finite server owner with named async byte-request handlers,
bounded concurrent admission, EOF request/reply framing, monotonic deadlines and
graceful draining. The runtime owns sockets and handler lifetimes. Expected
client failures are isolated; runtime traps cancel the owner. Listening is a
separate embedded capability; WASI remains unavailable. General source spawning,
resource handles, HTTP, DNS, TLS and indefinite service operation are excluded.

Delivered in two PRs: contract, then source/native implementation. Independent
source and saved-bytecode peers exercise binary, fragmented and empty transfers,
request/response limits, stalled-reader and busy-handler fairness, nested outbound
waits and connection reuse. Tests cover capabilities, forged bytecode, allocation
and descriptor failures, pending-phase shutdown and final-admission cancellation.
The runnable finite echo example and documented limits are included. General
spawning, DNS, TLS, HTTP and indefinite services remain separate proposals.
