# P04a diagnostic acceptance

P04a has a finite boundary: the package tool's existing failure paths for commands,
manifest and lock input, dependency resolution, acquisition, verification and
publication. This audit covers all 24 `.panack` modules in `src/packages`, plus
the shared JSON and HTTP errors those modules translate. It does not add package
features or require a separate transcript for every possible input value.

The completion change corrects the remaining HTTP/redirect explanations, names
unknown manifest fields correctly, identifies filesystem destinations and replaces
terse internal wording that could reach a user. The compiler, native adapters and
bootstrap sources are unchanged. P04b installation and P04c programme acceptance
remain separate work.

## Fixed acceptance checklist

| Requirement | Evidence | Disposition |
| --- | --- | --- |
| Explain the detected cause; separate known alternatives | All producers reviewed below; contrasting redirect, response, manifest, lock and cache cases | Complete |
| Identify project, dependency and affected location | Full lock and cache paths; alias/package labels; declaring manifests and transitive chains | Complete |
| Give relevant, safe recovery | Commands exercised by `package_diagnostics.sh`; changed-content updates distinguished from restore; conflicting files preserved | Complete |
| Preserve causes through wrappers | Structured `PackageProblem` propagation; filesystem host details; archive errors preserved through HTTP streaming | Complete |
| Avoid invented certainty and secret disclosure | TLS/Git limitations stated; rejected redirect values and raw Git stderr omitted; terminal controls escaped | Complete |
| Document actual user-visible output | 100 exact CLI transcripts in the [examples](PACKAGE_DIAGNOSTIC_EXAMPLES.md), compared with fixtures by functional tests | Complete |
| Verify corrections and existing behaviour | Diagnostic units, public workflows, full `make check`, and exact-head Linux/macOS PR checks | Validation results recorded below |

There is no further diagnostic catalogue queued behind this checklist. New
features need their own diagnostics when implemented; a subsequently discovered
defect should identify the failing path and expected behaviour rather than reopen
an unspecified wording programme.

## Source audit

Paths below are relative to `src/packages`. Tests are relative to `tests`.
Every source module is accounted for, including modules that only propagate
errors or provide non-diagnostic helpers.

| Module | Reviewed responsibility | Verification |
| --- | --- | --- |
| `main.panack` | Command boundary, lock requirement, final rendering | `package_diagnostics.sh`, `package_validation_diagnostics.sh` |
| `command_diagnostic.panack` | Missing/unsupported operations, options, project path, selected names | `runner/package_diagnostic_unit.panack`, command transcripts |
| `diagnostic.panack` | Error-family explanations, context, quoting, escaping, recovery and publication | Diagnostic units and all transcript suites |
| `input_diagnostic.panack` | Read/encoding/size/depth/JSON/semantic distinctions; no invented source position | Diagnostic units, input transcripts |
| `manifest.panack` | Required/unknown fields, values, exports, dependency paths, Git selectors and URLs | `runner/package_manifest_unit.panack`, validation transcripts |
| `lock.panack` | Records, source kinds, identities, hashes, graph consistency | `runner/package_lock_unit.panack`, `package_lock_diagnostics.sh` |
| `local_lock.panack` | Changed manifests, archive/tree mismatch, lock read/write and concurrent changes | Lock transcripts, `package_dependencies.sh` |
| `selective_update.panack` | Unknown aliases, unselected changes, changed project identity | Diagnostic units, selected-update transcripts |
| `project_graph.panack` | Missing/app packages, containment, cycles, conflicting identities, overlapping roots, graph limits | Validation transcripts, `functional/cases/cli_commands/package_graph_limits.sh`, remote suites |
| `acquisition.panack` | Local/HTTPS pins, cache/state inspection, directory conflicts, publication | Diagnostic/lock suites, `package_https.sh` |
| `archive.panack` | Source reads, gzip, extraction and cleanup failures | `package_archive_diagnostics.sh`, archive units |
| `tar_header.panack` | Tar/PAX syntax, format, path and numeric causes | `runner/package_tar_unit.panack`, archive transcripts |
| `tar_stream.panack` | Truncation, entry/path/count conflicts, extracted writes and missing manifest | Archive units and transcripts |
| `tree_digest.panack` | Local tree reads, file kinds, limits, concurrent writes and checksum mismatch | Archive/cache transcripts and archive units |
| `remote_archive.panack` | HTTP variants/statuses, response limits, redirect decisions, remote extraction | Diagnostic units, `package_https.sh` |
| `git_acquisition.panack` | Git lock records, shared snapshot identity, publication and offline cache | `package_git_integration.sh` |
| `git_refs.panack` | Git discovery/version, missing/ambiguous/malformed reference results | `runner/package_git_unit.panack`, `package_git_diagnostics.sh` |
| `git_process.panack` | Launch, exit, signal, timeout, output limits and isolation | Git units and controlled executable CLI scenarios |
| `git_inventory.panack` | File listings, IDs, kinds, submodules, paths and repository bounds | Git units, `package_git.sh` |
| `git_links.panack` | Relative targets, escaping and bounded link chains | Git units, Git CLI scenarios |
| `git_snapshot.panack` | Object output, LFS, size changes, staging, temporary files and commit identity | Git units, Git HTTPS scenarios |
| `git_cache.panack` | Local Git entries, link validation, file contents and checksum mismatch | Git cache corruption/restore scenarios |
| `sha256.panack` | Generic digest errors translated into package-checksum wording | `runner/package_sha256_unit.panack`; defensive host errors reviewed in source |
| `case_fold.panack` | Path-comparison helper; no diagnostic producer | `runner/package_case_fold_unit.panack` |

The two `PKG_GRAPH` acquisition values used to request a refresh are internal
control-flow sentinels, not user messages. Missing-lock placeholders are replaced
by the operation-aware renderer. Defensive failures after validated input retain
useful explanations, but this audit does not claim that every operating-system
failure has been injected in a CLI test.

The shared JSON parser remains unchanged. Package-owned unknown-field validation
avoids its unsupported record-field interpolation without rebuilding the compiler.
The HTTP API still supplies only coarse TLS/transport errors; preserving deeper
HTTPS details remains the previously deferred HTTP API task. P04a reports that
limitation honestly rather than making speculative suggestions.

## Final response cases

The HTTPS fixture now covers five-redirect exhaustion; missing, empty and duplicate
Location headers; HTTP downgrade; unsupported schemes; credentials/query/fragment/
space/backslash rejection; content and transfer encoding; incomplete and malformed
responses; the 64 MiB archive limit; and HTTP 204, 401, 403, 404, 407, 429 and 500.
Each rejected update preserves the previous lock and package files. No rejected
redirect destination is requested, and secret values do not appear in diagnostics.
Unit checks distinguish the archive, combined header/trailer, chunk-line and
informational-response limits. Existing success tests retain redirect, chunked,
selective-update, relocation and network-free workflows.

One concrete policy defect was found while making redirect failures explicit:
a non-HTTPS scheme such as `ftp:` could be interpreted as a relative path. The
package tool now rejects that destination before another request. Valid relative,
root-relative, scheme-relative and absolute HTTPS redirects remain supported.

## Validation and impact

Final local and hosted results are recorded here before completion.

Successful package operations retain their filesystem, hashing and network work.
Redirect validation adds a bounded scan of the already limited Location value;
unknown-field validation remains a bounded field/allowed-name scan. Additional
prose is built on failure. No paired performance improvement is claimed.
The package-only rebuild test continues to verify that package edits do not
rebuild the compiler or seed. Historical validation timing budgets remain subject
to GI#106 reassessment.

Website impact: none. These diagnostics belong to unreleased alpha.15; the live
alpha.14 claims remain accurate. README installation instructions, architecture,
language specification and self-hosting status were reviewed; no new installation,
syntax, component or bootstrap contract is introduced by this correction.
