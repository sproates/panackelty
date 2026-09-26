#!/bin/sh
# Each negative control starts from a clean, otherwise valid source tree.
set -eu
script=$(pwd -P)/tests/no_python.sh
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-policy.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
language=py
language=${language}thon
mkdir -p "$work/tree/.github/workflows" "$work/tree/docs"
printf '# Historical %s migration\n' "$language" > "$work/tree/docs/history.md"
printf '#!/bin/sh\nprintf "hello\\n"\n' > "$work/tree/valid"
printf '\377\376\000\200binary asset\n' > "$work/tree/image.bin"
sh "$script" "$work/tree" > "$work/log"
reject() {
    if sh "$script" "$work/tree" > "$work/log" 2>&1; then
        echo "policy accepted invalid control: $1" >&2; exit 1
    fi
    grep -F "$1" "$work/log" >/dev/null || { cat "$work/log" >&2; exit 1; }
}
for extension in py pyw pyc pyo PY PYW; do
    printf 'forbidden\n' > "$work/tree/file with spaces.$extension"
    reject 'forbidden source or cache'
    rm "$work/tree/file with spaces.$extension"
done
for interpreter in "$language" "${language}3" "${language}3.12"; do
    for prefix in '#!/usr/bin/' '#!/usr/bin/env ' '#!/usr/bin/env -S '; do
        printf '%s%s\n' "$prefix" "$interpreter" > "$work/tree/hidden"
        reject 'forbidden interpreter shebang'
        rm "$work/tree/hidden"
    done
    for command in "$interpreter tool" "/usr/bin/$interpreter tool" "env $interpreter tool" "sh -c '$interpreter tool'" "exec \"$interpreter\" tool"; do
        printf '#!/bin/sh\n%s\n' "$command" > "$work/tree/hidden"
        reject 'forbidden interpreter dependency'
        rm "$work/tree/hidden"
    done
done
printf 'all:\n\t%s tool\n' "${language}3" > "$work/tree/Makefile"
reject 'forbidden interpreter dependency'
rm "$work/tree/Makefile"
upper=$(printf '%s' "$language" | tr '[:lower:]' '[:upper:]')
for invocation in "\$($upper)" "\${$upper}" "\$$upper"; do
    printf '#!/bin/sh\n%s tool\n' "$invocation" > "$work/tree/hidden"
    reject 'forbidden interpreter dependency'
    rm "$work/tree/hidden"
done
printf '%s tool\n' "${language}3" > "$work/tree/executable"
chmod +x "$work/tree/executable"
reject 'forbidden interpreter dependency'
rm "$work/tree/executable"
mkdir -p "$work/tree/src/build"
printf 'forbidden\n' > "$work/tree/src/build/hidden.py"
reject 'forbidden source or cache'
rm "$work/tree/src/build/hidden.py"
printf 'run: %s tool\n' "${language}3" > "$work/tree/.github/workflows/check.yml"
reject 'forbidden interpreter dependency'
printf 'uses: actions/setup-%s@v6\n' "$language" > "$work/tree/.github/workflows/check.yml"
reject 'forbidden interpreter dependency'
rm "$work/tree/.github/workflows/check.yml"
ln -s valid "$work/tree/alias.py"
reject 'forbidden source or cache'
rm "$work/tree/alias.py"
sh "$script" "$work/tree" > "$work/log"
if sh "$script" "$work/missing" > "$work/log" 2>&1; then
    echo 'policy accepted a missing tree' >&2; exit 1
fi
grep -F 'missing source tree' "$work/log" >/dev/null
echo 'source policy: positive and adversarial controls passed'
