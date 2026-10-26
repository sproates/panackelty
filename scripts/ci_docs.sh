# Explicitly informational files. README.md is eligible only when its changes
# are confined to a marked positioning or coverage-publication block; other
# README edits retain package validation.
ci_informational_doc() {
    case "$1" in
        docs/reference/SPEC.md|docs/guides/*.md) return 1 ;;
        README.md|ROADMAP.md|ARCHITECTURE.md|SELF_HOSTING.md|RELEASE_POLICY.md|docs/internals/ARCHITECTURE.md|docs/internals/SELF_HOSTING.md|docs/releases/RELEASE_POLICY.md|docs/*.md|tests/README.md|tests/COVERAGE.md|tests/VALIDATION_PROFILE.md|CONTRIBUTING.md|docs/roadmap/ROADMAP_PROCESS.md|.github/pull_request_template.md) return 0 ;;
        *) return 1 ;;
    esac
}

# Strip only the explicitly bounded informational blocks. Historical/unmarked
# versions use the first executable README section as the positioning boundary.
ci_readme_without_informational_copy() {
    awk '
        BEGIN {
            begin = "<!-- validation:positioning-copy-begin -->"
            end = "<!-- validation:positioning-copy-end -->"
            coverage_begin = "<!-- validation:coverage-publication-copy-begin -->"
            coverage_end = "<!-- validation:coverage-publication-copy-end -->"
            coverage_legacy_begin = "Browse the [public native VM coverage report]"
            coverage_legacy_end = "Direct bytecode/verification coverage runs in"
            boundary = "## Exact fractions and Unit"
        }
        {
            lines[NR] = $0
            if ($0 == begin) { begins++; begin_line = NR }
            if ($0 == end) { ends++; end_line = NR }
            if ($0 == coverage_begin) { coverage_begins++; coverage_begin_line = NR }
            if ($0 == coverage_end) { coverage_ends++; coverage_end_line = NR }
            if (index($0, coverage_legacy_begin) == 1) { legacy_coverage_begins++; legacy_coverage_begin_line = NR }
            if (index($0, coverage_legacy_end) == 1) { legacy_coverage_ends++; legacy_coverage_end_line = NR }
            if ($0 == boundary || $0 == "## Get started") { boundaries++; boundary_line = NR }
        }
        END {
            if (boundaries != 1) exit 2
            if (coverage_begins > 1 || coverage_ends > 1 || coverage_begins != coverage_ends ||
                (coverage_begins == 1 && coverage_begin_line >= coverage_end_line)) exit 2
            if (coverage_begins == 0 && (legacy_coverage_begins != legacy_coverage_ends ||
                legacy_coverage_begins > 1 ||
                (legacy_coverage_begins == 1 && legacy_coverage_begin_line >= legacy_coverage_end_line))) exit 2
            if (coverage_begins == 0 && legacy_coverage_begins == 1) {
                coverage_begin_line = legacy_coverage_begin_line - 1
                coverage_end_line = legacy_coverage_end_line - 1
            }
            if (begins == 1 && ends == 1 && begin_line < end_line && end_line < boundary_line) {
                for (i = 1; i <= NR; i++)
                    if ((i < begin_line || i > end_line) &&
                        !((coverage_begins == 1 || legacy_coverage_begins == 1) &&
                          i >= coverage_begin_line && i <= coverage_end_line)) print lines[i]
                exit 0
            }
            if (begins == 0 && ends == 0) {
                for (i = boundary_line; i <= NR; i++)
                    if (!((coverage_begins == 1 || legacy_coverage_begins == 1) &&
                          i >= coverage_begin_line && i <= coverage_end_line)) print lines[i]
                exit 0
            }
            exit 2
        }
    '
}

# Arguments: base revision, head revision, and optional `worktree` mode. For
# local selection compare the base with both the index and working-tree file so
# an unsafe staged edit cannot be hidden by a later unstaged reversal.
ci_readme_informational_only() {
    local base=$1 head=$2 mode=${3-commit} temporary current
    temporary=$(mktemp -d) || return 1
    if ! git show "$base:README.md" > "$temporary/base" 2>/dev/null ||
       ! ci_readme_without_informational_copy < "$temporary/base" > "$temporary/base.rest"; then
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
            if ! ci_readme_without_informational_copy < "$temporary/$current" > "$temporary/$current.rest" ||
               ! cmp -s "$temporary/base.rest" "$temporary/$current.rest"; then
                rm -rf "$temporary"
                return 1
            fi
        done
    else
        if ! git show "$head:README.md" > "$temporary/head" 2>/dev/null ||
           ! ci_readme_without_informational_copy < "$temporary/head" > "$temporary/head.rest" ||
           ! cmp -s "$temporary/base.rest" "$temporary/head.rest"; then
            rm -rf "$temporary"
            return 1
        fi
    fi
    rm -rf "$temporary"
    return 0
}
