# Next release status

Updated: 28 November 2026. Release: **v0.1.0-alpha.16**. Integration branch: **next**.
Latest published release: **v0.1.0-alpha.15**. Alpha.16 is **in release preparation**, not yet published.

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
| Exact-source native release validation and public packages | Pending. |
| Default installer promotion and public-artifact acceptance | Pending verified new packages. |
| Matching browser playground package | Local candidate passes 29 runtime/asset/release tests and 30 Chromium/Firefox/WebKit checks. Final public bundle pins, hosted checks and browser publication remain. |
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
