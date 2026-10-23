# P03a HTTPS archive acceptance

P03a completes on merge of [PR#578](https://github.com/sproates/panackelty/pull/578),
after its full local and hosted checks pass. It adds HTTPS archive acquisition to the separate Panack package tool. The
native streaming prerequisite merged as PR#576. Active manifest policy lives in `src/packages/manifest.panack`. The old shared
full-manifest parser remains frozen solely to preserve compiler/bootstrap input;
package commands and manifest policy tests use the package-owned parser.
Removing the unused shared copy requires a separate compiler cleanup.

This slice changes no native
implementation, compiler implementation or saved seed. Git acquisition and the
remaining P03/P04 packets remain open under GI#568 and GI#200.

## Contracts and evidence

`tests/package_https.sh` exercises the public lock, update, restore, check,
compile and run commands against a local TLS peer. The fixture records every
HTTP request and serves controlled redirects, framing errors and changed bytes.
It is test infrastructure; production acquisition uses the existing libcurl
adapter and Panack HTTP parser, never Node or an external downloader.

Verified focused cases:

- The lock retains the declared URL and an independently computed SHA-256 of
  compressed bytes, including when the response follows a redirect.
- Source and saved bytecode produce exact output. Repeated lock, check, compile,
  run and warm offline restore leave lock/state unchanged and make no requests.
- Cold or corrupt remote caches fail offline without requests. Explicit online
  restore repairs them only with bytes matching the pinned digest. Changed bytes
  fail and preserve the lock; update accepts the change deliberately.
- Selective update fetches the selected root dependency while retaining the
  unselected source and package pins. Verified caches survive project relocation.
- Transitive HTTPS dependencies resolve through the same pipeline. A manifest
  supplied by a remote archive cannot escape that archive into host-local paths,
  including through another local archive carried inside it.
- Redirect loops, HTTP downgrade, credentials, queries, untrusted certificates,
  non-200 status, declared oversize, truncation and content encoding fail. Failed
  updates preserve the previous lock and remove private staging directories.
- An incompressible 2 MiB payload exceeds the old HTTP client's 1 MiB cap and
  completes acquisition, execution and offline verification.

HTTP package fixtures additionally check fixed-length, chunked and
close-delimited bodies, duplicate framing, truncation and chunked size limits in
both source and bytecode. Existing local package regression tests still pass.
The manifest unit probe passes 30 cases, including HTTPS declarations and unsafe
URL rejection. The package-tool handoff test proves a package behaviour edit
invokes no compiler build and changes neither compiler artifact nor seed.

## Bounds and limitations

Panack HTTP framing feeds bounded chunks directly into the existing zlib/tar
pipeline. There is no complete compressed archive buffer or temporary archive
copy. The unchanged limits are 64 MiB compressed, 256 MiB expanded and 16,384
archive entries. HTTP headers/trailers are bounded to 64 KiB, chunk lines to
8 KiB, raw transport to 256 MiB and each response to 60 seconds. Redirects permit
at most five HTTPS hops. Offline repair from a declared local archive remains
supported; remote entries require a verified cache.

Publication remains atomic per cache entry, not across the entire graph. A
failure during publication requires explicit restore. No arbitrary FFI, package
hooks, external archive commands or compiler acquisition path is introduced.

## Validation and performance

The canonical local command is `make check`; PR#578 records its final result
and duration before merge. Hosted HTTPS acquisition passed on Linux and macOS,
including nested archives, URL policy and the larger-than-1-MiB fixture. Source
coverage policy checks pass (13 tests). The delivery remains conditional on the
full local check and green checks for the exact final PR head.

An earlier full run failed one saved-bytecode CLI regression while its source
run passed. The failing assertion was hidden by a 160-character transcript
preview. Two focused CLI runs and eight repetitions of the package-root race
check passed; diagnostics now retain up to 4 KiB on failure, with all assertions
unchanged. The full-suite rerun is the acceptance gate, not the focused passes.

On the local Apple Silicon Mac, one 2 MiB incompressible archive update and warm
offline restore were measured with `/usr/bin/time -l`. The update took 0.07 seconds with 15,597,568 bytes maximum resident set size;
warm verification took 0.04 seconds with 12,664,832 bytes maximum resident set
size. A later portable-timing repeat measured 0.08 and 0.04 seconds. These are local observations including the
launcher and package process, not a throughput guarantee or a new performance
budget. Use `PANACK_PACKAGE_MEASURE=1 sh tests/package_https.sh` to reproduce
platform peak-memory reporting; the default test uses portable elapsed timing.
