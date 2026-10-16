[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Make imports independent of repository paths — complete

<a id="rm-81"></a>

**RM#81: Logical imports**.

User programs and examples no longer need to know the source-tree location of
the standard library. The canonical `import stdlib/option` and
`import project/shared/module` forms use reserved logical namespaces;
file-relative imports remain quoted. A terminal `.panack` suffix and quoted
logical paths are accepted compatibility spellings. The launcher supplies the
toolchain-owned library root, including from installed layouts, while the
project root is the entry source file's directory.

- [x] Choose and specify the canonical logical-import syntax, including whether
      quotes and the `.panack` suffix are required, optional, or distinguish
      logical imports from file-relative imports
- [x] Define deterministic resolution rules for file-relative, project-local,
      and standard-library modules without depending on the process working
      directory
- [x] Define project-root discovery, search precedence, shadowing, ambiguity,
      path traversal, canonical identity, and useful missing-module diagnostics
- [x] Make the compiler locate bundled standard-library modules in both a source
      checkout and an installed or packaged toolchain
- [x] Preserve load-once and cycle-detection behavior when the same module is
      reachable through different valid import spellings
- [x] Implement the accepted syntax and resolution rules in every compiler and
      loader that remains part of the development and bootstrap workflow
- [x] Migrate examples, tests, compiler sources where appropriate, and
      documentation away from repository-relative standard-library paths
- [x] Add focused and public-CLI coverage for logical standard-library imports,
      project-local imports, installed layouts, ambiguity and shadowing, missing
      modules, invalid paths, cycles, and source/bytecode execution
