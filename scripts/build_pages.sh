#!/bin/sh
# Assemble one complete Pages artifact. Never publish a partial site/report.
set -eu
site=$1
coverage=$2
destination=$3
commit=$4
generated=$5
run=$6
site_commit=$7
playground=$8
case "$commit:$site_commit" in *[!0-9a-f:]*|'') echo 'Invalid source commit' >&2; exit 1;; esac
test "${#commit}" -eq 40 && test "${#site_commit}" -eq 40
case "$generated" in *[!0-9TZ:.-]*|'') echo 'Invalid coverage date' >&2; exit 1;; esac
case "$run" in *[!0-9]*|'') echo 'Invalid workflow run' >&2; exit 1;; esac
test -s "$site/index.html"
test -s "$site/styles.css"
test -s "$site/favicon.svg"
test -s "$coverage/html/index.html"
test -s "$coverage/summary.txt"
test -s "$playground/index.html"
version=$(cat "$playground/asset-version.txt")
case "$version" in *[!0-9a-f]*|'') echo 'Invalid playground asset version' >&2; exit 1;; esac
test "${#version}" -eq 64
for asset in style.css app.mjs examples.mjs controller.mjs worker.mjs runtime.mjs vm.wasm compiler.bc stdlib.json provenance.json LICENSE vendor/index.js vendor/LICENSE-MIT; do
    test -s "$playground/assets/$version/$asset"
done
test ! -e "$destination"
# Reject symlinks instead of following them into unrelated files.
test -z "$(find "$site" "$coverage" "$playground" -type l -print)"
mkdir -p "$destination/coverage"
cp -R "$site/." "$destination/"
mkdir "$destination/playground"
cp -R "$playground/." "$destination/playground/"
cp -R "$coverage/html" "$destination/coverage/html"
cp "$coverage/summary.txt" "$destination/coverage/summary.txt"
cat > "$destination/coverage/index.html" <<EOF
<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Native VM coverage — Panackelty</title>
<link rel="stylesheet" href="../styles.css"></head>
<body><main style="max-width: 70rem; margin: 3rem auto; padding: 1.5rem">
<p><a href="../">Panackelty home</a></p>
<h1>Native C VM coverage</h1>
<p>LLVM line and branch coverage from the native test corpus. This does not
measure the self-hosted compiler or the whole language.</p>
<p><a href="html/index.html">Browse the interactive source report</a> ·
<a href="summary.txt">Download the coverage summary</a></p>
<p>Report archived (UTC): <time>$generated</time></p>
<p>Coverage source: <a href="https://github.com/sproates/panackelty/commit/$commit">$commit</a></p>
<p>Website source: <a href="https://github.com/sproates/panackelty/commit/$site_commit">$site_commit</a></p>
<p><a href="https://github.com/sproates/panackelty/actions/runs/$run">Successful validation run</a></p>
<p>This is the latest successfully published report. Documentation-only changes
reuse prior coverage. Failed validation or publication leaves the previous
report in place; check the date and source commit when assessing freshness.</p>
</main></body></html>
EOF
printf 'coverage_commit=%s\narchived_at=%s\ncheck_run=%s\nsite_commit=%s\n' \
    "$commit" "$generated" "$run" "$site_commit" > "$destination/coverage/provenance.txt"
