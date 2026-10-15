#!/bin/sh
# Installed next to panack; resolve the tool from this installation, not cwd.
set -eu
bin=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
exec "$bin/panack" run "$bin/../share/panackelty/http-server-v1.bc" "$@"
