#!/bin/sh
. tests/unit/harness/common.sh
case_name=native-module-filesystem-race-bound
contains tests/runner/vm_unit.panack 'std_time.seconds(60), 1048576)'
contains tests/unit/vm/native_modules.c 'i < 20000'
contains tests/unit/vm/native_modules.c 'i < 500'
contains tests/unit/vm/native_modules.c 'record_module_progress("filesystem_root_race_reads")'
pass
case_name=large-cli-fixture-compilation-bound
contains tests/runner/main.panack 'compile_timeout = if execution_timeout == 120 { 60 } else { 20 }'
contains tests/runner/main.panack 'label == "case/stdlib", compile_timeout)'
contains tests/unit/harness/runner.sh 'cli_commands|cli_explanations) fixture_timeout=360'
pass
case_name=isolated-environment-retains-tls-fixture-tool
contains tests/without_interpreter.sh 'curl-config openssl'
contains tests/without_interpreter.sh 'openssl security'
contains tests/http_client_package.sh 'extendedKeyUsage=serverAuth'
contains tests/http_client_package.sh 'basicConstraints=critical,CA:FALSE'
contains tests/http_client_package.sh 'openssl verify -CAfile'
contains tests/http_client_package.sh 'openssl req -config "$work/request.cnf" -extensions ca'
contains tests/http_client_package.sh 'openssl req -config "$work/request.cnf" -new'
contains tests/http_client_package.sh 'security verify-cert -c'
contains tests/http_client_package.sh 'HTTP client TLS fixture requires openssl on PATH'
pass
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
contains .github/workflows/check.yml 'runs-on: ${{ matrix.runner }}'
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
contains .github/workflows/check.yml 'group: validation-${{ github.event.pull_request.number || github.run_id }}'
contains .github/workflows/check.yml 'cancel-in-progress: ${{ github.event_name == '"'"'pull_request'"'"' }}'
contains .github/workflows/check.yml 'name: Package (${{ matrix.target }})'
test "$(grep -Fc "if: matrix.suite == 'compiler' && github.event_name == 'pull_request'" .github/workflows/check.yml)" = 5 || fail 'platform compiler experiments must run only on pull requests'
contains .github/workflows/check.yml 'name: Next Linux smoke'
contains .github/workflows/check.yml 'timeout-minutes: 5'
contains .github/workflows/check.yml 'run: make check-no-interpreter CI_SUITE=next-smoke'
contains .github/workflows/check.yml "needs.changes.outputs.next_integration != 'true' && !cancelled()"
absent .github/workflows/check.yml 'platform_smoke'
awk '/^  test_run:$/ { selected=1 } /^  package:$/ { selected=0 } selected { print }' .github/workflows/check.yml > "$work/next-job"
for experiment in source-mapping-experiment runtime-provenance-experiment source-coverage-experiment semantic-impact-experiment; do
    absent "$work/next-job" "$experiment"
done
pass
case_name=expensive-validation-profile-is-manual-only
contains .github/workflows/profile.yml 'workflow_dispatch:'
absent .github/workflows/profile.yml 'pull_request:'
absent .github/workflows/profile.yml 'push:'
contains .github/workflows/profile.yml 'libcurl4-openssl-dev'
pass
case_name=nightly-package-workflow-targets-next
contains .github/workflows/nightly-packages.yml 'schedule:'
contains .github/workflows/nightly-packages.yml "ref: \${{ inputs.branch || 'next' }}"
contains .github/workflows/nightly-packages.yml 'suite: [compiler, runtime, bootstrap, conformance-source, conformance-bytecode]'
contains .github/workflows/nightly-packages.yml 'make check-no-interpreter CI_SUITE=${{ matrix.suite }}'
contains .github/workflows/nightly-packages.yml 'echo "sha=$(git rev-parse HEAD)"'
contains .github/workflows/nightly-packages.yml 'name: panackelty-${{ matrix.target }}-${{ steps.source.outputs.sha }}'
pass
case_name=release-publication-requires-every-gate
absent .github/workflows/release.yml 'source_coverage_gate'
absent .github/workflows/release.yml 'coverage/source/'
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
contains README.md './panackelty/bin/panack --version
./panackelty/bin/panack check hello.panack
./panackelty/bin/panack run hello.panack
./panackelty/bin/panack compile hello.panack
./panackelty/bin/panack run hello.bc'
contains README.md '<!-- quick-start-program-begin -->'
contains README.md '<!-- quick-start-program-end -->'
contains README.md '<!-- quick-start-output-begin -->'
contains README.md '<!-- quick-start-output-end -->'
contains Makefile 'check-phases: policy'
contains Makefile 'sh tests/run_check_phases.sh'
contains Makefile '@$(MAKE) --no-print-directory functional'
contains Makefile '@$(MAKE) --no-print-directory quick-start'
contains Makefile 'package: native-check
	$(MAKE) quick-start'
contains tests/quick_start.sh 'for utility in awk cmp dirname gzip ln mkdir mv readlink rm tar; do'
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
printf '  push:\n    branches: [main, next]\n  pull_request:\n' > "$work/expected"
equal_files "$work/events" "$work/expected"
cp .github/workflows/source-coverage.yml "$work/source-job"
for text in 'node --test tests/source_coverage/reader.test.cjs' \
    'tests/source_coverage/manifest.test.cjs' 'tests/source_coverage/policy.test.cjs' \
    'node tests/source_coverage/parity.cjs' 'node scripts/source_coverage_baseline.cjs' \
    'name: source-coverage-${{ github.run_id }}' 'path: build/source-coverage' 'if-no-files-found: error'; do
    contains "$work/source-job" "$text"
done
absent .github/workflows/check.yml 'node scripts/source_coverage_baseline.cjs'
contains .github/workflows/check.yml 'name: Source coverage policy tests'
contains .github/workflows/check.yml 'tests/source_coverage/manifest.test.cjs'
contains .github/workflows/check.yml 'tests/source_coverage/release_gate.test.cjs'
contains .github/workflows/check.yml 'tests/source_coverage/route.test.cjs'
contains .github/workflows/check.yml 'needs: [changes, test_run, next_smoke, source_policy_tests, workflow_validation]'
contains .github/workflows/check.yml 'name: Workflow configuration validation'
contains .github/workflows/check.yml "needs.changes.outputs.route == 'workflow'"
contains .github/workflows/check.yml 'timeout-minutes: 5'
contains .github/workflows/check.yml 'run: sh tests/workflow_yaml.sh'
contains .github/workflows/check.yml 'run: node scripts/source_coverage_route.cjs --gate'
contains scripts/validate_change.sh 'route=workflow)'
contains scripts/validate_change.sh 'bash tests/ci_scope.sh'
contains scripts/validate_change.sh 'sh tests/workflow_yaml.sh'
contains scripts/validate_change.sh 'node --test tests/source_coverage/route.test.cjs'
awk '/^  test_run:$/ { selected=1; next } /^  package:$/ { selected=0 } selected { print }' .github/workflows/check.yml > "$work/test-job"
sed -n 's/^        run: //p' "$work/test-job" > "$work/commands"
cat > "$work/expected" <<'COMMANDS'
|
sh tests/profile_command.sh ci/${{ matrix.suite }} make ci-${{ matrix.suite }}
make native-sanitize CC=clang SANITIZE_SUITE="${SANITIZE_JOB#sanitize-}"
|
make native-coverage LLVM_CC=/usr/bin/clang-18 LLVM_COV=/usr/bin/llvm-cov-18 LLVM_PROFDATA=/usr/bin/llvm-profdata-18
|
COMMANDS
equal_files "$work/commands" "$work/expected"
contains "$work/test-job" 'runs-on: ubuntu-24.04'
for tool in clang-18 llvm-cov-18 llvm-profdata-18; do
    contains "$work/test-job" "/usr/bin/$tool --version"
done
contains "$work/test-job" 'sudo apt-get install --yes libcurl4-openssl-dev'
contains "$work/source-job" 'sudo apt-get install --yes libcurl4-openssl-dev'
for workflow in .github/workflows/nightly-packages.yml .github/workflows/release.yml; do
    contains "$workflow" 'sudo apt-get install --yes libcurl4-openssl-dev'
done
for text in build/coverage/summary.txt build/coverage/html/ 'VALIDATION_TIMINGS_FILE: ${{ github.workspace }}/validation-timings.tsv' \
    'suite: [compiler, runtime, bootstrap, sanitize-vm, sanitize-oracle, sanitize-runner, coverage]' \
    "if: startsWith(matrix.suite, 'sanitize-')" "if: matrix.suite == 'coverage'"; do contains "$work/test-job" "$text"; done
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
