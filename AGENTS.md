# Panackelty development instructions

These instructions apply to every change made within the Panackelty project.

## Planning and approvals

Before selecting work, read [ROADMAP.md](ROADMAP.md) and the
[roadmap decision process](docs/ROADMAP_PROCESS.md), including its GitHub Issues
workflow. Follow [contributor conventions](CONTRIBUTING.md#conventions) for branch
names, commits, PRs and source style. The repository defines direction, priority,
decisions and how to find
issue details; do not rely on prior chat history or infer priority from issue age.
Keep accepted decisions and completion summaries in the repository as described
there. Use feature branches and PRs; never push directly to main. Obtain explicit
user permission for each PR merge; agreement on scope is not merge permission.

Respect the [three-deliverable grooming checkpoint](docs/ROADMAP_PROCESS.md#three-deliverable-grooming-checkpoint).
Before starting a principal task, check the review baseline and completed-outcome
ledger in `ROADMAP.md`. After three accepted deliverables, review priorities with
the user and record the decision before starting the next principal task. Count
task outcomes, not PRs; update the ledger during each completion handover. Review
sooner when new evidence warrants it. Urgent fixes may proceed with the reason
recorded, but must not silently reset or discard a due review.

When asked "what's next", "what's the next item", or a similar prioritisation
question, read and follow the repository's
[Next Item skill](.agents/skills/next-item/SKILL.md). Compare viable candidates,
explain the recommendation over the alternatives, estimate task size and PR
count, and give an ELI5 explanation. Use this repository copy for Panackelty if
a personal copy is also available. Agents without automatic skill discovery
should read the linked file directly; no personal skill installation is needed.

## Definition of done

For every implementation change:

- Add or update unit tests that cover changed internal behavior and important
  failure cases.
- Add or update functional tests when observable program behavior changes.
  Functional tests must run complete Panackelty programs through the `panack`
  command and assert their output.
- Run `make check` after the final edit.
- Do not report the work as complete unless both the unit and functional suites
  pass through `make check`.
- If validation cannot run, explain why and identify the unverified behavior.
- Never weaken, skip, or delete a test merely to make validation pass.

Review documentation during every implementation change and update all files
whose claims, examples, paths, diagrams, or status are affected:

- Update `SPEC.md` for language syntax, types, effects, runtime semantics, and
  deliberate limitations.
- Update `ARCHITECTURE.md` for components, dependencies, execution flows,
  bytecode or VM behavior, and repository structure.
- Update `README.md` for user-facing features, commands, examples, setup, and
  development workflow.
- Update `SELF_HOSTING.md` for bootstrap progress, completed milestones, and
  remaining work.
- Update `ROADMAP.md` for progress on non-bootstrap language and engineering
  initiatives.
- Update `tests/COVERAGE.md` when specified behavior, test evidence, coverage
  status, or the prioritized test backlog changes.
- Update component README files when their ownership or contracts change.

If no documentation change is necessary, state in the final response that the
documentation was reviewed and why it remains accurate.

## Website impact

For every core change and release, follow the
[website impact and follow-up process](docs/ROADMAP_PROCESS.md#website-impact-and-follow-ups).
Record needed updates in the roadmap's website follow-up register in the same
PR, with affected claims, versions, prerequisites and ownership. State no impact
with a reason when applicable. Recording work does not start or schedule it.
Version lag is acceptable only while the live site remains truthful for its
stated versions; broken release links and incorrect syntax are correctness
defects, not optional promotions. Review pending entries at releases, website
changes and grooming, and verify the live correction before closing them.

## Completion handover

Before requesting merge approval, follow the
[completion handover](docs/ROADMAP_PROCESS.md#completion-handover) and identify
whether the PR finishes its task, delivers an intermediate slice, or leaves
post-merge acceptance pending. Treat missing status updates as unfinished PR work.

- The final delivery PR must include the roadmap's Done summary, supported by
  acceptance evidence, and `Closes #...` for each completed issue. The summary
  takes effect on merge; do not describe an unmerged PR as already delivered.
- If acceptance needs deployment or other post-merge verification, keep the issue
  open and record the exact remaining checks and who will perform them. Use
  `Refs #...`, not an automatic closing keyword.
- After an authorised merge, verify the merged roadmap and issue state. Complete
  any authorised post-merge checks and promptly prepare the completion update;
  do not leave bookkeeping for the next grooming pass. Further PR merges still
  require explicit approval.
- Include completion state and any remaining acceptance in the final handover.
  If a task has no issue, say so rather than creating one solely to close it.
- Update the roadmap's deliverable ledger when an accepted outcome completes,
  and state when the three-deliverable review is due. Status maintenance remains
  part of every delivery; it must not wait for the grooming checkpoint.

## Repository health after major tasks

After the final PR for a major or principal task is merged and any post-merge
acceptance is completed or explicitly recorded, run the repository
[Repository Health skill](.agents/skills/repository-health/SKILL.md). Check open
PRs, issue/completion bookkeeping, default-branch health and merged branch
clutter. Keep this automatic post-task audit lightweight unless it finds an
inconsistency. It is not required after minor documentation edits, typo fixes,
intermediate PRs or routine bookkeeping changes.

Agents without automatic skill discovery should read the linked file directly.
The audit does not grant authority to close issues, delete branches, change
repository settings or merge PRs; existing approval rules still apply.

## Repository hygiene

Perform a cleanup audit after every change and before final validation:

- Remove files, compatibility layers, imports, declarations, tests, fixtures,
  documentation passages, and configuration entries made obsolete by the
  change. Do not leave parallel legacy and replacement implementations unless
  an explicitly documented compatibility or bootstrap requirement needs both.
- Remove temporary outputs and generated artifacts created while working,
  including bytecode files, caches, scratch files, logs, and empty directories.
  Run `make clean` when applicable and verify generated files have not returned
  after validation.
- Search for references before deleting a tracked file or symbol, then update
  every affected caller, test, command, path, and document in the same change.
- Keep cleanup scoped to the current work and clearly obsolete repository
  material. Preserve unrelated changes, ignored user-owned directories, local
  configuration, and artifacts whose ownership or purpose is uncertain.
- Finish with `git status` and a targeted file/reference search so the commit
  contains no accidental outputs, stale references, or unexplained files.

## Informational documentation changes

For changes limited to regular, non-executable files in the explicit
`scripts/ci_docs.sh` allowlist, run `make docs` and review the edited content.
These informational edits do not require rebuilding the compiler or running
`make check`. The allowlist covers the roadmap, architecture, self-hosting status, three
test/report documents and the five explicitly listed process documents. These
process files are not build, package or executable-fixture inputs. Use
`bash scripts/validate_change.sh --run origin/main` for selection across the
branch and local staged, unstaged and untracked edits. Additions, deletions and
renames must keep local links valid.

For changes confined to the four regular, non-executable site files listed by
`validation_website_only_path` (optionally with informational documents), use
`bash scripts/validate_change.sh --run origin/main` and review the content.
This explicit website route replaces native validation with website automation
locally and release-integrity plus all browser tests in hosted Check. Do not
claim complete website validation from local automation alone. Visual changes
still require preview acceptance. Routing/publisher/workflow changes remain
full implementation changes requiring `make check`.

README quick-start content, specifications, packaged documents, unlisted
instructions, workflows, code, mixed changes and unknown impact require full
validation. The reviewed process files still need content review; Markdown
syntax alone never establishes that a new file is informational.
Changes to the routing/checking implementation require its regression suite
and the canonical `make check`. CI preserves the existing check names on all
routes; routing failures must fail those checks.

## Validation

`make check` is the canonical project validation command. It must remain usable
from the repository root and must run both `make unit` and `make functional`.

Validation performance is an internal nonfunctional requirement:

- A clean `make check` should complete within 120 seconds on the reference CI
  or development environment.
- A focused incremental check, with the native toolchain already built, should
  complete within 15 seconds.
- Validation commands must report enough timing information to identify a
  budget regression. When an observed run exceeds its budget, emit or report a
  warning and add or update a prioritized reminder in `ROADMAP.md`; do not let a
  known regression become the unrecorded norm.
- Never skip, weaken, or silently move required coverage merely to meet a time
  budget. Remove duplicated work, reuse safe artifacts, improve test selection,
  or optimize the implementation instead.

Unit tests live under `tests/unit` and exercise compiler, bytecode, VM, and
runtime internals directly. Functional tests live under `tests/functional`, use
a varied set of complete `.panack` programs as input, compile or run them on the VM
through the public CLI, capture their output, and assert the observable result.

When changing the CLI, compiler, bytecode format, VM, runtime, imports, or file
layout, ensure the tests include the relevant end-to-end workflow as well as
focused behavior tests.

## Change discipline

- Commit every completed change to this local repository after validation. Keep
  commits focused and use messages that describe the resulting behavior or
  repository state.
- Do not add a Git remote unless the user explicitly requests one.
- Keep the VM as the only execution engine; source execution must compile to
  bytecode before running.
- Preserve the purity effect boundary and exact numeric semantics.
- Treat bytecode as untrusted input and retain verification and runtime safety
  checks.
- Keep implementation sources under their logical `src` component directory;
  reserve `examples` for user-facing example programs.
- Preserve the stable `panack` command unless a deliberate CLI change is part of
  the task.
