# Compiler understanding acceptance

The interactive command opens a function in source order. A developer selects a
statement, opens a compiler decision, reads the requirement and result, and
follows supporting source where available. Successful programs also expose the
instructions produced for each source construct. The terminal interface and
navigation are implemented in Panackelty. The native adapter supplies terminal
key, resize and signal handling.

The [final alpha.15 integration](#final-alpha15-integration) covers validated
domain types. The earlier measurements and checks below record the original
delivery of the existing-language explorer.

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
compiler has 95,081 instructions and 95,081 origins. Both projects passed with
zero missing origins and zero invalid ranges, instruction indices or function
identities.

## Compiler and release acceptance

The compiler seed reproduced across successive self-compilation stages. Its
SHA-256 is `8f8a1a197436ad53cdcbcc0b517a29c09b02382fe5684787738dc113fb76c5bf`.
The standard-library SHA-256 is
`823a5f93fe2886a44b680cb09c1d634bdf6aa8fc0f5cdff57510cfe743eb8a0d`.

The implementation at `b1f125e` passed local `make check` on macOS arm64 in
957 seconds, including
unit, functional, bootstrap, packaged-release and quick-start checks. Unit work
took 559 seconds; bootstrap ran alongside it in 275 seconds. Functional work
took 291 seconds and packaged-release smoke took 19 seconds. Printed historical
timing-budget warnings are not current acceptance thresholds. The final unit
run includes the assignment-mutability decision and source-span regression.

Clean-checkout packaging and conformance dry runs also pass. The explorer's
VM prerequisite delegates to the native build when no executable exists, so
recursive dry runs do not depend on an already-built checkout. CI routing,
partition and conformance-control regressions cover this build path.

Final real-project audits and checkout/installed terminal checks passed against
the refreshed seed. Comparable cost measurements are recorded below. Hosted
checks remain a separate exact-head merge gate.
The domain-constraints and public explanation fixtures also verify GI#435
static proof, known failure and runtime-check evidence. The imported record
guard is shown at its declaration. A reverse lookup on its comparison finds
the instructions emitted in the validating caller. The final integration below covers the remaining cross-feature acceptance.


## Cost measurement method

Measurements use macOS 26.5 arm64, the same VM and identical final source and
standard-library files. The baseline compiler is revision `8120706`, seed
`d2c59d86d6bfe12d62efa66a0e4f8d4e1b931451014d14635ddd750eedba27e7`;
the candidate is implementation revision `b1f125e` and the seed recorded above.
Each command starts a fresh process. Filesystem caches are not flushed. The
validation suite finished before measurement; no other validation was
intentionally run alongside it. Three sequential before/after pairs are taken
for each workload. Ranges disclose the variability on this development Mac.

The entry points are `src/packages/main.panack` and `src/compiler/main.panack`.
Text queries select `hash_archive_file` and `prior_index`, respectively. Check
mode disables explanation retention; text explain enables it. Explorer startup
also prepares source/instruction relationships, so it does more work than the
text query. Peak RSS is measured during preparation, not steady navigation.

The measurement commands, from the checkout root with its source and standard
library roots selected, are:

```sh
/usr/bin/time -lp ./panack-vm run COMPILER.bc check SOURCE.panack
/usr/bin/time -lp ./panack-vm run COMPILER.bc explain SOURCE.panack --function NAME
printf 'resize\n80\n30\nquit\n' | /usr/bin/time -lp ./panack-vm run \
  build/explain/explorer.bc SOURCE.panack --function NAME
```

The compiler seed grows from 1,015,464 to 1,050,539 bytes. The installed explorer
adds 973,599 bytes of Panack bytecode and a 35,392-byte native terminal adapter on
this Mac. Installation builds the explorer once; ordinary source checks do not
build it. Its build dependencies exclude package-policy sources, verified with
a dry-run package-source modification. These figures do not claim unchanged
build or memory costs.

| Project | Command | Median seconds (range) | Median peak RSS MiB |
| --- | --- | ---: | ---: |
| packages | before-check | 8.22 (7.71–9.70) | 158.0 |
| packages | after-check | 7.92 (7.91–8.44) | 162.1 |
| packages | before-explain | 8.51 (8.41–8.75) | 199.5 |
| packages | after-explain | 8.66 (8.48–10.41) | 216.3 |
| packages | explorer | 9.62 (9.34–11.17) | 253.0 |
| compiler | before-check | 34.99 (32.98–35.79) | 313.7 |
| compiler | after-check | 33.63 (33.28–35.83) | 331.4 |
| compiler | before-explain | 35.70 (34.93–36.59) | 421.8 |
| compiler | after-explain | 35.76 (34.25–37.62) | 450.8 |
| compiler | explorer | 38.30 (35.64–42.73) | 539.3 |

The before/after timing ranges overlap on both workloads; these measurements do
not establish a material ordinary-check or text-query latency regression. They
also do not establish a speed improvement. Additional retained evidence raises
text-query peak memory by about 17 MiB for packages and 29 MiB for the compiler.
Ordinary-check peak memory rises by about 4 MiB and 18 MiB respectively, including
the additional source spans carried through the frontend.

The explorer prepares the whole project even when selecting one function. Its
median startup is 9.62 seconds for packages and 38.30 seconds for the compiler,
with peak memory of 253.0 and 539.3 MiB. This is the disclosed cost of the current
source/instruction view, not a claim of instant large-project startup. The
delivery assessment accepts these bounded preparation costs for the requested
capability; no ordinary compilation or execution regression was observed in
correctness tests. The final measurements below revisit these workloads after GI#435.

## Final alpha.15 integration

The final compiler seed is `9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.
This checkpoint covers GI#134 Compiler explanations,
GI#173 Compilation provenance and their umbrella GI#180 Compiler understanding.
Runtime tracing, counterfactual compilation and change prediction have separate
issue records.

| Inventory reconciliation | Final evidence |
| --- | --- |
| C01–C04, C06, C09–C10: value checking, generic and contextual requirements, callbacks and partial results | Existing component and CLI cases continue to cover calls, joins, callbacks, methods, unresolved dependencies and usable sibling functions. New regressions cover guarded array elements and the declared input type of `validate`. A rejected input reports why validation cannot run. |
| C05, C07–C08: effects, guard proofs and specialized calls | Append, map, set and indirect calls retain the checked guard and its declaration. A source link identifies a branch or assignment supplying a relevant domain fact, or a write invalidating earlier facts. A static proof, a missing proof and a check deferred to runtime have distinct labels. Effects from an imported guard are selected by their owning function. |
| E01–E03, E07: emitted instructions and generated origins | The whole-project audit verifies every instruction has an origin and those origins name a real function, an in-range instruction and a nonempty range of the corresponding source. The imported guard test exercises the public explorer with a declaration beyond the length of the caller's source file. |
| E04–E06, E08: transformations, shared code, no output and both lookup directions | Existing source-map and instruction-source fixtures cover loops, shared generics, awaits, unused declarations and invalid sidecars. The installed domain validation fixture compares the executable bytes with and without a map, finds a generated validation call by the guard declaration, then looks up its instruction to recover the same module and expression. |

For example, in a `[Word]` array with a non-blank element guard,
`words.append("ok")` reports the requirement on the literal and links to the
`Word` declaration. In a conditional proof, selecting **Source of the guard fact**
opens the expression supplying the evidence. For `validate[Word](input)` the
outcome is **Checked at runtime**. The explorer can open the imported guard's
source and its generated instructions without executing the program.

The public CLI suite passed all 283 assertions. The installed mapped and ordinary executables were byte-identical (8,834 bytes). The separate map was 53,698 bytes.

The installed PTY regression runs the reviewed booking example, exercises the
first-unproved shortcut and supporting-source navigation, exits normally and
with Ctrl-C, and verifies terminal mode restoration. It then opens an imported
guard in the overview and detail screens. Plain installed queries also exercise
a rejected function with a valid generic sibling, an invalidated local proof,
and the packaged dispatch example.

### Whole-project audit

The development audit now counts retained type-requirement records as well as
instruction origins. This is one specific record family; it excludes operator,
effect, subtraction and other evidence families from the retention count.
The command under **Real projects** above accepts `plain` or `explained`.
Both modes produced the same instruction and origin counts for each project.

| Entry point | Instructions | Origins | Type requirements, plain / explained |
| --- | ---: | ---: | ---: |
| Validated dispatch example | 1,004 | 1,004 | 0 / 558 |
| Domain constraints fixture | 1,488 | 1,488 | 0 / 932 |
| Package tool | 39,476 | 39,486 | 0 / 16,415 |
| Self-hosted compiler | 103,974 | 103,974 | 0 / 48,212 |

All audited instructions had an origin. The audit found zero invalid ranges,
indices or source/function identities in either mode. The public lookup and
navigation regressions separately check what those origins actually mean.

### Final cost

The timings below use the same local machine, seed and VM. Each check and text explanation ran in a fresh process three times with alternating order. Cells show median wall time (range in seconds) and median peak resident memory. Explorer startup is one observation per project, ending after the first render and quit. These measure whole-project preparation even when the query selects one function.

| Project | Check | Text explanation | Explorer startup |
| --- | --- | --- | --- |
| Validated dispatch | 0.25 s (0.22–0.27), 26.6 MiB | 0.32 s (0.29–0.37), 28.6 MiB | 0.31 s, 28.5 MiB |
| Package tool | 8.31 s (8.25–8.44), 165.7 MiB | 9.38 s (8.83–9.62), 220.5 MiB | 9.50 s, 256.9 MiB |
| Self-hosted compiler | 38.19 s (37.61–38.26), 366.3 MiB | 38.80 s (38.50–38.90), 493.6 MiB | 40.15 s, 589.0 MiB |

The dispatch example is small enough for immediate exploration. On the compiler itself, preparing an interactive session costs tens of seconds and substantially more memory than an ordinary check. This affects how quickly a developer can begin exploring a large project. Keep these workloads in the alpha.16 performance review (RM#157) when evaluating startup improvements.
