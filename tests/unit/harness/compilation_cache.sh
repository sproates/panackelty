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
