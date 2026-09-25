import os
import subprocess
import shutil
import tempfile
import unittest
from pathlib import Path

from panackelty import build, bytecode_bytes


PROJECT = Path(__file__).resolve().parents[3]


class NativeLoaderTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory()
        cls.executable = Path(cls.temporary.name) / "panack-vm"
        override = os.environ.get("PANACK_NATIVE_BINARY")
        if not override:
            result = subprocess.run(
                ["make", "--no-print-directory", "native"], cwd=PROJECT,
                capture_output=True, text=True,
            )
            if result.returncode:
                raise AssertionError(result.stdout + result.stderr)
        shutil.copy2(Path(override) if override else PROJECT / "panack-vm", cls.executable)

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()


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


if __name__ == "__main__":
    unittest.main()
