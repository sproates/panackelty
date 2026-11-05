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

The [P04a source audit](PACKAGE_DIAGNOSTIC_ACCEPTANCE.md) records the finite
completion checklist and verification boundaries. The catalogue contains 100
actual transcripts; transcript count is evidence, not the acceptance criterion.

## How to reproduce and maintain these examples

Build with `make package-tool`, then run `sh tests/package_diagnostics.sh`,
`sh tests/package_https.sh`, `sh tests/package_git.sh` and
`sh tests/package_archive_diagnostics.sh` and `sh tests/package_validation_diagnostics.sh` and `sh tests/package_lock_diagnostics.sh`. The Git suite includes
controlled executable tests in `tests/package_git_diagnostics.sh`. The remote tests use
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
panack: error[PKG_GIT_FETCH]: Cannot finish the Git operation because Git exited with status 128.

  Project: <workspace>/app/panack.json
  Package: git.root
  Repository: https://localhost:<port>/sha.git
  Dependency name: left
  Requested commit: <locked-commit>
  Git operation: fetching the requested commit
  Exit status: 128

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

The 33 general diagnostic unit checks, 74 Git unit checks, 22 archive/stream unit checks and thirty-four public transcripts are representative
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

Git diagnostics now retain process, reference, inventory and symbolic-link causes,
and check the installed Git version before acquisition. Archive parsing and extraction now preserve typed causes through local and HTTPS acquisition. The general validation catalogue is still outstanding. Do not describe
this slice as completion of every agreed diagnostic. P04b/c remain open. HTTP API
error-detail preservation is tracked separately in issue #586.

The change affects failure reporting and recovery guidance, not accepted package
formats or dependency selection. Input failure handling reparses rejected JSON
to distinguish syntax from package settings; normal successful parsing is unchanged.
No compiler, native interface or bootstrap source changes are required.

## A manifest is not UTF-8 text

Command: `panack deps update --project "<workspace>/Ian's app/panack.json"`.

The manifest contains the invalid UTF-8 byte `0xff`. Panack rejects the file
before parsing JSON, explains how to correct its encoding and leaves the lockfile
unchanged. No line or column is claimed for text Panack cannot decode.

```text
panack: error[PKG_MANIFEST_INVALID]: Cannot understand panack.json because the file is not valid UTF-8 text.

  Project: <workspace>/Ian's app/panack.json
  File: <workspace>/Ian's app/panack.json

Save the file using UTF-8 encoding, or obtain a valid copy, then retry your command.
```

## Installed Git is too old

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

A controlled Git executable reports version 2.28.9. Panack stops before looking up the repository. The failed command leaves no lockfile or published package.

```text
panack: error[PKG_GIT_VERSION]: Cannot obtain the Git dependency because the installed Git version is older than 2.29.

  Project: <workspace>/app/panack.json
  Repository: https://example.test/repo.git
  Dependency name: library
  Git executable: <workspace>/bin/git
  Installed Git version: 2.28.9

Install Git 2.29 or newer, or put a supported Git installation earlier on PATH, then retry your command. Git is needed only when Panack must obtain a Git dependency.
```

## The requested Git reference does not exist

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

A controlled Git executable returns the documented no-matching-reference status (2) from ls-remote. Other command failures do not receive this explanation. The failed command leaves no lockfile or published package.

```text
panack: error[PKG_GIT_REF_NOT_FOUND]: Cannot select the Git dependency because Git did not find the requested branch or tag.

  Project: <workspace>/app/panack.json
  Repository: https://example.test/repo.git
  Dependency name: library
  Requested reference: release

Check the ref field in the dependency's Git declaration. Use an existing branch, tag or complete commit ID, then retry your command.
```

## A repository contains a submodule

Command: `panack deps lock --project "<workspace>/diagnostic-app/panack.json"`.

A real HTTPS Git repository contains a submodule. Panack rejects the repository before publishing a package. The failed command leaves no lockfile or published package.

```text
panack: error[PKG_GIT_SUBMODULE]: Cannot prepare the Git package because the repository contains a submodule.

  Project: <workspace>/diagnostic-app/panack.json
  Repository: https://localhost:<port>/repo.git
  Dependency name: library
  Requested commit: <locked-commit>
  Submodule path: submodule

A submodule refers to another Git repository. Panack does not fetch submodule contents for packages.

If you maintain this package, provide the required files without a submodule or declare the other package as a dependency in panack.json. Otherwise, ask the package maintainer for a compatible package.
```

## A repository contains an LFS pointer

Command: `panack deps lock --project "<workspace>/diagnostic-app/panack.json"`.

A real HTTPS Git repository contains a Git LFS pointer. Panack explains that the referenced contents are not downloaded. The failed command leaves no lockfile or published package.

```text
panack: error[PKG_GIT_LFS]: Cannot prepare the Git package because a file is a Git LFS pointer instead of the required contents.

  Project: <workspace>/diagnostic-app/panack.json
  Repository: https://localhost:<port>/repo.git
  Dependency name: library
  Requested commit: <locked-commit>
  Repository path: lfs

Git LFS stores file contents separately from the Git repository. Panack does not download those contents.

If you maintain this package, provide the required file contents without Git LFS, for example in a complete package archive. Otherwise, ask the package maintainer for a compatible package.
```

## A repository link uses an absolute target

Command: `panack deps lock --project "<workspace>/diagnostic-app/panack.json"`.

A real HTTPS Git repository contains a symbolic link to an absolute path. Panack refuses to publish the package. The failed command leaves no lockfile or published package.

```text
panack: error[PKG_SYMLINK_INVALID]: Cannot prepare the Git package because the target is an absolute path.

  Project: <workspace>/diagnostic-app/panack.json
  Repository: https://localhost:<port>/repo.git
  Dependency name: library
  Requested commit: <locked-commit>
  Symbolic link: escape

If you maintain this package, correct the symbolic link so its relative target stays inside the repository. Otherwise, ask the package maintainer for a corrected package.
```

## A repository link chain exceeds the limit

Command: `panack deps lock --project "<workspace>/diagnostic-app/panack.json"`.

A real HTTPS Git repository contains a link cycle. The diagnostic reports the observed 40-link bound without claiming that every such failure proves a cycle. The failed command leaves no lockfile or published package.

```text
panack: error[PKG_SYMLINK_LIMIT]: Cannot prepare the Git package because resolving a symbolic link exceeded Panack's limit of 40 links.

  Project: <workspace>/diagnostic-app/panack.json
  Repository: https://localhost:<port>/repo.git
  Dependency name: library
  Requested commit: <locked-commit>
  Symbolic link: loop-one

The limit does not establish that the links form a cycle.

If you maintain this package, shorten the link chain and check for links that point back to each other. Otherwise, ask the package maintainer for a corrected package.
```

## Archive parsing and extraction examples

Each case below runs `panack deps lock --project <workspace>/app/panack.json`
against a deliberately constructed local archive. The test asserts status 1, no
lockfile and no abandoned staging files. These are initial acquisition failures;
no existing package is replaced. Reproduce with
`sh tests/package_archive_diagnostics.sh`.

### Archive paths differ only in case

```text
panack: error[PKG_ARCHIVE_PATH_CONFLICT]: Cannot unpack the archive because two archive paths differ only in letter case.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib
  Conflicting archive path: data
  First archive path: Data

Panack cannot create a portable package directory from the reported paths.

If you maintain this package, correct the reported paths and recreate the archive. Otherwise, ask the package maintainer for a corrected archive.
```

### Repeated archive path

```text
panack: error[PKG_ARCHIVE_PATH_CONFLICT]: Cannot unpack the archive because the archive contains the same path more than once.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib
  Archive entry: data

Panack cannot create a portable package directory from the reported paths.

If you maintain this package, correct the reported paths and recreate the archive. Otherwise, ask the package maintainer for a corrected archive.
```

### Invalid gzip data

```text
panack: error[PKG_ARCHIVE_INVALID]: Cannot unpack the archive because the file is not valid gzip data.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib

Panack has stopped extraction.

Obtain a valid copy of the archive and retry your command. If a fresh copy produces the same error, ask the package maintainer to recreate the archive. A replacement must still match the checksums in an existing lockfile.
```

### Corrupt tar header

```text
panack: error[PKG_ARCHIVE_INVALID]: Cannot unpack the archive because a tar header's checksum does not match the header data.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib

Panack has stopped extraction.

Obtain a valid copy of the archive and retry your command. If a fresh copy produces the same error, ask the package maintainer to recreate the archive. A replacement must still match the checksums in an existing lockfile.
```

### Incomplete tar end marker

```text
panack: error[PKG_ARCHIVE_INCOMPLETE]: Cannot unpack the archive because the archive ended before its contents and end marker were complete.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib

Panack has stopped extraction.

Obtain a valid copy of the archive and retry your command. If a fresh copy produces the same error, ask the package maintainer to recreate the archive. A replacement must still match the checksums in an existing lockfile.
```

### Missing top-level manifest

```text
panack: error[PKG_MANIFEST_MISSING]: Cannot use the archive because the archive has no panack.json at its top level.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib

Panack expects panack.json at the archive's top level, alongside the package's source directory.

If you maintain this package, recreate the archive with that layout. Otherwise, ask the package maintainer for a corrected archive.

See the package manifest specification:
https://github.com/sproates/panackelty/blob/main/docs/design/MODULE_PACKAGE_DESIGN.md#package-roots-manifest-and-source-distribution
```

### Archive numeric bound

```text
panack: error[PKG_ARCHIVE_NUMBER_LIMIT]: Cannot unpack the archive because a numeric archive field exceeds Panack's supported maximum of 268435456.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib
  Archive field: entry size

Panack has stopped extraction because the reported archive limit was exceeded.

If you maintain this package, reduce the reported value or count and recreate the archive. Otherwise, ask the package maintainer for an archive within Panack's limits.
```

### Malformed archive number

```text
panack: error[PKG_ARCHIVE_INVALID]: Cannot unpack the archive because an archive number contains invalid digits or spacing.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib
  Archive field: entry size

Panack has stopped extraction.

Obtain a valid copy of the archive and retry your command. If a fresh copy produces the same error, ask the package maintainer to recreate the archive. A replacement must still match the checksums in an existing lockfile.
```

### Parent-directory component

```text
panack: error[PKG_ARCHIVE_PATH_INVALID]: Cannot unpack the archive because the path contains a parent-directory component ('..').

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib
  Archive entry: a/../b

Use paths relative to the package directory without parent-directory components ('..'). Recreate the archive after correcting the path. If you do not maintain this package, ask the package maintainer for a corrected archive.
```

### Repeated PAX field

```text
panack: error[PKG_ARCHIVE_INVALID]: Cannot unpack the archive because PAX metadata repeats a field name.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib
  PAX field: path

PAX metadata describes the file that follows it in the archive.

Obtain a valid copy of the archive and retry your command. If a fresh copy produces the same error, ask the package maintainer to recreate the archive. A replacement must still match the checksums in an existing lockfile.
```

### Unsupported PAX field

```text
panack: error[PKG_ARCHIVE_UNSUPPORTED]: Cannot unpack the archive because the archive uses an unsupported PAX metadata field.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib
  PAX field: unknown

PAX metadata describes the file that follows it in the archive.

If you maintain this package, recreate the archive using the supported format and entries. Otherwise, ask the package maintainer for a compatible archive.
```

### Malformed PAX size

```text
panack: error[PKG_ARCHIVE_INVALID]: Cannot unpack the archive because the PAX size field must contain decimal digits only.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib

PAX metadata describes the file that follows it in the archive.

Obtain a valid copy of the archive and retry your command. If a fresh copy produces the same error, ask the package maintainer to recreate the archive. A replacement must still match the checksums in an existing lockfile.
```

### Data after the archive end marker

```text
panack: error[PKG_ARCHIVE_INVALID]: Cannot unpack the archive because the archive contains unexpected data after its end marker.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib

Panack has stopped extraction.

Obtain a valid copy of the archive and retry your command. If a fresh copy produces the same error, ask the package maintainer to recreate the archive. A replacement must still match the checksums in an existing lockfile.
```

### Unsupported symbolic link entry

```text
panack: error[PKG_ARCHIVE_UNSUPPORTED]: Cannot unpack the archive because the archive contains an unsupported entry type.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib
  Archive entry: link
  Tar entry type: symbolic link

Panack accepts POSIX ustar archives containing ordinary files, directories and supported per-file PAX metadata.

If you maintain this package, recreate the archive using the supported format and entries. Otherwise, ask the package maintainer for a compatible archive.
```

### Invalid UTF-8 filename

```text
panack: error[PKG_ARCHIVE_PATH_INVALID]: Cannot unpack the archive because an archive header's text field is not valid UTF-8 text.

  Project: <workspace>/app/panack.json
  Source archive: <workspace>/source.tar.gz
  Dependency name: lib
  Archive field: filename

Panack cannot create a portable package directory from the reported paths.

If you maintain this package, correct the reported paths and recreate the archive. Otherwise, ask the package maintainer for a corrected archive.
```

## Command, input and graph validation

The following cases are reproduced by `tests/package_validation_diagnostics.sh`,
which also runs through the ordinary package acceptance suite. Invalid commands
are rejected before package files are accessed. The selected-update test executes
the suggested initial-lock command. Filesystem rejection tests preserve the
existing lock or conflicting entry; permission cases run on non-root POSIX hosts.
The publication test deliberately denies directory access. The atomic-write API
reports only `filesystem_error`, so the explanation does not invent a finer cause.

### No dependency operation was supplied

Command: `panack deps`.

```text
panack: error[PKG_USAGE]: Cannot run panack deps because no operation was supplied.

Panack has not read or changed any package files.

Choose lock, update or restore, followed by --project /path/to/panack.json.
```

### The project option has no path

Command: `panack deps lock --project`.

```text
panack: error[PKG_USAGE]: Cannot run the package command because --project has no manifest path.

Panack has not read or changed any package files.

Supply the path to the project's panack.json immediately after --project.
```

### Offline was requested for an update

Command: `panack deps update --project "<workspace>/app/panack.json" --offline`.

```text
panack: error[PKG_USAGE]: Cannot run this command because --offline is only supported by panack deps restore.

  Project: <workspace>/app/panack.json

Panack has not read or changed any package files.

Remove --offline to run this operation. To restore recorded packages without network access, use panack deps restore --project /path/to/panack.json --offline.
```

### A dependency name was passed to restore

Command: `panack deps restore --project "<workspace>/app/panack.json" lib`.

```text
panack: error[PKG_USAGE]: Cannot run this command because dependency names are only accepted by panack deps update.

  Project: <workspace>/app/panack.json
  Argument: lib

Panack has not read or changed any package files.

Remove the extra argument. To update selected dependencies, use panack deps update --project /path/to/panack.json followed by their names from the manifest.
```

### An unsupported package option

Command: `panack deps lock --project "<workspace>/app/panack.json" --unknown`.

```text
panack: error[PKG_USAGE]: Cannot run the package command because an option is not supported.

  Project: <workspace>/app/panack.json
  Option: --unknown

Panack has not read or changed any package files.

Remove the unsupported option. Only panack deps restore accepts --offline; panack deps update accepts dependency names after the manifest path.
```

### A selected update has no baseline lock

Command: `panack deps update --project "<workspace>/app/panack.json" lib`.

```text
panack: error[PKG_LOCK_REQUIRED]: Cannot update selected dependencies because this project has no lockfile.

  Project: <workspace>/app/panack.json
  Missing file: <workspace>/app/panack.lock.json

A selected update needs an existing lockfile so Panack can preserve the dependencies you did not select. Panack has not changed any package files.

Restore an existing lockfile for this project, or create an initial lockfile for all dependencies by running:

  panack deps lock --project '<workspace>/app/panack.json'
```

### The selected manifest has the wrong filename

Command: `panack deps lock --project "<workspace>/app/good.json"`.

```text
panack: error[PKG_MANIFEST_INVALID]: Cannot use the selected manifest because its filename is not panack.json.

  Project: <workspace>/app/good.json
  Selected file: <workspace>/app/good.json

Panack requires the package manifest to be named panack.json in the package's root directory.

Pass the path to the package's panack.json using --project. If this is the intended manifest, rename the file to panack.json.
```

### The manifest path identifies a directory

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_FILESYSTEM]: Cannot read panack.json: the path does not identify a regular file (not_regular_file).

  Project: <workspace>/app/panack.json
  File: <workspace>/app/panack.json

Correct the reported filesystem problem, then retry your command. Check access using the account running Panack.
```

### The manifest exceeds the byte limit

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_MANIFEST_INVALID]: Cannot process panack.json because the file exceeds Panack's 64 KiB (65536 bytes) size limit.

  Project: <workspace>/app/panack.json
  File: <workspace>/app/panack.json

Panack stopped reading the file at the size limit. This error does not establish whether the file contains valid JSON.

Reduce the file's size without removing required information, then retry your command. If the project needs a larger file, report this limit with your Panack version.
```

### JSON exceeds the nesting limit

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_MANIFEST_INVALID]: Cannot process panack.json because its JSON exceeds Panack's 32-level nesting limit.

  Project: <workspace>/app/panack.json
  File: <workspace>/app/panack.json
  Location: line 1, column 34

manifest JSON nesting exceeds 32 levels

Reduce the JSON nesting to at most 32 levels without removing required information, then retry your command.
```

### The lockfile path identifies a directory

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because the path identifies a directory.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

Panack requires the lockfile itself to be a regular file.

Preserve any contents you need before moving the conflicting entry elsewhere. Then restore a regular lockfile for this project at the lockfile path.
```

### The lockfile path identifies a symbolic link

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because the path identifies a symbolic link.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

Panack requires the lockfile itself to be a regular file.

Preserve any contents you need before moving the conflicting entry elsewhere. Then restore a regular lockfile for this project at the lockfile path.
```

### The lockfile exceeds the byte limit

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_LOCK_INVALID]: Cannot process panack.lock.json because the file exceeds Panack's 1 MiB (1048576 bytes) size limit.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json
  File: <workspace>/app/panack.lock.json

Panack stopped reading the file at the size limit. This error does not establish whether the file contains valid JSON.

Reduce the file's size without removing required information, then retry your command. If the project needs a larger file, report this limit with your Panack version.
```

### Valid JSON contains invalid lock records

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

The lockfile is missing the required 'lock_schema' field.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

### An application was selected as a dependency

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_MANIFEST_INVALID]: Cannot use dependency 'lib' because package 'demo.application' is an application.

  Project: <workspace>/app/panack.json
  Dependency directory: <workspace>/application
  Dependency name: lib
  Dependency manifest: <workspace>/application/panack.json

The package manifest declares an 'entry' file. Panack only accepts library packages as dependencies.

Choose a library package for this dependency. If you maintain this package and intend to provide a library, remove 'entry' and declare the public modules in 'exports'.

See the package manifest specification:
https://github.com/sproates/panackelty/blob/main/docs/design/MODULE_PACKAGE_DESIGN.md#package-roots-manifest-and-source-distribution
```

### Two directories declare the same package identity

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_IDENTITY_CONFLICT]: Cannot resolve the dependencies because package demo.library@1.0.0 was found in two different directories.

  Project: <workspace>/app/panack.json
  Dependency directory: <workspace>/other
  Dependency name: second
  Conflicting package directory: <workspace>/other
  First package directory: <workspace>/library

These directories declare the same package name and version, but refer to separate physical package directories. Panack cannot choose which copy to use.

Correct the dependency paths so all references to this package use the same directory, then retry your command.
```

### Two versions of the same package

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_VERSION_CONFLICT]: Cannot resolve the dependencies because package demo.library is declared with conflicting versions.

  Project: <workspace>/app/panack.json
  Dependency directory: <workspace>/other
  Dependency name: second
  Conflicting package: demo.library@2.0.0 in <workspace>/other
  First package: demo.library@1.0.0 in <workspace>/library

Panack requires one version of each package name throughout the dependency graph. The two package declarations name different versions.

Check the dependency paths and versions in the reported package manifests. Choose compatible dependencies that use the same version of this package, then retry your command.
```

### A dependency returns to the project package

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_DEPENDENCY_CYCLE]: Cannot resolve dependency 'self' because the dependency chain returns to the project package.

  Project: <workspace>/app/panack.json
  Dependency directory: <workspace>/app
  Dependency name: self
  Repeated package directory: <workspace>/app

Following these dependency declarations leads back to a package already being resolved. Panack does not allow circular dependencies.

Check the dependency chain and the dependencies sections of those packages' manifests. Remove or redirect the declaration that creates the cycle, then retry your command.
```

### A dependency returns to another package in the chain

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_DEPENDENCY_CYCLE]: Cannot resolve dependency 'self' because the dependency chain returns to a package already in this chain.

  Project: <workspace>/app/panack.json
  Dependency: demo.app -> lib
  Dependency: demo.library -> self
  Dependency directory: <workspace>/library
  Dependency declared in: <workspace>/library/panack.json
  Dependency name: self
  Repeated package directory: <workspace>/library

Following these dependency declarations leads back to a package already being resolved. Panack does not allow circular dependencies.

Check the dependency chain and the dependencies sections of those packages' manifests. Remove or redirect the declaration that creates the cycle, then retry your command.
```

### Two packages share a source directory through a symbolic link

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_LAYOUT]: Cannot use dependency lib because its source directory overlaps the project's source directory.

  Project: <workspace>/app/panack.json
  Dependency directory: <workspace>/library
  Dependency name: lib
  Conflicting source directory: <workspace>/library/src
  First source directory: <workspace>/app/src

These directories overlap, so the same source files could belong to both packages. Panack requires separate source directories for each package.

Check the source field in each panack.json. Correct the paths or move the source files so neither source directory contains the other.

See the package manifest specification:
https://github.com/sproates/panackelty/blob/main/docs/design/MODULE_PACKAGE_DESIGN.md#package-roots-manifest-and-source-distribution
```

### The account cannot read the manifest

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_FILESYSTEM]: Cannot read panack.json: the account running Panack does not have permission (permission_denied).

  Project: <workspace>/app/panack.json
  File: <workspace>/app/panack.json

Correct the reported filesystem problem, then retry your command. Check access using the account running Panack.
```

### The account cannot read the lockfile

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_FILESYSTEM]: Cannot read panack.lock.json: the account running Panack does not have permission (permission_denied).

  Project: <workspace>/app/panack.json
  File: <workspace>/app/panack.lock.json

Correct the reported filesystem problem, then retry your command. Check access using the account running Panack.
```

### The initial lockfile cannot be saved

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

```text
panack: error[PKG_PUBLICATION]: Cannot save panack.lock.json: the filesystem operation failed; no more specific cause was reported (filesystem_error).

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json
  Destination file: <workspace>/app/panack.lock.json

Panack stopped before publishing the prepared packages.

Correct the reported filesystem problem at the destination path, then retry your command. Check access using the account running Panack.
```

## Unsupported lockfile schema

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The lockfile declares schema 2; this tool supports schema 1. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

The 'lock_schema' field must be the integer 1. This Panack version supports lockfile schema 1.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Invalid manifest checksum in a lockfile

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The recorded manifest checksum contains three characters instead of a SHA-256 checksum. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

The 'manifest_sha256' field must contain a SHA-256 checksum written as 64 lowercase hexadecimal characters.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Duplicate package record

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The same package name and version appears twice. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

The lockfile contains more than one record for package 'demo.library@1.0.0'.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Multiple versions of one package

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

Two records declare the same package name at different versions. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

The lockfile records multiple versions of package 'demo.library'. Panack requires one version of each package name.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Unreachable package record

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The package record is present, but the project no longer references that package. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

The lockfile contains a record for package 'demo.library@1.0.0', but no dependency chain from the project reaches that package.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Missing package record

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

A dependency points to a package that has no record in the lockfile. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

A dependency refers to package 'demo.library@1.0.0', but the lockfile has no record for that package.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Checksum on a live directory package

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The lockfile incorrectly pins a package-content checksum for a live local directory. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

Package 'demo.library@1.0.0' is a live local directory, but its lockfile record contains 'tree_sha256'. Live directory contents are not pinned to a package checksum.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Missing package-content checksum

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

An archive dependency has no checksum for its extracted package files. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

Package 'demo.library@1.0.0' is missing 'tree_sha256', the checksum used to verify the package files obtained from its archive or Git repository.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Unsupported dependency source in a lockfile

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The source kind is set to 'registry', which this package model does not support. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json
  Location: line 1, column 248

A dependency source has an unsupported 'kind'. Expected 'path', 'archive' or 'git'.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Two locations for one archive source

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The archive source contains both a local path and an HTTPS URL. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json
  Location: line 1, column 281

An archive source specifies both 'path' and 'url'. Exactly one archive location is allowed.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Incomplete Git commit in a lockfile

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The Git source records an abbreviated commit ID instead of a complete ID. The test verifies the rejected lockfile remains byte-for-byte unchanged.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json
  Location: line 1, column 281

A Git source's 'commit' must contain the complete commit ID: 40 lowercase hexadecimal characters for sha1, or 64 for sha256.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Project manifest changed after locking

Command: `panack check --project "<workspace>/app/panack.json"`.

Whitespace is appended to the project manifest. The test verifies rejection preserves the lock, then explicitly updates and successfully checks the project.

```text
panack: error[PKG_LOCK_STALE]: Cannot use the lockfile because the project manifest has changed since the lockfile was created.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

The package definitions no longer match the lockfile. Panack checks the complete manifest files, so formatting changes also require the lockfile to be updated.

If the change was unintended, restore the previous package definition. Otherwise, review the change before running panack deps update. Updating the lockfile may select different package versions or contents.
```

## Dependency manifest changed after locking

Command: `panack check --project "<workspace>/app/panack.json"`.

Whitespace is appended to the local library manifest. The test preserves the old lock until an explicit selected update, then successfully checks the project.

```text
panack: error[PKG_LOCK_STALE]: Cannot use the lockfile because a dependency manifest has changed since the lockfile was created.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json
  Manifest: <workspace>/library/panack.json
  Package: demo.library@1.0.0

The package definitions no longer match the lockfile. Panack checks the complete manifest files, so formatting changes also require the lockfile to be updated.

If the change was unintended, restore the previous package definition. Otherwise, review the change before running panack deps update. Updating the lockfile may select different package versions or contents.
```

## Conflicting package destination

Command: `panack deps restore --project "<workspace>/Ian's app/panack.json"`.

A symbolic link occupies the package destination. Panack preserves the link, its target, the lockfile and the previous state record. The test removes the link, executes the suggested restore command and runs the program successfully.

```text
panack: error[PKG_PUBLICATION]: Cannot save the prepared package directory: the destination is not a replaceable package directory (destination_conflict).

  Project: <workspace>/Ian's app/panack.json
  Lockfile: <workspace>/Ian's app/panack.lock.json
  Package: demo.library@1.0.0
  Destination directory: <workspace>/Ian's app/.panack_modules/demo.library

Panack stopped while saving prepared packages. The lockfile may already have been saved, and other package directories may already have been replaced.

Preserve any contents you need before moving the conflicting destination elsewhere. Then prepare the packages recorded in the lockfile by running:

  panack deps restore --project '<workspace>/Ian'"'"'s app/panack.json'

Running this command will prepare the local packages without changing the lockfile. An internet connection is needed if remote packages must be downloaded.
```

## Conflicting package state destination

Command: `panack deps restore --project "<workspace>/Ian's app/panack.json"`.

A directory occupies the state-record file path. Package directories have been saved, but the state record cannot be written. The test preserves the lockfile, removes the empty conflicting directory, executes the suggested restore command and runs the program successfully.

```text
panack: error[PKG_PUBLICATION]: Cannot save the package state record because the destination is not a regular file.

  Project: <workspace>/Ian's app/panack.json
  Lockfile: <workspace>/Ian's app/panack.lock.json
  Destination file: <workspace>/Ian's app/.panack_modules/.state/lock.json

Panack saved the package directories, but could not save the state record used to verify those directories against the lockfile.

Preserve any contents you need before moving the conflicting destination elsewhere. Then prepare the packages recorded in the lockfile by running:

  panack deps restore --project '<workspace>/Ian'"'"'s app/panack.json'

Running this command will prepare the local packages without changing the lockfile. An internet connection is needed if remote packages must be downloaded.
```

## Unsupported lockfile field

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

An extra field named 'unexpected' is present in the lockfile. The test verifies rejection preserves the supplied lockfile.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json
  Location: line 1, column 436

The lockfile record contains an unsupported field: 'unexpected'.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Wrong JSON type in a lockfile

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The project's name is a number instead of a string. The test verifies rejection preserves the supplied lockfile.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json
  Location: line 1, column 370

The lockfile field 'name' must be a JSON string.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## Wrong JSON type for a lockfile record

Command: `panack deps restore --project "<workspace>/app/panack.json"`.

The project's record is an array instead of an object. The test verifies rejection preserves the supplied lockfile.

```text
panack: error[PKG_LOCK_INVALID]: Cannot use panack.lock.json because its dependency records are invalid.

  Project: <workspace>/app/panack.json
  Lockfile: <workspace>/app/panack.lock.json

A lockfile record must be a JSON object.

If you edited this file, correct the reported problem or undo the edit. Otherwise, obtain a valid copy of this project's lockfile.

Creating a new lockfile may select different dependencies.
```

## A file blocks a required package directory

Command: `panack run --project "<workspace>/Ian's app/panack.json"`.

The package state directory is replaced with a regular file. Panack preserves the file and the lock. The test moves the conflict away, restores the directory and runs the program successfully.

```text
panack: error[PKG_FILESYSTEM]: Cannot prepare the local packages because a required directory path identifies a different kind of file.

  Project: <workspace>/Ian's app/panack.json
  Package state file: <workspace>/Ian's app/.panack_modules/.state/lock.json
  Required directory: <workspace>/Ian's app/.panack_modules/.state

Panack has not replaced the conflicting path.

Preserve any contents you need before moving the conflicting path elsewhere, then retry your command.
```

## A redirect destination contains a backslash

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/backslash.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the redirect destination contains a backslash.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/backslash.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## An HTTP header is malformed

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/bad-header.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_PROTOCOL]: Cannot finish downloading the archive because the server's HTTP response is incorrectly formatted.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/bad-header.tar.gz
  Dependency name: lib
  Response detail: HTTP header is missing a colon

Panack cannot safely determine the complete archive contents from this response.

Retry your command. If the same error continues, report the archive URL and response detail to the server administrator.
```

## A redirect would use unencrypted HTTP

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/downgrade.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the server redirected the download to an unencrypted HTTP address.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/downgrade.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## A redirect has an empty destination

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/empty-location.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the server sent an empty redirect destination (Location header).

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/empty-location.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## The server applies an additional content encoding

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/encoded.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP]: Cannot use the download because the server applied an additional content encoding.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/encoded.tar.gz
  Dependency name: lib

Panack needs the original .tar.gz bytes to verify the archive checksum. HTTP Content-Encoding would change the representation being checked.

Ask the server administrator to serve the .tar.gz file directly, without HTTP Content-Encoding, or with Content-Encoding: identity. Then retry your command.
```

## A redirect destination contains a fragment

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/fragment.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the redirect destination contains a fragment, which Panack does not allow in package URLs.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/fragment.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## The server exceeds the redirect limit

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/loop.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the server exceeded Panack's limit of five redirects.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/loop.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## A redirect has no destination

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/no-location.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the server sent a redirect without a destination (Location header).

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/no-location.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## The download exceeds the archive size limit

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/oversize.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_LIMIT]: Cannot finish downloading the archive because the archive exceeds Panack's 64 MiB download limit.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/oversize.tar.gz
  Dependency name: lib
  Response detail: response body

Panack stopped the download at the response limit.

Ask the package maintainer for an archive within this limit.
```

## A redirect destination contains a query

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/query.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the redirect destination contains a query, which Panack does not allow in package URLs.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/query.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## A redirect uses an unsupported URL scheme

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/scheme.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the redirect destination uses an unsupported URL scheme; Panack requires HTTPS.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/scheme.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## A redirect destination contains forbidden credential syntax

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/secret.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the redirect destination contains an '@' character, which Panack forbids to exclude credentials.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/secret.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## A redirect destination contains a space

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/space.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the redirect destination contains a space.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/space.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## The server returns no archive

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/status-204.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_STATUS]: Cannot download the archive because the server returned HTTP 204 instead of the archive.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/status-204.tar.gz
  Dependency name: lib
  Server response: HTTP 204

Check that the URL provides a direct archive download. If the URL is correct, report the archive URL and HTTP status to the server administrator.
```

## The archive requires authentication

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/status-401.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_STATUS]: Cannot download the archive because the server requires authentication.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/status-401.tar.gz
  Dependency name: lib
  Server response: HTTP 401

Panack requires an archive URL that can be downloaded without signing in. Ask the package maintainer for a suitable download URL.
```

## The server refuses access

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/status-403.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_STATUS]: Cannot download the archive because the server refused access.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/status-403.tar.gz
  Dependency name: lib
  Server response: HTTP 403

Check the archive URL. If the URL is correct, ask the server administrator why access was refused. Panack requires an archive URL that can be downloaded without signing in.
```

## The proxy requires authentication

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/status-407.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_STATUS]: Cannot download the archive because the proxy requires authentication.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/status-407.tar.gz
  Dependency name: lib
  Server response: HTTP 407

Check the proxy access requirements in the environment running Panack. Ask the network administrator how to make this download accessible.
```

## The server limits requests

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/status-429.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_STATUS]: Cannot download the archive because the server is limiting requests.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/status-429.tar.gz
  Dependency name: lib
  Server response: HTTP 429

Wait before retrying. If the server continues to reject the request, contact the server administrator.
```

## The server reports an error

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/status-500.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_STATUS]: Cannot download the archive because the server reported an error.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/status-500.tar.gz
  Dependency name: lib
  Server response: HTTP 500

Retry later. If the error continues, report the archive URL and HTTP status to the server administrator.
```

## The server uses unsupported transfer encoding

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/transfer.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_PROTOCOL]: Cannot download the archive because the server response uses an unsupported transfer encoding.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/transfer.tar.gz
  Dependency name: lib

Ask the server administrator to send the archive using either one Transfer-Encoding: chunked header or a Content-Length header without Transfer-Encoding. Then retry your command.
```

## The HTTP response ends early

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/truncated.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_HTTP_PROTOCOL]: Cannot finish downloading the archive because the HTTP response ended before Panack received all the required data.

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/truncated.tar.gz
  Dependency name: lib
  Response detail: truncated HTTP response body

Panack cannot safely determine the complete archive contents from this response.

Retry your command. If the same error continues, report the archive URL and response detail to the server administrator.
```

## A redirect has multiple destinations

Command: `panack deps update --project "<workspace>/app/panack.json"`.

The local TLS peer serves `/two-locations.tar.gz` with this response. The failed update preserves the existing lockfile and local package contents. Rejected redirect destinations are never requested or printed.

```text
panack: error[PKG_REDIRECT]: Cannot download the archive because the server sent more than one redirect destination (Location header).

  Project: <workspace>/app/panack.json
  Package: test.archive
  Source archive: https://localhost:<port>/two-locations.tar.gz
  Dependency name: lib

Panack stopped before following this redirect. The rejected destination is omitted because server-provided URLs can contain secrets.

Ask the server administrator to correct the redirect or the package maintainer to provide a direct HTTPS archive URL. Retry your command once the download is available.
```

## An unknown dependency field

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

A dependency declaration contains an unsupported field. The location identifies that field in the declaring manifest.

```text
panack: error[PKG_MANIFEST_INVALID]: Cannot use panack.json because its package settings are invalid.

  Project: <workspace>/app/panack.json
  File: <workspace>/app/panack.json
  Location: line 1, column 194

Unknown manifest field: 'typo'.

Correct the reported setting, then retry your command.

See the package manifest specification:
https://github.com/sproates/panackelty/blob/main/docs/design/MODULE_PACKAGE_DESIGN.md#package-roots-manifest-and-source-distribution
```

## An unknown manifest field

Command: `panack deps lock --project "<workspace>/app/panack.json"`.

The manifest contains an unsupported top-level field. The message names that field rather than printing an interpolation expression.

```text
panack: error[PKG_MANIFEST_INVALID]: Cannot use panack.json because its package settings are invalid.

  Project: <workspace>/app/panack.json
  File: <workspace>/app/panack.json
  Location: line 1, column 169

Unknown manifest field: 'unrecognised'.

Correct the reported setting, then retry your command.

See the package manifest specification:
https://github.com/sproates/panackelty/blob/main/docs/design/MODULE_PACKAGE_DESIGN.md#package-roots-manifest-and-source-distribution
```
