import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import Checker, PanackeltyError, Parser, VM, build, lex


PROJECT = Path(__file__).resolve().parents[3]
COMPILER = PROJECT / "src/compiler"


class SelfHostedCheckerTests(unittest.TestCase):
    def run_checker(self, expression: str) -> str:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in (
                "types.panack",
                "lexer.panack",
                "parser.panack",
                "resolver.panack",
                "checker.panack",
            ):
                (root / name).write_text(
                    (COMPILER / name).read_text(encoding="utf-8"),
                    encoding="utf-8",
                )
            main = root / "main.panack"
            main.write_text(
                'import "checker.panack";\n'
                f"main(): Void {{ print({expression}); }}",
                encoding="utf-8",
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                VM(build(main)).run()
            return output.getvalue().removesuffix("\n")

    def check_source(self, source: str) -> str:
        return self.run_checker(f"check_source_types({json.dumps(source)})")

    def check_modules(self, sources: list[tuple[str, str]], entry: str) -> str:
        loaded = ", ".join(
            f"LoadedSource({json.dumps(path)}, {json.dumps(source)})"
            for path, source in sources
        )
        return self.run_checker(
            f"check_loaded_source_types([{loaded}], {json.dumps(entry)})"
        )

    @staticmethod
    def bootstrap_accepts(source: str) -> bool:
        try:
            Checker(Parser(lex(source)).parse()).check()
        except PanackeltyError:
            return False
        return True

    def assert_differential(self, source: str, expected: str = "ok") -> None:
        actual = self.check_source(source)
        self.assertEqual(actual == "ok", self.bootstrap_accepts(source), actual)
        if expected != "ok":
            self.assertIn(expected, actual)

    def test_accepts_core_types_records_enums_and_control_flow(self):
        source = """
record Pair { left: Nat, right: Nat }
enum Option[T] { None, Some(T) }
pure choose(option: Option[Nat], fallback: Nat): Nat {
  match option { Some(value) => value, None() => fallback }
}
pure sum(values: [Nat]): Nat {
  mut total: Nat = 0;
  for value in values { total = total + value; }
  total
}
main(): Void {
  pair: Pair = Pair(20, 22);
  print(sum([pair.left, choose(Some(1), 0)]));
}
"""
        self.assert_differential(source)

    def test_checks_type_references_and_function_returns(self):
        cases = [
            (
                "pure bad(value: Missing): Nat { 0 } main(): Void {}",
                "unknown type Missing",
            ),
            (
                "pure bad(): Nat { true } main(): Void {}",
                "function bad returns Bool, expected Nat",
            ),
            (
                "record Box[T] { value: T } pure bad(value: Box): Nat { 0 } main(): Void {}",
                "Box expects 1 type arguments, got 0",
            ),
        ]
        for source, expected in cases:
            with self.subTest(expected=expected):
                self.assert_differential(source, expected)

    def test_checks_operators_arrays_indexing_and_loops(self):
        cases = [
            (
                'main(): Void { value: Nat = "bad" + 1; }',
                "operator + does not accept Str and Nat",
            ),
            (
                'main(): Void { values: [Nat] = [1, 2]; print(values["0"]); }',
                "index must be Nat, got Str",
            ),
            (
                "main(): Void { for value in true {} }",
                "for requires a Range or array, got Bool",
            ),
            (
                "main(): Void { while 1 {} }",
                "while condition must be Bool",
            ),
        ]
        for source, expected in cases:
            with self.subTest(expected=expected):
                self.assert_differential(source, expected)

    def test_checks_calls_fields_and_constructor_inference(self):
        cases = [
            (
                "enum Option[T] { None, Some(T) } main(): Void { value: Option[Nat] = Some(1); print(value); }",
                "ok",
            ),
            (
                "record Pair { left: Nat, right: Nat } main(): Void { print(Pair(1)); }",
                "Pair expects 2 arguments, got 1",
            ),
            (
                "record Pair { left: Nat, right: Nat } main(): Void { pair: Pair = Pair(1, 2); print(pair.missing); }",
                "record Pair has no field missing",
            ),
        ]
        for source, expected in cases:
            with self.subTest(expected=expected):
                self.assert_differential(source, expected)

    def test_checks_method_calls_as_receiver_first_calls(self):
        accepted = """
record Pair { left: Nat, right: Nat }
pure total(pair: Pair, extra: Nat): Nat { pair.left + pair.right + extra }
main(): Void {
  pair: Pair = Pair(20, 21);
  values: [Nat] = [pair.total(1)].append(43);
  mut answers: Map[Str,Nat] = map();
  answers = answers.put("answer", values[0]);
  mut seen: Set[Str] = set();
  seen = seen.add("answer");
  print(values.len());
  print(answers.has("answer"));
  print(answers.get("answer"));
  print(seen.has("answer"));
}
"""
        rejected = 'main(): Void { print(42.starts_with("4")); }'
        missing = "record Box { value: Nat } main(): Void { print(Box(1).value()); }"
        invalid_has = "main(): Void { print([1].has(1)); }"
        self.assert_differential(accepted)
        self.assert_differential(rejected, "argument 1 to starts_with is Nat, expected Str")
        self.assert_differential(missing, "unknown function value")
        self.assert_differential(invalid_has, "has expects a Map or Set, got Array[Nat]")

    def test_checks_match_exhaustiveness_payloads_and_branch_types(self):
        cases = [
            (
                "enum Maybe { None, Some(Nat) } pure bad(value: Maybe): Nat { match value { Some(item) => item } } main(): Void {}",
                "non-exhaustive match; missing None",
            ),
            (
                "enum Maybe { None, Some(Nat) } pure bad(value: Maybe): Nat { match value { Some() => 1, None() => 0 } } main(): Void {}",
                "pattern Some expects 1 bindings",
            ),
            (
                'pure bad(flag: Bool): Nat { if flag { 1 } else { "no" } } main(): Void {}',
                "branches have incompatible types Nat and Str",
            ),
            (
                "pure bad(flag: Bool): Nat { if flag { 1 } } main(): Void {}",
                "function bad returns Void, expected Nat",
            ),
        ]
        for source, expected in cases:
            with self.subTest(expected=expected):
                self.assert_differential(source, expected)

    def test_accepts_if_without_else_in_void_position(self):
        self.assert_differential(
            "main(): Void { mut answer: Nat = 0; "
            "if true { answer = 42; }; if false { 99 }; print(answer); }"
        )

    def test_checks_guarded_literals_and_control_flow_facts(self):
        accepted = """
type Positive = Int where value > 0;
pure positive(value: Int): Int {
  if value > 0 { checked: Positive = value; checked } else { 0 }
}
main(): Void { literal: Positive = 3; print(positive(literal)); }
"""
        rejected = """
type Positive = Int where value > 0;
main(): Void { value: Positive = 0; print(value); }
"""
        subtraction = """
pure decrement(value: Nat): Nat {
  if value > 0 { value - 1 } else { 0 }
}
main(): Void { print(decrement(2)); }
"""
        equality_else = """
pure decrement(value: Nat): Nat {
  if value == 0 { 0 } else { value - 1 }
}
main(): Void { print(decrement(2)); }
"""
        self.assert_differential(accepted)
        self.assert_differential(rejected, "guard is not proven")
        self.assert_differential(subtraction)
        self.assert_differential(equality_else)

    def test_checks_mutability_and_entry_point(self):
        cases = [
            (
                "main(): Void { value: Nat = 1; value = 2; }",
                "cannot assign to immutable local value",
            ),
            ("pure answer(): Nat { 42 }", "program has no main function"),
            ("main(value: Nat): Void {}", "main cannot take parameters"),
        ]
        for source, expected in cases:
            with self.subTest(expected=expected):
                self.assert_differential(source, expected)

    def test_checks_persistent_collections_and_builtin_arguments(self):
        accepted = """
pure values(): Map[Str,Nat] {
  mut result: Map[Str,Nat] = map();
  result = map_put(result, "answer", 42);
  result
}
pure unique(): Set[Nat] { set_add(set(), 1) }
main(): Void {
  items: [Nat] = append([], 1);
  print(len(concat(items, [2])));
  print(map_get(values(), "answer"));
  print(set_has(unique(), 1));
}
"""
        rejected = 'main(): Void { print(slice("text", "bad", 2)); }'
        self.assert_differential(accepted)
        self.assert_differential(rejected, "expected Nat")

    def test_checks_types_across_loaded_modules(self):
        sources = [
            (
                "main.panack",
                'import "model.panack"; main(): Void { item: Box = Box(42); print(item.value); }',
            ),
            ("model.panack", "record Box { value: Nat }"),
        ]
        self.assertEqual(self.check_modules(sources, "main.panack"), "ok")

        bad_sources = [
            (
                "main.panack",
                'import "model.panack"; main(): Void { item: Box = Box("bad"); }',
            ),
            ("model.panack", "record Box { value: Nat }"),
        ]
        self.assertIn(
            "argument 1 to Box is Str, expected Nat",
            self.check_modules(bad_sources, "main.panack"),
        )

        located_sources = [
            (
                "main.panack",
                'import "broken.panack"; main(): Void { broken(); }',
            ),
            (
                "broken.panack",
                'broken(): Void {\n  invalid: Nat = "bad";\n}',
            ),
        ]
        self.assertIn(
            "broken.panack:2:18: cannot assign Str to Nat; guard is not proven",
            self.check_modules(located_sources, "main.panack"),
        )

    def test_checks_callable_values_and_functional_array_methods(self):
        accepted = """
pure twice(value: Nat): Nat { value * 2 }
pure total(accumulator: Nat, value: Nat): Nat { accumulator + value }
pure apply(callback: PureFn[Nat,Nat], value: Nat): Nat { callback.call(value) }
main(): Void {
  callback: PureFn[Nat,Nat] = @twice
  print([1, 2].map(callback))
  print([1, 2].reduce(0, @total))
  print(apply(callback, 3))
}
"""
        rejected = "pure text(value: Str): Str { value } main(): Void { print([1].map(@text)); }"
        self.assert_differential(accepted)
        self.assert_differential(rejected, "map callback must accept Nat")


if __name__ == "__main__":
    unittest.main()
