import os
import subprocess
import tempfile
import unittest
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from tests.unit.file_io_cases import read_source, round_trip_source, write_source


PROJECT = Path(__file__).resolve().parents[2]
PANACK = PROJECT / "panack"
RELEASE_VERSION = (PROJECT / "VERSION").read_text(encoding="utf-8").strip()
COMPILER_SOURCE = PROJECT / "src/compiler/main.panack"
CASES = PROJECT / "tests/functional/cases"
FAILURES = PROJECT / "tests/functional/failures"
EXAMPLE_OUTPUTS = PROJECT / "tests/functional/expected/examples"
DEFAULT_JOBS = min(4, os.cpu_count() or 1)


def test_jobs() -> int:
    value = int(os.environ.get("PANACK_TEST_JOBS", DEFAULT_JOBS))
    if value < 1:
        raise ValueError("PANACK_TEST_JOBS must be at least 1")
    return value


def run_panack(*arguments: str) -> subprocess.CompletedProcess[str]:
    environment = dict(os.environ)
    environment.pop("PANACKELTY_STDLIB_VALUE", None)
    return subprocess.run(
        [str(PANACK), *arguments],
        capture_output=True,
        text=True,
        env=environment,
    )


def compile_and_run(source: Path, bytecode: Path) -> tuple[
    subprocess.CompletedProcess[str], subprocess.CompletedProcess[str] | None
]:
    compiled = run_panack("compile", str(source), "-o", str(bytecode))
    if compiled.returncode != 0:
        return compiled, None
    return compiled, run_panack("run", str(bytecode))


def run_source_and_compiled(
    source: Path,
    bytecode: Path,
) -> tuple[
    subprocess.CompletedProcess[str],
    subprocess.CompletedProcess[str] | None,
    subprocess.CompletedProcess[str] | None,
]:
    source_result = run_panack("run", str(source))
    cached_compiler = os.environ.get("PANACK_TEST_COMPILER")
    if cached_compiler is not None and source.resolve() == COMPILER_SOURCE:
        compiled = None
        bytecode_result = run_panack("run", cached_compiler)
    else:
        compiled, bytecode_result = compile_and_run(source, bytecode)
    return source_result, compiled, bytecode_result


def discovered_programs() -> list[tuple[str, Path, str]]:
    programs: list[tuple[str, Path, str]] = []

    for case in sorted(path for path in CASES.iterdir() if path.is_dir()):
        local_source = case / "main.panack"
        source_reference = case / "source.path"
        if local_source.exists() == source_reference.exists():
            raise AssertionError(f"{case} must contain exactly one of main.panack or source.path")
        if source_reference.exists():
            source = (PROJECT / source_reference.read_text(encoding="utf-8").strip()).resolve()
            if not source.is_relative_to(PROJECT):
                raise AssertionError(f"{source_reference} points outside the repository")
        else:
            source = local_source
        expected = (case / "expected.stdout").read_text(encoding="utf-8")
        programs.append((f"case/{case.name}", source, expected))

    examples = {path.stem: path for path in (PROJECT / "examples").glob("*.panack")}
    example_outputs = {path.stem: path for path in EXAMPLE_OUTPUTS.glob("*.stdout")}
    if examples.keys() != example_outputs.keys():
        missing = sorted(examples.keys() - example_outputs.keys())
        stale = sorted(example_outputs.keys() - examples.keys())
        raise AssertionError(f"example output mismatch; missing={missing}, stale={stale}")
    for name, source in sorted(examples.items()):
        expected = example_outputs[name].read_text(encoding="utf-8")
        programs.append((f"example/{name}", source, expected))

    return programs


def discovered_failures() -> list[tuple[str, Path, str]]:
    failures: list[tuple[str, Path, str]] = []
    for case in sorted(path for path in FAILURES.iterdir() if path.is_dir()):
        source = case / "main.panack"
        expected = case / "expected.stderr"
        if not source.exists() or not expected.exists():
            raise AssertionError(f"{case} must contain main.panack and expected.stderr")
        failures.append((case.name, source, expected.read_text(encoding="utf-8")))
    return failures


class PanackeltyProgramTests(unittest.TestCase):
    def invoke(self, *arguments: str) -> subprocess.CompletedProcess[str]:
        result = run_panack(*arguments)
        self.assertEqual(
            result.returncode,
            0,
            msg=f"command failed: {result.args!r}\nstdout:\n{result.stdout}\nstderr:\n{result.stderr}",
        )
        return result

    def test_help_uses_panack_command_name(self):
        result = self.invoke("--help")
        self.assertIn("usage: panack ", result.stdout)
        self.assertEqual(result.stderr, "")

    def test_version_identifies_release_and_bytecode_format(self):
        result = self.invoke("--version")
        self.assertEqual(
            result.stdout,
            f"panack {RELEASE_VERSION} (bytecode 7)\n",
        )
        self.assertEqual(result.stderr, "")

    def test_source_and_compiled_program_outputs(self):
        with tempfile.TemporaryDirectory() as directory:
            output_directory = Path(directory)
            programs = discovered_programs()
            with ThreadPoolExecutor(max_workers=test_jobs()) as executor:
                results = [
                    executor.submit(
                        run_source_and_compiled,
                        source,
                        output_directory / f"program-{index}.bc",
                    )
                    for index, (_, source, _) in enumerate(programs)
                ]
            for (name, _, expected), future in zip(programs, results):
                with self.subTest(program=name):
                    source_result, compiled, result = future.result()
                    self.assertEqual(
                        source_result.returncode,
                        0,
                        source_result.stderr,
                    )
                    self.assertEqual(source_result.stdout, expected)
                    self.assertEqual(source_result.stderr, "")
                    if compiled is not None:
                        self.assertEqual(compiled.returncode, 0, compiled.stderr)
                    self.assertIsNotNone(result)
                    assert result is not None
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(result.stdout, expected)
                    self.assertEqual(result.stderr, "")

    def test_bare_source_path_runs_program(self):
        source = CASES / "hello_world/main.panack"
        result = self.invoke(str(source))
        self.assertEqual(result.stdout, "hello world\n")
        self.assertEqual(result.stderr, "")

    def test_run_passes_program_arguments(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "arguments.panack"
            source.write_text(
                "main(): Void { arguments: [Str] = command_args(); "
                "print(len(arguments)); print(arguments[0]); }",
                encoding="utf-8",
            )
            result = subprocess.run(
                [str(PANACK), "run", str(source), "alpha", "beta"],
                capture_output=True,
                text=True,
            )
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stdout, "2\nalpha\n")
        self.assertEqual(result.stderr, "")

    def test_program_controls_stderr_and_exit_status(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "failure.panack"
            source.write_text(
                'main(): Void { eprint("failure"); process_exit(7); }',
                encoding="utf-8",
            )
            result = subprocess.run(
                [str(PANACK), "run", str(source)],
                capture_output=True,
                text=True,
            )
        self.assertEqual(result.returncode, 7)
        self.assertEqual(result.stdout, "")
        self.assertEqual(result.stderr, "failure\n")

    def test_standard_library_reads_the_process_environment(self):
        environment = dict(os.environ)
        environment["PANACKELTY_STDLIB_VALUE"] = "configured"
        source = CASES / "stdlib/main.panack"
        result = subprocess.run(
            [str(PANACK), "run", str(source)],
            capture_output=True,
            text=True,
            env=environment,
        )
        self.assertEqual(result.returncode, 0)
        self.assertTrue(result.stdout.endswith(".tar\nconfigured\n"))
        self.assertEqual(result.stderr, "")

    def test_public_cli_file_io_round_trips_and_failures(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            text_path = root / "cli-text.txt"
            bytes_path = root / "cli-data.bin"
            source = root / "file-io.panack"
            source.write_text(
                round_trip_source(text_path, bytes_path), encoding="utf-8"
            )

            result = self.invoke("run", str(source))
            self.assertEqual(result.stdout, "λ-data\nbytes(00ff41)\n")

            bytecode = root / "file-io.bc"
            self.invoke("compile", str(source), "-o", str(bytecode))
            result = self.invoke("run", str(bytecode))
            self.assertEqual(result.stdout, "λ-data\nbytes(00ff41)\n")
            self.assertEqual(text_path.read_text(encoding="utf-8"), "λ-data")
            self.assertEqual(bytes_path.read_bytes(), b"\x00\xffA")

            invalid_utf8 = root / "invalid-utf8.txt"
            invalid_utf8.write_bytes(b"\xff")
            failures = (
                read_source("read_file", root / "missing.txt"),
                read_source("read_bytes", root / "missing.bin"),
                write_source("write_file", root / "missing/text.txt"),
                write_source("write_bytes", root / "missing/data.bin"),
                read_source("read_file", invalid_utf8),
            )
            for index, failure_source in enumerate(failures):
                with self.subTest(failure=index):
                    failure = root / f"failure-{index}.panack"
                    failure.write_text(failure_source, encoding="utf-8")
                    result = run_panack("run", str(failure))
                    self.assertEqual(result.returncode, 1)
                    self.assertEqual(result.stdout, "")
                    self.assertIn("I/O error:", result.stderr)

    @unittest.skipIf(
        not hasattr(os, "geteuid") or os.geteuid() == 0,
        "permission denial requires an unprivileged POSIX process",
    )
    def test_public_cli_reports_denied_file_io(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            denied = root / "denied.txt"
            denied.write_text("secret", encoding="utf-8")
            denied.chmod(0)
            try:
                for service in ("read_file", "read_bytes", "write_file", "write_bytes"):
                    source_text = (
                        read_source(service, denied)
                        if service.startswith("read")
                        else write_source(service, denied)
                    )
                    source = root / f"denied-{service}.panack"
                    source.write_text(source_text, encoding="utf-8")
                    with self.subTest(service=service):
                        result = run_panack("run", str(source))
                        self.assertEqual(result.returncode, 1)
                        self.assertEqual(result.stdout, "")
                        self.assertIn("I/O error:", result.stderr)
            finally:
                denied.chmod(0o600)

    def test_self_hosted_compiler_driver_matches_bootstrap_artifacts(self):
        compiler = PROJECT / "src/compiler/main.panack"
        source = CASES / "hello_world/main.panack"
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            cached_compiler = os.environ.get("PANACK_TEST_COMPILER")
            if cached_compiler is None:
                compiler_artifact = root / "compiler.bc"
                self.invoke("compile", str(compiler), "-o", str(compiler_artifact))
            else:
                compiler_artifact = Path(cached_compiler)
                checked = self.invoke("check", str(compiler_artifact))
                self.assertEqual(checked.stdout, "ok\n")

            check = self.invoke("run", str(compiler_artifact), "check", str(source))
            self.assertEqual(check.stdout, "ok\n")

            run = self.invoke("run", str(compiler_artifact), "run", str(source))
            self.assertEqual(run.stdout, "hello world\n")

            self_hosted = root / "self-hosted.bc"
            bootstrap = root / "bootstrap.bc"
            compiled = self.invoke(
                "run",
                str(compiler_artifact),
                "--",
                "compile",
                str(source),
                "-o",
                str(self_hosted),
            )
            self.assertEqual(compiled.stdout, f"wrote {self_hosted}\n")
            self.invoke("compile", str(source), "-o", str(bootstrap))
            self.assertEqual(self_hosted.read_bytes(), bootstrap.read_bytes())

            source_disassembly = self.invoke(
                "run", str(compiler_artifact), "disasm", str(source)
            )
            bytecode_disassembly = self.invoke(
                "run", str(compiler_artifact), "disasm", str(self_hosted)
            )
            self.assertEqual(bytecode_disassembly.stdout, source_disassembly.stdout)

    def test_legacy_source_extension_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "legacy.nu"
            source.write_text("main(): Void {}", encoding="utf-8")

            result = subprocess.run(
                [str(PANACK), "run", str(source)],
                capture_output=True,
                text=True,
            )

        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, "")
        self.assertEqual(
            result.stderr,
            "error: expected a .panack source or .bc bytecode file\n",
        )

    def test_compile_default_output_and_bare_bytecode_path(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "hello.panack"
            source.write_text(
                (CASES / "hello_world/main.panack").read_text(encoding="utf-8"),
                encoding="utf-8",
            )

            result = self.invoke("compile", str(source))
            bytecode = source.with_suffix(".bc")
            self.assertEqual(result.stdout, f"wrote {bytecode}\n")
            self.assertEqual(result.stderr, "")
            self.assertTrue(bytecode.is_file())

            result = self.invoke(str(bytecode))
            self.assertEqual(result.stdout, "hello world\n")
            self.assertEqual(result.stderr, "")

    def test_check_accepts_source_and_bytecode(self):
        source = CASES / "hello_world/main.panack"
        result = self.invoke("check", str(source))
        self.assertEqual(result.stdout, "ok\n")
        self.assertEqual(result.stderr, "")

        with tempfile.TemporaryDirectory() as directory:
            bytecode = Path(directory) / "hello.bc"
            self.invoke("compile", str(source), "-o", str(bytecode))
            result = self.invoke("check", str(bytecode))
            self.assertEqual(result.stdout, "ok\n")
            self.assertEqual(result.stderr, "")

    def test_disasm_matches_for_source_and_bytecode(self):
        source = CASES / "hello_world/main.panack"
        source_disassembly = self.invoke("disasm", str(source))
        self.assertIn("FUNCTION|main|impure|\n", source_disassembly.stdout)
        self.assertIn("CALL|print|1\n", source_disassembly.stdout)
        self.assertEqual(source_disassembly.stderr, "")

        with tempfile.TemporaryDirectory() as directory:
            bytecode = Path(directory) / "hello.bc"
            self.invoke("compile", str(source), "-o", str(bytecode))
            bytecode_disassembly = self.invoke("disasm", str(bytecode))
            self.assertEqual(bytecode_disassembly.stdout, source_disassembly.stdout)
            self.assertEqual(bytecode_disassembly.stderr, "")

    def test_disasm_rejects_malformed_bytecode(self):
        with tempfile.TemporaryDirectory() as directory:
            bytecode = Path(directory) / "malformed.bc"
            bytecode.write_bytes(b"not panack bytecode")
            result = subprocess.run(
                [str(PANACK), "disasm", str(bytecode)],
                capture_output=True,
                text=True,
            )
            self.assertEqual(result.returncode, 1)
            self.assertEqual(result.stdout, "")
            self.assertEqual(
                result.stderr,
                "error: not a Panackelty bytecode file\n",
            )

    def assert_invalid_result(
        self,
        result: subprocess.CompletedProcess[str],
        source: Path,
        expected_stderr: str,
    ) -> None:
        normalized_stderr = result.stderr.replace(
            str(source.parent.resolve()),
            "<case>",
        )
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, "")
        self.assertEqual(normalized_stderr, expected_stderr)

    def test_invalid_source_programs_fail_check(self):
        failures = discovered_failures()
        with ThreadPoolExecutor(max_workers=test_jobs()) as executor:
            results = [
                executor.submit(run_panack, "check", str(source))
                for _, source, _ in failures
            ]
        for (name, source, expected_stderr), future in zip(failures, results):
            with self.subTest(program=name):
                result = future.result()
                self.assert_invalid_result(result, source, expected_stderr)

    def test_invalid_source_programs_fail_compile_without_artifacts(self):
        with tempfile.TemporaryDirectory() as directory:
            output_directory = Path(directory)
            failures = discovered_failures()
            bytecodes = [
                output_directory / f"invalid-{index}.bc"
                for index in range(len(failures))
            ]
            with ThreadPoolExecutor(max_workers=test_jobs()) as executor:
                results = [
                    executor.submit(
                        run_panack,
                        "compile",
                        str(source),
                        "-o",
                        str(bytecode),
                    )
                    for (_, source, _), bytecode in zip(failures, bytecodes)
                ]
            for (name, source, expected_stderr), bytecode, future in zip(
                failures, bytecodes, results
            ):
                with self.subTest(program=name):
                    result = future.result()
                    self.assert_invalid_result(result, source, expected_stderr)
                    self.assertFalse(bytecode.exists())


if __name__ == "__main__":
    unittest.main()
