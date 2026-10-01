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
# Validate report before creating output (failed inputs leave no partial site).
test -s "$coverage/html/index.html"
test -s "$coverage/summary.txt"
test -z "$(find "$coverage" -type l -print)"
case "$commit:$site_commit" in *[!0-9a-f:]*|'') exit 1;; esac
test "${#commit}" -eq 40 && test "${#site_commit}" -eq 40
case "$generated" in *[!0-9TZ:.-]*|'') exit 1;; esac
case "$run" in *[!0-9]*|'') exit 1;; esac
sh "$(dirname "$0")/assemble_site.sh" "$site" "$playground" "$destination"
sh "$(dirname "$0")/attach_coverage.sh" "$coverage" "$destination" "$commit" "$generated" "$run" "$site_commit"
