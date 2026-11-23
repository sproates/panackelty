# Async stdlib documentation coverage

Updated: 23 November 2026. Part of RM#162 / AS5, on the alpha.16 development
branch. This is the finite review checklist for the agreed API domains; it does
not add new language or library scope. Source signatures remain linked from the
[reference](../reference/STANDARD_LIBRARY.md).

| Domain | User-facing material | Executable evidence | Remaining documentation work |
| --- | --- | --- | --- |
| Imports, effects, compatibility | Reference and migration guide | Existing async_stdlib tests; all seven standalone async examples | None; published and live-verified |
| Clock and environment | Reference; timing example; HTTP uptime walkthrough | async_timing; http_uptime; async_stdlib | None; published and live-verified |
| Console and UTF-8 | Reference, input/EOF walkthrough and async_console_input | console_input; async_stdlib_examples (including split UTF-8, exact bound, oversized/invalid input); named examples | None; published and live-verified |
| General filesystem | Reference, migration guide, temporary-note walkthrough | async_temporary_note; async_filesystem; filesystem_errors | None; published and live-verified |
| Confined reads and staging | Reference, async_publish_note and confinement/publication walkthrough | async_publish_note; existing confined/staging ownership, cancellation and publication tests | None; published and live-verified |
| Processes | Reference, migration guide, bounded subprocess example | async_pipe_input; process_async; stdlib domain-error fixtures | None; published and live-verified |
| SHA-256 and gzip | Resource walkthrough and two standalone examples | async_digest; async_gzip; cpu_resources; resource_errors | None; published and live-verified |
| TCP and HTTP | Reference/migration; diagnostic-client example; existing HTTP server and uptime guides | async_stdlib_examples HTTP 200/404/invalid URL; TCP canonical and HTTP diagnostic/stream/uptime fixtures | None; published and live-verified |
| Pure values and test support | Reference links; API recipes distinguish canonical and compatibility APIs; checked reporter examples | Updated decode_message, testing_report and test_booking_limits; existing UTF-8/value-method/testing fixtures | None; published and live-verified |

Regression fixtures test failures, malformed input, bounds, cancellation and
native ownership. They support claims but are not substitutes for user-facing
walkthroughs. The four recorded core gaps now have implemented, focused-validated examples and
guides. Website PR #77 is published with nineteen module references, 72 function
entries, four guides and nine complete examples. All signatures match pinned core
source. The 51 local website tests, assembly checks, hosted checks and deployment
verification passed. Desktop and phone-width layouts were reviewed and approved.
No documentation acceptance remains for AS5. Do not add an arbitrary example quota
or require replacing every compatibility caller. Published-article changes still
require separate owner review.

Acceptance for each core item: current imports and effects; complete source code;
source/compile/saved-bytecode evidence where executable; honest limits and cleanup;
links from the relevant guide/index; development-versus-release labelling. Website
acceptance additionally needs the real site layout, persistent Panack HTTP preview
and owner review. Core integration and website publication remain separate.

23 November publication acceptance: the owner authorised website checks and publication.
[Website PR #77](https://github.com/sproates/panackelty-website/pull/77) merged as
`e59e93e` after [checks 45165626275](https://github.com/sproates/panackelty-website/actions/runs/45165626275)
passed. [Deployment 45166480787](https://github.com/sproates/panackelty-website/actions/runs/45166480787)
passed, including verification of deployed bytes and provenance. Live browser review
confirmed the Time reference, signatures, errors, complete example and stdlib-before-HTTP
navigation. The [published reference](https://panackelty.com/reference/standard-library/)
contains nineteen module pages and 72 function entries, alongside four guides and nine
complete examples. AS5 is accepted: all five slices of the expanded async stdlib and
documentation batch are complete (100/100 weighted points). RM#162 remains open for
broader real-program work, including anonymous functions, closures and shared state.
These are alpha.16 development docs; release pins, the playground and published articles
are unchanged. This completion record changes no runtime or build behaviour and requires
no performance measurement or repeated product validation.
