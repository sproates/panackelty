# Package diagnostics: verified examples

These examples record the experience delivered by P04a on unreleased `next`.
They are source material for a later article, not a published-release announcement.
Captured on 25 October 2026 for the alpha.15 programme; the development tree still
reports VERSION 0.1.0-alpha.14. The installed alpha.14 release does not include
this package workflow.

Each message explains what happened, why the operation cannot proceed and what
the user can safely do next. The package tool writes failures to stderr and
returns status 1; successful command output is unchanged. Codes identify error
families, while the explanation remains essential. No JSON diagnostic mode is
introduced.

## How to reproduce and maintain these examples

Build with `make package-tool`, then run `sh tests/package_diagnostics.sh`,
`sh tests/package_https.sh` and `sh tests/package_git.sh`. The remote tests use
private local TLS peers, not external services. Optional
`PANACK_DIAGNOSTICS_RECORD=/absolute/output/directory` captures the observed
messages without skipping assertions or updating expected files.

The transcripts below are actual stderr with only the temporary root replaced by
`<workspace>`, the TLS port by `<port>` and the generated Git commit by
`<locked-commit>`. Replace placeholders before using an example command. Shell
quoting in recovery commands is intentional: the fixture project is named
`Ian's app`. Tests execute the suggested commands and verify the recovered
program prints `42`, with its lock unchanged after cache repair.

Exact transcript fixtures live in `tests/fixtures/package_diagnostics`. Tests
check both the emitted output and its presence in this document. Changes to
wording require review of the explanation and recovery, not blind snapshot
regeneration.

## A project has no lock

Command: `panack check --project "<workspace>/Ian's app/panack.json"`.

The app declares a local archive, but has no lockfile. The test executes the suggested initial lock command, then runs the program successfully. Creating a lock selects content; it is not described as a pin-preserving repair.

```text
panack: error[PKG_LOCK_MISSING]: missing lockfile

  Project: <workspace>/Ian's app/panack.json
  Operation: project preparation (network-free)
  Phase: lock verification
  Dependency: demo.app -> lib (<workspace>/Ian's app/panack.json)

Dependencies need a lockfile so the selected packages can be reproduced.

Create the initial lock with the lock command below. This selects dependency content.

  panack deps lock --project '<workspace>/Ian'"'"'s app/panack.json'
```

## Locked content is missing

Command: `panack run --project "<workspace>/Ian's app/panack.json"`.

The package cache entry was moved aside while the lock and cache state remained. The suggested restore command recreates it and preserves the lock.

```text
panack: error[PKG_CACHE_MISSING]: missing cache: <workspace>/Ian's app/.panack_modules/demo.library

  Project: <workspace>/Ian's app/panack.json
  Operation: project preparation (network-free)
  Phase: cache verification
  Dependency: demo.app -> lib (<workspace>/Ian's app/panack.json)

The locked dependency content is not available in this project's cache.

Run the restore command below. Remote sources require a connection; restore preserves the existing pins.

  panack deps restore --project '<workspace>/Ian'"'"'s app/panack.json'
```

## Cached content was modified

Command: `panack run --project "<workspace>/Ian's app/panack.json"`.

A cached source file was overwritten. It is refused before compilation. The suggested restore repairs it, and the complete program runs successfully with the same lock.

```text
panack: error[PKG_CACHE_INVALID]: cache tree digest mismatch

  Project: <workspace>/Ian's app/panack.json
  Operation: project preparation (network-free)
  Phase: cache verification
  Dependency: demo.app -> lib (<workspace>/Ian's app/panack.json)

The cached content or its state does not match the verified dependency selection. It will not be used.

Run the restore command below to reacquire and verify the locked content. Do not update the lock to conceal cache corruption.

  panack deps restore --project '<workspace>/Ian'"'"'s app/panack.json'
```

## The archive changed after it was locked

Command: `panack deps restore --project "<workspace>/Ian's app/panack.json"`.

The local archive was replaced with different content. The failure preserves the lock, cache state and old cached program. Restoring the exact original archive bytes makes restore and run succeed. An update is a deliberate trust decision, not the default remedy.

```text
panack: error[PKG_INTEGRITY]: archive digest mismatch

  Project: <workspace>/Ian's app/panack.json
  Operation: deps restore
  Phase: content verification
  Dependency: demo.app -> lib

The supplied bytes do not match the content recorded in the lockfile. Accepting them would change the dependency you agreed to use.

Check the source with its maintainer or restore the original bytes. Only use deps update after deliberately reviewing and accepting the changed content.
```

## A dependency of a dependency is missing

Command: `panack deps update --project "<workspace>/Ian's app/panack.json"`.

The app depends on outer, whose inner dependency points to a missing directory. Both declaring edges and the missing manifest path are shown.

```text
panack: error[PKG_MANIFEST_MISSING]: package manifest was not found: <workspace>/missing/panack.json

  Project: <workspace>/Ian's app/panack.json
  Operation: deps update
  Phase: manifest discovery
  Dependency: demo.app -> outer (<workspace>/Ian's app/panack.json)
  Dependency: demo.outer -> inner (<workspace>/outer/panack.json)

Every package needs a panack.json at its declared root. That manifest or one of its parent directories could not be found.

Check the dependency path in the declaring manifest and make sure the package exists at the location shown above.
```

## A manifest is not valid JSON

Command: `panack deps update --project "<workspace>/Ian's app/panack.json"`.

The manifest contains {broken. The parser location and manifest path identify what must be corrected.

```text
panack: error[PKG_MANIFEST]: invalid project manifest at byte 1: expected JSON string

  Project: <workspace>/Ian's app/panack.json
  Operation: deps update
  Phase: manifest validation
  Context: manifest <workspace>/Ian's app/panack.json

This manifest cannot describe a valid package for the current toolchain.

Correct the indicated field or location in panack.json, then retry. For a third-party package, report the manifest problem to its maintainer.
```

## A lockfile is not valid JSON

Command: `panack deps restore --project "<workspace>/Ian's app/panack.json"`.

The lock contains {broken. Guidance favours recovering the known selection from version control rather than discarding it.

```text
panack: error[PKG_LOCK_INVALID]: invalid lockfile: expected JSON string

  Project: <workspace>/Ian's app/panack.json
  Operation: deps restore
  Phase: lock verification

The lockfile cannot provide a trustworthy dependency selection.

Restore a valid lockfile from version control. Do not delete it merely to bypass this failure.
```

## An update names an unknown alias

Command: `panack deps update --project "<workspace>/Ian's app/panack.json" absent`.

The selected alias does not occur in either graph. The first selection error is preserved instead of being overwritten by later consequences.

```text
panack: error[PKG_SELECTIVE]: unknown dependency alias: absent

  Project: <workspace>/Ian's app/panack.json
  Operation: deps update
  Phase: selective update validation

This update would change a dependency outside the selected aliases, or the selection is invalid.

Check the root aliases and shared dependencies. Select all affected aliases together, or review an explicit update of the whole graph.
```

## The archive server returns 404

Command: `panack deps update --project <workspace>/app/panack.json`.

The local TLS peer returns 404. The message preserves the status and explains that a downloadable archive is required.

```text
panack: error[PKG_HTTP_STATUS]: archive server returned HTTP 404; expected 200

  Project: <workspace>/app/panack.json
  Operation: deps update
  Phase: HTTPS response validation
  Dependency: test.app -> lib (<workspace>/app/panack.json)

The server did not return the downloadable archive required by this dependency.

Check that the URL points to an available .tar.gz, rather than a web page or login. Package retrieval does not prompt for credentials; ask the source maintainer for an accessible HTTPS archive.
```

## The HTTPS connection fails verification

Command: `panack deps update --project <workspace>/app/panack.json`.

The test removes trust in its private certificate. No HTTP request reaches the peer. The diagnostic does not recommend disabling certificate checks or imply that every TLS failure is an expired certificate.

```text
panack: error[PKG_HTTP]: archive TLS connection could not be verified or established

  Project: <workspace>/app/panack.json
  Operation: deps update
  Phase: HTTPS acquisition
  Dependency: test.app -> lib (<workspace>/app/panack.json)

A complete, acceptable HTTPS response was not received. The dependency cannot be verified.

Check connectivity, the source URL and TLS trust, then retry the original command. For an existing lock, deps restore preserves its pins. Do not disable certificate verification.
```

## A locked Git commit cannot be acquired

Command: `panack deps restore --project <workspace>/app/panack.json`.

The test replaces the remote repository with an empty reachable repository, so its locked commit is unavailable. Git returns 128. That status alone does not prove the cause; the message gives the repository, commit and safe investigation steps without inventing a diagnosis. Returning the original repository makes restore succeed without changing the pin.

```text
panack: error[PKG_GIT_FETCH]: Git command failed with exit status 128; remote error text withheld to protect credentials

  Project: <workspace>/app/panack.json
  Operation: deps restore
  Phase: Git snapshot acquisition
  Dependency: git.app -> left (<workspace>/app/panack.json)
  Context: repository https://localhost:<port>/sha.git at commit <locked-commit>

Git could not produce a verified snapshot of the requested commit. The cause may be connectivity, server access, an unavailable commit or unsupported repository content; this message does not establish which.

Check connectivity and TLS trust, then ask the repository maintainer whether the recorded commit is available over the direct HTTPS URL. Retry the original command once corrected. For an existing lock, deps restore keeps the same pin.
```

## Boundaries and acceptance

Diagnostic records and rendering belong to the separate Panack package tool.
Compiler sources, native interfaces, bootstrap artifacts and the resolved-project
descriptor are unchanged. Context is attached as the graph unwinds; lower-level
causes survive instead of being replaced by a generic cache failure.

Raw Git stderr and remote headers are not printed because they may contain
credentials or server-controlled text. Git exit/signal status is retained; an
unknown cause remains unknown. HTTP error variants preserve known DNS, connection,
TLS, timeout, status and protocol distinctions. Terminal controls in displayed
text are escaped, and actionable commands quote project paths for POSIX shells.

Failure guidance does not claim that the whole graph is transactional. Publication
is atomic per entry; its diagnostic explicitly warns that the lock or earlier
entries may already have changed and recommends restore after the filesystem
problem is corrected. No diagnostic introduces implicit downloads or accepts
unexpected content.

P04b installed workflow/help and P04c all-source final acceptance remain open.
Website promotion and an article remain separate work.
