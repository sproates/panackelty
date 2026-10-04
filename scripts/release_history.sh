#!/bin/sh
# Render only the documented changelog subset, without an interpreter dependency.
set -eu
[ "$#" = 1 ] || { echo 'usage: release_history.sh SOURCE_ROOT' >&2; exit 1; }
root=$1
script=$(CDPATH= cd "$(dirname "$0")" && pwd)
published=$(cat "$root/site/native-release.txt")
printf '%s\n' "$published" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+-alpha\.[0-9]+$'
browser=$(awk -F '"' '{ for(i=2;i<=NF;i+=2) if($i=="tag") print $(i+2) }' "$root/site/playground.json")
printf '%s\n' "$browser" | awk 'BEGIN { valid=1 } !/^v[0-9]+\.[0-9]+\.[0-9]+$/ { valid=0 } END { exit !(valid && NR==1) }'
fragment=$(mktemp)
trap 'rm -f "$fragment" "$fragment.page"' 0
trap 'exit 1' HUP INT TERM
awk -v published="$published" -v browser="$browser" -f "$script/release_history.awk" "$root/CHANGELOG.md" > "$fragment"
awk -v fragment="$fragment" '
  $0 == "<!-- RELEASE_HISTORY -->" { count++; while((getline line < fragment)>0) print line; close(fragment); next }
  { print }
  END { if(count!=1) exit 1 }
' "$script/release_history.html" > "$fragment.page"
awk -v page=history -f "$script/site_chrome.awk" "$root/site/index.html" "$fragment.page"
