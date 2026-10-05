#!/bin/sh
# Compare the standalone oracle with the union of its instrumented CI shards.
set -eu
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-sanitize-partition.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
tree=$work/tree
mkdir -p "$tree/tests/fixtures" "$tree/tests/functional/cases/runner_smoke" \
    "$tree/tests/functional/cases/ordinary" "$tree/tests/functional/expected/examples" "$tree/examples"
cp tests/native_oracle_contracts.sh tests/profile_command.sh tests/canonical_decimal.awk "$tree/tests/"
cp -R tests/fixtures/oracle_contracts "$tree/tests/fixtures/"
for source in tests/functional/cases/runner_smoke/main.panack tests/functional/cases/ordinary/main.panack examples/euler001.panack; do
    printf 'source\n' > "$tree/$source"
done
for output in tests/functional/cases/runner_smoke/expected.stdout tests/functional/cases/ordinary/expected.stdout tests/functional/expected/examples/euler001.stdout; do
    printf 'hello\n' > "$tree/$output"
done
for entry in driver-basic driver-relative driver-logical stdlib euler001; do
    case "$entry" in
        driver-basic) source=tests/fixtures/compiler_contracts/driver/basic.panack ;;
        driver-relative) source=tests/fixtures/compiler_contracts/driver/relative/main.panack ;;
        driver-logical) source=tests/fixtures/compiler_contracts/driver/logical/main.panack ;;
        stdlib) source=tests/functional/cases/stdlib/main.panack ;;
        euler001) source=examples/euler001.panack ;;
    esac
    printf '%s\n' "$source" | od -An -v -tx1 | tr -d ' \n' > "$tree/tests/fixtures/oracle_contracts/$entry.hex"
    printf '\n' >> "$tree/tests/fixtures/oracle_contracts/$entry.hex"
done
cat > "$tree/vm" <<'VM'
#!/bin/sh
set -eu
if [ "$1" = check ]; then
    if [ -f "$2" ]; then source=$(cat "$2"); else source=seed; fi
    operation=verify
elif [ "$2" = bootstrap/compiler-v9.bc ]; then
    operation=$3; source=$4
else
    operation=execute; source=$(cat "$2")
fi
printf '%s %s\n' "$operation" "$source" >> "$CI_SANITIZE_CALLS"
case "${CI_SANITIZE_FAIL:-}" in
    "report:$source")
        if [ "$operation" = execute ]; then echo 'FAIL retained runner diagnostic'; exit 7; fi ;;
    "$operation:$source") exit 7 ;;
    "stderr:$source") echo unexpected >&2 ;;
esac
case "$operation" in
    verify|check) printf 'ok\n' ;;
    compile) printf '%s\n' "$source" > "$6"; printf 'wrote %s\n' "$6" ;;
    run) printf 'hello\n' ;;
    execute)
        if [ "${CI_SANITIZE_FAIL:-}" = "output:$source" ]; then echo wrong; exit 0; fi
        case "$source" in
            tests/runner/oracle_command.panack)
                case "$4" in
                    *integer.stdin) cp tests/fixtures/oracle_contracts/integer.stdout "$5" ;;
                    *decimal.stdin) cp tests/fixtures/oracle_contracts/decimal.stdout "$5" ;;
                    /dev/null) cp tests/fixtures/oracle_contracts/builtin.stdout "$5" ;;
                    *) exit 8 ;;
                esac ;;
            tests/fixtures/oracle_contracts/rational.panack) cat tests/fixtures/oracle_contracts/rational.stdout ;;
            *) echo hello ;;
        esac ;;
    *) exit 9 ;;
esac
VM
chmod +x "$tree/vm"
export CI_SANITIZE_CALLS="$work/calls"
unset VALIDATION_PROFILE_FILE PANACK_CHECK_RUNNER_REPORT CI_SANITIZE_FAIL
fail() { echo "sanitizer partition: $*" >&2; exit 1; }
run() {
    : > "$CI_SANITIZE_CALLS"
    result=0
    (cd "$tree"; PANACK_NATIVE_BINARY=./vm sh tests/native_oracle_contracts.sh "$@") > "$work/output" 2>&1 || result=$?
}
run all
[ "$result" = 0 ] || { cat "$work/output"; fail 'standalone oracle'; }
sort "$CI_SANITIZE_CALLS" > "$work/all"
: > "$work/shards"
for mode in without-runner runner; do
    run "$mode"
    [ "$result" = 0 ] || fail "$mode failed"
    cat "$CI_SANITIZE_CALLS" >> "$work/shards"
done
sort "$work/shards" > "$work/sorted"
cmp "$work/all" "$work/sorted" || fail 'shards changed observation multiplicity'
for mode in without-runner runner; do
    for fault in compile execute verify stderr output; do
        # Inject at the earliest relevant operation. The complete positive union
        # above covers every later program; failure controls need no repeated prefix.
        if [ "$mode" = runner ]; then source=tests/functional/cases/runner_smoke/main.panack
        elif [ "$fault" = output ]; then source=tests/fixtures/oracle_contracts/rational.panack
        else source=tests/runner/oracle_command.panack; fi
        CI_SANITIZE_FAIL=$fault:$source; export CI_SANITIZE_FAIL
        run "$mode"
        [ "$result" != 0 ] || fail "accepted $mode $fault"
    done
done
CI_SANITIZE_FAIL=report:tests/functional/cases/runner_smoke/main.panack
export CI_SANITIZE_FAIL
run runner
[ "$result" = 7 ] || fail 'lost runner failure status'
grep -F 'FAIL retained runner diagnostic' "$work/output" >/dev/null || fail 'lost runner failure output'
grep -F 'running tests/functional/cases/runner_smoke/main.panack (exit 7)' "$work/output" >/dev/null || fail 'lost runner failure context'
unset CI_SANITIZE_FAIL
for args in unknown 'runner extra'; do
    run $args
    [ "$result" = 2 ] && [ ! -s "$CI_SANITIZE_CALLS" ] || fail 'invalid group performed work'
done
if make -s native-sanitize SANITIZE_SUITE=unknown MAKE=false > "$work/output" 2>&1; then
    fail 'accepted unknown sanitizer suite'
fi
grep -F 'unknown sanitizer suite' "$work/output" >/dev/null || fail 'built before validating suite'
echo 'Sanitizer oracle partition equivalence and failure controls passed.'
