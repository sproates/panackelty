#!/bin/sh
# Failure injection for the refresh transaction; --native proves real staging.
set -eu
root=$(pwd)
PANACKELTY_STDLIB_PATH=$root/src/stdlib
export PANACKELTY_STDLIB_PATH
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-seed-refresh.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
script=$root/bootstrap/regenerate-seed.sh
mkdir "$work/seed dir"
seed=$work/seed\ dir/compiler.bc
manifest=$seed.sha256
hash() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum < "$1" | awk '{print $1}'
    else
        shasum -a 256 < "$1" | awk '{print $1}'
    fi
}
reset() {
    printf 'original\n' > "$seed"
    printf '%s  compiler.bc\n' "$(hash "$seed")" > "$manifest"
    cp "$seed" "$work/before.bc"
    cp "$manifest" "$work/before.sha256"
    rm -f "$work/calls"
}
invoke() {
    sh "$script" "$work/vm" "$seed" "$manifest" compiler.panack "$work/conformance/main.panack" > "$work/log" 2>&1
}
unchanged() {
    cmp "$seed" "$work/before.bc"
    cmp "$manifest" "$work/before.sha256"
    test ! -e "$seed.refresh-lock"
}
reject() {
    if invoke; then echo "seed refresh test: unexpectedly accepted $MODE" >&2; exit 1; fi
    grep -F "$1" "$work/log" > /dev/null || { cat "$work/log" >&2; exit 1; }
    unchanged
}

if [ "${1:-}" = --native ]; then
    seed=$work/seed\ dir/compiler-v8.bc
    manifest=$seed.sha256
    cp bootstrap/compiler-v8.bc "$seed"
    cp bootstrap/compiler-v8.bc.sha256 "$manifest"
    # Only POSIX utilities and a hash tool are visible; no Python executable.
    mkdir "$work/bin"
    for utility in sh awk cmp cp mkdir rm mv chmod; do
        ln -s "$(command -v "$utility")" "$work/bin/$utility"
    done
    if command -v sha256sum >/dev/null 2>&1; then utility=sha256sum; else utility=shasum; fi
    ln -s "$(command -v "$utility")" "$work/bin/$utility"
    PATH="$work/bin" sh "$script" "$root/panack-vm" "$seed" "$manifest" src/compiler/main.panack tests/functional/cases/stdlib/main.panack
    test "$(hash "$seed")  compiler-v8.bc" = "$(cat "$manifest")"
    test ! -e "$seed.refresh-lock"
    echo 'seed refresh: native staging passed with a Python-free PATH'
    exit
fi

mkdir "$work/conformance"
printf 'ok\n' > "$work/conformance/expected.stdout"
cat > "$work/vm" <<'VM'
#!/bin/sh
set -eu
printf '%s\n' "$*" >> "$WORK/calls"
if [ "$1" = check ]; then
    case "$MODE:$2" in
        invalid-input:*/input.bc|invalid-stage:*/stage3.bc|invalid-stdlib:*/stdlib3.bc) echo 'injected verifier failure' >&2; exit 1;;
    esac
    exit
fi
if [ "$#" -eq 2 ]; then
    case "$MODE:$2" in
        runtime:*/stdlib3.bc) echo 'injected runtime failure' >&2; exit 1;;
        output:*/stdlib3.bc|expected-output:*) printf 'wrong\n';;
        *) printf 'ok\n';;
    esac
    exit
fi
out=$6
case "$MODE:$out" in
    signal:*/stage3.bc) kill -TERM "$PPID"; exit 1;;
    compile:*/stage3.bc) echo 'injected compiler failure' >&2; exit 1;;
    compiler23:*/stage3.bc|compiler34:*/stage4.bc|stdlib23:*/stdlib3.bc|stdlib34:*/stdlib4.bc) printf 'different\n' > "$out";;
    *) printf 'candidate\n' > "$out";;
esac
case "$MODE:$out" in
    edit-seed:*/stage4.bc) printf 'external edit\n' > "$SEED";;
    edit-digest:*/stage4.bc) printf 'external edit\n' > "$MANIFEST";;
esac
VM
chmod +x "$work/vm"
WORK=$work SEED=$seed MANIFEST=$manifest
export WORK SEED MANIFEST
MODE=success
export MODE
reset
printf 'corrupt\n' >> "$seed"
cp "$seed" "$work/before.bc"
reject 'input seed digest mismatch'
test ! -e "$work/calls"
reset
printf 'junk\n' >> "$manifest"
cp "$manifest" "$work/before.sha256"
reject 'input seed digest mismatch'
test ! -e "$work/calls"
for MODE in invalid-input invalid-stage invalid-stdlib compile runtime compiler23 compiler34 stdlib23 stdlib34 output expected-output; do
    reset
    case "$MODE" in
        invalid-*) message='injected verifier failure';;
        compile) message='injected compiler failure';;
        runtime) message='injected runtime failure';;
        compiler*) message='compiler fixed point mismatch';;
        stdlib*) message='standard-library artifact mismatch';;
        output) message='standard-library output mismatch';;
        expected-output) message='standard-library expected output mismatch';;
    esac
    reject "$message"
done
for MODE in edit-seed edit-digest; do
    reset
    if invoke; then echo 'seed refresh overwrote concurrent edit' >&2; exit 1; fi
    grep -F 'changed during refresh' "$work/log" > /dev/null
    if [ "$MODE" = edit-seed ]; then
        test "$(cat "$seed")" = 'external edit'
        cmp "$manifest" "$work/before.sha256"
    else
        test "$(cat "$manifest")" = 'external edit'
        cmp "$seed" "$work/before.bc"
    fi
done
MODE=success
reset
mv "$seed" "$work/real.bc"
ln -s "$work/real.bc" "$seed"
if invoke; then echo 'seed refresh accepted symlink' >&2; exit 1; fi
test -L "$seed"
rm "$seed"
mv "$work/real.bc" "$seed"
unchanged
MODE=signal
if invoke; then echo 'seed refresh ignored signal' >&2; exit 1; fi
unchanged
MODE=success
mkdir "$work/hash-bin"
for utility in sh cmp cp mkdir rm; do
    ln -s "$(command -v "$utility")" "$work/hash-bin/$utility"
done
printf '#!/bin/sh\nexit 1\n' > "$work/hash-bin/sha256sum"
chmod +x "$work/hash-bin/sha256sum"
if (PATH="$work/hash-bin"; export PATH; invoke); then echo 'seed refresh ignored hash failure' >&2; exit 1; fi
grep -F 'SHA-256 failed' "$work/log" > /dev/null
unchanged
mkdir "$seed.refresh-lock"
if invoke; then echo 'seed refresh ignored lock' >&2; exit 1; fi
test -d "$seed.refresh-lock"
rmdir "$seed.refresh-lock"
unchanged
invoke
test "$(cat "$seed")" = candidate
test "$(cat "$manifest")" = "$(hash "$seed")  compiler.bc"
cp "$seed" "$work/before.bc"
cp "$manifest" "$work/before.sha256"
invoke
grep -F 'already at verified fixed point' "$work/log" > /dev/null
unchanged
echo 'seed refresh: failure preservation, publication and idempotence passed'
