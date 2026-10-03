#!/bin/sh
# Execute the current README against the published archive on its native host.
# Network access is intentional: this gate is separate from offline make check.
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
case "$(uname -s)-$(uname -m)" in
  Linux-x86_64) platform=linux-x86_64 ;;
  Darwin-arm64) platform=macos-arm64 ;;
  *) echo 'README release installation: unsupported host' >&2; exit 1 ;;
esac

temporary=$(mktemp -d "${TMPDIR:-/tmp}/panack-readme-install.XXXXXX")
trap 'rm -rf "$temporary"' EXIT HUP INT TERM

extract() {
  awk -v marker="$1" '
    $0 == "<!-- " marker "-begin -->" { capture = 1; starts++; next }
    $0 == "<!-- " marker "-end -->" { capture = 0; ends++; next }
    capture && !/^```/ { print }
    END { if (starts != 1 || ends != 1 || capture) exit 1 }
  ' "$root/README.md" > "$2"
  test -s "$2"
}

extract "release-install-$platform" "$temporary/install.sh"
extract quick-start-program "$temporary/hello.panack"
extract release-install-commands "$temporary/commands.sh"
extract quick-start-output "$temporary/expected.stdout"
extract release-install-path "$temporary/path.sh"
cd "$temporary"
sh install.sh
sh -eu commands.sh > actual.stdout 2> actual.stderr
test ! -s actual.stderr
cmp expected.stdout actual.stdout
cat actual.stdout

# Only the isolated child receives a temporary home; never touch the user's home.
env HOME="$temporary/home" sh -eu path.sh
test "$("$temporary/home/.local/bin/panack" --version)" = \
  "$(head -n 1 expected.stdout)"
echo "README release installation ($platform): ok"
