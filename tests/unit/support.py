import contextlib
import io
import json
import os
import tempfile
import unittest
from pathlib import Path

from panackelty import VM, build


class PanackeltyTestCase(unittest.TestCase):
    def compile(self, source: str):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "test.panack"
            path.write_text(source, encoding="utf-8")
            return build(path)

    @staticmethod
    def run_code(code) -> str:
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        return output.getvalue()


class CompilerHarnessTestCase(unittest.TestCase):
    """Compile each parameterised probe once per class, with a fresh VM per call."""

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.harnesses = {}
        cls.addClassCleanup(cls.harnesses.clear)

    def run_harness(self, module, main_source, arguments=(), *, environment=None):
        key = (module, main_source)
        if key not in self.harnesses:
            imported = Path(__file__).resolve().parents[2] / "src" / module
            with tempfile.TemporaryDirectory() as directory:
                main = Path(directory) / "main.panack"
                main.write_text(
                    f"import {json.dumps(os.path.relpath(imported, Path(directory).resolve()))}\n{main_source}",
                    encoding="utf-8",
                )
                self.harnesses[key] = build(main)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(self.harnesses[key], list(arguments), environment=environment).run()
        return output.getvalue()
