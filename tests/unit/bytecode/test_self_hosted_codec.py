import decimal
import tempfile
import unittest
from pathlib import Path

from panackelty import Code, build, bytecode_bytes
from tests.unit.bytecode.test_serialization import raw_artifact, raw_function, u16
from tests.unit.compiler.test_self_hosted_emitter import render_bootstrap
from tests.unit.support import CompilerHarnessTestCase


PROJECT = Path(__file__).resolve().parents[3]


class SelfHostedBytecodeCodecTests(CompilerHarnessTestCase):
    def run_decoder_tool(self, artifact, function, *, binary=False):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "input.bc"
            destination = Path(directory) / "output"
            source.write_bytes(artifact)
            writer = "write_bytes" if binary else "write_file"
            self.run_harness(
                "bytecode/decoder.panack",
                f"main(): Void {{ {writer}(command_args()[1], "
                f"{function}(read_bytes(command_args()[0]))); }}",
                [str(source), str(destination)],
            )
            return destination.read_bytes() if binary else destination.read_text(encoding="utf-8")

    def serialize(self, source):
        with tempfile.TemporaryDirectory() as directory:
            destination = Path(directory) / "actual.bc"
            self.run_harness(
                "bytecode/decoder.panack",
                "main(): Void { write_bytes(command_args()[1], "
                "compile_source_bytecode(command_args()[0])); }",
                [source, str(destination)],
            )
            return destination.read_bytes()

    def expected(self, source):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "main.panack"
            path.write_text(source, encoding="utf-8")
            return bytecode_bytes(build(path))

    def assert_differential(self, source):
        self.assertEqual(self.serialize(source), self.expected(source))

    def test_rational_and_unit_bytecode_round_trip(self):
        source = "main(): Void { third = 1/3; print((third * 30).nat()); print((1/8).dec()); print(()) }"
        artifact = self.serialize(source)
        self.assertEqual(artifact, self.expected(source))
        self.assertEqual(self.serialize(source), artifact)
        self.assertEqual(self.run_decoder_tool(artifact, "round_trip_bytecode", binary=True), artifact)

    def test_serializes_minimal_program_exactly(self):
        self.assert_differential("main(): Void {}")

    def test_serializes_complete_instruction_and_constant_mix(self):
        source = r'''
record Pair { left: Nat, right: Nat }
enum Maybe { None, Some(Nat) }
pure calculate(limit: Nat): Dec {
  mut total: Nat = 0;
  for value in 0..limit { total = total + value; }
  if total > 0 && limit > 0 { 12.3400 } else { 0.0 }
}
pure unwrap(value: Maybe): Nat {
  match value { Some(item) => item, None() => 0 }
}
main(): Void {
  pair: Pair = Pair(20, 22);
  answer: Nat = unwrap(Some(pair.left + pair.right));
  print("answer ${answer}");
  print(calculate(answer));
}
'''
        self.assert_differential(source)

    def test_deserializes_and_reserializes_byte_identically(self):
        source = r'''
pure values(): Dec { 123.4500 }
main(): Void { print(values()); }
'''
        artifact = self.expected(source)
        self.assertEqual(
            self.run_decoder_tool(artifact, "round_trip_bytecode", binary=True),
            artifact,
        )

        scalar_artifact = bytecode_bytes(
            {
                "main": Code(
                    "main",
                    [],
                    [
                        ("CONST", ("Int", -123456789)),
                        ("POP", None),
                        ("CONST", ("Dec", decimal.Decimal("-0E+2"))),
                        ("POP", None),
                        ("CONST", ("Void", None)),
                        ("RETURN", None),
                    ],
                )
            }
        )
        self.assertEqual(
            self.run_decoder_tool(
                scalar_artifact, "round_trip_bytecode", binary=True
            ),
            scalar_artifact,
        )

    def test_disassembles_loaded_artifact_to_the_emitter_ir(self):
        source = "pure answer(): Nat { 42 } main(): Void { print(answer()); }"
        artifact = self.expected(source)
        self.assertEqual(
            self.run_decoder_tool(artifact, "disassemble_bytecode"),
            render_bootstrap(source),
        )

    def test_rejects_portable_malformed_vectors(self):
        vectors = PROJECT / "tests/fixtures/bytecode"
        cases = (
            ("bad-magic.hex", "not a Panackelty bytecode file"),
            ("minimal-v4.hex", "unsupported bytecode version 4"),
            ("minimal-v5.hex", "unsupported bytecode version 5"),
            ("minimal-v6.hex", "unsupported bytecode version 6"),
            ("unknown-opcode-v8.hex", "unknown bytecode opcode"),
            ("invalid-jump-v8.hex", "invalid jump target"),
            ("nonminimal-integer-v8.hex", "non-minimal integer"),
            ("truncated-v8.hex", "truncated data"),
            ("trailing-v8.hex", "trailing data"),
        )
        for name, expected in cases:
            with self.subTest(vector=name):
                artifact = bytes.fromhex((vectors / name).read_text(encoding="ascii"))
                self.assertIn(
                    expected,
                    self.run_decoder_tool(artifact, "validate_bytecode"),
                )

    def test_rejects_structural_call_and_purity_violations(self):
        def call(name, arity):
            encoded = name.encode("utf-8")
            return b"\x11" + u16(len(encoded)) + encoded + bytes((arity,))

        cases = (
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
                raw_artifact(
                    [raw_function(instructions=(b"\x00\x04\x02", b"\x14"))]
                ),
                "invalid Bool constant",
            ),
            (
                raw_artifact(
                    [
                        raw_function(
                            instructions=(
                                b"\x00\x02\x00\x00\x00\x00\x01\x10",
                                b"\x14",
                            )
                        )
                    ]
                ),
                "invalid decimal padding",
            ),
            (
                raw_artifact(
                    [raw_function(instructions=(call("missing", 0), b"\x14"))]
                ),
                "calls unknown function missing",
            ),
            (
                raw_artifact(
                    [raw_function(instructions=(call("print", 0), b"\x14"))]
                ),
                "invalid arity",
            ),
            (
                raw_artifact(
                    [
                        raw_function(
                            flags=1,
                            instructions=(call("print", 1), b"\x14"),
                        )
                    ]
                ),
                "pure bytecode function calls impure function print",
            ),
        )
        for artifact, expected in cases:
            with self.subTest(expected=expected):
                self.assertIn(
                    expected,
                    self.run_decoder_tool(artifact, "validate_bytecode"),
                )


if __name__ == "__main__":
    unittest.main()
