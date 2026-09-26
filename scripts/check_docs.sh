#!/usr/bin/env bash
# Lightweight repository-local checks; no compiler, network or package install.
set -eu
script_dir=$(cd "$(dirname "$0")" && pwd)
source "$script_dir/ci_docs.sh"
cd "$(git rev-parse --show-toplevel)"
normalize() {
    DOC_LINK_PATH=$1 awk 'BEGIN {
        n=split(ENVIRON["DOC_LINK_PATH"], parts, "/"); count=0
        for (i=1;i<=n;i++) {
            if (parts[i]=="" || parts[i]==".") continue
            if (parts[i]=="..") { if (!count) exit 1; count--; continue }
            stack[++count]=parts[i]
        }
        for (i=1;i<=count;i++) printf "%s%s", (i==1?"":"/"), stack[i]
        printf "\n"
    }'
}
documents=$(mktemp)
links=$(mktemp)
text_copy=$(mktemp)
trap 'rm "$documents" "$links" "$text_copy"' EXIT
git ls-files -z -- '*.md' > "$documents"

failed=0
while IFS= read -r -d '' document; do
    selected=0
    if ci_informational_doc "$document"; then
        selected=1
        if [[ ! -s "$document" || -L "$document" ]]; then
            printf 'docs: missing, empty or symlink document: %s\n' "$document" >&2
            failed=1; continue
        fi
        LC_ALL=C tr -d '\000' < "$document" > "$text_copy"
        if ! cmp -s "$document" "$text_copy"; then
            printf 'docs: NUL byte in %s\n' "$document" >&2
            failed=1; continue
        fi
        if grep -nE '^(<<<<<<< |=======|>>>>>>> )' "$document" >&2; then
            printf 'docs: conflict marker in %s\n' "$document" >&2
            failed=1
        fi
    fi
    [[ -f "$document" && ! -L "$document" ]] || continue
    awk -v strict="$selected" -f "$script_dir/doc_links.awk" "$document" > "$links"
    while IFS= read -r target; do
        case "$target" in
            \#*|//*) continue ;;
        esac
        if [[ "$target" =~ ^[a-zA-Z][a-zA-Z0-9+.-]*: ]]; then continue; fi
        target=${target%%#*}
        target=${target%%\?*}
        target=${target//%20/ }
        target=${target//%28/(}
        target=${target//%29/)}
        [[ -n "$target" ]] || continue
        if [[ "$target" == /* ]]; then
            resolved=${target#/}
        else
            resolved="${document%/*}/$target"
            [[ "$document" == */* ]] || resolved=$target
        fi
        if ! resolved=$(normalize "$resolved"); then
            if [[ "$selected" == 1 ]]; then
                printf 'docs: link escapes repository: %s -> %s\n' "$document" "$target" >&2
                failed=1
            fi
            continue
        fi
        # Also check incoming links to informational documents, so deletion or
        # rename cannot leave an unchanged README pointing at a missing file.
        if [[ "$selected" == 1 ]] || ci_informational_doc "$resolved"; then
            if [[ ! -e "$resolved" ]]; then
                printf 'docs: missing local link: %s -> %s\n' "$document" "$target" >&2
                failed=1
            fi
        fi
    done < "$links"
done < "$documents"
[[ "$failed" == 0 ]] || exit 1
printf 'Informational documentation and local file links passed.\n'
