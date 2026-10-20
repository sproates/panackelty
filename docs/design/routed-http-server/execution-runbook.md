# Execution and handoff runbook

## Start one packet

1. Read the workspace instructions, public contributor requirements, this plan,
   the selected packet and its acceptance rows. Check working tree and branch;
   preserve unrelated changes. Use current `next` as the ordinary integration
   baseline under the workspace's authorised workflow.
2. Read live GI#518 and compare with the snapshot. Record baseline SHA, accepted
   dependencies and any issue change. Do not guess from an earlier chat.
3. Read actual declarations in the source map and neighbouring tests before
   writing code. Record exact functions/types that will change. Distinguish
   production code, test-host code, quoted compiler input, source generators,
   installed-file lists and bytecode goldens.
4. Resolve packet decision gates. If a behaviour is not specified, stop that part
   and ask the owner. Do not change issue scope or relax acceptance independently.
5. Add the narrowest failing tests first. Implement only the selected packet.
   Keep the public facade truthful while internal work is incomplete.

## Existing commands and prerequisites

Run these from the core repository root. These are existing entry points at the
planning baseline, not promises that new HTTP tests already exist.

```sh
# Cheap inventory check before expensive work.
sh tests/fixture_inventory.sh

# Build the native launcher/runtime and existing native unit executable.
make native native-module-build
make native-unit

# Existing HTTP wire fixtures require their native peer binaries.
make build/vm/http_client build/vm/static_http_client build/vm/http_fixture
PANACK_HTTP_CLIENT="$PWD/build/vm/http_client" sh tests/http_package.sh
PANACK_HTTP_CLIENT="$PWD/build/vm/http_client" sh tests/continuous_service.sh
PANACK_HTTP_CLIENT="$PWD/build/vm/http_client" \
  PANACK_STATIC_HTTP_CLIENT="$PWD/build/vm/static_http_client" sh tests/static_http.sh
PANACK_BIN="$PWD/bin/panack" PANACK_HTTP_FIXTURE="$PWD/build/vm/http_fixture" \
  sh tests/http_client_package.sh

# Final product/shared-harness gate, after focused tests pass and edits settle.
make check
```

P01 verifies these commands on its actual baseline and records any environment
prerequisites. Do not run all of them for every tiny edit: select affected focused
checks, then retain the required final canonical gate. New pure HTTP tests need a
manifest/project-aware runner because package imports are not standalone scripts.
P02 must record the exact new focused command and ensure normal CI invokes it;
a command printed in a design document is not coverage.

For compiler probes, use `tests/run_probe.sh` and read its current invocation
contract. Do not improvise a staged-compiler environment. `PANACK_PROBE_SEED`
selects the compiler building a probe; `PANACK_TEST_COMPILER` only affects
cooperating test helpers. The public launcher uses its saved seed. Freeze inputs
before probe compilation and seed refresh. If compiler/native operations change,
follow the existing `make regenerate-seed` and fixed-point workflow; do not refresh
or commit seed bytes merely because package code changed.

## Before the comprehensive run

- Run the inventory preflight after fixture/example additions. Update independent
  counts/transcripts deliberately, never regenerate expected results blindly.
- Search all tests for changed API names, Makefile target text, generated snippets,
  exact diagnostics, installed-file expectations and public exports.
- Compile changed test hosts and execute generated variants. A passing compiler
  self-build does not exercise all test inputs or socket behaviours.
- Run targeted native failure/cancellation tests for runtime changes, plus affected
  public source and bytecode tests. Preserve the client and static-preview coverage.
- Review the final diff. Remove abandoned prototypes and unused public wrappers.
  Record every intentional retention with owner, caller and removal condition.

## Integration

Record full validation timing and workload; historical timing warnings do not
justify weakening assertions or increasing limits. Runtime packets need measured
bounded-resource evidence and comparison with the unchanged baseline where
possible. No throughput or credit-saving claim follows from a faster single run.

Open a PR against the authorised integration branch. Do not merge pending/failed
checks or checks for a different commit. For GI#518, Ian must explicitly approve this specific PR before merge, even
when it targets `next` and every check is green. This programme-specific rule
overrides general standing merge permission. Implementation approval, prior PR
approval and silence do not count. Leave the PR open and request approval after
presenting the exact head, validation evidence and remaining limitations. Do not
enable auto-merge. Seek fresh approval after substantive changes to an approved
PR, and obtain separate approval for every split PR. Each packet report lists what is complete and what remains unsupported.
Use `Refs #518` for intermediate work; only final accepted delivery closes it.
Keep alpha.14 release/website publication separate. Run scoped cleanup and verify
a clean tree; retain needed logs outside the checkout, not generated binaries in
source commits.

## Handoff template

Copy this into `pNN-acceptance.md` when recording the packet; fill every field.

```text
Packet and state: PNN / unstarted | implementing | blocked | validation pending | accepted
Baseline revision:
Delivered revision and PR:
Owner merge approval: pending | exact approval reference and approved head
Contract snapshot date / live issue differences:
Accepted prerequisites:
Files and declarations changed:
Acceptance IDs, test paths, exact commands and results:
Source / bytecode / installed modes actually checked:
Seed/ABI changes and evidence, or reason none:
Ownership, cancellation and resource evidence:
Observed timings and comparison limitations:
Unresolved gates and exact owner question:
Retained compatibility/prototypes and removal condition:
Documentation, roadmap and website follow-up:
Next eligible packet and why:
```

Never fill “accepted” while required tests or owner decisions are missing. A new
agent resumes from the evidence and repository state, not from the prior agent's
confidence. If a test cannot run, record the obstacle and unverified guarantee.
