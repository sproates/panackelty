# Package tool

`main.panack` is the independent Panackelty package-tool entry point. Make
builds it as `build/tools/panack-packages.bc` with the saved compiler seed.
Package-tool source edits rebuild that artifact only. The launcher routes
project commands through the tool, which writes a temporary schema-1
resolved-project descriptor for the compiler.

JSON parsing and descriptor records live in `src/project_model/`. Active package
manifest policy lives in `manifest.panack` here. The old shared manifest parser
remains frozen as compiler input to preserve the existing bootstrap artifact;
package commands do not call it. Removing that unused copy requires a separately
reviewed compiler/bootstrap cleanup, not a new package feature. This
directory owns package command behaviour, lock handling, source acquisition,
archive parsing and cache policy. It must not import compiler
implementation modules. The compiler treats the descriptor as untrusted input
and keeps its own package-root identity and source-containment checks.

Local directory dependencies remain live at their declared paths. Local tar.gz
archives are streamed into private staging directories, verified, and published
under the application's .panack_modules directory. Generic host adapters provide
confined filesystem operations, zlib decompression and OpenSSL SHA-256. Panack
owns tar parsing, Unicode collision checks, graph resolution, lock serialization,
selective updates and canonical tree hashes.

Run `panack deps lock --project panack.json` to create a lock. Use
`deps update --project panack.json [ALIAS...]` for deliberate changes and
`deps restore --project panack.json [--offline]` to rebuild materialization
without changing the lock. Ordinary project commands only verify existing state.
HTTPS archives use the same extraction and verification pipeline, fed by bounded
`http/stream` reads. Redirects and lock/cache policy stay in Panack; libcurl supplies
verified TLS. Offline restore requires an already verified remote cache. Git
acquisition remains subsequent P03 work.

Publication is atomic for each individual cache entry. Graph validation precedes
publication; a failure partway through publishing several entries requires
restore. It never makes a partially extracted package visible to the compiler.
P02 acceptance is recorded in `docs/design/PACKAGE_LOCAL_ACCEPTANCE.md`.

P03b adds controlled Git acquisition as internal Panack modules. `git_snapshot.stage_git_commit`
accepts an explicit absolute Git executable, a credential-free HTTPS URL, an
object format and a full commit ID. It returns an owned staging handle, source
key and canonical tree digest; the caller must publish or close that handle.
No manifest is read and no dependency graph is changed. P03c connects this API
to public commands, locks and shared source caches.

Git 2.29 or newer is a native-tool dependency for this path. Git receives an exact
private environment and no inherited configuration. Raw blobs are extracted
without checkout filters, then copied and hashed in 64 KiB chunks. The entire
commit tree is validated before symlinks are created. The limits and acceptance
fixtures are recorded in `docs/design/PACKAGE_GIT_ACQUISITION_ACCEPTANCE.md`.
