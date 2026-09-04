import subprocess
import tempfile
import unittest
from pathlib import Path

from panackelty import build, bytecode_bytes
from tests.unit.bytecode.test_serialization import raw_artifact, raw_function, u16


PROJECT = Path(__file__).resolve().parents[3]
VM = PROJECT / "src/vm"
VECTORS = PROJECT / "tests/fixtures/bytecode"


class NativeLoaderTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory()
        cls.executable = Path(cls.temporary.name) / "panack-vm"
        result = subprocess.run(
            [
                "cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
                str(VM / "native.c"), str(VM / "bigint.c"),
                "-o", str(cls.executable),
            ],
            capture_output=True,
            text=True,
        )
        if result.returncode:
            raise AssertionError(result.stderr)

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def artifact(self, name, root):
        path = root / name.removesuffix(".hex")
        path.write_bytes(bytes.fromhex((VECTORS / name).read_text(encoding="ascii")))
        return path

    def test_accepts_minimal_version_seven_artifact(self):
        with tempfile.TemporaryDirectory() as directory:
            artifact = self.artifact("minimal-v7.hex", Path(directory))
            result = subprocess.run(
                [str(self.executable), "check", str(artifact)],
                capture_output=True,
                text=True,
            )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "ok\n")

    def test_accepts_complete_compiler_and_standard_library_artifacts(self):
        sources = (
            PROJECT / "src/compiler/main.panack",
            PROJECT / "tests/functional/cases/stdlib/main.panack",
        )
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for index, source in enumerate(sources):
                with self.subTest(source=source):
                    artifact = root / f"program-{index}.bc"
                    artifact.write_bytes(bytecode_bytes(build(source)))
                    result = subprocess.run(
                        [str(self.executable), "check", str(artifact)],
                        capture_output=True,
                        text=True,
                    )
                    self.assertEqual(result.returncode, 0, result.stderr)

    def test_rejects_shared_malformed_artifacts(self):
        cases = {
            "bad-magic.hex": "not a Panackelty bytecode file",
            "unknown-opcode-v7.hex": "unknown bytecode opcode",
            "invalid-jump-v7.hex": "invalid jump target",
            "nonminimal-integer-v7.hex": "non-minimal integer",
            "truncated-v7.hex": "truncated data",
            "trailing-v7.hex": "trailing data",
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, message in cases.items():
                with self.subTest(vector=name):
                    artifact = self.artifact(name, root)
                    result = subprocess.run(
                        [str(self.executable), "check", str(artifact)],
                        capture_output=True,
                        text=True,
                    )
                    self.assertEqual(result.returncode, 1)
                    self.assertIn(message, result.stderr)

    def test_rejects_forged_structural_and_semantic_violations(self):
        call_print = b"\x11" + u16(5) + b"print" + b"\x01"
        call_missing = b"\x11" + u16(7) + b"missing" + b"\x00"
        cases = (
            (raw_artifact([raw_function(flags=2)]), "invalid function flags"),
            (raw_artifact([raw_function(name=b"\xff")]), "invalid UTF-8"),
            (
                raw_artifact([
                    raw_function(name=b"helper", params=(b"x", b"x")),
                    raw_function(),
                ]),
                "invalid function signature",
            ),
            (raw_artifact([raw_function(instructions=())]), "function is empty"),
            (
                raw_artifact([raw_function(instructions=(b"\x09\x00\x00", b"\x14"))]),
                "invalid INTERPOLATE",
            ),
            (
                raw_artifact([raw_function(instructions=(call_missing, b"\x14"))]),
                "unknown function",
            ),
            (
                raw_artifact([raw_function(instructions=(call_print, b"\x14"), flags=1)]),
                "pure function calls impure",
            ),
            (
                raw_artifact([raw_function(instructions=(b"\x00\x01\x01\x00\x00", b"\x14"))]),
                "negative zero integer",
            ),
            (
                raw_artifact([raw_function(instructions=(b"\x00\x02\x00\x00\x00\x00\x02\x01", b"\x14"))]),
                "non-minimal decimal coefficient",
            ),
        )
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for index, (data, message) in enumerate(cases):
                with self.subTest(message=message):
                    artifact = root / f"forged-{index}.bc"
                    artifact.write_bytes(data)
                    result = subprocess.run(
                        [str(self.executable), "check", str(artifact)],
                        capture_output=True,
                        text=True,
                    )
                    self.assertEqual(result.returncode, 1)
                    self.assertIn(message, result.stderr)

    def test_rejects_every_truncation_of_a_valid_artifact(self):
        data = raw_artifact([raw_function()])
        with tempfile.TemporaryDirectory() as directory:
            artifact = Path(directory) / "truncated.bc"
            for end in range(9, len(data)):
                with self.subTest(end=end):
                    artifact.write_bytes(data[:end])
                    result = subprocess.run(
                        [str(self.executable), "check", str(artifact)],
                        capture_output=True,
                        text=True,
                    )
                    self.assertEqual(result.returncode, 1)
                    self.assertIn("truncated data", result.stderr)


if __name__ == "__main__":
    unittest.main()
