#!/bin/sh
# Cache compilation, never test results. Each invocation executes the probe.
set -eu
output=
if [ "${1:-}" = --compile ]; then
    output=$3
    shift
    source=$1
    shift 2
else
    source=$1
    shift
fi
vm=${PANACK_PROBE_VM:-./panack-vm}
seed=${PANACK_PROBE_SEED:-bootstrap/compiler-v8.bc}
cache=${PANACK_PROBE_CACHE:-build/probes}
stdlib=${PANACKELTY_STDLIB_PATH:-src/stdlib}
export PANACKELTY_STDLIB_PATH="$stdlib"
test -f "$source"
mkdir -p "$cache"
work=$(mktemp -d "$cache/.compile.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
if command -v sha256sum >/dev/null 2>&1; then
    hash() { sha256sum "$@"; }
    hasher=sha256sum
else
    hash() { shasum -a 256 "$@"; }
    hasher=shasum
fi
fingerprint() {
    # Include names as well as bytes: additions, deletions, restored timestamps,
    # checkout relocation and different instrumented VMs must invalidate reuse.
    { pwd -P; printf '%s\n' "$source" "$stdlib"; } > "$work/inputs"
    hash "$0" "$vm" "$seed" "$source" >> "$work/inputs"
    if [ "$hasher" = sha256sum ]; then
        find -L src tests examples "$stdlib" -type f -name '*.panack' \
            -exec sha256sum {} + > "$work/sources"
    else
        find -L src tests examples "$stdlib" -type f -name '*.panack' \
            -exec shasum -a 256 {} + > "$work/sources"
    fi
    LC_ALL=C sort "$work/sources" >> "$work/inputs"
    hash "$work/inputs" | awk '{print $1}'
}
key=$(fingerprint)
artifact=$cache/$key.bc
digest=$artifact.sha256
valid=false
if [ -f "$artifact" ] && [ -f "$digest" ]; then
    hash "$artifact" | awk '{print $1}' > "$work/actual"
    if cmp -s "$work/actual" "$digest"; then valid=true; fi
fi
if [ "$valid" = false ]; then
    if ! sh tests/profile_command.sh "probe-build/$source" \
        "$vm" run "$seed" compile "$source" -o "$work/probe.bc" \
        > "$work/stdout" 2> "$work/stderr"; then
        cat "$work/stdout" "$work/stderr" >&2
        exit 1
    fi
    printf 'wrote %s\n' "$work/probe.bc" > "$work/expected"
    cmp "$work/expected" "$work/stdout"
    test ! -s "$work/stderr" || { cat "$work/stderr" >&2; exit 1; }
    "$vm" check "$work/probe.bc" > "$work/check"
    printf 'ok\n' > "$work/expected"
    cmp "$work/expected" "$work/check"
    test "$key" = "$(fingerprint)" || {
        echo 'probe inputs changed during compilation; rerun validation' >&2
        exit 1
    }
    hash "$work/probe.bc" | awk '{print $1}' > "$work/digest"
    # Concurrent builders publish complete files. A torn pair is a cache miss.
    mv "$work/probe.bc" "$artifact"
    mv "$work/digest" "$digest"
fi
if [ -n "$output" ]; then
    if ! cmp -s "$artifact" "$output"; then cp "$artifact" "$output"; fi
    printf 'wrote %s\n' "$output"
else
    sh tests/profile_command.sh "probe-run/$source" "$vm" run "$artifact" "$@"
fi
