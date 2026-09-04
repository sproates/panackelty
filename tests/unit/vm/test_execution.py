import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import Code, PanackeltyError, VM, build, load_bytecode, verify_bytecode
from tests.unit.support import PanackeltyTestCase


class ExecutionTests(PanackeltyTestCase):
    def test_pure_for_loop_with_mutable_accumulator(self):
        code = self.compile("""
pure sum_below(n: Nat): Nat {
  mut total: Nat = 0;
  for value in 0..n {
    total = total + value;
  }
  total
}
main(): Void { print(sum_below(100)); }
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "4950\n")

    def test_pure_while_loop_with_mutable_locals(self):
        code = self.compile("""
pure factorial(n: Nat): Nat {
  mut cursor: Nat = n;
  mut product: Nat = 1;
  while cursor > 0 {
    product = product * cursor;
    cursor = cursor - 1;
  }
  product
}
main(): Void { print(factorial(30)); }
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "265252859812191058636308480000000\n")

    def test_arrays_iteration_indexing_and_len(self):
        code = self.compile("""
pure sum(values: [Nat]): Nat {
  mut total: Nat = 0;
  for value in values { total = total + value; }
  total
}
main(): Void {
  values: [Nat] = [2, 3, 5, 7, 11];
  print(sum(values) + values[2] + len(values));
}
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "38\n")

    def test_array_index_is_bounds_checked_by_vm(self):
        code = self.compile("main(): Void { xs: [Nat] = [1]; print(xs[2]); }")
        with self.assertRaisesRegex(PanackeltyError, "out of bounds"):
            VM(code).run()

    def test_str_interpolation_concatenation_and_numeric_values(self):
        code = self.compile("""
pure message(name: Str, count: Nat, delta: Int, ratio: Dec): Str {
  "Hello, ${name}: " + "${count}, ${delta}, ${ratio}"
}
main(): Void { print(message("Ada", 7, -2, 0.125)); }
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "Hello, Ada: 7, -2, 0.125\n")

    def test_str_unicode_indexing_and_length(self):
        code = self.compile("""
pure select(text: Str, index: Nat): Str { text[index] }
main(): Void {
  text: Str = "Aλ🙂";
  print(select(text, 1) + select(text, 2));
  print(len(text));
}
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "λ🙂\n3\n")

    def test_string_methods_reverse_unicode_code_points(self):
        code = self.compile('main(): Void { print("Aλ🙂".reverse()); }')
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "🙂λA\n")

    def test_str_index_is_bounds_checked_by_vm(self):
        code = self.compile('main(): Void { print("hi"[2]); }')
        with self.assertRaisesRegex(PanackeltyError, "out of bounds"):
            VM(code).run()

    def test_boolean_operators_short_circuit(self):
        code = self.compile("""
pure safe(text: Str, index: Nat): Bool {
  index < len(text) && is_letter(text[index])
}
main(): Void { print(safe("", 0)); print(true || safe("", 0)); }
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "false\ntrue\n")

if __name__ == "__main__":
    unittest.main()
