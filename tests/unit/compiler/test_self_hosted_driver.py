"""Retained byte-identical Python bootstrap vs self-hosted driver oracle."""
import tempfile
from pathlib import Path
from panackelty import build, bytecode_bytes
from tests.unit.support import CompilerHarnessTestCase

PROJECT = Path(__file__).resolve().parents[3]
FIXTURES = PROJECT / "tests/fixtures/compiler_contracts/driver"


class SelfHostedDriverTests(CompilerHarnessTestCase):
    def test_shared_module_graph_artifacts_match_bootstrap(self):
        for entry in ("basic.panack", "relative/main.panack", "logical/main.panack"):
            with self.subTest(entry=entry), tempfile.TemporaryDirectory() as directory:
                source = FIXTURES / entry
                output = Path(directory) / "program.bc"
                self.run_harness(
                    "compiler/driver.panack",
                    "main(): Void { run_compiler_command(command_args()); }",
                    ["compile", str(source), "-o", str(output)],
                    environment={"PANACKELTY_STDLIB_PATH": str(PROJECT / "src/stdlib")},
                )
                self.assertEqual(output.read_bytes(), bytecode_bytes(build(source)))
