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
eligible production `.panack` file under `src/`, 14 unit roots, 12 positive
functional roots and three compiler paths: source check, disassembly and a
byte-identical bootstrap fixed point. A library-scope root retains declarations
that the initial corpus never executes. New tracked production sources fail
collection until the manifest is reviewed. Tests and generated inputs are
identified but excluded from the production denominator.

External-process/death/CLI integration suites, dynamically generated checker
executions beyond SC4's registry limit, TCP integration, packages, browser/WASI
and other functional paths are explicitly omitted in this first manifest.
Omission from the execution corpus never removes an eligible production file.
SC6 can use the report to agree and close gaps; there is no threshold gate yet.

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
