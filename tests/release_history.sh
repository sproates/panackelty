#!/bin/sh
# Native-tool regression: generated notes must follow canonical source and pins.
set -eu
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-history.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
mkdir "$work/site"
cp site/native-release.txt site/playground.json "$work/site/"
cp CHANGELOG.md "$work/CHANGELOG.md"
render() { sh "$root/scripts/release_history.sh" "$work"; }
render > "$work/first"
grep -q '<details class="release-pending">' "$work/first"
grep -q 'releases/tag/v0.1.0-alpha.10' "$work/first"
grep -q '0.1.0-alpha.1</h2>' "$work/first"
! grep -q 'releases/tag/vUnreleased' "$work/first"
# Prepared version does not become downloadable just because notes were merged.
awk '/^## 0.1.0-alpha.10 / { print "## 0.1.0-alpha.11 — 2026-10-04\n\n- Prepared only.\n" } { print }' CHANGELOG.md > "$work/CHANGELOG.md"
render > "$work/prepared"
grep -q 'Prepared notes' "$work/prepared"
! grep -q 'releases/tag/v0.1.0-alpha.11' "$work/prepared"
# Changes in canonical notes are reflected immediately; markup is escaped.
awk '/^## Unreleased/ { print; print "\n- New <script> & `safe` text.\n"; next } { print }' CHANGELOG.md > "$work/CHANGELOG.md"
render > "$work/second"
! cmp -s "$work/first" "$work/second"
grep -q 'New &lt;script&gt; &amp; <code>safe</code>' "$work/second"
for problem in duplicate empty heading inline markdown date missing; do
    cp CHANGELOG.md "$work/CHANGELOG.md"
    case "$problem" in
      duplicate) printf '\n## Unreleased\n- Duplicate.\n' >> "$work/CHANGELOG.md" ;;
      empty) printf '\n## 0.1.0-alpha.0 — 2026-09-01\n' >> "$work/CHANGELOG.md" ;;
      heading) printf '\n#### Unsupported\n' >> "$work/CHANGELOG.md" ;;
      inline) printf '\n- Unclosed `code\n' >> "$work/CHANGELOG.md" ;;
      markdown) printf '\n- Unsupported **emphasis**\n' >> "$work/CHANGELOG.md" ;;
      date) sed 's/2026-09-30/2026-02-30/' CHANGELOG.md > "$work/CHANGELOG.md" ;;
      missing) printf '0.1.0-alpha.99\n' > "$work/site/native-release.txt" ;;
    esac
    if render > "$work/bad" 2>/dev/null; then echo "Accepted $problem changelog" >&2; exit 1; fi
done
# Input routing must invalidate the certified Pages fingerprint.
. scripts/ci_docs.sh
. scripts/validation_components.sh
for input in CHANGELOG.md site/native-release.txt scripts/release_history.sh scripts/release_history.awk scripts/release_history.html; do
    [ "$(validation_pages_path "$input")" = true ]
done
echo 'PASS release history: canonical changes, availability, escaping and malformed inputs'
