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
byte-identical bootstrap fixed point. The three package-related sources and the
async filesystem, async host and continuous TCP service sources are currently
unmeasured and excluded from all denominators. A library-scope root
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
Each plan export has a five-minute timeout to accommodate hosted runner variation;
compiler-tool compilation and individual execution sessions retain three-minute
limits. Command failures report the full entry arguments, elapsed time and limit.
The hosted source-coverage job retains its overall 35-minute bound.

## Report integrity and release gate

`tests/source_coverage/policy.json` records the reviewed manifest and exclusions.
The manifest's policy hash is checked before compilation or collection and by
the fast policy tests. The policy must remain unchanged during collection.
Generation validates report identity, complete sessions, source rows, exclusions
and exact measured/unmeasured scope before writing or uploading. It does not
enforce a minimum percentage, covered count or protected-outcome count. The
checker consumes the locally validated SC2/SC4 report; it is not a substitute
for the raw reader or the publisher's independent validation of downloaded
artifacts.

The alpha.13 refresh retains the 45 measured files and 31 execution roots. The
reviewed six-file unmeasured list includes the three async/service files already
added during GI#162. Its 410 compile-time exclusions were checked against current
source declarations: 255 imports, 130 records and 25 enums. Updated identity and
line-position hashes bind the accepted namespace/compiler source; this refresh
does not remove measured files, execution roots or executable declarations.

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

Source coverage collection is independent of promotion and release. The **Source
coverage** workflow schedules a dispatch to `next` each Monday at 03:23 UTC.
Schedules originate from the default branch; the dispatched run is pinned to
its own `next` SHA so workflow and report provenance agree. Manual collection
uses **Source coverage → Run workflow → next**. Ordinary pushes and PRs never
collect the corpus; fast reader, manifest and policy checks remain in Check.

The independent coverage repository publishes successful reports weekly on
Monday at 05:23 UTC, or through a fresh manual **Coverage Pages** run on `main`.
It selects only trusted successful source-workflow runs on `next`, checks source
ancestry and exact artifact identity, and validates summary scope before
publication. Existing successful Check artifacts remain a migration fallback.
Failed collection or publication preserves the prior live report. The timestamp,
commit and run links show what was actually measured; a weekly report need not
represent the latest release. Native C/main reports retain their separate scope.

Both pinned channels are validated before replacement, and every deployed file
is compared with its expected bytes. Source report freshness and coverage amounts
are not release gates. Release still requires exact identity, product tests and
native package validation. Coverage-speed improvements remain alpha.14 scope.

## Operator rule: use a fresh publication run

For an ad hoc update, manually dispatch **Source coverage** on core `next`, wait
for its successful artifact, then dispatch a new **Coverage Pages** run on the
publisher's `main`. Do not rerun an old production preparation: duplicate
`github-pages` artifacts can make deploy-pages reject ambiguous selection.
Verify preparation, deployment, every-live-file comparison and exact source
provenance. Neither collection nor publication is required to release a candidate.
