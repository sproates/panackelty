#!/bin/sh
# Reproduce SC4's real nested compiler acceptance fixture without transcript reuse.
set -eu
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-compiler-coverage.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
source=tests/fixtures/compiler_contracts/coverage_session/main.panack
./panack compile "$source" -o "$work/main.bc"
./panack inventory "$source" -o "$work/main.inv"
./panack inventory src/compiler/main.panack -o "$work/compiler.inv"
./panack coverage-session "$work/session" "$work/main.bc" "$work/main.inv" \
  bootstrap/compiler-v9.bc "$work/compiler.inv" -- bootstrap/compiler-v9.bc \
  tests/functional/cases/hello_world/main.panack > "$work/stdout"
printf 'ok\n' > "$work/expected"
cmp "$work/stdout" "$work/expected"
./panack coverage-aggregate --source "$source" "$work/main.inv" \
  --source src/compiler/main.panack "$work/compiler.inv" \
  --session "$work/session" > "$work/report"
head -1 "$work/report" > "$work/header"
printf 'coverage: aggregate sessions=1 executions=2\n' > "$work/expected"
cmp "$work/header" "$work/expected"
awk -F '\t' '
  $3 == "function" && $4 == "compiler_main" && $5 == "observed" && $6 == 1 && $7 == 1 { main++ }
  $3 == "function" && $4 == "run_compiler_command" && $5 == "observed" && $6 == 1 && $7 == 1 { driver++ }
  $3 == "function" && $4 == "run_coverage_aggregate" && $5 == "observed" && $6 == 0 && $7 == 0 { unused++ }
  END { if (main != 1 || driver != 1 || unused != 1) exit 1 }
' "$work/report"
echo 'coverage compiler session: exact nested entries and unused command passed'
