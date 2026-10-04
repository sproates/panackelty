#!/bin/sh
. tests/unit/harness/common.sh
script=$root/scripts/install.sh
mkdir -p "$work/bin" "$work/assets" "$work/package/panackelty/bin"
cat > "$work/bin/curl" <<'STUB'
#!/bin/sh
set -eu
output=
while [ "$#" -gt 0 ]; do
    if [ "$1" = --output ]; then output=$2; shift; fi
    url=$1
    shift
done
case "${INSTALL_FAILURE:-}" in
    network) exit 22 ;;
    interrupt) kill -TERM "$PPID"; exit 1 ;;
esac
cp "$INSTALL_ASSETS/${url##*/}" "$output"
STUB
chmod +x "$work/bin/curl"
INSTALL_ASSETS=$work/assets
export INSTALL_ASSETS
PATH=$work/bin:$PATH
export PATH
# Exercise release targets on either source-build host family without requiring
# the host itself to satisfy release architecture or OS-version constraints.
case "$(uname -s)" in
    Linux) platform=linux-x86_64; fixture_system=Linux; fixture_machine=x86_64 ;;
    Darwin) platform=macos-arm64; fixture_system=Darwin; fixture_machine=arm64 ;;
    *) fail 'installer tests require Linux or macOS tools' ;;
esac
export fixture_system fixture_machine
cat > "$work/bin/uname" <<'STUB'
#!/bin/sh
case "$1" in -s) printf '%s\n' "$fixture_system" ;; -m) printf '%s\n' "$fixture_machine" ;; *) exit 1 ;; esac
STUB
real_sed=$(command -v sed)
export real_sed
cat > "$work/bin/sed" <<'STUB'
#!/bin/sh
if [ "${3:-}" = /etc/os-release ]; then
    case "$2" in 's/^ID=//p') printf 'ubuntu\n' ;; 's/^VERSION_ID=//p') printf '22.04\n' ;; *) exit 1 ;; esac
else
    exec "$real_sed" "$@"
fi
STUB
cat > "$work/bin/sw_vers" <<'STUB'
#!/bin/sh
printf '14.0\n'
STUB
chmod +x "$work/bin/uname" "$work/bin/sed" "$work/bin/sw_vers"
fixture() {
    fixture_version=$1
    printf '#!/bin/sh\nprintf "panack %s (bytecode 9)\\n"\n' "$fixture_version" > "$work/package/panackelty/bin/panack"
    # Model the real launcher's shell-only version path and native help path.
    printf '\n[ "${1:-}" != --help ] || exec "$(dirname "$0")/panack-native"\n' >> "$work/package/panackelty/bin/panack"
    printf '#!/bin/sh\nexit 0\n' > "$work/package/panackelty/bin/panack-native"
    chmod +x "$work/package/panackelty/bin/panack" "$work/package/panackelty/bin/panack-native"
    asset=panackelty-$fixture_version-$platform.tar.gz
    tar -czf "$work/assets/$asset" -C "$work/package" panackelty
    checksum
}
checksum() {
    (cd "$work/assets"; if command -v sha256sum >/dev/null; then sha256sum "$asset"; else shasum -a 256 "$asset"; fi) > "$work/assets/$asset.sha256"
}
run_install() { env -u SUDO_USER HOME="$work/home ' dollar\$ back\\slash space" sh "$script" "$@" > "$work/out" 2> "$work/error"; }
reject() {
    message=$1; shift
    if run_install "$@"; then fail "accepted: $message"; fi
    contains "$work/error" "$message"
}
home="$work/home ' dollar\$ back\\slash space"
prefix=$home/.local/opt/panackelty-installer
link=$home/.local/bin/panack
case_name=installer-argument-and-foreign-file-protection
reject 'invalid version' --version '../escape'
reject 'unknown argument' --wat
mkdir -p "${link%/*}"
printf 'my command\n' > "$link"
reject 'refusing unrelated command'
contains "$link" 'my command'
rm "$link"
mkdir -p "$prefix"
reject 'refusing unmarked location'
rmdir "$prefix"
pass
case_name=installer-clean-repeat-upgrade-and-rollback
fixture 0.1.0-alpha.10
cp "$work/assets/$asset" "$work/pristine.tar.gz"
cp "$work/assets/$asset.sha256" "$work/pristine.sha256"
run_install || { cat "$work/error"; fail install; }
contains "$work/out" 'export PATH='
first=$(readlink "$prefix/current")
test "$("$link" --version)" = 'panack 0.1.0-alpha.10 (bytecode 9)' || fail 'installed command'
run_install || fail repeat
test "$(readlink "$prefix/current")" = "$first" || fail repeat-link
fixture 0.1.0-alpha.11
run_install --version 0.1.0-alpha.11 || fail upgrade
test "$("$link" --version)" = 'panack 0.1.0-alpha.11 (bytecode 9)' || fail upgraded-command
test -d "$prefix/$first" || fail 'old release not retained'
run_install --version 0.1.0-alpha.10 || fail rollback
test "$(readlink "$prefix/current")" = "$first" || fail rollback-link
pass
case_name=installer-failures-preserve-existing-command
INSTALL_FAILURE=network; export INSTALL_FAILURE
# Check transport failure status without depending on curl's platform-specific wording.
if run_install --version 0.1.0-alpha.11; then fail network; fi
unset INSTALL_FAILURE
printf 'corrupt' >> "$work/assets/$asset"
reject 'checksum mismatch' --version 0.1.0-alpha.11
test "$(readlink "$prefix/current")" = "$first" || fail 'activated failed download'
printf '%s\n' 'malformed checksum' > "$work/assets/$asset.sha256"
reject 'invalid checksum file' --version 0.1.0-alpha.11
fixture 0.1.0-alpha.11
ln -s /tmp "$work/package/panackelty/escape"
fixture 0.1.0-alpha.11
reject 'unsafe archive member types' --version 0.1.0-alpha.11
rm "$work/package/panackelty/escape"
# An archive with another root must fail even with a correct checksum.
tar -czf "$work/assets/$asset" -C "$work/package/panackelty" bin
checksum
reject 'unsafe archive paths' --version 0.1.0-alpha.11
fixture 0.1.0-alpha.11
printf '#!/bin/sh\nprintf "wrong version\\n"\n' > "$work/package/panackelty/bin/panack"
tar -czf "$work/assets/$asset" -C "$work/package" panackelty
checksum
reject 'version mismatch' --version 0.1.0-alpha.11
fixture 0.1.0-alpha.11
printf '#!/bin/sh\nexit 1\n' > "$work/package/panackelty/bin/panack-native"
tar -czf "$work/assets/$asset" -C "$work/package" panackelty
checksum
reject 'downloaded toolchain cannot run native compiler' --version 0.1.0-alpha.11
test "$(readlink "$prefix/current")" = "$first" || fail 'activated unusable native toolchain'
asset=panackelty-0.1.0-alpha.10-$platform.tar.gz
cp "$work/pristine.tar.gz" "$work/assets/$asset"
cp "$work/pristine.sha256" "$work/assets/$asset.sha256"
chmod -x "$prefix/$first/bin/panack-native"
reject 'installed release cannot run native compiler'
chmod +x "$prefix/$first/bin/panack-native"
chmod -x "$prefix/$first/bin/panack"
reject 'installed release cannot run'
chmod +x "$prefix/$first/bin/panack"
printf 'user change\n' >> "$prefix/$first/bin/panack"
reject 'installed release differs'
contains "$prefix/$first/bin/panack" 'user change'
pass
case_name=installer-lock-interruption-and-removal
mkdir "$prefix/.lock"
reject 'installation locked'
rmdir "$prefix/.lock"
INSTALL_FAILURE=interrupt; export INSTALL_FAILURE
if run_install; then fail interrupt; fi
unset INSTALL_FAILURE
test ! -d "$prefix/.lock" || fail 'signal left lock'
for candidate in "$prefix"/.stage.*; do test ! -e "$candidate" || fail 'stage leaked'; done
# Refuse a foreign command even during removal, preserving both trees.
rm "$link"
printf 'foreign\n' > "$link"
reject 'refusing unrelated command' --uninstall
test -d "$prefix/$first" || fail 'removed toolchain after conflict'
rm "$link"
ln -s "$prefix/current/bin/panack" "$link"
run_install --uninstall || fail uninstall
test ! -e "$prefix" && test ! -L "$link" || fail 'removal incomplete'
run_install --uninstall || fail 'repeat uninstall'
ln -s "$prefix/current/bin/panack" "$link"
run_install --uninstall || fail 'dangling owned link uninstall'
test ! -L "$link" || fail 'dangling owned link remains'
pass
case_name=installer-unsupported-target
cat > "$work/bin/uname" <<'STUB'
#!/bin/sh
printf 'unsupported\n'
STUB
chmod +x "$work/bin/uname"
reject 'supported targets:'
pass
