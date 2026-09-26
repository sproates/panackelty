#!/bin/sh
# Prove clean development, bootstrap, conformance and packaging with only
# explicitly allowed native tools visible. Hosted machines need not be altered.
set -eu
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-native-environment.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
mkdir "$work/bin"
for utility in sh bash git make cc gcc clang as ld ar ranlib xcrun xcodebuild \
    awk basename cat chmod cmp comm cp cut date dd dirname env find grep gzip \
    head install ln mkdir mkfifo mktemp mv od readlink realpath rm rmdir sed sha256sum shasum \
    sleep sort stat tail tar tee touch tr uname wc which xargs; do
    path=$(command -v "$utility" || :)
    if [ -n "$path" ]; then ln -s "$path" "$work/bin/$utility"; fi
done
PATH=$work/bin
export PATH
unset PYTHONPATH PYTHONHOME VIRTUAL_ENV CONDA_PREFIX
language=py
language=${language}thon
for interpreter in "$language" "${language}2" "${language}3" "${language}3.12"; do
    if command -v "$interpreter" >/dev/null 2>&1; then
        echo 'isolated environment exposes an interpreter' >&2; exit 1
    fi
done
cd "$root"
make clean
VALIDATION_PROFILE_RUN=clean sh tests/profile_command.sh clean/check make check
VALIDATION_PROFILE_RUN=package sh tests/profile_command.sh package make package
echo 'isolated native development and package validation: passed'
