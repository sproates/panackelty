#!/usr/bin/env bash
# One conservative selector for committed CI changes and local working trees.
set -eu
script_dir=$(cd "$(dirname "$0")" && pwd)
source "$script_dir/ci_docs.sh"
source "$script_dir/validation_components.sh"
format=route
if [[ "${1-}" == --plan ]]; then format=plan; shift; fi
route=docs
components=' '
pages_required=false
add_component() {
    case "$components" in *" $1 "*) ;; *) components="$components$1 " ;; esac
}
emit() {
    if [[ "$format" == route ]]; then
        printf '%s\n' "$route"
    else
        pages=$pages_required
        printf 'route=%s\n' "$route"
        printf 'pages=%s\n' "$pages"
        printf 'components=%s\n' "${components:1:${#components}-2}"
        if [[ "$route" == docs ]]; then
            printf 'checks=documents,links,whitespace\n'
        elif [[ "$route" == website ]]; then
            printf 'checks=documents,links,whitespace,website-automation,release-integrity,browsers\n'
        else
            printf 'checks=documents,links,whitespace,compiler,runtime,tcp,bootstrap,conformance,sanitizers,coverage,packages'
            if [[ "$pages" == true ]]; then printf ',pages'; fi
            printf '\n'
        fi
    fi
}
full() { pages_required=true; route=full; add_component unknown; emit; exit 0; }
valid_revision() {
    [[ "$1" =~ ^([0-9a-fA-F]{40}|[0-9a-fA-F]{64})$ ]] &&
        git cat-file -e "$1^{commit}" 2>/dev/null
}
worktree=0
if [[ "${1-}" == --worktree ]]; then
    [[ $# == 2 ]] || full
    worktree=1
    valid_revision "$2" || full
    base=$(git merge-base "$2" HEAD 2>/dev/null) || full
    head=HEAD
else
    [[ $# == 2 ]] || full
    valid_revision "$1" && valid_revision "$2" || full
    base=$(git merge-base "$1" "$2" 2>/dev/null) || full
    head=$2
fi
files=$(mktemp)
trap 'rm "$files"' EXIT
# Expand renames: old and new paths must both be understood. NUL records retain
# spaces/newlines, and literal pathspecs prevent wildcard filenames hiding modes.
if [[ "$worktree" == 1 ]]; then
    git diff --no-renames --name-only -z "$base" -- > "$files" || full
    git ls-files --others --exclude-standard -z >> "$files" || full
    # Include staged changes even if an unstaged edit cancels their net content.
    git diff --cached --no-renames --name-only -z "$base" -- >> "$files" || full
else
    git diff --no-renames --name-only -z "$base" "$head" -- > "$files" || full
fi
count=0
while IFS= read -r -d '' path; do
    if [[ $(validation_pages_path "$path") == true ]]; then pages_required=true; fi
    component=$(validation_component "$path")
    add_component "$component"
    if validation_website_only_path "$path"; then
        [[ "$route" != docs ]] || route=website
    else
        case "$component" in documentation|process) ;; *) route=full ;; esac
    fi
    for revision in "$base" "$head"; do
        entry=$(git --literal-pathspecs ls-tree "$revision" -- "$path") || full
        mode=${entry%% *}
        if [[ -n "$mode" && "$mode" != 100644 ]]; then route=full; fi
    done
    if [[ "$worktree" == 1 ]]; then
        entry=$(git --literal-pathspecs ls-files --stage -- "$path") || full
        mode=${entry%% *}
        if [[ -n "$mode" && "$mode" != 100644 ]]; then route=full; fi
        if [[ -L "$path" || -x "$path" || ( -e "$path" && ! -f "$path" ) ]]; then route=full; fi
        # Unmerged stages are uncertain even when their modes are regular.
        [[ -z $(git --literal-pathspecs ls-files --unmerged -- "$path") ]] || full
    fi
    count=$((count + 1))
done < "$files"
[[ "$count" -gt 0 ]] || full
emit
