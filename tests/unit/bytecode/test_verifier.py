import decimal
import unittest
from unittest.mock import patch

from panackelty import Code, PanackeltyError, verify_bytecode
from tests.unit.support import PanackeltyTestCase


VALID_RETURN = [("CONST", ("Void", None)), ("RETURN", None)]


class VerifierTests(PanackeltyTestCase):
    @staticmethod
    def program(instructions, *, pure=False):
        return {"main": Code("main", [], instructions, pure=pure)}

    def assert_rejected(self, instructions, message):
        with self.assertRaisesRegex(PanackeltyError, message):
            verify_bytecode(self.program(instructions))

    def test_requires_valid_entry_point(self):
        with self.assertRaisesRegex(PanackeltyError, "no main function"):
            verify_bytecode({"helper": Code("helper", [], VALID_RETURN)})
        with self.assertRaisesRegex(PanackeltyError, "main function cannot take parameters"):
            verify_bytecode({"main": Code("main", ["argument"], VALID_RETURN)})

    def test_rejects_invalid_function_signatures(self):
        cases = (
            ("empty name", Code("", [], VALID_RETURN)),
            ("duplicate parameters", Code("helper", ["value", "value"], VALID_RETURN)),
        )
        for name, helper in cases:
            with self.subTest(case=name):
                functions = {"main": Code("main", [], VALID_RETURN), "helper": helper}
                with self.assertRaisesRegex(PanackeltyError, "invalid function signature"):
                    verify_bytecode(functions)

        with self.assertRaisesRegex(PanackeltyError, "does not match function name"):
            verify_bytecode({"main": Code("other", [], VALID_RETURN)})

    def test_rejects_empty_functions_and_missing_returns(self):
        with self.assertRaisesRegex(PanackeltyError, "is empty"):
            verify_bytecode(self.program([]))
        with self.assertRaisesRegex(PanackeltyError, "has no RETURN"):
            verify_bytecode(self.program([("CONST", ("Void", None))]))

    def test_rejects_malformed_and_unknown_instructions(self):
        self.assert_rejected([["RETURN", None]], "malformed instruction")
        self.assert_rejected([("EXPLODE", None), ("RETURN", None)], "unknown bytecode instruction")
        self.assert_rejected([(1, None), ("RETURN", None)], "unknown bytecode instruction")

    def test_rejects_invalid_simple_operands(self):
        cases = (
            ("LOAD", None, "invalid LOAD operand"),
            ("STORE", 1, "invalid STORE operand"),
            ("ITER_INIT", (), "invalid ITER_INIT operand"),
            ("FIELD_GET", False, "invalid FIELD_GET operand"),
            ("POP", 0, "invalid POP operand"),
            ("MAKE_RANGE", 0, "invalid MAKE_RANGE operand"),
            ("INDEX_GET", "index", "invalid INDEX_GET operand"),
            ("MATCH_FAIL", False, "invalid MATCH_FAIL operand"),
            ("RETURN", 0, "invalid RETURN operand"),
            ("UNARY", "~", "invalid UNARY operand"),
            ("BINARY", "**", "invalid BINARY operand"),
            ("MAKE_ARRAY", -1, "invalid MAKE_ARRAY operand"),
            ("MAKE_ARRAY", True, "invalid MAKE_ARRAY operand"),
        )
        for op, operand, message in cases:
            with self.subTest(op=op, operand=operand):
                self.assert_rejected([(op, operand), ("RETURN", None)], message)

    def test_rejects_invalid_control_flow_targets(self):
        cases = (
            ("JUMP", -1),
            ("JUMP", 2),
            ("JUMP_FALSE", True),
            ("MATCH_VARIANT", ("Some", 2)),
            ("ITER_NEXT", ("iterator", "item", -1)),
        )
        for op, operand in cases:
            with self.subTest(op=op, operand=operand):
                message = "invalid jump target" if op.startswith("JUMP") else f"invalid {op} operand"
                self.assert_rejected([(op, operand), ("RETURN", None)], message)

    def test_rejects_invalid_composite_operands(self):
        cases = (
            ("MAKE_RECORD", ("Pair", ["left", 2])),
            ("MAKE_RECORD", ("Pair",)),
            ("MAKE_VARIANT", ("Option", "Some", -1)),
            ("MAKE_VARIANT", ("Option", 1, 0)),
            ("MATCH_VARIANT", ("Some", "next")),
            ("INTERPOLATE", []),
            ("INTERPOLATE", ["text", 1]),
            ("ITER_NEXT", ("iterator", 1, 0)),
        )
        for op, operand in cases:
            with self.subTest(op=op, operand=operand):
                self.assert_rejected(
                    [(op, operand), ("RETURN", None)],
                    f"invalid {op} operand",
                )

    def test_rejects_invalid_constants(self):
        cases = (
            (None, "invalid CONST operand"),
            ((1, 2), "invalid CONST operand"),
            (("Nat", -1), "invalid Nat constant"),
            (("Nat", True), "invalid Nat constant"),
            (("Int", True), "invalid Int constant"),
            (("Dec", decimal.Decimal("NaN")), "invalid Dec constant"),
            (("Str", 1), "invalid Str constant"),
            (("Bool", 1), "invalid Bool constant"),
            (("Void", 0), "invalid Void constant"),
            (("Unknown", None), "invalid Unknown constant"),
        )
        for operand, message in cases:
            with self.subTest(operand=operand):
                self.assert_rejected([("CONST", operand), ("RETURN", None)], message)

    def test_rejects_malformed_unknown_and_wrong_arity_calls(self):
        cases = (
            (("print",), "invalid CALL operand"),
            (("print", True), "invalid CALL operand"),
            (("missing", 0), "calls unknown function"),
            (("print", 0), "invalid arity"),
        )
        for operand, message in cases:
            with self.subTest(operand=operand):
                self.assert_rejected([("CALL", operand), ("RETURN", None)], message)

        functions = {
            "main": Code("main", [], [("CALL", ("helper", 0)), ("RETURN", None)]),
            "helper": Code("helper", ["value"], VALID_RETURN, pure=True),
        }
        with self.assertRaisesRegex(PanackeltyError, "invalid arity"):
            verify_bytecode(functions)

    def test_bytecode_verifier_preserves_purity(self):
        functions = self.program(
            [("CONST", ("Nat", 1)), ("CALL", ("print", 1)), ("RETURN", None)],
            pure=True,
        )
        with self.assertRaisesRegex(PanackeltyError, "pure bytecode function"):
            verify_bytecode(functions)

    def test_validates_indirect_call_arity_operand(self):
        for operand in (-1, True, "1", None):
            with self.subTest(operand=operand):
                self.assert_rejected(
                    [("CALL_VALUE", operand), ("RETURN", None)],
                    "invalid CALL_VALUE operand",
                )

    def test_enforces_resource_limits_on_in_memory_bytecode(self):
        main = Code("main", [], VALID_RETURN)
        helper = Code("helper", [], VALID_RETURN, pure=True)
        cases = (
            (
                "MAX_BYTECODE_FUNCTIONS",
                1,
                {"main": main, "helper": helper},
                "function limit",
            ),
            (
                "MAX_BYTECODE_PARAMETERS",
                0,
                {
                    "main": main,
                    "helper": Code("helper", ["value"], VALID_RETURN, pure=True),
                },
                "parameter limit",
            ),
            (
                "MAX_BYTECODE_INSTRUCTIONS_PER_FUNCTION",
                1,
                {"main": main},
                "instruction limit",
            ),
            (
                "MAX_BYTECODE_TOTAL_INSTRUCTIONS",
                3,
                {"main": main, "helper": helper},
                "total instruction limit",
            ),
            (
                "MAX_BYTECODE_NAME_BYTES",
                3,
                {"main": main},
                "name limit",
            ),
            (
                "MAX_BYTECODE_TEXT_BYTES",
                3,
                {
                    "main": Code(
                        "main",
                        [],
                        [("CONST", ("Str", "four")), ("RETURN", None)],
                    )
                },
                "text limit",
            ),
            (
                "MAX_BYTECODE_NUMERIC_DIGITS",
                2,
                {
                    "main": Code(
                        "main",
                        [],
                        [("CONST", ("Nat", 100)), ("RETURN", None)],
                    )
                },
                "digit limit",
            ),
            (
                "MAX_BYTECODE_OPERAND_ITEMS",
                1,
                {
                    "main": Code(
                        "main",
                        [],
                        [("MAKE_ARRAY", 2), ("RETURN", None)],
                    )
                },
                "item limit",
            ),
        )
        for constant, limit, functions, message in cases:
            with self.subTest(limit=constant):
                with patch(f"src.bootstrap.panackelty.{constant}", limit):
                    with self.assertRaisesRegex(PanackeltyError, message):
                        verify_bytecode(functions)

        functions = {
            "main": Code("main", [], [("CALL", ("helper", 0)), ("RETURN", None)], pure=True),
            "helper": Code("helper", [], VALID_RETURN, pure=False),
        }
        with self.assertRaisesRegex(PanackeltyError, "pure bytecode function"):
            verify_bytecode(functions)


if __name__ == "__main__":
    unittest.main()
