#!/bin/sh
# Cache compilation, never test results. Each invocation executes the probe.
set -eu
incremental=false
if [ "${1:-}" = --incremental ]; then
    incremental=true
    shift
fi
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
seed=${PANACK_PROBE_SEED:-bootstrap/compiler-v9.bc}
cache=${PANACK_PROBE_CACHE:-build/probes}
stdlib=${PANACKELTY_STDLIB_PATH:-src/stdlib}
export PANACKELTY_STDLIB_PATH="$stdlib"
# Compiler and unit-test sources import through repository-root project paths.
case "$source" in
    src/compiler/*|tests/experiments/semantic_impact/*) probe_root=$(pwd -P) ;;
    tests/runner/*)
        if grep -Eq '^[[:space:]]*(pub[[:space:]]+)?import[[:space:]]+project/' "$source"; then
            probe_root=$(pwd -P)
        else
            probe_root=
        fi
        ;;
    tests/fixtures/compiler_contracts/*)
        if grep -Eq '^[[:space:]]*(pub[[:space:]]+)?import[[:space:]]+project/' "$source"; then
            probe_root=$(pwd -P)
        else
            probe_root=
        fi
        ;;
    *) probe_root= ;;
esac
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
    { pwd -P; printf '%s\n' "$source" "$stdlib" "$probe_root"; } > "$work/inputs"
    hash "$0" tests/probe_dependencies.awk tests/profile_command.sh \
        "$vm" "$seed" "$source" >> "$work/inputs"
    if ! awk -v cwd="$(pwd -P)" -v source="$source" \
        -v stdlib_root="$stdlib" -v project_root="$probe_root" \
        -f tests/probe_dependencies.awk > "$work/dependencies"; then
        # Never risk a stale artifact if the dependency graph is unfamiliar.
        if [ "$hasher" = sha256sum ]; then
            find -L src tests examples "$stdlib" -type f -name '*.panack' \
                -exec sha256sum {} + > "$work/sources"
        else
            find -L src tests examples "$stdlib" -type f -name '*.panack' \
                -exec shasum -a 256 {} + > "$work/sources"
        fi
        LC_ALL=C sort "$work/sources" >> "$work/inputs"
    else
        LC_ALL=C sort "$work/dependencies" > "$work/dependencies.sorted"
        if [ "$hasher" = sha256sum ]; then
            awk '{ printf "%s%c", $0, 0 }' "$work/dependencies.sorted" |
                xargs -0 sha256sum > "$work/sources"
        else
            awk '{ printf "%s%c", $0, 0 }' "$work/dependencies.sorted" |
                xargs -0 shasum -a 256 > "$work/sources"
        fi
        cat "$work/sources" >> "$work/inputs"
    fi
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
    # Module reuse complements the complete-artifact cache after a source edit.
    # Keep it outside source roots and separate by checkout and entry point.
    if [ "$incremental" = true ]; then
        vm_path=$(CDPATH= cd -- "$(dirname -- "$vm")" && printf '%s/%s\n' "$(pwd -P)" "$(basename -- "$vm")")
        compiler_path=$(CDPATH= cd -- "$(dirname -- "$seed")" && printf '%s/%s\n' "$(pwd -P)" "$(basename -- "$seed")")
        parent=${PANACKELTY_COMPILE_CACHE_DIR:-}
        if [ -z "$parent" ]; then
            cache_home=${XDG_CACHE_HOME:-${HOME:+$HOME/.cache}}
            case "$cache_home" in /*) parent=$cache_home/panackelty/compiler-probes ;; esac
        fi
        reuse_directory=
        case "$parent" in
            /*)
                scope=$({ pwd -P; printf '%s\n' "$source"; } | hash | awk '{print $1}')
                reuse_directory=$parent/$scope
                if ! (umask 077; mkdir -p "$reuse_directory") 2>/dev/null; then reuse_directory=; fi
                ;;
        esac
    fi
    invoke_compiler() {
        set -- "$vm" run "$seed"
        if [ "$incremental" = true ]; then
            PANACKELTY_COMPILER_PATH=$compiler_path PANACKELTY_VM_PATH=$vm_path PANACKELTY_COMPILE_CACHE_DIR=$reuse_directory
            export PANACKELTY_COMPILER_PATH PANACKELTY_VM_PATH PANACKELTY_COMPILE_CACHE_DIR
            set -- "$@" --local-compile-cache
        fi
        sh tests/profile_command.sh "probe-build/$source" "$@" compile "$source" -o "$work/probe.bc"
    }
    if [ -n "$probe_root" ]; then
        compile_probe() {
            (export PANACKELTY_BOOTSTRAP_ROOT="$probe_root"; invoke_compiler)
        }
    else
        compile_probe() {
            (unset PANACKELTY_BOOTSTRAP_ROOT; invoke_compiler)
        }
    fi
    if ! compile_probe \
        > "$work/stdout" 2> "$work/stderr"; then
        cat "$work/stdout" "$work/stderr" >&2
        exit 1
    fi
    printf 'wrote %s\n' "$work/probe.bc" > "$work/expected"
    cmp "$work/expected" "$work/stdout"
    if [ "$incremental" = true ] && [ "${PANACKELTY_COMPILE_CACHE_TRACE:-}" = 1 ]; then
        # Accept only the requested cache trace; other diagnostics still fail.
        sed -E '/^compile cache: (disabled|miss|hit|source inputs hit|stored|not stored|not stored: inputs changed or reuse limit exceeded|(code )?module hits [0-9]+|source summary hits [0-9]+)$/d' "$work/stderr" > "$work/diagnostics"
        cat "$work/stderr" >&2
        mv "$work/diagnostics" "$work/stderr"
    fi
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
    env -u PANACKELTY_BOOTSTRAP_ROOT sh tests/profile_command.sh "probe-run/$source" "$vm" run "$artifact" "$@"
fi
