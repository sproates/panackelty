#!/bin/sh

set -eu

if [ "$#" -ne 2 ]; then
  echo "usage: release_archive_smoke.sh ARCHIVE VERSION" >&2
  exit 2
fi

archive=$1
release_version=$2
case "$archive" in
  /*) ;;
  *) archive=$(pwd)/$archive ;;
esac

temporary=$(mktemp -d "${TMPDIR:-/tmp}/panack-release.XXXXXX")
trap 'rm -rf "$temporary"' EXIT HUP INT TERM

fail() {
  echo "release smoke: $*" >&2
  exit 1
}

run_clean() {
  PATH="$runtime_path" PANACKELTY_STDLIB_PATH=/unavailable \
    "$panack" "$@"
}

mkdir -p "$temporary/unpacked" "$temporary/relocated" "$temporary/runtime-bin"
tar -xzf "$archive" -C "$temporary/unpacked"
test -x "$temporary/unpacked/panackelty/bin/panack" || \
  fail "archive does not contain panackelty/bin/panack"
mv "$temporary/unpacked/panackelty" "$temporary/relocated/toolchain"

dirname_command=$(command -v dirname)
ln -s "$dirname_command" "$temporary/runtime-bin/dirname"
runtime_path=$temporary/runtime-bin
panack=$temporary/relocated/toolchain/bin/panack
workspace=$temporary/workspace
mkdir -p "$workspace"
cd "$workspace"

printf 'panack %s (bytecode 7)\n' "$release_version" >version.expected
run_clean --version >version.stdout 2>version.stderr || \
  fail "--version failed"
cmp version.expected version.stdout || fail "--version output differs"
test ! -s version.stderr || fail "--version wrote unexpected stderr"

run_clean --help >help.stdout 2>help.stderr || fail "--help failed"
grep 'usage: panack ' help.stdout >/dev/null || fail "--help is missing usage"
test ! -s help.stderr || fail "--help wrote unexpected stderr"

cat >hello.panack <<'EOF'
import stdlib/prelude

main(): Void {
  arguments: [Str] = command_args()
  selected: Option[Str] = Some(arguments[0])
  print(len(arguments))
  match selected {
    Some(value) => print(value),
    None() => print("missing")
  }
}
EOF

printf 'ok\n' >check.expected
run_clean check hello.panack >check.stdout 2>check.stderr || \
  fail "source check failed"
cmp check.expected check.stdout || fail "source check output differs"
test ! -s check.stderr || fail "source check wrote unexpected stderr"

printf '2\nalpha\n' >source.expected
run_clean run hello.panack alpha beta >source.stdout 2>source.stderr || \
  fail "source execution failed"
cmp source.expected source.stdout || fail "source output differs"
test ! -s source.stderr || fail "source execution wrote unexpected stderr"

printf 'wrote hello.bc\n' >compile.expected
run_clean compile hello.panack -o hello.bc >compile.stdout 2>compile.stderr || \
  fail "compilation failed"
test -s hello.bc || fail "compilation did not create bytecode"
cmp compile.expected compile.stdout || fail "compilation output differs"
test ! -s compile.stderr || fail "compilation wrote unexpected stderr"

printf '1\ngamma\n' >bytecode.expected
run_clean run hello.bc gamma >bytecode.stdout 2>bytecode.stderr || \
  fail "bytecode execution failed"
cmp bytecode.expected bytecode.stdout || fail "bytecode output differs"
test ! -s bytecode.stderr || fail "bytecode execution wrote unexpected stderr"

printf 'not Panackelty bytecode' >malformed.bc
if run_clean check malformed.bc >malformed.stdout 2>malformed.stderr; then
  fail "malformed bytecode was accepted"
fi
test ! -s malformed.stdout || fail "malformed bytecode wrote unexpected stdout"
grep 'not a Panackelty bytecode file' malformed.stderr >/dev/null || \
  fail "malformed-bytecode diagnostic differs"

echo "release smoke: ok"
