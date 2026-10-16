[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Interactive REPL — exploration; website learning use case identified

<a id="rm-78"></a>

**RM#78: Interactive REPL**.

Add a read-evaluate-print loop for exploring Panackelty expressions, trying
standard-library APIs and learning the language without creating a source file
for every experiment. This is an idea with no implementation slot or dependency
claim on other work. On 2026-09-30 the user identified trying the language on
the website as a concrete learning workflow. The complete-program browser
playground now provides that starting point; stateful REPL semantics remain
separately scoped. The website content update shipped in PR #108 and the complete-program playground shipped
in PR #115. Stateful REPL work remains unscheduled and should reuse the toolchain.

- [ ] Specify the entry command (for example `panack repl`), expression result
      display, multiline input and incomplete-input detection
- [ ] Define session semantics for bindings, functions, imports, mutation and
      redefinition, including what state survives compilation or runtime errors
- [ ] Compile interactive input to bytecode and execute it through the existing
      native VM, preserving exact numerics, static checks and purity/effect
      boundaries; do not introduce a separate evaluation engine
- [ ] Provide useful diagnostics and session commands for help, reset and exit;
      define interrupt, EOF, history and noninteractive-input behavior
- [ ] Add transcript and failure-recovery tests covering state across inputs,
      multiline definitions, imports, type/purity errors, runtime traps and
      interruption on both supported platforms
- [ ] Document the workflow and ship the REPL in the standalone toolchain,
      with packaging and release smoke coverage
