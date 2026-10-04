# Panackelty release policy

Panackelty `0.1.0-alpha.11` is the developer-preview release candidate.
Published download defaults remain alpha.10 until alpha.11 public artifacts pass
verification. It is intended for learning,
experimentation, feedback, and non-critical terminal programs. It is not yet
recommended for production systems or irreplaceable data.

## Supported systems

The initial preview targets:

- Ubuntu 22.04 or newer on x86-64
- macOS 14 or newer on Apple silicon (arm64)

Each supported release archive must pass the exact-artifact smoke test on its
target system. Other POSIX-like systems may work from source but are best effort.
Windows and other processor architectures are not supported by the initial
preview.

Downloaded toolchains require only the operating system, a terminal, `tar`, and
the platform's standard SHA-256 utility for download verification. The archive
includes the compiler, VM, and standard library needed to build and run programs.

## Release gate

Merging to `main` runs checks and creates CI artifacts; it does not publish a
release. Prepare version and changelog updates in a release PR. Keep verified download
links and installer defaults on the published version until the new archives
pass public-artifact verification, then promote them in a follow-up.
After its required checks pass and it is merged, use either release entry point:

- Push an annotated version tag on the merged commit. Its message supplies the
  public release notes.
- In GitHub Actions, select **Release → Run workflow**, select `main`, and confirm
  the exact `VERSION` value and full 40-character commit SHA currently on `main`.
  The workflow rejects branch, version or commit mismatches before building.
  If `main` advances before dispatch, refresh the SHA and review the new commit.
  Manual releases take their notes from the matching nonempty changelog section.

Both paths pin every checkout to the event's commit. A release tag must exactly
match `v` followed by `VERSION`. The workflow runs the complete development suite,
then independently builds and smoke-tests Linux x86-64 and macOS arm64 archives.
Only after both matrix jobs succeed does the final job download their retained
archives and verify SHA-256 checksums and source-commit provenance.

The Linux package job also builds the platform-independent `panackelty-browser-runtime-<version>.tar.gz` dependency bundle and checksum. The publish gate verifies it alongside the native archives; downstream browser releases can pin this immutable core asset instead of cloning the core repository.

Only the final publish job has repository write permission. For a manual release it
creates an annotated tag at the validated commit, then publishes the prerelease
in the same run: tags created using the workflow token do not start another
release workflow. No personal access token or terminal credentials are needed.
Existing tags are accepted only if annotated and pointing to that exact commit;
they are never moved. Existing releases are never edited or overwritten. A failed
publication can be retried with the same commit and matching tag if no release
was created. If GitHub created a partial release, inspect it before recovery;
the workflow will not overwrite it. Release runs are serialized and never cancel
an in-progress publication.

## Compatibility during the preview

Panackelty uses semantic versions with prerelease identifiers. While the project
is in alpha:

- Source syntax, type-checking behavior, and standard-library APIs may change
  between preview releases. Changes will be described in `CHANGELOG.md`.
- The command names `check`, `compile`, `run`, and `disasm` form the preview CLI
  surface. Incompatible CLI changes require release notes and a new preview
  version.
- Bytecode is an exchange format within one toolchain release, not a durable
  distribution format. The VM currently accepts bytecode version 9 only, and
  compatibility with bytecode produced by another Panackelty release is not
  promised. Version 8 artifacts must be recompiled from source; loaders reject
  them explicitly. The alpha.10 v9 seed replaced the alpha.9 v8 seed; alpha.11 retains v9
  with an updated compiler and additive TCP operations. Use the matching compiler,
  VM and standard library together.
- Patch releases in the same preview series should correct defects without
  deliberately changing accepted source programs.

The language specification remains authoritative for accepted behavior. Items
listed under its deliberately postponed section are not part of the preview
contract.

## Support lifetime

Only the latest developer-preview release receives fixes. A newer preview
supersedes earlier preview artifacts. Security handling is described in
`SECURITY.md`.

## Optional installer maintenance

The optional `scripts/install.sh` has an explicit default release version; it
never guesses GitHub's latest release (which may omit prereleases). After a new
release's supported archives and checksums are public and verified, update that
default, README installer examples, and `tests/release_installer.sh`'s explicit
repeat-version check together. Run the published-release acceptance on both
supported platforms before promoting the new default or website guidance.
Preserve manual installation as an alternative. Changes to archive layout,
platform baselines or installer ownership markers require corresponding safety
and upgrade tests; never overwrite existing release assets to repair them.

## Website release history

`CHANGELOG.md` is the sole source of release-note content. Every website assembly
and local preview generates `releases.html` with `scripts/release_history.sh`;
no generated notes are maintained in `site/`. The shared Pages assembly and
canonical `tests/pages.sh` gate run this without Node or Python. Website validation
also checks the generated page’s links and deployed bytes. Changelog, generator,
template and availability pins all invalidate the Pages input fingerprint.

Use a first `## Unreleased` section, followed by descending dated alpha headings
such as `## 0.1.0-alpha.10 — 2026-09-30`. Supported content is plain paragraphs,
`###` categories, hyphen bullets with two-space continuations and inline backtick
code. Unsupported Markdown, duplicate/empty sections, malformed dates and a
missing published version fail generation. Keep compatibility and migration
instructions with their version; do not describe staged infrastructure as an
executable feature. Historical entries retain their original scope.

`site/native-release.txt` records the latest **verified published** native release,
independently of `VERSION`. Promote it only after supported public archives and
checksums have been verified; keep homepage and installer guidance consistent.
A release preparation PR may add newer dated notes without promoting that pin:
the page labels those notes as prepared and provides no download links. Unreleased
source changes remain collapsed and explicitly unavailable in published binaries.
`site/playground.json` independently pins the browser package; native notes never
imply that the playground supports the same host capabilities. This page does not
publish a release, promote either runtime, or replace exact-artifact acceptance.

Before proposing release preparation, run `sh tests/pages.sh` and build a local
preview. Review the generated history alongside the canonical changelog and then
follow the existing release gate. Every visual update still requires a working
preview before merge approval and live verification after authorised publication.
