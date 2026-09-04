# Panackelty development instructions

These instructions apply to every change made within the Panackelty project.

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
