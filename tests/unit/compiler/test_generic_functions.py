import unittest

from panackelty import Checker, Compiler, PanackeltyError, Parser, lex
from tests.unit.support import CompilerHarnessTestCase, PanackeltyTestCase
from tests.unit.compiler.test_self_hosted_emitter import render_bootstrap


class GenericFunctionTests(CompilerHarnessTestCase):
    def assert_source(self, source, error=None):
        actual = self.run_harness(
            "compiler/purity.panack",
            "main(): Void { print(check_source_frontend(command_args()[0])) }",
            [source],
        ).strip()
        if error is None:
            self.assertEqual(actual, "ok", source)
            Checker(Parser(lex(source)).parse()).check()
        else:
            self.assertIn(error, actual, source)
            with self.assertRaises(PanackeltyError):
                Checker(Parser(lex(source)).parse()).check()

    def test_inference_explicit_arguments_and_lexical_type_scope(self):
        self.assert_source('''
record Box[T] { value: T }
enum Option[T] { None, Some(T) }
pure identity[T](value: T): T { local: T = value; local }
pure first[T](values: [T]): Option[T] {
  if values.len() == 0 { None() } else { Some(values[0]) }
}
pure unwrap[T](box: Box[T]): T { identity[T](box.value) }
pure choose[T](values: [T], fallback: T): T {
  if values.len() == 0 { fallback } else { values[0] }
}
main(): Void {
  n: Nat = identity(7)
  s: Str = "text".identity[Str]()
  b: Bool = identity[Bool](true)
  nested: [[Str]] = identity[[[Str]]]([["nested"]])
  empty: Option[Str] = first[Str]([])
  result: Nat = unwrap(Box(4))
  combined: Str = choose([], "fallback")
  widened: Int = choose([1], -1)
  indexed: Nat = [1, 2][0]
}
''')

    def test_recursion_mutual_calls_and_abstract_callback_parameters(self):
        self.assert_source('''
pure repeat[T](value: T, count: Nat): T {
 if count >= 1 { repeat[T](value, count - 1) } else { value }
}
pure apply[T,U](value: T, operation: PureFn[T,U]): U { operation.call(value) }
pure label(value: Nat): Str { "number" }
pure forward[A](value: A): A { backward(value) }
pure backward[B](value: B): B { value }
main(): Void {
  x: Str = apply(repeat(3, 2), @label)
  y: Bool = forward(true)
}
''')

    def test_empty_evidence_and_argument_order(self):
        for expression in ('choose([], ["x"])', 'choose(["x"], [])'):
            self.assert_source('pure choose[T](a: [T], b: [T]): [T] { a.concat(b) } '
                               'main(): Void { x: [Str] = ' + expression + ' }')
        self.assert_source('pure empty[T](): [T] { [] } main(): Void { x: [Nat] = empty[Nat]() }')
        self.assert_source('pure empty[T](): [T] { [] } main(): Void { x: [Nat] = empty() }',
                           'cannot infer type parameter T')
        self.assert_source('pure same[T](x: T): T { x } main(): Void { same([]) }',
                           'cannot infer type parameter T')

    def test_constructor_evidence_and_explicit_multiple_type_arguments(self):
        self.assert_source("""
enum Result[T,E] { Ok(T), Error(E) }
pure recover[T,E](value: Result[T,E], fallback: T): T {
 match value { Ok(item) => item, Error(message) => fallback }
}
main(): Void {
  failed: Nat = recover(Error("missing"), 7)
  success: Nat = recover[Nat,Str](Ok(5), 7)
}
""")
        self.assert_source("""
enum Result[T,E] { Ok(T), Error(E) }
pure recover[T,E](value: Result[T,E], fallback: T): T {
 match value { Ok(item) => item, Error(message) => fallback }
}
main(): Void { recover(Ok(5), 7) }
""", 'cannot infer type parameter E')

    def test_invalid_declarations_and_unknown_types(self):
        cases = (
            ('pure f[T,T](x: T): T { x } main(): Void {}', 'duplicate type parameter'),
            ('pure f[Nat](x: Nat): Nat { x } main(): Void {}', 'conflicts with a type name'),
            ('record Box[T] { value: T } pure f[Box](x: Box): Box { x } main(): Void {}', 'conflicts with a type name'),
            ('main[T](): Void {}', 'main cannot have type parameters'),
            ('pure f[T](x: T): Missing { x } main(): Void {}', 'unknown type Missing'),
            ('pure f[T](x: T): T { x } main(): Void { y: T = 1 }', 'unknown type T'),
            ('pure f[T](x: T): T { y: U = x; x } main(): Void {}', 'unknown type U'),
        )
        for source, error in cases:
            with self.subTest(source=source): self.assert_source(source, error)

    def test_invalid_calls_and_conflicting_evidence(self):
        prefix = 'pure same[T](a: T, b: T): T { a } '
        for body, error in (
            ('same(1, "x")', 'incompatible type arguments'),
            ('same[Nat,Str](1, 2)', 'expects 1 type arguments'),
            ('same[Missing](1, 2)', 'unknown type Missing'),
            ('same[Void](1, 2)', 'Void is only valid'),
            ('same[Str](1, "x")', 'argument 1'),
            ('same(1)', 'expects 2 arguments'),
            ('print[Nat](1)', 'explicit type arguments require a generic function'),
            ('same(print(1), print(2))', 'Void'),
        ):
            with self.subTest(body=body): self.assert_source(prefix+'main(): Void { '+body+' }',error)
        self.assert_source('pure f[T](a: T, b: T, c: T): T { a } main(): Void { f(1, "x", 2) }',
                           'incompatible type arguments')
        self.assert_source('pure f(x: Nat): Nat { x } main(): Void { f[Nat](1) }',
                           'explicit type arguments require a generic function')

    def test_bodies_are_checked_even_when_unused(self):
        for body, error in (
            ('x + x', 'operator +'), ('x.len()', 'len expects'),
            ('1', 'returns Nat, expected T'), ('missing', 'unknown name missing'),
        ):
            with self.subTest(body=body):
                self.assert_source('pure f[T](x: T): T { '+body+' } main(): Void {}',error)
        self.assert_source('pure f[T](x: T): T { print(x); x } main(): Void {}',
                           'pure function cannot call impure')

    def test_purity_and_generic_function_references(self):
        self.assert_source('f[T](x: T): T { print(x); x } pure g(): Nat { f(1) } main(): Void {}',
                           'pure function cannot call impure')
        self.assert_source('pure f[T](x: T): T { x } main(): Void { operation = @f }',
                           'generic function references require a non-generic wrapper')
        self.assert_source('''
pure f[T](x: T): T { x }
pure natural(x: Nat): Nat { f(x) }
main(): Void { result: [Nat] = [1,2].map(@natural) }
''')

    def test_generic_helpers_cannot_hide_callable_effects(self):
        prefix = """
effect(value: Nat): Nat { print(value); value }
pure identity[T](value: T): T { value }
pure pass_callback[T](value: Fn[T,T]): Fn[T,T] { value }
"""
        for body in ('identity(@effect).call(1)', 'pass_callback(@effect).call(1)'):
            with self.subTest(body=body):
                self.assert_source(prefix + 'pure leak(): Nat { ' + body + ' } main(): Void {}',
                                   'pure function cannot call impure function call')
        self.assert_source("""
effect(value: Nat): Nat { print(value); value }
pure apply[T,U](value: T, op: PureFn[T,U]): U { op.call(value) }
main(): Void { apply(1, @effect) }
""", 'argument 2')

    def test_guarded_types_are_preserved(self):
        self.assert_source('''
type Port = Nat where value >= 1 && value <= 65535
pure identity[T](x: T): T { x }
pure with_port[T](x: T, port: Port): T { x }
main(): Void { port: Port = 80; copy: Port = identity(port); text: Str = with_port("ok", 80) }
''')
        self.assert_source('''
type Port = Nat where value >= 1 && value <= 65535
pure identity[T](x: T): T { x }
main(): Void { invalid: Port = identity(0) }
''', 'cannot assign Nat to Port')

    def test_emission_erases_type_arguments_and_preserves_one_body(self):
        source = 'pure identity[T](x: T): T { x } main(): Void { print(identity(1)); print(identity[Str]("x")) }'
        emitted = self.run_harness('compiler/emitter.panack',
            'main(): Void { print(compile_source_disassembly(command_args()[0])) }', [source]).removesuffix('\n')
        self.assertEqual(emitted, render_bootstrap(source))
        program = Parser(lex(source)).parse()
        Checker(program).check()
        code = Compiler(program).compile()
        self.assertEqual(set(code), {'main', 'identity'})
        self.assertEqual(PanackeltyTestCase.run_code(code), '1\nx\n')


if __name__ == '__main__':
    unittest.main()
