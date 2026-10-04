# Static website inputs can use complete website validation without native builds.
# Publisher code, unknown assets and shared tooling remain full-validation inputs.
validation_website_only_path() {
    case "$1" in
        site/index.html|site/styles.css|site/favicon.svg|site/playground.json) return 0 ;;
        *) return 1 ;;
    esac
}

# Reviewed ownership map shared by local and CI selection.
validation_component() {
    if ci_informational_doc "$1"; then
        case "$1" in
            CONTRIBUTING.md|docs/ROADMAP_PROCESS.md|.github/pull_request_template.md) printf 'process\n' ;;
            *) printf 'documentation\n' ;;
        esac
        return
    fi
    case "$1" in
        src/compiler/*|tests/fixtures/compiler_*/*|tests/runner/compiler_*) printf 'compiler\n' ;;
        src/bytecode/*|BYTECODE*|tests/fixtures/vm_contracts/*|tests/runner/bytecode_*) printf 'bytecode\n' ;;
        src/vm/*tcp*|tests/tcp*|tests/functional/cases/*tcp*/*) printf 'tcp\n' ;;
        src/vm/*|src/runtime/*|tests/runner/host_runtime*|tests/fixtures/host_runtime/*) printf 'runtime\n' ;;
        src/stdlib/*) printf 'stdlib\n' ;;
        site/*|scripts/*pages*|tests/pages*|scripts/fetch_playground.cjs|tests/playground_release.test.cjs) printf 'website\n' ;;
        README.md|LICENSE|CHANGELOG.md|RELEASE_POLICY.md|SECURITY.md|VERSION|tests/release*|tests/quick_start.sh) printf 'package\n' ;;
        examples/*|tests/functional/*) printf 'examples\n' ;;
        SPEC.md|bootstrap/*|Makefile|panack|scripts/*|tests/*|.github/workflows/*) printf 'shared\n' ;;
        *) printf 'unknown\n' ;;
    esac
}

# Website consumes the pinned browser release, never the native compiler seed.
# Only explicitly reviewed native-only paths may avoid browser validation.
validation_pages_path() {
    if ci_informational_doc "$1"; then printf 'false\n'; return; fi
    case "$1" in
        src/compiler/*|src/bytecode/*|src/vm/*|src/runtime/*|src/stdlib/*|bootstrap/*|examples/*|tests/fixtures/*|tests/runner/*|tests/unit/*|tests/functional/*|tests/tcp*|SPEC.md|BYTECODE*) printf 'false\n' ;;
        *) printf 'true\n' ;;
    esac
}
