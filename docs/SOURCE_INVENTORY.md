# Executable-source inventory (SC2 / GI#131)

An inventory records what is eligible for future source coverage before anything
executes. It is not a hit report or a coverage percentage. Inventory generation
checks and compiles source but never runs the resulting program.

```sh
panack inventory main.panack -o project.pinv --include other/unloaded.panack
panack inventory-check main.panack --inventory project.pinv --include other/unloaded.panack
```

Each explicitly ordered root includes its transitive imports and implicit core.
An additional root can be a library without `main`. Use `--include` to inventory
files outside the entry's import closure, including unused libraries. There is no
implicit filesystem scan and no claim that a single root covers a repository.
SC5 must establish the suite's complete explicit root manifest. Duplicate resolved
roots are rejected. Shared imported files occur in each compilation context;
SC4 must aggregate their identities without summing duplicate denominators.

Creation requires a nonexistent destination in a caller-controlled directory.
Existing files, symlinks and hard links are refused. Success prints `wrote
inventory FILE`; successful validation prints `inventory: valid`. Invalid syntax,
source diagnostics, duplicate/excessive roots, stale/malformed/missing inventory,
identity failures and output failures exit 1 with a diagnostic. A failed write
may leave a partial file to discard. Publication is not atomic; callers must not
concurrently replace output paths or modify the source/toolchain tree.

## What the inventory counts

The compiler traverses `LoadedModule.parsed`, before method, namespace and generic
lowering. It never reconstructs a source denominator from emitted PCs. Entries
are ordered by original declaration and syntax traversal, including unused
functions, empty bodies, imported files and core bodies removed from a particular
artifact. Each generic declaration has one erased source identity, regardless of
how many type arguments its callers use.

| Kind | Eligibility and location |
| --- | --- |
| `function` | Every source function declaration, including empty/unused/async/generic functions; complete declaration span and original name. |
| `expression` | Every located source expression retained by the parser; half-open original span. An eventual hit means attempted evaluation, not successful completion. |
| `statement` | Binding, assignment/inferred binding, expression statement, while or for; anchored to its first evaluated expression (initializer, RHS, condition or iterable). A tail expression is an expression, not a separate statement. |
| `decision` | If condition; left operand of `&&`/`||`; while condition; for iteration test anchored to the iterable; match subject. |
| `outcome` | If true/false, including absent else; short-circuit evaluate-right/skip-right; loop body/exit; one selected outcome per source match arm, anchored to the arm body. |
| `excluded` | Import, record, enum and compile-time refinement declarations, with explicit reason and declaration span. |

Decision and outcome IDs have no relationship to the number of emitted VM
branches. Match arms are mutually exclusive source outcomes, not a sequence of
lowered tag tests; exhaustive matching has no invented unmatched outcome. A
loop's `exit` means its normal false/exhausted decision, not a trap or killed
execution. `await` retains its source expression identity across suspension.
Statement anchors deliberately exclude declaration names/types, `=` and loop
keywords; their spans do not promise the complete statement range.

Comments, whitespace, punctuation, type annotations, implicit returns,
compiler-generated operations and unlocated synthetic AST wrappers do not create
executable entries. Descendant located expressions remain eligible. Source
record/enum construction calls are expressions; their declarations are excluded.
Refinement predicates are compile-time checks, not emitted source functions.
No blanket path exclusion removes standard-library, test or generated input files.

A future line denominator is the union of eligible expression/statement **start
lines**, with function and decision metrics reported separately. Do not mark all
lines of an enclosing span executable or reached, sum overlapping item counts,
or label this inventory's item count a line count. SC3 still needs explicit
item-to-probe associations; missing emitted code is not proof of a zero hit.

## Identity and validation

The envelope contains exact compiler bytecode, each root's exact compiled artifact,
and all captured UTF-8 source text, including comments and unused files. IDs are
structural paths such as `declaration/2/statement/0/value/decision/false`, scoped
by source identity and compilation context. They are not stable across arbitrary
source edits and must never be merged by name or path alone.

The public launcher supplies `PANACKELTY_COMPILER_PATH` for the installed compiler.
The VM verifies the running compiler. The command captures its exact bytecode
identity and rechecks it after generation, without redundantly decoding it.
Direct VM users must set this trusted input to the compiler artifact they actually
run; invoking an arbitrary compiler with a falsely labelled environment is outside
this local trust contract. This is reproducibility, not producer authentication.
The VM/native host binary is not a source-compiler identity or coverage target.

Validation regenerates the entire envelope from the explicitly supplied local
roots and compiler. It compares foreign bytes only up to the locally expected
length; **no foreign inventory is decoded**, and foreign paths, counts, coordinates
or lengths never control file access or attribution. Changes to compiler,
artifact, source text, root ordering or scope invalidate the inventory. Rechecking
all roots' source snapshots after compilation detects observed edits during the
run, but does not create an atomic snapshot of a concurrently changing tree.
Portable source names are relative to the first entry directory (`project/`) or
the configured standard library (`stdlib/`), as in [source maps](SOURCE_MAPS.md).

## Version 1 layout and limits

The outer format is `PANACKINVENTORY1\n`, a u32 root count, then one length-prefixed
inner inventory per root, in the explicitly supplied order. Each inner format is:

1. `PANACKINV1\nlocal-replay-v9\n`.
2. Compiler byte blob, entry identifier, compiled bytecode blob (library roots need not be runnable).
3. Snapshot count; snapshots sorted by portable identifier.
4. Per snapshot: identifier, full source text, item count.
5. Per item: structural ID, kind, detail, then start and end positions, each with
   code-point offset, one-based line and column. The enclosing snapshot supplies
   the file; function detail is its original name, decision detail its kind,
   outcome detail its label, and exclusion detail its reason.

Integers are big-endian u32. Text and byte blobs have u32 UTF-8/byte lengths.
A format or eligibility-policy change requires a new version, not silent reuse.
The compiler identity is repeated in each inner context to make its scope explicit.

Limits: 16 explicit roots, 16 MiB compiler, the source-map loader's 1 MiB per source,
8 MiB/256 files per root, 16 MiB artifact, 100,000 items per file, 16 MiB per inner
inventory and 16 MiB overall (the native filesystem capability limit). Serialization joins bounded chunks as a balanced
tree. These are accepted input/output limits, not a CPU/memory sandbox for trusted
source parsing or compilation. The ordinary compiler, bytecode v9, VM dispatcher,
release runtime and normal execution paths remain uninstrumented.

## Acceptance evidence

`compiler_source_inventory_unit.panack` specifies complete tiny inventories and
literal decision/outcome/function/statement lists, independently of emission. It
covers Unicode code-point positions, unused imports, empty functions, erased
generics, while/for exits, short-circuiting, match arms and implicit else outcomes.
`compiler_source_inventory_cli.panack` exercises the public command, explicit
unimported roots, exact repeatability, ordinary bytecode equivalence, source edits,
root omission, malformed/truncated/forged input and output protection. Both run
in the canonical compiler/unit routes. Canonical bootstrap checks establish the
compiler fixed point; no coverage probes or metadata enter executable artifacts.
