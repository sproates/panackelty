# P08: Source core methods and canonical error identities

GI#525 P08 starts from P07 merge `8ba79d8`. Completion takes effect on integration
after the required validation and exact-head CI pass.

## Delivered scope

Core adds 19 source-defined methods, preserving existing contracts:

| Owner | Source methods |
| --- | --- |
| Str | ends_with |
| Array[T] | first, sort_by |
| Option[T], Result[T,E] | value_or |
| Path | join, to_text, native_bytes, display, is_absolute, directory, filename |
| Duration | add, subtract, scale, is_before, as_seconds, divide, ratio |

PathError and DurationError now have one canonical core declaration. The old path
and time import routes re-export that identity and its variants. ClockError stays
in time because clock access remains an explicit module service. Core imports no
modules, preserving an acyclic dependency graph. Intrinsic error lookup supports
both the new canonical declaration and pre-conversion/custom toolchain layouts.

Core functions now emit from checked bodies. This is required for compatibility
helpers that forward to owned calls; the previous core-only path left such calls
unlowered. Owned core emission names participate in existing reachability pruning,
so unused core algorithms are omitted. Dispatch remains keyed by owner metadata.
The loader emits core separately when preparing legacy programs, preserving their
resolver diagnostics and effect-availability reporting despite core's impl syntax.

## Compatibility boundary

These temporary forwarding functions retain existing callers until P10/P11 migrate
them and P12 removes the obsolete receiver-function surfaces:

- core: core_ends_with, core_first, core_sort_by (internal only).
- option/result: each value_or.
- path: join, to_text, native_bytes, display, is_absolute, directory, filename.
- time: ticks, add_instant, is_before_instant, difference, add, subtract, scale,
  is_before, as_seconds, divide, ratio.

Their algorithms live in owned methods; no duplicate implementation is kept.
Existing primitive/bytes/text wrappers remain covered by the migration inventory.
Factories, clock/host services and ordinary compiler helpers retain their roles.
No broad stdlib/compiler caller conversion or HTTP runtime work is included.

## Seed ordering and acceptance

P07 input seed: `57ee20ce5916b59e85fb23db0a286c3e372124617489e09e342d8b0d50a1152c`.
Canonical-error support was saved and validated while library declarations still
used their original syntax. Its checkpoint `5a55d75` passed make check in 651s.
A legacy-call probe then exposed the separate core-helper emission path. The
library conversion was restored to that checkpoint before updating emission and
saving a new seed. Focused binding validation passed 1,249 assertions, including
the identity, core-bridge and retained-local-type regressions. The CLI explanation
fixture passed all 218 assertions after preserving local types for its tooling recheck.

The second pre-conversion checkpoint `aa405e5` passed full `make check` in 643s.
Its saved seed is
`260ed7299003f96a87abd8f8cfe7806081383f5a265007562d7e36a34e314250`;
stages 2/3/4 match and the original stdlib conformance hash remains
`eea94d9ea68fcafaa4a68d1eabb926add89f1fc8f3e2d9b6b5877353609c052b`.

Post-conversion compiler stages 2/3/4 reproduce
`c4f6b8b2b3c567a0400353eff7ba4f9f5373caa22027d6f3b4f42bb1f8f1bdfd`.
Their stdlib conformance bytecode matches at
`8232a580910c912a04b5e8f62b89d4e174a3d01b0878314c462bf48bac14b216`,
and all stages produce the expected output. The compiler and stdlib bytecode
change because compatibility calls now reach owned implementations.
Final `make check` passed in 588s: unit 416s, bootstrap 293s (parallel with
unit), functional 123s, release smoke 11s and quick start 1s. It includes 1,249
binding assertions and 532 functional assertions. Historical timing warnings
remain informational under GI#106; no deadlines or validation policy changed.

The owned_core_methods fixture checks source and saved-bytecode execution. New
negative CLI cases assert exact diagnostics and no output artifact. Existing core
and host-types fixtures preserve legacy behavior. All 101 failure fixtures retain
exact check diagnostics. The failure-only runner passes 333 assertions; the
legacy checker adapter loads real core through the module pipeline and passes
164 assertions. The driver-logical and stdlib bytecode goldens are updated
for the reviewed forwarding calls; their runtime output is unchanged. Root README commands remain
accurate; compiler/stdlib/specification/coverage/architecture documents are updated.
Website: existing alpha.14 plans remain truthful; no released-feature claim changes.

Performance: method bodies retain existing algorithms and runtime operations.
Compatibility callers gain a forwarding call until their planned migration;
owned callers invoke the implementation directly. Core reachability pruning is
preserved. Whole-suite timings are validation evidence, not a controlled runtime
benchmark. No timeout, validation policy or source-coverage baseline is changed.

## Programme handoff

P01–P08 complete on integration; P09 is next and remains a separate arrow-capable
seed checkpoint. GI#525 stays open. Completing source built-ins raises estimated
weighted progress from 35% to 45% under unchanged weights. No P09/P10 credit is
claimed for prerequisites delivered here.

| Stage / packets | Status on acceptance | Weight | Complete | Earned |
| --- | --- | ---: | ---: | ---: |
| Inventory: P01 | Done | 5% | 100% | 5 |
| Method model: P02–P05, P07 | Done | 20% | 100% | 20 |
| Built-ins: P06, P08 | Done | 20% | 100% | 20 |
| Stdlib: P10 | Unstarted | 15% | 0% | 0 |
| Compiler/source: P09, P11 | P09 next; P11 unstarted | 20% | 0% | 0 |
| Legacy removal: P12 | Unstarted | 10% | 0% | 0 |
| Dot cutover: P13–P14 | Unstarted | 5% | 0% | 0 |
| Release/consumers: P15 | Unstarted | 5% | 0% | 0 |
| Total | | 100% | | 45% |
