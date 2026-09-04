#!/bin/sh

set -eu

project=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
panack="$project/panack"
temporary=$(mktemp -d "${TMPDIR:-/tmp}/panack-native.XXXXXX")
trap 'rm -rf "$temporary"' EXIT HUP INT TERM
unset PANACKELTY_STDLIB_VALUE || true

fail() {
  echo "native conformance: $*" >&2
  exit 1
}

assert_program() {
  label=$1
  source=$2
  expected=$3
  artifact="$temporary/$label.bc"
  actual="$temporary/$label.stdout"
  errors="$temporary/$label.stderr"

  "$panack" run "$source" >"$actual" 2>"$errors" || fail "$label source execution failed"
  test ! -s "$errors" || fail "$label wrote unexpected stderr"
  cmp "$expected" "$actual" || fail "$label source output differs"

  "$panack" compile "$source" -o "$artifact" >"$temporary/compile.stdout" 2>"$errors" || fail "$label compilation failed"
  test ! -s "$errors" || fail "$label compilation wrote unexpected stderr"
  "$panack" run "$artifact" >"$actual" 2>"$errors" || fail "$label artifact execution failed"
  test ! -s "$errors" || fail "$label artifact wrote unexpected stderr"
  cmp "$expected" "$actual" || fail "$label artifact output differs"
}

for case_directory in "$project"/tests/functional/cases/*; do
  label="case-$(basename "$case_directory")"
  if [ -f "$case_directory/source.path" ]; then
    IFS= read -r referenced_source <"$case_directory/source.path"
    source="$project/$referenced_source"
  else
    source="$case_directory/main.panack"
  fi
  assert_program "$label" "$source" "$case_directory/expected.stdout"
done

for source in "$project"/examples/*.panack; do
  name=$(basename "$source" .panack)
  assert_program "example-$name" "$source" "$project/tests/functional/expected/examples/$name.stdout"
done

for case_directory in "$project"/tests/functional/failures/*; do
  name=$(basename "$case_directory")
  source="$case_directory/main.panack"
  actual="$temporary/failure-$name.stderr"
  normalized="$temporary/failure-$name.normalized"
  if "$panack" check "$source" >"$temporary/failure.stdout" 2>"$actual"; then
    fail "$name was accepted"
  fi
  test ! -s "$temporary/failure.stdout" || fail "$name wrote unexpected stdout"
  sed "s|$case_directory|<case>|g" "$actual" >"$normalized"
  cmp "$case_directory/expected.stderr" "$normalized" || fail "$name diagnostic differs"
done

"$panack" --help >"$temporary/help.stdout" 2>"$temporary/help.stderr"
grep 'usage: panack ' "$temporary/help.stdout" >/dev/null || fail "help output is missing usage"
test ! -s "$temporary/help.stderr" || fail "help wrote unexpected stderr"

IFS= read -r release_version <"$project/VERSION"
printf 'panack %s (bytecode 7)\n' "$release_version" >"$temporary/version.expected"
"$panack" --version >"$temporary/version.stdout" 2>"$temporary/version.stderr"
cmp "$temporary/version.expected" "$temporary/version.stdout" || fail "version output differs"
test ! -s "$temporary/version.stderr" || fail "version wrote unexpected stderr"

printf 'not Panackelty bytecode' >"$temporary/malformed.bc"
if "$project/panack-vm" check "$temporary/malformed.bc" >"$temporary/malformed.stdout" 2>"$temporary/malformed.stderr"; then
  fail "malformed bytecode was accepted"
fi
grep 'not a Panackelty bytecode file' "$temporary/malformed.stderr" >/dev/null || fail "malformed diagnostic differs"

echo "native conformance: ok"
