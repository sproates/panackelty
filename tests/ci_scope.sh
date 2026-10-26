#!/usr/bin/env bash
# Standalone routing/guard/link regressions: no compiler or installed packages.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
work=$(mktemp -d)
trap 'rm -r "$work"' EXIT
unset VALIDATION_PROFILE_FILE VALIDATION_TIMINGS_FILE
index=0
fail() { echo "FAIL CI: $*" >&2; exit 1; }
fixture() {
    index=$((index + 1))
    mkdir "$work/$index"
    cd "$work/$index"
    git init -q
    git config user.name sproates
    git config user.email 218187+sproates@users.noreply.github.com
    mkdir src
    printf '# Roadmap\n\n[Architecture](ARCHITECTURE.md)\n' > ROADMAP.md
    printf '# Architecture\n' > ARCHITECTURE.md
    cat > README.md <<'README'
<!-- validation:positioning-copy-begin -->
# Readme
Positioning copy.
[Roadmap](ROADMAP.md)
<!-- validation:positioning-copy-end -->
## Exact fractions and Unit
<!-- validation:coverage-publication-copy-begin -->
Browse the coverage report. Publisher behavior is described here.
<!-- validation:coverage-publication-copy-end -->
Direct bytecode/verification coverage runs in the test suite.
README
    printf 'int main(void) { return 0; }\n' > src/main.c
    git add .
    git commit -qm baseline
    base=$(git rev-parse HEAD)
}
commit() { git add -A; git commit -qm change; head=$(git rev-parse HEAD); }
route() {
    actual=$(bash "$root/scripts/ci_scope.sh" "$base" "$head")
    [[ "$actual" == "$1" ]] || fail "expected $1, got $actual ($index)"
}
reject_docs() {
    if bash "$root/scripts/check_docs.sh" > "$work/output" 2>&1; then fail 'accepted broken docs'; fi
    grep -F "$1" "$work/output" >/dev/null || { cat "$work/output"; fail "missing diagnostic: $1"; }
}
fixture
printf '\nNext step\n' >> ROADMAP.md
commit; route docs
bash "$root/scripts/check_docs.sh" >/dev/null
# The package design is an informational document: edits use the documentation
# route while its local links and formatting remain checked.
fixture
mkdir -p docs
printf '# Module and package design\n' > docs/MODULE_PACKAGE_DESIGN.md
git add docs/MODULE_PACKAGE_DESIGN.md
git commit -qm 'design document fixture'
base=$(git rev-parse HEAD)
printf '\nClarify package boundaries.\n' >> docs/MODULE_PACKAGE_DESIGN.md
commit; route docs
plan=$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")
[[ "$plan" == $'route=docs\ncomponents=documentation\nchecks=documents,links,whitespace' ]] || fail 'module package design plan'
bash "$root/scripts/check_docs.sh" >/dev/null
# The bounded public-coverage publication guidance is also informational.
fixture
awk '{ if ($0 ~ /Publisher behavior is described here/) sub(/Publisher behavior is described here/, "Publication is on demand."); print }' README.md > "$work/readme"
mv "$work/readme" README.md
commit; route docs
plan=$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")
[[ "$plan" == $'route=docs\ncomponents=documentation\nchecks=documents,links,whitespace' ]] || fail 'README coverage publication plan'
bash "$root/scripts/check_docs.sh" >/dev/null
# Copy inside the explicit README positioning region takes the docs route.
fixture
awk '{ if ($0 == "Positioning copy.") $0 = "Updated positioning copy."; print }' README.md > "$work/readme"
mv "$work/readme" README.md
commit; route docs
plan=$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")
[[ "$plan" == $'route=docs\ncomponents=documentation\nchecks=documents,links,whitespace' ]] || fail 'README positioning plan'
bash "$root/scripts/check_docs.sh" >/dev/null
# A historical README without markers may add the bounded markers and edit the
# preamble in one change; content from the first executable section onward must
# still remain byte-identical.
fixture
awk '/^<!-- validation:positioning-copy-begin -->$/ { next }
     /^<!-- validation:positioning-copy-end -->$/ { next }
     { print }' README.md > "$work/readme"
mv "$work/readme" README.md
git add README.md
git commit -qm 'legacy README fixture'
base=$(git rev-parse HEAD)
awk 'BEGIN { print "<!-- validation:positioning-copy-begin -->" }
     /^Positioning copy\.$/ { print "Updated positioning copy."; next }
     /^## Exact fractions and Unit$/ { print "<!-- validation:positioning-copy-end -->" }
     { print }' README.md > "$work/readme"
mv "$work/readme" README.md
commit; route docs
# Executable/example/package prose outside the bounded region remains full.
fixture
printf '\nREADME quick-start or package guidance\n' >> README.md
commit; route full
# The complete PR diff matters, not just its latest docs commit.
printf '\n/* implementation */\n' >> src/main.c
commit
printf '\nMore notes\n' >> ROADMAP.md
commit; route full
fixture
printf '# Bootstrap\n' > SELF_HOSTING.md
commit; route docs
fixture
mkdir docs
printf '# Package design\n' > docs/MODULE_PACKAGE_DESIGN.md
commit; route docs
fixture
mkdir docs
printf '# New documentation\n' > docs/new.md
commit; route docs
fixture
git rm -q ROADMAP.md
commit; route docs
reject_docs 'missing local link: README.md -> ROADMAP.md'
fixture
git mv ROADMAP.md SELF_HOSTING.md
commit; route docs
reject_docs 'missing local link: README.md -> ROADMAP.md'
fixture
git mv ROADMAP.md spec.txt
commit; route full
fixture
git mv src/main.c SELF_HOSTING.md
commit; route full
fixture
chmod +x ROADMAP.md
commit; route full
fixture
git rm -q ROADMAP.md
ln -s ARCHITECTURE.md ROADMAP.md
commit; route full
reject_docs 'symlink document'
# Unknown, packaged, executable, specification and policy changes stay full.
for path in README.md SPEC.md VERSION CHANGELOG.md \
    tests/functional/input.md \
    'notes with spaces.md' $'notes\nnewline.md'; do
    fixture
    mkdir -p "$(dirname "$path")"
    printf '\nchange\n' >> "$path"
    commit; route full
done
# Markdown documentation in nested docs folders is covered by the same glob.
fixture
mkdir -p docs/nested
printf '# Nested documentation\n' > docs/nested/new.md
commit; route docs
# Reviewed process prose is not consumed by builds, packages or fixtures.
for path in CONTRIBUTING.md docs/ROADMAP_PROCESS.md RELEASE_POLICY.md \
    docs/SOURCE_COVERAGE_BASELINE.md .github/pull_request_template.md; do
    fixture
    mkdir -p "$(dirname "$path")"
    printf '# Process\n' > "$path"
    commit; route docs
    plan=$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")
    [[ "$plan" == $'route=docs\ncomponents=process\nchecks=documents,links,whitespace' ]] || fail 'process plan'
done
fixture
mkdir -p .github/workflows
printf 'name: Nightly\non: {}\njobs: {}\n' > .github/workflows/nightly-packages.yml
commit; route workflow
plan=$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")
[[ "$plan" == $'route=workflow\ncomponents=workflow\nchecks=workflow-syntax,workflow-contracts,documents,links,whitespace' ]] || fail 'workflow-only plan'
fixture
mkdir -p .github/workflows
printf 'name: Profile\non: {}\njobs: {}\n' > .github/workflows/profile.yml
printf '\nWorkflow scope documentation\n' >> ROADMAP.md
commit; route workflow
fixture
mkdir -p .github/workflows
printf 'name: Nightly\non: {}\njobs: {}\n' > .github/workflows/nightly-packages.yml
printf '\n/* mixed product change */\n' >> src/main.c
commit; route full
fixture
mkdir -p .github/workflows
printf 'name: Check\non: {}\njobs: {}\n' > .github/workflows/check.yml
commit; route workflow
fixture
mkdir -p scripts tests
printf '#!/bin/sh\n' > scripts/ci_scope.sh
printf '#!/bin/sh\n' > scripts/validate_change.sh
printf '#!/bin/sh\n' > scripts/ci_gate.sh
printf '#!/usr/bin/env bash\n' > tests/ci_scope.sh
commit; route workflow
fixture
mkdir -p .github docs tests
for path in .github/pull_request_template.md ROADMAP.md docs/ROADMAP_PROCESS.md tests/COVERAGE.md; do
    printf '\nProcess handover update\n' >> "$path"
done
commit; route docs
# Retired website paths are unknown/shared inputs and must never bypass core checks.
for path in site/index.html site/styles.css site/playground.json; do
    fixture
    mkdir site; printf 'asset\n' > "$path"
    commit; route full
    git rm -q "$path"; commit; route full
done
# Expected ownership examples are deliberately independent of classifier patterns.
while read -r path component; do
    fixture
    mkdir -p "$(dirname "$path")"
    printf 'changed\n' > "$path"
    commit; route full
    plan=$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")
    [[ "$plan" == *"components=$component"* ]] || fail "component: $path"
    checks=compiler,runtime,tcp,bootstrap,conformance,sanitizers,coverage,packages
    [[ "$plan" == *"$checks" ]] || fail "lost integrations: $path"
done <<'CASES'
src/compiler/checker.panack compiler
src/bytecode/reader.panack bytecode
src/vm/vm.c runtime
src/runtime/host.panack runtime
src/vm/host_tcp.c tcp
tests/tcp_serve.sh tcp
src/stdlib/array.panack stdlib
scripts/unlisted_helper.sh shared
site/extra.js unknown
VERSION package
examples/hello.panack examples
SPEC.md shared
bootstrap/compiler-v9.bc shared
Makefile shared
unrecognised/input.data unknown
CASES
# Local classification includes staged, unstaged and untracked changes, with the
# same plan after commit. Index-only changes cannot disappear behind a reversal.
fixture
printf '\nprose\n' >> ROADMAP.md
local_plan=$(bash "$root/scripts/ci_scope.sh" --plan --worktree "$base")
commit
[[ "$local_plan" == "$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")" ]] || fail 'local/CI disagreement'
printf 'new code\n' > 'untracked input.c'
[[ $(bash "$root/scripts/ci_scope.sh" --worktree "$base") == full ]] || fail 'ignored untracked file'
rm 'untracked input.c'
printf 'staged code\n' >> src/main.c
git add src/main.c
git show "$base:src/main.c" > src/main.c
[[ $(bash "$root/scripts/ci_scope.sh" --worktree "$base") == full ]] || fail 'ignored index-only change'
fixture
printf '\npackage guidance\n' >> README.md
git add README.md
git show "$base:README.md" > README.md
[[ $(bash "$root/scripts/ci_scope.sh" --worktree "$base") == full ]] || fail 'ignored staged README package change'
fixture
chmod +x ROADMAP.md
[[ $(bash "$root/scripts/ci_scope.sh" --worktree "$base") == full ]] || fail 'ignored local executable mode'
fixture
rm ROADMAP.md
ln -s ARCHITECTURE.md ROADMAP.md
[[ $(bash "$root/scripts/ci_scope.sh" --worktree "$base") == full ]] || fail 'ignored local symlink'
fixture
mkdir docs
printf '# Process\n\n[bad](missing.md)\n' > docs/ROADMAP_PROCESS.md
[[ $(bash "$root/scripts/ci_scope.sh" --worktree "$base") == docs ]] || fail 'untracked process doc'
reject_docs 'missing local link'
rm docs/ROADMAP_PROCESS.md
bash "$root/scripts/check_docs.sh" >/dev/null || fail 'local unreferenced deletion'
fixture
head=$base; route full
[[ $(bash "$root/scripts/ci_scope.sh" unknown "$base") == full ]] || fail 'unknown base'
[[ $(bash "$root/scripts/ci_scope.sh" "$base" 0000000000000000000000000000000000000000) == full ]] || fail 'missing head'
[[ $(bash "$root/scripts/ci_scope.sh") == full ]] || fail 'missing arguments'
# A PR whose base advanced must compare from its merge base.
printf '\nfeature\n' >> ROADMAP.md
commit; feature=$head
git checkout -q "$base"
printf '\nbase only code change\n' >> src/main.c
commit; base=$head; head=$feature
route docs
# Exercise the public local entry point with the actual Makefile and no compiler
# tree. Forbidden tools fail if a prose edit accidentally invokes native/network
# work. This also proves failures propagate and planning does not execute checks.
fixture
mkdir scripts
cp "$root"/scripts/{ci_docs.sh,ci_scope.sh,validation_components.sh,validate_change.sh,check_docs.sh,doc_links.awk} scripts/
cp "$root/Makefile" .
printf '0.0.0-test\n' > VERSION
commit; base=$head
printf '\nlocal prose\n' >> ROADMAP.md
mkdir "$work/forbidden"
for tool in cc gcc clang panack panack-vm curl nc node npm; do
    printf '#!/bin/sh\necho forbidden-tool >&2\nexit 93\n' > "$work/forbidden/$tool"
    chmod +x "$work/forbidden/$tool"
done
PATH="$work/forbidden:$PATH" bash scripts/validate_change.sh --run "$base" > "$work/output" 2>&1 || { cat "$work/output"; fail 'local docs execution'; }
[[ ! -e build && ! -e panack-vm ]] || fail 'documentation created native artifacts'
# The local entry point also keeps a marked README positioning edit on docs.
awk '{ if ($0 == "Positioning copy.") $0 = "Locally updated positioning copy."; print }' README.md > "$work/readme"
mv "$work/readme" README.md
PATH="$work/forbidden:$PATH" bash scripts/validate_change.sh --run "$base" > "$work/output" 2>&1 || { cat "$work/output"; fail 'local README positioning route'; }
[[ ! -e build && ! -e panack-vm ]] || fail 'README positioning created native artifacts'
# The workflow route runs the focused configuration contracts and never falls
# through to make docs or the full compiler suite.
fixture
mkdir -p scripts .github/workflows tests/unit/harness tests/source_coverage
cp "$root"/scripts/{ci_docs.sh,ci_scope.sh,validation_components.sh,validate_change.sh,check_docs.sh,doc_links.awk} scripts/
cp "$root"/Makefile .
printf '%s\n' 'printf "ci_scope\\n" >> "$VALIDATION_CALLS"' > tests/ci_scope.sh
printf '%s\n' 'printf "workflow_yaml\\n" >> "$VALIDATION_CALLS"' > tests/workflow_yaml.sh
printf '%s\n' 'printf "layout\\n" >> "$VALIDATION_CALLS"' > tests/unit/harness/layout.sh
printf '%s\n' "require('node:test')('workflow route contract dispatch', () => require('node:fs').appendFileSync(process.env.VALIDATION_CALLS, 'route\\n'));" > tests/source_coverage/route.test.cjs
printf 'name: Check\non: {}\njobs: {}\n' > .github/workflows/check.yml
git add .
git commit -qm baseline
base=$(git rev-parse HEAD)
printf '\nname: Check\non: {}\njobs: {}\n' > .github/workflows/check.yml
commit
mkdir "$work/no-make"
printf '%s\n' "echo 'unexpected make invocation' >&2" 'exit 93' > "$work/no-make/make"
chmod +x "$work/no-make/make"
export VALIDATION_CALLS="$work/validation-calls"
: > "$VALIDATION_CALLS"
PATH="$work/no-make:$PATH" bash scripts/validate_change.sh --run "$base" > "$work/output" 2>&1 || { cat "$work/output"; fail 'local workflow route'; }
printf 'ci_scope\nworkflow_yaml\nlayout\nroute\n' > "$work/expected-calls"
cmp "$VALIDATION_CALLS" "$work/expected-calls" || fail 'workflow route ran the wrong focused checks'
unset VALIDATION_CALLS
printf '\n[broken](absent.md)\n' >> ROADMAP.md
bash scripts/validate_change.sh --plan "$base" >/dev/null || fail 'plan executed checks'
if bash scripts/validate_change.sh --run "$base" > "$work/output" 2>&1; then fail 'local ignored docs failure'; fi
git restore ROADMAP.md
printf '\ntrailing whitespace  \n' >> ROADMAP.md
commit
if bash scripts/validate_change.sh --run "$base" > "$work/output" 2>&1; then fail 'ignored committed whitespace'; fi
if bash scripts/validate_change.sh --plan missing-ref > "$work/output" 2>&1; then fail 'accepted missing local base'; fi
if bash scripts/validate_change.sh --invalid "$base" > "$work/output" 2>&1; then fail 'accepted invalid local mode'; fi
# Full-route execution keeps its canonical check and propagates failures.
fixture
mkdir scripts
cp "$root"/scripts/{ci_docs.sh,ci_scope.sh,validation_components.sh,validate_change.sh} scripts/
commit; base=$head
printf '\ncode\n' >> src/main.c
mkdir "$work/local-bin"
cat > "$work/local-bin/make" <<'MAKE'
#!/bin/sh
printf '%s\n' "$*" >> "$LOCAL_CHECK_CALLS"
if [ "$*" = "${LOCAL_CHECK_FAIL:-}" ]; then exit 7; fi
MAKE
chmod +x "$work/local-bin/make"
export LOCAL_CHECK_CALLS="$work/local-calls"
for failure in none docs check; do
    : > "$LOCAL_CHECK_CALLS"
    result=0
    PATH="$work/local-bin:$PATH" LOCAL_CHECK_FAIL="$failure" bash scripts/validate_change.sh --run "$base" > "$work/output" 2>&1 || result=$?
    if [[ "$failure" == none ]]; then [[ "$result" == 0 ]] || fail 'full local route';
    else [[ "$result" == 7 ]] || fail "local failure lost: $failure"; fi
    printf 'docs\n' > "$work/expected"
    if [[ "$failure" != docs ]]; then printf 'check\n' >> "$work/expected"; fi
    cmp "$LOCAL_CHECK_CALLS" "$work/expected" || fail 'incorrect local check execution'
done
unset LOCAL_CHECK_CALLS
# Link forms, fences, spaces, nesting and parent-directory resolution.
fixture
mkdir tests assets
printf '# Notes\n\n[root](../ROADMAP.md)\n' > tests/README.md
printf 'image\n' > 'assets/picture (one).png'
cat >> ROADMAP.md <<'DOC'

![Image](assets/picture%20%28one%29.png)
[Image with spaces](<assets/picture (one).png>)
[Architecture](ARCHITECTURE.md#heading "Title")
[arch]: ARCHITECTURE.md
[external](https://example.invalid/no-network-request)
`[example](missing-code-example.md)`
~~~text
[example](missing-fenced-example.md)
~~~
DOC
commit
bash "$root/scripts/check_docs.sh" >/dev/null
printf '\n[broken](missing.md)\n' >> ROADMAP.md
reject_docs 'missing local link'
fixture
printf '\n<<<<<<< unresolved\n' >> ROADMAP.md
reject_docs 'conflict marker'
fixture
: > ROADMAP.md
reject_docs 'empty or symlink document'
fixture
printf '\n[escape](../outside.md)\n' >> ROADMAP.md
reject_docs 'link escapes repository'
fixture
printf '\000binary\n' > ROADMAP.md
reject_docs 'NUL byte'
fixture
printf '\n[broken](missing.md\n' >> ROADMAP.md
reject_docs 'unclosed inline link'
# A failed or cancelled classifier must fail every stable check, not skip green.
for result in failure cancelled skipped ''; do
    for selected in docs workflow website full ''; do
        if CI_SCOPE_RESULT="$result" CI_SCOPE_ROUTE="$selected" CI_VALIDATION_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted failed classification'; fi
    done
done
for selected in '' invalid; do
    if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE="$selected" CI_VALIDATION_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted invalid route'; fi
done
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=docs CI_VALIDATION_RESULT=skipped CI_WORKFLOW_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=workflow CI_VALIDATION_RESULT=skipped CI_WORKFLOW_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_VALIDATION_RESULT=success CI_WORKFLOW_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null
if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=docs CI_VALIDATION_RESULT=skipped CI_NEXT_SMOKE_RESULT=success CI_WORKFLOW_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted product smoke on a docs route'; fi
if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=workflow CI_VALIDATION_RESULT=skipped CI_NEXT_SMOKE_RESULT=success CI_WORKFLOW_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted product smoke on a workflow route'; fi
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_VALIDATION_RESULT=skipped CI_NEXT_SMOKE_RESULT=success CI_NEXT_INTEGRATION=true CI_EVENT_NAME=pull_request CI_WORKFLOW_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_VALIDATION_RESULT=skipped CI_NEXT_SMOKE_RESULT=skipped CI_NEXT_INTEGRATION=true CI_EVENT_NAME=push CI_WORKFLOW_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null
if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_VALIDATION_RESULT=success CI_NEXT_SMOKE_RESULT=success CI_NEXT_INTEGRATION=true CI_EVENT_NAME=pull_request CI_WORKFLOW_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted repeated full next validation'; fi
if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_VALIDATION_RESULT=skipped CI_NEXT_SMOKE_RESULT=skipped CI_NEXT_INTEGRATION=true CI_EVENT_NAME=pull_request CI_WORKFLOW_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted a skipped next PR smoke'; fi
if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=website CI_VALIDATION_RESULT=skipped CI_WORKFLOW_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted retired website route'; fi
if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=workflow CI_VALIDATION_RESULT=skipped CI_WORKFLOW_RESULT=failure sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted failed workflow validation'; fi
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=docs CI_PACKAGE_BUILD_RESULT=skipped \
    CI_PLATFORM_SMOKE_RESULT=skipped CI_WORKFLOW_RESULT=skipped CI_NEXT_INTEGRATION=true CI_EVENT_NAME=pull_request \
    sh "$root/scripts/ci_package_gate.sh" >/dev/null
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=workflow CI_PACKAGE_BUILD_RESULT=skipped \
    CI_PLATFORM_SMOKE_RESULT=skipped CI_WORKFLOW_RESULT=success CI_NEXT_INTEGRATION=true CI_EVENT_NAME=pull_request \
    sh "$root/scripts/ci_package_gate.sh" >/dev/null
if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=workflow CI_PACKAGE_BUILD_RESULT=skipped \
    CI_PLATFORM_SMOKE_RESULT=skipped CI_WORKFLOW_RESULT=failure CI_NEXT_INTEGRATION=true CI_EVENT_NAME=pull_request \
    sh "$root/scripts/ci_package_gate.sh" >/dev/null 2>&1; then fail 'accepted failed workflow validation in package gate'; fi
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_PACKAGE_BUILD_RESULT=skipped \
    CI_PLATFORM_SMOKE_RESULT=skipped CI_WORKFLOW_RESULT=skipped CI_NEXT_INTEGRATION=true CI_EVENT_NAME=pull_request \
    sh "$root/scripts/ci_package_gate.sh" >/dev/null
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_PACKAGE_BUILD_RESULT=skipped \
    CI_PLATFORM_SMOKE_RESULT=skipped CI_WORKFLOW_RESULT=skipped CI_NEXT_INTEGRATION=true CI_EVENT_NAME=push \
    sh "$root/scripts/ci_package_gate.sh" >/dev/null
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_PACKAGE_BUILD_RESULT=success \
    CI_PLATFORM_SMOKE_RESULT=skipped CI_WORKFLOW_RESULT=skipped CI_NEXT_INTEGRATION=false CI_EVENT_NAME=pull_request \
    sh "$root/scripts/ci_package_gate.sh" >/dev/null
for policy in \
    'CI_PACKAGE_BUILD_RESULT=success CI_PLATFORM_SMOKE_RESULT=skipped CI_NEXT_INTEGRATION=true CI_EVENT_NAME=pull_request' \
    'CI_PACKAGE_BUILD_RESULT=cancelled CI_PLATFORM_SMOKE_RESULT=skipped CI_NEXT_INTEGRATION=false CI_EVENT_NAME=pull_request'; do
    if env CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_WORKFLOW_RESULT=skipped $policy sh "$root/scripts/ci_package_gate.sh" >/dev/null 2>&1; then
        fail 'accepted a skipped, failed or misrouted package validation'
    fi
done
[ "$(sh "$root/scripts/ci_next_integration.sh" pull_request next refs/pull/123/merge)" = true ] || fail 'missed a next pull request'
[ "$(sh "$root/scripts/ci_next_integration.sh" push '' refs/heads/next)" = true ] || fail 'missed a next push'
[ "$(sh "$root/scripts/ci_next_integration.sh" pull_request main refs/pull/123/merge)" = false ] || fail 'misrouted a main pull request'
[ "$(sh "$root/scripts/ci_next_integration.sh" push '' refs/heads/main)" = false ] || fail 'misrouted a main push'
if sh "$root/scripts/ci_next_integration.sh" pull_request next >/dev/null 2>&1; then fail 'accepted incomplete branch policy input'; fi
for selected in docs workflow full; do
    for result in failure cancelled skipped success ''; do
        if [[ "$selected:$result" == docs:skipped || "$selected:$result" == workflow:skipped || "$selected:$result" == full:success ]]; then continue; fi
        workflow_result=skipped
        [[ "$selected" != workflow ]] || workflow_result=success
        if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE="$selected" CI_VALIDATION_RESULT="$result" CI_WORKFLOW_RESULT="$workflow_result" sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then
            fail 'accepted failed, cancelled or unexpected execution result'
        fi
    done
done
cd "$root"
bash scripts/check_docs.sh >/dev/null
echo 'CI routing, stable-result guards and documentation checks passed.'
# Only short result gates may be unconditional. Real work must be cancellable.
awk '
/^  (package|test):$/ { gate=1; heavy=0; workflow=0; name=$1; next }
/^  (package_build|next_smoke|test_run):$/ { gate=0; heavy=1; next }
/^  workflow_validation:$/ { gate=0; heavy=0; workflow=1; next }
/^  [a-z_]+:$/ { gate=0; heavy=0; workflow=0 }
gate && name=="package:" && /^    needs: \[changes, package_build, workflow_validation\]$/ { dependencies++ }
gate && name=="test:" && /^    needs: \[changes, test_run, next_smoke, source_policy_tests, workflow_validation\]$/ { dependencies++ }
gate && /^    if: always\(\)$/ { guards++ }
gate && /^    timeout-minutes: 2$/ { bounds++ }
heavy && /^    needs: changes$/ { work_dependencies++ }
heavy && /^    if: / && /needs\.changes\.result == \047success\047/ && /needs\.changes\.outputs\.route == \047full\047/ && /!cancelled\(\)/ { routes++ }
workflow && /^    needs: changes$/ { workflow_dependencies++ }
workflow && /^    if: / && /needs\.changes\.result == \047success\047/ && /needs\.changes\.outputs\.route == \047workflow\047/ && /!cancelled\(\)/ { workflow_routes++ }
heavy && /^    if: always/ { bad=1 }
END { exit bad || dependencies!=2 || guards!=2 || bounds!=2 || work_dependencies!=3 || routes!=3 || workflow_dependencies!=1 || workflow_routes!=1 }
' .github/workflows/check.yml || fail 'stable gates or cancellable work dependencies'
grep -F 'CI_WORKFLOW_RESULT: ${{ needs.workflow_validation.result }}' .github/workflows/check.yml >/dev/null || fail 'missing workflow validation result gate'
grep -F 'run: sh tests/workflow_yaml.sh' .github/workflows/check.yml >/dev/null || fail 'workflow route omitted YAML validation'
grep -F 'run: sh tests/unit/harness/layout.sh' .github/workflows/check.yml >/dev/null || fail 'workflow route omitted workflow contracts'
awk '/^  workflow_validation:$/ { selected=1 } /^  changes:$/ { selected=0 } selected { print }' .github/workflows/check.yml > "$work/workflow-job"
grep -F 'timeout-minutes: 5' "$work/workflow-job" >/dev/null || fail 'workflow validation has no bounded timeout'
test "$(grep -c 'run: sh scripts/ci_gate.sh' .github/workflows/check.yml)" = 1 || fail 'missing test result script'
grep -F 'run: sh scripts/ci_package_gate.sh' .github/workflows/check.yml >/dev/null || fail 'missing package result script'
grep -F 'CI_PACKAGE_BUILD_RESULT: ${{ needs.package_build.result }}' .github/workflows/check.yml >/dev/null || fail 'missing package result'
grep -F 'CI_VALIDATION_RESULT: ${{ needs.test_run.result }}' .github/workflows/check.yml >/dev/null || fail 'missing test result'
grep -F 'CI_NEXT_SMOKE_RESULT: ${{ needs.next_smoke.result }}' .github/workflows/check.yml >/dev/null || fail 'missing next smoke result'
grep -F 'name: Package (${{ matrix.target }})' .github/workflows/check.yml >/dev/null || fail 'package check names changed'
grep -F "needs.changes.outputs.next_integration != 'true' && !cancelled()" .github/workflows/check.yml >/dev/null || fail 'next package matrix was not deferred'
grep -F 'name: Next Linux smoke' .github/workflows/check.yml >/dev/null || fail 'next PR smoke job is missing'
grep -F "needs.changes.outputs.next_integration == 'true' && github.event_name == 'pull_request' && !cancelled()" .github/workflows/check.yml >/dev/null || fail 'next smoke is not limited to next PRs'
! grep -F 'platform_smoke' .github/workflows/check.yml >/dev/null || fail 'macOS smoke remains in Check'
grep -F 'CI_PACKAGE_SMOKE_TARGETS := quick-start' Makefile >/dev/null || fail 'nightly package smoke default changed'
grep -F 'ci-bootstrap: bootstrap-check $(CI_PACKAGE_SMOKE_TARGETS)' Makefile >/dev/null || fail 'bootstrap package smoke is not branch-gated'
grep -F 'ci-conformance-bytecode: ci-conformance-bytecode-check $(CI_PACKAGE_SMOKE_TARGETS)' Makefile >/dev/null || fail 'bytecode package smoke is not branch-gated'
grep -F 'ci-conformance-bytecode-check: native $(STAGE2_COMPILER)' Makefile >/dev/null || fail 'missing package-free bytecode conformance target'
# Inspect Make's expanded dependency graph without running a recipe. Standalone
# suites must prepare the package tool before invoking package-aware CLI tests;
# a warm developer build must not be able to hide a missing prerequisite.
database_status=0
make -qp > "$work/make-database" || database_status=$?
[[ "$database_status" -le 1 ]] || fail 'could not inspect Make prerequisites'
awk '
function reaches(target, wanted, count, parts, i) {
    if (target == wanted) return 1
    if (seen[target]++) return 0
    count = split(edges[target], parts, " ")
    for (i = 1; i <= count; i++) if (reaches(parts[i], wanted)) return 1
    return 0
}
/^[^# \t][^=]*:/ {
    target = $1
    if (target !~ /:$/) next
    sub(/:$/, "", target)
    $1 = ""
    edges[target] = edges[target] " " $0
}
END {
    count = split("check-compiler check-bytecode check-vm unit-harness unit-runtime harness native-oracle-contracts native-check native-coverage ci-compiler ci-runtime ci-conformance-source ci-conformance-bytecode-check", targets, " ")
    for (i = 1; i <= count; i++) {
        for (key in seen) delete seen[key]
        if (!reaches(targets[i], "package-tool")) {
            print "missing package tool prerequisite: " targets[i]
            bad = 1
        }
    }
    for (key in seen) delete seen[key]
    if (reaches("native", "package-tool")) {
        print "native VM build must remain independent of the package tool"
        bad = 1
    }
    exit bad
}' "$work/make-database" || fail 'standalone package CLI preparation'
grep -F 'cron: "17 3 * * *"' .github/workflows/nightly-packages.yml >/dev/null || fail 'missing nightly schedule'
grep -F "ref: \${{ inputs.branch || 'next' }}" .github/workflows/nightly-packages.yml >/dev/null || fail 'nightly does not check out next by default'
grep -F 'suite: [compiler, runtime, bootstrap, conformance-source, conformance-bytecode]' .github/workflows/nightly-packages.yml >/dev/null || fail 'nightly matrix is incomplete'
grep -F 'suite: [compiler, runtime, bootstrap, sanitize-vm, sanitize-oracle, sanitize-runner, coverage]' .github/workflows/nightly-packages.yml >/dev/null || fail 'nightly Linux validation matrix is incomplete'
grep -F 'echo "sha=$(git rev-parse HEAD)"' .github/workflows/nightly-packages.yml >/dev/null || fail 'nightly package provenance does not record checked-out source'
grep -F 'name: panackelty-${{ matrix.target }}-${{ steps.source.outputs.sha }}' .github/workflows/nightly-packages.yml >/dev/null || fail 'nightly package is not identified by source commit'
# Every consumer must use the same selection. No path filter may silently omit
# an unknown/shared dependency, and docs must not enter browser build jobs.
for workflow in check; do
    grep -F 'bash scripts/ci_scope.sh --plan "$CI_BASE_SHA" "$CI_HEAD_SHA"' ".github/workflows/$workflow.yml" >/dev/null || fail "selector missing: $workflow"
    if grep -E '^[[:space:]]+paths(-ignore)?:' ".github/workflows/$workflow.yml" >/dev/null; then fail "independent filter: $workflow"; fi
done
echo 'CI workflow routing and cancellation contracts passed.'
sh tests/ci_partition.sh
sh tests/ci_conformance.sh
