# Changelog

Notable changes to Panackelty are recorded here. Preview releases may change
source syntax, checking behavior, standard-library APIs, and bytecode as described
in `RELEASE_POLICY.md`.

## 0.1.0-alpha.1 — unreleased

This will be the first public developer preview.

### Highlights

- Self-hosted Panackelty compiler running on the native C11 VM
- Arbitrary-precision `Nat` and `Int` values and exact base-10 `Dec` arithmetic
- Guarded domain types and explicit pure/effectful function boundaries
- Records, generic tagged unions, exhaustive matching, persistent collections,
  callable values, modules, and a bundled standard library
- Verified, deterministic version-7 bytecode
- `check`, `compile`, `run`, and `disasm` commands
- Python-free, relocatable download archives with SHA-256 checksums and build
  provenance for both supported platforms
- Download-first quick start and a compact tour backed by packaged, tested
  example programs
- Primary `file:line:column` locations for lexer, parser, name-resolution, and
  type-checking failures, including failures in imported modules
- A structured public bug-report form that collects version, platform, minimal
  input, reproduction command, expected behavior, and complete output

### Preview limitations

- Source and standard-library compatibility are not yet stable
- Bytecode compatibility is not promised across Panackelty releases
- The initial binary targets are Linux x86-64 and macOS arm64
- Windows, package management, concurrency, generic source functions, traits,
  and a single-file executable are not included
- Diagnostics outside the primary lexer, parser, name, and type failures do not
  yet consistently include source locations; source excerpts are not rendered

See `RELEASE_POLICY.md`, `SPEC.md`, and `ROADMAP.md` for the complete preview
contract and remaining work.
