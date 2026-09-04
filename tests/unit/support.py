import contextlib
import io
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
