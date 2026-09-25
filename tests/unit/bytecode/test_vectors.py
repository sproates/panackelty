import tempfile
import unittest
from pathlib import Path

from panackelty import VM, bytecode_bytes, load_bytecode


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

    # Wire rejection is now checked against the same vectors by both native
    # tests/runner/bytecode_*_unit.panack suites. Runtime return remains a VM contract.


if __name__ == "__main__":
    unittest.main()
