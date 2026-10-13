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
        [ "${CI_WORKFLOW_RESULT-}" = skipped ] || {
            echo 'unexpected workflow-validation result for documentation route' >&2
            exit 1
        }
        echo 'Informational documentation checks passed; full validation is not applicable.' ;;
    workflow)
        [ "${CI_VALIDATION_RESULT-}" = skipped ] || {
            echo 'product validation unexpectedly ran for a workflow-only route' >&2
            exit 1
        }
        [ "${CI_WORKFLOW_RESULT-}" = success ] || {
            echo 'workflow configuration validation did not succeed' >&2
            exit 1
        }
        echo 'Workflow syntax and repository contracts passed; product validation is not applicable.' ;;
    full)
        if [ "${CI_VALIDATION_RESULT-}" != success ]; then
            echo 'full validation did not succeed' >&2
            exit 1
        fi
        [ "${CI_WORKFLOW_RESULT-}" = skipped ] || {
            echo 'unexpected workflow-validation result for full validation' >&2
            exit 1
        }
        echo 'Full validation passed.' ;;
    *) echo 'missing or invalid validation route' >&2; exit 1 ;;
esac
