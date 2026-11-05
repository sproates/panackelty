# Compiler understanding acceptance

The interactive command opens a function in source order. A developer selects a
statement, opens a compiler decision, reads the requirement and result, and
follows supporting source where available. Successful programs also expose the
instructions produced for each source construct. The terminal interface and
navigation are implemented in Panackelty. The native adapter supplies terminal
key, resize and signal handling.

This record covers the existing language. Final release integration includes
GI#435 Validated domain types when that feature supplies its new decisions.

## Evidence and provenance

The maintained C01–C10 and E01–E08 inventories remain in the
[delivery checklist](roadmap/records/next-release--agreed-scope-2026-10-06.md#preparation-evidence-register--source-reconciliation-28-october-2026).
The following groups identify the implementation and its acceptance evidence.

| Inventory entries | Implemented behavior | Regression evidence |
| --- | --- | --- |
| C01, C02, C08, C09, C10 | Preserve checked children on special-call failure. Retain typed host arguments, HTTP state/handler requirements and callable-reference decisions. | Module-binding suite and public explanation CLI cases cover accepted, rejected and unresolved requirements, arity and retained child evidence. |
| C03 | Retain the first generic constraint, subsequent joins and their earlier source, contextual constraint application and resulting types. | Generic inference tests assert first/subsequent records, binder identity, dependency span, conflicts and retention disabled. CLI checks the earlier argument and inferred result. |
| C04 | Present checked requirements and specific retained reasons, including field failures and unresolved constraints. The explorer keeps the enclosing statement visible. | CLI checks exact field failure causes and separates assignment mutability from value compatibility. Unit tests check mutability retention and its expression span. PTY checks navigate source, decisions and supporting conditions. |
| C05 | Retain the actual local-resource permission and the wrapper calls supporting it. | CLI exercises an async call through `stdlib/http_runtime.body_size` and checks the original primitive's source. Existing effect tests cover rejected, deferred and skipped checking. |
| C06 | Retain each callback requirement actually checked, including purity, arity, parameter and result requirements. | Map/reduce callback regressions and CLI component checks. Later requirements are absent when an earlier requirement prevents checking them. |
| C07 | Retain upper/lower bound origins, reassignment invalidation and specialized guarded arguments. | CLI checks a mutation-invalidated subtraction and an upper-bound conversion, alongside the existing guarded conversion and subtraction matrix. |
| E01–E03, E07 | Preserve statement/block spans and attribute stores, discards, loop machinery and implicit results/returns. | Exact instruction-source traces and the invariant requiring an origin for every instruction in real-source fixtures. Source-map-enabled and ordinary compilation produce identical executable bytes. |
| E04–E06, E08 | Retain lowering purposes and multiple await origins. Reverse lookup identifies original module/function names, shared generic bodies and source declarations with no standalone code. | Public source-map CLI cases, direct and indirect await traces, generated control-flow traces and realistic project audits. |

## Public commands and installed use

The explanation CLI passed 224 cases. The source-map CLI passed 57 cases,
including altered, stale and mismatched artifacts, source relocation and reverse
lookup. Invalid source-map inputs fail validation before a location is reported.

The PTY acceptance script exercises initial source-order selection, the first
unproved shortcut, opening an explanation, following its condition, returning to
the previous selection, corrected source, normal quit and Ctrl-C. It passed
against both the checkout and an installation made with `make install`.

```sh
make explain-tool
expect tools/explain/test.exp
make install PREFIX=/tmp/panack-explain-install
PANACK_BIN=/tmp/panack-explain-install/bin/panack expect tools/explain/test.exp
```

A normal terminal function query opens the explorer. `--text` and piped output
retain the plain report and compilation exit status. The interactive session
stays open to investigate a failed compilation and exits normally when the
reader quits. It does not run the inspected program.

## Real projects

The archive-hashing function in the package manager and `prior_index` in the
compiler have been opened through the explorer. The source-origin audit also
checks their complete projects, including imported functions and generated
instructions. It verifies instruction indices, function identity, original
source availability, nonempty source ranges and range bounds.

```sh
PANACKELTY_BOOTSTRAP_ROOT="$PWD" ./bin/panack compile \
  tests/runner/compiler_origin_audit.panack -o build/origin-audit.bc
PANACKELTY_BOOTSTRAP_ROOT="$PWD" ./bin/panack run \
  build/origin-audit.bc src/packages/main.panack explained
PANACKELTY_BOOTSTRAP_ROOT="$PWD" ./bin/panack run \
  build/origin-audit.bc src/compiler/main.panack explained
```

The package manager has 39,476 instructions and 39,486 recorded origins. The
compiler has 95,045 instructions and 95,045 origins. Both projects passed with
zero missing origins and zero invalid ranges, instruction indices or function
identities.

## Compiler and release acceptance

The compiler seed reproduced across successive self-compilation stages. Its
SHA-256 is `8f8a1a197436ad53cdcbcc0b517a29c09b02382fe5684787738dc113fb76c5bf`.
The standard-library SHA-256 is
`823a5f93fe2886a44b680cb09c1d634bdf6aa8fc0f5cdff57510cfe743eb8a0d`.

The final local `make check` passed on macOS arm64 in 957 seconds, including
unit, functional, bootstrap, packaged-release and quick-start checks. Unit work
took 559 seconds; bootstrap ran alongside it in 275 seconds. Functional work
took 291 seconds and packaged-release smoke took 19 seconds. Printed historical
timing-budget warnings are not current acceptance thresholds. The final unit
run includes the assignment-mutability decision and source-span regression.

Final real-project audit refresh, installed-terminal checks and cost measurements
are in progress. Hosted checks remain a separate exact-head merge gate.
GI#180 remains open for the final cross-feature acceptance after GI#435.
