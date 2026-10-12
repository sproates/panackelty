#!/bin/sh

set -eu

if [ "$#" -ne 2 ]; then
  echo "usage: release_archive_smoke.sh ARCHIVE VERSION" >&2
  exit 2
fi

archive=$1
release_version=$2
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
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

printf 'panack %s (bytecode 9)\n' "$release_version" >version.expected
run_clean --version >version.stdout 2>version.stderr || \
  fail "--version failed"
cmp version.expected version.stdout || fail "--version output differs"
test ! -s version.stderr || fail "--version wrote unexpected stderr"

run_clean --help >help.stdout 2>help.stderr || fail "--help failed"
grep 'usage: panack ' help.stdout >/dev/null || fail "--help is missing usage"
test ! -s help.stderr || fail "--help wrote unexpected stderr"

cat >hello.panack <<'EOF'
import stdlib/host as host

main(): Void {
  arguments: [Str] = host.arguments()
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

cat >inferred.panack <<'EOF'
main(): Void {
  name = "Ada"
  mut total = 0
  for value in [1, 2, 3] { total = total + value }
  print(name)
  print(total)
}
EOF
printf 'Ada\n6\n' >inferred.expected
run_clean run inferred.panack >inferred.stdout 2>inferred.stderr || \
  fail "inferred source execution failed"
cmp inferred.expected inferred.stdout || fail "inferred source output differs"
test ! -s inferred.stderr || fail "inferred source wrote unexpected stderr"

cat >broken.panack <<'EOF'
main(): Void {
  print(missing)
}
EOF
printf 'error: %s/broken.panack:2:9: unknown name missing\n' "$(pwd -P)" >broken.expected
cat >>broken.expected <<'EOF'
  2 |   print(missing)
    |         ^
EOF
if run_clean check broken.panack >broken.stdout 2>broken.stderr; then
  fail "invalid source was accepted"
fi
test ! -s broken.stdout || fail "invalid source wrote unexpected stdout"
cmp broken.expected broken.stderr || fail "source excerpt differs"

printf 'not Panackelty bytecode' >malformed.bc
if run_clean check malformed.bc >malformed.stdout 2>malformed.stderr; then
  fail "malformed bytecode was accepted"
fi
test ! -s malformed.stdout || fail "malformed bytecode wrote unexpected stdout"
grep 'not a Panackelty bytecode file' malformed.stderr >/dev/null || \
  fail "malformed-bytecode diagnostic differs"

namespace_source=$temporary/relocated/toolchain/examples/namespaces/trip_planner/main.panack
printf 'wrote inventory trip.pinv\n' >inventory.expected
run_clean inventory "$namespace_source" -o trip.pinv >inventory.stdout 2>inventory.stderr || \
  fail "packaged source inventory failed"
cmp inventory.expected inventory.stdout || fail "inventory output differs"
test ! -s inventory.stderr || fail "inventory wrote unexpected stderr"
run_clean inventory-check "$namespace_source" --inventory trip.pinv \
  >inventory-check.stdout 2>inventory-check.stderr || fail "packaged inventory check failed"
grep -Fx 'inventory: valid' inventory-check.stdout >/dev/null || \
  fail "inventory check did not confirm validity"
test ! -s inventory-check.stderr || fail "inventory check wrote unexpected stderr"

run_clean compile "$namespace_source" -o trip.bc >coverage-compile.stdout \
  2>coverage-compile.stderr || fail "packaged coverage fixture compile failed"
test -s trip.bc || fail "packaged coverage fixture produced no bytecode"
test ! -s coverage-compile.stderr || fail "coverage fixture compile wrote unexpected stderr"
cat "$root/tests/functional/expected/examples/trip_planner.stdout" >coverage.expected
run_clean coverage-run trip.bc trip.pinv trip.pcov >coverage.stdout 2>coverage.stderr || \
  fail "packaged coverage collection failed"
cmp coverage.expected coverage.stdout || fail "coverage collection changed program output"
test -s trip.pcov || fail "coverage collection produced no record"
test ! -s coverage.stderr || fail "coverage collection wrote unexpected stderr"
run_clean coverage-report "$namespace_source" --inventory trip.pinv --coverage trip.pcov \
  >coverage-report.stdout 2>coverage-report.stderr || fail "packaged coverage report failed"
grep -F 'coverage: single-execution terminal=1' coverage-report.stdout >/dev/null || \
  fail "coverage report is missing its terminal record"
test ! -s coverage-report.stderr || fail "coverage report wrote unexpected stderr"

run_clean coverage-session trip-session trip.bc trip.pinv -- \
  >coverage-session.stdout 2>coverage-session.stderr || \
  fail "packaged coverage session failed"
cmp coverage.expected coverage-session.stdout || fail "coverage session changed program output"
run_clean coverage-aggregate --source "$namespace_source" trip.pinv --session trip-session \
  >coverage-aggregate.stdout 2>coverage-aggregate.stderr || \
  fail "packaged coverage aggregation failed"
grep -F 'aggregate sessions=1 executions=1' coverage-aggregate.stdout >/dev/null || \
  fail "coverage aggregation is missing its session summary"
test ! -s coverage-aggregate.stderr || fail "coverage aggregation wrote unexpected stderr"

echo "release smoke: ok"
