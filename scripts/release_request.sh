#!/bin/sh
# Validate release identity before any build or publication; emit reviewed notes.
set -eu
export LC_ALL=C
fail() { echo "release request: $*" >&2; exit 1; }
[ "$#" -eq 5 ] || fail 'expected event, ref, commit, requested version and commit'
event=$1 ref=$2 commit=$3 requested_version=$4 requested_commit=$5
version=$(cat VERSION)
case "$version" in *[!0-9.alpha-]*) fail 'invalid preview version' ;; esac
printf '%s\n' "$version" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+-alpha\.[0-9]+$' || fail 'invalid preview version'
[ "${#commit}" -eq 40 ] || fail 'invalid source commit'
case "$commit" in *[!0-9a-f]*) fail 'invalid source commit' ;; esac
case "$event" in
    workflow_dispatch)
        [ "$ref" = refs/heads/main ] || fail 'manual releases require main'
        [ "$requested_version" = "$version" ] || fail 'version confirmation mismatch'
        [ "$requested_commit" = "$commit" ] || fail 'commit confirmation mismatch'
        ;;
    push)
        [ "$ref" = "refs/tags/v$version" ] || fail 'noncanonical version tag'
        ;;
    *) fail 'unsupported event' ;;
esac
# Exactly one nonempty changelog section, delimited by the next level-2 heading.
awk -v version="$version" '
    /^## / { selected=($2==version); if (selected) count++; next }
    selected { notes=notes $0 "\n"; if ($0 ~ /[^[:space:]]/) nonempty=1 }
    END {
        if (count != 1 || !nonempty) exit 1
        printf "Panackelty %s\n%s", version, notes
    }
' CHANGELOG.md || fail 'missing, empty or duplicate changelog section'
