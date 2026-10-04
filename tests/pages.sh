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
printf '<!doctype html><head></head><body><header class="site-header">Old</header><main id="main"><footer class="playground-footer">Runtime resources</footer></main></body>\n' > "$work/playground/index.html"
for asset in style.css app.mjs examples.mjs controller.mjs worker.mjs runtime.mjs vm.wasm compiler.bc stdlib.json provenance.json LICENSE vendor/index.js vendor/LICENSE-MIT; do
    printf 'fixture\n' > "$assets/$asset"
done
assemble() {
    sh "$root/scripts/assemble_site.sh" "$root/site" "$work/playground" "$work/pages"
}
assemble
grep -q 'id="v0.1.0-alpha.10"' "$work/pages/releases.html"
awk -v page=home -f scripts/site_chrome.awk site/index.html site/index.html > "$work/home"
cmp "$work/home" "$work/pages/index.html"
for page in index.html releases.html playground/index.html; do
    test "$(grep -c 'aria-current="page"' "$work/pages/$page")" = 1
    grep -q 'What’s changed' "$work/pages/$page"
    grep -q 'Under the hood' "$work/pages/$page"
done
grep -q 'href="../#engineering"' "$work/pages/playground/index.html"
grep -q 'href="./releases.html" aria-current="page"' "$work/pages/releases.html"
# Shared footer includes branding, licence and valid page-relative navigation.
for page in releases.html playground/index.html; do
    grep -q 'Experimental. Open source. Still evolving.' "$work/pages/$page"
    grep -q 'LICENSE">MIT</a>' "$work/pages/$page"
    grep -q 'href="#main">Back to top</a>' "$work/pages/$page"
    grep -q 'id="main"' "$work/pages/$page"
done
grep -q 'class="brand" href="../"' "$work/pages/playground/index.html"
grep -q 'class="playground-footer">Runtime resources' "$work/pages/playground/index.html"
# Every pinned runtime asset remains byte-identical; only the HTML shell changes.
diff -r "$work/playground/assets" "$work/pages/playground/assets"
# A missing insertion point fails before an output directory is created.
cp "$work/playground/index.html" "$work/original-index"
printf 'missing body close\n' > "$work/playground/index.html"
if sh "$root/scripts/assemble_site.sh" "$root/site" "$work/playground" "$work/invalid-pages" 2>/dev/null; then
    echo 'Accepted missing browser footer insertion point' >&2; exit 1
fi
test ! -e "$work/invalid-pages"
cp "$work/original-index" "$work/playground/index.html"

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
# A malformed canonical changelog fails before creating any publishable output.
rm "$work/site/coverage/leak"
printf '## Unreleased\n\n' > "$work/CHANGELOG.md"
if sh "$root/scripts/assemble_site.sh" "$work/site" "$work/playground" "$work/pages" 2>/dev/null; then
    echo 'Accepted invalid canonical changelog' >&2; exit 1
fi
test ! -e "$work/pages"
echo 'PASS Pages assembly, coverage landing and failure handling'

sh tests/release_history.sh
