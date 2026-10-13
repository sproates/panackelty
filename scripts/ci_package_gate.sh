#!/bin/sh
# Keep the required Package check green while next distributions move to nightly.
set -eu

fail() {
    echo "$1" >&2
    exit 1
}

[ "${CI_SCOPE_RESULT-}" = success ] || fail 'validation routing did not succeed'

case "${CI_SCOPE_ROUTE-}" in
    docs)
        [ "${CI_PACKAGE_BUILD_RESULT-}" = skipped ] || \
            fail 'package validation unexpectedly ran for a documentation route'
        [ "${CI_PLATFORM_SMOKE_RESULT-}" = skipped ] || \
            fail 'platform validation unexpectedly ran for a documentation route'
        [ "${CI_WORKFLOW_RESULT-}" = skipped ] || \
            fail 'workflow validation unexpectedly ran for a documentation route'
        echo 'Documentation checks passed; package and platform validation are not applicable.' ;;
    workflow)
        [ "${CI_PACKAGE_BUILD_RESULT-}" = skipped ] || \
            fail 'package validation unexpectedly ran for a workflow-only route'
        [ "${CI_PLATFORM_SMOKE_RESULT-}" = skipped ] || \
            fail 'platform validation unexpectedly ran for a workflow-only route'
        [ "${CI_WORKFLOW_RESULT-}" = success ] || \
            fail 'workflow configuration validation did not succeed'
        echo 'Workflow syntax and repository contracts passed; package builds are not applicable.' ;;
    full)
        [ "${CI_WORKFLOW_RESULT-}" = skipped ] || \
            fail 'unexpected workflow validation outside the workflow-only route'
        if [ "${CI_NEXT_INTEGRATION-}" = true ]; then
            [ "${CI_PACKAGE_BUILD_RESULT-}" = skipped ] || \
                fail 'next package matrix must be deferred to nightly'
            case "${CI_EVENT_NAME-}" in
                pull_request)
                    [ "${CI_PLATFORM_SMOKE_RESULT-}" = success ] || \
                        fail 'next pull requests require the macOS compiler validation' ;;
                push)
                    [ "${CI_PLATFORM_SMOKE_RESULT-}" = skipped ] || \
                        fail 'post-merge next pushes must not run the pull-request platform smoke' ;;
                *) fail 'unexpected event for next validation' ;;
            esac
            echo 'Focused Linux checks passed; next full Linux/macOS validation and distribution builds are nightly.'
        else
            [ "${CI_PACKAGE_BUILD_RESULT-}" = success ] || \
                fail 'full two-platform package validation did not succeed'
            [ "${CI_PLATFORM_SMOKE_RESULT-}" = skipped ] || \
                fail 'unexpected macOS smoke outside a next pull request'
            echo 'Full two-platform package validation passed.'
        fi ;;
    *) fail 'missing or invalid validation route' ;;
esac
