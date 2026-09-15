import unittest

from panackelty import Checker, PanackeltyError, Parser, lex
from tests.unit.support import CompilerHarnessTestCase


class LocalInferenceTests(CompilerHarnessTestCase):
    def assert_program(self, source, error=None):
        actual = self.run_harness(
            "compiler/purity.panack",
            "main(): Void { print(check_source_frontend(command_args()[0])) }",
            [source],
        ).strip()
        if error is None:
            self.assertEqual(actual, "ok")
            Checker(Parser(lex(source)).parse()).check()
        else:
            self.assertIn(error, actual)
            with self.assertRaises(PanackeltyError):
                Checker(Parser(lex(source)).parse()).check()

    def test_infers_scalars_expressions_and_preserves_numeric_defaults(self):
        self.assert_program('''
pure natural(value: Nat): Nat { value }
pure signed(): Int { 41 }
main(): Void {
  answer = 41
  zero = 0
  negative = -41
  price = 41.0
  name = "Ada"
  active = true
  label = "Hi, ${name}"
  result = signed()
  data = utf8_encode(label)
  choice = if active { "yes" } else { "no" }
  remaining = answer - 40
  natural(answer)
  natural(zero)
  checked_negative: Int = negative
  checked_price: Dec = price
  checked_data: Bytes = data
  checked_result: Int = result
}
''')
        self.assert_program('pure signed(): Int { 41 } main(): Void { n = signed(); x: Nat = n }',
                            "cannot assign Int to Nat")
        self.assert_program('main(): Void { n = 0; x: Str = n }', "cannot assign Nat to Str")
        self.assert_program('main(): Void { n = -1; x: Nat = n }', "cannot assign Int to Nat")

    def test_mutable_types_are_fixed_and_nat_facts_remain_sound(self):
        self.assert_program('''main(): Void {
  mut label = "first"
  label = "second"
  mut balance: Int = 0
  balance = -1
  mut count = 2
  if count >= 1 { count = count - 1 }
}''')
        for source, error in (
            ('mut n = 0; n = -1', 'cannot assign Int to Nat'),
            ('mut label = "first"; label = false', 'cannot assign Bool to Str'),
            ('n = 1; n = 2', 'cannot assign to immutable local n'),
            ('mut n = 1; n = 0; x = n - 1', 'Nat subtraction may underflow'),
        ):
            with self.subTest(source=source):
                self.assert_program('main(): Void { ' + source + ' }', error)

    def test_nested_collection_and_constructor_evidence_is_order_independent(self):
        prefix = 'enum Option[T] { None, Some(T) } record Box[T] { value: T } '
        cases = (
            ('[[], ["Ada"]]', '[[Str]]'),
            ('[["Ada"], []]', '[[Str]]'),
            ('[[1], [-1]]', '[[Int]]'),
            ('[[-1], [1]]', '[[Int]]'),
            ('[None(), Some("Ada")]', '[Option[Str]]'),
            ('[Some("Ada"), None()]', '[Option[Str]]'),
            ('if true { None() } else { Some("Ada") }', 'Option[Str]'),
            ('if true { Some("Ada") } else { None() }', 'Option[Str]'),
            ('[Box([]), Box(["Ada"])]', '[Box[[Str]]]'),
            ('map().put("Ada", true)', 'Map[Str,Bool]'),
            ('set().add("Ada")', 'Set[Str]'),
        )
        for expression, annotation in cases:
            with self.subTest(expression=expression):
                self.assert_program(prefix + 'main(): Void { inferred = ' + expression +
                                    '; checked: ' + annotation + ' = inferred }')

    def test_incomplete_initializers_need_annotations_now(self):
        prefix = 'enum Option[T] { None, Some(T) } record Box[T] { value: T } '
        for expression in ('[]', '[[]]', 'None()', 'Some([])', 'Box(None())', 'map()', 'set()'):
            for declaration in ('value = ', 'mut value = '):
                with self.subTest(expression=expression, declaration=declaration):
                    self.assert_program(prefix + 'main(): Void { ' + declaration + expression + ' }',
                                        'cannot infer complete type for local value; add a type annotation')
        self.assert_program('main(): Void { mut names = []; names = names.append("Ada") }',
                            'cannot infer complete type for local names')
        self.assert_program('pure missing(): [Str] { values = []; values } main(): Void {}',
                            'cannot infer complete type for local values')
        self.assert_program(prefix + '''main(): Void {
  names: [Str] = []
  selected: Option[Str] = None()
  boxes: [Box[Str]] = []
  mut counts: Map[Str,Nat] = map()
  counts = counts.put("Ada", 1)
}''')

    def test_block_scope_no_shadowing_and_declaration_order(self):
        self.assert_program('''main(): Void {
  mut message = "waiting"
  if true { message = "ready"; local = "first"; print(local) }
  if false { local = "second"; print(local) }
}''')
        for source, error in (
            ('main(): Void { value = value + 1 }', 'unknown name value'),
            ('main(): Void { print(value); value = 1 }', 'unknown name value'),
            ('main(): Void { if true { value = 1 }; print(value) }', 'unknown name value'),
            ('main(): Void { value = 1; mut value = 2 }', 'shadows an existing binding'),
            ('main(): Void { value = 1; if true { value: Nat = 2 } }', 'shadows an existing binding'),
            ('main(): Void { value = 1; for value in 0..2 {} }', 'shadows an existing binding'),
            ('pure f(value: Nat): Nat { mut value = 2; value } main(): Void {}', 'shadows an existing binding'),
            ('pure f(value: Nat): Nat { value = 2; value } main(): Void {}', 'cannot assign to immutable local value'),
        ):
            with self.subTest(source=source):
                self.assert_program(source, error)

    def test_domain_types_are_preserved_but_not_invented(self):
        prefix = 'type Port = Nat where value >= 1 && value <= 65535; '
        self.assert_program(prefix + 'main(): Void { configured: Port = 8080; copied = configured; checked: Port = copied }')
        self.assert_program(prefix + 'main(): Void { configured: Port = 8080; mut copied = configured; copied = 0 }',
                            'cannot assign Nat to Port')
        self.assert_program(prefix + 'main(): Void { mut ordinary = 8080; ordinary = 0 }')
        self.assert_program(prefix + 'type Count = Nat where value <= 10; '
                            'main(): Void { port: Port = 8080; count: Count = 2; '
                            'values = [port, count]; checked: [Nat] = values }')

    def test_inferred_callables_keep_effects_in_nested_scopes(self):
        self.assert_program('''
enum Option[T] { None, Some(T) }
pure increment(value: Nat): Nat { value + 1 }
pure use(): Nat {
  action = @increment
  alias = action
  mut result = alias.call(40)
  actions = [action]
  for item in actions { copied = item; result = copied.call(result) }
  selected = Some(action)
  match selected {
    Some(item) => { copied = item; copied.call(result) },
    None() => result
  }
}
main(): Void { print(use()) }
''')
        self.assert_program('''
noisy(value: Nat): Nat { print(value); value }
pure bad(): Nat { action = @noisy; alias = action; alias.call(1) }
main(): Void {}
''', 'pure function cannot call impure function call')
        self.assert_program('pure bad(): Str { value = read_file("x"); value } main(): Void {}',
                            'pure function cannot call impure function read_file')
        self.assert_program('pure quiet(value: Nat): Nat { value } '
                            'noisy(value: Nat): Nat { print(value); value } '
                            'main(): Void { mut action = @quiet; action = @noisy }',
                            'cannot assign Fn')

    def test_void_and_incompatible_values_are_rejected(self):
        for source, error in (
            ('value = print("hello")', 'Void'),
            ('value = if true { "hello" }', 'Void'),
            ('value = ["hello", false]', 'array elements have incompatible types'),
            ('value = if true { "hello" } else { false }', 'branches have incompatible types'),
        ):
            with self.subTest(source=source):
                self.assert_program('main(): Void { ' + source + ' }', error)
        self.assert_program('''
pure natural(value: Nat): Nat { value }
pure signed(value: Int): Nat { 1 }
main(): Void { actions = [@natural, @signed] }
''', 'array elements have incompatible types')


if __name__ == '__main__':
    unittest.main()
