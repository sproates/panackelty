#!/bin/sh
. tests/unit/harness/common.sh
script=$root/scripts/release_request.sh
mkdir "$work/repo"
cd "$work/repo"
printf '0.1.0-alpha.9\n' > VERSION
cat > CHANGELOG.md <<'NOTES'
# Changelog
## 0.1.0-alpha.9 — 2026-09-26

- Reviewed release change.

## 0.1.0-alpha.8 — 2026-09-24
- Excluded old change.
NOTES
commit=0123456789abcdef0123456789abcdef01234567
case_name=release-request-accepts-confirmed-main-and-canonical-tag
sh "$script" workflow_dispatch refs/heads/main "$commit" 0.1.0-alpha.9 "$commit" > "$work/notes"
contains "$work/notes" 'Reviewed release change.'
absent "$work/notes" 'Excluded old change.'
sh "$script" push refs/tags/v0.1.0-alpha.9 "$commit" '' '' > "$work/tag-notes"
equal_files "$work/notes" "$work/tag-notes"
pass
reject() {
    expected=$1
    shift
    if sh "$script" "$@" > "$work/out" 2> "$work/error"; then fail "accepted invalid request: $expected"; fi
    contains "$work/error" "$expected"
}
case_name=release-request-rejects-unconfirmed-or-wrong-source
reject 'manual releases require main' workflow_dispatch refs/heads/feature "$commit" 0.1.0-alpha.9 "$commit"
reject 'manual releases require main' workflow_dispatch refs/tags/v0.1.0-alpha.9 "$commit" 0.1.0-alpha.9 "$commit"
reject 'version confirmation mismatch' workflow_dispatch refs/heads/main "$commit" 0.1.0-alpha.8 "$commit"
reject 'version confirmation mismatch' workflow_dispatch refs/heads/main "$commit" '' "$commit"
reject 'commit confirmation mismatch' workflow_dispatch refs/heads/main "$commit" 0.1.0-alpha.9 0123456
reject 'commit confirmation mismatch' workflow_dispatch refs/heads/main "$commit" 0.1.0-alpha.9 1123456789abcdef0123456789abcdef01234567
reject 'invalid source commit' workflow_dispatch refs/heads/main short 0.1.0-alpha.9 short
reject 'noncanonical version tag' push refs/heads/main "$commit" '' ''
reject 'noncanonical version tag' push refs/tags/v0.1.0-alpha.8 "$commit" '' ''
reject 'unsupported event' pull_request refs/heads/main "$commit" '' ''
reject 'expected event' workflow_dispatch
pass
case_name=release-request-rejects-invalid-version-and-notes
printf 'invalid\n' > VERSION
reject 'invalid preview version' push refs/tags/vinvalid "$commit" '' ''
printf '0.1.0-alpha.9\ninjected\n' > VERSION
reject 'invalid preview version' push refs/tags/v0.1.0-alpha.9 "$commit" '' ''
printf '0.1.0-alpha.9\n' > VERSION
printf '## 0.1.0-alpha.8\n- Wrong version\n' > CHANGELOG.md
reject 'changelog section' push refs/tags/v0.1.0-alpha.9 "$commit" '' ''
printf '## 0.1.0-alpha.9\n\n' > CHANGELOG.md
reject 'changelog section' push refs/tags/v0.1.0-alpha.9 "$commit" '' ''
printf '## 0.1.0-alpha.9\n- First\n## 0.1.0-alpha.9\n- Duplicate\n' > CHANGELOG.md
reject 'changelog section' push refs/tags/v0.1.0-alpha.9 "$commit" '' ''
pass
# Execute the actual publication block against a local Git remote. Only the
# GitHub CLI boundary is stubbed; tag creation, fetch, push and peeling are real.
case_name=release-publication-tag-safety
mkdir "$work/bin" "$work/run-temp"
cat > "$work/bin/gh" <<'STUB'
#!/bin/sh
set -eu
case "$*" in
    'auth setup-git') exit 0 ;;
    'release create '*) printf '%s\n' "$*" >> "$RELEASE_CALLS" ;;
    *) echo 'unexpected GitHub CLI operation' >&2; exit 1 ;;
esac
STUB
chmod +x "$work/bin/gh"
PATH="$work/bin:$PATH"
RELEASE_CALLS=$work/release-calls
RUNNER_TEMP=$work/run-temp
export PATH RELEASE_CALLS RUNNER_TEMP
awk '
 /- name: Create or verify the annotated tag and publish/ { selected=1 }
 selected && /        run: \|/ { body=1; next }
 body { print substr($0,11) }
' "$root/.github/workflows/release.yml" > "$work/publish.sh"
test -s "$work/publish.sh" || fail 'missing publication block'
printf '## 0.1.0-alpha.9\n- Reviewed release change.\n' > CHANGELOG.md
mkdir scripts
cp "$script" scripts/release_request.sh
git init -q -b main
git config user.name sproates
git config user.email 218187+sproates@users.noreply.github.com
test "$(git var GIT_AUTHOR_IDENT | cut -d '>' -f 1)" = 'sproates <218187+sproates@users.noreply.github.com' || fail 'fixture identity'
git add VERSION CHANGELOG.md scripts
git commit -qm 'Release test fixture'
git init -q --bare "$work/remote.git"
git remote add origin "$work/remote.git"
git push -q origin main
GITHUB_SHA=$(git rev-parse HEAD)
GITHUB_REF=refs/heads/main
GITHUB_EVENT_NAME=workflow_dispatch
REQUESTED_COMMIT=$GITHUB_SHA
REQUESTED_VERSION=0.1.0-alpha.9
export GITHUB_SHA GITHUB_REF GITHUB_EVENT_NAME REQUESTED_COMMIT REQUESTED_VERSION
bash -e "$work/publish.sh" > "$work/publish-log" 2>&1 || { cat "$work/publish-log"; fail 'new tag publication'; }
test "$(git --git-dir="$work/remote.git" cat-file -t refs/tags/v0.1.0-alpha.9)" = tag || fail 'not annotated'
test "$(git --git-dir="$work/remote.git" rev-parse 'refs/tags/v0.1.0-alpha.9^{}')" = "$GITHUB_SHA" || fail 'wrong tagged commit'
contains "$RELEASE_CALLS" 'release create v0.1.0-alpha.9'
tag_object=$(git rev-parse refs/tags/v0.1.0-alpha.9)
# A retry must preserve the original annotated object.
bash -e "$work/publish.sh" > "$work/publish-log" 2>&1 || fail 'matching-tag retry'
test "$(git rev-parse refs/tags/v0.1.0-alpha.9)" = "$tag_object" || fail 'rewrote tag'
# Corrupt only these disposable fixture repositories, never the project tags.
git tag -d v0.1.0-alpha.9 >/dev/null
git --git-dir="$work/remote.git" update-ref -d refs/tags/v0.1.0-alpha.9
git tag v0.1.0-alpha.9
git push -q origin refs/tags/v0.1.0-alpha.9
before=$(wc -l < "$RELEASE_CALLS")
if bash -e "$work/publish.sh" > "$work/publish-log" 2>&1; then fail 'accepted lightweight tag'; fi
test "$(wc -l < "$RELEASE_CALLS")" = "$before" || fail 'published after tag rejection'
git tag -d v0.1.0-alpha.9 >/dev/null
git --git-dir="$work/remote.git" update-ref -d refs/tags/v0.1.0-alpha.9
printf 'new source\n' > changed
git add changed
git commit -qm 'Other source'
git tag -a v0.1.0-alpha.9 -m 'Wrong commit'
git push -q origin refs/tags/v0.1.0-alpha.9
git checkout -q --detach "$GITHUB_SHA"
if bash -e "$work/publish.sh" > "$work/publish-log" 2>&1; then fail 'accepted wrong-commit tag'; fi
test "$(wc -l < "$RELEASE_CALLS")" = "$before" || fail 'published wrong commit'
pass
