# Reviewed ownership map shared by local and CI selection. All non-document
# components retain the full native integration envelope. The explicit downstream
# browser pin permits the Pages PR selection documented below.
validation_component() {
    if ci_informational_doc "$1"; then
        case "$1" in
            AGENTS.md|CONTRIBUTING.md|docs/ROADMAP_PROCESS.md|.agents/skills/next-item/SKILL.md|.github/pull_request_template.md) printf 'process\n' ;;
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

# The browser product consumes a pinned core version, not this PR's sources.
# Website/shared/unknown changes still exercise the assembled artifact. This
# selector changes PR provisioning only; production always retains browser gates.
validation_pages() {
    for component in $1; do
        case "$component" in
            documentation|process|compiler|bytecode|runtime|tcp|stdlib|examples) ;;
            *) printf 'true\n'; return ;;
        esac
    done
    printf 'false\n'
}
