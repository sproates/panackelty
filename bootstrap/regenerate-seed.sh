#!/bin/sh
# Run from the repository root. Never use cached bootstrap artifacts.
set -eu

fail() { echo "seed refresh: $*" >&2; exit 1; }
[ "$#" -eq 5 ] || fail "expected VM, seed, digest file, compiler source and conformance source"
vm=$1
seed=$2
manifest=$3
source=$4
conformance=$5
[ -f "$seed" ] && [ ! -L "$seed" ] || fail "seed must be a regular, non-symlink file"
[ -f "$manifest" ] && [ ! -L "$manifest" ] || fail "digest must be a regular, non-symlink file"
seed_dir=${seed%/*}
manifest_dir=${manifest%/*}
[ "$seed_dir" != "$seed" ] || seed_dir=.
[ "$manifest_dir" != "$manifest" ] || manifest_dir=.
[ "$seed_dir" = "$manifest_dir" ] || fail "seed and digest must share a directory"

digest() {
    if command -v sha256sum >/dev/null 2>&1; then
        result=$(sha256sum < "$1") || fail "SHA-256 failed"
    elif command -v shasum >/dev/null 2>&1; then
        result=$(shasum -a 256 < "$1") || fail "SHA-256 failed"
    else
        fail "no SHA-256 utility found"
    fi
    result=${result%% *}
    [ "${#result}" -eq 64 ] || fail "invalid SHA-256 result"
    case "$result" in *[!0-9a-f]*) fail "invalid SHA-256 result";; esac
    printf '%s\n' "$result"
}

lock=${seed}.refresh-lock
mkdir "$lock" 2>/dev/null || fail "refresh lock exists: $lock (inspect before removing)"
trap 'rm -rf "$lock"' 0
trap 'exit 1' HUP INT TERM

# Snapshot both inputs under the lock, and validate the exact manifest format.
cp "$seed" "$lock/input.bc"
cp "$manifest" "$lock/input.sha256"
input_digest=$(digest "$lock/input.bc")
printf '%s  %s\n' "$input_digest" "${seed##*/}" > "$lock/expected.sha256"
cmp -s "$lock/input.sha256" "$lock/expected.sha256" || fail "input seed digest mismatch"
"$vm" check "$lock/input.bc"
printf 'Input seed SHA-256: %s\n' "$input_digest"

previous=$lock/input.bc
for stage in 2 3 4; do
    artifact=$lock/stage$stage.bc
    "$vm" run "$previous" compile "$source" -o "$artifact"
    "$vm" check "$artifact"
    stage_digest=$(digest "$artifact")
    printf 'Stage %s compiler SHA-256: %s\n' "$stage" "$stage_digest"
    previous=$artifact
done
cmp -s "$lock/stage2.bc" "$lock/stage3.bc" || fail "stage 2/3 compiler fixed point mismatch"
cmp -s "$lock/stage3.bc" "$lock/stage4.bc" || fail "stage 3/4 compiler fixed point mismatch"

# A refreshed seed must also reproduce and execute the standard-library program.
for stage in 2 3 4; do
    artifact=$lock/stdlib$stage.bc
    "$vm" run "$lock/stage$stage.bc" compile "$conformance" -o "$artifact"
    "$vm" check "$artifact"
    "$vm" run "$artifact" > "$lock/stdlib$stage.stdout"
done
cmp -s "$lock/stdlib2.bc" "$lock/stdlib3.bc" || fail "stage 2/3 standard-library artifact mismatch"
cmp -s "$lock/stdlib3.bc" "$lock/stdlib4.bc" || fail "stage 3/4 standard-library artifact mismatch"
cmp -s "$lock/stdlib2.stdout" "$lock/stdlib3.stdout" || fail "stage 2/3 standard-library output mismatch"
cmp -s "$lock/stdlib3.stdout" "$lock/stdlib4.stdout" || fail "stage 3/4 standard-library output mismatch"
cmp -s "$lock/stdlib2.stdout" "${conformance%/*}/expected.stdout" || fail "standard-library expected output mismatch"
stdlib_digest=$(digest "$lock/stdlib2.bc")
printf 'Standard-library SHA-256: %s\n' "$stdlib_digest"

# Fail rather than overwrite edits made while the stages were being built.
cmp -s "$seed" "$lock/input.bc" || fail "input seed changed during refresh"
cmp -s "$manifest" "$lock/input.sha256" || fail "input digest changed during refresh"
candidate_digest=$(digest "$lock/stage2.bc")
printf '%s  %s\n' "$candidate_digest" "${seed##*/}" > "$lock/candidate.sha256"
if cmp -s "$seed" "$lock/stage2.bc"; then
    echo "seed refresh: already at verified fixed point"
else
    # Each rename is atomic and on the same filesystem. An interruption between
    # them leaves a detectable digest mismatch, never a silently accepted seed.
    chmod 644 "$lock/stage2.bc" "$lock/candidate.sha256"
    mv -f "$lock/stage2.bc" "$seed"
    mv -f "$lock/candidate.sha256" "$manifest"
    echo "seed refresh: updated seed and SHA-256; review both files together"
fi
