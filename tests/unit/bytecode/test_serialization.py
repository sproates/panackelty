"""Stage-0 limits, verification hook and VM/CLI safeguards until bootstrap retirement."""

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
    bytecode_bytes,
    load_bytecode,
    write_bytecode,
)
from tests.unit.support import PanackeltyTestCase


PROJECT = Path(__file__).resolve().parents[3]


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


if __name__ == "__main__":
    unittest.main()
