#!/bin/sh
# Shared website/playground assembly for production and review builds.
set -eu
site=$1
playground=$2
destination=$3
for file in index.html capabilities/index.html styles.css chrome.css favicon.svg; do test -s "$site/$file"; done
test -s "$playground/index.html"
version=$(cat "$playground/asset-version.txt")
case "$version" in *[!0-9a-f]*|'') echo 'Invalid playground asset version' >&2; exit 1;; esac
test "${#version}" -eq 64
for asset in style.css app.mjs examples.mjs controller.mjs worker.mjs runtime.mjs vm.wasm compiler.bc stdlib.json provenance.json LICENSE vendor/index.js vendor/LICENSE-MIT; do
    test -s "$playground/assets/$version/$asset"
done
test ! -e "$destination"
test ! -e "$site/playground"
test -s "$site/coverage/index.html"
test -s "$site/coverage/html/index.html"
test ! -e "$site/coverage/summary.txt"
test ! -e "$site/coverage/provenance.txt"
test -z "$(find "$site" "$playground" -type l -print)"
history=$(mktemp)
trap 'rm -f "$history" "$history.playground" "$history.home" "$history.capabilities"' 0
trap 'exit 1' HUP INT TERM
sh "$(dirname "$0")/release_history.sh" "$site/.." > "$history"
awk -v page=playground -f "$(dirname "$0")/site_chrome.awk" "$site/index.html" "$playground/index.html" > "$history.playground"
awk -v page=home -f "$(dirname "$0")/site_chrome.awk" "$site/index.html" "$site/index.html" > "$history.home"
awk -v page=capabilities -f "$(dirname "$0")/site_chrome.awk" "$site/index.html" "$site/capabilities/index.html" > "$history.capabilities"
mkdir -p "$destination"
cp -R "$site/." "$destination/"
mkdir "$destination/playground"
cp -R "$playground/." "$destination/playground/"
cp "$history" "$destination/releases.html"
cp "$history.playground" "$destination/playground/index.html"
cp "$history.home" "$destination/index.html"
cp "$history.capabilities" "$destination/capabilities/index.html"
