import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import VM, build


PROJECT = Path(__file__).resolve().parents[3]
COMPILER = PROJECT / "src/compiler"


class SelfHostedLexerTests(unittest.TestCase):
    def run_program(self, main_source: str) -> str:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ("types.panack", "lexer.panack"):
                (root / name).write_text(
                    (COMPILER / name).read_text(encoding="utf-8"),
                    encoding="utf-8",
                )
            main = root / "main.panack"
            main.write_text(
                f'import "lexer.panack";\n{main_source}',
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(main)).run()
            return output.getvalue()

    def render_lex(self, source: str) -> str:
        literal = json.dumps(source)
        return self.run_program(f"main(): Void {{ print(render(lex({literal}))); }}")

    def test_tokenizes_every_token_class_and_skips_comments(self):
        source = '// ignored\n_name42 17 3.14 "a\\\\b"'
        self.assertEqual(
            self.render_lex(source),
            "identifier:_name42\n"
            "number:17\n"
            "decimal:3.14\n"
            'string:"a\\\\b"\n\n',
        )

    def test_tokenizes_complete_symbol_vocabulary_with_longest_match(self):
        source = "=> .. == != <= >= && || ( ) { } [ ] . , ; : = + - * / % < > !"
        expected_symbols = (
            "=>", "..", "==", "!=", "<=", ">=", "&&", "||",
            "(", ")", "{", "}", "[", "]", ".", ",", ";", ":", "=",
            "+", "-", "*", "/", "%", "<", ">", "!",
        )
        expected = "".join(f"punctuation:{symbol}\n" for symbol in expected_symbols) + "\n"
        self.assertEqual(self.render_lex(source), expected)

    def test_preserves_half_open_range_when_decimal_point_has_no_fraction(self):
        self.assertEqual(
            self.render_lex("1..2 3. 4.5"),
            "number:1\n"
            "punctuation:..\n"
            "number:2\n"
            "number:3\n"
            "punctuation:.\n"
            "decimal:4.5\n\n",
        )

    def test_reports_each_invalid_character_at_its_source_position(self):
        self.assertEqual(
            self.render_lex("valid\n# & | ?"),
            "2:1: unexpected character #\n"
            "2:3: unexpected character &\n"
            "2:5: unexpected character |\n"
            "2:7: unexpected character ?\n\n",
        )

    def test_reports_unterminated_plain_and_escaped_quote_strings(self):
        for source in ('"open', '"escaped\\"'):
            with self.subTest(source=source):
                self.assertEqual(
                    self.render_lex(source),
                    "1:1: unterminated string\n\n",
                )

    def test_token_offsets_are_half_open(self):
        source = json.dumps("name 42")
        output = self.run_program(
            "main(): Void {\n"
            f"  match lex({source}) {{\n"
            "    Ok(tokens) => {\n"
            "      for token in tokens {\n"
            "        text: Str = token.text;\n"
            "        start: Nat = token.start;\n"
            "        end: Nat = token.end;\n"
            '        print("${text}:${start}..${end}");\n'
            "      }\n"
            "    },\n"
            '    Error(diagnostics) => print("unexpected error")\n'
            "  };\n"
            "}",
        )
        self.assertEqual(output, "name:0..4\n42:5..7\n")

    def test_normalizes_only_terminating_line_breaks(self):
        source = (
            "first\nsecond\nthird +\nfour\nitems[\n0\n]\n"
            "if ready {\ncall()\n}\nelse {\nother()\n}\nlast"
        )
        self.assertEqual(
            self.render_lex(source),
            "identifier:first\n"
            "punctuation:;\n"
            "identifier:second\n"
            "punctuation:;\n"
            "identifier:third\n"
            "punctuation:+\n"
            "identifier:four\n"
            "punctuation:;\n"
            "identifier:items\n"
            "punctuation:[\n"
            "number:0\n"
            "punctuation:]\n"
            "punctuation:;\n"
            "identifier:if\n"
            "identifier:ready\n"
            "punctuation:{\n"
            "identifier:call\n"
            "punctuation:(\n"
            "punctuation:)\n"
            "punctuation:}\n"
            "identifier:else\n"
            "punctuation:{\n"
            "identifier:other\n"
            "punctuation:(\n"
            "punctuation:)\n"
            "punctuation:}\n"
            "punctuation:;\n"
            "identifier:last\n\n",
        )


if __name__ == "__main__":
    unittest.main()
