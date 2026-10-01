#!/bin/sh
# Shared website/playground assembly for production and review builds.
set -eu
site=$1
playground=$2
destination=$3
for file in index.html styles.css favicon.svg; do test -s "$site/$file"; done
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
mkdir -p "$destination"
cp -R "$site/." "$destination/"
mkdir "$destination/playground"
cp -R "$playground/." "$destination/playground/"
