#!/usr/bin/env bash
set -eu

root=$(cd "$(dirname "$0")/.." && pwd)
out="$root/build/browser-runtime"
rm -rf "$out"
mkdir -p "$out/vm" "$out/stdlib"

# The browser consumer needs the portable VM implementation but supplies its own
# host-capability adapter. Native networking and CLI entry points are not part of
# this dependency bundle.
vm_files='bigint.c bigint.h buffer.c buffer.h builtins.c builtins.h builtins_collections.c builtins_internal.h builtins_numeric.c builtins_text.c builtins_vm.c decode.c decode.h execute.c host.c host.h host_capabilities.h host_types.c host_types.h numeric.c numeric.h platform.h program.c program.h render.c render.h tasks.c tasks.h utf8.c utf8.h value.c value.h verify.c verify.h vm.h'
for file in $vm_files; do cp "$root/src/vm/$file" "$out/vm/$file"; done
cp "$root/bootstrap/compiler-v9.bc" "$out/compiler-v9.bc"
cp "$root"/src/stdlib/*.panack "$out/stdlib/"
cp "$root/src/bytecode/FORMAT.md" "$out/BYTECODE_FORMAT.md"
cp "$root/LICENSE" "$out/LICENSE"

version=$(sed -n '1p' "$root/VERSION")
seed_sha=$(awk 'NR==1 {print $1}' "$root/bootstrap/compiler-v9.bc.sha256")
actual_seed=$(sha256sum "$out/compiler-v9.bc" 2>/dev/null | awk '{print $1}' || shasum -a 256 "$out/compiler-v9.bc" | awk '{print $1}')
[ "$seed_sha" = "$actual_seed" ]

hash_file() {
  if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | awk '{print $1}'; else shasum -a 256 "$1" | awk '{print $1}'; fi
}
{
  printf 'format=1\n'
  printf 'panackelty_version=%s\n' "$version"
  printf 'bytecode_version=9\n'
  printf 'compiler_seed_sha256=%s\n' "$seed_sha"
  find "$out/vm" "$out/stdlib" -type f | LC_ALL=C sort | while read -r file; do
    printf 'sha256 %s %s\n' "$(hash_file "$file")" "${file#"$out/"}"
  done
  printf 'sha256 %s %s\n' "$(hash_file "$out/compiler-v9.bc")" compiler-v9.bc
  printf 'sha256 %s %s\n' "$(hash_file "$out/BYTECODE_FORMAT.md")" BYTECODE_FORMAT.md
} > "$out/MANIFEST"

# Consumer contract: no native host-capability implementation, networking or CLI.
for forbidden in host_capabilities.c tcp.c tcp.h tcp_server.c tcp_server.h main.c; do
  [ ! -e "$out/vm/$forbidden" ]
done
grep -qx 'format=1' "$out/MANIFEST"
grep -qx "panackelty_version=$version" "$out/MANIFEST"
grep -qx 'bytecode_version=9' "$out/MANIFEST"
grep -qx "compiler_seed_sha256=$seed_sha" "$out/MANIFEST"
echo "browser-runtime bundle: ok"
