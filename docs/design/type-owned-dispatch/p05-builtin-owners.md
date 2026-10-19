# P05: Built-in ownership registry and spoofing rejection

GI#525 P05 builds on `0cb8cb7f3e996182e32262384fdc60f0696bb88c`.
The packet is complete on integration after exact-head checks pass.

## Delivered scope

`MethodOwnerIdentity` distinguishes nominal declaration identity from a canonical
built-in family tag. The closed registry covers Nat, Int, Dec, Rat, Bool, Unit,
Str, Bytes, Path, Duration, Instant, Array, Map and Set, with arities 0, 1 or 2.
These tags use the existing SignatureCore family names, never rendered receiver
types or import aliases. Option and Result retain their nominal core owners;
callable ownership/effects remain P07.

Only `ModuleIdentity(ToolchainModulePackage(), "core.panack")` may declare a
built-in implementation. The existing loader assigns package identity from the
configured toolchain stdlib root, independently of manifest package names.
Signature checking enforces this boundary even for empty implementation blocks.
Uniform binders, privacy, purity, implicit immutable self and duplicate checks
remain active. Array spellings `[T]` and `Array[T]` share the canonical Array key.

Lookup selects the designated core module and verifies the method's retained
owner metadata. It cannot resolve a built-in method as a nominal declaration or
borrow a same-named global function. Guarded nominal types do not inherit built-in
methods. Existing receiver/method argument inference and checked direct-call
emission work for registered source methods, without new VM instructions.

P05 does not register intrinsic methods or migrate production APIs. P06 supplies
owned intrinsic dispatch; P08 migrates core source methods. Existing dot sugar
and the P02 internal function-name bridge remain deliberately unchanged. No HTTP
runtime or networking work is included.

## Acceptance evidence

The binding suite covers every registered family: core signatures and bodies,
checked caller status, canonical metadata, nominal/built-in separation, privacy,
and standalone/manifest/wrong-toolchain-module spoof rejection. Further tests
cover the closed registry, invalid and specialised headers, duplicate array
spellings, global fallback and guarded-type separation.

Six `owned_builtin_*` functional fixtures assert check/compile rejection, exact
diagnostics and no output artifact. The spoof fixture includes a manifest named
`toolchain` and `src/core.panack`; a compiler-driver assertion additionally runs
it through the public `--project` CLI to exercise real loader package identity.

The existing isolated-core fixture now executes Map, Set, array, Str and Bytes
source methods, a generic method and a private core helper. Native-driver source
execution, compilation and public saved-bytecode execution assert exact stdout:

```text
7
9
yes
no
9
7
42
selected
true
9
text
5
```

The seed refresh starts from P04's
`b97ffe6165ef9037b1aa27a3d6225f958afa21129aadb06eebcd9dd4e5798b6d`
and reproduces
`2f64ce0ad979ec07080fe263ddf546aa53dc454ff3e47e807eac128687158e12`
in stages 2/3/4. Standard-library bytecode remains
`eea94d9ea68fcafaa4a68d1eabb926add89f1fc8f3e2d9b6b5877353609c052b`,
with unchanged expected output. Full `make check` passed in 677 seconds, including
1,067 binding assertions, 210 parser assertions, 41 compiler-driver assertions,
493 functional assertions and 66 normal identity-audit roots. Release smoke and
quick-start checks passed. The historical 120-second timing warning remains
informational under the existing GI#106 policy; this is not a controlled
clean-build performance comparison.

Performance: registry and metadata checks occur during compilation. Ordinary
function calls and runtime execution are unchanged; owned source calls remain
direct calls with one receiver evaluation. Whole-suite timing is validation
cost evidence, not a controlled compiler-speed benchmark. Historical timing
warnings remain informational under GI#106.

Documentation reviewed: specification, architecture, self-hosting, compiler
notes, coverage, migration handoff and roadmap. README commands and released
alpha.13 claims remain accurate. Website: no impact on the pinned alpha.13
contract; this is unreleased compiler support without API migration.

## Programme handoff

P01–P05 are complete on acceptance. P06 owned intrinsic dispatch is next; GI#525
remains open. Original stage weights are unchanged. P05 advances the general
method model from 40% to 55% for canonical built-in registration and ownership
acceptance; callable/effect contracts remain. Overall estimated completion rises
from 13% to **16%**, not a count of completed packets. No P06/P08/P09 credit is
claimed for source-method fixture execution or this seed refresh.

| Stage / packets | Status on acceptance | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | P02–P05 done; P07 unstarted | 20% | 55% | 11 |
| Built-ins: P06, P08 | Unstarted; P06 next | 20% | 0% | 0 |
| Stdlib: P10 | Unstarted | 15% | 0% | 0 |
| Compiler/source: P09, P11 | Unstarted | 20% | 0% | 0 |
| Legacy removal: P12 | Unstarted | 10% | 0% | 0 |
| Dot cutover: P13–P14 | Unstarted | 5% | 0% | 0 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| Total | | 100% | | 16% |

P06 supplies collection/text/numeric intrinsic declarations; P07 callable, purity
and async acceptance; P08 source core methods/error identity; P09 the expanded
arrow seed checkpoint; P10 stdlib migration; P11 compiler and other callers;
P12 legacy deletion; P13 the owned-dot bridge; P14 dot conversion/arrow removal;
P15 release and consumer validation. None is started by this packet.
