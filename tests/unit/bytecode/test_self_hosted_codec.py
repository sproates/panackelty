import decimal
import json
import tempfile
import unittest
from pathlib import Path

from panackelty import Code, VM, build, bytecode_bytes
from tests.unit.bytecode.test_serialization import raw_artifact, raw_function, u16
from tests.unit.compiler.test_self_hosted_emitter import render_bootstrap


PROJECT = Path(__file__).resolve().parents[3]
COMPILER = PROJECT / "src/compiler"
BYTECODE = PROJECT / "src/bytecode"


class SelfHostedBytecodeCodecTests(unittest.TestCase):
    def run_decoder_tool(self, artifact, function, *, binary=False):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            compiler = root / "compiler"
            bytecode = root / "bytecode"
            compiler.mkdir()
            bytecode.mkdir()
            for path in COMPILER.glob("*.panack"):
                (compiler / path.name).write_text(
                    path.read_text(encoding="utf-8"), encoding="utf-8"
                )
            for path in BYTECODE.glob("*.panack"):
                (bytecode / path.name).write_text(
                    path.read_text(encoding="utf-8"), encoding="utf-8"
                )
            source = root / "input.bc"
            destination = root / ("output.bc" if binary else "output.txt")
            source.write_bytes(artifact)
            writer = "write_bytes" if binary else "write_file"
            main = bytecode / "test_main.panack"
            main.write_text(
                'import "decoder.panack";\n'
                f"main(): Void {{ {writer}({json.dumps(str(destination))}, "
                f"{function}(read_bytes({json.dumps(str(source))}))); }}",
                encoding="utf-8",
            )
            VM(build(main)).run()
            return destination.read_bytes() if binary else destination.read_text(encoding="utf-8")

    def serialize(self, source):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            compiler = root / "compiler"
            bytecode = root / "bytecode"
            compiler.mkdir()
            bytecode.mkdir()
            for path in COMPILER.glob("*.panack"):
                (compiler / path.name).write_text(
                    path.read_text(encoding="utf-8"), encoding="utf-8"
                )
            for path in BYTECODE.glob("*.panack"):
                (bytecode / path.name).write_text(
                    path.read_text(encoding="utf-8"), encoding="utf-8"
                )
            destination = root / "actual.bc"
            source_expression = ' + "$" + '.join(
                json.dumps(part) for part in source.split("$")
            )
            main = bytecode / "test_main.panack"
            main.write_text(
                'import "decoder.panack";\n'
                f"main(): Void {{ write_bytes({json.dumps(str(destination))}, "
                f"compile_source_bytecode({source_expression})); }}",
                encoding="utf-8",
            )
            VM(build(main)).run()
            return destination.read_bytes()

    def expected(self, source):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "main.panack"
            path.write_text(source, encoding="utf-8")
            return bytecode_bytes(build(path))

    def assert_differential(self, source):
        self.assertEqual(self.serialize(source), self.expected(source))

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
            ("unknown-opcode-v7.hex", "unknown bytecode opcode"),
            ("invalid-jump-v7.hex", "invalid jump target"),
            ("nonminimal-integer-v7.hex", "non-minimal integer"),
            ("truncated-v7.hex", "truncated data"),
            ("trailing-v7.hex", "trailing data"),
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
