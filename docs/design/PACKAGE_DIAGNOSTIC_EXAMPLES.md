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
panack: error[PKG_LOCK_MISSING]: Cannot build this project because its lockfile is missing.

  Project: <workspace>/Ian's app/panack.json
  Missing file: <workspace>/Ian's app/panack.lock.json

The lockfile records the exact dependencies the project uses and belongs alongside panack.json.

If you have an existing lockfile for this project, restore that file to the expected location. Otherwise, create one by running:

  panack deps lock --project '<workspace>/Ian'"'"'s app/panack.json'
```

## Locked content is missing

Command: `panack run --project "<workspace>/Ian's app/panack.json"`.

The package cache entry was moved aside while the lock and cache state remained. The suggested restore command recreates it and preserves the lock.

```text
panack: error[PKG_CACHE_MISSING]: Cannot build this project because dependency 'lib' (package 'demo.library') is missing locally.

  Project: <workspace>/Ian's app/panack.json
  Lockfile: <workspace>/Ian's app/panack.lock.json
  Source archive: <workspace>/library.tar.gz
  Missing directory: <workspace>/Ian's app/.panack_modules/demo.library

The lockfile records this dependency, but Panack cannot find its local files.

Restore the packages identified by the lockfile by running:

  panack deps restore --project '<workspace>/Ian'"'"'s app/panack.json'

Running this command will restore the local packages without changing the lockfile.

An internet connection is needed if remote packages must be downloaded.
```

## Cached content was modified

Command: `panack run --project "<workspace>/Ian's app/panack.json"`.

A cached source file was overwritten. It is refused before compilation. The suggested restore repairs it, and the complete program runs successfully with the same lock.

```text
panack: error[PKG_CACHE_INVALID]: Cannot build this project because the local copy of dependency 'lib' (package 'demo.library') has changed.

  Project: <workspace>/Ian's app/panack.json
  Lockfile: <workspace>/Ian's app/panack.lock.json
  Source archive: <workspace>/library.tar.gz
  Local package directory: <workspace>/Ian's app/.panack_modules/demo.library

Panack calculated a checksum for the local package files. That checksum does not match the checksum recorded in the lockfile.

Save any edits you want to keep elsewhere before restoring these files.

To restore the local package files to the contents identified by the lockfile, run:

  panack deps restore --project '<workspace>/Ian'"'"'s app/panack.json'

Running this command will replace the local copy without changing the lockfile.

Panack will check the restored package checksums against the lockfile before using the packages.

An internet connection is needed if remote packages must be downloaded.
```

## The archive changed after it was locked

Command: `panack deps restore --project "<workspace>/Ian's app/panack.json"`.

The local archive was replaced with different content. The failure preserves the lock, cache state and old cached program. Restoring the exact original archive bytes makes restore and run succeed. An update is a deliberate trust decision, not the default remedy.

```text
panack: error[PKG_INTEGRITY]: Cannot restore dependency 'lib' (package 'demo.library') because its source archive's checksum does not match the lockfile.

  Project: <workspace>/Ian's app/panack.json
  Lockfile: <workspace>/Ian's app/panack.lock.json
  Package: demo.library
  Source archive: <workspace>/library.tar.gz
  Dependency name: lib

Panack tried to restore the local files originally extracted from the source archive, but the archive's checksum does not match the checksum recorded in the lockfile. The archive bytes differ from the bytes used to calculate that checksum. Panack has refused to use that archive.

To restore the recorded dependency, obtain the original archive, put that archive at the source archive path, then run:

  panack deps restore --project '<workspace>/Ian'"'"'s app/panack.json'

If you intend to use the changed archive instead, review its contents before running:

  panack deps update --project '<workspace>/Ian'"'"'s app/panack.json' 'lib'

Running this update will change the lockfile to record the new dependency selection and checksums.
```

## A dependency of a dependency is missing

Command: `panack deps update --project "<workspace>/Ian's app/panack.json"`.

The app depends on outer, whose inner dependency points to a missing directory. Both declaring edges and the missing manifest path are shown.

```text
panack: error[PKG_MANIFEST_MISSING]: Cannot load the package because panack.json is missing.

  Project: <workspace>/Ian's app/panack.json
  Dependency: demo.app -> outer
  Dependency: demo.outer -> inner
  Dependency directory: <workspace>/missing
  Dependency declared in: <workspace>/outer/panack.json
  Dependency name: inner
  Missing file: <workspace>/missing/panack.json

Panack needs panack.json to read the package's name, version and dependencies.

Check the dependency path in the declaring manifest. If the path is wrong, correct the path. If the path is right, restore the missing panack.json or obtain a complete copy of the package.
```

## A manifest is not valid JSON

Command: `panack deps update --project "<workspace>/Ian's app/panack.json"`.

The manifest contains {broken. The parser location and manifest path identify what must be corrected.

```text
panack: error[PKG_MANIFEST_INVALID]: Cannot understand panack.json because it contains invalid JSON.

  Project: <workspace>/Ian's app/panack.json
  File: <workspace>/Ian's app/panack.json
  Location: line 1, column 2

JSON property names must be enclosed in double quotes. Panack expected an opening double quote at this location.

Correct the reported JSON problem, then retry your command.
```

## A lockfile is not valid JSON

Command: `panack deps restore --project "<workspace>/Ian's app/panack.json"`.

The lock contains {broken. Guidance explains the JSON error and favours recovering a valid copy of the existing lockfile; creating a new lockfile may select different dependencies.

```text
panack: error[PKG_LOCK_INVALID]: Cannot understand panack.lock.json because it contains invalid JSON.

  Project: <workspace>/Ian's app/panack.json
  Lockfile: <workspace>/Ian's app/panack.lock.json
  Location: line 1, column 2

JSON property names must be enclosed in double quotes. Panack expected an opening double quote at this location.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## An update names an unknown alias

Command: `panack deps update --project "<workspace>/Ian's app/panack.json" absent`.

The selected alias does not occur in either graph. The first selection error is preserved instead of being overwritten by later consequences.

```text
panack: error[PKG_DEPENDENCY_UNKNOWN]: Cannot update dependency 'absent' because this project has no dependency with that name.

  Project: <workspace>/Ian's app/panack.json
  Lockfile: <workspace>/Ian's app/panack.lock.json

Panack could not find this dependency name in panack.json or in the existing lockfile. Panack has not changed the lockfile.

Check the name in your command. Use the dependency name declared in panack.json, which may differ from the package's own name.
```

## The archive server returns 404

Command: `panack deps update --project <workspace>/app/panack.json`.

The local TLS peer returns 404. The message preserves the status and explains that a downloadable archive is required.

```text
panack: error[PKG_HTTP_STATUS]: Cannot download the archive because the server could not find the requested archive.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/missing.tar.gz
  Dependency name: lib
  Server response: HTTP 404

Check the archive URL in panack.json. If the URL is incorrect, correct the URL and retry your command.

If the URL is correct, ask the package maintainer where the archive is available.
```

## The HTTPS connection fails verification

Command: `panack deps update --project <workspace>/app/panack.json`.

The test removes trust in its private certificate. No HTTP request reaches the peer. The diagnostic does not recommend disabling certificate checks or imply that every TLS failure is an expired certificate.

```text
panack: error[PKG_HTTPS_CONNECTION]: Cannot download the archive because Panack could not establish a secure HTTPS connection to the archive server.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/lib.tar.gz
  Dependency name: lib

The HTTP API has not provided enough detail for Panack to identify the cause.

Check whether the archive URL is accessible over HTTPS from the same environment. If you manage the server, check its HTTPS configuration and certificate; otherwise, report the failure to the server administrator.
```

## A locked Git commit cannot be acquired

Command: `panack deps restore --project <workspace>/app/panack.json`.

The test replaces the remote repository with an empty reachable repository, so its locked commit is unavailable. Git returns 128. That status alone does not prove the cause; the message gives the repository, commit and safe investigation steps without inventing a diagnosis. Returning the original repository makes restore succeed without changing the pin.

```text
panack: error[PKG_GIT_FETCH]: Git command failed with exit status 128; remote error text withheld to protect credentials

  Project: <workspace>/app/panack.json
  Package: git.root
  Repository: https://localhost:<port>/sha.git
  Dependency name: left
  Context: repository https://localhost:<port>/sha.git at commit <locked-commit>

Panack could not prepare the required Git package files. A Git exit status alone does not identify the cause.

Check that the repository is accessible from the environment running Panack and that the required commit is available. Correct the reported problem, then retry your command.
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
panack: error[PKG_DEPENDENCY_NOT_SELECTED]: Cannot complete this update without also changing dependency 'lib'.

  Project: <workspace>/Ian's app/panack.json
  Lockfile: <workspace>/Ian's app/panack.lock.json

Your command did not select this dependency. Panack has stopped because the update would also change this dependency. Panack has not changed the lockfile.

If you intend to update this dependency too, include its declaration name in your update command. Otherwise, undo the change to this dependency in panack.json and retry your original command.
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

The 30 diagnostic unit checks and twelve public transcripts are representative
regression evidence, not a claim that every low-level failure has its own
end-to-end fixture. The existing archive, Git, graph, containment and lock tests
continue to exercise the wider rejection surface. Exact transcript matching
keeps this document current; human review is still needed to judge whether an
explanation is useful.

## Diagnostic implementation follow-up

The current follow-up applies the reviewed presentation to lock/cache recovery,
manifest JSON, selected updates and HTTP failures, including full relevant paths,
checksum terminology and warnings before replacement commands. Internal operation
and phase labels are omitted from the rendered messages. Line and column locations
are supplied by the package tool without modifying the shared compiler parser.

This is an intermediate implementation: the wider reviewed Git/archive cause
catalogue and Git-version detection are not all implemented here. Do not describe
this slice as completion of every agreed diagnostic. P04b/c remain open. HTTP API
error-detail preservation is tracked separately in issue #586.

The change affects failure reporting and recovery guidance, not accepted package
formats or dependency selection. Input failure handling reparses rejected JSON
to distinguish syntax from package settings; normal successful parsing is unchanged.
No compiler, native interface or bootstrap source changes are required.
