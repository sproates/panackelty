import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT = Path(__file__).resolve().parents[3]
SEED = PROJECT / "bootstrap/compiler-v7.bc"


class ReproducibleBootstrapTests(unittest.TestCase):
    def test_corrupt_seed_is_rejected_before_bootstrap(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            corrupt = root / "compiler.bc"
            shutil.copyfile(SEED, corrupt)
            data = bytearray(corrupt.read_bytes())
            data[0] ^= 0xFF
            corrupt.write_bytes(data)
            result = subprocess.run(
                [
                    "make",
                    "bootstrap",
                    f"BUILD_DIR={root / 'build'}",
                    f"SEED_COMPILER={corrupt}",
                ],
                cwd=PROJECT,
                capture_output=True,
                text=True,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("not a Panackelty bytecode file", result.stderr)


if __name__ == "__main__":
    unittest.main()
