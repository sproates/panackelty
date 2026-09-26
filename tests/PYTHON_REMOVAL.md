# Repository-wide interpreter removal

This is a historical audit of the completed toolchain migration. For current
test commands and suite ownership, see [the testing guide](README.md).

The transitional implementation, root import facade, unit discovery packages
and their support helper are removed. Development, bootstrap, conformance,
installation, packaging and release validation use Panackelty, C and POSIX tools.
No language semantics, bytecode layout or compiler seed changes in this step.

## Final test retirement audit

All 21 final implementation-only methods passed before removal. Their complete
names are recorded in [HARNESS_MIGRATION.md](HARNESS_MIGRATION.md).

| Retired behavior | Continuing evidence or retirement reason |
| --- | --- |
| Twelve verifier methods | Python tuple/list/Boolean object shapes, dictionary identity and patched global limits disappear with that implementation. Representable wire signatures, entry points, operands, constants, control flow, call arity and purity remain in the malformed corpus, Panackelty codec/verifier probes and direct C verifier/fault tests. |
| Five serialization methods | Patched Python limits and its build verification hook no longer exist. Native decoder bounds and verified execution remain covered independently. ADT execution is in functional source/bytecode and fixed artifact tests. Canonical compilation is checked against frozen independent bytes and across bootstrap stages; Python hash randomisation no longer affects any component. |
| One minimal-vector method | The same minimal version-8 vector, canonical representation and Void result remain in native/self-hosted codec checks. |
| Two bootstrap diagnostic methods | Native callable-purity and logical-extension diagnostics retain their fixed compiler probe expectations. Only the removed implementation's distinct wording is retired. |
| One compile-time decimal method | The removed compiler rejected nonterminating decimal division during compilation. The public native compiler accepts it and the VM rejects it at execution; that exact native trap remains in the VM corpus. This phase difference is explicit, not claimed as equivalence. |

No native assertion, golden expectation, sanitizer, fault-injection check or
functional case is removed. The earlier oracle audit preserves 1,800 integer,
422 decimal and 96 rational expectations and all 82 builtin signatures.

## Repository policy

`make policy`, included in `make check`, runs `no_python.sh` and its independent
positive/negative controls. The policy walks actual files, including untracked
files and symlinks; only root `.git`, `build` and `output` directories are excluded.
It rejects source/cache extensions, interpreter shebangs on extensionless files,
literal interpreter commands in build/workflow/script/source files, interpreter
variable invocations and the CI setup action. Comments and historical Markdown
are allowed. Scanning uses the C locale so arbitrary bytes in assets do not cause
multibyte conversion failures; an invalid-byte binary asset is a positive control.
Command patterns cover unversioned, major-version and dotted-version
names, absolute paths, quoted commands and environment wrappers.

Controls inject each prohibited category into isolated trees, including names
with spaces, source symlinks, shell commands, Make recipes and CI steps. A valid
shell program and migration documentation must pass before and after the negative
controls. This is a conservative lexical policy, not a general shell interpreter;
dynamically constructed commands also face the isolated execution proof.

## Isolated full validation

`make check-no-interpreter` builds a temporary allowlist of native compiler and
POSIX utility commands, verifies interpreter commands cannot be resolved, then
runs `make clean`, `make check` and `make package` (which includes
`make native-check`). Thus it
includes unit and functional tests, failure injection, fresh bootstrap and seed
refresh, native conformance, archive/checksum checks and the packaged quick start.
Linux x86-64 and macOS arm64 CI package jobs both run this workflow. The separate
Linux test job retains sanitizer and LLVM coverage gates.

This proof makes Python unavailable through the project's command environment;
it does not uninstall interpreters outside `PATH` on hosted machines or claim
process sandboxing. Absolute literal interpreter dependencies are rejected by
the source policy. The allowlist is explicit and cannot inherit arbitrary host
commands. Temporary directories are cleaned on normal exit and handled signals.

Strong coverage takes priority over speed. Existing timing budgets and warnings
remain unchanged. The immediate follow-up is the validation-speed work in
`ROADMAP.md`; it must preserve all these correctness and platform gates.
