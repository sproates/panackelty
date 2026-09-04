import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import VM, build


PROJECT = Path(__file__).resolve().parents[3]
COMPILER = PROJECT / "src/compiler"


class SelfHostedResolverTests(unittest.TestCase):
    def run_resolver(self, expression: str) -> str:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ("types.panack", "lexer.panack", "parser.panack", "resolver.panack"):
                (root / name).write_text(
                    (COMPILER / name).read_text(encoding="utf-8"),
                    encoding="utf-8",
                )
            main = root / "main.panack"
            main.write_text(
                'import "resolver.panack";\n'
                f"main(): Void {{ print({expression}); }}",
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(main)).run()
            return output.getvalue().removesuffix("\n")

    def resolve_source(self, source: str) -> str:
        return self.run_resolver(f"resolve_source({json.dumps(source)})")

    def resolve_modules(self, sources: list[tuple[str, str]], entry: str) -> str:
        loaded = ", ".join(
            f"LoadedSource({json.dumps(path)}, {json.dumps(source)})"
            for path, source in sources
        )
        return self.run_resolver(
            f"resolve_loaded_sources([{loaded}], {json.dumps(entry)})"
        )

    def test_resolves_top_level_callables_builtins_and_local_names(self):
        source = """
record Pair { left: Nat, right: Nat }
enum Option[T] { None, Some(T) }

pure sum(pair: Pair): Nat { pair.left + pair.right }

pure value_or(option: Option[Nat], fallback: Nat): Nat {
  match option { Some(value) => value, None() => fallback }
}

main(): Void {
  pair: Pair = Pair(20, 22);
  print(sum(pair));
  print(pair.sum());
  mut values: Map[Str,Nat] = map();
  values = values.put("answer", 42);
  print(values.has("answer"));
  print(values.get("answer"));
  print(set().add("seen").has("seen"));
  print(value_or(Some(1), 0));
}
"""
        self.assertEqual(self.resolve_source(source), "ok")

    def test_reports_duplicate_and_conflicting_top_level_symbols(self):
        cases = {
            "type Value = Nat where value > 0; record Value {} main(): Void {}": (
                "top-level name Value is already declared"
            ),
            "print(): Void {} main(): Void {}": (
                "top-level name print conflicts with a constructor or built-in"
            ),
            "enum First { Same } enum Second { Same } main(): Void {}": (
                "variant constructor Same is declared by more than one enum"
            ),
            "enum Output { print } main(): Void {}": (
                "variant constructor print conflicts with a built-in"
            ),
            "enum Choice { Pick } record Pick {} main(): Void {}": (
                "top-level name Pick conflicts with a constructor or built-in"
            ),
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.resolve_source(source), expected)

    def test_reports_unknown_names_functions_and_variants(self):
        source = """
enum Option[T] { None, Some(T) }
main(): Void {
  print(missing);
  unknown();
  option: Option[Nat] = None();
  match option { Other(value) => value };
}
"""
        self.assertEqual(
            self.resolve_source(source),
            "unknown name missing\nunknown function unknown\nunknown variant Other",
        )

    def test_method_calls_use_global_callable_lookup(self):
        source = "record Box { value: Nat } main(): Void { Box(1).missing(); }"
        self.assertEqual(
            self.resolve_source(source),
            "unknown function missing",
        )

    def test_enforces_lexical_scope_and_shadowing(self):
        cases = {
            "main(value: Nat, value: Nat): Void {}": (
                "parameter value is already declared"
            ),
            "main(value: Nat): Void { value: Nat = 1; }": (
                "local value shadows an existing binding"
            ),
            "main(): Void { value: Nat = 1; for value in 0..1 {} }": (
                "loop variable value shadows an existing binding"
            ),
            "enum Option[T] { None, Some(T) } "
            "main(value: Option[Nat]): Void { match value { "
            "Some(value) => print(value), None() => print(0) }; }": (
                "pattern binding value shadows an existing binding"
            ),
            "main(): Void { if true { inner: Nat = 1; } else {}; print(inner); }": (
                "unknown name inner"
            ),
        }
        for source, expected in cases.items():
            with self.subTest(source=source):
                self.assertEqual(self.resolve_source(source), expected)

    def test_resolves_guard_value_and_recursive_calls(self):
        source = """
type Positive = Int where value > 0;
pure countdown(value: Nat): Nat {
  if value == 0 { 0 } else { countdown(value - 1) }
}
main(): Void { print(countdown(3)); }
"""
        self.assertEqual(self.resolve_source(source), "ok")

    def test_propagates_lexer_and_parser_failures(self):
        self.assertEqual(self.resolve_source("#"), "lexing failed")
        self.assertEqual(self.resolve_source("main"), "expected opening function parenthesis")

    def test_resolves_reachable_transitive_modules(self):
        sources = [
            (
                "main.panack",
                'import "service.panack"; main(): Void { print(result()); }',
            ),
            (
                "service.panack",
                'import "values.panack"; pure result(): Nat { answer() }',
            ),
            ("values.panack", "pure answer(): Nat { 42 }"),
            ("unused.panack", "main(): Void { print(missing); }"),
        ]
        self.assertEqual(self.resolve_modules(sources, "main.panack"), "ok")

    def test_visits_a_diamond_dependency_only_once(self):
        sources = [
            (
                "main.panack",
                'import "left.panack"; import "right.panack"; '
                "main(): Void { print(left() + right()); }",
            ),
            (
                "left.panack",
                'import "shared.panack"; pure left(): Nat { shared() }',
            ),
            (
                "right.panack",
                'import "shared.panack"; pure right(): Nat { shared() }',
            ),
            ("shared.panack", "pure shared(): Nat { 21 }"),
        ]
        self.assertEqual(self.resolve_modules(sources, "main.panack"), "ok")

    def test_reports_module_graph_failures(self):
        cases = [
            (
                [("main.panack", 'import "missing.panack"; main(): Void {}')],
                "main.panack",
                "module missing.panack is not loaded",
            ),
            (
                [("other.panack", "pure answer(): Nat { 42 }")],
                "main.panack",
                "module main.panack is not loaded",
            ),
            (
                [
                    ("first.panack", 'import "second.panack";'),
                    ("second.panack", 'import "first.panack";'),
                ],
                "first.panack",
                "import cycle includes first.panack",
            ),
            (
                [
                    ("main.panack", "main(): Void {}"),
                    ("main.panack", "pure answer(): Nat { 42 }"),
                ],
                "main.panack",
                "module main.panack is loaded more than once",
            ),
            (
                [
                    (
                        "main.panack",
                        'import "first.panack"; import "second.panack"; main(): Void {}',
                    ),
                    ("first.panack", "pure answer(): Nat { 1 }"),
                    ("second.panack", "pure answer(): Nat { 2 }"),
                ],
                "main.panack",
                "top-level name answer is already declared",
            ),
        ]
        for sources, entry, expected in cases:
            with self.subTest(expected=expected):
                self.assertEqual(self.resolve_modules(sources, entry), expected)

    def test_reports_module_parse_and_lex_failures_with_paths(self):
        cases = [
            ("#", "broken.panack:1:1: unexpected character #"),
            (
                "// heading\nmain",
                "broken.panack:2:5: expected opening function parenthesis",
            ),
        ]
        for source, expected in cases:
            with self.subTest(source=source):
                self.assertEqual(
                    self.resolve_modules([("broken.panack", source)], "broken.panack"),
                    expected,
                )

    def test_reports_name_failure_in_owning_module(self):
        sources = [
            (
                "main.panack",
                'import "broken.panack"; main(): Void { broken(); }',
            ),
            (
                "broken.panack",
                "broken(): Void {\n  print(missing);\n}",
            ),
        ]
        self.assertEqual(
            self.resolve_modules(sources, "main.panack"),
            "broken.panack:2:9: unknown name missing",
        )


if __name__ == "__main__":
    unittest.main()
