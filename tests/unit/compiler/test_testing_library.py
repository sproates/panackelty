import contextlib
import io
import tempfile
import unittest
from pathlib import Path

from panackelty import VM, build


class TestingLibraryTests(unittest.TestCase):
    def run_program(self, body: str) -> str:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.panack"
            source.write_text(
                "import stdlib/testing\nmain(): Void {\n" + body + "\n}\n",
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(source)).run()
            return output.getvalue()

    def test_empty_report_has_zero_failures(self):
        self.assertEqual(
            self.run_program(
                "  results: [TestResult] = []\n"
                "  failures = test_report(results)\n"
                "  print(failures)"
            ),
            "tests: 0, failures: 0\n0\n",
        )

    def test_failed_condition_retains_reason_and_order(self):
        self.assertEqual(
            self.run_program(
                '  results: [TestResult] = [test_expect("first", false, "reason"), '
                'test_expect("second", true, "unused")]\n'
                "  print(test_report(results))"
            ),
            "FAIL first: reason\nPASS second\ntests: 2, failures: 1\n1\n",
        )


if __name__ == "__main__":
    unittest.main()
