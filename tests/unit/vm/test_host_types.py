"""Python VM differential oracle for the shared host type program."""
import contextlib
import io
from pathlib import Path
import unittest
from panackelty import VM, build

class HostTypeOracleTests(unittest.TestCase):
    def test_oracle_matches_shared_host_type_conformance(self):
        case = Path(__file__).resolve().parents[2] / 'functional/cases/host_types'
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            VM(build(case / 'main.panack')).run()
        self.assertEqual(output.getvalue(), (case / 'expected.stdout').read_text())
