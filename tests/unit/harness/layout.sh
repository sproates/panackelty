#!/bin/sh
. tests/unit/harness/common.sh
# Literal contracts migrated from RepositoryLayoutTests; structural checks follow.
case_name=public-launcher-has-no-python-dependency
contains panack panack-vm
pass
case_name=release-version-has-one-canonical-source
contains panack 'read -r release_version < "$version_file"'
pass
case_name=ci-packages-every-supported-release-target
contains .github/workflows/check.yml 'target: linux-x86_64
            runner: ubuntu-22.04'
contains .github/workflows/check.yml 'target: macos-arm64
            runner: macos-14'
contains .github/workflows/check.yml "runs-on: \${{ needs.changes.outputs.route == 'docs' && 'ubuntu-22.04' || matrix.runner }}"
contains .github/workflows/check.yml 'run: make check-no-interpreter'
contains .github/workflows/check.yml 'uses: actions/upload-artifact@v6'
contains .github/workflows/check.yml 'build/panackelty-*.tar.gz.sha256'
contains .github/workflows/check.yml 'build/panackelty-*.tar.gz.provenance'
contains .github/workflows/check.yml 'echo "source_commit=$GITHUB_SHA"'
contains .github/workflows/check.yml 'echo "runner_image_version=${ImageVersion:-unknown}"'
absent .github/workflows/check.yml release:
absent .github/workflows/check.yml tags:
pass
case_name=ci-runs-full-validation-once-per-pr-revision
contains .github/workflows/check.yml 'group: check-${{ github.event.pull_request.number || github.run_id }}'
contains .github/workflows/check.yml 'cancel-in-progress: ${{ github.event_name == '"'"'pull_request'"'"' }}'
contains .github/workflows/check.yml 'name: Package (${{ matrix.target }})'
pass
case_name=release-publication-requires-every-gate
contains .github/workflows/release.yml 'tags:
      - "v*"'
contains .github/workflows/release.yml 'permissions:
  contents: read'
contains .github/workflows/release.yml 'expected_tag="v$(cat VERSION)"'
contains .github/workflows/release.yml 'sh scripts/release_request.sh'
contains .github/workflows/release.yml 'workflow_dispatch:'
contains .github/workflows/release.yml 'cancel-in-progress: false'
contains .github/workflows/release.yml 'test "$(git rev-parse "refs/tags/$expected_tag^{}")" = "$GITHUB_SHA"'
contains .github/workflows/release.yml 'run: make check'
contains .github/workflows/release.yml 'needs: validate'
contains .github/workflows/release.yml 'run: make package'
contains .github/workflows/release.yml 'target: linux-x86_64
            runner: ubuntu-22.04'
contains .github/workflows/release.yml 'target: macos-arm64
            runner: macos-14'
contains .github/workflows/release.yml 'needs: [validate, package]'
contains .github/workflows/release.yml 'uses: actions/download-artifact@v7'
contains .github/workflows/release.yml 'sha256sum -c "${archive}.sha256"'
contains .github/workflows/release.yml 'grep -Fx "source_commit=$GITHUB_SHA"'
contains .github/workflows/release.yml 'gh release create "$expected_tag"'
contains .github/workflows/release.yml 'GH_TOKEN: ${{ github.token }}'
contains .github/workflows/release.yml --verify-tag
contains .github/workflows/release.yml --prerelease
pass
case_name=bug-report-form-requires-actionable-reproduction-details
contains .github/ISSUE_TEMPLATE/bug_report.yml 'name: Bug report'
contains .github/ISSUE_TEMPLATE/bug_report.yml 'panack --version'
contains .github/ISSUE_TEMPLATE/bug_report.yml SECURITY.md
contains CONTRIBUTING.md 'choose
**Bug report**'
contains CONTRIBUTING.md 'private reporting channel'
pass
case_name=readme-quick-start-is-an-executable-release-gate
contains README.md 'panack --version
panack check hello.panack
panack run hello.panack
panack compile hello.panack
panack run hello.bc'
contains README.md '<!-- quick-start-program-begin -->'
contains README.md '<!-- quick-start-program-end -->'
contains README.md '<!-- quick-start-output-begin -->'
contains README.md '<!-- quick-start-output-end -->'
contains Makefile 'check-phases: policy unit functional bootstrap-check quick-start'
contains Makefile 'package: native-check
	$(MAKE) quick-start'
contains tests/quick_start.sh 'for utility in awk cmp dirname gzip ln mkdir mv readlink rm tar; do'
pass
case_name=project-website-is-static-and-deploys-only-from-main
contains site/index.html '<meta name="viewport"'
contains site/index.html '<link rel="canonical" href="https://panackelty.com/">'
contains site/index.html 'href="styles.css"'
contains site/index.html https://github.com/sproates/panackelty
absent site/index.html 'are being prepared'
absent site/index.html 'href="http://'
absent site/index.html 'src="http://'
contains .github/workflows/pages.yml 'push:
    branches: [main]'
contains .github/workflows/pages.yml 'pull_request:
    branches: [main]'
contains .github/workflows/pages.yml 'uses: actions/configure-pages@v5'
contains .github/workflows/pages.yml 'uses: actions/upload-pages-artifact@v4'
contains .github/workflows/pages.yml 'path: site'
contains .github/workflows/pages.yml 'if: github.event_name != '"'"'pull_request'"'"''
contains .github/workflows/pages.yml 'needs: build'
contains .github/workflows/pages.yml 'pages: write'
contains .github/workflows/pages.yml 'id-token: write'
contains .github/workflows/pages.yml 'uses: actions/deploy-pages@v4'
pass
case_name=layout-version-and-implementation
sh tests/no_python.sh
awk 'NR!=1 || !/^[0-9]+\.[0-9]+\.[0-9]+-[a-z]+\.[0-9]+$/ { bad=1 } END { exit bad || NR!=1 }' VERSION || fail 'invalid version'
version=$(cat VERSION)
printf '%s\n' "$version" > "$work/version"
equal_files VERSION "$work/version"
absent panack "$version"
pass
case_name=ci-event-and-command-contracts
contains .github/workflows/check.yml 'run: make check-no-interpreter'
awk '/^on:$/ { selected=1; next } /^concurrency:/ { selected=0 } selected && NF { print }' .github/workflows/check.yml > "$work/events"
printf '  push:\n    branches: [main]\n  pull_request:\n' > "$work/expected"
equal_files "$work/events" "$work/expected"
awk '/^  test:$/ { selected=1; next } selected { print }' .github/workflows/check.yml > "$work/test-job"
sed -n 's/^        run: //p' "$work/test-job" > "$work/commands"
cat > "$work/expected" <<'COMMANDS'
sh scripts/ci_gate.sh
make check
make native-sanitize CC=clang
sudo apt-get update && sudo apt-get install -y clang llvm
make native-coverage LLVM_CC=/usr/bin/clang LLVM_COV=/usr/bin/llvm-cov LLVM_PROFDATA=/usr/bin/llvm-profdata
|
COMMANDS
equal_files "$work/commands" "$work/expected"
for text in build/coverage/summary.txt build/coverage/html/ 'VALIDATION_TIMINGS_FILE: validation-timings.tsv'; do contains "$work/test-job" "$text"; done
test "$(grep -c 'persist-credentials: false' .github/workflows/release.yml)" = 3 || fail 'release checkout credential policy'
test "$(grep -c 'contents: write' .github/workflows/release.yml)" = 1 || fail 'release permission policy'
pass
case_name=required-bug-report-fields
for field in version platform source command expected actual bytecode; do
    awk -v field="$field" '$0=="    id: " field { found=1; active=1; next }
        /^  - type:/ { active=0 } active && /required: true/ { required=1 }
        END { exit !found || !required }' .github/ISSUE_TEMPLATE/bug_report.yml || fail "required field: $field"
done
printf 'blank_issues_enabled: false\n' > "$work/expected"
equal_files .github/ISSUE_TEMPLATE/config.yml "$work/expected"
pass
case_name=website-files-and-version
find site -maxdepth 1 -type f -exec basename {} \; | sort > "$work/files"
printf 'favicon.svg\nindex.html\nstyles.css\n' > "$work/expected"
equal_files "$work/files" "$work/expected"
contains site/index.html "https://github.com/sproates/panackelty/releases/tag/v$version"
contains site/index.html "Developer preview $version is available"
pass
case_name=tour-example-and-specification-links
awk '/^## A quick language tour$/ { active=1; next } /^## Language highlights/ { active=0 } active' README.md > "$work/tour"
grep -o '(examples/[^)]*\.panack)' "$work/tour" | sed 's/^(examples\///;s/)$//' | sort -u > "$work/examples"
cat > "$work/expected" <<'EXAMPLES'
callables.panack
collections_and_bytes.panack
decimal.panack
euler001_iterative.panack
euler003.panack
fizzbuzz.panack
guards.panack
lexer_foundation.panack
option_result.panack
strings.panack
EXAMPLES
equal_files "$work/examples" "$work/expected"
while IFS= read -r example; do
    test -f "examples/$example" || fail "missing example $example"
    test -f "tests/functional/expected/examples/${example%.panack}.stdout" || fail "missing expected output $example"
done < "$work/examples"
grep -o '(SPEC.md#[^)]*)' "$work/tour" | sed 's/^(SPEC.md#//;s/)$//' | sort -u > "$work/links"
test -s "$work/links" || fail 'tour has no specification links'
sed -n 's/^## //p' SPEC.md | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9 -]//g;s/ /-/g' | sort -u > "$work/anchors"
comm -23 "$work/links" "$work/anchors" > "$work/missing"
test ! -s "$work/missing" || fail 'tour links nonexistent specification anchors'
pass
