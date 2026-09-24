import hashlib
import os
import shutil
import shlex
import subprocess
import tarfile
import tempfile
import unittest
from pathlib import Path


PROJECT = Path(__file__).resolve().parents[2]
RELEASE_VERSION = (PROJECT / "VERSION").read_text(encoding="utf-8").strip()
EXAMPLE_FILES = {
    "examples/README.md",
} | {
    f"examples/{path.name}"
    for path in (PROJECT / "examples").glob("*.panack")
}
INSTALLED_FILES = {
    "bin/panack",
    "libexec/panackelty/panack-vm",
    "share/doc/panackelty/CHANGELOG.md",
    "share/doc/panackelty/LICENSE",
    "share/doc/panackelty/RELEASE_POLICY.md",
    "share/doc/panackelty/SECURITY.md",
    "share/doc/panackelty/SPEC.md",
    "share/panackelty/VERSION",
    "share/panackelty/compiler-v8.bc",
    "share/panackelty/stdlib/bytes.panack",
    "share/panackelty/stdlib/collections.panack",
    "share/panackelty/stdlib/environment.panack",
    "share/panackelty/stdlib/option.panack",
    "share/panackelty/stdlib/path.panack",
    "share/panackelty/stdlib/prelude.panack",
    "share/panackelty/stdlib/result.panack",
    "share/panackelty/stdlib/text.panack",
    "share/panackelty/stdlib/testing.panack",
    "share/panackelty/stdlib/testing_commands.panack",
    "share/panackelty/stdlib/testing_files.panack",
    "share/panackelty/stdlib/time.panack",
    "share/panackelty/stdlib/host.panack",
    "share/panackelty/stdlib/filesystem.panack",
    "share/panackelty/stdlib/process.panack",
}


class NativeDistributionTests(unittest.TestCase):
    def test_native_build_flags_are_configurable(self):
        # Inspect the actual recipe selected by make without building another VM.
        environment = os.environ.copy()
        for name in ("MAKEFLAGS", "MFLAGS", "MAKELEVEL", "CFLAGS", "CPPFLAGS", "LDFLAGS", "LDLIBS"):
            environment.pop(name, None)
        for overrides, expected in (
            ([], ["-O2"]),
            (["CFLAGS=-O0 -g", "CPPFLAGS=-DPANACK_BUILD_TEST=1",
              "LDFLAGS=-L/tmp", "LDLIBS=-lm"],
             ["-O0", "-g", "-DPANACK_BUILD_TEST=1", "-L/tmp", "-lm"]),
        ):
            with self.subTest(overrides=overrides):
                result = subprocess.run(
                    ["make", "--always-make", "--dry-run", "native", *overrides],
                    cwd=PROJECT, env=environment, capture_output=True, text=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
                command = shlex.split(result.stdout.strip())
                for flag in expected + ["-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic"]:
                    self.assertIn(flag, command)
                if overrides:
                    self.assertNotIn("-O2", command)

    def test_release_checksum_matches_archive(self):
        with tempfile.TemporaryDirectory() as directory:
            checkout = Path(directory) / "checkout with spaces (test)"
            checkout.mkdir()
            for name in (
                "Makefile", "VERSION", "panack", "panack-vm", "bootstrap",
                "src", "tests", "examples", "README.md", "LICENSE",
                "CHANGELOG.md", "RELEASE_POLICY.md", "SECURITY.md", "SPEC.md",
            ):
                (checkout / name).symlink_to(PROJECT / name)
            build = checkout / "build"
            result = subprocess.run(
                [
                    "make",
                    "quick-start",
                    "PYTHON=false",
                ],
                cwd=checkout,
                capture_output=True,
                text=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)

            archives = list(build.glob("panackelty-*.tar.gz"))
            self.assertEqual(len(archives), 1, archives)
            archive = archives[0]
            checksum = Path(f"{archive}.sha256")
            self.assertTrue(checksum.is_file())
            digest, name = checksum.read_text(encoding="utf-8").split()
            self.assertEqual(name, archive.name)
            self.assertEqual(
                digest,
                hashlib.sha256(archive.read_bytes()).hexdigest(),
            )

    def test_functional_recipe_passes_compiler_path_from_spaced_checkout(self):
        with tempfile.TemporaryDirectory() as directory:
            checkout = Path(directory) / "checkout with spaces (test)"
            checkout.mkdir()
            for name in ("Makefile", "VERSION", "bootstrap", "panack-vm", "panack", "tests"):
                (checkout / name).symlink_to(PROJECT / name)
            shutil.copytree(PROJECT / "src", checkout / "src")
            shutil.copytree(PROJECT / "examples", checkout / "examples")
            for stage in ("stage1", "stage2"):
                target = checkout / "build/bootstrap" / stage / "compiler.bc"
                target.parent.mkdir(parents=True)
                shutil.copyfile(PROJECT / "bootstrap/compiler-v8.bc", target)
            probe = checkout / "probe.sh"
            probe.write_text(
                '#!/bin/sh\nset -eu\n'
                'test "$PANACK_TEST_COMPILER" = "$PWD/build/bootstrap/stage2/compiler.bc"\n'
                './panack-vm run "$PANACK_TEST_COMPILER" compile hello.panack -o hello.bc\n'
                './panack-vm run hello.bc\n',
                encoding="utf-8",
            )
            (checkout / "hello.panack").write_text(
                'main(): Void { print(42) }\n', encoding="utf-8"
            )
            result = subprocess.run(
                ["make", "functional-impl", "PYTHON=sh probe.sh"],
                cwd=checkout, capture_output=True, text=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
            self.assertIn("42", result.stdout.splitlines())
            self.assertEqual(result.stdout.splitlines()[-1], "tests: 255, failures: 0")

    def test_installed_cli_runs_without_source_tree_layout(self):
        with tempfile.TemporaryDirectory() as directory:
            destination = Path(directory) / "installation with spaces (test)"
            result = subprocess.run(
                [
                    "make",
                    "install",
                    f"DESTDIR={destination}",
                    "PREFIX=/usr/local",
                ],
                cwd=PROJECT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            installed = destination / "usr/local"
            command = subprocess.run(
                [str(installed / "bin/panack"), "--help"],
                capture_output=True,
                text=True,
            )
            self.assertEqual(command.returncode, 0, command.stderr)
            self.assertIn("usage: panack ", command.stdout)

            command = subprocess.run(
                [str(installed / "bin/panack"), "--version"],
                capture_output=True,
                text=True,
            )
            self.assertEqual(command.returncode, 0, command.stderr)
            self.assertEqual(
                command.stdout,
                f"panack {RELEASE_VERSION} (bytecode 8)\n",
            )
            self.assertEqual(command.stderr, "")

            source = destination / "logical-import.panack"
            source.write_text(
                "import stdlib/option\n"
                "import stdlib/time\n"
                "import stdlib/path\n"
                "main(): Void {\n"
                "  match Some(42) { Some(value) => print(value), None() => print(0) }\n"
                "  print(duration_ticks(duration_seconds(1)))\n"
                "  print(path_display(path_current()))\n"
                "}\n",
                encoding="utf-8",
            )
            command = subprocess.run(
                [str(installed / "bin/panack"), "run", str(source)],
                capture_output=True,
                text=True,
            )
            self.assertEqual(command.returncode, 0, command.stderr)
            self.assertEqual(command.stdout, "42\n1000000000\n.\n")
            self.assertEqual(command.stderr, "")

            installed_files = {
                path.relative_to(installed).as_posix()
                for path in installed.rglob("*")
                if path.is_file()
            }
            self.assertEqual(
                installed_files,
                INSTALLED_FILES,
            )

    def test_release_archive_is_friendly_and_relocatable(self):
        with tempfile.TemporaryDirectory() as directory:
            temporary = Path(directory)
            build = temporary / "build"
            result = subprocess.run(
                [
                    "make",
                    "package-archive",
                    "PYTHON=false",
                    f"BUILD_DIR={build}",
                ],
                cwd=PROJECT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)

            archives = list(build.glob("panackelty-*.tar.gz"))
            self.assertEqual(len(archives), 1, archives)
            with tarfile.open(archives[0], "r:gz") as archive:
                members = archive.getmembers()
                names = [member.name for member in members]
                self.assertTrue(names)
                self.assertTrue(
                    all(
                        not name.startswith("/")
                        and ".." not in Path(name).parts
                        and Path(name).parts[0] == "panackelty"
                        for name in names
                    ),
                    names,
                )
                self.assertTrue(
                    all(
                        member.uid == 0
                        and member.gid == 0
                        and member.uname == "root"
                        and member.gname == "root"
                        for member in members
                    ),
                    [(member.name, member.uname, member.gname) for member in members],
                )
                archive.extractall(temporary / "extracted")

            extracted = temporary / "extracted" / "panackelty"
            packaged_files = {
                path.relative_to(extracted).as_posix()
                for path in extracted.rglob("*")
                if path.is_file()
            }
            self.assertEqual(
                packaged_files,
                INSTALLED_FILES | EXAMPLE_FILES | {"LICENSE", "README.md"},
            )

            relocated = temporary / "relocated" / "toolchain"
            relocated.parent.mkdir()
            shutil.move(extracted, relocated)
            command = relocated / "bin" / "panack"

            version = subprocess.run(
                [str(command), "--version"],
                cwd=temporary,
                capture_output=True,
                text=True,
            )
            self.assertEqual(version.returncode, 0, version.stderr)
            self.assertEqual(
                version.stdout,
                f"panack {RELEASE_VERSION} (bytecode 8)\n",
            )

            source = temporary / "relocated-import.panack"
            source.write_text(
                "import stdlib/option\n"
                "import stdlib/time\n"
                "import stdlib/path\n"
                "main(): Void {\n"
                "  match Some(42) { Some(value) => print(value), None() => print(0) }\n"
                "  print(duration_ticks(duration_seconds(1)))\n"
                "  print(path_display(path_current()))\n"
                "}\n",
                encoding="utf-8",
            )
            run = subprocess.run(
                [str(command), "run", str(source)],
                cwd=temporary,
                capture_output=True,
                text=True,
            )
            self.assertEqual(run.returncode, 0, run.stderr)
            self.assertEqual(run.stdout, "42\n1000000000\n.\n")

            tour_example = relocated / "examples/collections_and_bytes.panack"
            run = subprocess.run(
                [str(command), "run", str(tour_example)],
                cwd=temporary,
                capture_output=True,
                text=True,
            )
            expected = PROJECT / "tests/functional/expected/examples"
            self.assertEqual(run.returncode, 0, run.stderr)
            self.assertEqual(
                run.stdout,
                (expected / "collections_and_bytes.stdout").read_text(
                    encoding="utf-8"
                ),
            )


if __name__ == "__main__":
    unittest.main()
