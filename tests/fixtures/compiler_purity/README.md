# Shared purity source cases

These ten source files are declarative inputs, including deliberately invalid
programs. They are not standalone success examples.

`tests/runner/compiler_purity_unit.panack` calls the real complete frontend on
each file. Positive cases require `ok`; negative cases require the original
diagnostic substring. One cross-module case remains inline because it had no
Python differential counterpart.

`tests/unit/compiler/test_self_hosted_purity.py` reads every source file and
retains the original comparison between the self-hosted frontend on the Python
VM and the bootstrap checker. This oracle remains until the separate oracle
migration. The Python class compiles one parameterized frontend harness.

Names preserve the former `test_<group>` method and expanded case ordinal.
The migration audit verified identical source text and expectations. The
per-group mapping is in `tests/PYTHON_MIGRATION.md`.

When adding or removing a case, update the native assertion, the oracle corpus
count, this inventory, and the migration/coverage documentation together. Run
from the repository root with `make unit` or `make check-compiler`. Missing
inputs fail the native probe; the oracle also rejects a changed corpus count.
