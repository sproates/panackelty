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
        README.md|LICENSE|CHANGELOG.md|RELEASE_POLICY.md|SECURITY.md|VERSION|tests/release*|tests/quick_start.sh) printf 'package\n' ;;
        examples/*|tests/functional/*) printf 'examples\n' ;;
        SPEC.md|bootstrap/*|Makefile|panack|scripts/*|tests/*|.github/workflows/*) printf 'shared\n' ;;
        *) printf 'unknown\n' ;;
    esac
}
