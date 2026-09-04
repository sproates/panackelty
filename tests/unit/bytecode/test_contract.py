import contextlib
import io
import unittest

from panackelty import Code, PanackeltyError, VM, verify_bytecode
from tests.unit.forged_runtime import FORGED_DYNAMIC_FAILURES


VOID_RETURN = [("CONST", ("Void", None)), ("RETURN", None)]


class BytecodeContractTests(unittest.TestCase):
    @staticmethod
    def execute(functions):
        verify_bytecode(functions)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            result = VM(functions).run()
        return output.getvalue(), result

    def test_binary_operands_preserve_push_order(self):
        functions = {
            "main": Code(
                "main",
                [],
                [
                    ("CONST", ("Nat", 7)),
                    ("CONST", ("Nat", 2)),
                    ("BINARY", "-"),
                    ("CALL", ("print", 1)),
                    ("RETURN", None),
                ],
            )
        }
        output, result = self.execute(functions)
        self.assertEqual(output, "5\n")
        self.assertEqual((result.type_name, result.data), ("Void", None))

    def test_call_frames_have_isolated_locals_and_return_values(self):
        functions = {
            "main": Code(
                "main",
                [],
                [
                    ("CONST", ("Nat", 7)),
                    ("STORE", "value"),
                    ("CONST", ("Nat", 9)),
                    ("CALL", ("helper", 1)),
                    ("POP", None),
                    ("LOAD", "value"),
                    ("CALL", ("print", 1)),
                    ("RETURN", None),
                ],
            ),
            "helper": Code(
                "helper",
                ["input"],
                [
                    ("LOAD", "input"),
                    ("STORE", "value"),
                    ("LOAD", "value"),
                    ("RETURN", None),
                ],
                pure=True,
            ),
        }
        output, _ = self.execute(functions)
        self.assertEqual(output, "7\n")

    def test_indirect_calls_validate_and_deliver_results(self):
        functions = {
            "main": Code(
                "main",
                [],
                [
                    ("CONST", ("Str", "identity")),
                    ("CONST", ("Nat", 42)),
                    ("CALL_VALUE", 1),
                    ("CALL", ("print", 1)),
                    ("RETURN", None),
                ],
            ),
            "identity": Code(
                "identity",
                ["value"],
                [("LOAD", "value"), ("RETURN", None)],
                pure=True,
            ),
        }
        output, _ = self.execute(functions)
        self.assertEqual(output, "42\n")

    def test_indirect_calls_recheck_purity_at_runtime(self):
        functions = {
            "main": Code(
                "main",
                [],
                [
                    ("CONST", ("Str", "effect")),
                    ("CALL_VALUE", 0),
                    ("RETURN", None),
                ],
                pure=True,
            ),
            "effect": Code("effect", [], VOID_RETURN, pure=False),
        }
        verify_bytecode(functions)
        with self.assertRaisesRegex(PanackeltyError, "pure function invokes impure callable"):
            VM(functions).run()

    def test_conditional_jump_consumes_its_condition(self):
        functions = {
            "main": Code(
                "main",
                [],
                [
                    ("CONST", ("Bool", False)),
                    ("JUMP_FALSE", 4),
                    ("CONST", ("Nat", 1)),
                    ("JUMP", 5),
                    ("CONST", ("Nat", 2)),
                    ("CALL", ("print", 1)),
                    ("RETURN", None),
                ],
            )
        }
        output, _ = self.execute(functions)
        self.assertEqual(output, "2\n")

    def test_invalid_dynamic_state_becomes_a_vm_trap(self):
        cases = {
            "stack underflow": [("POP", None), *VOID_RETURN],
            "uninitialized local": [("LOAD", "missing"), ("RETURN", None)],
            "missing call argument": [("CALL", ("print", 1)), ("RETURN", None)],
            "wrong runtime operand": [
                ("CONST", ("Nat", 1)),
                ("FIELD_GET", "field"),
                ("RETURN", None),
            ],
        }
        for name, instructions in cases.items():
            with self.subTest(case=name):
                functions = {"main": Code("main", [], instructions)}
                verify_bytecode(functions)
                with self.assertRaisesRegex(
                    PanackeltyError,
                    "VM trap:",
                ):
                    VM(functions).run()

    def test_forged_runtime_safety_failures_trap_in_the_oracle(self):
        for name, instructions, messages in FORGED_DYNAMIC_FAILURES:
            with self.subTest(case=name):
                functions = {"main": Code("main", [], instructions)}
                verify_bytecode(functions)
                with self.assertRaises(PanackeltyError) as raised:
                    VM(functions).run()
                self.assertIn("VM trap:", str(raised.exception))
                for message in messages:
                    self.assertIn(message, str(raised.exception))


if __name__ == "__main__":
    unittest.main()
