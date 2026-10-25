# P03 remote-source acceptance

[PR#582](https://github.com/sproates/panackelty/pull/582) completes the pinned
remote-source packet when it merges with its
Linux and macOS checks green. P04 installation, help, diagnostics and final
all-source workflow acceptance remain separate. GI#200 stays open through P04.

## Acceptance matrix

The tests use private TLS peers and complete programs through the public CLI.
They observe TCP connections and Git invocations, rather than inferring absence
of network access from a successful cached run.

| Requirement | Executable evidence |
| --- | --- |
| Moved refs retain full-commit pins; explicit update selects new content | `tests/package_git_integration.sh`: annotated tag, default HEAD, full branch/commit, SHA-1 and SHA-256 |
| Unavailable locked commits fail without selecting another revision | Same integration test: reachable empty SHA-256 repository, unchanged lock/state, recovery when the repository returns |
| Exact archive bytes, redirect and transport policy | `tests/package_https.sh`: independent compressed SHA-256, changed bytes, redirect bounds/downgrade/credentials, TLS trust, status, size and truncation |
| Shared snapshots and package containment | Git integration: canonical subdirectory, siblings, diamond identity and package-private links; `tests/package_git.sh`: links, submodules, LFS, isolated configuration |
| Corrupt caches and interrupted acquisition preserve state | Mixed Git/HTTPS integration: fingerprint every cache path and file before/after separate HTTP and Git interruptions and changed-byte rejection; compare exact lock/state bytes |
| Selective updates preserve the other source | Mixed integration: advance both peers, update each root alias separately, compare unselected package and edge records, observe no unselected transport access |
| Warm and cold offline behaviour | HTTPS and Git integration: warm verification, cold rejection, no TCP connections or Git invocation; mixed graph check/compile/run and saved bytecode |
| Safe publication and cleanup | `tests/package_dependencies.sh`: symlinked cache/state rejection; Git/HTTPS integration: no staging remnants after success or failure |
| Package-only build independence | `tests/package_tool_handoff.sh`: changed tool behaviour with unchanged compiler/seed artifacts and no compiler build |

P03d extends the existing Git integration route, which is included in canonical
`make check` and the HTTP/TLS Linux and macOS workflow. It adds no production
source, native API, compiler change, bootstrap artifact or dependency. Existing
source-coverage classification is unchanged; these are public-command acceptance
tests, not a claim of instrumented source coverage.

## Validation

On 25 October 2026 the focused Git integration test and `make docs` passed.
Canonical `make check` passed in 792 seconds, including the mixed-source cases,
package-tool build independence and unchanged bootstrap fixed points. The
previous P03c run took 699 seconds; different workloads and machine conditions
mean this is not a paired performance comparison. Hosted Linux and macOS gates
must pass on the exact PR head before merge.

## Interpretation and remaining limits

Failure preservation is tested before publication: acquisition or graph validation
fails, so the old cache and lock remain intact. Publication remains atomic per
entry, not a transaction across the whole graph. A late publication failure can
require restore, as the P01/P02 contract already records.

Git 2.29+ is required for acquisition. Git redirects remain rejected. The fetched
Git pack has no pre-fetch byte quota; expanded snapshots are bounded at 256 MiB
and 16,384 entries. Individual Git commands retain the existing 60-second and
16 MiB output limits. No repository code, hooks, filters or credentials run.

This delivery changes only tests and documentation, so it has no package runtime
performance impact. The additional integration cases increase validation work;
full-suite elapsed time is recorded with the delivery evidence. Small local-peer
cold/warm timings remain observations, not production throughput estimates.
P04 retains installed artifacts, all four source forms together, relocation and
final command/help/diagnostic acceptance. Website promotion waits for P04 and a
matching native release. Native-interface decoupling remains alpha.16 work.
