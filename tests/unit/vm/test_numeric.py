"""Retained Python compile-time numeric oracle; VM contracts use shared fixtures."""
from panackelty import PanackeltyError
from tests.unit.support import PanackeltyTestCase


class NumericTests(PanackeltyTestCase):
    def test_dec_nonterminating_division_requires_rounding(self):
        with self.assertRaisesRegex(PanackeltyError, "non-terminating"):
            self.compile("main(): Void { print(1.0 / 3.0); }")
