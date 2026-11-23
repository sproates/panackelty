# Async stdlib documentation coverage

Updated: 23 November 2026. Part of RM#162 / AS5, on the alpha.16 development
branch. This is the finite review checklist for the agreed API domains; it does
not add new language or library scope. Source signatures remain linked from the
[reference](../reference/STANDARD_LIBRARY.md).

| Domain | User-facing material | Executable evidence | Remaining documentation work |
| --- | --- | --- | --- |
| Imports, effects, compatibility | Reference and migration guide | Existing async_stdlib tests; all seven standalone async examples | Website owner review/publication |
| Clock and environment | Reference; timing example; HTTP uptime walkthrough | async_timing; http_uptime; async_stdlib | Website owner review/publication |
| Console and UTF-8 | Reference, input/EOF walkthrough and async_console_input | console_input; async_stdlib_examples (including split UTF-8, exact bound, oversized/invalid input); named examples | Website owner review/publication |
| General filesystem | Reference, migration guide, temporary-note walkthrough | async_temporary_note; async_filesystem; filesystem_errors | Website owner review/publication |
| Confined reads and staging | Reference, async_publish_note and confinement/publication walkthrough | async_publish_note; existing confined/staging ownership, cancellation and publication tests | Website owner review/publication |
| Processes | Reference, migration guide, bounded subprocess example | async_pipe_input; process_async; stdlib domain-error fixtures | Website owner review/publication |
| SHA-256 and gzip | Resource walkthrough and two standalone examples | async_digest; async_gzip; cpu_resources; resource_errors | Website owner review/publication |
| TCP and HTTP | Reference/migration; diagnostic-client example; existing HTTP server and uptime guides | async_stdlib_examples HTTP 200/404/invalid URL; TCP canonical and HTTP diagnostic/stream/uptime fixtures | Website owner review/publication |
| Pure values and test support | Reference links; API recipes distinguish canonical and compatibility APIs; checked reporter examples | Updated decode_message, testing_report and test_booking_limits; existing UTF-8/value-method/testing fixtures | Website owner review/publication |

Regression fixtures test failures, malformed input, bounds, cancellation and
native ownership. They support claims but are not substitutes for user-facing
walkthroughs. The four recorded core gaps now have implemented, focused-validated examples and
guides. Website adaptation is ready in [website PR #77](https://github.com/sproates/panackelty-website/pull/77), with five pages and nine complete examples. The 49 website tests and assembly checks pass; desktop and phone-width layouts were checked. Remaining AS5 work is owner review and publication; integration
acceptance remains pending. Do not add an arbitrary example quota or require replacing
every compatibility caller. Published-article changes still require separate
owner review; no website publication is implied by this record.

Acceptance for each core item: current imports and effects; complete source code;
source/compile/saved-bytecode evidence where executable; honest limits and cleanup;
links from the relevant guide/index; development-versus-release labelling. Website
acceptance additionally needs the real site layout, persistent Panack HTTP preview
and owner review. Core integration and website publication remain separate.

Owner review correction (23 November): website PR #77 now provides nineteen self-contained module references with 72 function entries, parameter/result/error details, bounds and ownership, plus embedded complete examples. All signatures match pinned core source; 51 local website tests and assembly checks pass. The persistent preview is refreshed. Owner review, integration and publication remain pending; no CI was triggered.

23 November integration handoff: the owner approved the website documentation approach and preview. Core PR #693 is ready for integration into next under the alpha.16 trial; its merge starts hosted validation. AS1–AS4 implementation and AS5 core guides/examples are locally validated; hosted acceptance remains pending. Website PR #77 has preview approval, not publication permission. RM#162 remains open for the broader real-program work; this stdlib batch does not implement closures, anonymous functions or shared state.
