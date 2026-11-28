# Alpha.16 release acceptance

Version: `0.1.0-alpha.16`. Prepared 28 November 2026.
Status: native packages, installer and browser package published and verified; website acceptance and publication pending.
Browser runtime bundle preflight found and corrected omitted console-read and State source/header entries. The bundle now passes its transitive-header integrity check.

Source baseline: `90b2ec5f473c91c23acb593da7b61bd3ec968aec` on next.
Release preparation: [PR #710](https://github.com/sproates/panackelty/pull/710),
with the implementation/preparation tree at `b53bb26`. The owner authorised
release CI and merge when green on 28 November. The merged source is `5e9fc13eccec0ed4c77db8ac0f961e07533cf3e4`; its files
exactly match the validated PR head. [Release workflow 46098177472](https://github.com/sproates/panackelty/actions/runs/46098177472)
succeeded for that commit. The public release contains the exact tested assets.

Scope: delivered VM/compiler optimisations and incremental compilation,
ordinary/async stdlib, anonymous functions, immutable closures, explicit State,
examples and documentation. All unfinished allocations move to alpha.17;
existing future release allocations shift one release later.

Supported native targets remain Ubuntu 22.04+ x86-64 and macOS 15+ arm64.
Bytecode remains v9. The default installer now selects alpha.16 after public archive acceptance. Browser compiler/VM/stdlib inputs move together;
HTTP/native host features are not promised in the browser. Website publication
is the final delivery step. Existing implementation evidence is preserved in
[the implementation snapshot](../ALPHA16_IMPLEMENTATION_STATUS.md).

## Gate evidence

- Native candidate: final local `make check` passed in 761 seconds, including unit, bootstrap, functional, distribution and release smoke checks. Initial preflight caught stale alpha.15 toolchain requirements in bundled manifests; these were updated and lockfiles regenerated with `panack deps update`. All eight focused distribution checks also passed with local networking enabled.
- Hosted candidate: all applicable initial jobs passed except the macOS compiler harness, where launching a subprocess to resolve a fixture path returned `launch_failed`. The same test passed locally and on Linux. One unchanged-source retry with debug logging passed in [Check run 46095771171](https://github.com/sproates/panackelty/actions/runs/46095771171). All required PR checks passed before merge. The original launch cause is not established.
- Public native/runtime bundle: all eight downloaded assets match GitHub digests; both native provenance records identify the release commit. Both platform package jobs passed native conformance and archive smoke tests. The public macOS archive is byte-identical to the retained artifact that also passed local relocation/package smoke acceptance.
- Published-source release validation passed in 1625 seconds on Ubuntu.
- Installer promotion: prepared on `release/alpha16-installer`; all five focused installer harness cases passed, including upgrade/rollback and failure preservation. Final local `make check` passed in 728 seconds; rebasing the promotion onto merged main preserved its exact file tree. Public README and candidate-installer acceptance passed on macOS, including default selection, repeat install, explicit older release and removal. The owner approved final hosted checks and merge of PR #711 when green on 28 November. All hosted gates passed in workflow 46102883316, including public Linux/macOS installer acceptance. PR #711 merged as `2fa1da4`; the downloaded public-main installer exactly matches the accepted bytes.
- Browser local compatibility: 29 runtime/asset/release tests and 30 real-browser checks passed across Chromium, Firefox and WebKit using the candidate VM/compiler/stdlib. Browser PR [#16](https://github.com/sproates/panackelty-browser/pull/16), commit `475b8a2296d1dc3011c6cff4ca0e3281825e5d0d`, pins the verified public bundle and source. The public bundle is byte-identical to the tested candidate; rebuilding from it passed all 29 tests and the updated footer checks in all three browsers. Hosted checks passed and browser v0.4.1 was published by workflow 46106558913 after PRs #16–#19. Public SHA-256 is `d5ccc8fef6cfe5089232a9d7de7ebe48495800bd88e680289c15b6762ea589e4`; asset identity is `ad161a9f8ce8ebd0239bac192feda66670366032337cc294549ef68456982aea`. Compiler seed, core manifest and all artifact hashes were verified. PR #17 corrected a squash body containing inherited CI-skip directives; no product changes were needed.
- Website PR [#81](https://github.com/sproates/panackelty-website/pull/81): 56 website tests and assembly checks passed; 30 playground scenarios passed through Panack HTTP server. Public v0.4.1 assets match tested code; differing gzip bytes decode to the identical compiler seed. Six final public-artifact worker/State checks passed across all three engines, and the public phone preview executed successfully. Owner approval, hosted checks, deployment and live verification remain.
- Preview compatibility: direct WASI imports avoid unused modules, and website assembly embeds unchanged stylesheet contents and omits an unused clipboard script. All scenarios remain covered; the bounded preview server is used with one worker and fresh editorial servers per engine. Browser PR #19 adds manual publication recovery after no push run appeared for PR #18; manual execution preserves the full validation pipeline.

## Public artifact hashes

| Artifact | SHA-256 |
| --- | --- |
| Linux x86-64 | `d72e2f3f0dde60a990ff8777877962e6c4a6558041ad15b258d23511b8782531` |
| macOS arm64 | `294bad030de25151c904f5100e38f370459d9730fd60df49c7cec92c60459cd9` |
| Browser runtime bundle | `682565019663ad5ceb2394f3a1d90dfc144e2636ce9bfb2fa03b5ef845d2322b` |

## Governing checklist

This is the governing sequence for completing a Panackelty release across the
core, browser and website repositories. The individual repositories' procedures
provide implementation details; completing one repository's procedure does not
complete the release.

**Publishing the website is the final delivery step.** Do not announce the new
release on the production website until its packages, default installer and
browser playground are ready, verified and mutually consistent. A published
GitHub release alone is not a completed rollout.

Create a copy of this checklist in the release's acceptance record. Record the
release version, exact source commits, PRs, artifact hashes and verification
results. Mark a box complete only when its evidence exists. Pending checks,
unmerged changes and local previews are not completion evidence.

## 1. Confirm scope and prepare the release

- [x] Reconcile the agreed scope with delivered work, issues and acceptance
      records. Identify every remaining blocker before starting publication.
- [x] Confirm supported platforms, runtime dependencies, compatibility changes,
      migration instructions and the intended browser capabilities.
- [x] Prepare and review the version, changelog and release notes. Distinguish
      current features from historical releases and future work.
- [x] Identify the core, browser and website changes needed for this release.
      Include installer defaults, documentation, examples and all version pins.
- [x] Record the full remaining sequence in `docs/NEXT_RELEASE_STATUS.md` on the
      integration branch. Do not describe browser promotion as optional follow-up
      while calling the overall release complete.

## 2. Publish and verify release packages

Follow [the release policy](RELEASE_POLICY.md#release-gate).

- [x] Complete the required native, compiler, runtime, sanitizer and package
      checks for the exact release source.
- [x] Publish the versioned native archives, checksums and browser runtime
      dependency bundle through the release workflow.
- [x] Download the public artifacts and verify their hashes and source provenance
      against the tested artifacts. Verify installation and execution on every
      supported target platform.
- [x] Confirm public release notes, asset names and download URLs are correct.
      Preserve older release assets and links; do not overwrite releases.

## 3. Update and verify the installer

Follow [installer maintenance](RELEASE_POLICY.md#optional-installer-maintenance).

- [x] Update and merge the default release version, help/examples and affected
      installer tests together, after the public archives are verified.
- [x] Verify the published installer selects the new release without a version
      argument. Check explicit version selection, repeat installation and upgrade
      behaviour using the relevant existing acceptance tests.
- [x] Keep manual installation and supported older-version installation accurate.
      Verify platform requirements and dependencies in installation guidance.

## 4. Build, publish and verify the browser playground package

Follow the [browser publication procedure](https://github.com/sproates/panackelty-browser#website-artifact-publication).

- [x] Update the browser's compiler, VM and standard-library inputs together to
      the verified release bundle. Update browser package version and provenance.
- [x] Resolve compatibility failures and update affected playground examples and
      guidance. Test compilation, execution, diagnostics and relevant new language
      features in the supported browsers.
- [x] Publish the tested browser artifact and verify its public archive, checksum
      and recorded core version. A core browser-runtime bundle is a dependency,
      not a published playground package.
- [x] Prepare the website dependency pin for that exact browser release. Confirm
      the website candidate actually loads and runs the new compiler; changing a
      version label alone is insufficient.

Native-only capabilities need not become browser capabilities. Explain browser
limitations accurately, but do not silently leave the playground on the previous
compiler version. Any exception to this checklist requires an explicit owner
scope decision before website publication, recorded in the acceptance record.
An implementing agent must not invent a deferral to declare completion.

## 5. Prepare and approve the complete website candidate

Follow the [website promotion procedure](https://github.com/sproates/panackelty-website#publish-and-promote-releases).

- [x] Update native downloads, hashes, installer source/version pins, browser
      dependency pins, release notes, installation requirements and examples.
- [x] Audit all current-release and older-release references: homepage, releases
      and roadmap, getting started, capabilities, API/reference pages, explanation
      guide, playground, navigation and articles. Remove stale upcoming/current
      claims, broken documentation paths, incorrect ordering and duplicated
      release sections. Historical descriptions must retain their correct
      version context and link to the intended older release where appropriate.
- [ ] Before changing published articles, tell the owner which articles and what
      changes are proposed. Show the changes in context and obtain review; a
      general website audit is not permission to silently rewrite articles.
- [ ] Validate the assembled website, downloads, executable examples and browser
      integration against the exact candidate. Verify release links and pins.
- [ ] Provide a working phone/desktop preview of the complete candidate and obtain
      the required publication approval. Include the updated playground in that
      preview; do not publish the native promotion first and catch up later.
- [ ] Reconcile release issues and records. The status must say that only website
      publication and live verification remain. Prepare the completion record
      without claiming it has already happened.

## 6. Publish the website — final delivery step

- [ ] Immediately before publication, confirm steps 1–5 are complete for the
      exact candidate and that no package, installer or playground work remains.
- [ ] Merge/publish the approved website candidate through its normal deployment
      workflow. This is the last user-facing delivery action in the release.
- [ ] Verify production deployment identity and bytes, then check the live
      homepage, release history, downloads, installer instructions, updated
      articles/guides and playground. Confirm the live playground reports the new
      compiler version and successfully compiles and runs an example.
- [ ] Record the live verification evidence, reconcile completed issues and update
      the release acceptance record and integration-branch status. Only now report
      the release as complete. Administrative recording must not conceal another
      unfinished delivery step.

If a gate fails, report the exact blocker and remaining work. Do not publish the
website to make partial progress appear complete. If failure is found after
publication, report the rollout as incomplete and fix or roll back the affected
promotion through the existing procedures.
