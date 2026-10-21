# P10: Standard-library owned callers

GI#525 P10 follows the accepted [P09 seed checkpoint](p09-arrow-seed-checkpoint.md).
Completion takes effect on integration after full validation and exact-head CI.

## Scope

Library bodies and the stdlib conformance/unit programs now call owned receiver
APIs. The testing command supervisor uses owned array, text and Result methods;
fixture discovery uses Path.join and Array.append. Bytes, text and numeric
compatibility functions forward to their intrinsic-owned methods.
`TestWorkspace.remove_empty` is defined in its declaring module and preserves the
existing effectful filesystem contract. Constructors, host services, reporting,
and subprocess operations stay explicit functions. Core Path methods retain their
low-level implementation primitives; those are implementation backends, not
unmigrated caller APIs.

The bootstrap conformance program, stdlib module API case, 42 library assertions,
and fixture discovery/cleanup case use owned calls. Generic Option/Result owners
use typed bindings where inference lacks evidence. No compiler, VM or bytecode
format change is required. P11 migrates the remaining compiler/tests/examples/tools;
P12 removes compatibility surfaces. Historical inventory documents remain baseline
evidence rather than pretending the whole corpus has migrated.

## Exact temporary source-wrapper register

These 32 source functions remain solely to support callers pending P11/P12.
Every entry forwards to the indicated owned method and is removed in P12.
Compiler-provided legacy globals are separately tracked in the
[P01 API register](p01-api-register.md); this table counts source wrappers only.

| Module | Temporary functions | Owned replacement |
| --- | --- | --- |
| core (internal, 3) | core_ends_with, core_first, core_sort_by | Str.ends_with, Array.first, Array.sort_by |
| bytes (4) | push, join, length, at | Bytes methods with the same names |
| text (3) | reversed, encode_utf8, decode_utf8 | Str.reverse, Str.encode_utf8, Bytes.decode_utf8 |
| numeric (1) | divide | Nat.divide |
| option (1) | value_or | Option.value_or |
| result (1) | value_or | Result.value_or |
| path (7) | join, to_text, native_bytes, display, is_absolute, directory, filename | Path methods with the same names |
| time (11) | ticks; add_instant, is_before_instant, difference; add, subtract, scale, is_before, as_seconds, divide, ratio | Duration.ticks; Instant.add/is_before/difference; remaining Duration methods with the same names |
| testing_files (1) | workspace_remove_empty | TestWorkspace.remove_empty |

Retained ordinary functions include bytes.empty; path.from_text/from_native/current;
time.from_nanoseconds/from_seconds/seconds/milliseconds/now; environment.get;
filesystem, process, host, async and TCP services; testing assertions/reporting;
testing_files.workspace_create/discover_fixtures; and testing_commands services.
`host.decode_utf8` retains its checked Result contract. Pure Bytes.decode_utf8
retains its trap on malformed UTF-8. Neither becomes an alias for the other.

## Validation and impact

Focused source execution passes the stdlib conformance and module cases, all 42
stdlib assertions, and fixture cleanup including nonempty refusal and success.
The stdlib bytecode golden changes only in the conformance main: calls bypass the
compatibility wrappers, with one typed Result local. Disassembly was reviewed and
exact runtime output retained. Existing remaining callers exercise compatibility
forwarders in the full suite; no test or coverage requirement is removed.

Seed regeneration passes with identical compiler stages 2/3/4:
`c4f6b8b2b3c567a0400353eff7ba4f9f5373caa22027d6f3b4f42bb1f8f1bdfd`.
The saved seed is unchanged. All three stdlib artifacts match
`27af27f328cd4b4bb55c4f190ceff1cf55cfd522a0c54093ecb12df2953c41f8`
and retain exact output. All native oracle artifact contracts pass. The three
functional cases also pass nine exact-output CLI checks (source execution,
compilation and saved-bytecode execution), with empty stderr.
Full `make check` passed in 705s on 20 October 2026 (Europe/Gibraltar),
including 1,249 binding assertions and 532 functional assertions, unit 537s,
bootstrap 289s in parallel, release smoke 10s and quick start 1s. Runtime primitives and algorithms are unchanged. Owned calls remove
forwarding calls at migrated sites; no runtime speed claim is made. Compile and
validation times are recorded as evidence, with historical budgets informational
under GI#106. Website impact: none for currently released syntax; the planned
alpha.14 method-dispatch article already describes this migration.

SPEC, the stdlib README, coverage and self-hosting notes document the new library
boundary. Root README and architecture remain accurate: this changes library
callers, not the CLI, execution architecture or stable-release setup.

## Programme status on acceptance

Stage weights are unchanged. P10 completes the 15% stdlib stage, taking estimated
completion from 47% to 62%. P11 is next; GI#525 remains open.

| Stage / packets | Status | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | Done | 20% | 100% | 20 |
| Built-ins: P06, P08 | Done | 20% | 100% | 20 |
| Stdlib: P10 | Done | 15% | 100% | 15 |
| Compiler/source: P09, P11 | P09 done; P11 next | 20% | 10% | 2 |
| Legacy removal: P12 | Unstarted | 10% | 0% | 0 |
| Dot cutover: P13–P14 | Unstarted | 5% | 0% | 0 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| Total | | 100% | | 62% |
