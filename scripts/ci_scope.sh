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
workflow_change=0
ci_static_validation_path() {
    case "$1" in
        .github/workflows/*.yml|.github/workflows/*.yaml|\
        scripts/ci_scope.sh|scripts/validate_change.sh|scripts/ci_gate.sh|scripts/ci_package_gate.sh|\
        scripts/ci_next_integration.sh|scripts/source_coverage_route.cjs|\
        scripts/validation_components.sh|scripts/ci_docs.sh|scripts/check_docs.sh|\
        tests/ci_scope.sh|tests/ci_partition.sh|tests/ci_conformance.sh|\
        tests/source_coverage/route.test.cjs|tests/unit/harness/layout.sh|tests/workflow_yaml.sh)
            return 0 ;;
        *) return 1 ;;
    esac
}
add_component() {
    case "$components" in *" $1 "*) ;; *) components="$components$1 " ;; esac
}
emit() {
    if [[ "$format" == route ]]; then
        printf '%s\n' "$route"
    else
        printf 'route=%s\n' "$route"
        printf 'components=%s\n' "${components:1:${#components}-2}"
        if [[ "$route" == docs ]]; then
            printf 'checks=documents,links,whitespace\n'
        elif [[ "$route" == workflow ]]; then
            printf 'checks=workflow-syntax,workflow-contracts,documents,links,whitespace\n'
        else
            printf 'checks=documents,links,whitespace,compiler,runtime,tcp,bootstrap,conformance,sanitizers,coverage,packages'
            printf '\n'
        fi
    fi
}
full() { route=full; add_component unknown; emit; exit 0; }
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
position_mode=commit
[[ "$worktree" == 1 ]] && position_mode=worktree
while IFS= read -r -d '' path; do
    if [[ "$path" == README.md ]] && ci_readme_positioning_only "$base" "$head" "$position_mode"; then
        component=documentation
    elif ci_static_validation_path "$path"; then
        # CI definitions and their narrow routing-contract suite do not change
        # product inputs. Their dedicated checks still validate the complete
        # CI contract; any mixed product/shared input keeps full validation.
        component=workflow
        workflow_change=1
    else
        component=$(validation_component "$path")
    fi
    add_component "$component"
    case "$component" in documentation|process|workflow) ;; *) route=full ;; esac
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
if [[ "$route" == docs && "$workflow_change" == 1 ]]; then route=workflow; fi
emit
