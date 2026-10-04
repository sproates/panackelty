#!/bin/sh
# Network acceptance of the optional installer against the actual published release.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-installer-release.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
extract() {
    awk -v marker="$1" '
      $0 == "<!-- " marker "-begin -->" { capture=1; starts++; next }
      $0 == "<!-- " marker "-end -->" { capture=0; ends++; next }
      capture && !/^```/ { print }
      END { if (starts != 1 || ends != 1 || capture) exit 1 }
    ' "$root/README.md" > "$2"
}
HOME="$work/home with spaces"
export HOME
unset SUDO_USER
sh "$root/scripts/install.sh"
extract quick-start-program "$work/hello.panack"
extract quick-start-output "$work/expected"
extract release-install-commands "$work/commands"
# Run the identical README program/workflow through the installer-owned command.
sed 's|\./panackelty/bin/panack|"$HOME/.local/bin/panack"|g' "$work/commands" > "$work/installed-commands"
cd "$work"
sh -eu installed-commands > actual 2> errors
test ! -s errors
cmp expected actual
cat actual
sh "$root/scripts/install.sh" --version 0.1.0-alpha.10
sh "$root/scripts/install.sh" --uninstall
test ! -e "$HOME/.local/opt/panackelty-installer"
test ! -L "$HOME/.local/bin/panack"
printf '%s\n' 'Published-release installer acceptance: ok'
