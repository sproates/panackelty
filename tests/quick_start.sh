#!/bin/sh

set -eu

if [ "$#" -ne 2 ]; then
  echo "usage: quick_start.sh ARCHIVE VERSION" >&2
  exit 2
fi

archive=$1
release_version=$2
case "$archive" in
  /*) ;;
  *) archive=$(pwd)/$archive ;;
esac
checksum=$archive.sha256

test -f "$archive" || {
  echo "quick start: archive not found: $archive" >&2
  exit 1
}
test -f "$checksum" || {
  echo "quick start: checksum not found: $checksum" >&2
  exit 1
}

temporary=$(mktemp -d "${TMPDIR:-/tmp}/panack-quick-start.XXXXXX")
trap 'rm -rf "$temporary"' EXIT HUP INT TERM
runtime_bin=$temporary/runtime-bin
mkdir -p "$runtime_bin"

for utility in awk cmp dirname gzip ln mkdir mv readlink rm tar; do
  utility_path=$(command -v "$utility")
  ln -s "$utility_path" "$runtime_bin/$utility"
done
if command -v sha256sum >/dev/null 2>&1; then
  ln -s "$(command -v sha256sum)" "$runtime_bin/sha256sum"
  checksum_command=sha256sum
elif command -v shasum >/dev/null 2>&1; then
  ln -s "$(command -v shasum)" "$runtime_bin/shasum"
  checksum_command='shasum -a 256'
else
  echo "quick start: no SHA-256 utility found" >&2
  exit 1
fi

download=$temporary/download
workspace=$temporary/workspace
mkdir -p "$download" "$workspace"
archive_name=${archive##*/}
checksum_name=${checksum##*/}
cp "$archive" "$download/$archive_name"
cp "$checksum" "$download/$checksum_name"

HOME=$temporary/home
PATH=$runtime_bin
PANACKELTY_STDLIB_PATH=/unavailable
export HOME PATH PANACKELTY_STDLIB_PATH

cd "$download"
if [ "$checksum_command" = sha256sum ]; then
  sha256sum -c "$checksum_name" >/dev/null
else
  shasum -a 256 -c "$checksum_name" >/dev/null
fi
tar -xzf "$archive_name"

awk '
  /<!-- quick-start-program-begin -->/ { capture = 1; next }
  /<!-- quick-start-program-end -->/ { capture = 0 }
  capture && !/^```/ { print }
' panackelty/README.md >"$workspace/hello.panack"
awk '
  /<!-- quick-start-output-begin -->/ { capture = 1; next }
  /<!-- quick-start-output-end -->/ { capture = 0 }
  capture && !/^```/ { print }
' panackelty/README.md >"$workspace/expected.stdout"
test -s "$workspace/hello.panack" || \
  { echo "quick start: README program is missing" >&2; exit 1; }
test -s "$workspace/expected.stdout" || \
  { echo "quick start: README transcript is missing" >&2; exit 1; }

mkdir -p "$HOME/.local/opt" "$HOME/.local/bin"
mv panackelty "$HOME/.local/opt/panackelty"
ln -s "$HOME/.local/opt/panackelty/bin/panack" "$HOME/.local/bin/panack"
PATH=$HOME/.local/bin:$runtime_bin
export PATH

cd "$workspace"
{
  panack --version
  panack check hello.panack
  panack run hello.panack
  panack compile hello.panack
  panack run hello.bc
} >actual.stdout 2>actual.stderr
cmp expected.stdout actual.stdout || \
  { echo "quick start: documented output differs" >&2; exit 1; }
test ! -s actual.stderr || \
  { echo "quick start: commands wrote unexpected stderr" >&2; exit 1; }

cd "$download"
tar -xzf "$archive_name"
test ! -e "$HOME/.local/opt/panackelty.new" || \
  { echo "quick start: upgrade staging path exists" >&2; exit 1; }
test ! -e "$HOME/.local/opt/panackelty.old" || \
  { echo "quick start: upgrade backup path exists" >&2; exit 1; }
mv panackelty "$HOME/.local/opt/panackelty.new"
mv "$HOME/.local/opt/panackelty" "$HOME/.local/opt/panackelty.old"
mv "$HOME/.local/opt/panackelty.new" "$HOME/.local/opt/panackelty"
expected_version="panack $release_version (bytecode 7)"
test "$(panack --version)" = "$expected_version" || \
  { echo "quick start: upgraded command has the wrong version" >&2; exit 1; }
rm -rf "$HOME/.local/opt/panackelty.old"

rm "$HOME/.local/bin/panack"
rm -rf "$HOME/.local/opt/panackelty"
test ! -e "$HOME/.local/bin/panack" || \
  { echo "quick start: command link was not removed" >&2; exit 1; }
test ! -e "$HOME/.local/opt/panackelty" || \
  { echo "quick start: toolchain was not removed" >&2; exit 1; }

echo "quick start: ok"
