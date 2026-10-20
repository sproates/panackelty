# Routed HTTP server implementation plan

Status: implementation proposal and handoff, 20 October 2026. Product implementation
has not started. This plan implements [GI#518](https://github.com/sproates/panackelty/issues/518)
within [RM#156](../../roadmap/records/next-release--agreed-scope-2026-10-06.md#rm-156).
It does not reopen the completed low-level HTTP work or GI#525.

The required result is a small public API: create a server, register named handlers,
and listen. The difficult work is preserving typed ownership, request isolation,
bounded transport and predictable shutdown behind that API. Do not mistake the
small public surface for a small runtime change.

## Read in this order

1. [Agreed contract and exact issue examples](contract-snapshot.md).
2. [Implementation boundaries and decision gates](architecture-and-gates.md).
3. [Ordered task packets](task-packets.md).
4. [Acceptance register](acceptance-register.md).
5. [Execution and handoff runbook](execution-runbook.md).

The issue is authoritative. The snapshot prevents an implementer from relying on
chat history, but must be compared with the live issue before each packet. Owner
clarifications belong in the issue before implementation relies on them. Proposed
internal designs below cannot override its behaviour. Do not start implementation
merely because this planning PR has merged; the owner selects the next packet.

## Baseline

Core inspected: `f12717c9f8c3b79421688fd3cc829cd6c7d9ed72` on `next`.
GI#525 is complete, including owned dot calls, generic methods and async callback
checking. That is a dependency, not earned progress for this HTTP programme.
The existing HTTP package exports low-level `server` and `service` modules; its
Request has `target`, rather than the required `path` and `query`. The static
preview owns a private context containing its filesystem identity and currently
accepts `--host`, with loopback as the default. Native TCP handler traps currently
propagate to the server. These are inspected starting facts, not missing features
that may be waved away by changing the acceptance criteria.

The RM#156 record existed on `main` but was absent from this `next` checkout.
This planning change brings that one scope record across, without importing
unrelated main-branch roadmap changes.

## Scope

Deliver both ordinary and contextual servers, explicit-address variants,
request accessors, response writers, four methods and fallback, setup errors,
HTTP framing and Expect handling, admission limits, deadlines, request-local
recovery, lifecycle, static-preview migration, package documentation and installed
consumer acceptance. Preserve the HTTP client. Remove obsolete public low-level
server exports when maintained consumers have migrated.

No closures, general mutable records, wildcard routes, middleware, cookie API,
streaming responses, TLS server, HTTP/2, keep-alive, pipelining, public resource
options or general TCP redesign. Repeated Set-Cookie headers are supported as
headers; that does not create a cookie API. Browser networking is not implied.

## Owner-approved bind-address clarification

The simple constructors still bind `0.0.0.0`. Add
`create_bound_server(address, port)` and
`create_bound_contextual_server(address, port, context)` for explicit IPv4
binding. They return the corresponding existing server type and use the same
fixed resource defaults. Address validation and errors are specified in the
snapshot. The preview uses the explicit contextual form, preserving loopback
by default and `--host`. This clarification is recorded in GI#518; it does not
introduce a public options record or transport tuning.

## Sequence and weighted baseline

Weights are provisional scope estimates, not time estimates. Runtime ownership,
recovery and protocol integration carry more weight than documentation. Freeze
this baseline when P01 is accepted; record any later change explicitly. Earn
credit only for accepted outcomes, never for time spent, commits or a saved plan.
Each packet starts at 0%; all earned values are currently zero.

| Stage / packets | Status | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Contract and feasibility: P01–P02 | Unstarted | 10% | 0% | 0 |
| Handles and request/response model: P03–P05 | Unstarted | 20% | 0% | 0 |
| Routing and protocol: P06–P08 | Unstarted | 20% | 0% | 0 |
| Runtime isolation and lifecycle: P09–P11 | Unstarted | 30% | 0% | 0 |
| Consumers and API cleanup: P12–P13 | Unstarted | 10% | 0% | 0 |
| Documentation and acceptance: P14–P15 | Unstarted | 10% | 0% | 0 |
| **Total** | **Planning saved; implementation unstarted** | **100%** | | **0%** |

Packet allocations: P01 4, P02 6, P03 8, P04 4, P05 8, P06 6, P07 7,
P08 7, P09 12, P10 10, P11 8, P12 6, P13 4, P14 4, P15 6.
Stage completion is earned packet points divided by that stage's allocation.
Do not count GI#525 or the previous HTTP implementation again.

Use one packet per implementation request by default. P01 and P02 are mandatory
before committing to the runtime representation. Thereafter the dependency table
in the packet document is authoritative; do not integrate a public facade that
claims unsupported behaviour. Expected scale is roughly 15–20 implementation PRs,
including possible splits of runtime packets, plus this planning PR. This is a
planning estimate, not a promise or a requirement to manufacture PRs. P02 must
revise the estimate if the verified runtime design requires it.

## Completion

GI#518 closes only after every acceptance row has linked executable evidence,
all maintained consumers use the intended public API, installed examples pass,
full final validation and exact-head CI pass, and documentation describes the
actual result. Alpha.14 publication remains separate release work. Keep the issue
open for intermediate packets and record outstanding acceptance explicitly.
