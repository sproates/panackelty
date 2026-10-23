#!/bin/sh
set -eu

repo_root=$(pwd -P)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-package-handoff.XXXXXX")
variant_dir=$(mktemp -d "$repo_root/build/package-tool-test.XXXXXX")
trap 'rm -rf "$work" "$variant_dir"' 0
trap 'exit 1' HUP INT TERM

version=$(sed -n '1p' VERSION)
compiler_seed=bootstrap/compiler-v9.bc
compiler_artifact=build/bootstrap/stage2/compiler.bc
seed_before=$(shasum -a 256 "$compiler_seed" | awk '{print $1}')
compiler_before=
if [ -f "$compiler_artifact" ]; then
    compiler_before=$(shasum -a 256 "$compiler_artifact" | awk '{print $1}')
fi

mkdir -p "$work/app/src" "$work/geometry/src"
cat > "$work/app/panack.json" <<MANIFEST
{"schema":1,"package":"example.handoff_app","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"geometry":{"path":"../geometry"}}}
MANIFEST
cat > "$work/app/src/main.panack" <<'SOURCE'
import geometry/api as geometry
main(): Void { print(geometry.answer()) }
SOURCE
cat > "$work/geometry/panack.json" <<MANIFEST
{"schema":1,"package":"example.handoff_geometry","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","exports":{"api":"api.panack"},"dependencies":{}}
MANIFEST
cat > "$work/geometry/src/api.panack" <<'SOURCE'
pub pure answer(): Nat { 42 }
SOURCE

"$repo_root/bin/panack" deps lock --project "$work/app/panack.json" > "$work/lock.out"
"$repo_root/bin/panack" check --project "$work/app/panack.json" > "$work/check.out"
test "$(cat "$work/check.out")" = ok
"$repo_root/bin/panack" run --project "$work/app/panack.json" > "$work/run.out"
test "$(cat "$work/run.out")" = 42

if rg -n '^(pub )?import project/src/(compiler|bytecode)/' src/packages src/project_model; then
    echo 'package tool imports compiler or bytecode implementation sources' >&2
    exit 1
fi

# Exercise the real dependency graph in a private copy, so tests never mutate
# another concurrent build's source tree. Preserve timestamps for native inputs.
mkdir -p "$variant_dir/tests" "$variant_dir/build/tools"
cp -p Makefile VERSION panack-vm "$variant_dir/"
cp -pR src bootstrap "$variant_dir/"
cp -p tests/profile_command.sh "$variant_dir/tests/"
cp -p build/tools/panack-packages.bc "$variant_dir/build/tools/"
# The native executable has already been validated by the caller. Keep this
# focused proof on tool/compiler dependencies without rebuilding the VM copy.
(cd "$variant_dir" && make --no-print-directory -o panack-vm package-tool) > "$work/baseline.out"
sed 's/panack-packages prepare/panack-packages inspect/' \
    src/packages/main.panack > "$variant_dir/src/packages/main.panack"
# Make the edited source newer even on filesystems with coarse timestamps.
# Keep the artifact present: otherwise a missing dependency could pass the test.
touch -t 203001010000 "$variant_dir/src/packages/main.panack"
(cd "$variant_dir" && VALIDATION_PROFILE_FILE="$work/profile" \
    make --no-print-directory -o panack-vm package-tool) > "$work/rebuild.out"
grep -F 'package-tool-build' "$work/profile" >/dev/null
status=0
./panack-vm run "$variant_dir/build/tools/panack-packages.bc" invalid > "$work/edited.out" 2> "$work/edited.err" || status=$?
test "$status" = 1
grep -F 'panack-packages inspect --project panack.json' "$work/edited.err" >/dev/null
test "$(shasum -a 256 "$variant_dir/$compiler_seed" | awk '{print $1}')" = "$seed_before"
test ! -e "$variant_dir/$compiler_artifact"
if grep -E 'bootstrap-build|seed-refresh|src/compiler/main.panack' "$work/profile" "$work/rebuild.out" >/dev/null; then
    echo 'package-only behavior edit invoked a compiler or seed build' >&2
    exit 1
fi
test "$(shasum -a 256 "$compiler_seed" | awk '{print $1}')" = "$seed_before"
if [ -n "$compiler_before" ]; then
    test "$(shasum -a 256 "$compiler_artifact" | awk '{print $1}')" = "$compiler_before"
fi

printf 'PASS local directory dependency uses a resolved-project handoff\n'
printf 'PASS package-tool behavior edit rebuilds no compiler or seed artifact\n'
