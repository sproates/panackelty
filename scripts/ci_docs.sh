# Explicitly informational files. Unknown paths always need full validation.
ci_informational_doc() {
    case "$1" in
        ROADMAP.md|ARCHITECTURE.md|SELF_HOSTING.md|tests/README.md|tests/COVERAGE.md|tests/VALIDATION_PROFILE.md) return 0 ;;
        *) return 1 ;;
    esac
}
