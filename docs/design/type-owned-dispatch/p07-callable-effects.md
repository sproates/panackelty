# P07: Callable and effect contracts

GI#525 P07 builds on integrated P06 and nightly repair `1890860`. Completion is
gated by validation and exact-head CI, taking effect on integration.

## Delivered scope

The designated core registry now supplies 35 intrinsic declarations. The five new
identities are Array.map, Array.reduce and PureFn/Fn/AsyncFn.call. Callable schemas
are specialised from the checked receiver, including arbitrary argument arity
and trailing result type. The three owners preserve their distinct effects;
source code cannot declare variadic callable implementations. Source impl methods
now retain async flags and follow ordinary await/effect restrictions.

Lookup precedes intrinsic checking and lowering. Bound calls retain canonical
owned declaration IDs. No new VM operations, callback variance, closures, bound
method references or HTTP runtime implementation are introduced. Array callbacks
remain pure. Legacy dot/global APIs remain until their planned removal in P12.

## Acceptance

Binding tests exercise every registry entry and retained identity, callable
arguments, pure/ordinary/async effects, array callback contracts and async methods.
The owned_callbacks program checks output and single receiver evaluation. The
owned_async_handlers program stores and invokes the GI#518 contracts:
`AsyncFn[Request,ResponseWriter,Result[Unit,Str]]` and
`AsyncFn[T,Request,ResponseWriter,Result[Unit,Str]]`. Registration-shaped methods
return Unit; execution awaits a genuinely suspending service and checks results.
These are language fixtures, not production HTTP registration or networking.

Focused binding validation passed 1,239 assertions. Both new public CLI fixtures
passed source execution and saved-bytecode execution. Seed refresh reproduced
`57ee20ce5916b59e85fb23db0a286c3e372124617489e09e342d8b0d50a1152c`
across compiler stages 2/3/4. Standard-library bytecode remains
`eea94d9ea68fcafaa4a68d1eabb926add89f1fc8f3e2d9b6b5877353609c052b`.
Full `make check` passed in 487 seconds after correcting the fixture runner's
expected count/transcript for the two added programs: 1,239 binding assertions,
523 functional assertions, bootstrap fixed point, namespace seed, release smoke
and quick start. Historical timing-budget warnings remain informational under
GI#106; no deadline was changed. Self-review covered the source/test diff,
canonical family effects, retained call identity and async declaration flags.

Performance: runtime uses existing callable and collection lowering. The registry
adds five entries during module checking; no controlled compiler speedup is
claimed. Required full validation measures delivery cost. Website: existing
alpha.14 plans remain accurate; no released-feature change is needed. Root README
commands remain unchanged. Source-coverage baseline scope is unchanged.

## Programme handoff

P01–P07 complete on integration; P08 is next. GI#525 remains open. Completing the
method-model stage raises the weighted estimate from 26% to 35%, with unchanged
weights; built-in source migration remains outstanding.

| Stage / packets | Status on acceptance | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | Done | 20% | 100% | 20 |
| Built-ins: P06, P08 | P06 done; P08 next | 20% | 50% | 10 |
| Stdlib: P10 | Unstarted | 15% | 0% | 0 |
| Compiler/source: P09, P11 | Unstarted | 20% | 0% | 0 |
| Legacy removal: P12 | Unstarted | 10% | 0% | 0 |
| Dot cutover: P13–P14 | Unstarted | 5% | 0% | 0 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| Total | | 100% | | 35% |

P08 must refresh a seed supporting new declarations before migrating core source;
P09 remains a separate expanded-arrow checkpoint. No later packet is started.
