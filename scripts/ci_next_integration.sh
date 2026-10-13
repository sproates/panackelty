#!/bin/sh
# Identify next branch runs whose distribution build is supplied nightly.
set -eu
[ "$#" -eq 3 ] || { echo 'expected event, pull-request base and ref' >&2; exit 2; }
event=$1
base=$2
ref=$3

if [ "$event" = pull_request ] && [ "$base" = next ]; then
    echo true
elif [ "$event" = push ] && [ "$ref" = refs/heads/next ]; then
    echo true
else
    echo false
fi
