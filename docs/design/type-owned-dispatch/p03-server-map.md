# P03: Server and Map coexistence

GI#525 P03 builds on `7ade6efe6e0e195adbd2469eb47d7fd889b5b599` (P02).
This packet is complete on integration after final-head checks pass.

## What the experiment establishes

`tests/functional/cases/owned_server_map/` is a manifest-backed project using the
current toolchain requirement. Its Server owns `get(route, fallback)` and
`put(route, amount)`, and stores a Map internally. Server calls use arrow syntax;
Map calls use existing dots, exactly as required for P03. Four entry points
exercise selective, namespace, renamed and facade-re-exported imports using the
same Server implementation and the same expected stdout:

```text
11
21
15
25
7
7
9
receiver evaluated once
31
901
902
```

The values prove correct receiver binding for two instances, persistent Server
and Map updates, repeatable Map lookup, chained receiver evaluation once and
independence from ordinary global get/put functions. A fifth entry point imports
an unrelated module's Server with different get/put signatures; its exact output
is 11/1042/11/1040, proving each type selects its own implementation.

Compiler unit tests inspect checked call targets, argument counts, status and
return types: Server methods retain canonical OwnedMethodIdentity; Map calls
retain their current core-operation identity. Server.put returns Server while
Map.put returns Map[Str,Nat]. Every new executable case runs through the public
source and saved-bytecode paths. The base project also has explicit --project
run/compile acceptance.

## Negative cases and diagnostic correction

Eight public CLI failure fixtures implement F02-F08 and F10: Server get arity,
Map get arity, Server put argument type, no global fallback, private method through
a facade, duplicate methods across impl blocks with differing signatures,
foreign-owner impl and the purity boundary. They assert check/compile rejection,
source positions, exact semantic messages and no emitted artifact, using the
existing harness conventions. Plausible same-name globals in the arity/type
fixtures must not rescue invalid receiver calls.

Current arity messages count the implicit receiver. Thus Server.get expects
3 total arguments and rejects 2 (two explicit versus one); Map.get expects 2 and
rejects 3 (one explicit versus two). F08 is rejected during parsing under the
bounded earlier-local-record rule. F02-F07 fail during binding/signature checking
and F10 during effect checking; none reaches runtime.

The project-aware Map arity check exposed an existing diagnostic interpolation
bug: it printed `${len(children)}` literally. The only compiler source change
uses the existing number renderer for the shared collection-arity diagnostic.
A focused unit assertion and the Map failure fixture cover it. No method lookup,
Map runtime semantics, inference or language syntax changed. The seed is refreshed
so the public CLI reports the corrected count.

F09 (built-in ownership/spoofing), F11 (generic receiver inference), F12 (legacy
dot removal) and F13 (arrow removal) remain with their respective later packets.
P06 will change the fixture's Map calls to owned arrows; P14 will change all owned
calls to dots while preserving the expected outputs. Actual HTTP handler effects,
async signatures and networking remain unverified by this numeric experiment.

## Validation

Seed refresh passed compiler stages 2/3/4 at
`41456b74fb7535d0521e90d64ac1d562668aa8ad9632d5000497b2008bc186ed`,
from input seed `6a9eba9e4610bc11b02a1229df20f89b6ba745be2b867ca0c95138311e7ded21`.
Stdlib bytecode remains
`eea94d9ea68fcafaa4a68d1eabb926add89f1fc8f3e2d9b6b5877353609c052b`;
its expected output passes in all stages.

Full `make check` passed in 599 seconds: unit phase 448 seconds, bootstrap
269 seconds, functional phase 107 seconds, release smoke 9 seconds and quick
start 1 second (phases overlap). Compiler binding tests passed 870 assertions;
functional tests passed 439 assertions, with zero failures. Focused acceptance
also passed all five source and saved-bytecode outputs and explicit manifest
execution. `make docs` and `git diff --check` passed. Historical timing-budget
warnings are informational under GI#106 and are not acceptance failures.

Performance: no successful-program code generation or runtime change. The only
compiler change renders a count on an error path. The fixture suite adds 15
positive source/compile/bytecode assertions and 24 negative assertions. Harness
corpus totals are updated from measured fixture additions, without weakening
existing assertions. Full validation timings are not performance benchmarks.

Documentation review: specification, architecture and README remain accurate;
no syntax, ownership rules or released capabilities changed. Compiler notes,
coverage, self-hosting seed evidence and migration handoff are updated. Website:
no change to current alpha.13 claims.

## Programme handoff

P01, P02 and P03 are complete on acceptance. P04, generic nominal owners, is next.
P04-P15 remain unstarted; GI#525 stays open. The general-method stage gains five
percentage points of its own completion for this bounded regression gate, from
10% to 15%; the eight-stage weights remain unchanged. Overall estimated earned
progress rises from 7 to 8 points, not from an equal count of packets.

| Stage | Weight | Completion on acceptance | Earned contribution |
| --- | ---: | ---: | ---: |
| Inventory and ownership | 5% | 100% | 5 points |
| General method model | 20% | 15% | 3 points |
| Built-in owned implementations | 20% | 0% | 0 points |
| Stdlib migration | 15% | 0% | 0 points |
| Compiler and source migration | 20% | 0% | 0 points |
| Legacy removal | 10% | 0% | 0 points |
| Dot cutover | 5% | 0% | 0 points |
| Release and ecosystem acceptance | 5% | 0% | 0 points |
| Total | 100% | | 8 points |

Packet register: P01 inventory, P02 metadata and P03 coexistence complete on
acceptance; P04 generics, P05 core registry, P06 intrinsics, P07 effects, P08 core
sources, P09 seed checkpoint, P10 stdlib, P11 callers, P12 deletion, P13 dot bridge,
P14 dot conversion and P15 release acceptance remain unstarted.
