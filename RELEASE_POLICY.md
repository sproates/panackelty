# Panackelty release policy

Panackelty `0.1.0-alpha.9` is a developer preview. It is intended for learning,
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
release. Prepare version, changelog, and download-link updates in a release PR.
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

Only that final job has repository write permission. For a manual release it
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
  distribution format. The VM currently accepts bytecode version 8 only, and
  compatibility with bytecode produced by another Panackelty release is not
  promised.
- Patch releases in the same preview series should correct defects without
  deliberately changing accepted source programs.

The language specification remains authoritative for accepted behavior. Items
listed under its deliberately postponed section are not part of the preview
contract.

## Support lifetime

Only the latest developer-preview release receives fixes. A newer preview
supersedes earlier preview artifacts. Security handling is described in
`SECURITY.md`.
