import hashlib
import shutil
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
    "share/panackelty/compiler-v7.bc",
    "share/panackelty/stdlib/bytes.panack",
    "share/panackelty/stdlib/collections.panack",
    "share/panackelty/stdlib/environment.panack",
    "share/panackelty/stdlib/option.panack",
    "share/panackelty/stdlib/path.panack",
    "share/panackelty/stdlib/prelude.panack",
    "share/panackelty/stdlib/result.panack",
    "share/panackelty/stdlib/text.panack",
}


class NativeDistributionTests(unittest.TestCase):
    def test_release_checksum_matches_archive(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory) / "build"
            result = subprocess.run(
                [
                    "make",
                    "package-checksum",
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
            archive = archives[0]
            checksum = Path(f"{archive}.sha256")
            self.assertTrue(checksum.is_file())
            digest, name = checksum.read_text(encoding="utf-8").split()
            self.assertEqual(name, archive.name)
            self.assertEqual(
                digest,
                hashlib.sha256(archive.read_bytes()).hexdigest(),
            )

    def test_installed_cli_runs_without_source_tree_layout(self):
        with tempfile.TemporaryDirectory() as directory:
            destination = Path(directory)
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
                f"panack {RELEASE_VERSION} (bytecode 7)\n",
            )
            self.assertEqual(command.stderr, "")

            source = destination / "logical-import.panack"
            source.write_text(
                "import stdlib/option\n"
                "main(): Void {\n"
                "  match Some(42) { Some(value) => print(value), None() => print(0) }\n"
                "}\n",
                encoding="utf-8",
            )
            command = subprocess.run(
                [str(installed / "bin/panack"), "run", str(source)],
                capture_output=True,
                text=True,
            )
            self.assertEqual(command.returncode, 0, command.stderr)
            self.assertEqual(command.stdout, "42\n")
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
                f"panack {RELEASE_VERSION} (bytecode 7)\n",
            )

            source = temporary / "relocated-import.panack"
            source.write_text(
                "import stdlib/option\n"
                "main(): Void {\n"
                "  match Some(42) { Some(value) => print(value), None() => print(0) }\n"
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
            self.assertEqual(run.stdout, "42\n")

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
