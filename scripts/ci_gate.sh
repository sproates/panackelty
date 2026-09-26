#!/bin/sh
# Run even when the classifier failed or was cancelled; never accept a skip.
set -eu
if [ "${CI_SCOPE_RESULT-}" != success ]; then
    echo 'validation routing did not succeed' >&2
    exit 1
fi
case "${CI_SCOPE_ROUTE-}" in
    docs) echo 'Informational documentation checks passed; full validation is not applicable.' ;;
    full) echo 'Full validation is required.' ;;
    *) echo 'missing or invalid validation route' >&2; exit 1 ;;
esac
