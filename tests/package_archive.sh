#!/bin/sh
set -eu
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-archive-test.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
mkdir "$work/input"
printf '%s' '{"package":"archive.fixture"}' > "$work/input/panack.json"
COPYFILE_DISABLE=1 tar -czf "$work/package.tar.gz" -C "$work/input" panack.json
printf outside > "$work/escape"
(cd "$work/input" && COPYFILE_DISABLE=1 tar --format=ustar -P -czf "$work/traversal.tar.gz" panack.json ../escape)
ln -s panack.json "$work/input/payload"
(cd "$work/input" && COPYFILE_DISABLE=1 tar --format=ustar -P -czf "$work/symlink.tar.gz" panack.json payload)
rm "$work/input/payload"
(cd "$work/input" && COPYFILE_DISABLE=1 tar --format=ustar -czf "$work/duplicate.tar.gz" panack.json ./panack.json)
if command -v sha256sum >/dev/null 2>&1; then
    expected_archive_sha=$(sha256sum "$work/package.tar.gz" | awk '{print $1}')
else
    expected_archive_sha=$(shasum -a 256 "$work/package.tar.gz" | awk '{print $1}')
fi
PANACK_PACKAGE_ARCHIVE="$work/package.tar.gz" \
PANACK_EXPECTED_ARCHIVE_SHA256="$expected_archive_sha" \
PANACK_BAD_TRAVERSAL_ARCHIVE="$work/traversal.tar.gz" \
PANACK_BAD_SYMLINK_ARCHIVE="$work/symlink.tar.gz" \
PANACK_BAD_DUPLICATE_ARCHIVE="$work/duplicate.tar.gz" \
  "$@" tests/runner/package_archive_unit.panack
