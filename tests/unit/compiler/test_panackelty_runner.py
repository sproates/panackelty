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
    def test_smoke_rejects_mismatched_or_missing_captured_report(self):
        with tempfile.TemporaryDirectory() as temporary:
            report = pathlib.Path(temporary) / "runner-report.txt"
            environment = dict(os.environ, PANACK_TEST_RUNNER_REPORT=str(report))

            def run():
                return subprocess.run(
                    [str(ROOT / "panack"), "run",
                     "tests/functional/cases/runner_smoke/main.panack"],
                    cwd=ROOT, env=environment, capture_output=True,
                    timeout=30, check=False,
                )

            report.write_bytes(b"wrong runner output\n")
            mismatch = run()
            self.assertEqual(mismatch.returncode, 1, mismatch.stderr)
            self.assertIn(b"FAIL Panackelty fixture runner: runner report differed",
                          mismatch.stdout)

            report.unlink()
            missing = run()
            self.assertEqual(missing.returncode, 1, missing.stderr)
            self.assertIn(b"FAIL Panackelty fixture runner: runner report read failed",
                          missing.stdout)

    def test_failure_commands_cover_run_and_disassembly(self):
        completed = subprocess.run(
            [str(ROOT / "panack"), "run", "tests/runner/main.panack",
             "--failure", "unknown_name"], cwd=ROOT, capture_output=True,
            timeout=30, check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        for command in (b"check", b"compile", b"run", b"disasm"):
            self.assertIn(b"PASS failure/unknown_name/" + command, completed.stdout)
        self.assertIn(b"PASS failure/unknown_name/no artifact", completed.stdout)
        self.assertIn(b"tests: 7, failures: 0", completed.stdout)

    def test_failed_compilation_corpus_is_available_to_incremental_check(self):
        completed = subprocess.run(
            [str(ROOT / "panack"), "run", "tests/runner/main.panack",
             "--failures-only"], cwd=ROOT, capture_output=True,
            timeout=45, check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        self.assertIn(b"PASS failure/while_condition_type/no artifact", completed.stdout)
        self.assertIn(b"tests: 137, failures: 0", completed.stdout)

    def test_cli_environment_files_case_runs_through_public_runner(self):
        completed = subprocess.run(
            [str(ROOT / "panack"), "run", "tests/runner/main.panack",
             "--case", "cli_environment_files"], cwd=ROOT, capture_output=True,
            timeout=30, check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        self.assertIn(b"PASS case/cli_environment_files/source", completed.stdout)
        self.assertIn(b"PASS case/cli_environment_files/bytecode", completed.stdout)
        self.assertIn(b"tests: 4, failures: 0", completed.stdout)

    def test_cli_commands_case_runs_through_public_runner(self):
        completed = subprocess.run(
            [str(ROOT / "panack"), "run", "tests/runner/main.panack",
             "--case", "cli_commands"], cwd=ROOT, capture_output=True,
            timeout=30, check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        self.assertIn(b"PASS case/cli_commands/source", completed.stdout)
        self.assertIn(b"PASS case/cli_commands/bytecode", completed.stdout)
        self.assertIn(b"tests: 4, failures: 0", completed.stdout)

    def test_cli_check_disasm_case_runs_through_public_runner(self):
        completed = subprocess.run(
            [str(ROOT / "panack"), "run", "tests/runner/main.panack",
             "--case", "cli_check_disasm"], cwd=ROOT, capture_output=True,
            timeout=30, check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        self.assertIn(b"PASS case/cli_check_disasm/source", completed.stdout)
        self.assertIn(b"PASS case/cli_check_disasm/bytecode", completed.stdout)
        self.assertIn(b"tests: 4, failures: 0", completed.stdout)

    def test_failure_diagnostics_and_artifact_cleanup(self):
        with tempfile.TemporaryDirectory() as temporary:
            checkout = pathlib.Path(temporary) / "checkout with spaces (test)"
            checkout.mkdir()
            (checkout / "panack").symlink_to(ROOT / "panack")
            (checkout / "src").symlink_to(ROOT / "src")
            (checkout / "tests/functional/cases").mkdir(parents=True)
            fixture = checkout / "tests/functional/failures/imported_unknown_name"
            fixture.mkdir(parents=True)
            original = ROOT / "tests/functional/failures/imported_unknown_name"
            for filename in ("main.panack", "dependency.panack", "expected.stderr"):
                shutil.copyfile(original / filename, fixture / filename)

            def run():
                return subprocess.run(
                    [str(checkout / "panack"), "run", str(ROOT / "tests/runner/main.panack"),
                     "--failure", "imported_unknown_name"], cwd=checkout,
                    capture_output=True, timeout=30, check=False,
                )

            valid = run()
            self.assertEqual(valid.returncode, 0, valid.stderr + valid.stdout)
            self.assertIn(b"PASS failure/imported_unknown_name/check", valid.stdout)
            self.assertIn(b"PASS failure/imported_unknown_name/compile", valid.stdout)
            self.assertIn(b"PASS failure/imported_unknown_name/no artifact", valid.stdout)

            (fixture / "expected.stderr").write_bytes(b"wrong diagnostic\n")
            mismatch = run()
            self.assertEqual(mismatch.returncode, 1, mismatch.stderr)
            self.assertIn(b"FAIL failure/imported_unknown_name/check", mismatch.stdout)
            self.assertIn(b"FAIL failure/imported_unknown_name/compile", mismatch.stdout)
            self.assertIn(b"PASS failure/imported_unknown_name/no artifact", mismatch.stdout)
            self.assertNotIn(b"FAIL workspace cleanup", mismatch.stdout)

            (fixture / "main.panack").write_text(
                "main(): Void { print(42) }\n", encoding="utf-8"
            )
            compiled = run()
            self.assertEqual(compiled.returncode, 1, compiled.stderr)
            self.assertIn(b"FAIL failure/imported_unknown_name/no artifact", compiled.stdout)
            self.assertNotIn(b"FAIL workspace cleanup", compiled.stdout)

            (fixture / "expected.stderr").unlink()
            missing = run()
            self.assertEqual(missing.returncode, 1, missing.stderr)
            self.assertIn(b"FAIL failure/imported_unknown_name/fixture", missing.stdout)

    def test_example_output_mismatch_and_missing_pair_fail(self):
        with tempfile.TemporaryDirectory() as temporary:
            checkout = pathlib.Path(temporary)
            (checkout / "panack").symlink_to(ROOT / "panack")
            (checkout / "src").symlink_to(ROOT / "src")
            (checkout / "tests/functional/cases").mkdir(parents=True)
            sources = checkout / "examples"
            sources.mkdir()
            outputs = checkout / "tests/functional/expected/examples"
            outputs.mkdir(parents=True)
            shutil.copyfile(ROOT / "examples/vm_loop.panack", sources / "vm_loop.panack")
            expected = outputs / "vm_loop.stdout"
            expected.write_bytes(b"wrong output\n")

            def run(name):
                return subprocess.run(
                    [str(checkout / "panack"), "run", str(ROOT / "tests/runner/main.panack"),
                     "--example", name], cwd=checkout, capture_output=True,
                    timeout=30, check=False,
                )

            mismatch = run("vm_loop")
            self.assertEqual(mismatch.returncode, 1, mismatch.stderr)
            self.assertIn(b"FAIL example/vm_loop/source", mismatch.stdout)
            self.assertIn(b"FAIL example/vm_loop/bytecode", mismatch.stdout)
            self.assertIn(b"PASS selected example count", mismatch.stdout)
            self.assertNotIn(b"FAIL workspace cleanup", mismatch.stdout)

            expected.unlink()
            missing_output = run("vm_loop")
            self.assertEqual(missing_output.returncode, 1, missing_output.stderr)
            self.assertIn(b"FAIL example/vm_loop/fixture: missing expected stdout", missing_output.stdout)

            (outputs / "orphan.stdout").write_bytes(b"stale\n")
            missing_source = run("orphan")
            self.assertEqual(missing_source.returncode, 1, missing_source.stderr)
            self.assertIn(b"FAIL example/orphan/fixture: missing example source", missing_source.stdout)

    def test_nonempty_workspace_fails_and_is_recovered(self):
        with tempfile.TemporaryDirectory() as temporary:
            completed = subprocess.run(
                [str(ROOT / "panack"), "run", "tests/runner/main.panack",
                 "--test-cleanup-failure", temporary, "--case", "hello_world"],
                cwd=ROOT, capture_output=True, timeout=45, check=False,
            )
            self.assertEqual(completed.returncode, 1, completed.stderr)
            self.assertIn(b"FAIL workspace cleanup", completed.stdout)
            self.assertIn(b"tests: 5, failures: 1", completed.stdout)
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
                [str(checkout / "panack"), "run", str(ROOT / "tests/runner/main.panack"),
                 "--case", "hello_world"],
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
            [str(ROOT / "panack"), "run", "tests/runner/main.panack", "--case", "stdlib"],
            cwd=ROOT, env=environment, capture_output=True, timeout=45,
            check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        self.assertIn(b"PASS case/stdlib/source", completed.stdout)
        self.assertIn(b"PASS case/stdlib/bytecode", completed.stdout)
        self.assertIn(b"tests: 4, failures: 0", completed.stdout)

    def test_source_path_rejects_escape_and_symlink(self):
        with tempfile.TemporaryDirectory() as temporary:
            checkout = pathlib.Path(temporary) / "checkout"
            checkout.mkdir()
            (checkout / "panack").symlink_to(ROOT / "panack")
            fixture = checkout / "tests/functional/cases/compiler_skeleton"
            fixture.mkdir(parents=True)
            reference = fixture / "source.path"
            outside = pathlib.Path(temporary) / "outside.panack"
            outside.write_text("main(): Void { print(42) }\n", encoding="utf-8")
            (fixture / "outside.panack").symlink_to(outside)
            for target in ("../outside.panack", str(outside),
                           "tests/functional/cases/compiler_skeleton/outside.panack"):
                reference.write_text(target + "\n", encoding="utf-8")
                completed = subprocess.run(
                    [str(checkout / "panack"), "run", str(ROOT / "tests/runner/main.panack"),
                     "--case", "compiler_skeleton"],
                    cwd=checkout, capture_output=True, timeout=20, check=False,
                )
                self.assertEqual(completed.returncode, 1, completed.stderr)
                self.assertIn(b"FAIL case/compiler_skeleton/fixture", completed.stdout)
                self.assertNotIn(b"PASS case/compiler_skeleton/source", completed.stdout)


if __name__ == "__main__":
    unittest.main()
