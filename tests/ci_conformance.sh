#!/bin/sh
# The union of source and bytecode partitions must equal the standalone proof.
set -eu
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-conformance-partition.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
tree="$work/tree with spaces"
mkdir -p "$tree/tests/functional/cases/one" "$tree/examples" \
    "$tree/tests/functional/expected/examples" "$tree/tests/functional/failures/bad"
mkdir -p "$tree/tests/functional/cases/runner_smoke"
printf 'source\n' > "$tree/tests/functional/cases/runner_smoke/main.panack"
printf 'hello\n' > "$tree/tests/functional/cases/runner_smoke/expected.stdout"
cp tests/native_conformance.sh tests/native_conformance_program.sh tests/profile_command.sh "$tree/tests/"
printf 'source\n' > "$tree/tests/functional/cases/one/main.panack"
printf 'source\n' > "$tree/examples/one.panack"
printf 'hello\n' > "$tree/tests/functional/cases/one/expected.stdout"
printf 'hello\n' > "$tree/tests/functional/expected/examples/one.stdout"
printf 'bad\n' > "$tree/tests/functional/failures/bad/main.panack"
printf 'bad\n' > "$tree/tests/functional/failures/bad/expected.stderr"
printf '0.0.0-test.1\n' > "$tree/VERSION"
mkdir -p "$tree/bin"
cat > "$tree/bin/panack" <<'CLI'
#!/bin/sh
set -eu
operation=$1
if [ "$operation" = run ]; then
    case "$2" in *.bc) operation=bytecode ;; *) operation=source ;; esac
fi
case "${2:-}" in
    */runner_smoke/main.panack|*/case-runner_smoke.bc)
        # Both ordinary programs must finish all observations before this corpus.
        case "$CI_CONFORMANCE_MODE" in source) expected=2 ;; bytecode) expected=4 ;; all) expected=6 ;; esac
        [ "$(wc -l < "$CI_CONFORMANCE_ORDINARY")" -eq "$expected" ] || {
            echo 'nested corpus overlapped ordinary work' >&2; exit 8;
        }
        if [ "runner-$operation" = "${CI_CONFORMANCE_FAIL:-}" ]; then exit 7; fi
        ;;
    *)
        case "$operation" in source|compile|bytecode)
            printf '%s\n' "$operation" >> "$CI_CONFORMANCE_ORDINARY" ;;
        esac ;;
esac
printf '%s\n' "$operation" >> "$CI_CONFORMANCE_CALLS"
if [ "$operation" = "${CI_CONFORMANCE_FAIL:-}" ]; then exit 7; fi
if [ "$operation-stderr" = "${CI_CONFORMANCE_FAIL:-}" ]; then printf 'unexpected\n' >&2; fi
case "$operation" in
    source|bytecode)
        if [ "$operation-output" = "${CI_CONFORMANCE_FAIL:-}" ]; then printf 'wrong\n'; else printf 'hello\n'; fi ;;
    compile) printf 'bytecode\n' > "$4" ;;
    check)
        if [ "${CI_CONFORMANCE_FAIL:-}" = accepted ]; then exit 0; fi
        printf 'bad\n' >&2; exit 1 ;;
    --help) printf 'usage: panack command\n' ;;
    --version) printf 'panack 0.0.0-test.1 (bytecode 9)\n' ;;
    *) exit 9 ;;
esac
CLI
cat > "$tree/panack-vm" <<'VM'
#!/bin/sh
printf 'malformed\n' >> "$CI_CONFORMANCE_CALLS"
printf 'not a Panackelty bytecode file\n' >&2
exit 1
VM
chmod +x "$tree/bin/panack" "$tree/panack-vm"
export CI_CONFORMANCE_CALLS="$work/calls"
export CI_CONFORMANCE_ORDINARY="$work/ordinary"
unset VALIDATION_PROFILE_FILE CI_CONFORMANCE_FAIL
fail() { echo "conformance partition: $*" >&2; exit 1; }
run() {
    : > "$CI_CONFORMANCE_CALLS"
    : > "$CI_CONFORMANCE_ORDINARY"
    CI_CONFORMANCE_MODE=${1:-all}; export CI_CONFORMANCE_MODE
    result=0
    VALIDATION_JOBS=${CI_CONFORMANCE_WORKERS:-1} sh "$tree/tests/native_conformance.sh" "$@" > "$work/output" 2>&1 || result=$?
}
run
[ "$result" = 0 ] || fail 'standalone proof failed'
sort "$CI_CONFORMANCE_CALLS" > "$work/all"
for CI_CONFORMANCE_WORKERS in 1 2; do
    : > "$work/partitioned"
    for mode in source bytecode; do
        run "$mode"
        [ "$result" = 0 ] || fail "$mode proof failed"
        cat "$CI_CONFORMANCE_CALLS" >> "$work/partitioned"
    done
    sort "$work/partitioned" > "$work/sorted"
    cmp "$work/all" "$work/sorted" || fail 'partitions or workers changed the standalone observations'
done
for mode in source bytecode; do
    for fault in "$mode" "$mode-stderr" "$mode-output"; do
        CI_CONFORMANCE_FAIL=$fault; export CI_CONFORMANCE_FAIL
        run "$mode"
        [ "$result" != 0 ] || fail "accepted $fault"
        if [ "$fault" = bytecode-output ]; then
            grep -F 'native conformance: captured stdout (case-one):' "$work/output" >/dev/null ||
                fail 'bytecode output mismatch hid its captured stdout'
            grep -F 'wrong' "$work/output" >/dev/null ||
                fail 'bytecode output mismatch hid the differing output'
        fi
    done
done
for fault in compile compile-stderr accepted; do
    CI_CONFORMANCE_FAIL=$fault; export CI_CONFORMANCE_FAIL
    run bytecode
    [ "$result" != 0 ] || fail "accepted $fault"
done
for fault in runner-source runner-bytecode runner-compile; do
    CI_CONFORMANCE_FAIL=$fault; export CI_CONFORMANCE_FAIL
    run all
    [ "$result" != 0 ] || fail "accepted nested corpus failure $fault"
done
unset CI_CONFORMANCE_FAIL
run invalid
[ "$result" = 2 ] && [ ! -s "$CI_CONFORMANCE_CALLS" ] || fail 'unknown mode performed work'
run source bytecode
[ "$result" = 2 ] && [ ! -s "$CI_CONFORMANCE_CALLS" ] || fail 'extra mode was ignored'
for CI_CONFORMANCE_WORKERS in 0 33 invalid; do
    run source
    [ "$result" = 2 ] && [ ! -s "$CI_CONFORMANCE_CALLS" ] || fail 'invalid worker count performed work'
done
echo 'Conformance partition equivalence and failure controls passed.'
