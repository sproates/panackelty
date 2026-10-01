#!/bin/sh
# Run even when the classifier failed or was cancelled; never accept a skip.
set -eu
if [ "${CI_SCOPE_RESULT-}" != success ]; then
    echo 'validation routing did not succeed' >&2
    exit 1
fi
case "${CI_SCOPE_ROUTE-}" in
    docs)
        if [ "${CI_VALIDATION_RESULT-}" != skipped ]; then
            echo 'unexpected full-validation result for documentation route' >&2
            exit 1
        fi
        echo 'Informational documentation checks passed; full validation is not applicable.' ;;
    website)
        if [ "${CI_VALIDATION_RESULT-}" != skipped ] || [ "${CI_WEBSITE_RESULT-}" != success ]; then
            echo 'website validation did not succeed or native route was inconsistent' >&2
            exit 1
        fi
        echo 'Complete website validation passed; native builds are not applicable.' ;;
    full)
        if [ "${CI_VALIDATION_RESULT-}" != success ]; then
            echo 'full validation did not succeed' >&2
            exit 1
        fi
        echo 'Full validation passed.' ;;
    *) echo 'missing or invalid validation route' >&2; exit 1 ;;
esac
