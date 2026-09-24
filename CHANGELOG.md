# Changelog

Notable changes to Panackelty are recorded here. Preview releases may change
source syntax, checking behavior, standard-library APIs, and bytecode as described
in `RELEASE_POLICY.md`.

## Unreleased

- Generic source functions with abstract body checking, inference from all value
  arguments, explicit complete type arguments, receiver-first calls, recursion,
  and purity preservation. Type arguments erase into ordinary version-7 calls.
- Portable `option_value_or[T]`, `result_value_or[T,E]`, and `array_first[T]`
  standard-library helpers, with a runnable generic-functions example.
- Constraints, generic function references, and return-context inference remain
  deferred. Published alpha.3 archives do not include these additions.

## 0.1.0-alpha.3 — 2026-09-15

### Highlights

- Local bindings may omit type annotations: `name = value` declares an immutable
  local when the name is not visible, and `mut name = value` declares a mutable
  local. Assignments retain fixed types and require mutability; shadowing remains
  prohibited. An unknown assignment target now declares a local instead of
  reporting an unknown-name error.
- Inferred initializers must determine complete types without evidence from later
  statements. Nested collection and constructor evidence combines consistently;
  unresolved types request an annotation. Numeric defaults, guarded domain types,
  and callable effects are preserved. Function signatures remain explicit.

### Compatibility

- Assigning to an unknown local name now declares an immutable binding instead
  of reporting an error. A misspelled assignment can therefore introduce a new
  variable; unused-binding warnings are not yet implemented.
- Existing explicit local annotations remain supported. Function parameters,
  return types, and record fields still require annotations. Incomplete local
  types cannot be resolved from later assignments or uses.
- Bytecode remains version 7. This is an experimental preview for Linux x86-64
  and macOS arm64 under the compatibility policy in `RELEASE_POLICY.md`.

## 0.1.0-alpha.2 — 2026-09-15

### Highlights

- Positioned compiler errors now include numbered source excerpts and carets,
  including imported modules. Source snapshots, four-column tabs, and visible
  Unicode/control escapes keep excerpts accurate and aligned.
- Faster native string operations and self-hosted compiler validation, with
  cached character metadata and direct indexing for ASCII strings.
- Validation and packaging now support source checkout paths containing spaces.
- Optimised native builds, reusable unit-test compiler probes, and reduced
  duplicate CI work keep development feedback within the validation budgets.

### Compatibility

- The accepted language syntax and bytecode format remain unchanged (version 7).
- Diagnostic output now includes source lines and carets after positioned
  headers. Tools that consume compiler stderr should allow these extra lines.
- This remains an experimental preview for Linux x86-64 and macOS arm64,
  subject to the compatibility policy in `RELEASE_POLICY.md`.

## 0.1.0-alpha.1 — 2026-09-04

The first public developer preview.

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

### Release verification

- Published from annotated tag `v0.1.0-alpha.1`; each provenance file records
  the exact tagged source commit
- Passed complete validation and exact-archive smoke tests on Ubuntu 22.04
  x86-64 and macOS 14 arm64 in the release workflow
- Published exactly two archives with adjacent SHA-256 checksums and build
  provenance, all tied to the tagged source commit
- Downloaded all six public assets from the GitHub release and independently
  verified both archive checksums
- Repeated the packaged quick start with the downloaded macOS arm64 archive;
  the downloaded Linux checksum matched the exact archive exercised by the
  Ubuntu release job
