# RM#163: Install and run usability

[Roadmap](../../../ROADMAP.md#rm-163) · [Roadmap index](../README.md)

Status: **Planned for alpha.17**, by owner decision on 28 November 2026.
Implementation has not started. No separate GitHub issue is allocated.

## Problem and intended outcome

Preparing the HTTP status-endpoint walkthrough exposed avoidable setup work:
readers need to locate the installation directory, copy its bundled HTTP package,
write a manifest and explicitly generate a lock before running the program.
Alpha.16 already includes the required compiler, standard library and HTTP package.
A source checkout is not required, but the current tutorial still exposes too much
installation and package-management machinery.

Make installation, project creation and running someone else's program coherent
user workflows. Design the end-to-end experience first, then implement it; this is
an implementation commitment, not only a usability assessment.

## Agreed scope and acceptance

- **Install:** a supported installation leaves the user able to invoke Panack,
  with clear version and command-discovery guidance. Ordinary use does not require
  knowing the toolchain's internal directory layout.
- **Create:** initialise a project and declare/add dependencies through supported
  tooling, without requiring a handwritten manifest or manual copying of bundled
  packages. Panack can locate the HTTP package shipped with its installation.
- **Run:** provide one normal command that prepares declared dependencies and runs
  an acquired third-party program. Cover acquisition and the transition into this
  command in the documented journey; do not assume a compiler source checkout.
- **Preserve correctness:** keep explicit dependency records, reproducible locks
  and integrity checks. Existing locked versions are respected; preparing a run
  must not silently update them. Define first-run lock creation, restoration,
  offline behaviour and user-visible execution/trust boundaries in the design.
- **Explain failures:** installation, missing dependency, network, incompatible
  version and lock/integrity failures explain what happened and how to proceed.
- **Prove the journey:** from a clean supported machine, install the release,
  create and run the HTTP status service with decimal uptime and shared request
  count. Also run an existing third-party project from a fresh directory, repeat
  it with its lock, and verify the documented offline and failure behaviour.
- **Document:** update installation, package and run guidance and the HTTP
  walkthrough to match the verified commands. Record the actual steps required.

Agree concrete commands and acceptance transcripts before implementation. The
outcome is fewer necessary manual steps; no arbitrary step count or invented CLI
syntax is imposed by this scope record.

## Boundaries and delivery

Build on the existing installer and package manager. This does not automatically
add a package registry, a sandbox, new platforms, Homebrew or a package-manager
rewrite. GI#147: Homebrew installation retains its alpha.18 allocation. Respect
existing installation ownership and explain conflicts rather than replacing an
unrelated installation silently.

This is independently tracked from RM#162: Real-program fixes so it remains visible
in alpha.17. Reuse relevant work from RM#44: Package management rather than repeat
it. Effort is not yet estimated; the design should identify the smallest complete
journey and its implementation dependencies before setting delivery slices.

This record changes scope only, with no runtime or build-performance impact.
Website changes are required when the workflow is implemented and reviewed;
this decision does not itself change or publish website content.
