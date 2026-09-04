import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import Code, PanackeltyError, VM, build, load_bytecode, verify_bytecode
from tests.unit.support import PanackeltyTestCase


class TypeTests(PanackeltyTestCase):
    def test_records_enums_and_exhaustive_match(self):
        code = self.compile("""
record Position { line: Nat, column: Nat }
enum MaybeNat { None, Some(Nat) }

pure value_or(value: MaybeNat, fallback: Nat): Nat {
  match value { Some(found) => found, None() => fallback }
}

main(): Void {
  position: Position = Position(3, 14);
  print(position.line + position.column);
  print(value_or(Some(25), 0));
  print(value_or(None(), 7));
}
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "17\n25\n7\n")

    def test_generic_records_option_and_result_inference(self):
        code = self.compile("""
record Pair[A, B] { first: A, second: B }
enum Option[T] { None, Some(T) }
enum Result[T, E] { Ok(T), Error(E) }

pure choose(flag: Bool): Option[Nat] {
  if flag { Some(42) } else { None() }
}

pure divide(a: Nat, b: Nat): Result[Nat,Str] {
  if b == 0 { Error("zero") } else { Ok(a / b) }
}

pure unwrap(value: Option[Nat]): Nat {
  match value { None() => 0, Some(found) => found }
}

main(): Void {
  pair: Pair[Str,Nat] = Pair("answer", 42);
  print(pair.first);
  print(unwrap(choose(true)));
  print(unwrap(choose(false)));
}
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "answer\n42\n0\n")

if __name__ == "__main__":
    unittest.main()
