import contextlib
import io
import tempfile
import unittest
from pathlib import Path

from panackelty import VM, build


class TestingFilesTests(unittest.TestCase):
    def test_invalid_fixture_roots_are_structured_errors(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "main.panack"
            source.write_text(
                "import stdlib/testing_files\n"
                "main(): Void {\n"
                f'  missing = result_value_or(path_from_text("{directory}/missing"), path_current())\n'
                "  match test_discover_fixtures(missing) {\n"
                '    Error(error) => { print(error.code) },\n'
                '    Ok(fixtures) => { print("unexpected") }\n'
                "  }\n"
                f'  regular = result_value_or(path_from_text("{directory}/main.panack"), path_current())\n'
                "  match test_discover_fixtures(regular) {\n"
                '    Error(error) => { print(error.code) },\n'
                '    Ok(fixtures) => { print("unexpected") }\n'
                "  }\n"
                "}\n",
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(source)).run()
        self.assertEqual(output.getvalue(), "not_found\nnot_directory\n")


if __name__ == "__main__":
    unittest.main()
