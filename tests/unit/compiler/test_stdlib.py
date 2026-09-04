import contextlib
import io
import tempfile
import unittest
from pathlib import Path

from panackelty import VM, build, bytecode_bytes


PROJECT = Path(__file__).resolve().parents[3]
COMPILER = PROJECT / "src/compiler/main.panack"
STDLIB = PROJECT / "src/stdlib"
CONFORMANCE = PROJECT / "tests/functional/cases/stdlib/main.panack"


class StandardLibraryTests(unittest.TestCase):
    def test_complete_library_is_byte_identical_at_bootstrap_and_stage_one(self):
        bootstrap_artifact = bytecode_bytes(build(CONFORMANCE))
        compiler = build(COMPILER)

        with tempfile.TemporaryDirectory() as directory:
            stage_one_artifact = Path(directory) / "stdlib-conformance.bc"
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(
                    compiler,
                    ["compile", str(CONFORMANCE), "-o", str(stage_one_artifact)],
                    environment={"PANACKELTY_STDLIB_PATH": str(STDLIB)},
                ).run()

            self.assertEqual(
                output.getvalue(), f"wrote {stage_one_artifact}\n"
            )
            self.assertEqual(stage_one_artifact.read_bytes(), bootstrap_artifact)

    def test_environment_wrapper_returns_option_without_trapping(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "main.panack"
            source.write_text(
                "import stdlib/environment\n"
                "pure describe(value: Option[Str]): Str {\n"
                "  match value { Some(text) => text, None() => \"none\" }\n"
                "}\n"
                "main(): Void {\n"
                '  print(describe(environment("PRESENT")));\n'
                '  print(describe(environment("MISSING")));\n'
                "}\n",
                encoding="utf-8",
            )

            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(source), environment={"PRESENT": "configured"}).run()

        self.assertEqual(output.getvalue(), "configured\nnone\n")


if __name__ == "__main__":
    unittest.main()
