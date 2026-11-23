# Async stdlib documentation coverage

Updated: 23 November 2026. Part of RM#162 / AS5, on the alpha.16 development
branch. This is the finite review checklist for the agreed API domains; it does
not add new language or library scope. Source signatures remain linked from the
[reference](../reference/STANDARD_LIBRARY.md).

| Domain | User-facing material | Executable evidence | Remaining documentation work |
| --- | --- | --- | --- |
| Imports, effects, compatibility | Reference and migration guide | Existing async_stdlib tests; all five new examples | Website adaptation/review |
| Clock and environment | Reference; timing example; HTTP uptime walkthrough | async_timing; http_uptime; async_stdlib | Website adaptation/review |
| Console and UTF-8 | Reference, error guidance; all new examples use checked output | console_input; console/error fixtures; async_pipe_input and async_temporary_note | Website adaptation/review; illustrate bounded input/EOF in user-facing recipe |
| General filesystem | Reference, migration guide, temporary-note walkthrough | async_temporary_note; async_filesystem; filesystem_errors | Website adaptation/review |
| Confined reads and staging | Reference module map and linked specification | Existing confined/staging ownership, cancellation and publication tests | User-facing confinement/publication walkthrough, then website adaptation/review |
| Processes | Reference, migration guide, bounded subprocess example | async_pipe_input; process_async; stdlib domain-error fixtures | Website adaptation/review |
| SHA-256 and gzip | Resource walkthrough and two standalone examples | async_digest; async_gzip; cpu_resources; resource_errors | Website adaptation/review |
| TCP and HTTP | Reference/migration; existing HTTP client/server and uptime guides | TCP canonical and HTTP diagnostic/stream/uptime fixtures | User-facing diagnostic-client example, then website adaptation/review |
| Pure values and test support | Reference links to path/bytes/UTF-8 and testing modules; API recipes | Existing UTF-8/value-method/testing fixtures | Reconcile remaining recipe names and canonical testing imports |

Regression fixtures test failures, malformed input, bounds, cancellation and
native ownership. They support claims but are not substitutes for user-facing
walkthroughs. The remaining core documentation work is explicitly the four cells
above that name a recipe/walkthrough or reconciliation, followed by website
adaptation and review. Do not add an arbitrary example quota or require replacing
every compatibility caller. Published-article changes still require separate
owner review; no website publication is implied by this record.

Acceptance for each core item: current imports and effects; complete source code;
source/compile/saved-bytecode evidence where executable; honest limits and cleanup;
links from the relevant guide/index; development-versus-release labelling. Website
acceptance additionally needs the real site layout, persistent Panack HTTP preview
and owner review. Core integration and website publication remain separate.
