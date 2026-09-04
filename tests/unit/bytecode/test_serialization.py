import decimal
import os
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from panackelty import (
    BYTECODE_MAGIC,
    BYTECODE_VERSION,
    Code,
    PanackeltyError,
    VM,
    bytecode_bytes,
    load_bytecode,
    write_bytecode,
)
from tests.unit.support import PanackeltyTestCase


PROJECT = Path(__file__).resolve().parents[3]
HEADER_SIZE = len(BYTECODE_MAGIC) + 2


def u16(value):
    return value.to_bytes(2, "big")


def u32(value):
    return value.to_bytes(4, "big")


def raw_function(name=b"main", *, flags=0, params=(), instructions=(b"\x00\x05", b"\x14")):
    encoded_params = b"".join(u16(len(param)) + param for param in params)
    encoded_code = b"".join(instructions)
    return (
        u16(len(name)) + name + bytes((flags, len(params))) + encoded_params
        + u32(len(instructions)) + encoded_code
    )


def raw_artifact(functions, *, version=BYTECODE_VERSION, trailing=b""):
    payload = u16(len(functions)) + b"".join(functions) + trailing
    return BYTECODE_MAGIC + u16(version) + payload


class SerializationTests(PanackeltyTestCase):
    def load_artifact(self, data):
        with tempfile.TemporaryDirectory() as directory:
            bytecode = Path(directory) / "test.bc"
            bytecode.write_bytes(data)
            return load_bytecode(bytecode)

    def test_rejects_invalid_magic_and_version(self):
        with tempfile.TemporaryDirectory() as directory:
            bytecode = Path(directory) / "broken.bc"
            bytecode.write_bytes(b"not-panackelty-bytecode")
            with self.assertRaisesRegex(PanackeltyError, "not a Panackelty bytecode file"):
                load_bytecode(bytecode)

        with self.assertRaisesRegex(PanackeltyError, "unsupported bytecode version"):
            self.load_artifact(raw_artifact([], version=BYTECODE_VERSION + 1))

    def test_rejects_artifacts_over_the_size_limit_before_decoding(self):
        with tempfile.TemporaryDirectory() as directory:
            bytecode = Path(directory) / "oversized.bc"
            bytecode.write_bytes(b"x" * 32)
            with patch("src.bootstrap.panackelty.MAX_BYTECODE_BYTES", 16):
                with self.assertRaisesRegex(PanackeltyError, "size limit"):
                    load_bytecode(bytecode)

        functions = {"main": Code("main", [], [("CONST", ("Void", None)), ("RETURN", None)])}
        with patch("src.bootstrap.panackelty.MAX_BYTECODE_BYTES", 16):
            with self.assertRaisesRegex(PanackeltyError, "size limit"):
                bytecode_bytes(functions)

    def test_decoder_checks_declared_resource_limits(self):
        main = raw_function()
        helper = raw_function(name=b"helper")
        cases = (
            (
                "MAX_BYTECODE_FUNCTIONS",
                1,
                raw_artifact([main, helper]),
                "function count exceeds limit",
            ),
            (
                "MAX_BYTECODE_PARAMETERS",
                0,
                raw_artifact([raw_function(params=(b"value",))]),
                "parameter limit",
            ),
            (
                "MAX_BYTECODE_INSTRUCTIONS_PER_FUNCTION",
                1,
                raw_artifact([main]),
                "instruction limit",
            ),
            (
                "MAX_BYTECODE_TOTAL_INSTRUCTIONS",
                3,
                raw_artifact([main, raw_function(name=b"zebra")]),
                "total instruction count exceeds limit",
            ),
            (
                "MAX_BYTECODE_NAME_BYTES",
                3,
                raw_artifact([main]),
                "name exceeds limit",
            ),
            (
                "MAX_BYTECODE_TEXT_BYTES",
                3,
                raw_artifact(
                    [raw_function(instructions=(b"\x00\x03\x00\x00\x00\x04four", b"\x14"))]
                ),
                "text exceeds limit",
            ),
            (
                "MAX_BYTECODE_NUMERIC_DIGITS",
                2,
                raw_artifact([raw_function(instructions=(b"\x00\x00\x00\x01\x64", b"\x14"))]),
                "integer exceeds digit limit",
            ),
            (
                "MAX_BYTECODE_OPERAND_ITEMS",
                1,
                raw_artifact(
                    [raw_function(instructions=(b"\x09\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00", b"\x14"))]
                ),
                "interpolation exceeds item limit",
            ),
        )
        for constant, limit, artifact, message in cases:
            with self.subTest(limit=constant):
                with patch(f"src.bootstrap.panackelty.{constant}", limit):
                    with self.assertRaisesRegex(PanackeltyError, message):
                        self.load_artifact(artifact)

    def test_rejects_every_truncation_and_trailing_data(self):
        artifact = raw_artifact([raw_function()])
        for end in range(HEADER_SIZE, len(artifact)):
            with self.subTest(end=end):
                with self.assertRaisesRegex(PanackeltyError, "truncated data"):
                    self.load_artifact(artifact[:end])
        with self.assertRaisesRegex(PanackeltyError, "trailing data"):
            self.load_artifact(artifact + b"\x00")

    def test_rejects_malformed_function_records_and_opcodes(self):
        cases = (
            (raw_artifact([]), "no main function"),
            (raw_artifact([raw_function(name=b"\xff")]), "invalid UTF-8"),
            (raw_artifact([raw_function(flags=2)]), "unknown function flags"),
            (
                raw_artifact([raw_function(), raw_function()]),
                "duplicate function main",
            ),
            (
                raw_artifact([raw_function(name=b"zebra"), raw_function()]),
                "not canonically ordered",
            ),
            (
                raw_artifact([raw_function(instructions=(b"\xff",))]),
                "unknown bytecode opcode",
            ),
        )
        for artifact, message in cases:
            with self.subTest(message=message):
                with self.assertRaisesRegex(PanackeltyError, message):
                    self.load_artifact(artifact)

    def test_rejects_noncanonical_and_invalid_constants(self):
        cases = (
            (b"\x00\x00\x00\x01\x00", "non-minimal integer"),
            (b"\x00\x01\x01\x00\x00", "negative zero integer"),
            (b"\x00\x02\x00\x00\x00\x00\x01\x10", "invalid decimal padding"),
            (
                b"\x00\x02\x00\x00\x00\x00\x02\x01",
                "non-minimal decimal coefficient",
            ),
            (b"\x00\x04\x02", "invalid Bool constant"),
            (b"\x00\xff", "unknown constant tag"),
        )
        for instruction, message in cases:
            artifact = raw_artifact(
                [raw_function(instructions=(instruction, b"\x14"))]
            )
            with self.subTest(message=message):
                with self.assertRaisesRegex(PanackeltyError, message):
                    self.load_artifact(artifact)

    def test_all_scalar_constants_round_trip_canonically(self):
        instructions = [
            ("CONST", ("Nat", 0)),
            ("POP", None),
            ("CONST", ("Nat", 10 ** 100)),
            ("POP", None),
            ("CONST", ("Int", -123456789)),
            ("POP", None),
            ("CONST", ("Dec", decimal.Decimal("123.4500"))),
            ("POP", None),
            ("CONST", ("Str", "héllo 🌍")),
            ("POP", None),
            ("CONST", ("Bool", True)),
            ("POP", None),
            ("CONST", ("Void", None)),
            ("RETURN", None),
        ]
        functions = {"main": Code("main", [], instructions)}
        artifact = bytecode_bytes(functions)
        decoded = self.load_artifact(artifact)

        self.assertEqual(decoded["main"].instructions, instructions)
        self.assertEqual(bytecode_bytes(decoded), artifact)

    def test_every_instruction_operand_round_trips(self):
        instructions = [
            ("CONST", ("Nat", 1)),
            ("LOAD", "value"),
            ("STORE", "value"),
            ("POP", None),
            ("UNARY", "-"),
            ("BINARY", "+"),
            ("MAKE_RANGE", None),
            ("MAKE_ARRAY", 2),
            ("INDEX_GET", None),
            ("INTERPOLATE", ("before", "after")),
            ("ITER_INIT", "iterator"),
            ("ITER_NEXT", ("iterator", "item", 22)),
            ("MAKE_RECORD", ("Pair", ("left", "right"))),
            ("FIELD_GET", "left"),
            ("MAKE_VARIANT", ("Option", "Some", 1)),
            ("MATCH_VARIANT", ("Some", 22)),
            ("MATCH_FAIL", None),
            ("CALL", ("helper", 0)),
            ("CONST", ("Str", "helper")),
            ("CALL_VALUE", 0),
            ("JUMP_FALSE", 22),
            ("JUMP", 22),
            ("RETURN", None),
        ]
        functions = {
            "main": Code("main", [], instructions),
            "helper": Code(
                "helper",
                [],
                [("CONST", ("Void", None)), ("RETURN", None)],
                pure=True,
            ),
        }

        decoded = self.load_artifact(bytecode_bytes(functions))

        self.assertEqual(decoded["main"].instructions, instructions)

    def test_source_build_verifies_emitted_bytecode(self):
        with patch("src.bootstrap.panackelty.verify_bytecode") as verifier:
            functions = self.compile("main(): Void {}")

        verifier.assert_called_once_with(functions)

    def test_adt_bytecode_round_trip(self):
        code = self.compile("""
record Pair { left: Nat, right: Nat }
enum Answer { Missing, Found(Pair) }
pure total(answer: Answer): Nat {
  match answer { Missing() => 0, Found(pair) => pair.left + pair.right }
}
main(): Void { print(total(Found(Pair(19, 23)))); }
""")
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "adt.bc"
            write_bytecode(code, path)
            self.assertEqual(self.run_code(load_bytecode(path)), "42\n")

    def test_serialization_uses_canonical_function_order(self):
        main = Code(
            "main",
            [],
            [("CALL", ("zebra", 0)), ("RETURN", None)],
        )
        zebra = Code(
            "zebra",
            [],
            [("CONST", ("Void", None)), ("RETURN", None)],
            pure=True,
        )
        forward = {"main": main, "zebra": zebra}
        reverse = {"zebra": zebra, "main": main}

        self.assertEqual(bytecode_bytes(forward), bytecode_bytes(reverse))
        self.assertEqual(list(self.load_artifact(bytecode_bytes(reverse))), ["main", "zebra"])

    def test_repeated_compilation_is_byte_identical(self):
        source = """
pure answer(): Nat { 6 * 7 }
main(): Void { print(answer()); }
"""
        self.assertEqual(
            bytecode_bytes(self.compile(source)),
            bytecode_bytes(self.compile(source)),
        )

    def test_cli_compilation_is_identical_across_hash_seeds(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "program.panack"
            source.write_text(
                "pure zebra(): Nat { 2 } "
                "pure alpha(): Nat { 40 } "
                "main(): Void { print(alpha() + zebra()); }",
                encoding="utf-8",
            )
            artifacts = [root / "first.bc", root / "second.bc"]
            for seed, artifact in zip(("1", "987654"), artifacts):
                environment = os.environ.copy()
                environment["PYTHONHASHSEED"] = seed
                subprocess.run(
                    [
                        str(PROJECT / "panack"),
                        "compile",
                        str(source),
                        "-o",
                        str(artifact),
                    ],
                    check=True,
                    capture_output=True,
                    text=True,
                    env=environment,
                )
            self.assertEqual(artifacts[0].read_bytes(), artifacts[1].read_bytes())

    def test_load_and_reserialize_is_byte_identical(self):
        code = self.compile(
            'pure greeting(name: Str): Str { "Hello, ${name}" } '
            'main(): Void { print(greeting("Panackelty")); }'
        )
        original = bytecode_bytes(code)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "round-trip.bc"
            path.write_bytes(original)
            self.assertEqual(bytecode_bytes(load_bytecode(path)), original)


if __name__ == "__main__":
    unittest.main()
