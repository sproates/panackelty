import os
import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT = Path(__file__).resolve().parents[2]
TIMER = PROJECT / "tests/run_timed.sh"


class ValidationTimingTests(unittest.TestCase):
    def run_timer(
        self,
        budget: str,
        command: str,
        *,
        environment: dict[str, str] | None = None,
    ) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            ["sh", str(TIMER), "sample", budget, "sh", "-c", command],
            capture_output=True,
            text=True,
            env=environment,
        )

    def test_reports_phase_timing_and_budget_warning(self):
        result = self.run_timer("-1", "exit 0")

        self.assertEqual(result.returncode, 0)
        self.assertIn("timing: sample ", result.stdout)
        self.assertIn("(budget -1s)", result.stdout)
        self.assertIn("warning: sample exceeded its -1s validation budget", result.stderr)

    def test_preserves_command_failure_status(self):
        result = self.run_timer("15", "exit 7")

        self.assertEqual(result.returncode, 7)
        self.assertIn("timing: sample ", result.stdout)

    def test_appends_machine_readable_timing_record(self):
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "timings.tsv"
            environment = dict(os.environ)
            environment["VALIDATION_TIMINGS_FILE"] = str(report)

            result = self.run_timer("15", "exit 0", environment=environment)

            self.assertEqual(result.returncode, 0)
            label, elapsed, budget, status = report.read_text(
                encoding="utf-8"
            ).strip().split("\t")
            self.assertEqual(label, "sample")
            self.assertGreaterEqual(int(elapsed), 0)
            self.assertEqual(budget, "15")
            self.assertEqual(status, "0")


if __name__ == "__main__":
    unittest.main()
