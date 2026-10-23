#!/bin/sh
# Repeatedly swap a package root while the compiler loads its source closure.
set -eu
[ "$#" = 3 ] || { echo 'expected compiler, version and standard-library root' >&2; exit 2; }
compiler=$1
version=$2
stdlib=$3
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-package-root-race.XXXXXX")
active="$work/active"
next="$work/active.next"
ready="$work/ready"
running="$work/running"
failed="$work/racer-failed"
racer=
cleanup() {
    rm -f "$running"
    if [ -n "$racer" ]; then
        # Let an in-flight replacement finish before removing its directory.
        wait "$racer" 2>/dev/null || true
    fi
    rm -rf "$work"
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM

mkdir -p "$work/app-a/src" "$work/app-b/src"
race_modules=64
for root in "$work/app-a" "$work/app-b"; do
    cat > "$root/panack.json" <<MANIFEST
{
  "schema": 1,
  "package": "example.race",
  "version": "1.0.0",
  "language": "modules-v1",
  "toolchain": "$version",
  "source": "src",
  "entry": "main.panack",
  "exports": {},
  "dependencies": {}
}
MANIFEST
done
cat > "$work/app-a/src/main.panack" <<'SOURCE'
import project/value as value
main(): Void { if value.answer() == 101 { print("app-a"); } else { print("MIXED"); } }
SOURCE
cat > "$work/app-b/src/main.panack" <<'SOURCE'
import project/value as value
main(): Void { if value.answer() == 202 { print("app-b"); } else { print("MIXED"); } }
SOURCE
for root in "$work/app-a" "$work/app-b"; do
    if [ "$root" = "$work/app-a" ]; then answer=101; else answer=202; fi
    {
        module=1
        while [ "$module" -le "$race_modules" ]; do
            printf 'import project/race_%s as race_%s\n' "$module" "$module"
            module=$((module + 1))
        done
        printf 'pub pure answer(): Nat { %s }\n' "$answer"
    } > "$root/src/value.panack"
    module=1
    while [ "$module" -le "$race_modules" ]; do
        printf 'pub pure marker(): Nat { 1 }\n' > "$root/src/race_$module.panack"
        module=$((module + 1))
    done
done

platform=$(uname -s)
replace_root() {
    rm -f "$next"
    ln -s "$1" "$next"
    case "$platform" in
        Darwin) mv -h -f "$next" "$active" ;;
        Linux) mv -T -f "$next" "$active" ;;
        *) echo "unsupported symlink-race host: $platform" >&2; return 2 ;;
    esac
}
replace_root "$work/app-a"

# The rename is atomic on supported hosts, so every invocation sees one valid
# root. A loader must either read one root consistently or reject a root change
# before combining files from different package trees.
touch "$running"
(
    replace_root "$work/app-b" || { touch "$failed"; exit 1; }
    touch "$ready"
    while [ -e "$running" ]; do
        replace_root "$work/app-a" || { touch "$failed"; exit 1; }
        replace_root "$work/app-b" || { touch "$failed"; exit 1; }
    done
) &
racer=$!
attempt=0
while [ ! -e "$ready" ]; do
    if ! kill -0 "$racer" 2>/dev/null; then
        echo 'symlink replacement worker stopped before the race began' >&2
        exit 1
    fi
    sleep 0.001
done

successes=0
identity_changes=0
source_read_errors=0
manifest_read_errors=0
while [ "$attempt" -lt 40 ]; do
    attempt=$((attempt + 1))
    result=0
    PANACKELTY_VERSION="$version" PANACKELTY_STDLIB_PATH="$stdlib" \
        ./bin/panack run --project "$active/panack.json" \
        > "$work/stdout" 2> "$work/stderr" || result=$?
    if [ "$result" -eq 0 ]; then
        output=$(cat "$work/stdout")
        case "$output" in
            app-a|app-b) successes=$((successes + 1)) ;;
            *) echo "mixed or unexpected successful package output: $output" >&2; exit 1 ;;
        esac
        test ! -s "$work/stderr" || { cat "$work/stderr" >&2; exit 1; }
    else
        test ! -s "$work/stdout" || { cat "$work/stdout" >&2; exit 1; }
        errors=$(cat "$work/stderr")
        case "$errors" in
            *'resolved project package root changed after resolution'*)
                identity_changes=$((identity_changes + 1)) ;;
            *'package root identity changed while loading source'*)
                identity_changes=$((identity_changes + 1)) ;;
            *'package source read failed'*)
                source_read_errors=$((source_read_errors + 1)) ;;
            *'cannot read project manifest ('*)
                manifest_read_errors=$((manifest_read_errors + 1)) ;;
            *) echo "unexpected package race failure: $errors" >&2; exit 1 ;;
        esac
    fi
done

test ! -e "$failed" || { echo 'symlink replacement worker failed' >&2; exit 1; }
test "$identity_changes" -gt 0 || {
    echo 'race did not exercise a package-root identity change' >&2
    exit 1
}
printf 'package root race passed (%s consistent reads, %s identity changes, ' \
    "$successes" "$identity_changes"
printf '%s source read errors, %s manifest read errors)\n' \
    "$source_read_errors" "$manifest_read_errors"
