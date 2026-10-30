# Contributing to Panackelty

Bug reports, small programs that reproduce a problem, documentation improvements
and focused implementation changes are welcome.

## How Panackelty is developed

I use AI extensively in developing Panackelty, including implementation, tests,
documentation and verification. AI has made a substantial contribution to the
project, and I want to be clear about that.

I set the direction for the language and spend a lot of time working through
its design, defining requirements, questioning proposed solutions and reviewing
how the result behaves. I decide what belongs in the project and what needs more
work. Responsibility for those decisions and for accepting changes remains with me.

## AI-assisted contributions

AI-assisted contributions are welcome. The same standards apply however a
contribution was produced.

You are responsible for understanding, reviewing and validating what you submit,
and for responding to review feedback. Check generated explanations, references
and claims about test results. Report checks you actually ran and distinguish
those results from assumptions or suggestions.

You don't need to name your tools or label AI-generated passages. Using AI
doesn't transfer responsibility for the contribution to the tool.

## Before contributing

New to Panackelty? Start with the [first program](docs/guides/INSTALLATION.md#write-and-run-a-first-program)
and the [language tour](docs/guides/LANGUAGE_TOUR.md).

For development, use:

- [README](README.md) for source-build requirements.
- [Language specification](docs/reference/SPEC.md) for supported syntax and behaviour.
- [Architecture](docs/internals/ARCHITECTURE.md) for component responsibilities.
- [Roadmap](ROADMAP.md) for priorities and agreed work.
- [Testing guide](tests/README.md) for fixtures, runners and checks.

**Discuss substantial features and refactors before implementing them.** Use the
**Roadmap proposal** issue form to describe the problem, scope and proposed
investigation. An open issue isn't a commitment to implement it. Follow the
[roadmap decision process](docs/roadmap/ROADMAP_PROCESS.md) when agreeing work.

## Conventions

These conventions apply to new and substantially changed work. Preserve existing
public names and keep unrelated formatting or renaming out of focused changes.
The specification remains the authority on language behaviour.

### Branches, commits and pull requests

**Branch from current `next` and target core work at `next`.** Use a descriptive
name such as `feat/compiler-name-suggestions` or `fix/123-import-collision`.
Supported prefixes are `feat/`, `fix/`, `docs/`, `refactor/`, `test/`, `ci/` and
`chore/`. Issue numbers are optional; existing branches needn't be renamed.

Keep commits focused. Use a short imperative subject describing the result,
such as “Explain unresolved names with nearby suggestions”. Add a body when
the reason or trade-off needs explanation. Conventional Commit prefixes aren't
required. Use your GitHub noreply identity and verify it before committing.

### Panackelty source

The [contributor coding standard](docs/CONTRIBUTOR_CODING_STANDARD.md) contains
the required and preferred rules for Panackelty source, C, tests and file layout.
It applies to production code, tools, libraries, tests and examples.

<!-- Preserve existing contribution-guide links to the detailed standard. -->
<a id="readable-panackelty"></a>
<a id="layout-and-whitespace"></a>
<a id="names-and-file-organisation"></a>
<a id="imports-and-module-boundaries"></a>
<a id="types-declarations-and-effects"></a>
<a id="control-flow-patterns-and-errors"></a>
<a id="comments-tests-and-examples"></a>
<a id="exceptions-adoption-and-enforcement"></a>
<a id="c-tests-and-repository-structure"></a>

Start with [readable Panackelty](docs/CONTRIBUTOR_CODING_STANDARD.md#readable-panackelty).
Code should make its purpose, main flow and important constraints easy to follow.
Formatting alone doesn't establish readability. Small fixes can preserve the
surrounding style; don't reformat unrelated code.

Preserve the VM as the only execution engine, compilation before source execution,
the purity boundary, exact numeric semantics, bytecode verification and runtime
safety checks. Changes to the stable `panack` command need an agreed scope.

## Validate a change

Use the change-aware runner from the repository root. For a PR into `next`:

```sh
bash scripts/validate_change.sh --plan origin/next
bash scripts/validate_change.sh --run origin/next
```

Use `origin/main` instead for a PR targeting `main`. The runner considers branch
changes together with staged, unstaged and untracked edits.

The required checks depend on the change:

- **Informational documentation:** files allowed by `scripts/ci_docs.sh` run
  `make docs`. Review the wording, examples and local links as well. CI doesn't
  build the toolchain for this route.
- **CI and workflow changes:** files listed by `scripts/ci_scope.sh` run focused
  routing, YAML, result-gate and workflow/layout checks.
- **Product, specification, packaged-input, mixed or unknown changes:** run
  `make check`. This includes unit tests, complete programs run through `panack`
  and the reproducible-bootstrap proof.

See [change-aware CI](tests/README.md#change-aware-ci) for the exact boundaries.
Add focused unit tests for changed internals and functional tests for observable
behaviour. Never weaken coverage to make validation pass. If a check can't run,
say why and identify what remains unverified.

Development uses C, Panackelty and POSIX tools. `make policy` rejects interpreter
dependencies. `make check-no-interpreter` repeats the development and package
workflow in an allowlisted command environment.

Website changes belong in the [website repository](https://github.com/sproates/panackelty-website).
Use that repository's contributor, preview and validation instructions.

### Documentation and cleanup

Update affected documentation in the same PR:

- Language contracts in [SPEC.md](docs/reference/SPEC.md).
- Component boundaries in [ARCHITECTURE.md](docs/internals/ARCHITECTURE.md).
- User workflows in [README.md](README.md).
- Bootstrap progress in [SELF_HOSTING.md](docs/internals/SELF_HOSTING.md).
- Delivery state in [ROADMAP.md](ROADMAP.md) and test evidence in [COVERAGE.md](tests/COVERAGE.md).
- Component READMEs where their contracts or instructions change.

If you reviewed the documentation and no update is needed, explain why.

Remove obsolete files, callers, fixtures and references with the change.
Preserve unrelated work and user-owned configuration. Use `make clean` where
applicable to remove generated outputs, then inspect `git status` and the diff.

## Open the pull request

Keep each PR focused on one reviewable outcome. Write a descriptive title and
explain the problem, resulting behaviour, validation and material limitations.
Link related issues and roadmap items. Use automatic issue-closing links only
when the work will be complete on merge.

For substantive deliveries, include a proportionate
[performance assessment](docs/roadmap/ROADMAP_PROCESS.md#performance-impact-and-regression-decisions).
Give relevant evidence and its limits, or explain why performance is unaffected.
Report unresolved regressions and their proposed disposition. Correctness checks
alone don't establish performance acceptance.

The old 120-second clean-validation and 15-second incremental-check figures are
historical comparisons, not active acceptance budgets. Report the actual workload
and timing rather than using those figures as pass/fail thresholds.

Follow the [review rules](docs/roadmap/ROADMAP_PROCESS.md#independent-review) for
independent review of substantive delivery revisions. Record the reviewed commit
or tree, findings and their resolution before merge approval.

Never push directly to `main` or `next`. Changes go through PRs and required
checks; don't enable auto-merge. Maintainers merge agreed core work into `next`.
Promotions to `main` require a separate PR and explicit approval.

## Report a bug

Open **Issues**, select **New issue**, then choose **Bug report**. Include:

- The output of `panack --version`, your operating system and processor.
- The smallest complete `.panack` program that reproduces the problem.
- The command you ran and complete standard output and error output.
- What you expected, and whether the problem also occurs with compiled bytecode.

Don't include secrets or private data. Report suspected vulnerabilities through
the private channel in [SECURITY.md](SECURITY.md), not a public issue.
