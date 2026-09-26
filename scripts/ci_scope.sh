#!/usr/bin/env bash
# Print docs only for a completely understood informational change.
set -eu
source "$(dirname "$0")/ci_docs.sh"
full() { printf 'full\n'; exit 0; }
[[ $# == 2 ]] || full
for revision in "$@"; do
    [[ "$revision" =~ ^([0-9a-fA-F]{40}|[0-9a-fA-F]{64})$ ]] || full
    git cat-file -e "$revision^{commit}" 2>/dev/null || full
done
base=$(git merge-base "$1" "$2" 2>/dev/null) || full
files=$(mktemp)
trap 'rm "$files"' EXIT
# Renames are deliberately expanded into old-path deletion/new-path addition.
git diff --no-renames --name-only -z "$base" "$2" -- > "$files" || full
count=0
while IFS= read -r -d '' path; do
    ci_informational_doc "$path" || full
    for revision in "$base" "$2"; do
        entry=$(git ls-tree "$revision" -- "$path") || full
        mode=${entry%% *}
        # Absence is valid on one side of an addition/deletion, but symlinks,
        # executable documents and submodules must never enter the fast path.
        [[ -z "$mode" || "$mode" == 100644 ]] || full
    done
    count=$((count + 1))
done < "$files"
[[ "$count" -gt 0 ]] || full
printf 'docs\n'
