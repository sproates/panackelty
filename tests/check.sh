#!/bin/sh
# Share a successful fixture-runner transcript only within this check invocation.
set -eu
session=$(mktemp -d "${TMPDIR:-/tmp}/panack-check.XXXXXX")
trap 'rm -rf "$session"' 0
trap 'exit 1' HUP INT TERM
PANACK_CHECK_RUNNER_REPORT=$session/runner.report
export PANACK_CHECK_RUNNER_REPORT
unset PANACK_TEST_RUNNER_REPORT PANACK_TEST_CAPTURE_RUNNER_REPORT
"$@"
