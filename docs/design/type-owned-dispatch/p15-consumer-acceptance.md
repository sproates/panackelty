# P15: Method-dispatch consumer acceptance

P15 closes GI#525 on integration after the checks and companion browser delivery
below pass. The implementation baseline is P14 core
`539cda1d11bb505761668fc17cbeb03fff4ee84d`. This packet changes acceptance tests
and documentation, not the compiler, VM, library implementation or saved seed.
Native alpha.14 publication and website promotion are separate release work.

## Matching consumers

The native release smoke test now runs all twelve maintained owned-method
fixtures through the relocated installed `bin/panack`: records, imported and
re-exported owners, generics, intrinsics, callbacks, async handlers, core methods
and the five Server/Map variants. Manifest-based cases exercise project builds.
Every source result matches its maintained golden output. All copied fixture
sources are then deleted before their saved bytecode is run and compared again.
The original Fruit program separately produces `10`, `11`, `20`, `21` from source
and saved bytecode. The installed compiler rejects receiver arrows explicitly.
Existing archive contents, relocation, networking and CLI safety checks remain.

[Browser PR#12](https://github.com/sproates/panackelty-browser/pull/12) pins the exact
P14 core revision and builds matching VM, compiler and library assets with WASI
SDK 34.0. It migrates removed global/namespace APIs, updates the native CLI path,
corrects diagnostic expectations, and labels the candidate accurately. The
browser PR merged as `a73c12eb10f0ac2d2a0906724b5152925ba3ecce` after
[exact-head hosted Check](https://github.com/sproates/panackelty-browser/actions/runs/39383034294)
passed for `7262003465967761b2452533e2dbff29e8ceb00d`. The integrated tree matches.
The independently versioned browser artifact is `0.2.0`; it is an alpha.14 consumer
candidate, not publication of native alpha.14.

The [v0.2.0 archive](https://github.com/sproates/panackelty-browser/releases/tag/v0.2.0)
published after [post-merge Check](https://github.com/sproates/panackelty-browser/actions/runs/39383521908).
Downloaded archive SHA-256 is
`faf8196448d768980bb20b407bf529d490e50c872704023f06a88abe24fea616`.
Asset identity is
`4e41d5d60632e6d8848f56ba4c391f1215a9c2fc8715af7b6b049a02de8fc74a`.
Checksum, compiler seed and all 21 library source files match the intended
inputs. Every published file matches its locally tested counterpart byte for
byte. This verifies the public consumer artifact, not merely a local build.

The browser runtime suite passes 23 tests, including all twelve maintained
method fixtures without import flattening or source rewriting. Compiled artifacts
run without source files. It also checks arrow/global/fallback rejection and the
original Fruit output. All 145 VM contracts retain 131 exact results and 14
explicit browser-host rejections. Native/WASI bytecode equivalence remains
asserted. All ten selectable examples match their exact expected output in both
engines. The real-browser suite passes 27 scenarios across Chromium, Firefox and
WebKit, including Fruit, arrow rejection, cancellation, bounds, diagnostics,
cache upgrades and narrow layouts. Resource limits and timeouts are unchanged.
Two consecutive builds have the same complete asset identity.

The website remains pinned to browser `v0.1.3` and native alpha.13 commit
`a2ac16dfb7f7667b8531ddb9e821a35b13644fd5`. Its unchanged source revision
`d00ac9ab3a9c8531fa9c9106d291b07e7f7488b8` passed
[website Check](https://github.com/sproates/panackelty-website/actions/runs/39262842721),
which validates released artifacts/examples and browser integration. This reuses
release-matched evidence, not a claim that old examples run under alpha.14.
The dated method-dispatch article describes the migration in progress and the
future release; its completion update is recorded in the website follow-up
register. No website source or release pin changes in P15.

## Final deletion audit

- No `ValueMethodCall`, `$owned_call$`, or `$method_*` dispatch selection remains
  in compiler source. Missing methods never consult same-named global functions.
- All 32 temporary source wrappers in the P10 register are absent. Wrapper-only
  text, numeric, option and result modules are absent. The maintained
  `owned_dispatch_removal.sh` tests reject removed globals/wrappers and forged
  primitive grants through the public CLI, and assert retained ordinary services.
- Receiver arrows occur only in explicit rejection handling/fixtures or historical
  evidence; maintained executable callers use dots. The P13 fixture converter is
  absent. A separate parser path cannot still accept arrows.
- `$owned$Owner$method` labels remain necessary unique internal function identities
  for declaration/signature/direct-call emission. Explicit ownership metadata
  selects methods; labels do not select methods, expose ordinary source functions
  or provide a second dispatch implementation. The handoff now records this final
  representation decision instead of promising removal of every private label.
- Four `$method_*` VM services remain solely for the existing saved-v9 bytecode
  contract, as documented in P12. The source resolver/emitter cannot select them.
  Removing them would change bytecode compatibility; this packet does not do so.
- Historical inventories, earlier packet reports and intentionally invalid tests
  retain their evidence. Current specification, tour and contributor examples now
  consistently name natural division `Nat.divide`, not the removed global.

The [migration guide](../../guides/METHOD_MIGRATION.md) records the breaking source
requirements and retained constructor/service boundaries. The original plan's
acceptance matrix is covered by canonical unit/functional checks, installed
package tests and the matched browser contracts. HTTP-shaped fixtures establish
language behavior only; actual framework work remains GI#518.

## Validation and performance

Final canonical `make check` passed in 571 seconds on macOS arm64: unit
409 seconds, bootstrap 223 seconds concurrently, functional 109 seconds,
relocated-package release smoke
12 seconds and quick start below one second. All 532 functional assertions, the 70-root identity
audit and source/saved-bytecode network contracts passed. The new migration-guide
program was also extracted and run with exact expected output; documentation/link
checks pass. The local run reused native objects from the focused package check;
browser validation overlapped some of the run, so it is not a matched comparison
with P14’s 459-second run.
P14's compiler seed remains
`d9ebb4911470894ffb9cca33f14ab07f8f855f981da4f36afcaf9376179b7fa5`;
stdlib fixed-point bytecode remains
`4f2904808fe9b6be4633eb54a4022d4216ab4eab2dd5b196677f9155ebaaaba3`.
The exact source diff is self-reviewed. All applicable checks must pass on each
PR's exact head and integrated trees must match before acceptance.

Architecture, compiler/stdlib component contracts and self-hosting documentation
were reviewed; no execution-architecture change is needed.

No compiler/runtime algorithm or resource limit changes. Package verification
adds bounded source/bytecode acceptance work; its first focused run passed in
11 seconds before the separate original-Fruit check was added. Local browser
runtime/assets/publication checks take about 3.4 seconds and the real-engine run
about 1.6 minutes. These are workload observations, not paired performance
benchmarks. GI#106's broader validation-latency reassessment remains open; its
historical thresholds are informational. No coverage or timeout gate is weakened.

## Programme status on acceptance

All fifteen packets are accepted on integration, using unchanged stage weights.
Native release publication, website promotion and the HTTP framework are separate
work and are not represented as delivered by this migration.

| Stage / packets | Status | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | Done | 20% | 100% | 20 |
| Built-ins: P06, P08 | Done | 20% | 100% | 20 |
| Stdlib: P10 | Done | 15% | 100% | 15 |
| Compiler/source: P09, P11 | Done | 20% | 100% | 20 |
| Legacy removal: P12 | Done | 10% | 100% | 10 |
| Dot cutover: P13–P14 | Done | 5% | 100% | 5 |
| Release/consumers: P15 | Done on integration | 5% | 100% | 5 |
| **Total** | | **100%** | | **100%** |
