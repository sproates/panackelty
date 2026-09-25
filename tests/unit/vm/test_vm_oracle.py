"""Transitional Python VM oracle on the same artifacts as the native probes."""
import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from panackelty import VM, PanackeltyError, load_bytecode

ROOT = Path(__file__).resolve().parents[2] / "fixtures/vm_contracts"

class VMOracleTests(unittest.TestCase):
    def test_shared_vm_observations(self):
        cases = [c for c in json.loads((ROOT / "manifest.json").read_text())
                 if "python_stdout" in c]
        self.assertEqual(len(cases), 61)
        with tempfile.TemporaryDirectory() as directory:
            artifact = Path(directory) / "input.bc"
            for case in cases:
                with self.subTest(case=case["name"]):
                    artifact.write_bytes(bytes.fromhex((ROOT / (case["name"] + ".hex")).read_text()))
                    functions = load_bytecode(artifact)
                    output = io.StringIO()
                    with contextlib.redirect_stdout(output):
                        if case["python_error"] is not None:
                            with self.assertRaises(PanackeltyError) as caught:
                                VM(functions).run()
                            self.assertEqual(str(caught.exception), case["python_error"])
                        else:
                            value = VM(functions).run()
                            self.assertEqual(value.type_name, case["python_return"])
                            if value.type_name == "Void":
                                self.assertIsNone(value.data)
                    self.assertEqual(output.getvalue(), case["python_stdout"])
