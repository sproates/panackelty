import tempfile
import json
import unittest
from pathlib import Path

from panackelty import build, bytecode_bytes
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

    def test_shared_source_artifacts_match_bootstrap(self):
        fixtures = PROJECT / "tests/fixtures/bytecode/codec_contracts"
        paths = sorted(fixtures.glob("*.panack"))
        self.assertEqual(len(paths), 10)
        for path in paths:
            with self.subTest(case=path.stem):
                source = path.read_text(encoding="utf-8")
                golden = fixtures / ("order.hex" if path.stem == "order_reversed" else path.stem + ".hex")
                expected = bytes.fromhex(golden.read_text())
                actual = self.serialize(source)
                self.assertEqual(actual, self.expected(source))
                self.assertEqual(actual, expected)

    def test_shared_artifacts_round_trip_and_disassemble(self):
        root = PROJECT / "tests/fixtures/bytecode"
        paths = sorted((root / "valid_contracts").glob("*.hex"))
        self.assertEqual(len(paths), 5)
        for path in paths:
            with self.subTest(case=path.stem):
                artifact = bytes.fromhex(path.read_text())
                self.assertEqual(self.run_decoder_tool(artifact, "round_trip_bytecode", binary=True), artifact)
                self.assertEqual(self.run_decoder_tool(artifact, "disassemble_bytecode"),
                                 path.with_suffix(".disasm").read_text())
        source = (root / "codec_contracts/disassembly.panack").read_text()
        artifact = bytes.fromhex((root / "codec_contracts/disassembly.hex").read_text())
        self.assertEqual(self.run_decoder_tool(artifact, "disassemble_bytecode"), render_bootstrap(source))

    def test_shared_invalid_artifacts_preserve_self_hosted_diagnostics(self):
        root = PROJECT / "tests/fixtures/bytecode"
        cases = [(root / "contract_cases" / (case["name"] + ".hex"), case["expected"])
                 for manifest in ("manifest.json", "codec_manifest.json")
                 for case in json.loads((root / "contract_cases" / manifest).read_text())]
        cases += [(root / f"minimal-v{version}.hex", f"unsupported bytecode version {version}")
                  for version in (4, 5, 6, 7)]
        cases += [(root / (name + ".hex"), message) for name, message in (
            ("bad-magic", "not a Panackelty bytecode file"),
            ("unknown-opcode-v8", "unknown bytecode opcode"),
            ("invalid-jump-v8", "invalid jump target"),
            ("nonminimal-integer-v8", "non-minimal integer"),
            ("truncated-v8", "truncated data"),
            ("trailing-v8", "trailing data"))]
        cases += [(root / "contract_cases" / (name + ".hex"), message)
                  for name, message in (
                      ("limit-function-count", "function count exceeds limit"),
                      ("limit-name-bytes", "name exceeds limit"),
                      ("limit-text-bytes", "text exceeds limit"),
                      ("limit-integer-digits", "integer exceeds digit limit"),
                      ("limit-instructions", "function exceeds instruction limit"))]
        self.assertEqual(len(cases), 57)
        for path, expected in cases:
            with self.subTest(case=path.stem):
                actual = self.run_decoder_tool(bytes.fromhex(path.read_text()), "validate_bytecode")
                self.assertIn(expected, actual)


if __name__ == "__main__":
    unittest.main()
