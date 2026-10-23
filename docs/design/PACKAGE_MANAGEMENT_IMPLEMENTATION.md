# Package management implementation boundaries

GI#200 must deliver a package manager written in Panackelty, built separately
from the compiler. This document fixes the implementation boundaries for
P02–P04. Read it with the [P01 contract](PACKAGE_MANAGEMENT_CONTRACT.md), which
defines source identities, lock semantics, containment and command behaviour.
These are planned requirements, not claims about the current implementation.

## Starting point

P01 was accepted in [PR#571](https://github.com/sproates/panackelty/pull/571).
[PR#573](https://github.com/sproates/panackelty/pull/573) reverted the initial P02
archive implementation and restored that tracked tree on `next` at
`4398760adf93adcc49e67b09e10f9db41156449c`. Start replacement work from current
`next`. The withdrawn implementation and unfinished lock branch are references
for findings and regression cases, not an approved architecture to reinstate.

## Ownership and dependency direction

| Component | Required responsibility | Boundary |
| --- | --- | --- |
| Package tool under `src/packages/` | Panackelty entry point, manifest and lock handling, graph resolution, tar parsing, canonical hashing, cache policy, acquisition orchestration and diagnostics | Separately compiled bytecode; no import of compiler implementation modules |
| Shared project model | Pure Panackelty records and validation needed by both tool and compiler | Neutral module with no dependency on acquisition, CLI or compiler passes |
| Compiler | Source loading, package visibility, parsing, checking and code generation from a resolved project | No lock resolution, archive parsing, fetching or restoring; no import of package-tool implementation |
| Standard library and host adapters | HTTP transport, gzip decompression, cryptographic digests, filesystem and process operations | Generic capabilities; no manifest, lock, package identity, tar or cache policy in C |
| `bin/panack` | Command routing and invocation of separately built tools | No package policy in shell; preserve the public command and existing source/bytecode routes |

Reuse the existing HTTP package and its libcurl transport. Use zlib for gzip.
Implement tar parsing and digest canonicalisation in Panackelty. Use OpenSSL 3
or later through its EVP digest API for SHA-256, with known-answer tests. The
generic adapter accepts bounded byte chunks and returns digest bytes; it knows
nothing about packages, manifests, locks or cache paths. Do not introduce a C `package_archive` subsystem or a
package-specific hash intrinsic. If a required language capability is absent,
demonstrate that limitation before proposing a narrow generic extension.
Native-library management for user packages remains outside GI#200; linking
the toolchain to zlib or libcrypto does not add such a feature.

The owner approved the OpenSSL boundary on 23 October 2026 after the initial
pure-Panackelty SHA-256 implementation took 35.05 seconds to hash 64 KiB on the
development Mac. The EVP-backed implementation completed the same program in
0.11 seconds with the identical hash, including byte-buffer construction. This changes the earlier requirement to implement the hash
algorithm in Panackelty. Canonical input construction stays in Panackelty. Use
the same backend on macOS and Linux. OpenSSL 3 and later use Apache 2.0; preserve
its licence and applicable notices wherever release artifacts redistribute it.

## Separate build and compiler handoff

Build and install a separate package-tool bytecode artifact alongside the
compiler artifact. Give it its own source dependency fingerprint and focused
build/test target. A package-only source edit must rebuild that artifact without
recompiling the compiler or regenerating its bootstrap seed. Moving source files
while retaining the compiler import dependency does not meet this requirement.

For `deps lock`, `deps update` and `deps restore`, the launcher invokes the
package tool. For project `check`, `compile` and `run`, the tool first validates
the lock and existing sources without network access, extraction or writes to
the lock/cache. It emits a temporary resolved-project descriptor, then the
launcher invokes the compiler using that descriptor. Preserve arguments, exit
status and diagnostics, and remove temporary files on success and failure.
A project without dependencies needs no lock. Plain source and bytecode command
behaviour remains unchanged.

Use a versioned, strict JSON descriptor with a closed schema. It contains the
selected root package ID and one record per reachable package: identity,
physical package root, source root, entry, exports and owning alias-to-package-ID
edges. Include the root toolchain requirement. Acquisition URLs, selectors,
archive bytes and cache state are not compiler inputs. This descriptor is
temporary and machine-local; it is not a second persistent lockfile.

Schema 1 uses these exact fields. Every object is closed; fields shown below are
required unless their value is `null` as documented:

```json
{
  "schema": 1,
  "root": "example.app@1.0.0",
  "toolchain": "0.1.0-alpha.15",
  "packages": [{
    "id": "example.app@1.0.0",
    "name": "example.app",
    "version": "1.0.0",
    "package_root": "/workspace/app",
    "source_root": "/workspace/app/src",
    "entry": "/workspace/app/src/main.panack",
    "exports": [],
    "root_device": 1,
    "root_inode": 10,
    "dependencies": [{"alias": "geometry", "target": "example.geometry@1.0.0"}]
  }, {
    "id": "example.geometry@1.0.0",
    "name": "example.geometry",
    "version": "1.0.0",
    "package_root": "/workspace/geometry",
    "source_root": "/workspace/geometry/src",
    "entry": null,
    "exports": [{"module": "api", "path": "api.panack"}],
    "root_device": 1,
    "root_inode": 11,
    "dependencies": []
  }]
}
```

`root` and edge `target` are `name@version` IDs. `entry` is an absolute path
for an application and `null` for a library. Export paths remain package
relative. Root device/inode values bind each physical package root for this
invocation; the compiler rechecks them before loading sources. Package records
must describe exactly the reachable graph, with unique package names and no
conflicting physical roots.

P02's first slice must commit the exact descriptor schema, fixtures and internal
invocation before adding archive acquisition. The implementor chooses field
names and internal option spelling and records them there; owner consultation
is not required. Reuse existing project-model validation where appropriate.
The compiler treats the descriptor as untrusted: reject unsupported versions,
duplicate/unknown fields, invalid identities, dangling or unreachable graph
nodes and conflicting roots, and apply existing project/source limits. Retain
package visibility and physical containment checks when opening source files,
including symlinks. Validation by the package tool is no reason to remove them.

An initial compiler-interface change and seed update may be necessary. Generic
host-runtime changes may also invalidate compiler builds under current build
rules. Neither justifies coupling subsequent package-policy edits to compiler
rebuilds. GI#200 does not require a general separate-compilation or FFI project.

## Native adapter contract

Use the existing host-function mechanism to expose a small gzip stream adapter.
This crosses a foreign-function boundary; it does not expose arbitrary C calls
to Panackelty. The adapter owns zlib state and resource cleanup. Panackelty owns
the archive policy and passes the remaining output budget on each operation.

The operations must support opening a decoder, feeding bounded byte chunks,
reading bounded output, finishing and closing it. Results distinguish progress,
input needed, end of stream, invalid/truncated data and exhausted output budget.
Handle ownership must prevent use after close and release resources on every
error. No native allocation may depend on an unchecked archive-supplied size.

Select gzip decoding explicitly, not zlib/raw-deflate auto-detection. Validate
the gzip checksum and trailer. Support concatenated gzip members within one
shared compressed/expanded budget and reject trailing non-gzip data. zlib's
`inflate` finishes one member at a time, so the adapter must handle continuation
explicitly. See the [zlib manual](https://www.zlib.net/manual.html). No output is
published until the complete compressed input has been validated. Do not shell
out to `gzip` or `tar`, or implement DEFLATE in Panackelty.

Generic host filesystem operations may provide directory-relative creation,
no-follow opens and atomic publication. They receive handles, relative paths
and bytes, not package names or lock records. Panackelty decides which paths are
allowed and when verification is complete. A lexical path check followed by an
unprotected write is insufficient. Replacement must preserve the old usable
cache on failure; deleting it before rename does not satisfy atomic replacement.
Use the existing supported platform mechanisms, with explicit failure if the
required guarantee cannot be provided. Prove this before relying on it.

The original API inventory for P02a established the missing capability. `stdlib/filesystem`
provides path-based metadata, bounded read, truncating write, listing, removal,
directory creation and unique temporary paths. `stdlib/package_filesystem`
adds physical root identity, ancestry comparison and a no-follow rooted read.
The native implementation uses directory-relative opens for that rooted read,
but exposes no directory handle, directory-relative create/write, recursive
staging operation or atomic rename/publication operation to Panackelty. Ordinary
`fs_write` truncates its target in place. These APIs therefore do not establish
safe extraction or replacement of a published cache. P02 adds `stdlib/filesystem_stage`: opaque handles retain verified parent and
staging directories, create files through no-follow directory-relative opens,
and publish with an exclusive rename or atomic exchange. Native tests exercise
replacement, symlink refusal and cleanup. Panack decides when a validated
archive is ready to publish; the adapter receives no package or lock metadata.

Publication is atomic per cache entry. The complete graph is validated before
any entry is published, and each failed exchange preserves that entry. A group
of exchanges and the lock are not a single filesystem transaction. Update
writes the validated lock first; a later publication failure leaves complete
entries and requires `deps restore` before compilation can proceed. Cache state
is the canonical lock under `.panack_modules/.state/lock.json`, written after
all entries. Ordinary commands verify that state and each tree before emitting
a compiler descriptor.

## Archive and remote implementation choices

The initial tar reader supports POSIX ustar regular files and directories, plus
per-entry PAX `path`, `size` and timestamp/ownership metadata. Metadata does not
enter the canonical digest. Reject unknown PAX keys and global PAX headers,
GNU extensions, sparse files, links and special files with an unsupported-format
diagnostic. Validate header checksums, numeric overflow, PAX record lengths,
padding, payload length and the tar end marker. Permit zero padding after the
end marker; reject nonzero trailing tar data. Do not silently strip a wrapper
directory. Fixtures must include archives produced by ordinary archive tools
using the documented supported format.

Apply P01's 64 MiB compressed and 256 MiB expanded limits while streaming; the
expanded limit includes tar headers, padding and metadata. Enforce 16,384 unique
filesystem entries, including implicit parent directories, and at most 16,384
tar headers, including metadata headers. Validate effective PAX paths with the
same rules as header paths. Normalised duplicates, file/directory conflicts and
case collisions fail before publication. Sorting for the tree digest is UTF-8
byte order, independent of collision detection. Stage extraction under a fresh
private directory, then verify manifest and digest before publication.

For P03, use the existing process capability to invoke Git with explicit argv,
not a shell command assembled from user input. Work in a controlled temporary
repository; isolate system/global Git configuration, disable hooks, credential
helpers, external protocols and repository filters, and permit only the locked
HTTPS source. Do not run checkout filters, submodules, LFS or repository commands.
Obtain and validate the exact commit tree without executing repository content.
Preserve symbolic links for the P01 digest and enforce snapshot containment.
Treat Git as a documented native-tool dependency and test its absence. A Git
library migration is outside this implementation. The Git snapshot and sibling
package rules in P01 remain mandatory; do not copy each subdirectory into a
separate package cache.

## Delivery sequence and acceptance

| Slice | Deliverable and evidence required before continuing |
| --- | --- |
| P02a: independent tool | Package-tool entry/artifact, launcher route, exact descriptor schema and compiler handoff. Run a complete project with a local directory dependency. Demonstrate that a package-only behaviour edit changes tool output while compiler artifact/seed hashes and compiler build invocation count remain unchanged. Record the native API surface and prove staging/publication guarantees. |
| P02b: local graph and lock | Panackelty strict lock reader/writer, exact root/transitive manifest digests, graph validation and local-directory commands. Prove stale/missing lock failures, relocation, absolute-path caveat, diamond identity and deterministic serialization. |
| P02c: local archives | Narrow zlib adapter, Panackelty tar/canonicalisation implementation and safe cache lifecycle. Deliver local `deps lock/update/restore` and project check/compile/run through the public CLI. P02 ends with a usable local workflow, not disconnected parser modules. |
| P03: remote sources | HTTPS archives through existing HTTP support and controlled Git acquisition. Test redirects, moved refs, unavailable pinned commits, whole-snapshot verification, sibling/diamond roots, changed remote bytes and offline reuse. Extend the same tool and descriptor without importing acquisition into the compiler. |
| P04: final integration | Install/package both artifacts and native dependencies; finish help and diagnostics; validate all four source forms together, existing standalone routes, relocation and supported native platforms. Prove ordinary project commands remain network-free and never restore implicitly. |

P02c must include independently calculated SHA-256 and tree-digest fixtures,
not expectations generated by the implementation under test. Include
`src/nested/main.panack`, empty and implicit directories, mixed-case paths
`Z.panack` and `a.panack`, duplicate normalised names, parent/file conflicts,
traversal, corrupt/truncated gzip, concatenated members, expansion limits and
interrupted publication. Empty or malformed digest strings must fail; a real
root-manifest digest must round-trip and become stale when its exact bytes change.

Across P02–P04, test malformed lock graphs, cache tampering, symlinked cache/state
paths, package-bounded source symlinks and failure cleanup. Public-command tests
must execute complete Panackelty programs and assert output. Network prohibition
must be observed with an instrumented transport or blocked endpoint, not inferred
solely from a successful cached run. Capture lock/cache state before and after
ordinary commands to prove they do not mutate it.

Record comparable cold/warm timings, peak memory and compiler build counts for
a small graph and a larger bounded archive. Use chunked processing; do not build
the entire expanded archive or digest input through repeated byte concatenation.
Focused package tests must be usable without a compiler rebuild after package-only
edits. Required integration/full validation remains in place at delivery; build
independence is not permission to omit correctness checks.

## Implementation discretion

The language split, separate artifact, dependency direction, zlib choice, Git
process choice, command semantics and safety guarantees above are settled.
Choose internal names, module subdivision, error representation and test layout
without asking the owner. Resolve ordinary details using existing repository
conventions and document the choice in the delivery PR. Do not turn each slice
into an owner-approval checkpoint.

Escalate only when evidence shows that a settled requirement cannot be met,
requirements conflict, or a material scope/compatibility change is needed. Bring
the concrete failing case, impact and a recommended alternative. Do not silently
move package logic into C or the compiler to get past a difficulty. No additional
commands, registry, general FFI, new compiler modularity programme or package
build scripts are authorised by this plan.

## Unicode collision keys

Archive path collisions use Unicode 17.0.0 default full case folding (the C and
F mappings in Unicode CaseFolding.txt; no locale-specific Turkic mappings).
`src/packages/case_fold.panack` contains the sorted mapping table and a binary
search. These keys affect collision detection only: source names and canonical
tree digests retain their original UTF-8 bytes. No normalization is applied.
The data is distributed under `docs/licenses/Unicode-License.txt`, included in
installed documentation. When updating the Unicode version, regenerate both
sorted arrays from the C/F records and rerun the collision fixtures.
