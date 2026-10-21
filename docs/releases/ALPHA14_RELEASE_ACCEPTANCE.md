# Alpha.14 release acceptance

Native and browser releases are published and verified on 21 October 2026.
Website publication is verified. The installer-default update takes effect when
this follow-up merges after its Linux and macOS acceptance checks.

## Delivered scope

- GI#525: type-owned methods and dot notation, with P01–P15 accepted.
- GI#518: the routed HTTP server framework, with P01–P15 accepted.
- The HTTP slice of GI#552: Server and Client references, the playbook,
  complete examples, request/response transcripts, configuration, lifecycle
  and static serving. The wider standard-library reference remains open.

Coloured diagnostics were not included. Remaining foundation work, including
GI#106's validation-speed milestone, belongs to alpha.15 under the agreed
release rebalance.

## Native publication

[Release 0.1.0-alpha.14](https://github.com/sproates/panackelty/releases/tag/v0.1.0-alpha.14)
uses core commit `3432513b979f1fe1bc57d2bb70ccaa9a62649fd6`.
[Release run 39531324593](https://github.com/sproates/panackelty/actions/runs/39531324593)
passed complete validation, both platform package checks and publication.
All eight public assets match their retained CI artifacts and GitHub digests;
native provenance records identify the release commit. The public macOS archive
also passed local release smoke, including installed source and bytecode examples.
Linux public bytes are identical to the archive executed successfully in CI.

| Artifact | SHA-256 |
| --- | --- |
| Linux x86-64 archive | `20dfee7a88aa99475f25a6a2396a3c5666c4eabb439e8dc78603bcb52afc1aac` |
| macOS arm64 archive | `267fba1b67bcc084e7acb91d01376746da092f814fd5e10141996ef809041964` |
| Browser runtime dependency bundle | `f427fb6c3cbdead8504621ff3ba766d3d37c15f45baf757302762c347c894120` |

Core PR #560 promoted the release source. PR #561 completed the portable runtime
bundle and synchronised installed HTTP smoke checks with the server's readiness
message. The earlier publication run was cancelled before publishing assets.
A deliberately delayed six-second startup passed the corrected smoke test;
the HTTP connection timeout and protocol assertions were unchanged.

Final local validation of those repairs passed in 518 seconds. Release validation
on Linux took 1,533 seconds. Historical timing budgets remain comparisons, not
release gates; these measurements do not complete GI#106 or imply owner acceptance
of validation speed. Runtime behaviour and performance were unchanged by the
packaging and test-synchronisation repairs.

## Browser publication

[Browser v0.2.1](https://github.com/sproates/panackelty-browser/releases/tag/v0.2.1)
uses the same released core commit. Browser PR #13 merged at
`92a55e2e96807eae2e8102c3497c5c0136e3d2ff`.
[Run 39535980282](https://github.com/sproates/panackelty-browser/actions/runs/39535980282)
passed deterministic build, runtime and Chromium/Firefox/WebKit checks before
publishing. Public archive bytes match the retained CI artifact and SHA-256
`2ae810a0d31875581e82a57bfc380db6f79dbc3a1d424ef8aa6bebbd03dcfd06`.
Asset identity is `af1d63da51c3df514a733a0e4aca20b7bab917b40db9bd220fe836f7ee172a3f`.
Local verification also passed all 23 runtime and 27 browser tests.
HTTP networking remains unavailable in this browser profile.

## Website and installer follow-up

Website PR #51 published the verified releases, HTTP documentation and agreed
alpha.15–19 direction. It makes alpha.14 current and moves alpha.13 into Previous
releases. Core and website HTTP Markdown match. Publication followed native and
browser verification. Website commit `4331330b1a43728a71ad5c708242d69d8d4950c6`
passed deployed-byte verification in run 39537834581. Live browser checks
confirmed alpha.14 on the homepage and alpha.13 under Previous releases.

Website CI PR #52 already proved reuse of a green PR artifact on main in run
39520213084. Alpha.14 publication run 39537834581 reused validation run
39537462276, skipping duplicate assembly, release-integrity and browser jobs.
Final local website checks passed all 43 tests and nine cross-browser navigation
checks. Hosted release-integrity and full browser integration passed.

The installer-default follow-up selects alpha.14 only after public verification.
Its local full check passed in 521 seconds. Public macOS installer acceptance
passed installation, the documented example, rollback and uninstall. The follow-up PR requires hosted Linux/macOS installer acceptance before merge.
