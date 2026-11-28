# Alpha.16 release acceptance

Version: `0.1.0-alpha.16`. Prepared 28 November 2026.
Status: release preparation; no alpha.16 public artifacts or announcement yet.
Browser runtime bundle preflight found and corrected omitted console-read and State source/header entries. The bundle now passes its transitive-header integrity check.

Source baseline: `90b2ec5f473c91c23acb593da7b61bd3ec968aec` on next.
Release preparation: [PR #710](https://github.com/sproates/panackelty/pull/710),
with the implementation/preparation tree at `b53bb26`. The owner authorised
release CI and merge when green on 28 November. Published source commit and
artifact hashes remain pending.

Scope: delivered VM/compiler optimisations and incremental compilation,
ordinary/async stdlib, anonymous functions, immutable closures, explicit State,
examples and documentation. All unfinished allocations move to alpha.17;
existing future release allocations shift one release later.

Supported native targets remain Ubuntu 22.04+ x86-64 and macOS 15+ arm64.
Bytecode remains v9. Installer defaults stay on verified alpha.15 until alpha.16
archives pass public acceptance. Browser compiler/VM/stdlib inputs move together;
HTTP/native host features are not promised in the browser. Website publication
is the final delivery step. Existing implementation evidence is preserved in
[the implementation snapshot](../ALPHA16_IMPLEMENTATION_STATUS.md).

## Gate evidence

- Native source and sanitizer checks: pending final candidate. Initial local validation passed compiler/unit cases and fixed-point bootstrap, then caught stale alpha.15 toolchain requirements in bundled manifests. All maintained manifests are updated and their lockfiles regenerated with `panack deps update`. All eight focused distribution checks passed with local networking enabled, including archive relocation, installed imports and compiler artifacts. Final `make check` is running; hosted native and sanitizer gates remain pending.
- Public native/runtime bundle hashes and target installation tests: pending.
- Installer promotion: pending.
- Browser local compatibility: 29 runtime/asset/release tests and 30 real-browser checks passed across Chromium, Firefox and WebKit using the candidate VM/compiler/stdlib. Browser branch `release/alpha16-playground`, commit `30b0c16`, still needs the final public bundle/source/checksum pins and hosted publication.
- Website preview approval, deployment and live verification: pending.

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

- [ ] Reconcile the agreed scope with delivered work, issues and acceptance
      records. Identify every remaining blocker before starting publication.
- [ ] Confirm supported platforms, runtime dependencies, compatibility changes,
      migration instructions and the intended browser capabilities.
- [ ] Prepare and review the version, changelog and release notes. Distinguish
      current features from historical releases and future work.
- [ ] Identify the core, browser and website changes needed for this release.
      Include installer defaults, documentation, examples and all version pins.
- [ ] Record the full remaining sequence in `docs/NEXT_RELEASE_STATUS.md` on the
      integration branch. Do not describe browser promotion as optional follow-up
      while calling the overall release complete.

## 2. Publish and verify release packages

Follow [the release policy](RELEASE_POLICY.md#release-gate).

- [ ] Complete the required native, compiler, runtime, sanitizer and package
      checks for the exact release source.
- [ ] Publish the versioned native archives, checksums and browser runtime
      dependency bundle through the release workflow.
- [ ] Download the public artifacts and verify their hashes and source provenance
      against the tested artifacts. Verify installation and execution on every
      supported target platform.
- [ ] Confirm public release notes, asset names and download URLs are correct.
      Preserve older release assets and links; do not overwrite releases.

## 3. Update and verify the installer

Follow [installer maintenance](RELEASE_POLICY.md#optional-installer-maintenance).

- [ ] Update and merge the default release version, help/examples and affected
      installer tests together, after the public archives are verified.
- [ ] Verify the published installer selects the new release without a version
      argument. Check explicit version selection, repeat installation and upgrade
      behaviour using the relevant existing acceptance tests.
- [ ] Keep manual installation and supported older-version installation accurate.
      Verify platform requirements and dependencies in installation guidance.

## 4. Build, publish and verify the browser playground package

Follow the [browser publication procedure](https://github.com/sproates/panackelty-browser#website-artifact-publication).

- [ ] Update the browser's compiler, VM and standard-library inputs together to
      the verified release bundle. Update browser package version and provenance.
- [ ] Resolve compatibility failures and update affected playground examples and
      guidance. Test compilation, execution, diagnostics and relevant new language
      features in the supported browsers.
- [ ] Publish the tested browser artifact and verify its public archive, checksum
      and recorded core version. A core browser-runtime bundle is a dependency,
      not a published playground package.
- [ ] Prepare the website dependency pin for that exact browser release. Confirm
      the website candidate actually loads and runs the new compiler; changing a
      version label alone is insufficient.

Native-only capabilities need not become browser capabilities. Explain browser
limitations accurately, but do not silently leave the playground on the previous
compiler version. Any exception to this checklist requires an explicit owner
scope decision before website publication, recorded in the acceptance record.
An implementing agent must not invent a deferral to declare completion.

## 5. Prepare and approve the complete website candidate

Follow the [website promotion procedure](https://github.com/sproates/panackelty-website#publish-and-promote-releases).

- [ ] Update native downloads, hashes, installer source/version pins, browser
      dependency pins, release notes, installation requirements and examples.
- [ ] Audit all current-release and older-release references: homepage, releases
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
