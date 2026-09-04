import subprocess
import tempfile
import unittest
import os
from pathlib import Path

from panackelty import Code, build, bytecode_bytes
from tests.unit.file_io_cases import (
    nul_path_source,
    read_source,
    round_trip_source,
    write_source,
)
from tests.unit.forged_runtime import FORGED_DYNAMIC_FAILURES


PROJECT = Path(__file__).resolve().parents[3]
VM = PROJECT / "src/vm"
CASES = PROJECT / "tests/functional/cases"
EXAMPLES = PROJECT / "examples"
EXPECTED = PROJECT / "tests/functional/expected/examples"


class NativeExecutionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory()
        cls.executable = Path(cls.temporary.name) / "panack-vm"
        result = subprocess.run(
            [
                "cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
                str(VM / "native.c"), str(VM / "bigint.c"),
                "-o", str(cls.executable),
            ], capture_output=True, text=True,
        )
        if result.returncode:
            raise AssertionError(result.stderr)

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def programs(self):
        for case in sorted(path for path in CASES.iterdir() if path.is_dir()):
            source = case / "main.panack"
            if source.exists():
                yield f"case/{case.name}", source, (case / "expected.stdout").read_text(encoding="utf-8")
        for source in sorted(EXAMPLES.glob("*.panack")):
            yield f"example/{source.stem}", source, (EXPECTED / f"{source.stem}.stdout").read_text(encoding="utf-8")

    def run_source(self, source_text: str, name: str) -> subprocess.CompletedProcess[str]:
        root = Path(self.temporary.name)
        source = root / f"{name}.panack"
        artifact = root / f"{name}.bc"
        source.write_text(source_text, encoding="utf-8")
        artifact.write_bytes(bytecode_bytes(build(source)))
        return subprocess.run(
            [str(self.executable), "run", str(artifact)],
            capture_output=True,
            text=True,
        )

    def test_native_vm_matches_program_outputs(self):
        root = Path(self.temporary.name)
        for index, (name, source, expected) in enumerate(self.programs()):
            with self.subTest(program=name):
                artifact = root / f"program-{index}.bc"
                artifact.write_bytes(bytecode_bytes(build(source)))
                result = subprocess.run(
                    [str(self.executable), "run", str(artifact)],
                    capture_output=True,
                    text=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected)
                self.assertEqual(result.stderr, "")

    def test_native_vm_matches_numeric_and_trap_semantics(self):
        long_decimal = "1234567890" * 15 + ".0"
        scaled_decimal = str(int("1234567890" * 15) * 9) + ".00"
        cases = (
            (
                "main(): Void { print(999999999999999999999999999999 * 9); }",
                "8999999999999999999999999999991\n",
            ),
            (
                "main(): Void { mut n: Nat = 30; mut p: Nat = 1; "
                "while n > 0 { p = p * n; n = n - 1; } print(p); }",
                "265252859812191058636308480000000\n",
            ),
            (
                "main(): Void { print(0.1 + 0.2); print(1.0 / 8.0); }",
                "0.3\n0.125\n",
            ),
            (
                "main(): Void { print(-7 / 3); print(-7 % 3); print(7 % -3); "
                "print(1.0 == 1.00); print(is_letter(\"λ\")); }",
                "-2\n2\n-2\ntrue\nfalse\n",
            ),
            (
                f"main(): Void {{ print({long_decimal} * 9.0); }}",
                scaled_decimal + "\n",
            ),
        )
        root = Path(self.temporary.name)
        for index, (source_text, expected) in enumerate(cases):
            with self.subTest(source=source_text):
                source = root / f"numeric-{index}.panack"
                artifact = root / f"numeric-{index}.bc"
                source.write_text(source_text, encoding="utf-8")
                artifact.write_bytes(bytecode_bytes(build(source)))
                result = subprocess.run(
                    [str(self.executable), "run", str(artifact)],
                    capture_output=True,
                    text=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected)

    def test_native_vm_runs_the_self_hosted_compiler(self):
        root = Path(self.temporary.name)
        compiler = root / "compiler.bc"
        compiler.write_bytes(
            bytecode_bytes(build(PROJECT / "src/compiler/main.panack"))
        )
        source = PROJECT / "examples/euler001.panack"

        checked = subprocess.run(
            [str(self.executable), "run", str(compiler), "check", str(source)],
            capture_output=True,
            text=True,
        )
        self.assertEqual(checked.returncode, 0, checked.stderr)
        self.assertEqual(checked.stdout, "ok\n")

        native_artifact = root / "native.bc"
        compiled = subprocess.run(
            [
                str(self.executable), "run", str(compiler), "compile",
                str(source), "-o", str(native_artifact),
            ],
            capture_output=True,
            text=True,
        )
        self.assertEqual(compiled.returncode, 0, compiled.stderr)
        self.assertEqual(native_artifact.read_bytes(), bytecode_bytes(build(source)))

        executed = subprocess.run(
            [str(self.executable), "run", str(compiler), "run", str(source)],
            capture_output=True,
            text=True,
        )
        self.assertEqual(executed.returncode, 0, executed.stderr)
        self.assertEqual(executed.stdout, "233168\n")

    def test_native_host_boundary_passes_arguments_environment_and_status(self):
        root = Path(self.temporary.name)
        source = root / "host.panack"
        artifact = root / "host.bc"
        source.write_text(
            'main(): Void { args: [Str] = command_args(); print(args[0]); '
            'print(environment_get("PANACK_NATIVE_TEST")); '
            'eprint("native stderr"); process_exit(7); }',
            encoding="utf-8",
        )
        artifact.write_bytes(bytecode_bytes(build(source)))
        environment = dict(os.environ)
        environment["PANACK_NATIVE_TEST"] = "snapshot"
        result = subprocess.run(
            [str(self.executable), "run", str(artifact), "argument"],
            capture_output=True,
            text=True,
            env=environment,
        )
        self.assertEqual(result.returncode, 7)
        self.assertEqual(result.stdout, "argument\nsnapshot\n")
        self.assertEqual(result.stderr, "native stderr\n")

    def test_native_file_io_round_trips_and_failures(self):
        root = Path(self.temporary.name)
        text_path = root / "native-text.txt"
        bytes_path = root / "native-data.bin"
        result = self.run_source(
            round_trip_source(text_path, bytes_path), "file-round-trip"
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "λ-data\nbytes(00ff41)\n")
        self.assertEqual(text_path.read_text(encoding="utf-8"), "λ-data")
        self.assertEqual(bytes_path.read_bytes(), b"\x00\xffA")

        invalid_utf8 = root / "native-invalid-utf8.txt"
        invalid_utf8.write_bytes(b"\xff")
        cases = (
            (read_source("read_file", root / "native-missing.txt"), "missing-text"),
            (read_source("read_bytes", root / "native-missing.bin"), "missing-bytes"),
            (
                write_source("write_file", root / "missing/text.txt"),
                "missing-write-text",
            ),
            (
                write_source("write_bytes", root / "missing/data.bin"),
                "missing-write-bytes",
            ),
            (read_source("read_file", invalid_utf8), "invalid-utf8"),
        )
        for source, name in cases:
            with self.subTest(case=name):
                result = self.run_source(source, name)
                self.assertEqual(result.returncode, 1)
                self.assertIn("I/O error:", result.stderr)
        self.assertIn("file is not valid UTF-8", self.run_source(
            read_source("read_file", invalid_utf8), "invalid-utf8-detail"
        ).stderr)

    def test_native_file_io_rejects_embedded_nul_paths(self):
        for service in ("read_file", "read_bytes", "write_file", "write_bytes"):
            with self.subTest(service=service):
                result = self.run_source(nul_path_source(service), f"nul-{service}")
                self.assertEqual(result.returncode, 1)
                self.assertIn("VM trap: path contains NUL byte", result.stderr)

    @unittest.skipIf(
        not hasattr(os, "geteuid") or os.geteuid() == 0,
        "permission denial requires an unprivileged POSIX process",
    )
    def test_native_file_io_reports_denied_paths(self):
        denied = Path(self.temporary.name) / "native-denied.txt"
        denied.write_text("secret", encoding="utf-8")
        denied.chmod(0)
        try:
            for service in ("read_file", "read_bytes", "write_file", "write_bytes"):
                source = (
                    read_source(service, denied)
                    if service.startswith("read")
                    else write_source(service, denied)
                )
                with self.subTest(service=service):
                    result = self.run_source(source, f"denied-{service}")
                    self.assertEqual(result.returncode, 1)
                    self.assertIn("I/O error:", result.stderr)
        finally:
            denied.chmod(0o600)

    def test_native_vm_traps_on_forged_dynamic_failures(self):
        root = Path(self.temporary.name)
        for index, (name, instructions, messages) in enumerate(
            FORGED_DYNAMIC_FAILURES
        ):
            with self.subTest(case=name):
                artifact = root / f"trap-{index}.bc"
                artifact.write_bytes(
                    bytecode_bytes({"main": Code("main", [], instructions)})
                )
                result = subprocess.run(
                    [str(self.executable), "run", str(artifact)],
                    capture_output=True,
                    text=True,
                )
                self.assertEqual(result.returncode, 1)
                self.assertIn("VM trap:", result.stderr)
                for message in messages:
                    self.assertIn(message, result.stderr)

        invalid_operand = root / "trap-invalid-operand.bc"
        invalid_operand.write_bytes(
            bytecode_bytes(
                {
                    "main": Code(
                        "main",
                        [],
                        [
                            ("CONST", ("Nat", 1)),
                            ("CALL", ("len", 1)),
                            ("RETURN", None),
                        ],
                    )
                }
            )
        )
        result = subprocess.run(
            [str(self.executable), "run", str(invalid_operand)],
            capture_output=True,
            text=True,
        )
        self.assertEqual(result.returncode, 1)
        self.assertIn("len requires", result.stderr)

        source = root / "nonterminating.panack"
        artifact = root / "nonterminating.bc"
        source.write_text(
            "pure divide(a: Dec, b: Dec): Dec { a / b } "
            "main(): Void { print(divide(1.0, 3.0)); }",
            encoding="utf-8",
        )
        artifact.write_bytes(bytecode_bytes(build(source)))
        result = subprocess.run(
            [str(self.executable), "run", str(artifact)],
            capture_output=True,
            text=True,
        )
        self.assertEqual(result.returncode, 1)
        self.assertIn("non-terminating decimal division", result.stderr)

    def test_native_vm_rechecks_indirect_call_purity(self):
        root = Path(self.temporary.name)
        artifact = root / "indirect-purity.bc"
        artifact.write_bytes(
            bytecode_bytes(
                {
                    "main": Code(
                        "main",
                        [],
                        [
                            ("CONST", ("Str", "effect")),
                            ("CALL_VALUE", 0),
                            ("RETURN", None),
                        ],
                        pure=True,
                    ),
                    "effect": Code(
                        "effect",
                        [],
                        [("CONST", ("Void", None)), ("RETURN", None)],
                    ),
                }
            )
        )
        result = subprocess.run(
            [str(self.executable), "run", str(artifact)],
            capture_output=True,
            text=True,
        )
        self.assertEqual(result.returncode, 1)
        self.assertIn("pure function invokes impure callable", result.stderr)


if __name__ == "__main__":
    unittest.main()
