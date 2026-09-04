import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import Code, PanackeltyError, VM, build, load_bytecode, verify_bytecode
from tests.unit.support import PanackeltyTestCase


class ImportTests(PanackeltyTestCase):
    def test_relative_module_imports(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "math.panack").write_text(
                "pure answer(): Nat { 6 * 7 }",
                encoding="utf-8",
            )
            main = root / "main.panack"
            main.write_text(
                'import "math.panack"; main(): Void { print(answer()); }',
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(main)).run()
            self.assertEqual(output.getvalue(), "42\n")

    def test_logical_standard_library_imports_are_extensionless_and_load_once(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            local_stdlib = root / "stdlib"
            local_stdlib.mkdir()
            (local_stdlib / "option.panack").write_text(
                "# logical stdlib imports must not load this shadow",
                encoding="utf-8",
            )
            main = root / "main.panack"
            main.write_text(
                "import stdlib/option\n"
                'import "stdlib/option.panack"\n'
                "pure describe(value: Option[Nat]): Nat {\n"
                "  match value { Some(number) => number, None() => 0 }\n"
                "}\n"
                "main(): Void { print(describe(Some(42))) }\n",
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(main)).run()
            self.assertEqual(output.getvalue(), "42\n")

    def test_project_imports_are_rooted_at_the_entry_module(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            library = root / "shared"
            nested = root / "app"
            library.mkdir()
            nested.mkdir()
            (library / "answer.panack").write_text(
                "pure answer(): Nat { 42 }", encoding="utf-8"
            )
            (nested / "feature.panack").write_text(
                "import project/shared/answer\n"
                "pure feature(): Nat { answer() }",
                encoding="utf-8",
            )
            main = root / "main.panack"
            main.write_text(
                'import "app/feature.panack"\n'
                "main(): Void { print(feature()) }",
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(main)).run()
            self.assertEqual(output.getvalue(), "42\n")

    def test_invalid_logical_imports_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            invalid = (
                ('import "stdlib/../option"', "invalid logical import path"),
                ('import "stdlib/option.txt"', "logical import extension must be .panack"),
            )
            for index, (declaration, message) in enumerate(invalid):
                with self.subTest(declaration=declaration):
                    main = root / f"invalid-{index}.panack"
                    main.write_text(declaration, encoding="utf-8")
                    with self.assertRaisesRegex(PanackeltyError, message):
                        build(main)

    def test_import_cycles_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            first = root / "first.panack"
            second = root / "second.panack"
            first.write_text('import "second.panack";', encoding="utf-8")
            second.write_text('import "first.panack";', encoding="utf-8")
            with self.assertRaisesRegex(PanackeltyError, "import cycle"):
                build(first)

if __name__ == "__main__":
    unittest.main()
