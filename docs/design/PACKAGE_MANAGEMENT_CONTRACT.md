# Package management contract

This is the Alpha.15 implementation contract for [RM#44: Dependency
management](../roadmap/records/dependency-management--idea.md#rm-44) and
[GI#200](https://github.com/sproates/panackelty/issues/200). It completes
[P01: Lock and restore contract](../roadmap/records/dependency-management--idea.md#p01)
and guides P02–P04. The manifest remains strict JSON `panack.json`, schema 1.
The lockfile is strict JSON `panack.lock.json`, schema 1.

The `panack` command layer resolves, obtains and prepares dependencies. The
compiler receives resolved source paths and performs no network access. Restore
does not execute package code. Registries, package publication, compiled package
artifacts, build scripts and native library management are out of scope.

## Manifest sources

`dependencies` maps a package-local import alias to exactly one source object.
Aliases are single identifiers; `project` and `stdlib` are reserved. Relative
paths resolve against the directory containing the declaring package's
manifest, including for transitive dependencies. A package's own lockfile is
used when it is the selected project. When it is a dependency, the root
project's lockfile controls the complete graph.

```json
{
  "schema": 1,
  "package": "example.app",
  "version": "1.0.0",
  "language": "modules-v1",
  "toolchain": "0.1.0-alpha.15",
  "source": "src",
  "entry": "main.panack",
  "exports": {},
  "dependencies": {
    "geometry": {"path": "../geometry"},
    "text_archive": {"archive": "vendor/text-1.0.0.tar.gz"},
    "http_archive": {"archive": "https://packages.example.test/http-2.0.0.tar.gz"},
    "json": {
      "git": {
        "url": "https://github.com/example/json.git",
        "ref": "v2.1.0",
        "subdir": "packages/json"
      }
    }
  }
}
```

The supported source forms are:

- `{"path":"../library"}`: a local directory containing `panack.json`; used
  in place and not copied into `.panack_modules`.
- `{"archive":"vendor/library.tar.gz"}`: a local gzip-compressed tar archive.
- `{"archive":"https://host/path/library.tar.gz"}`: a remote archive, fetched
  only over HTTPS.
- `{"git":{"url":"https://host/repository.git","ref":"v1.2.0",
  "subdir":"packages/library"}}`: a Git repository and optional package
  subdirectory. `ref` and `subdir` are optional; omitted `ref` means the remote
  default-branch tip at explicit lock/update time, and omitted `subdir` means `.`.

Local and remote archives are materialized under
`.panack_modules/<package-name>`. Git snapshots use the shared source-keyed
location defined below. The declared package name is unique within a resolved
graph, so transitive packages with the same local alias cannot collide in the
archive cache. Source objects have closed schemas; unknown
keys, multiple source kinds, empty paths, URL credentials, non-HTTPS remote URLs
and reserved aliases are errors. Archive paths must end in `.tar.gz`; URL query
strings and fragments are not allowed. Bytes must also decode as gzip containing
a valid tar archive. The suffix is not treated as proof of format. Content-Type
headers are ignored. `.tgz`, zip and plain tar are unsupported.

Git `ref` may be a branch, tag or full commit object ID. Short object IDs are
rejected. Branches and tags are requests resolved only when explicitly locking
or updating; the lock stores the full resulting commit and object format.
Ordinary locked commands never resolve refs, so moved refs have no effect until
`panack deps update`. Submodules, Git LFS, hooks and repository-provided commands
are not run. A `subdir` is a normalized repository-relative path that must remain
within the selected checkout after physical resolution. Git acquisition keeps
one complete locked repository snapshot at
`.panack_modules/.sources/<source-key>`, where `source-key` is derived from the
canonical HTTPS URL, object format and full commit ID. Compute it as lowercase
hex SHA-256 over `PANACK-GIT-SOURCE\0v1\0`, followed by those three UTF-8 fields
in that order, each encoded as an unsigned 64-bit big-endian byte length and
then its bytes. Every package edge with that Git identity shares this snapshot;
its package root is the snapshot plus `subdir` (or the snapshot root when
omitted). Thus the same sibling package reached through multiple parents has
one canonical physical root.
Relative path dependencies declared by a Git package resolve from that
package's directory and may traverse within the shared snapshot to locate a
sibling package. When they resolve to another package inside the same locked
repository snapshot, the edge is recorded as a Git source with the same URL,
object format and full commit, and the target repository-relative path as its
`subdir`. Resolution outside the snapshot is rejected; it cannot read an
arbitrary path on the restoring machine. This also applies transitively. A
diamond graph in which two packages depend on the same sibling must resolve
both edges to the same package record and physical root.

Local directory paths are live by design: their normalized declared path,
package name/version and manifest digest are locked, while source-file edits are
used directly and do not require a lock update. Relative paths remain usable
when the project tree moves together. Absolute paths are recorded as absolute
and are machine-specific. The lock does not replace declared paths with
machine-specific canonical paths. Physical root checks still apply at use time.

## Lock graph and source identity

`panack.lock.json` sits beside the selected `panack.json`. It records the entire
resolved graph. The writer sorts package records by package identity and map
keys by UTF-8 byte order. The reader rejects duplicate JSON keys, duplicate
package identities, unknown keys, missing nodes and unreferenced nodes. The
lock schema version is independent of the manifest schema.

The top-level lock contains `lock_schema`, the root package name/version,
SHA-256 of the root manifest's exact bytes, the exact toolchain version, the
root alias-to-edge map, and one record per unique dependency package. Each edge
contains its target package ID and the resolved source identity declared by its
owning manifest. Keeping source identity on the edge makes relative paths
unambiguous for transitive dependencies. Each package record contains its
name/version, SHA-256 of its manifest's exact bytes, an optional
materialized-tree digest, and its own alias-to-edge map. A package ID is
`name@version`. A graph may contain only one version and one physical root for
a package name; repeated aliases to the same physical package are allowed.
Conflicts fail with both dependency paths. Resolution order never picks a
winner.

Manifest paths use `/` separators on every platform. Normalize lexically by
removing `.` components and collapsing an ordinary component followed by `..`;
preserve leading `..` on relative dependency paths because they can name sibling
packages. Reject NULs and backslashes. Absolute paths retain their platform root
and are machine-specific.

| Source | Locked identity |
| --- | --- |
| Local directory edge | Normalized declared path relative to the owning manifest (or absolute path), target package name/version and manifest digest |
| Local archive edge | Normalized declared path relative to the owning manifest (or absolute path) and SHA-256 of exact archive bytes; package record carries name/version, manifest digest and materialized-tree digest |
| Remote archive edge | HTTPS URL and SHA-256 of exact downloaded archive bytes; package record carries name/version, manifest digest and materialized-tree digest |
| Git edge | HTTPS URL, resolved repository-relative subdirectory, object format and full commit ID; package record carries name/version, manifest digest and repository-snapshot digest |

Archive digest pins the compressed input, including local archives. The Git
object ID pins the repository revision; the separate tree digest verifies the
repository snapshot and cache. Declared paths and URLs remain source identities;
redirect targets, temporary checkout paths and absolute physical paths do not
replace them in the lock.

The materialized-tree digest is SHA-256 over a canonical source snapshot. For a
Git edge, the snapshot is the entire repository tree at the locked commit,
excluding Git's administrative `.git` metadata; the selected package root is
still identified by `subdir`. For an archive edge, it is every entry in the
archive after validation and before extraction, with archive root as package
root. This makes sibling Git packages part of the verified snapshot. Begin with
`PANACK-PACKAGE-TREE\0v1\0`. For each entry in normalized, slash-separated
UTF-8 relative-path byte order, emit a type byte (`D` directory, `F` regular
file, `L` symlink), unsigned 64-bit big-endian path length, path bytes, unsigned
64-bit big-endian content length, then content bytes (for symlinks, link-target
bytes). Explicit directories and parent directories implied by entries are
included once, including empty directories. Owner, timestamp and permission
bits are excluded. Normalized duplicate and case-colliding paths are rejected.
Archives may contain only directories and regular files. Git administrative
metadata, untracked files and submodule contents are never included; submodules
are rejected. Git symlinks are included in the digest. Acquisition validates
that repository paths stay within the snapshot, while package source loading
and compilation separately require every followed symlink to resolve within
that package's own root. A package cannot read a sibling package's private
files through a symlink; sibling access requires a declared dependency edge.

The example uses synthetic digests and a synthetic Git commit. It documents the
shape, not usable source content.

```json
{
  "lock_schema": 1,
  "root": {
    "name": "example.app",
    "version": "1.0.0",
    "manifest_sha256": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    "toolchain": "0.1.0-alpha.15",
    "dependencies": {
      "geometry": {
        "package": "example.geometry@1.0.0",
        "source": {"kind": "path", "path": "../geometry"}
      },
      "text_archive": {
        "package": "example.text@1.0.0",
        "source": {
          "kind": "archive",
          "path": "vendor/text-1.0.0.tar.gz",
          "archive_sha256": "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
        }
      },
      "http_archive": {
        "package": "example.http@2.0.0",
        "source": {
          "kind": "archive",
          "url": "https://packages.example.test/http-2.0.0.tar.gz",
          "archive_sha256": "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
        }
      },
      "json": {
        "package": "example.json@2.1.0",
        "source": {
          "kind": "git",
          "url": "https://github.com/example/json.git",
          "subdir": "packages/json",
          "object_format": "sha1",
          "commit": "0123456789012345678901234567890123456789"
        }
      }
    }
  },
  "packages": [
    {
      "name": "example.geometry", "version": "1.0.0",
      "manifest_sha256": "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
      "dependencies": {}
    },
    {
      "name": "example.text", "version": "1.0.0",
      "manifest_sha256": "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",
      "tree_sha256": "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd",
      "dependencies": {}
    },
    {
      "name": "example.http", "version": "2.0.0",
      "manifest_sha256": "2222222222222222222222222222222222222222222222222222222222222222",
      "tree_sha256": "1111111111111111111111111111111111111111111111111111111111111111",
      "dependencies": {}
    },
    {
      "name": "example.json", "version": "2.1.0",
      "manifest_sha256": "4444444444444444444444444444444444444444444444444444444444444444",
      "tree_sha256": "3333333333333333333333333333333333333333333333333333333333333333",
      "dependencies": {}
    }
  ]
}
```

## Lock lifecycle

- `panack deps lock --project panack.json` creates a missing lock by resolving
  all selectors and the complete graph. If a lock exists and is current, it is a
  no-op. It refuses to silently replace a stale lock.
- `panack deps update --project panack.json [ALIAS...]` explicitly changes
  resolutions. With no alias it re-resolves the graph; with an alias it updates
  that dependency and the subgraph required by it. The lock is written atomically
  only after all dependencies resolve and validate.
- `panack deps restore --project panack.json` reads the current lock and
  materializes locked local archives, remote archives and Git sources. It never
  selects newer refs or rewrites the lock. It reuses only caches whose lock
  identity, source digest and materialized-tree digest match.
- Project `check`, `compile` and `run` require a current lock when dependencies
  are declared. They use local directories in place and verified materialized
  dependencies from `.panack_modules`. They never access the network, materialize
  archives or rewrite the lock. Missing materialization points to
  `panack deps restore`. A manifest with no dependencies needs no empty lock.

Changing a dependency, selector, package identity, toolchain version or
transitive dependency manifest makes the lock stale. Project commands fail
closed and point to `panack deps update`; they do not infer whether a resolution
change was intended. Edits to files under a local-directory dependency are live
and do not stale the lock. Changed local-archive bytes fail the locked digest
check and require an explicit update. Remote archive bytes that differ from the
lock fail restore; deliberate update records the new digest. Git restore fetches
the recorded full commit, not the current branch or tag target. If the locked
commit is unavailable, restore fails with the alias and URL and leaves the lock
unchanged.

`--offline` on `deps restore` prohibits network access. It may verify and extract
a locked local archive directly from its declared path, even when no cache entry
exists yet, then install the verified result atomically. Local directory
dependencies remain live in place. Locked remote archives and Git snapshots
must already be present and verified in the local cache; offline restore never
uses a network response, unverified directory or different Git commit. Without
`--offline`, restore accesses only locked HTTPS sources and the recorded Git
commit. Project `check`, `compile` and `run` are always network-free.

## Cache and acquisition safety

`.panack_modules/<package-name>` is the materialization path for local and
remote archives. Git repository snapshots instead use the shared
`.panack_modules/.sources/<source-key>` path defined above, and package roots
resolve directly to their subdirectories in that snapshot. Local directories
remain at their declared paths. Cache metadata is stored outside package
sources under `.panack_modules/.state/` and binds a canonical source identity
and tree digest to each package identity. Symlinked cache or state paths, or
paths escaping `.panack_modules`, are rejected. A missing cache is built in a
temporary sibling and atomically installed only after source, manifest and tree
verification. Mismatched or corrupt entries are never used; restore replaces
the generated entry atomically after successful reacquisition. Failure leaves
the previous cache untouched, and the compiler cannot see partial output.

Archive acquisition is bounded to 64 MiB compressed input, 256 MiB expanded
content and 16,384 total file/directory entries per archive. Exceeding a bound
fails before installing a cache entry. Extraction rejects absolute paths,
parent traversal, NULs, duplicate normalized paths, case collisions, hard links,
symbolic links and special files. Every path is validated before writing and
stays beneath a fresh staging directory. The archive root must contain
`panack.json`; no top-level directory is silently stripped. Existing package
manifest and source-closure limits still apply after extraction.
POSIX PAX local headers are accepted for path overrides and non-semantic file
metadata; unsupported PAX fields fail closed. Archive metadata never changes
the canonical tree digest or installed ownership and permissions.

Acquisition never invokes package code, Git hooks, archive commands, install
scripts or build scripts. Remote URLs cannot contain credentials, query strings
or fragments. HTTPS redirects are limited to five hops and cannot downgrade to
HTTP. Diagnostics never print authorization material and redact query values in
redirect locations.
Git submodules and LFS are not fetched; packages requiring that content fail
rather than compiling incomplete sources.

Diagnostics name the dependency alias, owning manifest and failed phase. They
distinguish missing/stale lock, invalid source, missing/corrupt cache, unavailable
network or commit, digest mismatch, unsupported archive, resource-bound breach
and containment rejection. Lock and cache writes are atomic; failures leave no
partial lock or package and do not expose credentials.

## Implementation sequence and evidence

| Packet | Implementation boundary and required evidence |
| --- | --- |
| P02 · Locked local dependencies | Add lock reader/writer and stale-manifest checks; support live local directories and local archives; implement bounded, path-safe extraction and cache identity. Cover stable serialization, relative/absolute paths, changed archive bytes, malformed archives, traversal, duplicate paths, limits, stale locks and atomic failure behavior. |
| P03 · Pinned remote sources | Resolve Git refs only during explicit lock/update; record full commit and object format; acquire shared Git repository snapshots and remote archives; verify source/tree digests and enforce redirect, cache and offline rules. Cover moved refs, unavailable commits, changed remote bytes, redirects, credentials, subdirectories, sibling dependencies and shared roots in a diamond graph, offline cache and network failure. |
| P04 · `panack` workflow integration | Add `deps lock`, `deps update`, `deps restore` and manifest-aware check/compile/run behavior. Keep compiler resolution filesystem-only. Run complete Panack programs with direct/transitive dependencies and verify restore, lock stability, stale failure, offline behavior, diagnostics, relocation, diamond-root deduplication and package-bounded symlink resolution. |

Acceptance covers all four source forms; lock reproducibility; missing/stale
locks and caches; changed archive/remote content; unavailable offline sources;
path-safe extraction; useful diagnostics; and full programs executed through
`panack`. P01 defines this contract; it does not claim acquisition or package
management implementation is complete. GI#200 remains open through P04 and final
end-to-end acceptance.
