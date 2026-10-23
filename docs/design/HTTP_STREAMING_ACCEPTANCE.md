# Bounded raw HTTP streaming prerequisite

P03a requires up to 64 MiB compressed archives, but the existing HTTP client
rejects body limits above 1 MiB and its raw exchange buffers at most 16 MiB.
A public CLI probe confirmed `InvalidLimits` before network access. The owner
approved a separate generic transport prerequisite on 24 October 2026.

`stdlib/http_stream` exposes blocking open/read/close for command-line consumers.
It reuses libcurl's verified connect-only transport, retaining one 16 KiB receive
buffer and returning at most 16 KiB per read. Requests are capped at 64 KiB,
raw responses at 256 MiB, handles at 512 per invocation and the absolute deadline
at 60 seconds, including time between reads. Handles are invocation-owned and
never reused; errors and EOF close the socket, and explicit close or teardown
releases the remaining context. Blocking reads are not an async server API.

No HTTP framing, redirect or package policy is implemented in C. The existing
buffered async client retains its limits and behaviour. The compiler changes
declare and restrict the three generic native operations; the seed refresh is
a prerequisite interface change, not a package-only rebuild dependency.

## Evidence

- Native tests verify all bytes in a 32 MiB response, chunk bounds, total-limit
  refusal, stale/foreign handles, deadline expiry between reads and cleanup.
- Public CLI fixtures run source and saved bytecode against a 32 MiB response
  and trusted/untrusted TLS, asserting exact output.
- Native ASan/UBSan checks pass, including the existing async transport tests.
- Seed stages 2, 3 and 4 agree on
  `34690cbd157483b57076e508d362a480405924326ce748de3905783d35d97eed`.
  The standard-library conformance artifact is unchanged.
- The 32 MiB public CLI fixture, including compilation and source/bytecode runs,
  took 0.54 seconds on the development Apple-silicon Mac. This is one warm
  observation with concurrent validation, not a network throughput guarantee.
  Native receive-buffer allocation is fixed at 16 KiB per stream; no peak-RSS
  measurement is claimed.

Canonical `make check` passed on 24 October 2026 in 731 seconds, including unit,
bootstrap, functional, installed distribution and quick-start checks. PR#576
delivers the prerequisite subject to its exact-head CI merge gate. The printed
historical budget warnings remain comparison data under the current policy.
This prerequisite does not complete P03a or GI#568. HTTPS archive acquisition follows
in a separate PR using the same package tool and descriptor.
