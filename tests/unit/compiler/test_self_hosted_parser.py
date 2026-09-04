import contextlib
import io
import tempfile
import unittest
from pathlib import Path

from panackelty import VM, build


PROJECT = Path(__file__).resolve().parents[3]
COMPILER = PROJECT / "src/compiler"


class SelfHostedParserTests(unittest.TestCase):
    programs = {}

    def render_source(self, function: str, source: str) -> str:
        if function not in self.programs:
            with tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                for name in ("types.panack", "lexer.panack", "parser.panack"):
                    (root / name).write_text(
                        (COMPILER / name).read_text(encoding="utf-8"),
                        encoding="utf-8",
                    )
                main = root / "main.panack"
                main.write_text(
                    'import "parser.panack";\n'
                    f"main(): Void {{ print({function}(command_args()[0])); }}",
                    encoding="utf-8",
                )
                self.programs[function] = build(main)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(self.programs[function], [source]).run()
        return output.getvalue().removesuffix("\n")

    def parse_source(self, source: str) -> str:
        return self.render_source("parse_source", source)

    def parse_block_source(self, source: str) -> str:
        return self.render_source("parse_block_source", source)

    def parse_program_source(self, source: str) -> str:
        return self.render_source("parse_program_source", source)

    def parse_type_source(self, source: str) -> str:
        return self.render_source("parse_type_source", source)

    def test_parses_scalar_and_array_literals(self):
        cases = {
            "42": "42",
            "12.50": "12.50",
            '"hello"': '"hello"',
            "true": "true",
            "false": "false",
            "[]": "[]",
            '[1, 2.5, "three", false]': '[1, 2.5, "three", false]',
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_source(source), expected)

    def test_parses_every_binary_precedence_level(self):
        self.assertEqual(
            self.parse_source("1..2 || false && 3 == 4 < 5 + 6 * 7"),
            "(1 .. (2 || (false && (3 == (4 < (5 + (6 * 7)))))))",
        )

    def test_parses_every_binary_operator(self):
        operators = (
            "..", "||", "&&", "==", "!=", "<", "<=", ">", ">=",
            "+", "-", "*", "/", "%",
        )
        for operator in operators:
            with self.subTest(operator=operator):
                self.assertEqual(
                    self.parse_source(f"left {operator} right"),
                    f"(left {operator} right)",
                )

    def test_binary_operators_are_left_associative(self):
        self.assertEqual(
            self.parse_source("10 - 3 - 2"),
            "((10 - 3) - 2)",
        )

    def test_parses_unary_calls_fields_and_indexes(self):
        cases = {
            "!items[1 + 2].ready": "(!items[(1 + 2)].ready)",
            "build(1, [2, 3], user.name)[0]": "build(1, [2, 3], user.name)[0]",
            "empty()": "empty()",
            "--value": "(-(-value))",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_source(source), expected)

    def test_method_calls_lower_to_receiver_first_calls(self):
        cases = {
            "value.transform()": "transform(value)",
            "values.append(1)": "append(values, 1)",
            "values.append(1).len()": "len(append(values, 1))",
            "make().field.transform(2)": "transform(make().field, 2)",
            "values.put(\"key\", 1)": '$method_put(values, "key", 1)',
            "values.has(\"key\")": '$method_has(values, "key")',
            "values.get(\"key\")": '$method_get(values, "key")',
            "values.add(1)": "$method_add(values, 1)",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_source(source), expected)

    def test_parentheses_override_precedence(self):
        self.assertEqual(
            self.parse_source("(1 + 2) * 3"),
            "((1 + 2) * 3)",
        )

    def test_parses_if_expressions_with_block_branches(self):
        cases = {
            "if ready { 1 } else { 0 }": "if ready { 1 } else { 0 }",
            "if ready { print(1); }": "if ready { print(1); }",
            "if value > 0 { print(value); value } else { 0 }": (
                "if (value > 0) { print(value); value } else { 0 }"
            ),
            "1 + if ready { 2 } else { 3 }": (
                "(1 + if ready { 2 } else { 3 })"
            ),
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_source(source), expected)

    def test_parses_if_expressions_as_block_items(self):
        self.assertEqual(
            self.parse_block_source(
                "{ if ready { print(1); } else { print(0); }; "
                "if answer > 0 { answer } else { 0 } }"
            ),
            "{ if ready { print(1); } else { print(0); }; "
            "if (answer > 0) { answer } else { 0 } }",
        )

    def test_reports_malformed_if_expressions(self):
        cases = {
            "if": "expected expression",
            "if ready": "expected opening brace",
            "if ready { 1 } else": "expected opening brace",
            "if ready { 1 } else 0": "expected opening brace",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_source(source), expected)

    def test_parses_match_expressions_and_patterns(self):
        cases = {
            "match option { Some(value) => value, None() => 0 }": (
                "match option { Some(value) => value, None() => 0 }"
            ),
            "match pair { Pair(left, right) => left + right }": (
                "match pair { Pair(left, right) => (left + right) }"
            ),
            "match result { Ok(value) => { print(value); value }, "
            "Error(message) => fail(message), }": (
                "match result { Ok(value) => { print(value); value }, "
                "Error(message) => fail(message) }"
            ),
            "match outer { Some(value) => match value { "
            "Some(inner) => inner, None() => 0 }, None() => 0 }": (
                "match outer { Some(value) => match value { "
                "Some(inner) => inner, None() => 0 }, None() => 0 }"
            ),
            "match value {}": "match value {}",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_source(source), expected)

    def test_match_expressions_participate_in_precedence_and_blocks(self):
        self.assertEqual(
            self.parse_block_source(
                "{ selected: Nat = 1 + match option { "
                "Some(value) => value, None() => 0 }; selected }"
            ),
            "{ selected: Nat = (1 + match option { Some(value) => value, "
            "None() => 0 }); selected }",
        )

    def test_reports_malformed_match_expressions(self):
        cases = {
            "match": "expected expression",
            "match value": "expected opening match brace",
            "match value {": "expected closing match brace",
            "match value { 1() => 1 }": "expected pattern variant",
            "match value { Some => 1 }": "expected opening pattern parenthesis",
            "match value { Some( }": "expected pattern binding",
            "match value { Some(item other) => item }": (
                "expected comma or closing pattern parenthesis"
            ),
            "match value { Some(item,) => item }": "expected pattern binding",
            "match value { Some(item) item }": "expected => after pattern",
            "match value { Some(item) => }": "expected expression",
            "match value { Some(item) => item None() => 0 }": (
                "expected comma or closing match brace"
            ),
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_source(source), expected)

    def test_reports_malformed_expressions(self):
        cases = {
            "": "expected expression",
            "()": "expected expression",
            "[1,]": "expected expression",
            "call(1 2)": "expected comma or )",
            "(1": "expected closing parenthesis",
            "items[0": "expected closing bracket",
            "value.": "expected field name",
            "value.method(1 2)": "expected comma or )",
            "1 +": "expected expression",
            "1 2": "unexpected token",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_source(source), expected)

    def test_propagates_lexer_failures(self):
        self.assertEqual(self.parse_source("#"), "lexing failed")

    def test_parses_empty_statement_and_value_blocks(self):
        cases = {
            "{}": "{}",
            "{ 42 }": "{ 42 }",
            "{ 42; }": "{ 42; }",
            "{ print(1); print(2); }": "{ print(1); print(2); }",
            "{ print(1); 2 + 3 }": "{ print(1); (2 + 3) }",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_block_source(source), expected)

    def test_parses_bindings_assignments_and_nested_type_references(self):
        source = (
            "{ answer: Nat = 40 + 2; "
            "mut values: Map[Str,[Nat]] = build(); "
            "values = update(values); "
            "values }"
        )
        self.assertEqual(
            self.parse_block_source(source),
            "{ answer: Nat = (40 + 2); mut values: Map[Str,[Nat]] = build(); "
            "values = update(values); values }",
        )

    def test_parses_while_and_for_statements(self):
        cases = {
            "{ while ready { print(1); } }": (
                "{ while ready { print(1); } }"
            ),
            "{ while cursor > 0 { cursor = cursor - 1; }; cursor }": (
                "{ while (cursor > 0) { cursor = (cursor - 1); } cursor }"
            ),
            "{ for value in 0..limit { print(value); } }": (
                "{ for value in (0 .. limit) { print(value); } }"
            ),
            "{ for item in values { while ready { use(item); } }; result }": (
                "{ for item in values { while ready { use(item); } } result }"
            ),
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_block_source(source), expected)

    def test_reports_malformed_loops(self):
        cases = {
            "{ while }": "expected expression",
            "{ while ready }": "expected opening brace",
            "{ for }": "expected loop variable",
            "{ for item }": "expected in after loop variable",
            "{ for item of values {} }": "expected in after loop variable",
            "{ for item in }": "expected expression",
            "{ for item in values }": "expected opening brace",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_block_source(source), expected)

    def test_reports_malformed_blocks(self):
        cases = {
            "42": "expected opening brace",
            "{": "expected closing brace",
            "{ mut }": "expected binding name",
            "{ mut name Nat = 1; }": "expected colon after binding name",
            "{ name: = 1; }": "expected type",
            "{ name: Nat 1; }": "expected = after binding type",
            "{ print(1) print(2); }": "expected semicolon or closing brace",
            "{ name: Map[] = 1; }": "expected type",
            "{} extra": "unexpected token after block",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_block_source(source), expected)

    def test_parses_semicolonless_blocks_and_multiline_expressions(self):
        self.assertEqual(
            self.parse_block_source(
                "{\nname: Nat = (\n20 +\n22\n)\nname = name +\n1\n"
                "print(name)\nname\n}"
            ),
            "{ name: Nat = (20 + 22); name = (name + 1); "
            "print(name); name }",
        )
        self.assertEqual(
            self.parse_block_source("{ name: Nat = 1 }"),
            "{ name: Nat = 1; }",
        )
        self.assertEqual(
            self.parse_block_source("{ name = 1 }"),
            "{ name = 1; }",
        )

    def test_parses_empty_and_import_only_programs(self):
        cases = {
            "": "",
            'import "types.panack"': 'import "types.panack";',
            'import "types.panack";': 'import "types.panack";',
            'import "types.panack";\n// shared parser data\nimport "lexer.panack";': (
                'import "types.panack"; import "lexer.panack";'
            ),
            'import "../shared/token.panack";': 'import "../shared/token.panack";',
            "import stdlib/option": 'import "stdlib/option";',
            "import project/shared/token.panack;": 'import "project/shared/token";',
            'import "stdlib/result.panack";': 'import "stdlib/result.panack";',
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_program_source(source), expected)

    def test_reports_malformed_import_declarations(self):
        cases = {
            "import": "expected import path",
            "import lexer.panack;": "expected '/' in logical import path",
            "import stdlib/;": "expected logical import path segment",
            "import stdlib/option.txt;": "logical import extension must be .panack",
            'import "lexer.panack"; extra': "expected opening function parenthesis",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_program_source(source), expected)

    def test_program_parser_propagates_lexer_failures(self):
        self.assertEqual(self.parse_program_source("#"), "lexing failed")

    def test_parses_named_function_references_and_functional_methods(self):
        self.assertEqual(self.parse_source("@twice"), "@twice")
        self.assertEqual(
            self.parse_source("values.map(@twice).reduce(0, @total)"),
            "$method_reduce($method_map(values, @twice), 0, @total)",
        )

    def test_parses_complete_nested_type_references(self):
        cases = {
            "Nat": "Nat",
            "[Nat]": "[Nat]",
            "Map[Str,[Nat]]": "Map[Str,[Nat]]",
            "Result[Option[Nat],Map[Str,[Dec]]]": (
                "Result[Option[Nat],Map[Str,[Dec]]]"
            ),
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_type_source(source), expected)

    def test_reports_malformed_type_references(self):
        cases = {
            "": "expected type",
            "[]": "expected type",
            "[Nat": "expected closing type bracket",
            "Map[]": "expected type",
            "Map[Nat,]": "expected type",
            "Map[Nat Str]": "expected comma or closing type bracket",
            "Nat Str": "unexpected token after type",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_type_source(source), expected)

    def test_parses_guarded_type_declarations(self):
        source = (
            'import "types.panack"; '
            "type Positive = Int where value > 0; "
            "type Port = Nat where value >= 1 && value <= 65535;"
        )
        self.assertEqual(
            self.parse_program_source(source),
            'import "types.panack"; type Positive = Int where (value > 0); '
            "type Port = Nat where ((value >= 1) && (value <= 65535));",
        )

    def test_reports_malformed_guarded_type_declarations(self):
        cases = {
            "type": "expected type name",
            "type = Int where value > 0;": "expected type name",
            "type Positive Int where value > 0;": "expected = after type name",
            "type Positive = ;": "expected guarded type base",
            "type Positive = where value > 0;": (
                "expected where after guarded type base"
            ),
            "type Positive = Int value > 0;": (
                "expected where after guarded type base"
            ),
            "type Positive = Int where;": "expected expression",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_program_source(source), expected)

    def test_parses_record_declarations(self):
        cases = {
            "record Empty {}": "record Empty {}",
            "record Position { line: Nat, column: Nat }": (
                "record Position { line: Nat, column: Nat }"
            ),
            "record Pair[A, B] { first: A, second: B, }": (
                "record Pair[A,B] { first: A, second: B }"
            ),
            "record Index[K, V] { values: Map[K,[Option[V]]] }": (
                "record Index[K,V] { values: Map[K,[Option[V]]] }"
            ),
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_program_source(source), expected)

    def test_reports_malformed_record_declarations(self):
        cases = {
            "record": "expected record name",
            "record Position": "expected opening record brace",
            "record Pair[] {}": "expected type parameter",
            "record Pair[A,] {}": "expected type parameter",
            "record Pair[A B] {}": (
                "expected comma or closing type parameter bracket"
            ),
            "record Position { 1: Nat }": "expected record field name",
            "record Position { line Nat }": (
                "expected colon after record field name"
            ),
            "record Position { line: }": "expected type",
            "record Position { line: Nat column: Nat }": (
                "expected comma or closing record brace"
            ),
            "record Position { line: Nat": "expected closing record brace",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_program_source(source), expected)

    def test_parses_enum_declarations(self):
        cases = {
            "enum Empty {}": "enum Empty {}",
            "enum Direction { North, South, East, West }": (
                "enum Direction { North, South, East, West }"
            ),
            "enum Option[T] { None, Some(T), }": (
                "enum Option[T] { None, Some(T) }"
            ),
            "enum Result[T, E] { Ok(T), Error(E) }": (
                "enum Result[T,E] { Ok(T), Error(E) }"
            ),
            "enum Entry[K, V] { Missing, Found(K,Map[K,[V]]) }": (
                "enum Entry[K,V] { Missing, Found(K,Map[K,[V]]) }"
            ),
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_program_source(source), expected)

    def test_reports_malformed_enum_declarations(self):
        cases = {
            "enum": "expected enum name",
            "enum Option": "expected opening enum brace",
            "enum Option[] {}": "expected type parameter",
            "enum Option[T,] {}": "expected type parameter",
            "enum Option[T E] {}": (
                "expected comma or closing type parameter bracket"
            ),
            "enum Option { 1 }": "expected enum variant name",
            "enum Option { Some( }": "expected type",
            "enum Option { Some(Nat }": (
                "expected comma or closing variant parenthesis"
            ),
            "enum Option { Some(Nat,) }": "expected type",
            "enum Pair { Both(Nat Str) }": (
                "expected comma or closing variant parenthesis"
            ),
            "enum Option { None Some(Nat) }": (
                "expected comma or closing enum brace"
            ),
            "enum Option { None": "expected closing enum brace",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_program_source(source), expected)

    def test_parses_function_declarations(self):
        cases = {
            "main(): Void {}": "main(): Void {}",
            "pure answer(): Nat { 40 + 2 }": (
                "pure answer(): Nat { (40 + 2) }"
            ),
            "pure lookup(index: Map[Str,[Nat]], fallback: Option[Nat]): "
            "Result[Nat,Str] { if len(index) > 0 { Ok(1) } else { "
            "Error(\"empty\") } }": (
                "pure lookup(index: Map[Str,[Nat]], fallback: Option[Nat]): "
                "Result[Nat,Str] { if (len(index) > 0) { Ok(1) } else { "
                "Error(\"empty\") } }"
            ),
            "process(values: [Nat]): Void { for value in values { "
            "print(value); } }": (
                "process(values: [Nat]): Void { for value in values { "
                "print(value); } }"
            ),
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_program_source(source), expected)

    def test_parses_complete_mixed_program(self):
        source = (
            'import "runtime.panack"; '
            "type Positive = Int where value > 0; "
            "record Pair[A, B] { first: A, second: B } "
            "enum Option[T] { None, Some(T) } "
            "pure value_or(value: Option[Nat], fallback: Nat): Nat { "
            "match value { Some(found) => found, None() => fallback } } "
            "main(): Void { print(value_or(Some(42), 0)); }"
        )
        self.assertEqual(
            self.parse_program_source(source),
            'import "runtime.panack"; type Positive = Int where (value > 0); '
            "record Pair[A,B] { first: A, second: B } "
            "enum Option[T] { None, Some(T) } "
            "pure value_or(value: Option[Nat], fallback: Nat): Nat { "
            "match value { Some(found) => found, None() => fallback } } "
            "main(): Void { print(value_or(Some(42), 0)); }",
        )

    def test_reports_malformed_function_declarations(self):
        cases = {
            "pure": "expected function name",
            "main": "expected opening function parenthesis",
            "main[Value](): Void {}": "expected opening function parenthesis",
            "main(": "expected function parameter",
            "main(1: Nat): Void {}": "expected function parameter",
            "main(value Nat): Void {}": "expected colon after parameter name",
            "main(value: ): Void {}": "expected type",
            "main(value: Nat": "expected closing function parenthesis",
            "main(value: Nat other: Nat): Void {}": (
                "expected comma or closing function parenthesis"
            ),
            "main(value: Nat,): Void {}": "expected function parameter",
            "main() Void {}": "expected colon after function parameters",
            "main(): {}": "expected type",
            "main(): Void": "expected opening brace",
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.parse_program_source(source), expected)


if __name__ == "__main__":
    unittest.main()
