import tempfile
import unittest
from pathlib import Path

from panackelty import PanackeltyError, VM, bytecode_bytes, load_bytecode


VECTORS = Path(__file__).resolve().parents[2] / "fixtures" / "bytecode"


def vector_bytes(name):
    return bytes.fromhex((VECTORS / name).read_text(encoding="ascii"))


class PortableBytecodeVectorTests(unittest.TestCase):
    def load_vector(self, name):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / name.removesuffix(".hex")
            path.write_bytes(vector_bytes(name))
            return load_bytecode(path)

    def test_minimal_v8_vector_loads_runs_and_is_canonical(self):
        artifact = vector_bytes("minimal-v8.hex")
        functions = self.load_vector("minimal-v8.hex")

        self.assertEqual(bytecode_bytes(functions), artifact)
        result = VM(functions).run()
        self.assertEqual((result.type_name, result.data), ("Void", None))

    def test_legacy_vectors_are_identified_and_rejected(self):
        for version in (4, 5, 6, 7):
            with self.subTest(version=version):
                with self.assertRaisesRegex(
                    PanackeltyError,
                    f"unsupported bytecode version {version}",
                ):
                    self.load_vector(f"minimal-v{version}.hex")

    def test_malformed_vectors_are_rejected(self):
        cases = (
            ("bad-magic.hex", "not a Panackelty bytecode file"),
            ("unknown-opcode-v8.hex", "unknown bytecode opcode"),
            ("invalid-jump-v8.hex", "invalid jump target"),
            ("nonminimal-integer-v8.hex", "non-minimal integer"),
            ("truncated-v8.hex", "truncated data"),
            ("trailing-v8.hex", "trailing data"),
        )
        for name, message in cases:
            with self.subTest(vector=name):
                with self.assertRaisesRegex(PanackeltyError, message):
                    self.load_vector(name)


if __name__ == "__main__":
    unittest.main()
