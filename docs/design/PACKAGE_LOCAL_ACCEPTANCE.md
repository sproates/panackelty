# P02 local dependency acceptance

This record describes the candidate implementation in PR#575. Full-suite and
exact-head CI acceptance are still pending; GI#567 remains in progress.

The package tool is Panack under `src/packages`. It emits a strict temporary
resolved-project descriptor; the compiler checks identity, reachability, depth,
physical roots and source containment. Package-only edits rebuild the separate
package artifact without rebuilding the compiler or refreshing its seed.
Generic native adapters own confined filesystem handles, zlib streams and
OpenSSL EVP contexts. Tar, lock, graph, cache and update policy remain in Panack.

## Verified focused evidence

- The refreshed seed produces identical compiler bytes in stages 2, 3 and 4.
- Public CLI fixtures cover live directories, local and transitive archives,
  selective updates, lock no-op bytes, stale manifests, changed archives,
  cache repair, relocation and offline restore without lock changes.
- Malformed archive fixtures reject links, duplicate paths, Unicode collisions,
  trailing or truncated gzip, the 64 MiB compressed and 256 MiB expanded limits,
  and a real archive exceeding 16,384 headers/entries.
  Failed extraction leaves no stage. Native tests cover output budget exhaustion,
  concatenated gzip and parent-directory substitution.
- SHA-256 and canonical tree digests match independently calculated vectors,
  including UTF-8 ordering of `Z.panack` before `a.panack`, implicit parents
  and an empty directory.
  Lock validation rejects missing digests and excessive graph depth. Descriptor
  validation rejects cycles and conflicting physical identities.
- Staging, gzip and digest native tests pass address/undefined-behaviour
  sanitizers. Installed-tool smoke and quick-start checks pass, including
  source/bytecode HTTP servers and signal-driven shutdown.

## Performance evidence

Measured on the development Apple-silicon Mac on 23 October 2026 with the native
optimised VM, using `/usr/bin/time`. The fixture has one archived library with
one 1 MiB repeated-byte data file and a small complete Panack program. It is
highly compressible and is not a general archive benchmark. Concurrent validation
was running; these are individual observations, not statistical guarantees.

| Operation | Elapsed | Peak resident memory |
| --- | ---: | ---: |
| Initial lock and extraction | 0.03 s | Unavailable in sandbox |
| Offline restore and atomic replacement | 0.03 s | 12,009,472 bytes |
| Existing-lock and cache verification | 0.02 s | 11,616,256 bytes |
| Project check including cache verification | 0.12 s | 22,364,160 bytes |

Repeating the restore and verification operations with a 32 MiB random-byte
payload (incompressible) produced:

| Operation | Elapsed | Peak resident memory |
| --- | ---: | ---: |
| Offline restore and replacement | 0.19 s | 12,156,928 bytes |
| Existing-lock and cache verification | 0.14 s | 11,599,872 bytes |
| Project check including cache verification | 0.26 s | 22,200,320 bytes |

The similar peak memory supports the streaming implementation for this workload.
It does not cover maximum-length path inventories or all 64-package graphs.

A separate adversarial archive with 16,384 empty files plus root and manifest
headers was rejected at the bound in 16.46 seconds, including the probe wrapper.
This exposes the cost of the current persistent collection operations at the
entry limit. It does not affect compiler build independence; large archives
remain deliberately bounded, and no numerical speed claim is made for them.

The earlier 64 KiB SHA comparison measured 35.05 seconds in pure Panack and
0.11 seconds through EVP, with identical digest bytes. Those timings include
fixture construction. They justify using the existing crypto library without
moving package policy into C.

## Failure boundaries and remaining scope

Publication is atomic per package directory; the whole graph and lock are not
one filesystem transaction. Validation completes before publication. Update
writes a complete validated lock before exchanging cache entries; a subsequent
publication failure requires restore. Cache state is published last, so ordinary
commands fail closed on mismatched state. This is visibility atomicity, not a
crash-durability guarantee. External replacement of an old staging name can
prevent cleanup; the adapter refuses to delete the substituted path.

Remote archives, Git snapshots and all-source workflow acceptance remain P03
and P04. New package modules are accounted as unmeasured in the source-coverage
inventory; focused assertions do not establish an execution coverage percentage.
