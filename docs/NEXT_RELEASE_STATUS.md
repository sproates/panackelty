# Next release status

Updated: 28 November 2026. Release: **v0.1.0-alpha.16**. Integration branch: **next**.
Native release: **v0.1.0-alpha.16**, published and verified. The default installer and matching browser v0.4.1 are also published and verified. The complete rollout is finished: website publication and live verification passed.

The owner closed alpha.16 scope around completed work on 28 November. The
[release acceptance checklist](releases/ALPHA16_RELEASE_ACCEPTANCE.md) records the
completed delivery gates. The [implementation snapshot](ALPHA16_IMPLEMENTATION_STATUS.md)
preserves detailed test and integration evidence.

## Alpha.16 scope

| Work | State |
| --- | --- |
| VM optimisation batches | Accepted and integrated; bytecode remains v9. |
| Compiler optimisations and persistent incremental compilation | Accepted and integrated. |
| Ordinary/async stdlib redesign and documentation (RM#162, AS1–AS5) | Accepted, 100/100; website PR #77 published. |
| Anonymous functions, closures, State and combined examples (RM#162, CS1–CS4) | Accepted, 100/100; core PRs #701–#709 and website PR #80. |
| Exact-source native release validation and public packages | PR #710 merged after local validation (761 seconds) and all hosted gates passed, including one unchanged-source macOS retry. Release workflow 46098177472 succeeded. All eight public assets match GitHub digests and release provenance; both target package jobs and macOS public acceptance passed. |
| Default installer promotion and public-artifact acceptance | PR #711 merged as `2fa1da4` after every hosted gate passed, including Linux and macOS public installer acceptance. The public main installer is byte-identical to the tested script and defaults to alpha.16. |
| Matching browser playground package | Browser v0.4.1 is published after PRs #16–#19 and workflow 46106558913. Its public archive, asset digests, compiler seed and core manifest match the verified alpha.16 inputs. Runtime contracts and all three browser engines passed. |
| Website promotion and live verification | Website PR [#81](https://github.com/sproates/panackelty-website/pull/81) merged as `0ced1655`. Exact-head checks 46125684113 and publication 46126200959 passed. Live bytes/provenance, pages and alpha.16 playground execution are verified; the closure/State example returned `1`, `2`, `2`. The release is complete. |

## Following allocations

All unfinished alpha.16 work is explicitly allocated to **alpha.17**: GI#166
Typed HTTP exchanges; GI#167 Recursive file discovery; GI#168 Subprocess examples;
GI#169 Tiny build tool; GI#138/GI#310 Tested guides and examples; GI#170 Inference
and diagnostics assessment; remaining RM#157 Performance review; GI#435 Guard
alternatives; native-interface/compiler decoupling; coverage approach review;
continuing RM#162 Real-program fixes; and [RM#163: Install and run usability](roadmap/records/install-and-run-usability.md).
RM#163 is planned, with implementation not started: simplify installation, project
creation and running third-party programs, retaining locks and reproducibility.
The clean-install HTTP status service is its acceptance case. Completed RM#157/RM#162 batches remain
credited to alpha.16. GI#552 Full API reference and GI#553 Load profiling remain
candidates, not extra commitments.

Previous alpha.17 scope is now **alpha.18**, previous alpha.18 is **alpha.19**,
and previous alpha.19 is **alpha.20**. See the [allocation table](roadmap/records/next-release--agreed-scope-2026-10-06.md#release-boundary-agreed--28-november-2026).
No new feature implementation is needed to satisfy the selected alpha.16 scope;
release compatibility defects, if found, remain release blockers until resolved.
