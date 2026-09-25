"""Stage-0 wording safeguards, retained until Python seed regeneration retires.

The corresponding native messages are fixed expectations in the compiler probes;
these tests no longer compare implementations or define the public diagnostics.
"""
from pathlib import Path
import unittest

from panackelty import PanackeltyError, build

FIXTURES = Path(__file__).resolve().parents[2] / "fixtures/compiler_contracts"


class BootstrapDiagnosticTests(unittest.TestCase):
    def test_impure_callable_wording(self):
        source = FIXTURES / "syntax/callable_diagnostics_preserve_types_and_purity-1.panack"
        with self.assertRaisesRegex(PanackeltyError, "pure function cannot invoke an impure callable"):
            build(source)

    def test_logical_extension_wording(self):
        source = FIXTURES / "imports/invalid_logical_imports_are_rejected-2/invalid-1.panack"
        with self.assertRaisesRegex(PanackeltyError, "logical import extension must be .panack"):
            build(source)
