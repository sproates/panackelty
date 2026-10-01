#!/usr/bin/env bash
# Local entry point. Planning is read-only; --run executes the selected route.
set -eu
[[ $# == 2 && ( "$1" == --plan || "$1" == --run ) ]] || {
    echo 'usage: bash scripts/validate_change.sh --plan|--run BASE_REF' >&2
    exit 2
}
cd "$(git rev-parse --show-toplevel)"
# Resolve user-friendly refs here; the underlying CI selector accepts only SHAs.
base=$(git rev-parse --verify --end-of-options "$2^{commit}")
plan=$(bash scripts/ci_scope.sh --plan --worktree "$base")
printf '%s\n' "$plan"
[[ "$1" == --run ]] || exit 0
merge_base=$(git merge-base "$base" HEAD)
git diff --check "$merge_base" --
git diff --cached --check "$merge_base" --
route=${plan%%$'\n'*}
case "$route" in
    route=docs) make docs ;;
    route=website)
        echo 'Running website automation locally; CI additionally verifies the pinned release and all browser scenarios.'
        make docs
        node --test tests/pages.test.cjs tests/playground_release.test.cjs tests/preview.test.cjs
        sh tests/pages.sh ;;
    route=full)
        echo 'Running canonical local validation; CI additionally runs platform, instrumentation and browser gates.'
        make docs
        make check ;;
    *) echo 'invalid validation plan' >&2; exit 1 ;;
esac
