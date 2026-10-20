# Acceptance register

The table defines required acceptance, not a blanket pass claim. Current evidence
and remaining validation are recorded below; rows are not complete solely because
a fixture exists. Packet
reports must link actual test files, exact commands, source/bytecode mode and the
validated revision. Do not replace assertions with log inspection. Run public
examples from a manifest importing the package, not from private implementation
modules. The [contract snapshot](contract-snapshot.md) supplies exact API signatures
and Examples 1–7; it prevails over shorthand in this table.

| ID | Required fixture and assertions | Packet |
| --- | --- | --- |
| A01 | Exact Example 1, unchanged: one opaque server, four direct registrations, no reassignment, Unit registration results, awaited/matched listen; exercise routes from source and bytecode | P02/P03/P14 |
| A02 | Example 3: inferred T, at least two named handlers, same stored context, distinct contexts on two servers; reject wrong T/arity/effects and closure substitutions | P02/P03/P14 |
| A03 | Four constructors: simple all-interface and explicit IPv4 forms; ports 0/1/65535/65536, malformed IPv4/whitespace/hostname/IPv6/leading-zero forms, unavailable bind address and occupied port; errors at listen, no bind for invalid setup | P03/P10/P14 |
| A04 | Raw target to path/query: absent path becomes /, absent query empty, preserve percent escapes/case/repeated slash/dot segments, do not decode query | P04 |
| A05 | Header order/repeats retained; first/all lookup ASCII case-insensitive, absent None/empty list; body exact bytes including non-text | P04 |
| A06 | GET/POST/PUT/DELETE registered and dispatched on both forms; valid URI query/fragment ignored for matching; malformed percent escape, space/control/invalid component rejected; same method/path with different suffix duplicate | P06 |
| A07 | Invalid path, duplicate method/path, duplicate fallback: Unit registration, sticky setup error, original handler retained, listen returns non-empty ServerError before bind; late registration traps without changing table | P06 |
| A08 | Unsupported HEAD/PATCH/OPTIONS/TRACE/CONNECT/extension never invoke handlers. Known path Allow lists only registered methods in GET, POST, PUT, DELETE order; unknown path Allow lists all four. Supported wrong method uses path-specific 405 | P06 |
| A09 | Supported unknown path uses fallback or 404; fallback never masks explicit-path 405; explicit match wins; raw path matching is exact | P06 |
| A10 | Default 200; statuses 200–599 valid; header replacement case-insensitive, add preserves repeats; text/Bytes defaults only when Content-Type absent; generated Content-Length and Connection: close | P05 |
| A11 | Reject application Content-Length/Transfer-Encoding/Connection, invalid token/value and injection. Full response header block includes status line/generated fields; invalid input raises recoverable request fault | P05/P09 |
| A12 | Exactly one valid send; no-send Ok becomes 500; second send/mutation after send traps but preserves first response; no second final response on wire | P05/P09 |
| A13 | 204/304 empty send succeeds with no body and no Content-Length; non-empty send becomes generic 500 before commit | P05/P09 |
| A14 | Response body and complete header block at N−1/N/N+1 bytes; include multibyte text. Oversize becomes logged generic 500 and zero partial application-response bytes | P05/P09 |
| A15 | HTTP/1.1 only, exactly one non-empty Host; bodyless unframed success and extra-byte rejection per clarified observation boundary; Content-Length and chunked success | P07 |
| A16 | Malformed head, duplicate Content-Length, CL+TE conflict, missing/duplicate Host ->400; unsupported transfer encoding ->501; unsupported version ->505; no handler | P07 |
| A17 | Request head 16,384 and body 1,048,576 byte boundaries, N−1/N/N+1, fragmented reads and chunked overflow; 431/413, no handler | P07 |
| A18 | Exact 100 Continue before delayed body; no handler until full body; trimmed case-insensitive single expectation accepted; no interim for no body/already-complete body | P08 |
| A19 | 400/404/405/413/503/417 early final outcomes without 100; unsupported/multiple/comma-list Expect ->417; chunked overflow after 100 ->413; only one final response | P08 |
| A20 | Hold four request slots then fifth gets 503/no queue/no handler; independently hold sixteen connections then excess gets 503 under clarified rejection-socket rule. Release work and prove later success, counters return to baseline | P07/P10 |
| A21 | Returned Error, index trap and checked arithmetic trap -> stderr diagnostic and generic 500; subsequent healthy request succeeds. Failure after valid send preserves buffered response. Fatal/OOM/integrity faults do not use request recovery | P09 |
| A22 | Ten-second deadline begins at accept, covers head/body/interim/handler/write; no-send ->504 when possible; sent-then-stalled handler cancelled and first response retained under clarified flush cutoff; disconnect releases resources | P10 |
| A23 | Listen resolves at readiness, service survives main return, bind failures return Result. Close/destroy before listen; repeated operations; listen after shutdown; registration after shutdown; multiple servers independent | P10/P11 |
| A24 | Close stops admission and drains at most one second; destroy cancels immediately and escalates Closing; normal exit/signals follow clarified semantics; port reusable, no stale host completion or leaked request | P11 |
| A25 | Static contextual fallback: root identity/path confinement, index/nested assets, MIME, redirect, traversal/size rejection; non-GET fallback ->405 Allow: GET; explicit route wins; loopback default and --host preserved | P12 |
| A26 | No static imports/knowledge in HTTP; external imports of obsolete server/service APIs fail; all maintained callers migrated; HTTP client acceptance unchanged | P13 |
| A27 | Handle aliasing, stale/forged values, context retention and release, generation reuse, native fault injection and cancellation races. Saved bytecode verification and required seed fixed points remain valid | P02/P03/P09/P11/P15 |
| A28 | Package README complete and linked; examples source/bytecode/installed relocation; preview installed workflow; canonical final make check and exact-head CI; release/website follow-up truthfully recorded | P14/P15 |

## Routing oracle

| Request | Explicit path exists? | Registered method? | Fallback? | Result |
| --- | --- | --- | --- | --- |
| Supported method | Yes | Yes | Either | Explicit handler |
| Supported method | Yes | No | Either | 405, path-specific Allow |
| Supported method | No | No | Yes | Fallback handler |
| Supported method | No | No | No | 404 |
| Unsupported method | Yes | No | Either | 405, path-specific Allow |
| Unsupported method | No | No | Either | 405, Allow: GET, POST, PUT, DELETE |

Static fallback adds its own GET-only policy. Do not encode that restriction in
generic routing. A generic fallback may accept all four supported methods.

## Response failure oracle

| Event | No valid send yet | Valid send completed |
| --- | --- | --- |
| Handler Ok | Generic 500 for missing send | Flush buffered response |
| Handler Error or recoverable trap | Log, generic 500 | Log, preserve buffered response |
| Deadline | Cancel, 504 when possible | Cancel, preserve response; G3 defines write cutoff |
| Disconnect | Cancel/clean up; no delivery claim | Clean up; no delivery claim |
| Fatal integrity/OOM/process failure | Fatal path | Fatal path; no safe-delivery promise |

## Fixture discipline

Use native C raw peers and existing Panackelty package probes; do not introduce
another runtime dependency. Use ready/handler-start/release barriers and bounded
polls, not arbitrary sleeps. Prove handler invocation with counted events or
observable response state. A log message alone does not prove cancellation.
Deadline tests may consume the real ten seconds; isolate them from unrelated
cases and use a monotonic clock with documented scheduling tolerance, without
changing the product deadline to pass tests. Use native fake-clock tests for
boundary ordering where the runtime exposes a suitable seam.

Build size boundaries from byte lengths, including generated fields and CRLFs.
Separate connection saturation (incomplete heads, no admitted handlers) from
request saturation (four admitted handlers). Release held fixtures in cleanup
on every failure. Ports should be selected/bound deterministically without
assuming a fixed port is free. Keep raw two-phase Expect fixtures independently
bounded so a missing interim response fails promptly and cannot hang the suite.

## Integration evidence — 20 October 2026

Foundation PR#548 and runtime PR#549 are merged into `next` (runtime merge
`02d1878`). The runtime canonical `make check` passed in 629 seconds; compiler and
stdlib seed fixed points passed. PR#549 passed all applicable exact-head CI checks
at `f4933e3`. Its source-coverage inventory explicitly accounts for the new
HTTP runtime adapter without claiming fresh coverage percentages.

The public migration is under validation, with these focused results:

- `tests/http_package.sh`: public source/bytecode framing, capacity, deadline,
  recovery, negative exports and corrected contract examples passed. Arithmetic
  division-by-zero recovery and later healthy requests passed. Early-Expect
  rejection/no-interim cases, chunked overflow and active shutdown also passed
  in the canonical run.
- `tests/continuous_service.sh`: source/bytecode lifetime, SIGINT/SIGTERM and
  startup-failure assertions passed with the routed public API.
- `tests/static_http.sh`: static policy plus ordinary/contextual application
  examples passed from source and bytecode against a relocated installation.
- The focused active-shutdown probe passed: an admitted request awaits its body;
  another listener awaits close/destroy and responds only after resource release.
- Final `make check` passed in 509 seconds with the product tree unchanged,
  including unit/functional suites, compiler and stdlib fixed points, relocated
  release-archive examples and quick start. The 120-second aggregate and
  15-second release-smoke budget warnings are historical comparisons, not
  newly agreed performance gates (release smoke took 16 seconds).
- Exact-head public-migration CI passed and PR#550 merged as `c720bc3`.
  The final row-by-row acceptance audit follows below.

Example errata are recorded in the contract snapshot: constructor patterns use
`Ok(done)` and record construction uses `AppContext("Hello")`. These are existing
syntax requirements, not new language features. Socket runs change only the fixed
port; both corrected examples are also compiled without that substitution.

Performance: runtime memory is bounded by accepted/admitted slots and byte caps;
body assembly uses a native bounded buffer. Protocol tests exercise maximum-size
bodies and real deadlines. This is correctness evidence, not a throughput or
latency benchmark claim. The public-migration canonical run took 509 seconds.

## Final acceptance audit

The final delivery retains the original scope. The mapping below identifies the
executable evidence for every row; final canonical validation and exact-head CI
must pass before merge. Repository-relative paths are used in this table.

| Rows | Evidence and audited outcome |
| --- | --- |
| A01–A02 | `tests/http_package.sh` compiles the corrected contract examples verbatim and runs port-isolated source/bytecode copies. Two simultaneously active contextual servers dispatch all four methods plus a second named handler with different retained contexts. Wrong context, arity, sync effect and closure substitutions are rejected. |
| A03 | `tests/unit/vm/http_state.c` covers IPv4 validation and port 0/1/65535/65536 boundaries. `routed_lifecycle_test.panack` covers deferred setup errors, unavailable address, occupied port and no-bind invalid setup. All four constructors are exercised by package and public fixtures. |
| A04–A05 | `parser_test.panack` verifies raw target splitting, escapes, path case, repeated headers, ASCII-insensitive first/all access and body preservation. Independent socket/size peers cover buffered bytes. |
| A06–A09 | `parser_test.panack`, `routed_test.panack`, native state contracts and public socket/context fixtures cover four methods, exact raw routing, invalid/duplicate registrations, sticky errors, late-registration faults, all unsupported method classes, canonical Allow and explicit-route/fallback precedence. |
| A10–A13 | `writer_test.panack` executes real package methods under native writer ownership. Socket cases cover missing send, returned Error, post-send failure, 204/304 empty responses with no Content-Length, and invalid non-empty 204. The peer checks the first status line, so a partial successful response cannot conceal a later 500. |
| A14 | `routed_test.panack` tests complete response-header N−1/N/N+1 boundaries and multibyte response bodies at 1 MiB−1/1 MiB/1 MiB+1 bytes. Live oversized header/body handlers produce generic 500 before application commitment. Header values retain the existing printable-ASCII/tab policy. |
| A15–A17 | Parser source/bytecode tests and raw `routed`/`routed-limits` peers cover HTTP/1.1, Host, unframed bytes, CL/chunked framing, conflicting/duplicate framing, unsupported versions/codings and exact request-header/body byte bounds. Chunked overflow is rejected before handler invocation. |
| A18–A19 | Delayed fixed/chunked bodies require one exact interim response; trimmed mixed-case Expect succeeds. Complete/no-body requests and early 400/404/405/413/503/417 outcomes cannot contain an interim response. Multiple/comma-list expectations fail; a post-interim oversized chunk receives 413. |
| A20 | `routed-capacity` independently holds four admitted requests and sixteen incomplete-head connections, checks excess 503 and releases held clients. Subsequent healthy requests and the rest of the socket suite verify recovery. Native connection cleanup releases admission ownership exactly once. |
| A21–A22 | Live returned Error, index and arithmetic faults return 500 then allow healthy requests; valid first responses survive later failure. `routed-deadline` observes accept/head/body/handler deadlines and first-send delivery during a stalled handler. Native TCP cancellation/write/deadline tests exercise the shared owner; the routed write path uses the same deadline check. Fatal operand/forged-handle and allocation failures remain outside request recovery. |
| A23–A24 | Lifecycle source/bytecode probes, two live contextual listeners, `continuous_service.sh`, and a second-listener close/destroy/escalation barrier cover readiness, main return, signals, idempotence, port reuse and active cancellation. A handler sending then calling `host.exit(7)` preserves its response and exits with 7; this exposed and fixed an eager-clear bug. |
| A25 | `tests/static_http.sh` and its C peer verify contextual static fallback, confinement/root replacement, indexes, redirects, MIME, file/response bounds, GET-only application policy and loopback/host CLI behavior. Generic routing tests prove explicit matches precede fallback. |
| A26 | Public-import rejection tests cover removed exports and the deleted service module. Maintained tools/examples/tests and the installed-file inventory are migrated. Generic HTTP imports no static package. Independent HTTP/HTTPS client fixtures remain unchanged. |
| A27 | `tests/unit/vm/http_state.c`, `resumable.c`, TCP/root-read/console cancellation contracts and `native_faults.c` cover aliases, retained context, forged/expired values, cleanup and allocation failures. Handles retain their backing allocation until release rather than exposing recyclable numeric IDs. Saved-bytecode verification and compiler/stdlib fixed points remain canonical gates. |
| A28 | Public guide and ordinary/contextual examples are linked; static/install and release-archive smoke run both from source and saved bytecode after relocation. Website promotion prerequisites are recorded in the handover register. Final canonical/CI results below complete delivery acceptance. |

Focused final evidence: the HTTP package suite passed after the exit fix, including
context isolation, callback rejection, active close/destroy/escalation and process
exit. Native module contracts passed; the fault suite passed 2,057 allocation
failure cases in total, including the new routed state/writer sweep. Additional
version/Host/coding cases and unavailable-address coverage are included in the
final canonical run.

Final canonical `make check` passed in 635 seconds, including compiler/stdlib
fixed points, unit/functional suites, HTTP/TLS checks, relocated installation and
release-archive examples, and quick start. The historical 120-second comparison
budget emitted a warning; it is not a replacement performance acceptance gate.
Exact-head CI remains a required merge gate; its result is recorded on the final
acceptance PR before merge. No native alpha.14 publication, website
deployment, TLS server, streaming/keep-alive, middleware, closures or new language
syntax is claimed.
