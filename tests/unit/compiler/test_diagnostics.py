import tempfile
from pathlib import Path

from tests.unit.support import CompilerHarnessTestCase


class DiagnosticRenderingTests(CompilerHarnessTestCase):
    def render(self, source, line, column, *, file="example.panack", available=True):
        return self.run_harness(
            "compiler/diagnostics.panack",
            'main(): Void {\n'
            '  mut sources: Map[Str,Str] = map()\n'
            '  if command_args()[4] == "yes" {\n'
            '    sources = sources.put(command_args()[0], command_args()[3])\n'
            '  }\n'
            '  position: SourcePos = SourcePos(command_args()[0], 0, '
            'nat_from_str(command_args()[1]), nat_from_str(command_args()[2]))\n'
            '  print(diagnostic_with_source(Diagnostic("bad input", position), sources))\n'
            '}',
            [file, str(line), str(column), source, "yes" if available else "no"],
        )

    def test_ascii_line_and_multidigit_gutter(self):
        self.assertEqual(
            self.render("\n" * 11 + "  bad\n", 12, 3),
            "example.panack:12:3: bad input\n  12 |   bad\n     |   ^\n",
        )

    def test_tabs_unicode_and_controls_have_deterministic_display_columns(self):
        self.assertEqual(
            self.render("\tλ中🙂e\u0301 #", 1, 8),
            "example.panack:1:8: bad input\n"
            "  1 |     \\u{3bb}\\u{4e2d}\\u{1f642}e\\u{301} #\n"
            "    | " + " " * 37 + "^\n",
        )
        self.assertEqual(
            self.render("\\\x1b\t?", 1, 4),
            "example.panack:1:4: bad input\n"
            "  1 | " + "\\" * 3 + "u{1b}    ?\n    | " + " " * 12 + "^\n",
        )

    def test_crlf_and_eof_positions(self):
        for source, line, column, excerpt in (
            ("first\r\n\tbad\r\n", 2, 2, "  2 |     bad\n    |     ^\n"),
            ("abc", 1, 4, "  1 | abc\n    |    ^\n"),
            ("abc\n", 2, 1, "  2 | \n    | ^\n"),
            ("", 1, 1, "  1 | \n    | ^\n"),
            ("abc\r", 1, 4, "  1 | abc\\u{d}\n    |    ^\n"),
        ):
            with self.subTest(source=source, line=line, column=column):
                self.assertEqual(
                    self.render(source, line, column),
                    f"example.panack:{line}:{column}: bad input\n" + excerpt,
                )

    def test_missing_sources_and_invalid_positions_preserve_header(self):
        self.assertEqual(self.render("bad", 1, 1, file=""), "bad input\n")
        self.assertEqual(
            self.render("bad", 1, 1, available=False),
            "example.panack:1:1: bad input\n",
        )
        for line, column in ((0, 1), (1, 0), (2, 1), (1, 5)):
            with self.subTest(line=line, column=column):
                self.assertEqual(
                    self.render("bad", line, column),
                    f"example.panack:{line}:{column}: bad input\n",
                )

    def test_loader_retains_the_diagnosed_source_snapshot(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory).resolve() / "broken.panack"
            original = "main(): Void { print(missing) }\n"
            source.write_text(original, encoding="utf-8")
            output = self.run_harness(
                "compiler/driver.panack",
                'main(): Void {\n'
                '  loaded: LoadedProject = load_project(command_args()[0])\n'
                '  write_file(command_args()[0], "main(): Void {}")\n'
                '  print(diagnostic_messages(loaded.diagnostics, loaded.sources))\n'
                '}',
                [str(source)],
            )
            self.assertEqual(source.read_text(), "main(): Void {}")
            self.assertEqual(
                output,
                f"{source}:1:22: unknown name missing\n"
                "  1 | main(): Void { print(missing) }\n"
                "    |                      ^\n",
            )
