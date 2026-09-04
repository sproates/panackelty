import re
import unittest
from pathlib import Path


PROJECT = Path(__file__).resolve().parents[2]
SRC = PROJECT / "src"


class RepositoryLayoutTests(unittest.TestCase):
    def test_python_implementation_is_confined_to_bootstrap(self):
        python_sources = {
            path.relative_to(PROJECT).as_posix() for path in SRC.rglob("*.py")
        }
        self.assertEqual(
            python_sources,
            {"src/bootstrap/panackelty.py"},
        )

    def test_public_launcher_has_no_python_dependency(self):
        launcher = (PROJECT / "panack").read_text(encoding="utf-8")
        self.assertNotIn("python", launcher.lower())
        self.assertIn("panack-vm", launcher)

    def test_release_version_has_one_canonical_source(self):
        version = (PROJECT / "VERSION").read_text(encoding="utf-8")
        self.assertRegex(version, re.compile(r"^\d+\.\d+\.\d+-[a-z]+\.\d+\n$"))
        launcher = (PROJECT / "panack").read_text(encoding="utf-8")
        self.assertIn('read -r release_version < "$version_file"', launcher)
        self.assertNotIn(version.strip(), launcher)

    def test_ci_packages_every_supported_release_target(self):
        workflow = (PROJECT / ".github/workflows/check.yml").read_text(
            encoding="utf-8"
        )
        self.assertIn(
            "target: linux-x86_64\n            runner: ubuntu-22.04", workflow
        )
        self.assertIn(
            "target: macos-arm64\n            runner: macos-14", workflow
        )
        self.assertIn("runs-on: ${{ matrix.runner }}", workflow)
        self.assertIn("run: make package PYTHON=false", workflow)
        self.assertIn("uses: actions/upload-artifact@v4", workflow)
        self.assertIn("build/panackelty-*.tar.gz.sha256", workflow)
        self.assertIn("build/panackelty-*.tar.gz.provenance", workflow)
        self.assertIn('echo "source_commit=$GITHUB_SHA"', workflow)
        self.assertIn(
            'echo "runner_image_version=${ImageVersion:-unknown}"', workflow
        )
        self.assertNotIn("release:", workflow)
        self.assertNotIn("tags:", workflow)

    def test_release_publication_requires_every_gate(self):
        workflow = (PROJECT / ".github/workflows/release.yml").read_text(
            encoding="utf-8"
        )
        self.assertIn('tags:\n      - "v*"', workflow)
        self.assertIn("permissions:\n  contents: read", workflow)
        self.assertIn('expected_tag="v$(cat VERSION)"', workflow)
        self.assertIn('[[ "$GITHUB_REF_NAME" != "$expected_tag" ]]', workflow)
        self.assertIn("run: make check", workflow)
        self.assertIn("needs: validate", workflow)
        self.assertIn("run: make package PYTHON=false", workflow)
        self.assertIn(
            "target: linux-x86_64\n            runner: ubuntu-22.04", workflow
        )
        self.assertIn(
            "target: macos-arm64\n            runner: macos-14", workflow
        )
        self.assertIn("needs: [validate, package]", workflow)
        self.assertIn("uses: actions/download-artifact@v4", workflow)
        self.assertIn('sha256sum -c "${archive}.sha256"', workflow)
        self.assertIn('grep -Fx "source_commit=$GITHUB_SHA"', workflow)
        self.assertIn('gh release create "$GITHUB_REF_NAME"', workflow)
        self.assertIn("GH_TOKEN: ${{ github.token }}", workflow)
        self.assertIn("--verify-tag", workflow)
        self.assertIn("--prerelease", workflow)
        self.assertEqual(workflow.count("persist-credentials: false"), 3)
        self.assertEqual(workflow.count("contents: write"), 1)

    def test_bug_report_form_requires_actionable_reproduction_details(self):
        template = (
            PROJECT / ".github/ISSUE_TEMPLATE/bug_report.yml"
        ).read_text(encoding="utf-8")
        self.assertIn("name: Bug report", template)
        self.assertIn("panack --version", template)
        self.assertIn("SECURITY.md", template)
        for field in (
            "version",
            "platform",
            "source",
            "command",
            "expected",
            "actual",
            "bytecode",
        ):
            marker = f"    id: {field}\n"
            self.assertIn(marker, template)
            field_body = template.split(marker, 1)[1].split("\n  - type:", 1)[0]
            self.assertIn("required: true", field_body)

        config = (
            PROJECT / ".github/ISSUE_TEMPLATE/config.yml"
        ).read_text(encoding="utf-8")
        self.assertEqual(config, "blank_issues_enabled: false\n")

        contributing = (PROJECT / "CONTRIBUTING.md").read_text(encoding="utf-8")
        self.assertIn("choose\n**Bug report**", contributing)
        self.assertIn("private reporting channel", contributing)

    def test_readme_quick_start_is_an_executable_release_gate(self):
        readme = (PROJECT / "README.md").read_text(encoding="utf-8")
        commands = (
            "panack --version\n"
            "panack check hello.panack\n"
            "panack run hello.panack\n"
            "panack compile hello.panack\n"
            "panack run hello.bc"
        )
        self.assertIn("<!-- quick-start-program-begin -->", readme)
        self.assertIn("<!-- quick-start-program-end -->", readme)
        self.assertIn("<!-- quick-start-output-begin -->", readme)
        self.assertIn("<!-- quick-start-output-end -->", readme)
        self.assertIn(commands, readme)
        makefile = (PROJECT / "Makefile").read_text(encoding="utf-8")
        self.assertIn(
            "check-phases: unit functional bootstrap-check quick-start", makefile
        )
        self.assertIn("package: native-check\n\t$(MAKE) quick-start", makefile)

    def test_language_tour_links_tested_examples_and_specification_sections(self):
        readme = (PROJECT / "README.md").read_text(encoding="utf-8")
        tour = readme.split("## A quick language tour\n", 1)[1].split(
            "\n## Language highlights", 1
        )[0]
        examples = set(re.findall(r"\(examples/([^)]+\.panack)\)", tour))
        self.assertEqual(
            examples,
            {
                "callables.panack",
                "collections_and_bytes.panack",
                "decimal.panack",
                "euler001_iterative.panack",
                "euler003.panack",
                "fizzbuzz.panack",
                "guards.panack",
                "lexer_foundation.panack",
                "option_result.panack",
                "strings.panack",
            },
        )
        for example in examples:
            self.assertTrue((PROJECT / "examples" / example).is_file(), example)
            expected = PROJECT / "tests/functional/expected/examples"
            self.assertTrue((expected / f"{Path(example).stem}.stdout").is_file())

        linked_anchors = set(re.findall(r"\(SPEC\.md#([^)]+)\)", tour))
        specification = (PROJECT / "SPEC.md").read_text(encoding="utf-8")
        heading_anchors = {
            re.sub(r"[^a-z0-9 -]", "", heading.lower()).replace(" ", "-")
            for heading in re.findall(r"^## (.+)$", specification, re.MULTILINE)
        }
        self.assertTrue(linked_anchors)
        self.assertEqual(linked_anchors - heading_anchors, set())


if __name__ == "__main__":
    unittest.main()
