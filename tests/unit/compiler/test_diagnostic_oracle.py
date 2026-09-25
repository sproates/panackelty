"""Two intentional diagnostic wording differences between compiler implementations."""
from pathlib import Path
from panackelty import PanackeltyError, build
from tests.unit.support import CompilerHarnessTestCase

FIXTURES = Path(__file__).resolve().parents[2] / "fixtures/compiler_contracts"


class DiagnosticOracleTests(CompilerHarnessTestCase):
    def test_impure_callable_wording(self):
        source = FIXTURES / "syntax/callable_diagnostics_preserve_types_and_purity-1.panack"
        with self.assertRaisesRegex(PanackeltyError, "pure function cannot invoke an impure callable"):
            build(source)
        actual = self.run_harness("compiler/purity.panack",
            "main(): Void { print(check_source_frontend(command_args()[0])) }", [source.read_text()])
        self.assertIn("pure function cannot call impure function call", actual)

    def test_logical_extension_wording(self):
        source = FIXTURES / "imports/invalid_logical_imports_are_rejected-2/invalid-1.panack"
        with self.assertRaisesRegex(PanackeltyError, "logical import extension must be .panack"):
            build(source)
        actual = self.run_harness("compiler/driver.panack",
            "main(): Void { loaded = load_project(command_args()[0]); print(diagnostic_messages(loaded.diagnostics, loaded.sources)) }",
            [str(source)])
        self.assertIn("invalid logical import path", actual)
