import subprocess
import shutil
import tempfile
import unittest
import os
import random
from fractions import Fraction
from pathlib import Path

from panackelty import build, bytecode_bytes


PROJECT = Path(__file__).resolve().parents[3]
CASES = PROJECT / "tests/functional/cases"
EXAMPLES = PROJECT / "examples"
EXPECTED = PROJECT / "tests/functional/expected/examples"


class NativeExecutionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory()
        cls.executable = Path(cls.temporary.name) / "panack-vm"
        override = os.environ.get("PANACK_NATIVE_BINARY")
        if not override:
            result = subprocess.run(
                ["make", "--no-print-directory", "native"], cwd=PROJECT,
                capture_output=True, text=True,
            )
            if result.returncode:
                raise AssertionError(result.stdout + result.stderr)
        shutil.copy2(Path(override) if override else PROJECT / "panack-vm", cls.executable)

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


    def test_rational_arithmetic_matches_fraction_oracle(self):
        randomizer = random.Random(812)
        statements, expected = [], []
        for _ in range(24):
            a, b = randomizer.randint(-10**25, 10**25), randomizer.randint(1, 10**12)
            c, d = randomizer.randint(1, 10**25), randomizer.randint(1, 10**12)
            left, right = Fraction(a, b), Fraction(c, d)
            for op, value in (("+", left + right), ("-", left - right),
                              ("*", left * right), ("/", left / right)):
                statements.append(f"print(({a}/{b}) {op} ({c}/{d}))")
                expected.append(f"{value.numerator}/{value.denominator}")
        result = self.run_source("main(): Void { " + "; ".join(statements) + " }", "rat-oracle")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "\n".join(expected) + "\n")


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


if __name__ == "__main__":
    unittest.main()
