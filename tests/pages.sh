#!/bin/sh
# Canonical harness regression coverage; requires no Node or network access.
set -eu
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-pages.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
sha=1111111111111111111111111111111111111111
version=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
assets="$work/playground/assets/$version"
mkdir -p "$assets/vendor"
printf '%s\n' "$version" > "$work/playground/asset-version.txt"
printf 'fixture\n' > "$work/playground/index.html"
for asset in style.css app.mjs examples.mjs controller.mjs worker.mjs runtime.mjs vm.wasm compiler.bc stdlib.json provenance.json LICENSE vendor/index.js vendor/LICENSE-MIT; do
    printf 'fixture\n' > "$assets/$asset"
done
mkdir -p "$work/report/html/coverage/src/vm"
printf '<a href="coverage/src/vm/value.c.html">source</a>\n' > "$work/report/html/index.html"
printf '<a href="../../../index.html">index</a>\n' > "$work/report/html/coverage/src/vm/value.c.html"
printf 'TOTAL 100 90 90%%\n' > "$work/report/summary.txt"
assemble() {
    sh "$root/scripts/build_pages.sh" "$root/site" "$work/report" "$work/pages" \
        "$1" '2026-09-28T21:00:00Z' '123' "$sha" "$work/playground"
}
assemble "$sha"
cmp site/index.html "$work/pages/index.html"
cmp "$assets/vm.wasm" "$work/pages/playground/assets/$version/vm.wasm"
cmp "$work/report/summary.txt" "$work/pages/coverage/summary.txt"
cmp "$work/report/html/coverage/src/vm/value.c.html" "$work/pages/coverage/html/coverage/src/vm/value.c.html"
grep -q "$sha" "$work/pages/coverage/index.html"
grep -q 'Native C VM' "$work/pages/coverage/index.html"
grep -q '2026-09-28T21:00:00Z' "$work/pages/coverage/provenance.txt"
# Coverage-only publication preserves website and browser bytes exactly, without
# running assembly, downloading a release or provisioning a browser.
cp -R "$work/pages" "$work/reused"
rm -rf "$work/reused/coverage"
sh "$root/scripts/attach_coverage.sh" "$work/report" "$work/reused" "$sha" '2026-09-28T21:00:00Z' '124' "$sha"
find "$work/pages/playground" -type f | while IFS= read -r file; do
    relative=${file#"$work/pages/"}
    cmp "$file" "$work/reused/$relative"
done
cmp "$work/pages/index.html" "$work/reused/index.html"
grep -q 'check_run=124' "$work/reused/coverage/provenance.txt"
if sh "$root/scripts/attach_coverage.sh" "$work/report" "$work/reused" "$sha" '2026-09-28T21:00:00Z' '125' "$sha" 2>/dev/null; then
    echo 'Overwrote coverage output' >&2; exit 1
fi
# Reusing an output directory could silently retain stale files; reject it.
if assemble "$sha" 2>/dev/null; then echo 'Overwrote existing Pages output' >&2; exit 1; fi
rm -rf "$work/pages"
for invalid in ../escape abcd; do
    printf '%s\n' "$invalid" > "$work/playground/asset-version.txt"
    if assemble "$sha" 2>/dev/null; then echo 'Accepted invalid asset version' >&2; exit 1; fi
    test ! -e "$work/pages"
done
printf '%s\n' "$version" > "$work/playground/asset-version.txt"
mv "$assets/vm.wasm" "$work/vm.wasm"
if assemble "$sha" 2>/dev/null; then echo 'Accepted missing playground VM' >&2; exit 1; fi
test ! -e "$work/pages"
mv "$work/vm.wasm" "$assets/vm.wasm"
ln -s "$root/README.md" "$work/playground/leak"
if assemble "$sha" 2>/dev/null; then echo 'Accepted playground symlink' >&2; exit 1; fi
rm "$work/playground/leak"
if assemble '<script>' 2>/dev/null; then echo 'Accepted invalid provenance' >&2; exit 1; fi
test ! -e "$work/pages"
ln -s "$root/README.md" "$work/report/html/leak"
if assemble "$sha" 2>/dev/null; then echo 'Accepted symlink' >&2; exit 1; fi
rm "$work/report/html/leak"
rm "$work/report/html/index.html"
if assemble "$sha" 2>/dev/null; then echo 'Accepted missing report' >&2; exit 1; fi
test ! -e "$work/pages"
echo 'PASS Pages assembly, provenance and failure handling'
