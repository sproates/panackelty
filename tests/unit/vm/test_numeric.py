import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import Code, PanackeltyError, VM, build, load_bytecode, verify_bytecode
from tests.unit.support import PanackeltyTestCase
from tests.unit.rational_cases import RATIONAL_FAILURES, rational_failure_source


class NumericTests(PanackeltyTestCase):
    def test_big_natural_arithmetic(self):
        code = self.compile("""
pure huge(): Nat { 999999999999999999999999999999 * 9 }
main(): Void { print(huge()); }
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "8999999999999999999999999999991\n")

    def test_integer_division_produces_rational(self):
        code = self.compile("main(): Void { print(7 / 2); }")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "7/2\n")

    def test_rational_conversion_and_zero_failures(self):
        for expression, message in RATIONAL_FAILURES:
            with self.subTest(expression=expression):
                code = self.compile(rational_failure_source(expression))
                with self.assertRaisesRegex(PanackeltyError, message):
                    VM(code).run()

    def test_rational_unit_program_matches_contract(self):
        root = Path(__file__).resolve().parents[2] / "functional/cases/rational_unit"
        self.assertEqual(self.run_code(build(root / "main.panack")),
                         (root / "expected.stdout").read_text())

    def test_dec_is_exact(self):
        code = self.compile("pure add(a: Dec, b: Dec): Dec { a + b } main(): Void { print(add(0.1, 0.2)); }")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "0.3\n")

    def test_dec_arithmetic_exceeds_host_context_without_rounding(self):
        left = "1234567890" * 15 + ".0"
        expected = str((int("1234567890" * 15) * 9)) + ".00"
        code = self.compile(
            f"pure scale(value: Dec): Dec {{ value * 9.0 }} "
            f"main(): Void {{ print(scale({left})); }}"
        )
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), expected + "\n")

    def test_dec_finite_division_is_exact(self):
        code = self.compile("main(): Void { print(1.0 / 8.0); }")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "0.125\n")

    def test_dec_nonterminating_division_requires_rounding(self):
        with self.assertRaisesRegex(PanackeltyError, "non-terminating"):
            self.compile("main(): Void { print(1.0 / 3.0); }")

if __name__ == "__main__":
    unittest.main()
