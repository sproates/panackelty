# Contributing to Panackelty

Panackelty welcomes focused bug reports, reduced failing programs, documentation
improvements, and implementation changes that advance the current roadmap.

## Before contributing

Read `README.md` for the source-build requirements, `SPEC.md` for accepted
language behavior, `ARCHITECTURE.md` for component boundaries, and `ROADMAP.md`
for current priorities. Security-sensitive reports must follow `SECURITY.md`
instead of using a public issue.

The developer preview has shipped; implementation priorities are being assessed.
Follow the [roadmap decision process](docs/ROADMAP_PROCESS.md) and discuss
substantial features before implementing them. Use the **Roadmap proposal** issue
form for ideas or bounded investigations; an open issue is not a commitment.
The repository records agreed direction and priorities, with Issues providing
linked work details and discussion.

## Conventions

These conventions apply to new and substantially changed work. Preserve existing
public names and keep unrelated formatting or renaming out of focused changes.
Language syntax and semantics remain defined by [SPEC.md](SPEC.md).

### Shared agent skills

Repository skills live in `.agents/skills/` and travel with a clone. The
[Next Item skill](.agents/skills/next-item/SKILL.md) guides answers to "what's
next?", including alternatives, recommendation rationale, size, PR estimates
and a simple explanation. Codex supports this repository skill location;
other agents can follow the file directly through the link in `AGENTS.md`.
See the [official skill documentation](https://learn.chatgpt.com/docs/build-skills)
for discovery details. Maintain the repository copy through normal PR review;
personal copies are independent and do not update it automatically.

### Repository health skill

The [Repository Health skill](.agents/skills/repository-health/SKILL.md) audits
dangling PRs, issue/completion bookkeeping, default-branch health and merged
branch clutter. `AGENTS.md` requires a lightweight run after major/principal
task completion; it is also the shared procedure for an explicit repository
health or housekeeping request.

### Programme status skill

The [Programme Status skill](.agents/skills/programme-status/SKILL.md) establishes
provisional scope-based weights when planning and reports all scoped tasks after
each programme task delivery or completion. It distinguishes evidenced progress
from formal acceptance and preserves paused work in the denominator. Baselines
and evidence belong in the roadmap's programme register; see the
[programme tracking rules](docs/ROADMAP_PROCESS.md#programme-tracking).

### Branches, commits and pull requests

- Branch from current `main` using `<kind>/<short-kebab-case-description>`.
  Use `feat/`, `fix/`, `docs/`, `refactor/`, `test/`, `ci/` or `chore/`
  for the principal purpose; use `chore/` for maintenance that does not fit
  another kind. For example: `feat/compiler-name-suggestions` or
  `docs/adoption-direction-and-issue-workflow`. An issue number is optional:
  `fix/123-import-collision`. Existing branches need not be renamed.
- Use focused commits with a short imperative subject describing the result,
  such as “Explain unresolved names with nearby suggestions”. Add a body when
  the reason or trade-off is not obvious. Conventional Commit prefixes are
  not required. Use your GitHub noreply identity and verify it before committing.
- Give PRs a descriptive title and explain the problem, resulting behavior,
  scope, validation and material limitations. Link related issues and roadmap
  items; reserve automatic issue-closing links for work completed on merge.
- Include a proportionate [performance assessment](docs/ROADMAP_PROCESS.md#performance-impact-and-regression-decisions)
  for substantive deliveries: relevant evidence and comparability limits, or a
  reasoned no-impact statement. Present unresolved regressions and their explicit
  disposition; passing correctness checks alone do not establish performance acceptance.
- Keep each PR reviewable around one coherent outcome. Update affected docs,
  report actual test results and preserve unrelated work. Follow the
  [roadmap process](docs/ROADMAP_PROCESS.md) for priorities and decisions.
- Obtain independent review of substantive delivery revisions before merge approval;
  record the reviewed commit or tree, findings and resolution under the
  [review rules](docs/ROADMAP_PROCESS.md#independent-review).
- Never push directly to `main`. Obtain explicit user permission for each PR
  merge; scope approval and green checks alone are not merge permission.

### Panackelty source

- Use two spaces for indentation, spaces rather than tabs, and a final newline.
  Keep imports together at the start and separate top-level declarations with
  blank lines. Match nearby layout where a small edit does not justify reformatting.
- Use descriptive `snake_case` function, binding, field and file names;
  `PascalCase` type and variant names; and short conventional type parameters
  such as `T` where their meaning is clear. Preserve deliberate public spellings.
- Prefer small cohesive functions, clear intermediate names and multi-line control
  flow when nesting or multiple operations make a compact expression hard to read.
  Simple one-line expressions are acceptable; avoid dense statement chains.
- Comment intent, invariants, non-obvious constraints and trade-offs. Keep comments
  accurate and avoid narrating code that already explains itself.
- Preserve explicit purity and type/effect contracts. Use inference where it
  improves clarity; do not remove useful type information merely to shorten code.
- Follow [standard-library guidance](src/stdlib/README.md) for current public
  prefixes and imports. Namespace migration remains pending; this guide does not
  authorise API renaming or claim that a source formatter is available.

### C, tests and repository structure

- Follow [VM editing and ownership conventions](src/vm/README.md#ownership-and-editing-conventions)
  and `src/vm/.clang-format` for C: four spaces, braces, descriptive names and
  explicit ownership contracts. Format changed C code and inspect the diff;
  keep unrelated formatting out of the PR.
- Follow [ARCHITECTURE.md](ARCHITECTURE.md#repository-layout) and component READMEs
  for file placement and responsibility boundaries. Keep implementation in
  `src/`, user examples in `examples/`, and tests in the documented test layout.
- Follow [tests/README.md](tests/README.md) for fixtures, runners and assertions,
  and [AGENTS.md](AGENTS.md) for meaningful coverage, cleanup and validation.
  Test names should describe the behavior or failure being checked.
- Follow [RELEASE_POLICY.md](RELEASE_POLICY.md) for versions, tags, compatibility
  and release gates rather than defining a separate release naming scheme here.

These are review conventions, not a claim of automated enforcement. Existing
policy, formatting and validation tools cover only their documented scope.

## Validate a change

For website changes, run `node scripts/preview.cjs` (Node 24) to build and start
a temporary loopback server. Open the printed URL on the development machine;
stop with Ctrl-C, rerun after editing, then refresh the browser.
See [local previews](docs/PR_PREVIEWS.md) for saved artifacts and remote-workspace
access. No hosting provider or contributor hosting account is required.

Run `bash scripts/validate_change.sh --plan origin/main` to inspect the affected
components and required checks, then use `--run` to execute the selected local
route. It includes branch changes plus staged, unstaged and untracked edits.
For changes limited to the informational/process files in `scripts/ci_docs.sh`,
this runs `make docs`; review the content as well. CI checks those documents and local file
links without building the toolchain. See
[change-aware CI](tests/README.md#change-aware-ci) for the exact boundary.

For implementation, specification, packaged-input, workflow, mixed or unknown
changes, run from the repository root:

```sh
make check
```

This is the canonical validation command and includes unit tests, complete
program tests through `panack`, and the reproducible-bootstrap proof. Add focused
unit tests for changed internals and functional tests for observable behavior.

Development uses C, Panackelty and POSIX tools. `make policy` rejects interpreter
dependencies; `make check-no-interpreter` repeats the complete development and
package workflow with an allowlisted command environment. See
[the testing guide](tests/README.md) for the validation commands.

Repository-specific requirements for documentation, cleanup, validation budgets,
and commits are defined in `AGENTS.md` and apply to every contribution.

## Report a bug

Open the repository's **Issues** page, select **New issue**, and choose
**Bug report**. The structured form requires the output of `panack --version`,
the host operating system and processor, the smallest complete `.panack` program
that reproduces the problem, the command used, and the complete standard output
and error output. It also asks what you expected and whether the issue occurs
after compiling the program to bytecode.

Do not include secrets or private data. Suspected vulnerabilities belong in the
private reporting channel described by `SECURITY.md`, not in a public bug report.
