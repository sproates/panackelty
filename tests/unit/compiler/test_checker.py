import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import Code, PanackeltyError, VM, build, load_bytecode, verify_bytecode
from tests.unit.support import PanackeltyTestCase


class CheckerTests(PanackeltyTestCase):
    def test_guard_accepts_proven_literal(self):
        self.compile("""
type Positive = Int where value > 0;
pure id(x: Positive): Positive { x }
main(): Void { x: Positive = 3; print(id(x)); }
""")

    def test_guard_rejects_bad_literal(self):
        with self.assertRaisesRegex(PanackeltyError, "guard is not proven"):
            self.compile("""
type Positive = Int where value > 0;
main(): Void { x: Positive = 0; print(x); }
""")

    def test_pure_function_cannot_print(self):
        with self.assertRaisesRegex(PanackeltyError, "pure function cannot call impure"):
            self.compile("pure main(): Void { print(1); }")

    def test_guard_is_proven_by_if_fact(self):
        self.compile("""
type Positive = Int where value > 0;
pure positive(x: Int): Int {
  if x > 0 { safe: Positive = x; safe } else { 0 }
}
main(): Void { print(positive(8)); }
""")

    def test_assignment_requires_mut(self):
        with self.assertRaisesRegex(PanackeltyError, "immutable local"):
            self.compile("""
pure bad(): Nat { value: Nat = 1; value = 2; value }
main(): Void { print(bad()); }
""")

    def test_pure_loop_cannot_hide_io(self):
        with self.assertRaisesRegex(PanackeltyError, "pure function cannot call impure"):
            self.compile("""
pure bad(): Void {
  for value in 0..3 { print(value); }
}
main(): Void {}
""")

    def test_loop_variable_cannot_overwrite_outer_binding(self):
        with self.assertRaisesRegex(PanackeltyError, "shadows an existing binding"):
            self.compile("""
pure bad(): Nat {
  value: Nat = 10;
  for value in 0..3 {}
  value
}
main(): Void {}
""")

    def test_old_primitive_names_are_rejected(self):
        with self.assertRaisesRegex(PanackeltyError, "unknown type String"):
            self.compile('pure old(value: String): String { value } main(): Void {}')

    def test_int_accepts_negative_values_but_nat_does_not(self):
        code = self.compile("pure identity(value: Int): Int { value } main(): Void { print(identity(-42)); }")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "-42\n")
        with self.assertRaisesRegex(PanackeltyError, "cannot assign Int to Nat"):
            self.compile("main(): Void { invalid: Nat = -1; print(invalid); }")

    def test_match_must_be_exhaustive(self):
        with self.assertRaisesRegex(PanackeltyError, "non-exhaustive match; missing None"):
            self.compile("""
enum MaybeNat { None, Some(Nat) }
pure unwrap(value: MaybeNat): Nat { match value { Some(found) => found } }
main(): Void {}
""")

    def test_constructor_arguments_are_checked(self):
        with self.assertRaisesRegex(PanackeltyError, "Some expects 1 arguments, got 0"):
            self.compile("""
enum MaybeNat { None, Some(Nat) }
main(): Void { print(Some()); }
""")

if __name__ == "__main__":
    unittest.main()
