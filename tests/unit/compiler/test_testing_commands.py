import contextlib
import io
import tempfile
import unittest
from pathlib import Path

from panackelty import VM, build


class TestingCommandsTests(unittest.TestCase):
    def test_output_comparison_and_error_result_are_pure(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.panack"
            source.write_text(
                "import stdlib/testing_commands\n"
                "main(): Void {\n"
                '  expected = ProcessOutput(0, 0, utf8_encode("out"), utf8_encode("err"))\n'
                '  print(test_report([\n'
                '    test_command_output("match", expected, expected),\n'
                '    test_command_output("exit", ProcessOutput(2, 0, bytes(), bytes()), expected),\n'
                '    test_command_output("signal", ProcessOutput(0, 9, bytes(), bytes()), expected),\n'
                '    test_command_output("stdout", ProcessOutput(0, 0, bytes(), utf8_encode("err")), expected),\n'
                '    test_command_output("stderr", ProcessOutput(0, 0, utf8_encode("out"), bytes()), expected),\n'
                '    test_command_result("host", Error(HostError("process", "timeout", 0)), expected)\n'
                '  ]))\n'
                "}\n",
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(source)).run()
        self.assertEqual(
            output.getvalue(),
            "PASS match\n"
            "FAIL exit: expected exit 0, got 2\n"
            "FAIL signal: expected signal 0, got 9\n"
            "FAIL stdout: stdout differed\n"
            "FAIL stderr: stderr differed\n"
            "FAIL host: command failed: timeout\n"
            "tests: 6, failures: 5\n5\n",
        )


if __name__ == "__main__":
    unittest.main()
