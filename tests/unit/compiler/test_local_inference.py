"""Retained differential oracle; direct contracts are native shared fixtures."""

import json
from pathlib import Path

from panackelty import Checker, PanackeltyError, Parser, lex
from tests.unit.support import CompilerHarnessTestCase

FIXTURES = Path(__file__).resolve().parents[2] / "fixtures/compiler_contracts"


class LocalInferenceTests(CompilerHarnessTestCase):
    def test_shared_sources_match_bootstrap(self):
        cases = [case for case in json.loads((FIXTURES / "manifest.json").read_text())
                 if case["group"] == "local_inference"]
        self.assertEqual(len(cases), 59)
        for case in cases:
            with self.subTest(case=case["name"]):
                source = (FIXTURES / (case["name"] + ".panack")).read_text(encoding="utf-8")
                actual = self.run_harness(
                    "compiler/purity.panack",
                    "main(): Void { print(check_source_frontend(command_args()[0])) }",
                    [source],
                ).removesuffix("\n")
                try:
                    Checker(Parser(lex(source)).parse()).check()
                    accepted = True
                except PanackeltyError:
                    accepted = False
                self.assertEqual(accepted, case["expect"] == "ok")
                self.assertEqual(actual == "ok", accepted, actual)
