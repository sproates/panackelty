import contextlib
import io
import json
import os
import tempfile
import unittest
from pathlib import Path

from panackelty import (
    Code,
    PanackeltyError,
    PanackeltyProcessExit,
    VM,
    build,
    load_bytecode,
    verify_bytecode,
    write_bytecode,
)
from tests.unit.support import PanackeltyTestCase
from tests.unit.file_io_cases import (
    nul_path_source,
    read_source,
    round_trip_source,
    write_source,
)


class RuntimeTests(PanackeltyTestCase):
    def test_lexer_character_classes_are_deliberately_ascii(self):
        code = self.compile('main(): Void { print(is_letter("A")); '
                            'print(is_letter("λ")); print(is_digit("7")); '
                            'print(is_digit("٧")); print(is_whitespace("\\t")); }')
        self.assertEqual(self.run_code(code), "true\nfalse\ntrue\nfalse\ntrue\n")

    def test_text_and_binary_file_round_trips(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            text_path = root / "text.txt"
            bytes_path = root / "data.bin"
            code = self.compile(round_trip_source(text_path, bytes_path))
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(code).run()
            self.assertEqual(text_path.read_text(encoding="utf-8"), "λ-data")
            self.assertEqual(bytes_path.read_bytes(), b"\x00\xffA")
            self.assertEqual(output.getvalue(), "λ-data\nbytes(00ff41)\n")

    def test_file_io_failures_become_language_errors(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            invalid_utf8 = root / "invalid-utf8.txt"
            invalid_utf8.write_bytes(b"\xff")
            cases = (
                (read_source("read_file", root / "missing.txt"), "I/O error:"),
                (read_source("read_bytes", root / "missing.bin"), "I/O error:"),
                (
                    write_source("write_file", root / "missing/text.txt"),
                    "I/O error:",
                ),
                (
                    write_source("write_bytes", root / "missing/data.bin"),
                    "I/O error:",
                ),
                (
                    read_source("read_file", invalid_utf8),
                    "I/O error: file is not valid UTF-8",
                ),
            )
            for source, message in cases:
                with self.subTest(message=message, source=source):
                    with self.assertRaisesRegex(PanackeltyError, message):
                        VM(self.compile(source)).run()

    def test_file_io_rejects_paths_with_embedded_nul(self):
        for service in ("read_file", "read_bytes", "write_file", "write_bytes"):
            with self.subTest(service=service):
                with self.assertRaisesRegex(PanackeltyError, "path contains NUL"):
                    VM(self.compile(nul_path_source(service))).run()

    @unittest.skipIf(
        not hasattr(os, "geteuid") or os.geteuid() == 0,
        "permission denial requires an unprivileged POSIX process",
    )
    def test_file_io_reports_denied_paths(self):
        with tempfile.TemporaryDirectory() as directory:
            denied = Path(directory) / "denied.txt"
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
                        with self.assertRaisesRegex(PanackeltyError, "I/O error:"):
                            VM(self.compile(source)).run()
            finally:
                denied.chmod(0o600)

    def test_command_arguments_are_available_to_programs(self):
        code = self.compile("""
main(): Void {
  arguments: [Str] = command_args();
  print(len(arguments));
  print(arguments[0]);
}
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code, ["alpha", "beta"]).run()
        self.assertEqual(output.getvalue(), "2\nalpha\n")

    def test_environment_is_snapshotted_and_missing_values_trap(self):
        code = self.compile("""
main(): Void {
  print(environment_has("PANACK_PRESENT"));
  print(environment_get("PANACK_PRESENT"));
  print(environment_has("PANACK_MISSING"));
}
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code, environment={"PANACK_PRESENT": "configured"}).run()
        self.assertEqual(output.getvalue(), "true\nconfigured\nfalse\n")

        missing = self.compile(
            'main(): Void { print(environment_get("PANACK_MISSING")); }'
        )
        with self.assertRaisesRegex(
            PanackeltyError,
            "VM trap: environment variable 'PANACK_MISSING' was not found",
        ):
            VM(missing, environment={}).run()

    def test_stderr_and_process_exit_boundary(self):
        code = self.compile('main(): Void { eprint("failure"); process_exit(7); }')
        error = io.StringIO()
        with contextlib.redirect_stderr(error):
            with self.assertRaises(PanackeltyProcessExit) as raised:
                VM(code).run()
        self.assertEqual(raised.exception.code, 7)
        self.assertEqual(error.getvalue(), "failure\n")

    def test_path_operations_and_nested_bytecode_execution(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            nested_source = root / "nested.panack"
            nested_source.write_text(
                'main(): Void { print("nested"); }', encoding="utf-8"
            )
            nested_bytecode = root / "nested.bc"
            write_bytecode(build(nested_source), nested_bytecode)
            path_literal = json.dumps(str(nested_bytecode))
            code = self.compile(f"""
main(): Void {{
  print(path_suffix({path_literal}));
  print(file_exists(path_resolve({path_literal})));
  run_bytecode(read_bytes({path_literal}));
}}
""")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(code).run()
        self.assertEqual(output.getvalue(), ".bc\ntrue\nnested\n")

    def test_nested_bytecode_can_receive_an_explicit_argument_snapshot(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            nested_source = root / "arguments.panack"
            nested_source.write_text(
                "main(): Void { values: [Str] = command_args(); "
                "print(len(values)); print(values[0]); }",
                encoding="utf-8",
            )
            nested_bytecode = root / "arguments.bc"
            write_bytecode(build(nested_source), nested_bytecode)
            path_literal = json.dumps(str(nested_bytecode))
            code = self.compile(
                f'main(): Void {{ run_bytecode_args(read_bytes({path_literal}), ["child"]); }}'
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(code, ["parent"]).run()
        self.assertEqual(output.getvalue(), "1\nchild\n")

    def test_path_join_is_lexical_and_does_not_make_relative_paths_absolute(self):
        code = self.compile("""
main(): Void {
  print(path_join("project/src", "../tests"));
  print(path_join("/project/src", "../tests"));
}
""")
        self.assertEqual(self.run_code(code), "project/tests\n/project/tests\n")

if __name__ == "__main__":
    unittest.main()
