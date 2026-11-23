# Async stdlib documentation coverage

Updated: 23 November 2026. Part of RM#162 / AS5, on the alpha.16 development
branch. This is the finite review checklist for the agreed API domains; it does
not add new language or library scope. Source signatures remain linked from the
[reference](../reference/STANDARD_LIBRARY.md).

| Domain | User-facing material | Executable evidence | Remaining documentation work |
| --- | --- | --- | --- |
| Imports, effects, compatibility | Reference and migration guide | Existing async_stdlib tests; all seven standalone async examples | Website adaptation/review |
| Clock and environment | Reference; timing example; HTTP uptime walkthrough | async_timing; http_uptime; async_stdlib | Website adaptation/review |
| Console and UTF-8 | Reference, input/EOF walkthrough and async_console_input | console_input; async_stdlib_examples (including split UTF-8, exact bound, oversized/invalid input); named examples | Website adaptation/review |
| General filesystem | Reference, migration guide, temporary-note walkthrough | async_temporary_note; async_filesystem; filesystem_errors | Website adaptation/review |
| Confined reads and staging | Reference, async_publish_note and confinement/publication walkthrough | async_publish_note; existing confined/staging ownership, cancellation and publication tests | Website adaptation/review |
| Processes | Reference, migration guide, bounded subprocess example | async_pipe_input; process_async; stdlib domain-error fixtures | Website adaptation/review |
| SHA-256 and gzip | Resource walkthrough and two standalone examples | async_digest; async_gzip; cpu_resources; resource_errors | Website adaptation/review |
| TCP and HTTP | Reference/migration; diagnostic-client example; existing HTTP server and uptime guides | async_stdlib_examples HTTP 200/404/invalid URL; TCP canonical and HTTP diagnostic/stream/uptime fixtures | Website adaptation/review |
| Pure values and test support | Reference links; API recipes distinguish canonical and compatibility APIs; checked reporter examples | Updated decode_message, testing_report and test_booking_limits; existing UTF-8/value-method/testing fixtures | Website adaptation/review |

Regression fixtures test failures, malformed input, bounds, cancellation and
native ownership. They support claims but are not substitutes for user-facing
walkthroughs. The four recorded core gaps now have implemented, focused-validated examples and
guides. Remaining AS5 work is website adaptation and owner review; integration
acceptance remains pending. Do not add an arbitrary example quota or require replacing
every compatibility caller. Published-article changes still require separate
owner review; no website publication is implied by this record.

Acceptance for each core item: current imports and effects; complete source code;
source/compile/saved-bytecode evidence where executable; honest limits and cleanup;
links from the relevant guide/index; development-versus-release labelling. Website
acceptance additionally needs the real site layout, persistent Panack HTTP preview
and owner review. Core integration and website publication remain separate.
