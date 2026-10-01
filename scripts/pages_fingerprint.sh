#!/usr/bin/env bash
# Hash names, modes and blob identities of every website dependency. Unknowns
# participate; deletions, renames and executable/symlink modes change identity.
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
source "$root/scripts/ci_docs.sh"
source "$root/scripts/validation_components.sh"
if command -v sha256sum >/dev/null 2>&1; then hash() { sha256sum; }; else hash() { shasum -a 256; }; fi
git ls-tree -rz --full-tree "${1:-HEAD}" | while IFS= read -r -d '' entry; do
    path=${entry#*$'\t'}
    if [[ $(validation_pages_path "$path") == true ]]; then printf '%s\0' "$entry"; fi
done | hash | cut -d ' ' -f 1
