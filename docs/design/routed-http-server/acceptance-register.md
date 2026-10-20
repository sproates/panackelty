# Acceptance register

These are test specifications, not results. Every row starts **Not run**. Packet
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
