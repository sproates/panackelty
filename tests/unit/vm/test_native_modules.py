"""Contracts made testable by compiling the native VM as independent modules."""
from decimal import Decimal
from fractions import Fraction
import random
import os
from pathlib import Path
import subprocess
import unittest

from panackelty import BUILTINS

PROJECT = Path(__file__).resolve().parents[3]
VM = PROJECT / "src/vm"


class NativeModuleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.executable = Path(os.environ.get("PANACK_NATIVE_MODULE_TEST", PROJECT / "build/vm/test_modules"))
        result = subprocess.run(
            ["make", "--no-print-directory", "native-module-build"], cwd=PROJECT,
            capture_output=True, text=True,
        )
        if result.returncode:
            raise AssertionError(result.stdout + result.stderr)


    def test_registry_matches_oracle_signatures_and_has_handlers(self):
        names = sorted(BUILTINS)
        result = subprocess.run(
            [str(self.executable), "registry", *names],
            capture_output=True, text=True, timeout=10,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        expected = [
            f"{name} {len(BUILTINS[name][0])} {int(BUILTINS[name][2])}"
            for name in names
        ]
        self.assertEqual(result.stdout.splitlines(), expected)


    def test_bigint_carry_borrow_sign_and_division_properties(self):
        randomizer = random.Random(0x50414E)
        boundaries = [0, 1, 10**9 - 1, 10**9, 10**9 + 1, 10**18 - 1,
                      10**18, 2**64 - 1, 10**81 - 1, 10**81]
        pairs = [(sign * a, bsign * b) for a in boundaries for b in boundaries
                 for sign, bsign in ((1, 1), (-1, 1), (1, -1), (-1, -1))]
        pairs += [(randomizer.randrange(-10**100, 10**100),
                   randomizer.randrange(-10**50, 10**50)) for _ in range(60)]
        commands, expected = [], []
        for a, b in pairs:
            for op in range(4):
                if op == 3 and not b:
                    continue
                if op == 3:
                    quotient = abs(a) // abs(b) * (-1 if (a < 0) != (b < 0) else 1)
                    value, remainder = quotient, a - quotient * b
                else:
                    value, remainder = (a + b, a - b, a * b)[op], 0
                commands.append(f"integer {op} {a} 0 {b} 0")
                expected.append(f"{value} {remainder}")
        result = subprocess.run([str(self.executable), "arithmetic"],
                                input="\n".join(commands) + "\n", capture_output=True,
                                text=True, timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.splitlines(), expected)

    def test_decimal_scale_and_exact_arithmetic_properties(self):
        randomizer = random.Random(0xDEC)
        cases = [(a, ae, b, be) for a, b in ((0, 0), (0, 1), (1, 0), (1, 10),
                                            (-1, -10), (12345, -25))
                 for ae, be in ((-4096, -4095), (4096, 4095), (-50, 50), (0, 0))]
        cases += [(randomizer.randrange(-10**24, 10**24), randomizer.randrange(-40, 40),
                   randomizer.randrange(-10**24, 10**24), randomizer.randrange(-40, 40))
                  for _ in range(80)]
        commands, expected = [], []
        for a, ae, b, be in cases:
            left, right = Fraction(a) * Fraction(10)**ae, Fraction(b) * Fraction(10)**be
            for op in (0, 1, 2, 5):
                value = {0: left + right, 1: left - right, 2: left * right,
                         5: (left > right) - (left < right)}[op]
                commands.append(f"decimal {op} {a} {ae} {b} {be}")
                expected.append(value)
        for divisor in (2, 5, 8, 125, 2**30, 5**20):
            commands.append(f"decimal 3 -12345 -3 {divisor} -2")
            expected.append(Fraction(-12345, 10 * divisor))
        result = subprocess.run([str(self.executable), "arithmetic"],
                                input="\n".join(commands) + "\n", capture_output=True,
                                text=True, timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr)
        actual = [Fraction(Decimal(line)) for line in result.stdout.splitlines()]
        self.assertEqual(actual, expected)
