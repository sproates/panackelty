[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Harden and expand test coverage — candidate; foundation delivered

<a id="rm-60"></a>

**RM#60: Test coverage hardening**.

The [invariant-testing item](groomed-development-and-engineering-work.md#systematic-invariant-testing) scopes generated inputs
and equivalence checks separately from [source measurement](review-github-repository-settings-and-tooling--idea.md#measure-panackelty-source-coverage--candidate-pending-assessment).
Both extend existing evidence; neither changes coverage status by being planned.

The goal is to make regressions difficult to introduce and failures easy to
localize while keeping `make check` the canonical validation command.

- [x] Separate internal unit tests from black-box functional program tests
- [x] Discover functional cases and example expectations without a central
      manifest
- [x] Inventory the behavior promised by `docs/reference/SPEC.md` and map it to existing tests
- [ ] Audit success and failure assertions by language construct and runtime
      built-in; turn verified omissions into bounded, risk-ranked test PRs
- [x] Cover every CLI command and shorthand through end-to-end subprocess tests
- [ ] Expand type, refinement, purity, and name-resolution diagnostic coverage
- [ ] Exercise file, import, malformed-input, and operating-system failure paths
- [ ] Expand malformed and adversarial bytecode verifier and VM coverage
- [x] Establish deterministic compilation and canonical bytecode round-trip tests;
      extend edge cases only against identified gaps
- [x] Establish direct lexer, parser, resolver, checker, purity, emitter and
      driver probes; this is a foundation, not proof of exhaustive behavior
- [ ] Audit remaining compiler/library assertions and source coverage by component
- [x] Establish and publish a native C line/branch baseline
- [ ] Establish the separate `.panack` baseline through the candidate measurement initiative
- [ ] Review intentional exclusions and untested host-boundary behavior explicitly
- [x] Organize the suite so focused failures remain fast and the full suite stays
      practical to run after every change

Testing work that is also a prerequisite for self-hosting should be reflected
in both roadmaps when completed.

### Publish public native C coverage reports — complete

<a id="rm-61"></a>

**RM#61: Native coverage publication**.

The native VM's LLVM line and branch coverage remains uploaded as a CI
artifact. The coordinated Pages publisher makes the HTML report available at
`https://panackelty.com/coverage/`, so readers can open it directly without
visiting an Actions run or downloading and extracting an archive. PR #81 merged
as `52fb0d7`; its main validation and Pages build, deployment and live verification
all passed on 2026-09-28. The published report is native C coverage only: 87.43%
lines, 80.65% branches and 100% functions; it is not a `.panack` baseline.

- [x] Implement publication of the successful `main` build's native coverage HTML and summary
      through the project's existing GitHub Pages site, with source commit,
      generation date and an explicit label that this covers the native C VM,
      not the self-hosted compiler or the entire language
- [x] Keep the website and coverage deployment coordinated so a normal site
      update cannot erase the latest report and a report update cannot replace
      the site with stale content; publish only from trusted, successful builds
- [x] Link the stable report from the site and README, add deployment checks for
      its entry page and relative source-navigation links, and keep the existing
      downloadable CI artifact for debugging
- [x] Document how publication failures and stale results appear, and ensure
      release and PR workflows cannot publish an unreviewed coverage site
- [x] Confirm the first merged `main` run publishes successfully and passes the
      live entry-page, source-navigation and provenance checks
