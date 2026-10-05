# SC5 source baseline and publication

The initial baseline is **native `.panack` source execution on core `next`**.
It is separate from native C/LLVM coverage on core `main` and from browser/WASI
execution. It is not advertised as the entire canonical test suite.

## Reproduction and exact scope

Run `make source-coverage-report-tests`, then `make source-coverage-baseline`
from a clean committed checkout. The default output is `build/source-coverage/`;
an existing destination is refused. Node 24 and the normal native toolchain are
required, without npm dependencies or LLVM instrumentation flags. The VM's
opt-in SC4 collector gathers counters during fresh execution sessions.

[The versioned manifest](../tests/source_coverage/manifest.json) identifies every
eligible production `.panack` file under `src/`, 14 unit roots, 13 positive
functional roots and three compiler paths: source check, disassembly and a
byte-identical bootstrap fixed point. A library-scope root retains declarations
that the initial corpus never executes. New tracked production sources fail
collection until the manifest is reviewed. Tests and generated inputs are
identified but excluded from the production denominator.

External-process/death/CLI integration suites, dynamically generated checker
executions beyond SC4's registry limit, TCP integration, packages, browser/WASI
and other functional paths are explicitly omitted in this first manifest.
Omission from the execution corpus never removes an eligible production file.
SC6 adds the existing `host_types` fixture and targeted stdlib/UTF-8 assertions.
The reviewed regression policy below gates successful report generation.

Every listed root runs without transcript reuse. Unit assertion totals and
functional stdout are checked. Bootstrap output must match the checked-in seed;
SC5 does not regenerate or replace release/seed artifacts. Each session has
exact artifact/inventory identities, admissions, claims, raw records, a seal,
a nonce and a byte budget. Missing, partial, replayed, expired or gapped data
is not accepted as a successful collection.

## Source identities and figures

The self-hosted local exporter reproduces SC2 inventory and emission probes.
Each file-sized plan segment stays within existing native host file bounds;
compiler/artifact/inventory bytes and exact source items are checked by the
bounded Node reader. Plans are never downloaded or executed by the publisher.
Golden parity compares every source item with the accepted SC4 renderer on
both sides of a branch, including loops, short-circuit decisions and match arms.

Shared production source is counted once using canonical checkout path, exact
source bytes, structural item ID, kind, detail and span. Executions across
different artifact contexts contribute actual hits, not duplicate denominators.

- Lines: unique expression/statement **start lines**, not every spanned line.
- Functions: original source declarations, including known unexecuted functions.
- Branches: source outcomes, not incidental VM/lowering branches.
- Compile-time/import/type/record/enum declarations: explicit per-file exclusions.
- Missing/ambiguous associations and non-emitted eligible items: **unavailable**,
  never silently treated as zero. A known positive hit is definitely covered.

Metrics include covered, total, known zero and unavailable counts, plus lower
and upper bounds. The complete percentage is `null` whenever any denominator
item is unavailable (or the denominator is empty). Separate compiler, bytecode
and standard-library totals appear alongside every eligible source file.

`summary.json` contains exact scope, identities, SHA-256 source/toolchain/manifest
hashes, session evidence, timestamp and metrics. HTML shows covered/zero/unknown
source lines, functions, outcomes and exclusions. Development `--allow-dirty`
reports are non-publishable; hosted generation uses a clean committed checkout.
Raw local evidence is retained in the printed temporary workspace for diagnosis.

## SC6 regression policy

`tests/source_coverage/policy.json` is the versioned review record. Generation
checks this policy before writing or uploading a report. The checker consumes
the locally validated SC2/SC4 report; it is not a substitute for the raw reader
or the publisher's independent validation of downloaded artifacts.

Compiler, bytecode and stdlib each have exact line/function/source-outcome
baseline counts. Compare integer fractions, never rounded percentages. An
improvement in one component cannot offset a loss in another. Denominator,
execution manifest and declaration-exclusion changes fail until explicitly
reviewed. Missing roots/files, unknown measurements, inconsistent counts and
stale commit/manifest identities fail; development reports still require
`--allow-dirty` and remain non-publishable.

Selected UTF-8, option/result and duration source items additionally require
positive coverage. Their exact source hashes and structural IDs are pinned so
editing or deleting a protected item cannot silently evade its guard. These
guards establish execution; the new assertions establish correct results.

To change a baseline, show the old/new component counts, protected items,
manifest and exclusions with a written reason in the PR. Generate a fresh full
report, inspect every policy difference, update the committed policy deliberately,
and run the policy failure tests and full collection. Changes use the normal
review/branch approval rules; source-hash or scope changes are not automatically
waived. No automatic regeneration, automatic lowering, command-line bypass,
universal target percentage or publisher-only exception exists. A failed policy
keeps the previous live report; native C/main policy remains separate.

The first assertion batch covers UTF-8 boundary/malformed sequences, both
option/result arms, and exact/negative/zero duration ratios. It found and fixes
`duration_ratio` testing signed ticks against a natural zero, and `duration_divide`
receiving a signed zero: kind-sensitive equality skipped the error arm and trapped.
Comparing two Duration values preserves the intended typed error.
Existing division/path/clock assertions in
`host_types` now contribute to the measured corpus with unchanged expected stdout.

The complete post-batch development run has 31 fresh executions, zero unavailable
measurements and 68 covered protected items. Floors are reviewed as exact counts:

| Component | Lines | Functions | Source outcomes |
| --- | ---: | ---: | ---: |
| Compiler | 5,173/5,998 | 397/429 | 5,514/6,922 |
| Bytecode | 587/601 | 60/60 | 517/588 |
| Stdlib | 72/133 | 23/33 | 33/64 |

This gains 28 covered lines, 11 functions and 37 source outcomes over SC5 with
unchanged denominators and exclusions. Standalone tests pass 140 bytecode and
36 stdlib assertions; mutation checks reject overlong UTF-8, incorrect fallback
values and a removed duration error guard. Eleven policy tests cover regression,
denominator/scope/exclusion changes, incomplete or stale evidence and deliberate
reviewed baseline updates; the 13 raw-reader tests and 514 SC4 parity comparisons
remain passing. Clean hosted checks, integration and live verification are required
before this SC6 candidate is accepted.

## Publication boundary

Core Check runs on `next` pushes and PRs. Its separate source job creates
`source-coverage-RUN_ID` only after collection succeeds. The independent
publisher selects a successful trusted **push to `next`**, not a PR/fork run,
checks ancestry, artifact identity, expiry, summary completeness and monotonic
per-channel provenance, and exposes `source/html/index.html` and
`source/summary.json` at the coverage host. Failed/newer incomplete runs leave
the last accepted report in place; timestamps and commit links show freshness.

Native C/main report URLs and provenance remain unchanged. Both independently
pinned channels are verified before replacing the site, and every deployed file
is compared with its expected bytes. Publisher changes require their own PR and
approval. Core `main` is not changed by this programme item.
