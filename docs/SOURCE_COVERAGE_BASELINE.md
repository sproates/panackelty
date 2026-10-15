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

[The versioned manifest](../tests/source_coverage/manifest.json) partitions every
tracked production `.panack` file under `src/` into measured and explicitly
unmeasured sources. Its 45 measured files use 14 unit roots, 13 positive
functional roots and three compiler paths: source check, disassembly and a
byte-identical bootstrap fixed point. The three package-related sources are
currently unmeasured and excluded from all denominators. A library-scope root
retains declarations that the corpus never executes. New tracked production
sources must be assigned to one side of the partition before report generation.
Tests and generated inputs are identified but excluded from the production
denominator.

External-process/death/CLI integration suites, dynamically generated checker
executions beyond SC4's registry limit, TCP integration, packages, browser/WASI
and other functional paths are explicitly omitted in this first manifest.
Omission from the execution corpus never removes an eligible production file.
SC6 adds the existing `host_types` fixture and targeted stdlib/UTF-8 assertions.

Every listed root runs without transcript reuse. Unit assertion totals and
functional stdout are checked. Bootstrap output must match the checked-in seed;
SC5 does not regenerate or replace release/seed artifacts. Each session has
exact artifact/inventory identities, admissions, claims, raw records, a seal,
a nonce and a byte budget. Missing, partial, replayed, expired or gapped data
is not accepted as a successful collection.

## Source identities and figures

The self-hosted local exporter reproduces SC2 inventory and emission probes.
Plan headers, session registries and raw records are bounded at 32 MiB;
individual compiler/artifact/inventory blobs and source segments remain 16 MiB.
These containers fit the synchronous filesystem read/write bound;
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
and standard-library totals appear alongside every measured source file.

`summary.json` contains exact scope, identities, SHA-256 source/toolchain/manifest
hashes, session evidence, timestamp and metrics. HTML shows covered/zero/unknown
source lines, functions, outcomes and exclusions. Development `--allow-dirty`
reports are non-publishable; hosted generation uses a clean committed checkout.
Raw local evidence is retained in the printed temporary workspace for diagnosis.

## Report integrity and release gate

`tests/source_coverage/policy.json` records the reviewed manifest and exclusions.
Generation validates report identity, complete sessions, source rows, exclusions
and exact measured/unmeasured scope before writing or uploading. It does not
enforce a minimum percentage, covered count or protected-outcome count. The
checker consumes the locally validated SC2/SC4 report; it is not a substitute
for the raw reader or the publisher's independent validation of downloaded
artifacts.

Coverage percentages and counts are report data for review; any amount, including
zero, is valid and never blocks report acceptance or release. Unavailable items
remain explicitly unknown and make the relevant percentage incomplete; they do
not invalidate a report. Missing roots or files, incomplete sessions,
inconsistent counts and stale commit/manifest identities still make a report
invalid. Development reports still require `--allow-dirty` and remain
non-publishable. The release workflow requires a fresh, valid published report
for the release candidate, but it does not inspect or enforce coverage amounts.

The report's historical percentages remain useful when comparing releases and
deciding where to invest in tests. Those comparisons inform product decisions;
they are not release criteria. Native C/main policy remains separate.

The first assertion batch covers UTF-8 boundary/malformed sequences, both
option/result arms, and exact/negative/zero duration ratios. It found and fixes
`duration_ratio` testing signed ticks against a natural zero, and `duration_divide`
receiving a signed zero: kind-sensitive equality skipped the error arm and trapped.
Comparing two Duration values preserves the intended typed error.
Existing division/path/clock assertions in
`host_types` now contribute to the measured corpus with unchanged expected stdout.

The complete post-batch development run had 31 fresh executions, zero unavailable
measurements and 68 covered outcomes. These are historical report results, not
floors or release criteria:

| Component | Lines | Functions | Source outcomes |
| --- | ---: | ---: | ---: |
| Compiler | 5,173/5,998 | 397/429 | 5,514/6,922 |
| Bytecode | 587/601 | 60/60 | 517/588 |
| Stdlib | 72/133 | 23/33 | 33/64 |

This gained 28 covered lines, 11 functions and 37 source outcomes over SC5 with
unchanged denominators and exclusions. Standalone tests passed 140 bytecode and
36 stdlib assertions; mutation checks rejected overlong UTF-8, incorrect fallback
values and a removed duration error guard. Tests cover report integrity, exact
scope, exclusions, incomplete/stale evidence and zero-coverage acceptance; the
13 raw-reader tests and 514 SC4 parity comparisons remain applicable. SC6 was
accepted on 2026-10-06: PR#307 merged to `next` as
`b382bd84ce65b64bdcd859f4decfd600d75663c4`; fresh next Check run 37384119034
passed all 22 jobs and reproduced these counts, 31 executions and 68 protected
items. Publisher run 37387725315 passed preparation, deployment and byte verification
of every live file. Live provenance identifies the SC6 commit. The bounded GI#131
programme is 100% accepted with unchanged weights; core main remains untouched.

## Publication boundary

Core Check runs on `next` pushes and PRs. Full source collection runs automatically
only for full-validation PRs targeting `main`; ordinary `next` PRs and pushes run
the fast reader/policy/routing tests without collecting the corpus. An explicit
`next` refresh push opts in by including `[source-coverage]` in its head commit
message (for example `git commit --allow-empty -m 'Refresh report [source-coverage]'`
followed by `git push origin next`). This runs the source collection on top of
the route's usual checks, including when the change itself is documentation-only;
it does not select product validation or package builds. This is an intentional
expensive report refresh, not a requirement for a task slice. Promotion still
requires separate owner approval. Unmarked documentation-only changes retain
the short route.

The separate source job creates
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

## Operator rule: refresh with a fresh publication run

For release preparation, create a fresh successful source-coverage artifact
on the `next` release candidate by including `[source-coverage]` in its commit
message. Then start a **new** Coverage Pages run on the publisher main branch
using workflow_dispatch. In GitHub Actions select Coverage Pages, Run workflow,
branch main, then Run workflow. If the connector cannot dispatch workflows, use
the authorised GitHub browser fallback; do not substitute a rerun of an old
successful production run.

Rerunning preparation can leave multiple artifacts named github-pages in one
workflow run; deploy-pages rejects the ambiguous selection. On 2026-10-06 a rerun
of 37378463006 hit that exact failure. A fresh run has its own artifact namespace.
Verify prepare, deploy and every-live-file verification, then confirm source
provenance matches the intended core candidate before promoting it. The core
Release workflow independently checks this provenance, ancestry, and unchanged
coverage inputs and source-scope identity before it allows package validation to
start. It does not require any coverage percentage or amount. No publisher code
change is required to refresh the report.
