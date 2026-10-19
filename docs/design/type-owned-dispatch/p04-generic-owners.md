# P04: Generic nominal owners and method parameters

GI#525 P04 builds on `1d829866695dc1e63f87c9c12760f04ca271e8db` (P03).
This packet is complete on integration after exact-head checks pass.

## Delivered scope

`impl[T] Box[T]` and generic enum owners use the existing canonical nominal
identity. Implementation parameters remain explicit metadata, separately from
method parameters such as `replace[U]`. Uniform owner arguments may rename or
reorder binders. Ownership is checked after declaration collection, allowing an
impl before its record/enum; imported types and specialised impls are rejected.
Implementation headers are retained even for empty blocks, so semicolon-only
blocks and reserved binders cannot bypass signature validation.
Duplicate/unused implementation binders, duplicate or shadowing method binders,
explicit self parameters and assignment to self are rejected. Existing duplicate
method, public-signature accessibility, privacy and purity checks remain active.

The receiver is bound once. Its known arguments fix the implementation parameters; method arguments cannot
change those fixed types. Holes inside an empty constructor can be constrained by
method arguments, including when method parameters are explicit. Explicit type arguments supply
only method parameters. Existing constraint solving supports expected-result
inference and holes inside known nominal receivers. Remaining unresolved type
arguments fail the identity-emission gate; there is no owner guessing or global
fallback. Checked direct calls and the existing bytecode format are unchanged.

Canonical Option and Result support this mechanism in their defining toolchain
module. An isolated core fixture tests methods with the same name and a private
helper. This exposed and corrects the loader's old staging-marker gate, which
otherwise skipped entry loading when core contained owned declarations. Only the
known namespace staging marker is ignored at that gate; real errors still fail.
Core owned methods now use checked emission; existing core helpers retain their
bootstrap lowering. Production stdlib APIs are not migrated by this packet.

The `$owned$Owner$method` bridge remains as allowed by P02. Built-in owner
registration, collection intrinsics, callbacks/async and the dot cutover remain
later packets. No HTTP networking is implemented.

## Acceptance and validation

`owned_generic_methods` has exact stdout:

```text
42
0
0
1
chosen
text
hello
explicit
7
9
```

It covers Box[Nat]/Box[Str], namespace construction, a renamed facade import,
inferred and explicit method binders, expected-result inference, empty receiver
arguments and enum methods with reordered implementation binders. The existing
runner executes source, compile and saved-bytecode assertions. Eleven failure
fixtures assert check/compile rejection, exact diagnostics and no output artifact.
Existing P03 Server/Map cases remain unchanged except the foreign-owner diagnostic
now comes from signature checking instead of the parser.

The compiler-driver fixture supplies a separate test core and checks exact
`7/9/yes/no/9/7` output through the native compiler driver and saved bytecode through `panack`. The
launcher continues to pin its bundled stdlib. Binding tests check privacy,
canonical imports, invalid signatures, inferred types and checked status. Parser
tests inspect implementation metadata and owner spans.

Focused probes passed 899 binding assertions and 209 parser assertions. The
compiler-driver suite passed 40 assertions, including the three isolated core
source/compile/bytecode checks. The final seed refresh reproduces
`b97ffe6165ef9037b1aa27a3d6225f958afa21129aadb06eebcd9dd4e5798b6d`
in compiler stages 2/3/4; stdlib bytecode remains
`eea94d9ea68fcafaa4a68d1eabb926add89f1fc8f3e2d9b6b5877353609c052b`
and its expected output passes. No production stdlib source changed.

Final canonical `make check` passed (exit 0) in 466 seconds: unit 308 seconds,
bootstrap 270 seconds in parallel, functional 113 seconds, release smoke 10 seconds
and quick start under one second. It passed 899 binding assertions, 209 parser
assertions, 40 compiler-driver assertions, 475 functional assertions and all 66
normal identity-audit roots. The final run reused valid compilation caches but
executed the assertions; it is not a clean-build performance comparison. Earlier
runs exposed stale harness summary totals and a missing positive-case selection;
all were corrected without removing assertions. The custom-core fixture uses a
dedicated entry filename because normal main roots are audited against the
production stdlib. `make docs` and `git diff --check` also pass. Hosted checks must
still pass for the exact PR head before integration.

Performance: runtime dispatch still uses ordinary direct calls and evaluates the
receiver once; no runtime registry or new opcode is introduced. New generic work
occurs during compilation. Ordinary function calls avoid the method metadata scan.
The new corpus adds 36 runner assertions and three compiler-driver CLI assertions.
Whole-suite timing is validation evidence, not a controlled performance benchmark.
Historical validation-budget warnings remain informational under GI#106.

Documentation reviewed: specification, architecture, self-hosting, compiler notes,
coverage and roadmap. README commands and released alpha.13 claims remain accurate.
Website: no change to the site's pinned alpha.13 contract; this is unreleased
compiler work, with no production stdlib method migration or release promotion.

## Programme handoff

P01–P04 are complete on acceptance. P05, the built-in ownership registry, is next;
GI#525 stays open. The original eight-stage weights are unchanged. P04 advances
the general-method stage from 15% to 40% for generic owners, binders and inference;
built-in ownership and callable/effect acceptance remain. Overall estimated
completion rises from 8 to **13 points**. This is scope-based estimated credit,
not acceptance of the entire general-method stage.

| Stage / packets | Weight | Completion on acceptance | Earned contribution |
| --- | ---: | ---: | ---: |
| Inventory and ownership (P01) | 5% | 100% | 5 points |
| General method model (P02–P05, P07) | 20% | 40% | 8 points |
| Built-in owned implementations (P06, P08) | 20% | 0% | 0 points |
| Stdlib migration (P10) | 15% | 0% | 0 points |
| Compiler and source migration (P09, P11) | 20% | 0% | 0 points |
| Legacy removal (P12) | 10% | 0% | 0 points |
| Dot cutover (P13–P14) | 5% | 0% | 0 points |
| Release and ecosystem acceptance (P15) | 5% | 0% | 0 points |
| Total | 100% | | 13 points |

| Packet | State on acceptance | Evidence / remaining gate |
| --- | --- | --- |
| P01 Inventory | Complete | PR#526 |
| P02 Ownership metadata | Complete | PR#527 |
| P03 Server/Map coexistence | Complete | PR#528 |
| P04 Generic nominal owners | Complete | This report and executable fixtures |
| P05 Built-in ownership registry | Unstarted | Canonical core registry and spoof rejection |
| P06 Owned intrinsics | Unstarted | Collection/text/numeric dispatch |
| P07 Callbacks and effects | Unstarted | Callable, purity and async acceptance |
| P08 Core methods and error identity | Unstarted | Pre-conversion seed and source migration |
| P09 Arrow seed checkpoint | Unstarted | Expanded arrow fixed point |
| P10 Standard library | Unstarted | Receiver API and caller migration |
| P11 Compiler and remaining callers | Unstarted | Full source migration |
| P12 Legacy deletion | Unstarted | Remove fallback/global aliases |
| P13 Owned-dot bridge | Unstarted | Shared resolver and seed |
| P14 Dot conversion | Unstarted | Convert callers; remove arrow |
| P15 Release/consumers | Unstarted | Matching artifacts and deletion audit |

Packet evidence is allocated within the stage weights above, not counted a second
time. P04's seed refresh does not complete the later P09 expanded-arrow checkpoint.
