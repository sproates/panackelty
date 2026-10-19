# P06: Owned intrinsic dispatch

GI#525 P06 builds on P05 (`f7858b169d855756213aa72d3781e6a56a61dc56`).
The packet completes on integration after exact-head CI passes.

## Delivered scope

The designated toolchain core receives 30 compiler-provided method declarations.
Each has canonical owner/name metadata, visibility, a typed signature, pure effect
and a closed `OwnedIntrinsic` operation ID. Source methods retain no intrinsic ID.
No user-facing intrinsic syntax or ordinary callable names are introduced.
Synthetic declarations participate in the separate owner index and duplicate
checking. A source implementation cannot replace an intrinsic, and a user package
cannot register one by adopting a core filename or package name.

Owned lookup selects the declaration before any intrinsic-specific checking.
The existing primitive checkers preserve collection inference, array joins, exact
numeric conversions, argument checks and runtime traps. The resulting bound call
retains its `BodyDeclaration` and intrinsic ID; emission maps that ID to an existing
VM primitive. It does not resolve the source method spelling again. Receiver and
arguments are evaluated once in their original order. No VM, bytecode format or
collection runtime change is included.

| Owner | Registered intrinsic methods |
| --- | --- |
| Map[K,V] | put, get, has |
| Set[T] | add, has |
| Array[T] | append, concat, len |
| Str | len, slice, reverse, starts_with, starts_with_at, is_digit, is_letter, is_whitespace, parse_nat, encode_utf8 |
| Bytes | push, at, length, join, decode_utf8 |
| Nat | divide |
| Rat | nat, dec |
| Duration | ticks |
| Instant | add, difference, is_before |

Callable invocation and array map/reduce remain P07. Source core methods and
Path methods exposing canonical error identities remain P08. Constructors and
module services retain their existing roles. Legacy dot sugar and global primitive
entry points remain until P12; this packet enables their owned replacements.

## Acceptance evidence

The binding suite checks all 30 method signatures, canonical operation metadata,
fully checked generic callers and retained bound-call targets. Rejection cases
cover wrong keys/items/arguments, missing owners/methods, explicit type arguments,
guarded-type separation, no global fallback and source/intrinsic duplicates.

The P03 Server/Map family now uses owned Map calls both externally and inside
Server methods, preserving its exact output, alias/re-export/namespace coverage,
independent signatures and single receiver evaluation. Seven new public CLI
failure fixtures check exact diagnostics and absent output artifacts.

The new `owned_intrinsics` complete program runs collection, text, byte, numeric
and time methods through source execution and saved bytecode. The rational CLI
fixture retains all earlier cases and adds owned exact-Nat, non-terminating Dec
and division-by-zero traps for both source and saved bytecode. Existing VM tests
continue to cover the underlying primitive contracts.

Compiler seed refresh starts from P05's
`2f64ce0ad979ec07080fe263ddf546aa53dc454ff3e47e807eac128687158e12`.
Compiler stages 2/3/4 reproduce
`99f7b745472e5361c7cca72cb8bb9afdc1591e82431dd1934d1b3506af484670`.
Standard-library bytecode remains
`eea94d9ea68fcafaa4a68d1eabb926add89f1fc8f3e2d9b6b5877353609c052b`,
with unchanged output.

Full `make check` passed in 485 seconds, including 1,199 binding assertions,
210 parser assertions, 41 compiler-driver assertions, 517 functional assertions,
67 normal identity-audit roots, namespace seed acceptance, release smoke and
quick start. The direct fixture runner also passed all 517 assertions. Self-review
covered the exact source/test diff, synthetic declaration completeness, retained
call identity, constraints and emission. The namespace copy list and functional
selection list explicitly include the new registry and fixture.

Performance: registration and typed metadata add compilation work; execution
reuses existing VM instructions. Whole-suite timing records validation cost,
not a controlled compiler benchmark. Historical budget warnings retain their
existing informational status under GI#106.

Documentation reviewed: SPEC, architecture, self-hosting, compiler README,
coverage, packet handoff and roadmap. Root README commands and released alpha.13
claims remain accurate. Website: the published article and alpha.14 plans already
identify this migration as upcoming; no released-feature or download change is
needed for this intermediate packet.

## Programme handoff

P01–P06 are complete on acceptance; P07 callable and effect contracts is next.
GI#525 remains open. Stable stage weights are unchanged. Intrinsic registration,
checking/emission and Server/Map acceptance earn an estimated half of the built-in
stage; source core APIs and canonical error identities remain. Overall weighted
completion rises from 16% to **26%**. This is a scope estimate, not completed-packet
counting; P07 and P09 receive no credit for this packet's tests or seed refresh.

| Stage / packets | Status on acceptance | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | P02–P05 done; P07 next | 20% | 55% | 11 |
| Built-ins: P06, P08 | P06 done; P08 unstarted | 20% | 50% | 10 |
| Stdlib: P10 | Unstarted | 15% | 0% | 0 |
| Compiler/source: P09, P11 | Unstarted | 20% | 0% | 0 |
| Legacy removal: P12 | Unstarted | 10% | 0% | 0 |
| Dot cutover: P13–P14 | Unstarted | 5% | 0% | 0 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| Total | | 100% | | 26% |

P07 covers callable/purity/async acceptance; P08 source core APIs and error
identity; P09 the audited arrow seed checkpoint; P10 stdlib migration; P11 compiler
and other callers; P12 legacy deletion; P13 the owned-dot bridge; P14 dot conversion
and arrow removal; P15 release/consumer acceptance. These remain undelivered.
