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
    printf '# Readme\n\n[Roadmap](ROADMAP.md)\n' > README.md
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
# The complete PR diff matters, not just its latest docs commit.
printf '\n/* implementation */\n' >> src/main.c
commit
printf '\nMore notes\n' >> ROADMAP.md
commit; route full
fixture
printf '# Bootstrap\n' > SELF_HOSTING.md
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
for path in README.md SPEC.md VERSION CHANGELOG.md RELEASE_POLICY.md \
    .github/workflows/check.yml tests/functional/input.md docs/new.md \
    'notes with spaces.md' $'notes\nnewline.md'; do
    fixture
    mkdir -p "$(dirname "$path")"
    printf '\nchange\n' >> "$path"
    commit; route full
done
# Reviewed process prose is not consumed by builds, packages or fixtures.
for path in AGENTS.md CONTRIBUTING.md docs/ROADMAP_PROCESS.md .agents/skills/next-item/SKILL.md .github/pull_request_template.md; do
    fixture
    mkdir -p "$(dirname "$path")"
    printf '# Process\n' > "$path"
    commit; route docs
    plan=$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")
    [[ "$plan" == $'route=docs\npages=false\ncomponents=process\nchecks=documents,links,whitespace' ]] || fail 'process plan'
done
fixture
mkdir -p .github docs tests
for path in .github/pull_request_template.md AGENTS.md ROADMAP.md docs/ROADMAP_PROCESS.md tests/COVERAGE.md; do
    printf '\nProcess handover update\n' >> "$path"
done
commit; route docs
# Expected ownership examples are deliberately independent of classifier patterns.
for path in site/index.html site/styles.css site/favicon.svg site/playground.json; do
    fixture
    mkdir site; printf 'asset\n' > "$path"
    commit; route website
    plan=$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")
    [[ "$plan" == *'pages=true'* && "$plan" == *'website-automation,release-integrity,browsers' ]] || fail 'website checks missing'
    printf '\nNotes\n' >> ROADMAP.md
    commit; route website
    chmod +x "$path"
    commit; route full
done
fixture
mkdir site; printf 'asset\n' > site/index.html
commit; base=$head
git rm -q site/index.html; commit; route website
fixture
mkdir site; ln -s ../README.md site/index.html
commit; route full
fixture
mkdir site; printf 'asset\n' > site/styles.css
commit; base=$head
git mv site/styles.css site/unreviewed.css; commit; route full
fixture
mkdir site; printf 'asset\n' > site/index.html
printf 'native change\n' >> src/main.c
commit; route full
fixture
mkdir site; printf 'asset\n' > site/index.html
[[ $(bash "$root/scripts/ci_scope.sh" --worktree "$base") == website ]] || fail 'untracked website input'
git add site/index.html; chmod +x site/index.html
[[ $(bash "$root/scripts/ci_scope.sh" --worktree "$base") == full ]] || fail 'unstaged executable website input'

# Expected ownership examples are deliberately independent of classifier patterns.
while read -r path component; do
    fixture
    mkdir -p "$(dirname "$path")"
    printf 'changed\n' > "$path"
    commit; route full
    plan=$(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head")
    [[ "$plan" == *"components=$component"* ]] || fail "component: $path"
    case "$path" in src/compiler/*|src/bytecode/*|src/vm/*|src/runtime/*|src/stdlib/*|tests/tcp*|examples/*|SPEC.md|bootstrap/*) pages=false ;; *) pages=true ;; esac
    [[ "$plan" == *"pages=$pages"* ]] || fail "website dependency: $path"
    checks=compiler,runtime,tcp,bootstrap,conformance,sanitizers,coverage,packages
    if [[ "$pages" == true ]]; then checks=$checks,pages; fi
    [[ "$plan" == *"$checks" ]] || fail "lost integrations: $path"
done <<'CASES'
src/compiler/checker.panack compiler
src/bytecode/reader.panack bytecode
src/vm/vm.c runtime
src/runtime/host.panack runtime
src/vm/host_tcp.c tcp
tests/tcp_serve.sh tcp
src/stdlib/array.panack stdlib
scripts/fetch_playground.cjs website
site/extra.js website
VERSION package
examples/hello.panack examples
SPEC.md shared
bootstrap/compiler-v9.bc shared
Makefile shared
.github/workflows/check.yml shared
unrecognised/input.data unknown
CASES
# Compiler delivery (source, seed, harness, fixtures and documentation) does
# not assemble a pinned downstream website. Actual browser pins still do.
fixture
for path in src/compiler/checker.panack bootstrap/compiler-v9.bc tests/unit/harness/runner.sh tests/runner/main.panack SPEC.md; do
    mkdir -p "$(dirname "$path")"; printf 'change\n' >> "$path"
done
commit
[[ $(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head") == *'pages=false'* ]] || fail 'compiler delivery entered Pages'
# Fingerprints use the same dependency boundary in publication and PR selection.
mkdir -p scripts
cp "$root"/scripts/{ci_docs.sh,validation_components.sh,pages_fingerprint.sh} scripts/
commit; base=$head
first=$(bash scripts/pages_fingerprint.sh)
printf 'compiler edit\n' >> src/compiler/checker.panack
commit
[[ $(bash scripts/pages_fingerprint.sh) == "$first" ]] || fail 'native-only fingerprint changed'
for path in site/playground.json tests/pages.test.cjs scripts/attach_coverage.sh .github/workflows/pages.yml unknown.data; do
    before=$(bash scripts/pages_fingerprint.sh)
    mkdir -p "$(dirname "$path")"; printf 'input\n' > "$path"
    commit
    [[ $(bash scripts/pages_fingerprint.sh) != "$before" ]] || fail "fingerprint ignored $path"
done
before=$(bash scripts/pages_fingerprint.sh)
git rm -q site/playground.json; commit
[[ $(bash scripts/pages_fingerprint.sh) != "$before" ]] || fail 'fingerprint ignored deletion'
# Mixed inputs cannot hide website/shared changes behind native-only changes.
fixture
mkdir -p src/vm site
printf 'code\n' > src/vm/vm.c
printf 'html\n' > site/index.html
commit
[[ $(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head") == *'pages=true'* ]] || fail 'mixed website input'
base=$head
git mv site/index.html src/vm/old.c
commit
[[ $(bash "$root/scripts/ci_scope.sh" --plan "$base" "$head") == *'pages=true'* ]] || fail 'renamed website input'
[[ $(bash "$root/scripts/ci_scope.sh" --plan missing "$head") == *'pages=true'* ]] || fail 'unknown revision omitted website'
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
export CI_WEBSITE_REQUIRED=false CI_WEBSITE_RESULT=skipped
for result in failure cancelled skipped ''; do
    for selected in docs website full ''; do
        if CI_SCOPE_RESULT="$result" CI_SCOPE_ROUTE="$selected" CI_VALIDATION_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted failed classification'; fi
    done
done
for selected in '' invalid; do
    if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE="$selected" CI_VALIDATION_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted invalid route'; fi
done
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=docs CI_VALIDATION_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_VALIDATION_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=website CI_VALIDATION_RESULT=skipped CI_WEBSITE_REQUIRED=true CI_WEBSITE_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null
for native in success failure skipped cancelled ''; do
    for website in success failure skipped cancelled ''; do
        [[ "$native:$website" != skipped:success ]] || continue
        if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=website CI_VALIDATION_RESULT="$native" CI_WEBSITE_REQUIRED=true CI_WEBSITE_RESULT="$website" sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then
            fail 'website route accepted missing/failed tests or inconsistent native result'
        fi
    done
done
for required in true false '' invalid; do
    for native in success failure skipped cancelled ''; do
        for website in success failure skipped cancelled ''; do
            expected=failure
            if [[ "$native" == success && ( "$required:$website" == true:success || "$required:$website" == false:skipped ) ]]; then expected=success; fi
            actual=failure
            if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_VALIDATION_RESULT="$native" CI_WEBSITE_REQUIRED="$required" CI_WEBSITE_RESULT="$website" sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then actual=success; fi
            [[ "$actual" == "$expected" ]] || fail "full gate: applicability=$required native=$native website=$website"
        done
    done
done
for selected in docs full; do
    for result in failure cancelled skipped success ''; do
        if [[ "$selected:$result" == docs:skipped || "$selected:$result" == full:success ]]; then continue; fi
        if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE="$selected" CI_VALIDATION_RESULT="$result" sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then
            fail 'accepted failed, cancelled or unexpected execution result'
        fi
    done
done
cd "$root"
bash scripts/check_docs.sh >/dev/null
echo 'CI routing, stable-result guards and documentation checks passed.'
# Only short result gates may be unconditional. Real work must be cancellable.
awk '
/^  (package|test):$/ { gate=1; heavy=0; next }
/^  (package_build|test_run):$/ { gate=0; heavy=1; next }
/^  [a-z_]+:$/ { gate=0; heavy=0 }
gate && /^    needs: \[changes, (package_build|test_run), website\]$/ { dependencies++ }
gate && /^    if: always\(\)$/ { guards++ }
gate && /^    timeout-minutes: 2$/ { bounds++ }
heavy && /^    needs: changes$/ { work_dependencies++ }
heavy && $0=="    if: needs.changes.result == \047success\047 && needs.changes.outputs.route == \047full\047 && !cancelled()" { routes++ }
heavy && /^    if: always/ { bad=1 }
END { exit bad || dependencies!=2 || guards!=2 || bounds!=2 || work_dependencies!=2 || routes!=2 }
' .github/workflows/check.yml || fail 'stable gates or cancellable work dependencies'
test "$(grep -c 'run: sh scripts/ci_gate.sh' .github/workflows/check.yml)" = 2 || fail 'stable result scripts'
grep -F 'CI_VALIDATION_RESULT: ${{ needs.package_build.result }}' .github/workflows/check.yml >/dev/null || fail 'missing package result'
grep -F 'CI_VALIDATION_RESULT: ${{ needs.test_run.result }}' .github/workflows/check.yml >/dev/null || fail 'missing test result'
grep -F 'name: Package (${{ matrix.target }})' .github/workflows/check.yml >/dev/null || fail 'package check names changed'
# Every consumer must use the same selection. No path filter may silently omit
# an unknown/shared dependency, and docs must not enter browser build jobs.
for workflow in check pages; do
    grep -F 'bash scripts/ci_scope.sh --plan "$CI_BASE_SHA" "$CI_HEAD_SHA"' ".github/workflows/$workflow.yml" >/dev/null || fail "selector missing: $workflow"
    if grep -E '^[[:space:]]+paths(-ignore)?:' ".github/workflows/$workflow.yml" >/dev/null; then fail "independent filter: $workflow"; fi
done
grep -F "steps.scope.outputs.pages == 'true'" .github/workflows/pages.yml >/dev/null || fail 'docs PR enters Pages build'
echo 'CI workflow routing and cancellation contracts passed.'
sh tests/ci_partition.sh
sh tests/ci_conformance.sh
