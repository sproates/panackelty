import unittest

from panackelty import Checker, PanackeltyError, Parser, lex
from tests.unit.support import CompilerHarnessTestCase


class SelfHostedPurityTests(CompilerHarnessTestCase):
    def run_frontend(self, expression: str, arguments=()) -> str:
        return self.run_harness(
            "compiler/purity.panack",
            f"main(): Void {{ print({expression}); }}",
            arguments,
        ).removesuffix("\n")

    def check_source(self, source: str) -> str:
        return self.run_frontend("check_source_frontend(command_args()[0])", [source])

    def check_modules(self, sources: list[tuple[str, str]], entry: str) -> str:
        loaded = ", ".join(
            f"LoadedSource(command_args()[{2 * index + 1}], command_args()[{2 * index + 2}])"
            for index in range(len(sources))
        )
        arguments = [entry] + [value for pair in sources for value in pair]
        return self.run_frontend(
            f"check_loaded_source_frontend([{loaded}], command_args()[0])", arguments
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

    def test_accepts_pure_calls_constructors_and_recursion(self):
        source = """
record Pair { left: Nat, right: Nat }
enum Option[T] { None, Some(T) }
pure sum(pair: Pair): Nat { pair.left + pair.right }
pure countdown(value: Nat): Nat {
  if value > 0 { countdown(value - 1) } else { 0 }
}
pure wrapped(): Option[Nat] { Some(sum(Pair(20, 22)) + countdown(2)) }
main(): Void { print(wrapped()); }
"""
        self.assert_differential(source)

    def test_rejects_impure_builtins_from_pure_functions(self):
        cases = [
            (
                "pure bad(): Void { print(1); } main(): Void {}",
                "pure function cannot call impure function print",
            ),
            (
                'pure bad(): Str { read_file("data.txt") } main(): Void {}',
                "pure function cannot call impure function read_file",
            ),
            (
                "pure bad(): Str { if true { read_line() } else { \"\" } } main(): Void {}",
                "pure function cannot call impure function read_line",
            ),
        ]
        for source, expected in cases:
            with self.subTest(expected=expected):
                self.assert_differential(source, expected)

    def test_rejects_calls_to_impure_user_functions(self):
        source = """
output(): Void { print(1); }
pure hidden(): Void {
  for value in 0..1 { if true { output(); } else {} }
}
main(): Void {}
"""
        self.assert_differential(
            source,
            "pure function cannot call impure function output",
        )

        method_source = """
output(value: Nat): Void { print(value); }
pure hidden(value: Nat): Void { value.output(); }
main(): Void {}
"""
        self.assert_differential(
            method_source,
            "pure function cannot call impure function output",
        )

    def test_impure_functions_may_call_pure_and_impure_functions(self):
        source = """
pure answer(): Nat { 42 }
output(): Void { print(answer()); }
main(): Void { output(); }
"""
        self.assert_differential(source)

    def test_guard_expressions_are_pure(self):
        source = """
type Enabled = Bool where read_line() == "yes";
main(): Void {}
"""
        self.assert_differential(
            source,
            "pure function cannot call impure function read_line",
        )

    def test_checks_purity_across_loaded_modules(self):
        sources = [
            (
                "main.panack",
                'import "output.panack"; pure bad(): Void { output(); } main(): Void {}',
            ),
            ("output.panack", "output(): Void { print(1); }"),
        ]
        self.assertIn(
            "pure function cannot call impure function output",
            self.check_modules(sources, "main.panack"),
        )

    def test_callable_effects_are_part_of_purity_checking(self):
        accepted = """
pure twice(value: Nat): Nat { value * 2 }
pure apply(callback: PureFn[Nat,Nat]): Nat { callback.call(2) }
main(): Void { print(apply(@twice)); }
"""
        rejected = """
output(value: Nat): Nat { print(value); value }
pure apply(callback: Fn[Nat,Nat]): Nat { callback.call(2) }
main(): Void { print(apply(@output)); }
"""
        self.assert_differential(accepted)
        self.assert_differential(rejected, "pure function cannot call impure function call")


if __name__ == "__main__":
    unittest.main()
