import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import VM, build, bytecode_bytes


PROJECT = Path(__file__).resolve().parents[3]
COMPILER = PROJECT / "src/compiler"
BYTECODE = PROJECT / "src/bytecode"
STDLIB = PROJECT / "src/stdlib"


class SelfHostedDriverTests(unittest.TestCase):
    def materialize(self, root):
        root.mkdir(parents=True)
        compiler = root / "compiler"
        bytecode = root / "bytecode"
        compiler.mkdir()
        bytecode.mkdir()
        for path in COMPILER.glob("*.panack"):
            (compiler / path.name).write_text(
                path.read_text(encoding="utf-8"), encoding="utf-8"
            )
        for path in BYTECODE.glob("*.panack"):
            (bytecode / path.name).write_text(
                path.read_text(encoding="utf-8"), encoding="utf-8"
            )
        return compiler

    def invoke(self, root, arguments):
        compiler = self.materialize(root)
        rendered = ", ".join(json.dumps(argument) for argument in arguments)
        main = compiler / "test_main.panack"
        main.write_text(
            'import "driver.panack";\n'
            f"main(): Void {{ status: Nat = run_compiler_command([{rendered}]); "
            'if status != 0 { print("status ${status}"); } else {} }',
            encoding="utf-8",
        )
        stdout = io.StringIO()
        stderr = io.StringIO()
        with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
            VM(
                build(main),
                environment={"PANACKELTY_STDLIB_PATH": str(STDLIB)},
            ).run()
        return stdout.getvalue(), stderr.getvalue()

    def test_checks_compiles_disassembles_and_runs_source(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "program.panack"
            source.write_text(
                'pure answer(): Nat { 42 } main(): Void { print(answer()); }',
                encoding="utf-8",
            )

            self.assertEqual(self.invoke(root / "check", ["check", str(source)]), ("ok\n", ""))

            output = root / "program.bc"
            stdout, stderr = self.invoke(
                root / "compile", ["compile", str(source), "-o", str(output)]
            )
            self.assertEqual((stdout, stderr), (f"wrote {output}\n", ""))
            self.assertEqual(output.read_bytes(), bytecode_bytes(build(source)))

            stdout, stderr = self.invoke(root / "disasm", ["disasm", str(source)])
            self.assertIn("FUNCTION|main|impure|", stdout)
            self.assertIn("CALL|print|1", stdout)
            self.assertEqual(stderr, "")

            self.assertEqual(self.invoke(root / "run", ["run", str(source)]), ("42\n", ""))

            self.assertEqual(
                self.invoke(root / "artifact-check", ["check", str(output)]),
                ("ok\n", ""),
            )
            artifact_disassembly = self.invoke(
                root / "artifact-disasm", ["disasm", str(output)]
            )
            self.assertEqual(artifact_disassembly, (stdout, ""))
            self.assertEqual(
                self.invoke(root / "artifact-run", [str(output)]),
                ("42\n", ""),
            )

    def test_loads_relative_module_graph_and_matches_bootstrap_artifact(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "main.panack"
            dependency = root / "answer.panack"
            source.write_text(
                'import "answer.panack"; main(): Void { print(answer()); }',
                encoding="utf-8",
            )
            dependency.write_text(
                "pure answer(): Nat { 42 }", encoding="utf-8"
            )
            output = root / "self.bc"

            stdout, stderr = self.invoke(
                root / "tool", ["compile", str(source), "-o", str(output)]
            )

            self.assertEqual((stdout, stderr), (f"wrote {output}\n", ""))
            self.assertEqual(output.read_bytes(), bytecode_bytes(build(source)))

    def test_loads_logical_standard_library_and_project_imports(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            shared = root / "shared"
            nested = root / "nested"
            shared.mkdir()
            nested.mkdir()
            (shared / "answer.panack").write_text(
                "pure answer(): Nat { 42 }", encoding="utf-8"
            )
            (nested / "feature.panack").write_text(
                "import project/shared/answer\n"
                "pure feature(): Option[Nat] { Some(answer()) }",
                encoding="utf-8",
            )
            source = root / "main.panack"
            source.write_text(
                "import stdlib/option\n"
                'import "stdlib/option.panack"\n'
                'import "nested/feature.panack"\n'
                "main(): Void {\n"
                "  match feature() { Some(value) => print(value), None() => print(0) }\n"
                "}",
                encoding="utf-8",
            )
            output = root / "logical.bc"

            stdout, stderr = self.invoke(
                root / "logical-tool", ["compile", str(source), "-o", str(output)]
            )

            self.assertEqual((stdout, stderr), (f"wrote {output}\n", ""))
            self.assertEqual(output.read_bytes(), bytecode_bytes(build(source)))

    def test_reports_missing_modules_and_import_cycles(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            missing = root / "missing-main.panack"
            missing.write_text(
                'import "absent.panack"; main(): Void {}', encoding="utf-8"
            )
            stdout, stderr = self.invoke(
                root / "missing-tool", ["check", str(missing)]
            )
            self.assertEqual(stdout, "status 1\n")
            self.assertIn("missing source module", stderr)

            first = root / "first.panack"
            second = root / "second.panack"
            first.write_text(
                'import "second.panack"; main(): Void {}', encoding="utf-8"
            )
            second.write_text('import "first.panack";', encoding="utf-8")
            stdout, stderr = self.invoke(
                root / "cycle-tool", ["check", str(first)]
            )
            self.assertEqual(stdout, "status 1\n")
            self.assertIn("import cycle includes", stderr)


if __name__ == "__main__":
    unittest.main()
