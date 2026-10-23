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

# The same canonical tree must result when a tar omits directory headers.
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/implicit.tar.gz" -C "$work/source" panack.json nested/example.txt
sh tests/run_probe.sh tests/runner/package_archive_unit.panack "$work/staging" "$work/implicit.tar.gz"
mkdir -p "$work/source/empty" "$work/source/src/nested"
printf 'Z\n' > "$work/source/Z.panack"
printf 'a\n' > "$work/source/a.panack"
printf 'main(): Void {}\n' > "$work/source/src/nested/main.panack"
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/ordering.tar.gz" -C "$work/source" .
# Independently calculated with Node SHA-256 and Buffer.compare UTF-8 ordering.
# Case-folded ordering would put a.panack before Z.panack and change this hash.
sh tests/run_probe.sh tests/runner/package_archive_unit.panack "$work/staging" "$work/ordering.tar.gz" \
    626c915e1c0dc6cff91f4db735da6cc6bbbf9bc646207c2f75320c67b923bba5
test -z "$(ls -A "$work/staging")"

# Every rejected archive must release its private extraction directory.
reject() {
    sh tests/run_probe.sh tests/runner/package_archive_rejection.panack \
        "$work/staging" "$work/rejected.tar.gz" "$1"
    test -z "$(ls -A "$work/staging")"
}
cp "$work/package.tar.gz" "$work/rejected.tar.gz"
printf junk >> "$work/rejected.tar.gz"
reject gzip
# Remove the trailer while retaining enough bytes to enter the decoder.
dd if="$work/package.tar.gz" of="$work/rejected.tar.gz" bs=1 count=40 2>/dev/null
reject gzip
ln -s /etc/passwd "$work/source/link"
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/rejected.tar.gz" -C "$work/source" panack.json link
reject unsupported
rm "$work/source/link"
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/rejected.tar.gz" -C "$work/source" panack.json panack.json
reject duplicate
printf x > "$work/source/Straße"
printf y > "$work/source/STRASSE"
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/rejected.tar.gz" -C "$work/source" panack.json Straße STRASSE
reject case-colliding
printf '%s\n' 'PASS archive corruption, links, duplicate paths, Unicode collisions and cleanup'

# The bound is on real streamed headers, including the archive root header.
mkdir "$work/many"
printf '{}' > "$work/many/panack.json"
index=0
while [ "$index" -lt 16384 ]; do
    : > "$work/many/f$index"
    index=$((index + 1))
done
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/rejected.tar.gz" -C "$work/many" .
reject 16384
printf '%s\n' 'PASS streamed archive entry/header bound leaves no partial stage'

# A gzip comment can consume input without producing output. Exercise the
# compressed-byte limit independently of tar/header and expanded-byte limits.
printf '\037\213\010\020\000\000\000\000\000\003' > "$work/rejected.tar.gz"
dd if=/dev/zero bs=65536 count=1024 2>/dev/null | tr '\000' x >> "$work/rejected.tar.gz"
reject '64 MiB'
# Sparse fixture creation avoids storing an expanded archive in the repository.
mkdir "$work/expanded"
printf '{}' > "$work/expanded/panack.json"
dd if=/dev/zero of="$work/expanded/payload" bs=1 count=1 seek=268435455 2>/dev/null
COPYFILE_DISABLE=1 tar --format ustar -czf "$work/rejected.tar.gz" -C "$work/expanded" panack.json payload
reject output_limit
printf '%s\n' 'PASS compressed and expanded byte limits leave no partial stage'
