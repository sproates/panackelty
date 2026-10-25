# Package-management final acceptance

P04c completes GI#569 and GI#200 on merge after canonical local validation and
the Ubuntu 22.04/macOS 14 hosted gates pass on the exact delivery head. P01–P04
then have 100% acceptance under the existing weights. This is acceptance of the
implementation on next, not publication of an alpha.15 release.

## Final acceptance matrix

| Requirement | Evidence |
| --- | --- |
| All four source forms in one complete program | tests/package_git_integration.sh combines a live directory with a transitive child, a local archive, an HTTPS archive and a pinned Git package; output is 55 |
| Repeatable lock generation | Delete the lock, resolve the same source inputs again and compare exact bytes; warm lock also preserves the complete cache fingerprint |
| Relocation and network-free ordinary commands | Move the project, relative directory dependency and local archive into a path containing spaces; check/compile/run and offline restore leave TCP/Git counts and lock/cache bytes unchanged |
| Actual offline usability | Stop the TLS peer, repeat the installed project routes and verify saved-bytecode output |
| Live source versus stale manifest | A transitive local source edit changes output to 56 without a new lock; a manifest edit fails with PKG_LOCK_STALE and preserves lock/cache |
| Missing lock and explicit recovery | Missing lock is not recreated by check; a missing local archive cache is not repaired by run, but explicit offline restore repairs it |
| Missing/corrupt remote cache | Ordinary commands and offline restore fail, leave state unchanged and create no compiled output; restored fixture content works with the original lock |
| Saved bytecode independence | Move the complete four-source project and dependency tree away; saved bytecode still prints the original 55 |
| Shared Git snapshots and package boundaries | The same installed integration suite retains its sibling diamond, canonical linked subdirectory, SHA-1/SHA-256, repeated alias and package-private symlink assertions |
| Remote updates and interruption | Existing mixed-source assertions retain exact-commit restore, changed-byte rejection, selected updates, unavailable commits and complete cache fingerprints around interruptions |
| Installation, native dependencies and standalone commands | P04b release-archive smoke and installed HTTPS suite retain their assertions on both platforms |
| Diagnostics and package-only build independence | P04a's 100 exact transcripts and the real package-only rebuild proof remain in canonical make check |

The new cases extend the existing Git integration fixture and run through both
the source launcher and the separately installed toolchain. The hosted workflow
uses the installation already relocated by P04b. Test setup uses Git, Node,
OpenSSL and tar; these fixture tools are not new package-manager dependencies.
The HTTP peer is counted while live before offline usability is tested, so a
successful cached build alone is not the network-isolation evidence.

No production code, compiler/native interface or bootstrap artifact changes in
P04c. The earlier [remote acceptance](PACKAGE_REMOTE_ACCEPTANCE.md),
[diagnostic audit](PACKAGE_DIAGNOSTIC_ACCEPTANCE.md) and
[installed-workflow checklist](../roadmap/records/dependency-management--idea.md#p04b-installed-workflow-acceptance)
remain part of the final evidence.

## Retained limits

Publication is atomic for each lockfile or cache entry, not one transaction
across the graph. Validation and acquisition failures preserve the old cache;
a later publication failure can require explicit restore. The main contract
now states this existing implementation boundary consistently.

Git acquisition requires Git 2.29 or newer. Git redirects, submodules and LFS
remain unsupported. Git packs have no pre-fetch byte quota; expanded snapshots,
command output and execution time retain their documented bounds. Native API
decoupling and richer HTTP TLS causes remain separate deferred work. These are
not hidden acceptance tasks or additional P04 slices.

## Performance and release boundary

This delivery adds acceptance tests and reconciles documentation; it changes no
package or compiler runtime algorithm. The full-suite elapsed time and exact
commit are recorded in the delivery PR. Extra installed checks add validation
work, not product startup or compilation work. Historical timing warnings remain
under GI#106 reassessment; no replacement performance budget is claimed.

Alpha.15 publication and website promotion are separate release work. Existing
alpha.14 download instructions remain accurate for that release. The website
follow-up register retains the package examples, prerequisites and diagnostic
article material for a matching native release.
