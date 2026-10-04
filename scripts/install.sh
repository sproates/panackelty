#!/bin/sh
# Optional user-owned installer. Inspect this file before executing it.
main() {
set -eu
LC_ALL=C
export LC_ALL
umask 077
fail() { printf 'panack installer: %s\n' "$*" >&2; exit 1; }
version=0.1.0-alpha.10
remove=false
while [ "$#" -gt 0 ]; do
    case "$1" in
        --version) [ "$#" -ge 2 ] || fail '--version requires a value'; version=$2; shift ;;
        --uninstall) remove=true ;;
        --help) printf '%s\n' 'Usage: sh install.sh [--version 0.1.0-alpha.10 | --uninstall]' 'Uses $HOME/.local/opt/panackelty-installer and $HOME/.local/bin/panack.' 'Never edits shell startup files; do not run with sudo.'; exit 0 ;;
        *) fail "unknown argument: $1" ;;
    esac
    shift
done
printf '%s\n' "$version" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+(-[a-z]+\.[0-9]+)?$' || fail 'invalid version'
case "${HOME:-}" in /*) ;; *) fail 'HOME must be an absolute path' ;; esac
case "$HOME" in *'
'*) fail 'HOME must not contain a newline' ;; esac
[ -z "${SUDO_USER:-}" ] || fail 'run as your own user without sudo'
prefix=$HOME/.local/opt/panackelty-installer
command_path=$HOME/.local/bin/panack
exists() { [ -e "$1" ] || [ -L "$1" ]; }
owned_link() { [ -L "$command_path" ] && [ "$(readlink "$command_path")" = "$prefix/current/bin/panack" ]; }
if exists "$command_path"; then owned_link || fail "refusing unrelated command: $command_path"; fi
if exists "$prefix"; then
    [ -d "$prefix" ] && [ ! -L "$prefix" ] || fail "refusing unrelated location: $prefix"
    [ -f "$prefix/.installer" ] && [ ! -L "$prefix/.installer" ] &&
        [ "$(cat "$prefix/.installer")" = 'panackelty-installer-v1' ] || fail "refusing unmarked location: $prefix"
else
    if "$remove"; then
        if owned_link; then rm "$command_path"; fi
        printf '%s\n' 'Nothing installed; any owned command link removed.'
        exit 0
    fi
    mkdir -p "$HOME/.local/opt"
    mkdir "$prefix" || fail "cannot create $prefix"
    printf '%s\n' panackelty-installer-v1 > "$prefix/.installer"
fi
mkdir "$prefix/.lock" 2>/dev/null || fail "installation locked: $prefix/.lock (see README interruption recovery)"
stage=
cleanup() {
    [ -z "$stage" ] || rm -rf "$stage"
    rmdir "$prefix/.lock" 2>/dev/null || :
}
trap cleanup 0
trap 'exit 1' HUP INT TERM
if "$remove"; then
    # The marker dedicates this entire tree to this installer. Do not store projects here.
    if owned_link; then rm "$command_path"; fi
    rm -rf "$prefix"
    printf '%s\n' 'Removed the installer-managed toolchains and command link.'
    exit 0
fi
case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        [ -f /etc/os-release ] || fail 'requires Ubuntu 22.04 or newer'
        # OS metadata is read as data, not sourced as shell code.
        os_id=$(sed -n 's/^ID=//p' /etc/os-release | tr -d '"')
        os_version=$(sed -n 's/^VERSION_ID=//p' /etc/os-release | tr -d '"')
        [ "$os_id" = ubuntu ] || fail 'supported Linux baseline is Ubuntu 22.04 or newer'
        awk -v v="$os_version" 'BEGIN { exit !(v ~ /^[0-9]+\.[0-9]+$/ && v+0 >= 22.04) }' || fail 'requires Ubuntu 22.04 or newer'
        platform=linux-x86_64
        ;;
    Darwin-arm64)
        major=$(sw_vers -productVersion | cut -d . -f 1)
        case "$major" in ''|*[!0-9]*) fail 'cannot detect macOS version' ;; esac
        [ "$major" -ge 14 ] || fail 'requires macOS 14 or newer'
        platform=macos-arm64
        ;;
    *) fail 'supported targets: Ubuntu 22.04+ x86_64 and macOS 14+ arm64' ;;
esac
for utility in curl tar mktemp; do command -v "$utility" >/dev/null || fail "missing command: $utility"; done
if [ "$platform" = linux-x86_64 ]; then
    command -v sha256sum >/dev/null || fail 'missing command: sha256sum'
else
    command -v shasum >/dev/null || fail 'missing command: shasum'
fi
[ ! -L "$prefix/releases" ] || fail 'refusing symlinked releases directory'
mkdir -p "$prefix/releases" "$HOME/.local/bin"
if exists "$prefix/current"; then
    [ -L "$prefix/current" ] || fail 'refusing unrelated current path'
    case "$(readlink "$prefix/current")" in releases/*) ;; *) fail 'refusing unrelated current link' ;; esac
fi
stage=$(mktemp -d "$prefix/.stage.XXXXXX")
archive=panackelty-$version-$platform.tar.gz
url=https://github.com/sproates/panackelty/releases/download/v$version
fetch() { curl --proto '=https' --proto-redir '=https' --tlsv1.2 --fail --location --silent --show-error --connect-timeout 30 --max-time 300 --output "$stage/$1" "$url/$1"; }
fetch "$archive"
fetch "$archive.sha256"
# Accept only one checksum for exactly this basename, never arbitrary checksum paths.
expected=$(awk -v name="$archive" 'NF == 2 && length($1) == 64 && $1 !~ /[^0-9a-fA-F]/ && ($2 == name || $2 == "*" name) { digest=$1; valid++ } END { if (NR != 1 || valid != 1) exit 1; print tolower(digest) }' "$stage/$archive.sha256") || fail 'invalid checksum file'
if [ "$platform" = linux-x86_64 ]; then actual=$(sha256sum < "$stage/$archive"); else actual=$(shasum -a 256 < "$stage/$archive"); fi
actual=${actual%% *}
[ "$actual" = "$expected" ] || fail 'checksum mismatch; nothing activated'
# Released packages contain only files/directories rooted in panackelty/.
# Reject links, traversal and unexpected member types before extraction.
tar -tzf "$stage/$archive" > "$stage/members"
awk 'index($0,"panackelty/") != 1 || $0 ~ /(^|\/)\.\.?($|\/)/ || $0 ~ /\\/ { bad=1 } END { exit (NR == 0 || bad) }' "$stage/members" || fail 'unsafe archive paths'
tar -tvzf "$stage/$archive" > "$stage/types"
awk 'substr($0,1,1) != "-" && substr($0,1,1) != "d" { bad=1 } END { exit (NR == 0 || bad) }' "$stage/types" || fail 'unsafe archive member types'
mkdir "$stage/unpack"
tar -xzf "$stage/$archive" -C "$stage/unpack"
tool=$stage/unpack/panackelty
[ -x "$tool/bin/panack" ] || fail 'archive has no executable panack'
reported=$("$tool/bin/panack" --version) || fail 'downloaded toolchain cannot run'
case "$reported" in "panack $version (bytecode "*")") ;; *) fail 'downloaded toolchain version mismatch' ;; esac
"$tool/bin/panack" --help >/dev/null || fail 'downloaded toolchain cannot run native compiler'
slot=$version-$platform-$expected
destination=$prefix/releases/$slot
if exists "$destination"; then
    [ -d "$destination" ] && [ ! -L "$destination" ] || fail 'refusing unrelated release destination'
    diff -qr "$tool" "$destination" >/dev/null || fail 'installed release differs; refusing overwrite'
    [ -x "$destination/bin/panack" ] && [ "$("$destination/bin/panack" --version)" = "$reported" ] || fail 'installed release cannot run; refusing activation'
    "$destination/bin/panack" --help >/dev/null || fail 'installed release cannot run native compiler; refusing activation'
else
    mv "$tool" "$destination"
fi
ln -s "releases/$slot" "$stage/current"
# Rename the symlink itself, never follow the existing directory symlink.
if [ "$platform" = linux-x86_64 ]; then mv -fT "$stage/current" "$prefix/current"; else mv -fh "$stage/current" "$prefix/current"; fi
if ! exists "$command_path"; then ln -s "$prefix/current/bin/panack" "$command_path"; fi
printf 'Installed %s\nCommand: %s\n' "$reported" "$command_path"
printf '%s\n' 'To opt in for this shell: export PATH="$HOME/.local/bin:$PATH"' 'To persist it, add that line to your chosen shell startup file.'
}
main "$@"
