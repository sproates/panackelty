#!/bin/sh
# Verify the checked-in stage-1 seed can execute the namespace path it will
# need to bootstrap the coordinated compiler-source migration.
set -eu

seed=${SEED_COMPILER:-bootstrap/compiler-v9.bc}
stdlib=$(pwd)/src/stdlib
fixture=tests/fixtures/compiler_contracts/namespaces/emission
workspace=$(mktemp -d "${TMPDIR:-/tmp}/panackelty seed namespace.XXXXXX")
trap 'rm -rf "$workspace"' EXIT HUP INT TERM

fail() { echo "namespace seed: $*" >&2; exit 1; }

mkdir "$workspace/project"
cp "$fixture"/*.panack "$workspace/project/"
sed 's/^import "left.panack" as left$/import project\/left as left/; s/^import "right.panack" as right$/import project\/right as right/' \
  "$workspace/project/main.panack" > "$workspace/main.panack"
mv "$workspace/main.panack" "$workspace/project/main.panack"
source="$workspace/project/main.panack"
artifact="$workspace/program.bc"

cat > "$workspace/expected.stdout" <<'EOF'
41
42
43
44
41
42
44
45
51
52
EOF

PANACKELTY_STDLIB_PATH="$stdlib" ./panack-vm run "$seed" check "$source" \
  > "$workspace/check.stdout" 2> "$workspace/check.stderr" || fail "seed rejected namespace source"
printf 'ok\n' > "$workspace/check.expected"
cmp -s "$workspace/check.expected" "$workspace/check.stdout" || fail "unexpected check output"
[ ! -s "$workspace/check.stderr" ] || fail "check wrote diagnostics"

PANACKELTY_STDLIB_PATH="$stdlib" ./panack-vm run "$seed" run "$source" \
  > "$workspace/source.stdout" 2> "$workspace/source.stderr" || fail "seed failed namespace source execution"
cmp -s "$workspace/expected.stdout" "$workspace/source.stdout" || fail "source output mismatch"
[ ! -s "$workspace/source.stderr" ] || fail "source execution wrote diagnostics"

PANACKELTY_STDLIB_PATH="$stdlib" ./panack-vm run "$seed" compile "$source" -o "$artifact" \
  > "$workspace/compile.stdout" 2> "$workspace/compile.stderr" || fail "seed failed namespace compilation"
[ -f "$artifact" ] || fail "seed did not publish bytecode"
[ ! -s "$workspace/compile.stderr" ] || fail "compile wrote diagnostics"
./panack-vm check "$artifact" > "$workspace/bytecode-check.stdout" 2> "$workspace/bytecode-check.stderr" || fail "generated artifact did not verify"
printf 'ok\n' > "$workspace/check.expected"
cmp -s "$workspace/check.expected" "$workspace/bytecode-check.stdout" || fail "unexpected bytecode check output"
[ ! -s "$workspace/bytecode-check.stderr" ] || fail "bytecode check wrote diagnostics"

rm -rf "$workspace/project"
./panack-vm run "$artifact" > "$workspace/bytecode.stdout" 2> "$workspace/bytecode.stderr" || fail "namespace bytecode failed after source removal"
cmp -s "$workspace/expected.stdout" "$workspace/bytecode.stdout" || fail "saved bytecode output mismatch"
[ ! -s "$workspace/bytecode.stderr" ] || fail "bytecode execution wrote diagnostics"

echo "namespace seed: source and saved-v9 acceptance passed"
