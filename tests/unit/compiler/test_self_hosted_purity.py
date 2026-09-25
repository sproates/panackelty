"""Retained Python differential oracle; direct contracts live in Panackelty."""

import unittest
from pathlib import Path

from panackelty import Checker, PanackeltyError, Parser, lex
from tests.unit.support import CompilerHarnessTestCase


FIXTURES = Path(__file__).resolve().parents[2] / "fixtures/compiler_purity"


class SelfHostedPurityTests(CompilerHarnessTestCase):
    @staticmethod
    def bootstrap_accepts(source: str) -> bool:
        try:
            Checker(Parser(lex(source)).parse()).check()
        except PanackeltyError:
            return False
        return True

    def test_shared_source_acceptance_matches_bootstrap(self):
        # Keep the original Python-VM-vs-bootstrap comparison until the oracle
        # migration. Native expected-success/diagnostic assertions now run in
        # tests/runner/compiler_purity_unit.panack using these same inputs.
        fixtures = sorted(FIXTURES.glob("*.panack"))
        self.assertEqual(len(fixtures), 10, "purity oracle corpus changed")
        for fixture in fixtures:
            with self.subTest(case=fixture.stem):
                source = fixture.read_text(encoding="utf-8")
                actual = self.run_harness(
                    "compiler/purity.panack",
                    "main(): Void { print(check_source_frontend(command_args()[0])); }",
                    [source],
                ).removesuffix("\n")
                self.assertEqual(actual == "ok", self.bootstrap_accepts(source), actual)


if __name__ == "__main__":
    unittest.main()
