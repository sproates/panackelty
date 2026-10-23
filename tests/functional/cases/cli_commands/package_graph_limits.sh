#!/bin/sh
set -eu

[ "$#" = 3 ] || { echo 'expected compiler, toolchain version, and standard library root' >&2; exit 2; }
compiler=$1
version=$2
stdlib=$3
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-package-graph.XXXXXX")
trap 'rm -rf "$work"' EXIT
trap 'exit 1' HUP INT TERM

write_library() {
    library_root=$1
    library_name=$2
    library_version=$3
    library_dependencies=$4
    mkdir -p "$library_root/src"
    cat > "$library_root/panack.json" <<MANIFEST
{"schema":1,"package":"$library_name","version":"$library_version","language":"modules-v1","toolchain":"$version","source":"src","exports":{"api":"api.panack"},"dependencies":$library_dependencies}
MANIFEST
    printf 'pub pure answer(): Nat { 42 }\n' > "$library_root/src/api.panack"
}

write_app() {
    app_root=$1
    app_dependencies=$2
    mkdir -p "$app_root/src"
    cat > "$app_root/panack.json" <<MANIFEST
{"schema":1,"package":"example.graph_app","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":$app_dependencies}
MANIFEST
    printf 'main(): Void { }\n' > "$app_root/src/main.panack"
}

run_ok() {
    label=$1
    manifest=$2
    if PANACKELTY_VERSION="$version" PANACKELTY_STDLIB_PATH="$stdlib" \
        ./bin/panack check --project "$manifest" \
        > "$work/stdout" 2> "$work/stderr"; then
        if [ "$(cat "$work/stdout")" = 'ok' ] && [ ! -s "$work/stderr" ]; then
            printf 'PASS %s\n' "$label"
            return
        fi
    fi
    printf 'FAIL %s\n' "$label"
    cat "$work/stdout" >&2
    cat "$work/stderr" >&2
    return 1
}

run_rejected() {
    label=$1
    manifest=$2
    expected=$3
    status=0
    PANACKELTY_VERSION="$version" PANACKELTY_STDLIB_PATH="$stdlib" \
        ./bin/panack check --project "$manifest" \
        > "$work/stdout" 2> "$work/stderr" || status=$?
    if [ "$status" -eq 1 ] && [ ! -s "$work/stdout" ] &&
        grep -F "$expected" "$work/stderr" >/dev/null; then
        printf 'PASS %s\n' "$label"
        return
    fi
    printf 'FAIL %s (exit %s; expected %s)\n' "$label" "$status" "$expected"
    cat "$work/stdout" >&2
    cat "$work/stderr" >&2
    return 1
}

conflicts="$work/conflicts"
write_library "$conflicts/math-v1" example.math_support 1.0.0 '{}'
write_library "$conflicts/math-v2" example.math_support 2.0.0 '{}'
write_library "$conflicts/math-v1-copy" example.math_support 1.0.0 '{}'
write_library "$conflicts/geometry" example.geometry 1.0.0 \
    '{"common":{"path":"../math-v1"}}'
app="$conflicts/app"
write_app "$app" \
    '{"geometry":{"path":"../geometry"},"math_direct":{"path":"../math-v2"}}'
run_rejected 'nested version conflict' "$app/panack.json" \
    'package example.math_support is declared with conflicting versions'
write_app "$app" \
    '{"geometry":{"path":"../geometry"},"math_direct":{"path":"../math-v1-copy"}}'
run_rejected 'nested package identity resolves to multiple roots' "$app/panack.json" \
    'package example.math_support@1.0.0 resolves to multiple physical roots'

packages="$work/packages"
i=1
while [ "$i" -le 64 ]; do
    write_library "$packages/nodes/pkg-$i" "example.node$i" 1.0.0 '{}'
    i=$((i + 1))
done
app="$packages/app"
make_fanout_dependencies() {
    count=$1
    dependencies=''
    i=1
    while [ "$i" -le "$count" ]; do
        [ "$i" -eq 1 ] && dependencies='{' || dependencies="$dependencies,"
        dependencies="$dependencies\"dep$i\":{\"path\":\"../nodes/pkg-$i\"}"
        i=$((i + 1))
    done
    printf '%s}\n' "$dependencies"
}
dependencies=$(make_fanout_dependencies 63)
write_app "$app" "$dependencies"
run_ok '64 package manifests accepted' "$app/panack.json"
dependencies=$(make_fanout_dependencies 64)
write_app "$app" "$dependencies"
run_rejected '65 package manifests rejected' "$app/panack.json" \
    'package dependency graph exceeds 64 packages'

edges="$work/edges"
write_library "$edges/shared" example.edge_target 1.0.0 '{}'
app="$edges/app"
make_edge_dependencies() {
    count=$1
    dependencies=''
    i=1
    while [ "$i" -le "$count" ]; do
        [ "$i" -eq 1 ] && dependencies='{' || dependencies="$dependencies,"
        dependencies="$dependencies\"edge$i\":{\"path\":\"../shared\"}"
        i=$((i + 1))
    done
    printf '%s}\n' "$dependencies"
}
dependencies=$(make_edge_dependencies 256)
write_app "$app" "$dependencies"
run_ok '256 dependency edges accepted' "$app/panack.json"
dependencies=$(make_edge_dependencies 257)
write_app "$app" "$dependencies"
run_rejected '257 dependency edges rejected' "$app/panack.json" \
    'package dependency graph exceeds 256 edges'

make_chain() {
    count=$1
    chain_root="$work/chain-$count"
    i=1
    while [ "$i" -le "$count" ]; do
        next=$((i + 1))
        if [ "$i" -lt "$count" ]; then
            deps="{\"next\":{\"path\":\"../pkg-$next\"}}"
        else
            deps='{}'
        fi
        write_library "$chain_root/pkg-$i" "example.chain$i" 1.0.0 "$deps"
        i=$((i + 1))
    done
    write_app "$chain_root/app" '{"first":{"path":"../pkg-1"}}'
    printf '%s\n' "$chain_root/app/panack.json"
}

manifest=$(make_chain 31)
run_ok '32 package levels accepted' "$manifest"
manifest=$(make_chain 32)
run_rejected '33 package levels rejected' "$manifest" \
    'package dependency graph exceeds 32 package levels'
