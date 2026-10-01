#!/bin/sh
# Canonical harness regression coverage; requires no Node or network access.
set -eu
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-pages.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
version=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
assets="$work/playground/assets/$version"
mkdir -p "$assets/vendor"
printf '%s\n' "$version" > "$work/playground/asset-version.txt"
printf 'fixture\n' > "$work/playground/index.html"
for asset in style.css app.mjs examples.mjs controller.mjs worker.mjs runtime.mjs vm.wasm compiler.bc stdlib.json provenance.json LICENSE vendor/index.js vendor/LICENSE-MIT; do
    printf 'fixture\n' > "$assets/$asset"
done
assemble() {
    sh "$root/scripts/assemble_site.sh" "$root/site" "$work/playground" "$work/pages"
}
assemble
cmp site/index.html "$work/pages/index.html"
cmp "$assets/vm.wasm" "$work/pages/playground/assets/$version/vm.wasm"
for landing in coverage/index.html coverage/html/index.html; do
    cmp "site/$landing" "$work/pages/$landing"
    grep -q 'https://sproates.github.io/panackelty-coverage/' "$work/pages/$landing"
done
test ! -e "$work/pages/coverage/summary.txt"
test ! -e "$work/pages/coverage/provenance.txt"
# Reusing an output directory could silently retain stale files; reject it.
if assemble 2>/dev/null; then echo 'Overwrote existing Pages output' >&2; exit 1; fi
rm -rf "$work/pages"
for invalid in ../escape abcd; do
    printf '%s\n' "$invalid" > "$work/playground/asset-version.txt"
    if assemble 2>/dev/null; then echo 'Accepted invalid asset version' >&2; exit 1; fi
    test ! -e "$work/pages"
done
printf '%s\n' "$version" > "$work/playground/asset-version.txt"
mv "$assets/vm.wasm" "$work/vm.wasm"
if assemble 2>/dev/null; then echo 'Accepted missing playground VM' >&2; exit 1; fi
test ! -e "$work/pages"
mv "$work/vm.wasm" "$assets/vm.wasm"
ln -s "$root/README.md" "$work/playground/leak"
if assemble 2>/dev/null; then echo 'Accepted playground symlink' >&2; exit 1; fi
rm "$work/playground/leak"
# Site symlinks are rejected too, including inside the compatibility landing.
cp -R "$root/site" "$work/site"
ln -s "$root/README.md" "$work/site/coverage/leak"
if sh "$root/scripts/assemble_site.sh" "$work/site" "$work/playground" "$work/pages" 2>/dev/null; then
    echo 'Accepted site symlink' >&2; exit 1
fi
test ! -e "$work/pages"
echo 'PASS Pages assembly, coverage landing and failure handling'
