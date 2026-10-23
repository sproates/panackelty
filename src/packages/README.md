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

The P02 implementation is in progress. Generic host adapters provide confined
filesystem staging, zlib streaming and OpenSSL SHA-256. Panack owns tar parsing
and the canonical source-tree digest. Focused tests cover extraction, digest
vectors, changed-tree rejection and cleanup. Archive/cache integration, selective
updates and full acceptance validation remain outstanding; this is not yet a
completed local-archive workflow.
