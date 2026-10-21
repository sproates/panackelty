#!/bin/sh
# Mutate isolated inventories; never edit the working checkout or run a compiler.
set -eu
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-inventory-test.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
tree="$work/tree with spaces"
fail() { echo "fixture inventory regression: $*" >&2; exit 1; }
reset_tree() {
    rm -rf "$tree"
    mkdir -p "$tree/tests/runner" "$tree/tests/functional/cases/runner_smoke" \
        "$tree/tests/functional/expected" "$tree/examples"
    cp -R examples/. "$tree/examples/"
    cp -R tests/functional/expected/examples "$tree/tests/functional/expected/"
    cp tests/runner/main.panack "$tree/tests/runner/"
    cp tests/functional/cases/runner_smoke/main.panack "$tree/tests/functional/cases/runner_smoke/"
}
replace() {
    sed "$2" "$tree/$1" > "$work/edited"
    mv "$work/edited" "$tree/$1"
}
reject() {
    if sh "$root/tests/fixture_inventory.sh" "$tree" > "$work/out" 2>&1; then fail "accepted $1"; fi
    grep -F "$2" "$work/out" >/dev/null || { cat "$work/out" >&2; fail "wrong diagnostic for $1"; }
}
reset_tree
sh "$root/tests/fixture_inventory.sh" "$tree"
printf 'main(): Void {}\n' > "$tree/examples/new_example.panack"
: > "$tree/tests/functional/expected/examples/new_example.stdout"
reject addition 'examples list missing new_example'
grep -F 'count is 25, inventory requires 26' "$work/out" >/dev/null
# Real Makefile must stop before invoking policy/build sub-makes, even with -j.
cp Makefile VERSION "$tree/"
cp tests/fixture_inventory.sh tests/fixture_inventory.awk "$tree/tests/"
cat > "$work/next-phase" <<EOF_INNER
#!/bin/sh
touch "$work/phase-started"
exit 0
EOF_INNER
chmod +x "$work/next-phase"
if (cd "$tree" && make -j4 check-phases MAKE="$work/next-phase") > "$work/make.out" 2>&1; then
    fail 'check-phases accepted stale inventory'
fi
grep -F 'examples list missing new_example' "$work/make.out" >/dev/null
test ! -e "$work/phase-started" || fail 'expensive phase started before rejection'
# A deliberately maintained inventory is accepted, without rewriting goldens.
replace tests/runner/main.panack 's/== "" { 25 }/== "" { 26 }/'
replace tests/functional/cases/runner_smoke/main.panack 's/"option_result"/"new_example", "option_result"/; s/tests: 538/tests: 541/'
sh "$root/tests/fixture_inventory.sh" "$tree"
reset_tree
replace tests/functional/cases/runner_smoke/main.panack 's/"generic_functions", "generic_methods"/"generic_methods", "generic_functions"/'
reject order 'example order differs'
reset_tree
rm "$tree/examples/owned_methods.panack"
reject removal 'stale example owned_methods'
reset_tree
rm "$tree/tests/functional/expected/examples/owned_methods.stdout"
reject missing-output 'owned_methods.stdout: missing output'
reset_tree
: > "$tree/tests/functional/expected/examples/orphan.stdout"
reject orphan 'orphan expected output: orphan.stdout'
reset_tree
mkdir "$tree/examples/namespaces/owned_methods"
: > "$tree/examples/namespaces/owned_methods/main.panack"
reject duplicate-source 'duplicate example name owned_methods'
reset_tree
replace tests/runner/main.panack 's/== "" { 25 }/== "" { 24 }/'
reject count 'count is 24, inventory requires 25'
reset_tree
replace tests/functional/cases/runner_smoke/main.panack 's/"generic_methods"/"replacement"/'
reject same-size-list 'examples list missing generic_methods'
reset_tree
replace tests/functional/cases/runner_smoke/main.panack 's/"generic_methods"/"owned_methods"/'
reject duplicate-list 'duplicate examples entry owned_methods'
reset_tree
replace tests/functional/cases/runner_smoke/main.panack 's/tests: 538/tests: 532/'
reject total 'transcript total is 532, inventory requires 538'
reset_tree
replace tests/functional/cases/runner_smoke/main.panack 's/examples = /renamed_examples = /'
reject format 'expected declaration not found'
reset_tree
replace tests/runner/main.panack 's/name == "unknown_name"/name == "other_name"/'
reject mode-drift 'expected declaration not found'
reset_tree
rm "$tree/examples/namespaces/public_api/main.panack"
reject namespace-removal 'stale example public_api'
printf 'fixture inventory regressions: passed\n'
