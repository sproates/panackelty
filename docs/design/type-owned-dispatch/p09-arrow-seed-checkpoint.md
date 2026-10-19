# P09: Audited arrow-capable seed checkpoint

GI#525 P09 audits P08 integration commit
`c60b85efc26fc997138c831f039724d77bfaee93` before the P10/P11 caller migrations.
Completion takes effect on integration after validation and applicable exact-head CI.

## Accepted boundary

The committed seed already supports the expanded owned-arrow language delivered
by P02–P08. P09 verifies that saved artifact against the committed compiler and
library sources. Regeneration produces identical bytes, so the committed seed
and digest remain unchanged. P08's core impls and compatibility forwarding calls are part of
the input. Broader stdlib/compiler caller conversion has not started.

Input and output compiler seed SHA-256:
`c4f6b8b2b3c567a0400353eff7ba4f9f5373caa22027d6f3b4f42bb1f8f1bdfd`.

Standard-library conformance bytecode SHA-256:
`8232a580910c912a04b5e8f62b89d4e174a3d01b0878314c462bf48bac14b216`.

## Reproduction and evidence

From the repository root:

```sh
make regenerate-seed
make check
git diff --exit-code c60b85efc26fc997138c831f039724d77bfaee93 -- src bootstrap
```

The refresh builds compiler stages 2, 3 and 4 without reusing cached bootstrap
artifacts, verifies their bytecode, and compares their hashes. Each stage compiles
and runs stdlib conformance, requiring matching bytecode and exact expected output.
Refresh result: already at the verified fixed point; stages 2/3/4 and conformance outputs match.

The saved seed also passes 27 exact-output public CLI checks across nine existing
fixtures. Each fixture runs from source, compiles to a temporary artifact, then
runs that saved bytecode; all commands must succeed with empty stderr:

| Fixtures | Boundary checked |
| --- | --- |
| owned_method_dispatch, owned_methods | Owner selection and receiver calls |
| owned_imports (project) | Imported owners and visibility |
| owned_server_map (project) | Independent Server/Map dispatch |
| owned_generic_methods | Generic owners, methods and inference |
| owned_intrinsics | Typed built-in dispatch |
| owned_callbacks, owned_async_handlers | Callable effects and async handlers |
| owned_core_methods | Source methods and canonical error identities |

The canonical suite supplies parser, binding, purity, rejection, compatibility,
public CLI, bootstrap and package regressions; no duplicate fixtures or new
language behavior are needed for this audit. Final result: `make check` passed in 707s on 20 October 2026 (Europe/Gibraltar), including
1,249 binding and 532 functional assertions, unit 538s, bootstrap 288s in parallel,
functional 119s, release smoke 10s and quick start 1s.

## Handoff and impact

P09 is complete on integration. P10 is next: migrate standard-library callers and
record the exact remaining compatibility wrappers. P11 then covers compiler,
tests, examples, packages and other maintained callers. Retain P08's documented
forwarders until their users migrate and P12 removes the old surfaces. P13's
owned-dot-capable seed is a later, separate checkpoint. GI#525 remains open.

No compiler, library, VM, bytecode, fixture, timeout or coverage-policy changes are
introduced. Runtime, compile performance and artifact size are unchanged because
the executable inputs and saved seed are byte-identical to P08. Validation times
are audit evidence, not runtime performance measurements; historical budget
warnings remain informational under GI#106.

Documentation review: SPEC, ARCHITECTURE, root/component READMEs and coverage
contracts remain accurate because behavior and test coverage are unchanged.
SELF_HOSTING, ROADMAP and the programme handoff are updated with this checkpoint.
Website impact: none; this audit adds no released feature or changed public claim.

## Programme status

The eight stage weights are unchanged. Within the 20% compiler/source stage,
this bounded bootstrap gate earns an estimated 10% of that stage (2 programme
points); the much larger P11 caller migration remains unstarted. This allocation
reflects scope rather than dividing the two packets equally. Overall estimated
completion rises from 45% to 47%; P10–P15 remain unstarted.

| Stage / packets | Status on acceptance | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | Done | 20% | 100% | 20 |
| Built-ins: P06, P08 | Done | 20% | 100% | 20 |
| Stdlib: P10 | Next; unstarted | 15% | 0% | 0 |
| Compiler/source: P09, P11 | P09 done; P11 unstarted | 20% | 10% | 2 |
| Legacy removal: P12 | Unstarted | 10% | 0% | 0 |
| Dot cutover: P13–P14 | Unstarted | 5% | 0% | 0 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| Total | | 100% | | 47% |
