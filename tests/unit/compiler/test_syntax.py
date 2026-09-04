import unittest

from panackelty import PanackeltyError
from tests.unit.support import PanackeltyTestCase


class SyntaxTests(PanackeltyTestCase):
    def test_keyword_free_functions_bindings_and_void_fallthrough(self):
        code = self.compile("""
pure increment(value: Nat): Nat {
  value + 1
}

main(): Void {
  answer: Nat = increment(41);
  mut message: Str = "answer";
  message = "${message} ${answer}";
  print(message);
}
""")
        self.assertEqual(self.run_code(code), "answer 42\n")

    def test_error_is_the_result_failure_variant(self):
        code = self.compile("""
enum Result[T, E] { Ok(T), Error(E) }

pure describe(result: Result[Nat,Str]): Str {
  match result { Ok(value) => "${value}", Error(message) => message }
}

main(): Void {
  print(describe(Error("failed")));
}
""")
        self.assertEqual(self.run_code(code), "failed\n")

    def test_non_void_function_requires_a_result(self):
        with self.assertRaisesRegex(PanackeltyError, "returns Void, expected Nat"):
            self.compile("pure answer(): Nat {} main(): Void {}")

    def test_trailing_semicolon_discards_a_block_value(self):
        with self.assertRaisesRegex(PanackeltyError, "returns Void, expected Nat"):
            self.compile("pure answer(): Nat { 42; } main(): Void {}")

    def test_line_breaks_replace_statement_semicolons(self):
        code = self.compile("""
pure add(left: Nat, right: Nat): Nat {
  left +
    right
}

main(): Void {
  mut answer: Nat = add(
    20,
    22
  )

  // Blank lines and comments do not disturb termination.
  if answer == 42 {
    print("yes")
  }

  answer = answer
    + 1
  print(answer)
}
""")
        self.assertEqual(self.run_code(code), "yes\n43\n")

    def test_semicolons_still_separate_same_line_statements(self):
        code = self.compile(
            'main(): Void { print("first"); print("second") }'
        )
        self.assertEqual(self.run_code(code), "first\nsecond\n")

    def test_method_calls_lower_to_calls_with_the_receiver_first(self):
        code = self.compile("""
record Pair { left: Nat, right: Nat }

pure total(pair: Pair, extra: Nat): Nat {
  pair.left + pair.right + extra
}

main(): Void {
  pair: Pair = Pair(20, 21)
  values: [Nat] = [pair.total(1)].append(43)
  mut answers: Map[Str,Nat] = map()
  answers = answers.put("answer", values[0])
  mut seen: Set[Str] = set()
  seen = seen.add("answer")
  print(values.len())
  print(answers.has("answer"))
  print(answers.get("answer"))
  print(seen.has("answer"))
}
""")
        self.assertEqual(self.run_code(code), "2\ntrue\n42\ntrue\n")

    def test_method_call_receiver_is_checked_as_argument_one(self):
        with self.assertRaisesRegex(
            PanackeltyError,
            "argument 1 to starts_with is Nat, expected Str",
        ):
            self.compile("main(): Void { print(42.starts_with(\"4\")); }")

    def test_parenthesized_dot_name_is_a_method_not_a_record_field_call(self):
        with self.assertRaisesRegex(PanackeltyError, "unknown function value"):
            self.compile(
                "record Box { value: Nat } "
                "main(): Void { box: Box = Box(42); print(box.value()); }"
            )

    def test_short_collection_methods_are_type_directed(self):
        invalid = (
            ("print([1].has(1));", "has expects a Map or Set, got Array\\[Nat\\]"),
            ("print(set().get(1));", "get expects a Map, got Set\\[\\$T\\]"),
            ("print(map().add(1));", "add expects a Set, got Map\\[\\$K,\\$V\\]"),
        )
        for body, expected in invalid:
            with self.subTest(body=body):
                with self.assertRaisesRegex(PanackeltyError, expected):
                    self.compile(f"main(): Void {{ {body} }}")

    def test_named_callable_values_map_reduce_and_indirect_calls(self):
        code = self.compile("""
pure twice(value: Nat): Nat { value * 2 }
pure total(accumulator: Nat, value: Nat): Nat { accumulator + value }
pure apply(callback: PureFn[Nat,Nat], value: Nat): Nat { callback.call(value) }
show(value: Nat): Void { print(value) }
main(): Void {
  callback: PureFn[Nat,Nat] = @twice
  values: [Nat] = [1, 2, 3]
  print(values.map(callback))
  print(values.reduce(0, @total))
  print(apply(callback, 4))
  output: Fn[Nat,Void] = @show
  output.call(9)
}
""")
        self.assertEqual(self.run_code(code), "[2, 4, 6]\n6\n8\n9\n")

    def test_callable_diagnostics_preserve_types_and_purity(self):
        invalid = (
            ("pure bad(callback: Fn[Nat,Nat]): Nat { callback.call(1) } main(): Void {}",
             "pure function cannot invoke an impure callable"),
            ("pure bad(value: Nat): Nat { value.call() } main(): Void {}",
             "call expects a callable, got Nat"),
            ("pure text(value: Str): Str { value } main(): Void { print([1].map(@text)); }",
             "map callback must accept Nat"),
            ("show(value: Nat): Void { print(value) } main(): Void { print([1].map(@show)); }",
             "map requires a pure callable"),
            ("pure widen(value: Nat): Nat { value } main(): Void { callback: PureFn[Int,Nat] = @widen; }",
             "cannot assign PureFn\\[Nat,Nat\\] to PureFn\\[Int,Nat\\]"),
        )
        for source, expected in invalid:
            with self.subTest(expected=expected):
                with self.assertRaisesRegex(PanackeltyError, expected):
                    self.compile(source)

    def test_same_line_statements_still_require_a_separator(self):
        with self.assertRaisesRegex(
            PanackeltyError,
            "expected line break, semicolon, or closing brace after binding",
        ):
            self.compile("main(): Void { answer: Nat = 42 print(answer) }")

    def test_if_else_is_optional_in_void_position(self):
        code = self.compile("""
main(): Void {
  mut answer: Nat = 0;
  if true { answer = 42; };
  if false { answer = 0; };
  if true { 99 };
  print(answer);
}
""")
        self.assertEqual(self.run_code(code), "42\n")

    def test_if_without_else_is_void_in_value_position(self):
        with self.assertRaisesRegex(PanackeltyError, "returns Void, expected Nat"):
            self.compile(
                "pure answer(flag: Bool): Nat { if flag { 42 } } main(): Void {}"
            )

    def test_void_is_only_valid_as_a_return_type(self):
        invalid_sources = (
            "consume(value: Void): Void {} main(): Void {}",
            "main(): Void { value: Void = print(1); }",
            "main(): Void { print(print(1)); }",
            "main(): Void { print([print(1)]); }",
        )
        for source in invalid_sources:
            with self.subTest(source=source):
                with self.assertRaisesRegex(PanackeltyError, "Void"):
                    self.compile(source)

    def test_legacy_function_keywords_and_arrows_are_rejected(self):
        invalid_sources = (
            "fn main(): Void {}",
            "main() -> Void {}",
            "pure fn answer(): Nat { 42 } main(): Void {}",
        )
        for source in invalid_sources:
            with self.subTest(source=source):
                with self.assertRaises(PanackeltyError):
                    self.compile(source)

    def test_legacy_let_unit_and_empty_parentheses_are_rejected(self):
        invalid_sources = (
            "main(): Void { let answer: Nat = 42; print(answer); }",
            "main(): Unit {}",
            "main(): Void { (); }",
        )
        for source in invalid_sources:
            with self.subTest(source=source):
                with self.assertRaises(PanackeltyError):
                    self.compile(source)


if __name__ == "__main__":
    unittest.main()
