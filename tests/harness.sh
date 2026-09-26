#!/bin/sh
# Native development harness contracts.
set -eu
root=$(pwd -P)
workspace=$(mktemp -d "${TMPDIR:-/tmp}/panack-harness-command.XXXXXX")
trap 'rm -rf "$workspace"' 0
trap 'exit 1' HUP INT TERM
HARNESS_COMMAND=$workspace/command.bc
export HARNESS_COMMAND
./panack compile tests/runner/harness_command.panack -o "$HARNESS_COMMAND"
case "${1:-all}" in
    all) groups='layout release validation bootstrap runner distribution' ;;
    compiler) groups='bootstrap runner' ;;
    *) echo 'usage: sh tests/harness.sh [all|compiler]' >&2; exit 1 ;;
esac
for group in $groups; do
    sh "$root/tests/unit/harness/$group.sh"
done
