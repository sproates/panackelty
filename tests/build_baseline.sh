#!/bin/sh
# Harness contracts use a small fake toolchain and the real probe-cache driver.
set -eu
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-baseline-test.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
fail() { printf 'build baseline regression: %s\n' "$*" >&2; exit 1; }
mkdir -p "$work/source/scripts" "$work/source/tests" "$work/source/src/stdlib" "$work/source/examples" "$work/source/bootstrap" "$work/bin"
cp scripts/build_baseline.sh "$work/source/scripts/"
cp tests/run_probe.sh tests/profile_command.sh "$work/source/tests/"
touch "$work/source/examples/.keep" "$work/source/src/stdlib/.keep"
printf 'seed\n' > "$work/source/bootstrap/compiler-v9.bc"
printf 'tracked\n' > "$work/source/deleted.txt"
printf 'build/\n' > "$work/source/.gitignore"
cat > "$work/source/panack-vm" <<'VM'
#!/bin/sh
case "$1" in
 check) printf 'ok\n' ;;
 run)
 if [ "${3:-}" = compile ]; then
   result=42
   if grep '+ 3' tests/build-baseline-fixture/direct.panack >/dev/null || grep '{ 41 }' tests/build-baseline-fixture/transitive.panack >/dev/null; then result=43; fi
   printf '%s\n' "$result" > "$6"
   printf 'wrote %s\n' "$6"
 else
   if [ "${BASELINE_TEST_BAD_OUTPUT:-0}" = 1 ]; then printf 'wrong\n'; else cat "$2"; fi
 fi ;;
 *) exit 2 ;;
esac
VM
chmod +x "$work/source/panack-vm"
cat > "$work/bin/make" <<'MAKE'
#!/bin/sh
[ -z "${CHECK_BUDGET_SECONDS-}${INCREMENTAL_BUDGET_SECONDS-}${FUNCTIONAL_BUDGET_SECONDS-}${BOOTSTRAP_BUDGET_SECONDS-}" ] || exit 7
case "$*" in
 --version) printf 'Fixture make\n' ;;
 clean) rm -rf build ;;
 '-j1 native') : ;;
 '-j1 check'|'-j1 check-compiler')
   [ "${BASELINE_TEST_FAIL:-0}" = 0 ] || exit 9
   printf 'fixture checks passed\n' ;;
 *) exit 3 ;;
esac
MAKE
chmod +x "$work/bin/make"
export PATH="$work/bin:$PATH"
cd "$work/source"
git init -q
git add .
git -c user.name=fixture -c user.email=fixture@example.invalid commit -qm fixture
# Both additions and deletions must be represented in the archived candidate.
rm deleted.txt
printf 'untracked candidate\n' > addition.txt
before=$(git status --porcelain)
CHECK_BUDGET_SECONDS=999 INCREMENTAL_BUDGET_SECONDS=999 FUNCTIONAL_BUDGET_SECONDS=999 BOOTSTRAP_BUDGET_SECONDS=999 \
sh scripts/build_baseline.sh "$work/report" fixture-host 2 all > "$work/output" 2>&1 || { cat "$work/output" >&2; exit 1; }
[ "$(git status --porcelain)" = "$before" ] || fail 'source worktree changed'
[ -f "$work/report/COMPLETE" ] || fail 'completion marker missing'
[ "$(wc -l < "$work/report/samples.tsv" | tr -d ' ')" = 13 ] || fail 'sample count'
awk -F '\t' '
$1 == "probe-unchanged" && ($5 != 0 || $6 != 1) { exit 1 }
$1 ~ /^probe-(unrelated|direct|transitive)$/ && ($5 != 1 || $6 != 1) { exit 1 }
' "$work/report/samples.tsv" || fail 'actual compilation/execution counts'
[ "$(wc -l < "$work/report/summary.tsv" | tr -d ' ')" = 7 ] || fail 'summary groups'
tar -tf "$work/report/source.tar" > "$work/paths"
grep '^addition.txt$' "$work/paths" >/dev/null || fail 'untracked candidate omitted'
if grep '^deleted.txt$' "$work/paths" >/dev/null; then fail 'deleted candidate retained'; fi
if find "$work/report" -type d -name 'work.*' | grep . >/dev/null; then fail 'disposable checkout retained'; fi
if sh scripts/build_baseline.sh "$work/report" fixture-host 1 probes > "$work/reject" 2>&1; then fail 'overwrote existing report'; fi
if sh scripts/build_baseline.sh "$PWD/output" fixture-host 1 probes > "$work/reject" 2>&1; then fail 'accepted source-local output'; fi
if sh scripts/build_baseline.sh "$work/invalid" fixture-host 0 probes > "$work/reject" 2>&1; then fail 'accepted invalid repeat count'; fi
if BASELINE_TEST_FAIL=1 sh scripts/build_baseline.sh "$work/failed" fixture-host 1 all > "$work/failure" 2>&1; then fail 'ignored failed validation'; fi
[ ! -e "$work/failed/COMPLETE" ] || fail 'failed run claimed complete'
awk -F '\t' '$1 == "clean-full" && $4 == 9 { found=1 } END { exit !found }' "$work/failed/samples.tsv" || fail 'failed sample not recorded'
if BASELINE_TEST_BAD_OUTPUT=1 sh scripts/build_baseline.sh "$work/wrong" fixture-host 1 probes > "$work/failure" 2>&1; then fail 'ignored incorrect probe output'; fi
[ ! -e "$work/wrong/COMPLETE" ] || fail 'incorrect output claimed complete'
printf 'Build baseline isolation, snapshot, cache counts, output and failure contracts passed.\n'
