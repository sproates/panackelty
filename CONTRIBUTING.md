# Contributing to Panackelty

Panackelty welcomes focused bug reports, reduced failing programs, documentation
improvements, and implementation changes that advance the current roadmap.

## Before contributing

Read `README.md` for the source-build requirements, `SPEC.md` for accepted
language behavior, `ARCHITECTURE.md` for component boundaries, and `ROADMAP.md`
for current priorities. Security-sensitive reports must follow `SECURITY.md`
instead of using a public issue.

The developer preview is the immediate priority. Please discuss substantial new
language features before implementing them so release work stays focused.

## Validate a change

For changes limited to the informational files listed in `scripts/ci_docs.sh`,
run `make docs` and review the content. CI checks those documents and local file
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
[the removal audit](tests/PYTHON_REMOVAL.md) for the exact validation boundary.

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
