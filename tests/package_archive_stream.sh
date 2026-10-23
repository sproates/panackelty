#!/bin/sh
set -eu
work=$(mktemp -d "$(pwd -P)/build/package-archive-test.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
mkdir -p "$work/source/nested" "$work/staging"
printf '%s' '{}' > "$work/source/panack.json"
printf '%s\n' hello > "$work/source/nested/example.txt"
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/package.tar.gz" -C "$work/source" panack.json nested
sh tests/run_probe.sh tests/runner/package_archive_unit.panack "$work/staging" "$work/package.tar.gz"
test -z "$(ls -A "$work/staging")"
