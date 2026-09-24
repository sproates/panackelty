"""Failure propagation for the transitional Panackelty fixture runner."""

import pathlib
import os
import shutil
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[3]
CASES = (
    "callables", "collections", "compiler_lexer", "hello_world",
    "host_capabilities", "host_process", "host_types", "local_inference",
    "modules", "optional_else", "rational_unit", "records_and_enums",
    "semicolonless", "stdlib", "string_boundaries", "testing_commands",
    "testing_fixtures", "testing_library",
    "vm_numeric_boundaries",
)


class PanackeltyRunnerTests(unittest.TestCase):
    def test_nonempty_workspace_fails_and_is_recovered(self):
        with tempfile.TemporaryDirectory() as temporary:
            completed = subprocess.run(
                [str(ROOT / "panack"), "run", "tests/runner/main.panack",
                 "--test-cleanup-failure", temporary],
                cwd=ROOT, capture_output=True, timeout=45, check=False,
            )
            self.assertEqual(completed.returncode, 1, completed.stderr)
            self.assertIn(b"FAIL workspace cleanup", completed.stdout)
            self.assertIn(b"tests: 59, failures: 1", completed.stdout)
            self.assertNotIn(b"FAIL cleanup recovery", completed.stdout)
            self.assertNotIn(b"FAIL workspace recovery", completed.stdout)
            self.assertEqual(list(pathlib.Path(temporary).iterdir()), [])

    def test_expected_output_mismatch_fails_the_runner(self):
        with tempfile.TemporaryDirectory() as temporary:
            checkout = pathlib.Path(temporary)
            (checkout / "panack").symlink_to(ROOT / "panack")
            (checkout / "src").symlink_to(ROOT / "src")
            fixtures = checkout / "tests" / "functional" / "cases"
            for name in CASES:
                destination = fixtures / name
                destination.mkdir(parents=True)
                original = ROOT / "tests" / "functional" / "cases" / name
                shutil.copyfile(original / "main.panack", destination / "main.panack")
                shutil.copyfile(original / "expected.stdout", destination / "expected.stdout")
            (fixtures / "hello_world" / "expected.stdout").write_bytes(b"wrong output\n")
            completed = subprocess.run(
                [str(checkout / "panack"), "run", str(ROOT / "tests/runner/main.panack")],
                cwd=checkout, capture_output=True, timeout=45, check=False,
            )
            self.assertEqual(completed.returncode, 1, completed.stderr)
            self.assertIn(b"FAIL case/hello_world/source", completed.stdout)
            self.assertIn(b"FAIL case/hello_world/bytecode", completed.stdout)
            self.assertIn(b"PASS selected fixture count", completed.stdout)
            self.assertNotIn(b"FAIL workspace cleanup", completed.stdout)

    def test_stdlib_fixture_discards_inherited_value(self):
        environment = dict(os.environ, PANACKELTY_STDLIB_VALUE="ambient-test")
        completed = subprocess.run(
            [str(ROOT / "panack"), "run", "tests/runner/main.panack"],
            cwd=ROOT, env=environment, capture_output=True, timeout=45,
            check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        self.assertIn(b"PASS case/stdlib/source", completed.stdout)
        self.assertIn(b"PASS case/stdlib/bytecode", completed.stdout)
        self.assertIn(b"tests: 58, failures: 0", completed.stdout)


if __name__ == "__main__":
    unittest.main()
