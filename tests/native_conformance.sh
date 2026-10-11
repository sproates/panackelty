#!/bin/sh

set -eu
[ "$#" -le 1 ] || { echo 'expected at most one conformance mode' >&2; exit 2; }
mode=${1:-all}
case "$mode" in all|source|bytecode) ;; *) echo 'unknown conformance mode' >&2; exit 2 ;; esac

jobs=${VALIDATION_JOBS:-4}
case "$jobs" in ''|*[!0-9]*) echo 'invalid conformance worker count' >&2; exit 2 ;; esac
while [ "${jobs#0}" != "$jobs" ] && [ "$jobs" != 0 ]; do jobs=${jobs#0}; done
[ "$jobs" -ge 1 ] && [ "$jobs" -le 32 ] || exit 2

project=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
panack="$project/panack"
temporary=$(mktemp -d "${TMPDIR:-/tmp}/panack-native.XXXXXX")
trap 'rm -rf "$temporary"' EXIT HUP INT TERM
unset PANACKELTY_STDLIB_VALUE || true

fail() {
  echo "native conformance: $*" >&2
  exit 1
}

: > "$temporary/programs"

for case_directory in "$project"/tests/functional/cases/*; do
  label="case-$(basename "$case_directory")"
  if [ -f "$case_directory/source.path" ]; then
    IFS= read -r referenced_source <"$case_directory/source.path"
    source="$project/$referenced_source"
  else
    source="$case_directory/main.panack"
  fi
  printf '%s\0%s\0%s\0' "$label" "$source" "$case_directory/expected.stdout" >> "$temporary/programs"
done

for source in "$project"/examples/*.panack; do
  name=$(basename "$source" .panack)
  printf '%s\0%s\0%s\0' "example-$name" "$source" "$project/tests/functional/expected/examples/$name.stdout" >> "$temporary/programs"
done

# NUL-delimited arguments preserve checkout and fixture paths containing spaces.
# Every worker owns its artifact and captures; failures propagate through xargs.
xargs -0 -n 3 -P "$jobs" sh "$project/tests/native_conformance_program.sh" "$mode" < "$temporary/programs"

if [ "$mode" = source ]; then
  echo 'native source conformance: ok'
  exit 0
fi

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
printf 'panack %s (bytecode 9)\n' "$release_version" >"$temporary/version.expected"
"$panack" --version >"$temporary/version.stdout" 2>"$temporary/version.stderr"
cmp "$temporary/version.expected" "$temporary/version.stdout" || fail "version output differs"
test ! -s "$temporary/version.stderr" || fail "version wrote unexpected stderr"

printf 'not Panackelty bytecode' >"$temporary/malformed.bc"
if "$project/panack-vm" check "$temporary/malformed.bc" >"$temporary/malformed.stdout" 2>"$temporary/malformed.stderr"; then
  fail "malformed bytecode was accepted"
fi
grep 'not a Panackelty bytecode file' "$temporary/malformed.stderr" >/dev/null || fail "malformed diagnostic differs"

echo "native conformance: ok"
