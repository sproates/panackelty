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
  Phase: checking panack.lock.json
  Dependency: demo.app -> lib (<workspace>/Ian's app/panack.json)

panack.lock.json records the exact dependencies this project uses. Panack needs that record before it can build the project.

Run the command below to select the dependencies and create panack.lock.json. Review and commit that file with the project.

  panack deps lock --project '<workspace>/Ian'"'"'s app/panack.json'
```

## Locked content is missing

Command: `panack run --project "<workspace>/Ian's app/panack.json"`.

The package cache entry was moved aside while the lock and cache state remained. The suggested restore command recreates it and preserves the lock.

```text
panack: error[PKG_CACHE_MISSING]: missing cache: <workspace>/Ian's app/.panack_modules/demo.library

  Project: <workspace>/Ian's app/panack.json
  Operation: project preparation (network-free)
  Phase: checking saved packages
  Dependency: demo.app -> lib (<workspace>/Ian's app/panack.json)

Panack cannot find all the saved files needed to use the dependencies in panack.lock.json.

Run the command below to obtain the missing package without changing the versions or contents recorded in panack.lock.json. An internet connection is needed for remote packages.

  panack deps restore --project '<workspace>/Ian'"'"'s app/panack.json'
```

## Cached content was modified

Command: `panack run --project "<workspace>/Ian's app/panack.json"`.

A cached source file was overwritten. It is refused before compilation. The suggested restore repairs it, and the complete program runs successfully with the same lock.

```text
panack: error[PKG_CACHE_INVALID]: saved package files do not match their recorded checksum

  Project: <workspace>/Ian's app/panack.json
  Operation: project preparation (network-free)
  Phase: checking saved packages
  Dependency: demo.app -> lib (<workspace>/Ian's app/panack.json)

Panack could not verify the saved local packages against panack.lock.json. It will not use them for this build.

Run the command below to restore and check the packages recorded in panack.lock.json. Keep the lockfile unchanged so the repair uses the same package contents.

  panack deps restore --project '<workspace>/Ian'"'"'s app/panack.json'
```

## The archive changed after it was locked

Command: `panack deps restore --project "<workspace>/Ian's app/panack.json"`.

The local archive was replaced with different content. The failure preserves the lock, cache state and old cached program. Restoring the exact original archive bytes makes restore and run succeed. An update is a deliberate trust decision, not the default remedy.

```text
panack: error[PKG_INTEGRITY]: archive contents do not match the SHA-256 checksum in panack.lock.json

  Project: <workspace>/Ian's app/panack.json
  Operation: deps restore
  Phase: checking package contents
  Dependency: demo.app -> lib

This package has different contents from those recorded in panack.lock.json. Panack has refused to use it.

Ask the package maintainer why the contents changed, or restore the original package contents. Use panack deps update only after you have reviewed and accepted the new contents.
```

## A dependency of a dependency is missing

Command: `panack deps update --project "<workspace>/Ian's app/panack.json"`.

The app depends on outer, whose inner dependency points to a missing directory. Both declaring edges and the missing manifest path are shown.

```text
panack: error[PKG_MANIFEST_MISSING]: package manifest was not found: <workspace>/missing/panack.json

  Project: <workspace>/Ian's app/panack.json
  Operation: deps update
  Phase: finding panack.json
  Dependency: demo.app -> outer (<workspace>/Ian's app/panack.json)
  Dependency: demo.outer -> inner (<workspace>/outer/panack.json)

Panack looked for panack.json at the location shown above and could not find it. It needs that file to read the package definition.

Check the dependency path in the declaring panack.json and make sure the package contains panack.json at the location shown above.
```

## A manifest is not valid JSON

Command: `panack deps update --project "<workspace>/Ian's app/panack.json"`.

The manifest contains {broken. The parser location and manifest path identify what must be corrected.

```text
panack: error[PKG_MANIFEST]: invalid project manifest at byte 1: expected JSON string

  Project: <workspace>/Ian's app/panack.json
  Operation: deps update
  Phase: reading panack.json
  Context: manifest <workspace>/Ian's app/panack.json

Panack cannot load this package until the problem above is corrected.

Fix the reported problem in panack.json, then retry the command. If this is someone else's package, send them this error so they can fix it.
```

## A lockfile is not valid JSON

Command: `panack deps restore --project "<workspace>/Ian's app/panack.json"`.

The lock contains {broken. Guidance favours recovering the known selection from version control rather than discarding it.

```text
panack: error[PKG_LOCK_INVALID]: invalid lockfile: expected JSON string

  Project: <workspace>/Ian's app/panack.json
  Operation: deps restore
  Phase: checking panack.lock.json

Panack cannot read a valid dependency record from panack.lock.json, so it cannot determine which packages to use.

Restore panack.lock.json from version control or obtain a valid copy from the project maintainer. Deleting it and creating a new lock may select different packages.
```

## An update names an unknown alias

Command: `panack deps update --project "<workspace>/Ian's app/panack.json" absent`.

The selected alias does not occur in either graph. The first selection error is preserved instead of being overwritten by later consequences.

```text
panack: error[PKG_SELECTIVE]: unknown dependency alias: absent

  Project: <workspace>/Ian's app/panack.json
  Operation: deps update
  Phase: checking selected dependencies

The name above is not a direct dependency in this project or its existing lockfile. The lockfile has not been changed.

Use a name from the dependencies section of your project panack.json, then retry. Use the declaration name, which may differ from the package name.
```

## The archive server returns 404

Command: `panack deps update --project <workspace>/app/panack.json`.

The local TLS peer returns 404. The message preserves the status and explains that a downloadable archive is required.

```text
panack: error[PKG_HTTP_STATUS]: archive server returned HTTP 404; expected 200

  Project: <workspace>/app/panack.json
  Operation: deps update
  Phase: checking the download response
  Dependency: test.app -> lib (<workspace>/app/panack.json)

The server returned the HTTP status shown above instead of the requested archive.

Check the archive URL in panack.json. It must be directly downloadable without signing in. Ask the package maintainer to correct a missing or inaccessible download.
```

## The HTTPS connection fails verification

Command: `panack deps update --project <workspace>/app/panack.json`.

The test removes trust in its private certificate. No HTTP request reaches the peer. The diagnostic does not recommend disabling certificate checks or imply that every TLS failure is an expired certificate.

```text
panack: error[PKG_HTTP]: could not establish a trusted HTTPS connection to the archive server

  Project: <workspace>/app/panack.json
  Operation: deps update
  Phase: downloading the archive
  Dependency: test.app -> lib (<workspace>/app/panack.json)

The secure connection failed before the archive could be downloaded. This error alone does not identify a particular certificate problem.

Check your system clock and trusted certificates. If they are correct, ask the server administrator to check its HTTPS configuration. Do not disable certificate verification.
```

## A locked Git commit cannot be acquired

Command: `panack deps restore --project <workspace>/app/panack.json`.

The test replaces the remote repository with an empty reachable repository, so its locked commit is unavailable. Git returns 128. That status alone does not prove the cause; the message gives the repository, commit and safe investigation steps without inventing a diagnosis. Returning the original repository makes restore succeed without changing the pin.

```text
panack: error[PKG_GIT_FETCH]: Git command failed with exit status 128; remote error text withheld to protect credentials

  Project: <workspace>/app/panack.json
  Operation: deps restore
  Phase: preparing files from Git
  Dependency: git.app -> left (<workspace>/app/panack.json)
  Context: repository https://localhost:<port>/sha.git at commit <locked-commit>

Panack could not prepare the package files from this Git commit. The failure reported above is the information available from this operation.

If the message identifies a problem with the repository contents, ask its maintainer to fix it. If Git only reports a failed command, check connectivity and repository access; that status alone does not identify the cause. Retry the same command and keep the existing lockfile.
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

## An update would change an unselected dependency

Command: `panack deps update --project "<workspace>/Ian's app/panack.json" other`.

The project has removed its previous `lib` declaration and added `other`, but the command selects only `other`. Panack identifies `lib` as the additional change and leaves the lockfile unchanged. This is a different explanation from an unknown dependency name.

```text
panack: error[PKG_SELECTIVE]: unselected dependency changed: lib

  Project: <workspace>/Ian's app/panack.json
  Operation: deps update
  Phase: checking selected dependencies

This update would also change the dependency named above, but you did not select it. The lockfile has not been changed.

Check its declaration in panack.json. Undo the unintended change, or include this dependency name in the update command if you intend to update it too.
```

## Diagnostic review scope

The wording review covers every package module that emits errors: command
arguments, manifest and lock parsing, graph validation, local and remote
acquisition, selective updates, archive headers and extraction, Git refs,
file inventories, symbolic links, saved packages and checksum operations.
The shared JSON parser remains compiler input; its existing syntax details
are retained beneath the package explanation without changing compiler code.

Messages report the detected failure, not all the ways their error family can
fail. An unknown dependency name and an unselected change have separate advice.
A changed project name, version or toolchain version is identified individually.
DNS, timeout and TLS failures have different recovery guidance. A Git command
exit status does not establish whether the cause is networking, permissions or
server configuration; the message states that limitation.

Archive errors distinguish duplicate paths, case conflicts, file/directory
conflicts, invalid paths, size bounds and incomplete metadata. Symbolic-link
errors identify an absolute target, an escape or the observed 40-link limit;
the latter does not assert that a cycle has been proved. Lockfile validation
identifies the failing project field instead of reporting a generic identity
problem. Existing rejection rules and resource bounds are unchanged.

The 21 diagnostic unit checks and twelve public transcripts are representative
regression evidence, not a claim that every low-level failure has its own
end-to-end fixture. The existing archive, Git, graph, containment and lock tests
continue to exercise the wider rejection surface. Exact transcript matching
keeps this document current; human review is still needed to judge whether an
explanation is useful.
