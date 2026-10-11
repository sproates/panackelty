#!/usr/bin/env bash
set -eu

compiler=${PANACK_IDENTITY_AUDIT_COMPILER:-./panack}
checked=0
failed=0

check_root() {
  local source=$1
  checked=$((checked + 1))
  if ! env -u PANACKELTY_BOOTSTRAP_ROOT "$compiler" check "$source" >/dev/null; then
    echo "identity root audit failed: $source" >&2
    failed=$((failed + 1))
  fi
}

while IFS= read -r -d '' source; do
  check_root "$source"
done < <(
  find tests/functional/cases tests/fixtures/compiler_contracts \
    -type f -name main.panack -print0
)
for source in examples/*.panack; do
  [ -f "$source" ] || continue
  check_root "$source"
done

if [ "$checked" -eq 0 ]; then
  echo "identity root audit found no entry roots" >&2
  exit 1
fi
if [ "$failed" -ne 0 ]; then
  echo "identity root audit: $failed of $checked roots failed" >&2
  exit 1
fi

echo "identity root audit: $checked roots passed"
