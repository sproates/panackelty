# Shared assertion helpers. Each entry point owns and cleans its temporary tree.
set -eu
export LC_ALL=C
root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-harness.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
fail() { printf 'FAIL %s: %s\n' "${case_name:-harness}" "$*" >&2; exit 1; }
pass() { printf 'PASS harness/%s\n' "$case_name"; }
contains() {
    NEEDLE=$2 awk 'BEGIN { wanted=ENVIRON["NEEDLE"] } { text=text $0 "\n" }
        END { exit !index(text,wanted) }' "$1" || fail "missing text in $1: $2"
}
absent() {
    if NEEDLE=$2 awk '{ text=text $0 "\n" } END { exit !index(text,ENVIRON["NEEDLE"]) }' "$1"; then
        fail "unexpected text in $1: $2"
    fi
}
equal_files() { cmp "$1" "$2" || fail "files differ: $1 / $2"; }
# Native supervisor preserves per-command timeouts, signals and byte streams.
# Arguments: expected status (or 'failure'), seconds, command and arguments.
capture() {
    expected_status=$1
    seconds=$2
    shift 2
    executable=$(command -v "$1") || fail "command not found: $1"
    shift
    "$root/panack-vm" run "$HARNESS_COMMAND" "$expected_status" "$seconds" "$work/stdout" "$work/stderr" "$(pwd -P)" "$executable" "$@" || {
        cat "$work/stdout" "$work/stderr" >&2 || :
        fail "command status, signal or timeout: $*"
    }
}
token_present() {
    awk -v wanted="$2" '{ for (i=1;i<=NF;i++) if ($i==wanted) found=1 }
        END { exit !found }' "$1" || fail "missing command token: $2"
}
