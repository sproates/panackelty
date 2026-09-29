#!/bin/sh
# Execute the programs printed on the website, including their saved bytecode.
set -eu
command=${PANACK_SITE_COMMAND:-./panack}
html=${PANACK_SITE_HTML:-site/index.html}
case "${1:-all}" in
    all) examples='hello guards exact' ;;
    release) examples=hello ;;
    *) echo 'expected all or release' >&2; exit 2 ;;
esac
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-site-examples.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
extract() {
    awk -v marker="id=\"$1\"" '
        index($0, marker) { active=1; next }
        active && /<\/(code|samp)>/ { exit }
        active { print }
    ' "$html" | sed 's/&gt;/>/g;s/&lt;/</g;s/&amp;/\&/g'
}
for example in $examples; do
    extract "$example-source" > "$work/$example.panack"
    extract "$example-output" > "$work/expected"
    test -s "$work/$example.panack" && test -s "$work/expected"
    "$command" check "$work/$example.panack" > "$work/check"
    "$command" run "$work/$example.panack" > "$work/actual"
    cmp "$work/expected" "$work/actual"
    "$command" compile "$work/$example.panack" -o "$work/$example.bc" > "$work/compile"
    "$command" run "$work/$example.bc" > "$work/actual"
    cmp "$work/expected" "$work/actual"
done
echo 'PASS website examples through source and bytecode'
