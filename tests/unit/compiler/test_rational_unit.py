import unittest
from panackelty import Checker, PanackeltyError, Parser, lex
from tests.unit.support import CompilerHarnessTestCase


class RationalUnitTypeTests(CompilerHarnessTestCase):
    def check_both(self, source, valid=True):
        result = self.run_harness(
            'compiler/purity.panack',
            'main(): Void { print(check_source_frontend(command_args()[0])) }',
            [source],
        ).strip()
        if valid:
            self.assertEqual(result, 'ok', source)
            Checker(Parser(lex(source)).parse()).check()
        else:
            self.assertNotEqual(result, 'ok', source)
            with self.assertRaises(PanackeltyError):
                Checker(Parser(lex(source)).parse()).check()

    def test_rational_and_unit_types_in_generic_values_and_callbacks(self):
        self.check_both('''
enum Result[T,E] { Ok(T), Error(E) }
record Box[T] { value: T }
pure identity[T](x: T): T { x }
pure ignore(x: Rat): Unit { () }
pure done(): Result[Unit,Str] { Ok(()) }
main(): Void {
 third = 1/3
 ten: Rat = third * 30
 x: Nat = ten.nat()
 d: Dec = (1/8).dec()
 negative: Rat = -third
 sum: Rat = 1 + third
 values: [Unit] = [third].map(@ignore)
 box: Box[Unit] = Box(identity(()))
 result: Result[Unit,Str] = done()
 print(1/2 == 2/4)
 print(1 < third)
 print(quotient(7, 2))
}
''')

    def test_invalid_conversions_arithmetic_and_void_remain_rejected(self):
        for body in ('x: Nat = 6/3', 'x: Rat = 1', 'x = (1/3) + 0.5',
                     'x = (1/3) % 2', 'x = ().nat()', 'x = (1/3).nat(2)',
                     'x = 1.nat()', 'x = () + ()', 'x = () < ()',
                     'x: Unit = print(1)', 'x: Void = ()',
                     'x = quotient(-7, 2)', 'x = (1/3).dec(2)'):
            with self.subTest(body=body):
                self.check_both('main(): Void { ' + body + ' }', False)
        self.check_both('pure done(): Unit {} main(): Void {}', False)
        self.check_both('pure f[Unit](x: Unit): Unit { x } main(): Void {}', False)
        self.check_both('pure f[Rat](x: Rat): Rat { x } main(): Void {}', False)


if __name__ == '__main__':
    unittest.main()
