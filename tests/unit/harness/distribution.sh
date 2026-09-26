#!/bin/sh
. tests/unit/harness/common.sh
. tests/unit/harness/archive.sh
# Nested make must not inherit the outer dry-run flags or command-line overrides.
unset MAKEFLAGS MFLAGS MAKELEVEL CFLAGS CPPFLAGS LDFLAGS LDLIBS
case_name=configurable-native-build-flags
capture 0 15 make --always-make --dry-run native
for flag in -O2 -std=c11 -Wall -Wextra -Werror -pedantic; do token_present "$work/stdout" "$flag"; done
capture 0 15 make --always-make --dry-run native 'CFLAGS=-O0 -g' CPPFLAGS=-DPANACK_BUILD_TEST=1 LDFLAGS=-L/tmp LDLIBS=-lm
for flag in -O0 -g -DPANACK_BUILD_TEST=1 -L/tmp -lm -std=c11 -Wall -Wextra -Werror -pedantic; do token_present "$work/stdout" "$flag"; done
absent "$work/stdout" '-O2'
pass
case_name=checksum-from-spaced-checkout
checkout=$work/'checkout with spaces (test)'
mkdir "$checkout"
for name in Makefile VERSION panack panack-vm bootstrap src tests examples README.md LICENSE CHANGELOG.md RELEASE_POLICY.md SECURITY.md SPEC.md; do
    ln -s "$root/$name" "$checkout/$name"
done
cd "$checkout"
capture 0 90 make quick-start
set -- "$checkout"/build/panackelty-*.tar.gz
test "$#" = 1 && test -f "$1" || fail 'expected exactly one archive'
archive=$1
archive_name=${archive##*/}
test -f "$archive.sha256" || fail 'missing checksum'
if command -v sha256sum >/dev/null 2>&1; then
    digest=$(sha256sum < "$archive" | awk '{print $1}')
else
    digest=$(shasum -a 256 < "$archive" | awk '{print $1}')
fi
printf '%s  %s\n' "$digest" "$archive_name" > "$work/checksum"
equal_files "$archive.sha256" "$work/checksum"
pass
case_name=archive-members-and-relocation
cd "$root"
capture 0 90 make package-archive "BUILD_DIR=$work/archive-build"
set -- "$work"/archive-build/panackelty-*.tar.gz
test "$#" = 1 && test -f "$1" || fail 'expected exactly one custom-build archive'
archive=$1
archive_contract "$archive" || fail 'unsafe archive paths, types or owner metadata'
mkdir "$work/extracted"
tar -xzf "$archive" -C "$work/extracted"
(cd "$work/extracted/panackelty" && find . -type f | sed 's|^./||' | sort) > "$work/files"
{
    cat "$root/tests/unit/harness/installed-files.txt"
    printf 'LICENSE\nREADME.md\nexamples/README.md\n'
    for example in "$root"/examples/*.panack; do printf 'examples/%s\n' "${example##*/}"; done
} | sort > "$work/expected"
equal_files "$work/files" "$work/expected"
mkdir "$work/relocated"
mv "$work/extracted/panackelty" "$work/relocated/toolchain"
version=$(cat "$root/VERSION")
printf 'panack %s (bytecode 8)\n' "$version" > "$work/version"
cat > "$work/logical-import.panack" <<'PROGRAM'
import stdlib/option
import stdlib/time
import stdlib/path
main(): Void {
  match Some(42) { Some(value) => print(value), None() => print(0) }
  print(duration_ticks(duration_seconds(1)))
  print(path_display(path_current()))
}
PROGRAM
printf '42\n1000000000\n.\n' > "$work/import.stdout"
# Empty stdlib override prevents the source checkout from masking missing resources.
unset PANACKELTY_STDLIB_PATH
cd "$work"
command=$work/relocated/toolchain/bin/panack
capture 0 15 "$command" --version
equal_files "$work/stdout" "$work/version"
test ! -s "$work/stderr" || fail 'version stderr'
capture 0 30 "$command" run "$work/logical-import.panack"
equal_files "$work/stdout" "$work/import.stdout"
test ! -s "$work/stderr" || fail 'relocated import stderr'
capture 0 30 "$command" run "$work/relocated/toolchain/examples/collections_and_bytes.panack"
equal_files "$work/stdout" "$root/tests/functional/expected/examples/collections_and_bytes.stdout"
pass
case_name=installed-files-and-logical-imports
cd "$root"
destination=$work/'installation with spaces (test)'
capture 0 30 make install "DESTDIR=$destination" PREFIX=/usr/local
installed=$destination/usr/local
(cd "$installed" && find . -type f | sed 's|^./||' | sort) > "$work/files"
equal_files "$work/files" "$root/tests/unit/harness/installed-files.txt"
cd "$work"
capture 0 15 "$installed/bin/panack" --help
contains "$work/stdout" 'usage: panack '
capture 0 15 "$installed/bin/panack" --version
equal_files "$work/stdout" "$work/version"
test ! -s "$work/stderr" || fail 'installed version stderr'
capture 0 30 "$installed/bin/panack" run "$work/logical-import.panack"
equal_files "$work/stdout" "$work/import.stdout"
test ! -s "$work/stderr" || fail 'installed import stderr'
pass
case_name=compiler-artifact-from-spaced-checkout
# Use real copied sources, as source.path must remain inside this checkout.
rm "$checkout/src" "$checkout/examples"
cp -R "$root/src" "$checkout/src"
cp -R "$root/examples" "$checkout/examples"
for stage in stage1 stage2; do
    mkdir -p "$checkout/build/bootstrap/$stage"
    cp "$root/bootstrap/compiler-v8.bc" "$checkout/build/bootstrap/$stage/compiler.bc"
done
cd "$checkout"
capture 0 30 env PANACK_TEST_COMPILER="$checkout/build/bootstrap/stage2/compiler.bc" ./panack run tests/runner/compiler_driver.panack
contains "$work/stdout" 'PASS byte-identical compiler output'
test "$(tail -n 1 "$work/stdout")" = 'tests: 8, failures: 0' || fail 'compiler driver summary'
pass
case_name=archive-rejects-invalid-metadata-and-links
cd "$work"
mkdir -p forged/panackelty
printf 'fixture\n' > forged/panackelty/file
case "$(uname -s)" in
    Darwin) owner_flags='--uid 0 --gid 0 --uname root --gname root'; bad_flags='--uid 1 --gid 1 --uname other --gname other' ;;
    *) owner_flags='--owner=root --group=root'; bad_flags='--owner=1 --group=1' ;;
esac
# Deliberately split these fixed option lists, never user input.
COPYFILE_DISABLE=1 tar $owner_flags -C forged -czf good.tar.gz panackelty
archive_contract good.tar.gz || fail 'archive checker rejects valid control'
COPYFILE_DISABLE=1 tar $bad_flags -C forged -czf bad.tar.gz panackelty
if archive_contract bad.tar.gz; then fail 'archive checker accepts wrong ownership'; fi
ln -s /tmp forged/panackelty/link
COPYFILE_DISABLE=1 tar $owner_flags -C forged -czf bad.tar.gz panackelty
if archive_contract bad.tar.gz; then fail 'archive checker accepts symlink'; fi
COPYFILE_DISABLE=1 tar $owner_flags -C forged/panackelty -czf bad.tar.gz file
if archive_contract bad.tar.gz; then fail 'archive checker accepts wrong top-level directory'; fi
pass
