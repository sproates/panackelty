"""Contracts made testable by compiling the native VM as independent modules."""
import os
from pathlib import Path
import shlex
import subprocess
import unittest

from panackelty import BUILTINS

PROJECT = Path(__file__).resolve().parents[3]
VM = PROJECT / "src/vm"


class NativeModuleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.executable = Path(os.environ.get("PANACK_NATIVE_MODULE_TEST", PROJECT / "build/vm/test_modules"))
        result = subprocess.run(
            ["make", "--no-print-directory", "native-module-build"], cwd=PROJECT,
            capture_output=True, text=True,
        )
        if result.returncode:
            raise AssertionError(result.stdout + result.stderr)

    def test_ownership_traps_unicode_and_decoder_mutations(self):
        result = subprocess.run(
            [str(self.executable)], capture_output=True, text=True, timeout=20,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(result.stdout, "native module contracts: ok\n")

    def test_registry_matches_oracle_signatures_and_has_handlers(self):
        names = sorted(BUILTINS)
        result = subprocess.run(
            [str(self.executable), "registry", *names],
            capture_output=True, text=True, timeout=10,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        expected = [
            f"{name} {len(BUILTINS[name][0])} {int(BUILTINS[name][2])}"
            for name in names
        ]
        self.assertEqual(result.stdout.splitlines(), expected)

    def test_headers_are_self_contained_and_repeatable(self):
        compiler = shlex.split(os.environ.get("CC", "cc"))
        for header in sorted(VM.glob("*.h")):
            with self.subTest(header=header.name):
                result = subprocess.run(
                    [*compiler, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
                     "-fsyntax-only", "-x", "c", "-I", str(VM), "-"],
                    input=f'#include "{header.name}"\n#include "{header.name}"\n'
                          'int main(void) { return 0; }\n',
                    capture_output=True, text=True, timeout=10,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
