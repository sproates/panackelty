[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Codebase-wide human readability and refactoring — candidate

<a id="rm-62"></a>

**RM#62: Component readability** · [GI#132: Component readability](https://github.com/sproates/panackelty/issues/132).

Work record: [Component readability and refactoring](https://github.com/sproates/panackelty/issues/132). State: Idea; implementation
unscheduled. See the [current comparison](current-status-and-grooming-2026-10-01.md#proposed-first-step) for first-slice
estimates and recommendation.

Extend the native VM readability cleanup across all project code, including
the compiler, bytecode tooling, runtime, standard library, bootstrap code,
CLI, build and CI scripts, and test harnesses. Make the code straightforward
for a human maintainer to read, navigate and change. Start with an audit, then
refactor in component-sized PRs after establishing
relevant test evidence. The assessment proposes using source coverage to inform
risk without making its delivery a blanket prerequisite for small, well-tested
refactors; review that dependency with the implementation priorities. The REPL is not a
prerequisite; do not mix feature changes into readability refactors.

- [ ] Audit each component for dense or oversized functions, unclear names,
      duplicated logic, hidden dependencies and obsolete code; record a scoped
      refactoring sequence using the native VM cleanup as the model.
- [ ] Split responsibilities into cohesive modules and small, clearly named
      functions; make interfaces, data flow, ownership and error handling
      explicit, with consistent formatting and comments explaining non-obvious
      decisions and invariants.
- [ ] Remove obsolete code and unnecessary duplication, keeping any required
      bootstrap distinctions explicit; update architecture and component
      documentation alongside each refactor.
- [ ] Preserve observable behavior, exact numeric semantics, purity, bytecode
      safety and bootstrap reproducibility. Retain all existing assertions,
      add meaningful regression coverage where gaps are found, and validate
      each scoped refactor with `make check` and applicable sanitizer/platform
      gates; measure and avoid validation or runtime performance regressions.
