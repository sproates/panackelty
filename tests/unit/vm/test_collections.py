import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import Code, PanackeltyError, VM, build, load_bytecode, verify_bytecode
from tests.unit.support import PanackeltyTestCase


class CollectionTests(PanackeltyTestCase):
    def test_empty_array_append_and_concat_are_pure(self):
        code = self.compile("""
pure values(): [Nat] {
  mut result: [Nat] = [];
  result = append(result, 2);
  result = append(result, 3);
  concat(result, [5, 7])
}
main(): Void { print(values()); }
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "[2, 3, 5, 7]\n")

    def test_persistent_maps_and_sets_are_pure(self):
        code = self.compile("""
pure symbols(): Map[Str,Nat] {
  mut values: Map[Str,Nat] = map();
  values = map_put(values, "answer", 42);
  values = map_put(values, "other", 7);
  values
}
pure names(): Set[Str] {
  mut values: Set[Str] = set();
  values = set_add(values, "x");
  values = set_add(values, "x");
  values
}
main(): Void {
  values: Map[Str,Nat] = symbols();
  print(map_get(values, "answer"));
  print(map_has(values, "missing"));
  print(set_has(names(), "x"));
}
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "42\nfalse\ntrue\n")

    def test_short_collection_aliases_dispatch_by_receiver_type(self):
        code = self.compile("""
main(): Void {
  mut values: Map[Str,Nat] = map();
  values = values.put("answer", 42);
  mut names: Set[Str] = set();
  names = names.add("Panackelty");
  print(values.has("answer"));
  print(values.get("answer"));
  print(names.has("Panackelty"));
}
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "true\n42\ntrue\n")

    def test_byte_buffers_and_utf8(self):
        code = self.compile("""
pure encoded(): Bytes {
  mut result: Bytes = utf8_encode("λ");
  result = byte_append(result, 33);
  result
}
main(): Void {
  data: Bytes = encoded();
  print(data);
  print(byte_len(data));
  print(byte_get(data, 0));
  print(utf8_decode(data));
}
""")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(code).run()
        self.assertEqual(output.getvalue(), "bytes(cebb21)\n3\n206\nλ!\n")

if __name__ == "__main__":
    unittest.main()
