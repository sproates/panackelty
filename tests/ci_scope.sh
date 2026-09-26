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
for path in README.md SPEC.md AGENTS.md VERSION CHANGELOG.md RELEASE_POLICY.md \
    .github/workflows/check.yml tests/functional/input.md docs/new.md \
    'notes with spaces.md' $'notes\nnewline.md'; do
    fixture
    mkdir -p "$(dirname "$path")"
    printf '\nchange\n' >> "$path"
    commit; route full
done
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
    for selected in docs full ''; do
        if CI_SCOPE_RESULT="$result" CI_SCOPE_ROUTE="$selected" CI_VALIDATION_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted failed classification'; fi
    done
done
for selected in '' invalid; do
    if CI_SCOPE_RESULT=success CI_SCOPE_ROUTE="$selected" CI_VALIDATION_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null 2>&1; then fail 'accepted invalid route'; fi
done
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=docs CI_VALIDATION_RESULT=skipped sh "$root/scripts/ci_gate.sh" >/dev/null
CI_SCOPE_RESULT=success CI_SCOPE_ROUTE=full CI_VALIDATION_RESULT=success sh "$root/scripts/ci_gate.sh" >/dev/null
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
gate && /^    needs: \[changes, (package_build|test_run)\]$/ { dependencies++ }
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
echo 'CI workflow routing and cancellation contracts passed.'
