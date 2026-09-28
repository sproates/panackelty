#!/bin/sh
# One complete source/compile/run observation, isolated for bounded conformance workers.
set -eu
[ "$#" = 4 ] || { echo 'expected mode, label, source and expectation' >&2; exit 2; }
mode=$1
label=$2
source=$3
expected=$4
case "$mode" in all|source|bytecode) ;; *) echo 'unknown conformance mode' >&2; exit 2 ;; esac
project=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
panack="$project/panack"
temporary=$(mktemp -d "${TMPDIR:-/tmp}/panack-native-program.XXXXXX")
trap 'rm -rf "$temporary"' EXIT HUP INT TERM
fail() { echo "native conformance: $*" >&2; exit 1; }

artifact="$temporary/$label.bc"
actual="$temporary/$label.stdout"
errors="$temporary/$label.stderr"

if [ "$mode" != bytecode ]; then
  sh "$project/tests/profile_command.sh" "conformance/$label/source" "$panack" run "$source" >"$actual" 2>"$errors" || fail "$label source execution failed"
  test ! -s "$errors" || fail "$label wrote unexpected stderr"
  cmp "$expected" "$actual" || fail "$label source output differs"
fi

if [ "$mode" != source ]; then
  sh "$project/tests/profile_command.sh" "conformance/$label/compile" "$panack" compile "$source" -o "$artifact" >"$temporary/compile.stdout" 2>"$errors" || fail "$label compilation failed"
  test ! -s "$errors" || fail "$label compilation wrote unexpected stderr"
  sh "$project/tests/profile_command.sh" "conformance/$label/bytecode" "$panack" run "$artifact" >"$actual" 2>"$errors" || fail "$label artifact execution failed"
  test ! -s "$errors" || fail "$label artifact wrote unexpected stderr"
  cmp "$expected" "$actual" || fail "$label artifact output differs"
fi
