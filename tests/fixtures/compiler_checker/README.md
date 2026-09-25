# Shared checker source cases

These 31 source files are declarative inputs, including deliberately invalid
programs. They are not standalone success examples.

`tests/runner/compiler_checker_unit.panack` calls the real checker on each file.
It requires `ok` for positive cases and the specified diagnostic substring for
negative cases. Three module-graph cases remain inline in that probe because
they have no Python differential counterpart.

`tests/unit/compiler/test_self_hosted_checker.py` reads every source file and
retains the original comparison between the self-hosted checker running in the
Python VM and the bootstrap checker. This oracle remains until the separate
oracle migration. The Python class compiles one parameterized checker probe.

Names preserve the former `test_<group>` method and expanded case ordinal.
The migration audit verified byte-identical source text. The per-group mapping
is in `tests/PYTHON_MIGRATION.md`.

When adding or removing a case, update the native assertion, the oracle corpus
count, this inventory, and the migration/coverage documentation together. Run
from the repository root with `make unit` or `make check-compiler`. Missing
inputs fail the native probe; the oracle also rejects a changed corpus count.
