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
        [ "${CI_NEXT_SMOKE_RESULT-skipped}" = skipped ] || {
            echo 'unexpected next smoke for documentation route' >&2
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
        [ "${CI_NEXT_SMOKE_RESULT-skipped}" = skipped ] || {
            echo 'unexpected next smoke for a workflow-only route' >&2
            exit 1
        }
        echo 'Workflow syntax and repository contracts passed; product validation is not applicable.' ;;
    full)
        [ "${CI_WORKFLOW_RESULT-}" = skipped ] || {
            echo 'unexpected workflow-validation result for full validation' >&2
            exit 1
        }
        if [ "${CI_NEXT_INTEGRATION-}" = true ]; then
            [ "${CI_VALIDATION_RESULT-}" = skipped ] || {
                echo 'full validation unexpectedly ran for next integration' >&2
                exit 1
            }
            case "${CI_EVENT_NAME-}" in
                pull_request)
                    [ "${CI_NEXT_SMOKE_RESULT-}" = success ] || {
                        echo 'next pull request smoke did not succeed' >&2
                        exit 1
                    } ;;
                push)
                    [ "${CI_NEXT_SMOKE_RESULT-}" = skipped ] || {
                        echo 'post-merge next push must not rerun the PR smoke' >&2
                        exit 1
                    } ;;
                *) echo 'unexpected event for next validation' >&2; exit 1 ;;
            esac
            echo 'Next PR smoke passed, or post-merge smoke was deferred to nightly.'
        else
            [ "${CI_VALIDATION_RESULT-}" = success ] || {
                echo 'full validation did not succeed' >&2
                exit 1
            }
            [ "${CI_NEXT_SMOKE_RESULT-skipped}" = skipped ] || {
                echo 'unexpected next smoke outside next integration' >&2
                exit 1
            }
            echo 'Full validation passed.'
        fi ;;
    *) echo 'missing or invalid validation route' >&2; exit 1 ;;
esac
