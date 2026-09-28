#!/bin/sh
# Canonical harness regression coverage; requires no Node or network access.
set -eu
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-pages.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
sha=1111111111111111111111111111111111111111
mkdir -p "$work/report/html/coverage/src/vm"
printf '<a href="coverage/src/vm/value.c.html">source</a>\n' > "$work/report/html/index.html"
printf '<a href="../../../index.html">index</a>\n' > "$work/report/html/coverage/src/vm/value.c.html"
printf 'TOTAL 100 90 90%%\n' > "$work/report/summary.txt"
assemble() {
    sh "$root/scripts/build_pages.sh" "$root/site" "$work/report" "$work/pages" \
        "$1" '2026-09-28T21:00:00Z' '123' "$sha"
}
assemble "$sha"
cmp site/index.html "$work/pages/index.html"
cmp "$work/report/summary.txt" "$work/pages/coverage/summary.txt"
cmp "$work/report/html/coverage/src/vm/value.c.html" "$work/pages/coverage/html/coverage/src/vm/value.c.html"
grep -q "$sha" "$work/pages/coverage/index.html"
grep -q 'Native C VM' "$work/pages/coverage/index.html"
grep -q '2026-09-28T21:00:00Z' "$work/pages/coverage/provenance.txt"
# Reusing an output directory could silently retain stale files; reject it.
if assemble "$sha" 2>/dev/null; then echo 'Overwrote existing Pages output' >&2; exit 1; fi
rm -rf "$work/pages"
if assemble '<script>' 2>/dev/null; then echo 'Accepted invalid provenance' >&2; exit 1; fi
test ! -e "$work/pages"
ln -s "$root/README.md" "$work/report/html/leak"
if assemble "$sha" 2>/dev/null; then echo 'Accepted symlink' >&2; exit 1; fi
rm "$work/report/html/leak"
rm "$work/report/html/index.html"
if assemble "$sha" 2>/dev/null; then echo 'Accepted missing report' >&2; exit 1; fi
test ! -e "$work/pages"
echo 'PASS Pages assembly, provenance and failure handling'
