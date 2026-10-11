# Explicitly informational files. README.md is eligible only when its changes
# are confined to the marked positioning-copy region; other README edits retain
# package validation.
ci_informational_doc() {
    case "$1" in
        README.md|ROADMAP.md|ARCHITECTURE.md|SELF_HOSTING.md|tests/README.md|tests/COVERAGE.md|tests/VALIDATION_PROFILE.md|CONTRIBUTING.md|docs/ROADMAP_PROCESS.md|.github/pull_request_template.md) return 0 ;;
        *) return 1 ;;
    esac
}

# Strip only the explicitly bounded positioning copy. Historical/unmarked
# versions use the first executable README section as the same boundary, which
# lets the marker-only migration itself take the documentation route.
ci_readme_without_positioning_copy() {
    awk '
        BEGIN {
            begin = "<!-- validation:positioning-copy-begin -->"
            end = "<!-- validation:positioning-copy-end -->"
            boundary = "## Exact fractions and Unit"
        }
        {
            lines[NR] = $0
            if ($0 == begin) { begins++; begin_line = NR }
            if ($0 == end) { ends++; end_line = NR }
            if ($0 == boundary) { boundaries++; boundary_line = NR }
        }
        END {
            if (boundaries != 1) exit 2
            if (begins == 1 && ends == 1 && begin_line < end_line && end_line < boundary_line) {
                for (i = 1; i <= NR; i++)
                    if (i < begin_line || i > end_line) print lines[i]
                exit 0
            }
            if (begins == 0 && ends == 0) {
                for (i = boundary_line; i <= NR; i++) print lines[i]
                exit 0
            }
            exit 2
        }
    '
}

# Arguments: base revision, head revision, and optional `worktree` mode. For
# local selection compare the base with both the index and working-tree file so
# an unsafe staged edit cannot be hidden by a later unstaged reversal.
ci_readme_positioning_only() {
    local base=$1 head=$2 mode=${3-commit} temporary current
    temporary=$(mktemp -d) || return 1
    if ! git show "$base:README.md" > "$temporary/base" 2>/dev/null ||
       ! ci_readme_without_positioning_copy < "$temporary/base" > "$temporary/base.rest"; then
        rm -rf "$temporary"
        return 1
    fi
    if [[ "$mode" == worktree ]]; then
        if ! git show :README.md > "$temporary/index" 2>/dev/null ||
           ! cp README.md "$temporary/worktree"; then
            rm -rf "$temporary"
            return 1
        fi
        for current in index worktree; do
            if ! ci_readme_without_positioning_copy < "$temporary/$current" > "$temporary/$current.rest" ||
               ! cmp -s "$temporary/base.rest" "$temporary/$current.rest"; then
                rm -rf "$temporary"
                return 1
            fi
        done
    else
        if ! git show "$head:README.md" > "$temporary/head" 2>/dev/null ||
           ! ci_readme_without_positioning_copy < "$temporary/head" > "$temporary/head.rest" ||
           ! cmp -s "$temporary/base.rest" "$temporary/head.rest"; then
            rm -rf "$temporary"
            return 1
        fi
    fi
    rm -rf "$temporary"
    return 0
}
