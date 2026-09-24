"""Deterministic allocation/syscall failures; no production fault hooks are linked."""
from decimal import Decimal
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from panackelty import Code, build, bytecode_bytes

PROJECT = Path(__file__).resolve().parents[3]
OWNERSHIP_SOURCE = '''
import stdlib/option
record Holder { label: Str, values: [Nat] }
pure identity(value: Nat): Nat { value }
main(): Nat {
    items: [Nat] = [1, 2, 3]
    holder: Holder = Holder("numbers", items)
    mut total: Nat = 0
    for item in holder.values { total = total + identity(item) }
    negative: Dec = -1.25
    label: Str = holder.label
    text: Str = "value ${negative} ${label}"
    encoded: Bytes = utf8_encode(text)
    values: Map[Str,Nat] = map_put(map(), "total", total)
    names: Set[Str] = set_add(set(), "total")
    operation: PureFn[Nat,Nat] = @identity
    choice: Option[Nat] = Some(operation.call(map_get(values, "total")))
    answer: Nat = match choice { Some(value) => value, None() => 0 }
    if answer == 6 && set_has(names, "total") && byte_len(encoded) > 0 { 42 } else { 0 }
}
'''


def all_operand_forms():
    constants = [("CONST", value) for value in
                 [("Nat", 10**36), ("Int", -10**36), ("Dec", Decimal("-123.4500")),
                  ("Str", "λ\0🙂"), ("Bool", True), ("Void", None)]]
    operations = constants + [
        ("LOAD", "value"), ("STORE", "value"), ("POP", None), ("UNARY", "-"),
        ("BINARY", "+"), ("MAKE_RANGE", None), ("MAKE_ARRAY", 3), ("INDEX_GET", None),
        ("INTERPOLATE", ["before", "after"]), ("ITER_INIT", "iterator"),
        ("ITER_NEXT", ("iterator", "value", 0)), ("MAKE_RECORD", ("Pair", ["a", "b"])),
        ("FIELD_GET", "a"), ("MAKE_VARIANT", ("Option", "Some", 1)),
        ("MATCH_VARIANT", ("Some", 0)), ("MATCH_FAIL", None), ("CALL", ("identity", 1)),
        ("JUMP_FALSE", 0), ("JUMP", 0), ("RETURN", None), ("CALL_VALUE", 1),
    ]
    # Structurally/semantically valid, deliberately not executable. Never run this corpus.
    return bytecode_bytes({
        "identity": Code("identity", ["value"], [("LOAD", "value"), ("RETURN", None)]),
        "main": Code("main", [], operations),
    })


class NativeFaultTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.executable = Path(os.environ.get("PANACK_NATIVE_FAULT_TEST", PROJECT / "build/fault/test_faults"))
        cls.modules = Path(os.environ.get("PANACK_NATIVE_MODULE_TEST", PROJECT / "build/vm/test_modules"))
        result = subprocess.run(["make", "--no-print-directory", "native-fault-build", "native-module-build"],
                                cwd=PROJECT, capture_output=True, text=True)
        if result.returncode:
            raise AssertionError(result.stdout + result.stderr)

    def run_contract(self, executable, *args):
        result = subprocess.run([str(executable), *map(str, args)], capture_output=True,
                                text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr[-4000:])
        return result.stdout

    def test_allocation_and_host_failures_release_resources(self):
        self.assertIn("allocation failures checked", self.run_contract(self.executable))

    def test_every_operand_form_handles_decode_failures_and_mutations(self):
        with tempfile.TemporaryDirectory() as directory:
            artifact = Path(directory) / "operands.bc"
            artifact.write_bytes(all_operand_forms())
            self.run_contract(self.executable, "decode", artifact)
            self.assertIn("decoder corpus:", self.run_contract(self.modules, "mutate", artifact))

    def test_execution_and_nested_traps_release_partial_values(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "ownership.panack"
            artifact = source.with_suffix(".bc")
            for mode in ("execute", "trap"):
                text = OWNERSHIP_SOURCE
                if mode == "trap":
                    text = text.replace("pure identity(value: Nat): Nat { value }",
                                        "pure identity(value: Nat): Nat { [value][9] }")
                source.write_text(text)
                functions = build(source)
                opcodes = {op for function in functions.values() for op, _ in function.instructions}
                self.assertTrue({"INTERPOLATE", "UNARY", "CALL_VALUE", "ITER_NEXT",
                                 "MAKE_RECORD", "MAKE_VARIANT"} <= opcodes)
                artifact.write_bytes(bytecode_bytes(functions))
                self.run_contract(self.executable, mode, artifact)
