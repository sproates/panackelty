#!/bin/sh
. tests/unit/harness/common.sh
unset PANACKELTY_BOOTSTRAP_ROOT
prefix=$work/toolchain
project=$work/project
cache=$work/cache
mkdir -p "$prefix/bin" "$prefix/libexec/panackelty" "$prefix/share/panackelty" "$project" "$cache"
chmod 700 "$cache"
cp bin/panack "$prefix/bin/"
cp panack-vm "$prefix/libexec/panackelty/"
cp "${PANACK_TEST_COMPILER:-bootstrap/compiler-v9.bc}" "$prefix/share/panackelty/compiler-v9.bc"
cp build/tools/panack-packages.bc "$prefix/share/panackelty/panack-packages-v1.bc"
cp VERSION "$prefix/share/panackelty/"
cp -R src/stdlib "$prefix/share/panackelty/stdlib"
panack=$prefix/bin/panack
export PANACKELTY_COMPILE_CACHE_DIR=$cache PANACKELTY_COMPILE_CACHE_TRACE=1
cat > "$project/main.panack" <<'SOURCE'
import "value.panack" as value
import stdlib/host as host
main(): Void { print(value.answer()); print(host.arguments().len()) }
SOURCE
printf 'pub pure answer(): Nat { 1 }\n' > "$project/value.panack"
case_name=compilation-cache-cold-warm-and-fresh-execution
capture 0 15 "$panack" compile "$project/main.panack" -o "$work/first.bc"
contains "$work/stderr" 'compile cache: miss'
contains "$work/stderr" 'compile cache: stored'
capture 0 15 "$panack" compile "$project/main.panack" -o "$work/second.bc"
contains "$work/stderr" 'compile cache: hit'
equal_files "$work/first.bc" "$work/second.bc"
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: hit'
printf 'ok\n' > "$work/expected"
equal_files "$work/expected" "$work/stdout"
capture 0 15 "$panack" run "$project/main.panack" first
contains "$work/stderr" 'compile cache: hit'
printf '1\n1\n' > "$work/expected"
equal_files "$work/expected" "$work/stdout"
capture 0 15 "$panack" run "$project/main.panack" first second
contains "$work/stderr" 'compile cache: hit'
printf '1\n2\n' > "$work/expected"
equal_files "$work/expected" "$work/stdout"
pass

case_name=compilation-cache-launcher-and-vm-identity
capture 0 15 env PANACKELTY_STDLIB_PATH="$prefix/share/panackelty/stdlib" PANACKELTY_COMPILER_PATH="$prefix/share/panackelty/compiler-v9.bc" PANACKELTY_VM_PATH="$root/panack-vm" "$root/panack-vm" run "$prefix/share/panackelty/compiler-v9.bc" check "$project/main.panack"
contains "$work/stderr" 'compile cache: disabled'
absent "$work/stderr" 'compile cache: hit'
cp "$root/panack-vm" "$work/alternate-vm"
capture 0 15 env PANACKELTY_STDLIB_PATH="$prefix/share/panackelty/stdlib" PANACKELTY_COMPILER_PATH="$prefix/share/panackelty/compiler-v9.bc" PANACKELTY_VM_PATH="$work/alternate-vm" "$work/alternate-vm" run "$prefix/share/panackelty/compiler-v9.bc" --local-compile-cache check "$project/main.panack"
contains "$work/stderr" 'compile cache: miss'
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: miss'
pass

case_name=compilation-cache-content-not-timestamps
cp -p "$project/value.panack" "$work/source-time"
printf 'pub pure answer(): Nat { 2 }\n' > "$project/value.panack"
touch -r "$work/source-time" "$project/value.panack"
capture 0 15 "$panack" run "$project/main.panack"
contains "$work/stderr" 'compile cache: miss'
printf '2\n0\n' > "$work/expected"
equal_files "$work/expected" "$work/stdout"
pass

case_name=compilation-cache-failures-and-confinement-stay-fresh
cp "$project/value.panack" "$work/good-source"
printf 'pub pure answer(value: Nat): Nat { value }\n' > "$project/value.panack"
capture failure 15 "$panack" check "$project/main.panack"
absent "$work/stderr" 'compile cache: hit'
absent "$work/stderr" 'compile cache: stored'
rm "$project/value.panack"
capture failure 15 "$panack" check "$project/main.panack"
absent "$work/stderr" 'compile cache: hit'
ln -s "$work/good-source" "$project/value.panack"
capture failure 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'symbolic link'
absent "$work/stderr" 'compile cache: hit'
rm "$project/value.panack"
cp "$work/good-source" "$project/value.panack"
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: hit'
pass

case_name=compilation-cache-corrupt-truncated-and-oversized-entries
printf 'extra' >> "$cache/compilation-v1.cache"
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: miss'
contains "$work/stderr" 'compile cache: stored'
printf 'partial' > "$cache/compilation-v1.cache"
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: miss'
dd if=/dev/zero of="$cache/compilation-v1.cache" bs=1 count=0 seek=16777217 2>/dev/null
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: miss'
pass

case_name=compilation-cache-optional-and-outside-project
PANACKELTY_COMPILE_CACHE_DIR= capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: disabled'
PANACKELTY_COMPILE_CACHE_DIR="$project" capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: disabled'
test ! -e "$project/compilation-v1.cache" || fail 'cache written inside project'
export PANACKELTY_COMPILE_CACHE_DIR=$cache
rm "$cache/compilation-v1.cache"
mkdir "$cache/compilation-v1.cache"
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: not stored'
rmdir "$cache/compilation-v1.cache"
pass

case_name=compilation-cache-concurrent-publication
"$panack" compile "$project/main.panack" -o "$work/concurrent-one.bc" > "$work/one.out" 2> "$work/one.err" &
first=$!
"$panack" compile "$project/main.panack" -o "$work/concurrent-two.bc" > "$work/two.out" 2> "$work/two.err" &
second=$!
wait "$first" || fail 'first compiler failed'
wait "$second" || fail 'second compiler failed'
equal_files "$work/concurrent-one.bc" "$work/concurrent-two.bc"
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: hit'
pass

case_name=compilation-cache-compiler-content-identity
compiler=$prefix/share/panackelty/compiler-v9.bc
cp "$compiler" "$work/compiler-original.bc"
# Change one string byte without changing length or the bytecode format. The
# altered compiler still works, but must not borrow the old compiler's cache.
position=$(LC_ALL=C grep -abo 'compile cache: ' "$compiler" | head -1 | cut -d: -f1)
test -n "$position" || fail 'cache trace constant missing'
printf '!' | dd of="$compiler" bs=1 seek=$((position + 13)) conv=notrunc 2>/dev/null
"$root/panack-vm" check "$compiler" > "$work/verified"
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache! miss'
cp "$work/compiler-original.bc" "$compiler"
capture 0 15 "$panack" check "$project/main.panack"
contains "$work/stderr" 'compile cache: miss'
pass

case_name=compilation-cache-resolved-project
mkdir -p "$work/application/src" "$work/library/src"
sed 's/import "value.panack" as value/import library\/api as value/' "$project/main.panack" > "$work/application/src/main.panack"
cp "$project/value.panack" "$work/library/src/api.panack"
version=$(cat VERSION)
cat > "$work/application/panack.json" <<MANIFEST
{"schema":1,"package":"cache.application","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"library":{"path":"../library"}}}
MANIFEST
cat > "$work/library/panack.json" <<MANIFEST
{"schema":1,"package":"cache.library","version":"1.0.0","language":"modules-v1","toolchain":"$version","source":"src","exports":{"api":"api.panack"},"dependencies":{}}
MANIFEST
capture 0 15 "$panack" deps lock --project "$work/application/panack.json"
capture 0 15 "$panack" run --project "$work/application/panack.json"
contains "$work/stderr" 'compile cache: miss'
capture 0 15 "$panack" run --project "$work/application/panack.json" argument
contains "$work/stderr" 'compile cache: hit'
printf '2\n1\n' > "$work/expected"
equal_files "$work/expected" "$work/stdout"
sed 's/"version":"1.0.0"/"version":"1.0.1"/' "$work/application/panack.json" > "$work/manifest-new"
mv "$work/manifest-new" "$work/application/panack.json"
capture failure 15 "$panack" run --project "$work/application/panack.json"
contains "$work/stderr" "PKG_LOCK_STALE"
absent "$work/stderr" "compile cache: hit"
capture 0 15 "$panack" deps update --project "$work/application/panack.json"
capture 0 15 "$panack" run --project "$work/application/panack.json"
contains "$work/stderr" 'compile cache: miss'
pass

case_name=incremental-managed-one-shot-cache
mkdir -p "$work/managed-project" "$work/user-cache"
printf 'import "value.panack" as value\nmain(): Void { print(value.answer()) }\n' > "$work/managed-project/main.panack"
printf 'pub pure answer(): Nat { 10 }\n' > "$work/managed-project/value.panack"
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" compile --incremental "$work/managed-project/main.panack" -o "$work/managed.bc"
contains "$work/stderr" 'compile cache: miss'
test -f "$work/user-cache/panackelty/compiler/compilation-v1.cache" || fail 'automatic cache missing'
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" compile --incremental "$work/managed-project/main.panack" -o "$work/managed-warm.bc"
contains "$work/stderr" 'compile cache: hit'
equal_files "$work/managed.bc" "$work/managed-warm.bc"
pass

case_name=incremental-entry-edit-reuses-unchanged-modules
printf '\n// entry edit\n' >> "$work/managed-project/main.panack"
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" compile --incremental "$work/managed-project/main.panack" -o "$work/edited.bc"
contains "$work/stderr" 'compile cache: module hits 2'
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= "$panack" compile "$work/managed-project/main.panack" -o "$work/edited-fresh.bc"
equal_files "$work/edited.bc" "$work/edited-fresh.bc"
pass

case_name=incremental-transitive-invalidation-and-fresh-diagnostics
printf 'pub pure answer(): Nat { 20 }\n' > "$work/managed-project/value.panack"
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" compile --incremental "$work/managed-project/main.panack" -o "$work/value-edited.bc"
contains "$work/stderr" 'compile cache: module hits 1'
capture 0 15 "$panack" run "$work/value-edited.bc"
printf '20\n' > "$work/expected"
equal_files "$work/expected" "$work/stdout"
printf 'pub pure answer(value: Nat): Nat { value }\n' > "$work/managed-project/value.panack"
capture failure 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" check --incremental "$work/managed-project/main.panack"
cp "$work/stderr" "$work/incremental-error"
capture failure 15 env PANACKELTY_COMPILE_CACHE_DIR= PANACKELTY_COMPILE_CACHE_TRACE= "$panack" check "$work/managed-project/main.panack"
sed '/^compile cache: /d' "$work/incremental-error" > "$work/incremental-error-clean"
equal_files "$work/incremental-error-clean" "$work/stderr"
pass

case_name=incremental-corrupt-module-falls-back
printf 'pub pure answer(): Nat { 20 }\n' > "$work/managed-project/value.panack"
rm "$work/user-cache/panackelty/compiler/compilation-v1.cache"
for file in "$work/user-cache/panackelty/compiler"/module-*.cache; do printf 'broken' > "$file"; done
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" compile --incremental "$work/managed-project/main.panack" -o "$work/recovered.bc"
contains "$work/stderr" 'compile cache: module hits 0'
equal_files "$work/value-edited.bc" "$work/recovered.bc"
# A previous module-format header must also fall back, even with otherwise
# intact payload bytes and content digest.
rm "$work/user-cache/panackelty/compiler/compilation-v1.cache"
for file in "$work/user-cache/panackelty/compiler"/module-*.cache; do
    printf 'PANACK-MODULE-1\n' | dd of="$file" bs=1 conv=notrunc 2>/dev/null
done
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" compile --incremental "$work/managed-project/main.panack" -o "$work/version-recovered.bc"
contains "$work/stderr" 'compile cache: module hits 0'
equal_files "$work/value-edited.bc" "$work/version-recovered.bc"
pass

case_name=incremental-unavailable-default-storage-is-optional
printf 'not a directory' > "$work/blocked-cache"
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/blocked-cache" "$panack" check --incremental "$work/managed-project/main.panack"
contains "$work/stderr" 'compile cache: disabled'
pass

case_name=incremental-global-type-source-invalidates-all-modules
printf 'pub record Box { value: Nat }\npub pure answer(): Nat { 20 }\n' > "$work/managed-project/value.panack"
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" compile --incremental "$work/managed-project/main.panack" -o "$work/type-change.bc"
contains "$work/stderr" 'compile cache: module hits 0'
capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= "$panack" compile "$work/managed-project/main.panack" -o "$work/type-change-fresh.bc"
equal_files "$work/type-change.bc" "$work/type-change-fresh.bc"
pass

# Independent sibling makes actual reuse visible after changing a typed module.
case_name=incremental-body-edit-in-type-module
printf 'import "value.panack" as value\nimport "other.panack" as other\nmain(): Void { print(value.answer()); print(other.answer()) }\n' > "$work/managed-project/main.panack"
printf 'pub pure answer(): Nat { 7 }\n' > "$work/managed-project/other.panack"
cat > "$work/managed-project/value.panack" <<'SOURCE'
pub record Box { value: Nat }
pub enum Choice { First, Second }
pub type Small = Nat where n => n <= 50
pub pure answer(): Nat { 20 }
SOURCE
incremental_compare() {
    capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" compile --incremental "$work/managed-project/main.panack" -o "$work/narrow.bc"
    contains "$work/stderr" "compile cache: module hits $1"
    capture 0 15 env PANACKELTY_COMPILE_CACHE_DIR= "$panack" compile "$work/managed-project/main.panack" -o "$work/narrow-fresh.bc"
    equal_files "$work/narrow.bc" "$work/narrow-fresh.bc"
}
replace_typed_source() {
    sed "$1" "$work/managed-project/value.panack" > "$work/replacement"
    mv "$work/replacement" "$work/managed-project/value.panack"
}
incremental_compare 0
replace_typed_source 's/{ 20 }/{ 123 }/'
incremental_compare 2
capture 0 15 "$panack" run "$work/narrow.bc"
printf '123\n7\n' > "$work/expected"
equal_files "$work/expected" "$work/stdout"
pass

case_name=incremental-type-coordinate-change-invalidates
# A leading multibyte character changes offsets and source locations.
{ printf '// λ\n'; cat "$work/managed-project/value.panack"; } > "$work/replacement"
mv "$work/replacement" "$work/managed-project/value.panack"
incremental_compare 0
pass

case_name=incremental-guard-change-invalidates
replace_typed_source 's/n <= 50/n <= 60/'
incremental_compare 0
pass

case_name=incremental-record-change-invalidates
replace_typed_source 's/value: Nat/value: Int/'
incremental_compare 0
pass

case_name=incremental-enum-change-invalidates
replace_typed_source 's/First, Second/First, Second, Third/'
incremental_compare 0
pass

case_name=incremental-declaration-removal-invalidates
replace_typed_source '/pub enum Choice/d'
incremental_compare 0
pass

case_name=incremental-body-error-in-type-module-stays-fresh
replace_typed_source 's/{ 123 }/{ "wrong" }/'
capture failure 15 env PANACKELTY_COMPILE_CACHE_DIR= XDG_CACHE_HOME="$work/user-cache" "$panack" check --incremental "$work/managed-project/main.panack"
contains "$work/stderr" 'compile cache: module hits 2'
sed '/^compile cache: /d' "$work/stderr" > "$work/narrow-error"
capture failure 15 env PANACKELTY_COMPILE_CACHE_DIR= PANACKELTY_COMPILE_CACHE_TRACE= "$panack" check "$work/managed-project/main.panack"
equal_files "$work/narrow-error" "$work/stderr"
replace_typed_source 's/{ "wrong" }/{ 321 }/'
incremental_compare 2
pass

case_name=incremental-function-edit-before-type-preserves-coordinates
# Even when declaration text is identical, moving it invalidates its origins.
{ printf 'pure helper(): Nat { 1 }\n'; cat "$work/managed-project/value.panack"; } > "$work/replacement"
mv "$work/replacement" "$work/managed-project/value.panack"
incremental_compare 0
replace_typed_source 's/helper(): Nat { 1 }/helper(): Nat { 2 }/'
incremental_compare 2
replace_typed_source 's/helper(): Nat { 2 }/helper(): Nat { 22 }/'
incremental_compare 0
pass

case_name=incremental-import-alias-change-invalidates
printf 'import "value.panack" as renamed\nimport "other.panack" as other\nmain(): Void { print(renamed.answer()); print(other.answer()) }\n' > "$work/managed-project/main.panack"
incremental_compare 0
pass
