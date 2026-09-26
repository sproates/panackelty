#!/bin/sh
# Source-tree policy, including untracked files and extensionless scripts.
# Historical documentation and frozen test data are permitted.
set -eu
export LC_ALL=C
root=${1:-.}
test -d "$root" || { echo 'policy: missing source tree' >&2; exit 1; }
list=$(mktemp)
trap 'rm -f "$list"' 0
trap 'exit 1' HUP INT TERM
find "$root" \( -path "$root/.git" -o -path "$root/build" -o -path "$root/output" \) -type d -prune -o \( -type f -o -type l \) -print > "$list"
failed=0
while IFS= read -r file; do
    case "$file" in
        *.py|*.pyw|*.pyc|*.pyo|*.PY|*.PYW)
            printf 'policy: forbidden source or cache: %s\n' "$file" >&2
            failed=1; continue ;;
    esac
    scan=0
    if [ -x "$file" ]; then scan=1; fi
    case "$file" in
        */Makefile|*.mk|*.sh|*.awk|*.panack|*.c|*.h|*/.github/workflows/*.yml|*/.github/workflows/*.yaml) scan=1 ;;
    esac
    if ! awk -v scan="$scan" '
        BEGIN {
            language="py" "thon"
            command="(^|[[:space:];|&\047\042=/({])" language "([0-9]+([.][0-9]+)*)?([[:space:];|&\047\042)}/]|$)"
        }
        NR==1 {
            if ($0 ~ /^#!/) {
                if ($0 ~ command) { bad=1; print FILENAME ":1: forbidden interpreter shebang"; exit }
                scan=1
            }
            if (!scan) exit
        }
        /^[[:space:]]*(#|\/\/)/ { next }
        $0 ~ command || index($0,"setup-" language "@") || $0 ~ /\$[({]?[P][Y][T][H][O][N]([})[:space:]]|$)/ {
            bad=1; print FILENAME ":" FNR ": forbidden interpreter dependency"
        }
        END { exit bad }
    ' "$file" >&2; then failed=1; fi
done < "$list"
test "$failed" = 0 || exit 1
echo 'source policy: passed'
