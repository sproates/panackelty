# Next release status

Updated: 28 November 2026. Release: **v0.1.0-alpha.16**. Integration branch: **next**.
Native release: **v0.1.0-alpha.16**, published and verified. The overall rollout is **in progress**: installer promotion, the matching browser package and final website publication remain.

The owner closed alpha.16 scope around completed work on 28 November. The
[release acceptance checklist](releases/ALPHA16_RELEASE_ACCEPTANCE.md) owns the
remaining delivery gates. The [implementation snapshot](ALPHA16_IMPLEMENTATION_STATUS.md)
preserves detailed test and integration evidence.

## Alpha.16 scope

| Work | State |
| --- | --- |
| VM optimisation batches | Accepted and integrated; bytecode remains v9. |
| Compiler optimisations and persistent incremental compilation | Accepted and integrated. |
| Ordinary/async stdlib redesign and documentation (RM#162, AS1–AS5) | Accepted, 100/100; website PR #77 published. |
| Anonymous functions, closures, State and combined examples (RM#162, CS1–CS4) | Accepted, 100/100; core PRs #701–#709 and website PR #80. |
| Exact-source native release validation and public packages | PR #710 merged after local validation (761 seconds) and all hosted gates passed, including one unchanged-source macOS retry. Release workflow 46098177472 succeeded. All eight public assets match GitHub digests and release provenance; both target package jobs and macOS public acceptance passed. |
| Default installer promotion and public-artifact acceptance | Promotion branch prepared; five focused cases and full local validation (728 seconds) passed. Public macOS README/installer acceptance passed. PR #711 requires approval and hosted checks before promotion. |
| Matching browser playground package | PR [#16](https://github.com/sproates/panackelty-browser/pull/16) pins the verified public bundle. Local validation passed 29 runtime/asset/release tests and 30 Chromium/Firefox/WebKit checks; hosted checks and publication remain. |
| Website promotion and live verification | Last delivery step; pending all prior gates and candidate preview approval. |

## Following allocations

All unfinished alpha.16 work is explicitly allocated to **alpha.17**: GI#166
Typed HTTP exchanges; GI#167 Recursive file discovery; GI#168 Subprocess examples;
GI#169 Tiny build tool; GI#138/GI#310 Tested guides and examples; GI#170 Inference
and diagnostics assessment; remaining RM#157 Performance review; GI#435 Guard
alternatives; native-interface/compiler decoupling; coverage approach review;
and continuing RM#162 Real-program fixes. Completed RM#157/RM#162 batches remain
credited to alpha.16. GI#552 Full API reference and GI#553 Load profiling remain
candidates, not extra commitments.

Previous alpha.17 scope is now **alpha.18**, previous alpha.18 is **alpha.19**,
and previous alpha.19 is **alpha.20**. See the [allocation table](roadmap/records/next-release--agreed-scope-2026-10-06.md#release-boundary-agreed--28-november-2026).
No new feature implementation is needed to satisfy the selected alpha.16 scope;
release compatibility defects, if found, remain release blockers until resolved.
