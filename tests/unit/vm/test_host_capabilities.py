"""Python VM differential oracle for the shared host conformance program."""
import contextlib
import io
from pathlib import Path
import unittest
from panackelty import VM, build

PROJECT = Path(__file__).resolve().parents[3]

class HostCapabilityOracleTests(unittest.TestCase):
    def test_oracle_shared_conformance(self):
        case = PROJECT / 'tests/functional/cases/host_capabilities'
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(build(case / 'main.panack')).run()
        self.assertEqual(output.getvalue(), (case / 'expected.stdout').read_text())
