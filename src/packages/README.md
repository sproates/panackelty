# Package tool

`main.panack` is the independent Panackelty package-tool entry point. Make
builds it as `build/tools/panack-packages.bc` with the saved compiler seed.
Package-tool source edits rebuild that artifact only. The launcher routes
project commands through the tool, which writes a temporary schema-1
resolved-project descriptor for the compiler.

Manifest parsing and descriptor records live in `src/project_model/`. This
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
Remote archives and Git sources remain the subsequent P03 delivery.

Publication is atomic for each individual cache entry. Graph validation precedes
publication; a failure partway through publishing several entries requires
restore. It never makes a partially extracted package visible to the compiler.
Full P02 acceptance remains in progress until the complete validation suite passes.
