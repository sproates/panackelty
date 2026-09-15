from unittest.mock import patch

from panackelty import PanackeltyError, build
from tests.unit.support import CompilerHarnessTestCase


class CompilerHarnessSupportTests(CompilerHarnessTestCase):
    def test_reuses_code_with_fresh_arguments_and_execution_after_failure(self):
        main = "main(): Void { print(command_args()[0][1]); }"
        with patch("tests.unit.support.build", wraps=build) as compile_probe:
            with self.assertRaises(PanackeltyError):
                self.run_harness("compiler/types.panack", main, ["a"])
            self.assertEqual(
                self.run_harness("compiler/types.panack", main, ["bc"]), "c\n"
            )
            self.assertEqual(
                self.run_harness("compiler/types.panack", main, ["d🙂"]), "🙂\n"
            )
            self.assertEqual(compile_probe.call_count, 1)
