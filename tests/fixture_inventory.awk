# This recognises the bounded declarations used by the runner, not Panackelty
# generally. Unrecognised declarations fail closed; executable assertions stay
# independent and are still exercised by the full suite.
function fail(message) {
    print "fixture inventory: " message > "/dev/stderr"
    failed = 1
}
function extract(text, pattern, location, result) {
    if (!match(text, pattern)) {
        fail(location ": expected declaration not found; update preflight for intentional format changes")
        return ""
    }
    result = substr(text, RSTART, RLENGTH)
    text = substr(text, RSTART + RLENGTH)
    if (match(text, pattern)) fail(location ": ambiguous duplicate declaration")
    return result
}
function list(text, binding, values, declaration, n, name) {
    declaration = extract(text, " " binding " = \\[[^]]*\\]", "runner_smoke/main.panack: " binding)
    sub(/^[^[]*\[/, "", declaration)
    sub(/\]$/, "", declaration)
    n = 0
    while (match(declaration, /"[a-zA-Z0-9_]+"/)) {
        name = substr(declaration, RSTART + 1, RLENGTH - 2)
        if (name in values) fail("runner_smoke/main.panack: duplicate " binding " entry " name)
        n++
        values[name] = n
        declaration = substr(declaration, 1, RSTART - 1) substr(declaration, RSTART + RLENGTH)
    }
    gsub(/[[:space:],]/, "", declaration)
    if (declaration != "" || n == 0) fail("runner_smoke/main.panack: unsupported " binding " list")
    return n
}
function count(text, selector, actual, declaration, value) {
    declaration = extract(text, "if " selector " == \"\" \\{ [0-9]+ \\}", "tests/runner/main.panack: " selector)
    sub(/^.*\{ /, "", declaration)
    sub(/ \}$/, "", declaration)
    value = declaration + 0
    if (value != actual) fail("tests/runner/main.panack: " selector " count is " value ", inventory requires " actual)
}
FILENAME == ARGV[1] { runner = runner $0 "\n"; next }
FILENAME == ARGV[2] { smoke = smoke $0 "\n"; next }
$1 == "source" {
    if ($2 in sources) fail("duplicate example name " $2 " across examples and namespaces")
    source_count++; sources[$2] = source_count; next
}
$1 == "output" { outputs[$2] = 1; next }
END {
    # Whitespace is insignificant in these declarations, including line wraps.
    gsub(/[[:space:]]+/, " ", runner)
    gsub(/[[:space:]]+/, " ", smoke)
    example_count = list(smoke, "examples", examples)
    case_count = list(smoke, "names", cases)
    failure_count = list(smoke, "failure_names", failures)
    for (name in sources) {
        if (!(name in examples)) fail("runner_smoke/main.panack: examples list missing " name)
        if ((name in examples) && examples[name] != sources[name])
            fail("runner_smoke/main.panack: example order differs at " name)
        if (!(name in outputs)) fail("tests/functional/expected/examples/" name ".stdout: missing output")
    }
    for (name in examples) if (!(name in sources)) fail("runner_smoke/main.panack: stale example " name)
    for (name in outputs) if (!(name in sources)) fail("orphan expected output: " name ".stdout")
    # Isolate the example function because failures use the same selector name.
    example_function = extract(runner, "runner_examples\\(.*runner_normalize_diagnostic", "runner_examples boundary")
    count(example_function, "selected", source_count)
    count(runner, "selected_case", case_count)
    failure_function = extract(runner, "runner_failures\\(.*selected failure count", "runner_failures boundary")
    count(failure_function, "selected", failure_count)

    # Both implementations must agree on the subset with run/disasm assertions.
    condition_pattern = "if name == \"unknown_name\"[^{}]*\\{"
    condition = extract(smoke, condition_pattern, "runner_smoke failure modes")
    if (condition != extract(runner, condition_pattern, "runner failure modes"))
        fail("runner and runner_smoke failure-mode conditions differ")
    rest = condition
    extras = 0
    while (match(rest, /name == "[a-zA-Z0-9_]+"/)) {
        term = substr(rest, RSTART, RLENGTH)
        sub(/^name == "/, "", term); sub(/"$/, "", term)
        extra_names[term] = 1
        rest = substr(rest, 1, RSTART - 1) substr(rest, RSTART + RLENGTH)
    }
    # Support the current prefix predicate explicitly; reject new condition forms.
    has_prefix = (rest ~ /name\.starts_with_at\("fact_mutation_", 0\)/)
    gsub(/name\.starts_with_at\("fact_mutation_", 0\)/, "", rest)
    gsub(/if|[ |{]/, "", rest)
    if (rest != "") fail("unsupported failure-mode condition; update preflight")
    for (name in failures) if ((name in extra_names) || (has_prefix && index(name, "fact_mutation_") == 1)) extras++
    total = 3 * (case_count + example_count + failure_count) + 2 * extras + 3
    declared = extract(smoke, "tests: [0-9]+, failures: 0", "runner_smoke transcript total")
    sub(/^tests: /, "", declared); sub(/,.*$/, "", declared)
    if (declared + 0 != total) fail("runner_smoke/main.panack: transcript total is " declared ", inventory requires " total)
    if (failed) exit 1
    print "fixture inventory: " source_count " examples, " total " transcript assertions; consistent"
}
