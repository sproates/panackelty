#!/bin/sh
# P13 bridge: both spellings must preserve the accepted owned API behavior.
set -eu
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-owned-dot.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
for fixture in owned_method_dispatch owned_imports owned_generic_methods owned_intrinsics \
    owned_callbacks owned_async_handlers owned_core_methods owned_server_map \
    owned_server_map_alias owned_server_map_facade owned_server_map_identity owned_server_map_namespace; do
    destination="$work/$fixture"
    cp -R "tests/functional/cases/$fixture" "$destination"
    # These accepted fixtures contain executable arrows, not diagnostic strings.
    find "$destination" -name '*.panack' -type f -exec sh -c '
        for source do
            sed "s/->/./g" "$source" > "$source.tmp"
            mv "$source.tmp" "$source"
        done
    ' sh {} +
    if [ -f "$destination/panack.json" ]; then
        ./bin/panack run --project "$destination/panack.json" > "$work/out"
        ./bin/panack compile --project "$destination/panack.json" -o "$work/program.bc" > /dev/null
    else
        source="$destination/main.panack"
        if [ -f "$destination/source.path" ]; then
            original=$(cat "$destination/source.path")
            source="$work/${original#tests/functional/cases/}"
        fi
        ./bin/panack run "$source" > "$work/out"
        ./bin/panack compile "$source" -o "$work/program.bc" > /dev/null
    fi
    cmp "$destination/expected.stdout" "$work/out"
    ./bin/panack run "$work/program.bc" > "$work/out"
    cmp "$destination/expected.stdout" "$work/out"
    echo "owned dot bridge: $fixture source and bytecode passed"
done
cat > "$work/library.panack" <<'SOURCE'
pub pure get(): Nat { 900 }
pub record Item { value: Nat }
impl Item { pub pure get(): Nat { self.value } pure hidden(): Nat { self.value } }
SOURCE
cat > "$work/facade.panack" <<'SOURCE'
pub import "library.panack" as nested
SOURCE
cat > "$work/main.panack" <<'SOURCE'
import "library.panack" as api
import "facade.panack"::{nested}
import "library.panack"::{Item}
record Holder { call: PureFn[Nat,Nat] }
impl Holder { pure call(value: Nat): Nat { value + 1 } }
pure twice(value: Nat): Nat { value * 2 }
pure parameter(api: Item): Nat { api.get() }
main(): Void {
  print(api.get())
  api = Item(42)
  print(api.get())
  print(api->get())
  print(parameter(api))
  print(nested.get())
  nested = Item(8)
  print(nested.get())
  holder = Holder(@twice)
  print(holder.call(3))
  print((holder.call).call(3))
  for entry in [Item(7)] { print(entry.get()) }
  print("abc".slice(0, 2)->len())
}
SOURCE
printf '900\n42\n42\n42\n900\n8\n4\n6\n7\n2\n' > "$work/expected"
./bin/panack run "$work/main.panack" > "$work/out"
cmp "$work/expected" "$work/out"
./bin/panack compile "$work/main.panack" -o "$work/program.bc" > /dev/null
./bin/panack run "$work/program.bc" > "$work/out"
cmp "$work/expected" "$work/out"
reject() {
    expected=$1
    if ./bin/panack check "$work/main.panack" > "$work/out" 2> "$work/error"; then
        echo 'invalid owned dot call accepted' >&2
        exit 1
    fi
    grep -F "$expected" "$work/error" > /dev/null || { cat "$work/error" >&2; exit 1; }
}
cat > "$work/main.panack" <<'SOURCE'
record Holder { call: PureFn[Nat,Nat] }
pure twice(value: Nat): Nat { value * 2 }
main(): Void { print(Holder(@twice).call(3)) }
SOURCE
reject 'owned method'
cat > "$work/main.panack" <<'SOURCE'
import "library.panack" as api
pure missing(value: Nat): Nat { value }
main(): Void { print(api.missing(1)) }
SOURCE
reject 'has no binding missing'
cat > "$work/main.panack" <<'SOURCE'
import "library.panack" as api
pure get(value: Nat): Nat { value }
main(): Void { api = 1; print(api.get()) }
SOURCE
reject 'owned method'
cat > "$work/main.panack" <<'SOURCE'
record Item { value: Nat }
impl Item { get(): Nat { print("effect"); self.value } }
pure f(item: Item): Nat { item.get() }
main(): Void { print(f(Item(1))) }
SOURCE
reject 'pure'
echo 'owned dot bridge: shadowing, callable field, mixed chains and rejection passed'

cat > "$work/main.panack" <<'SOURCE'
import "library.panack" as api
pure f[api](item: api): Nat { 1 }
main(): Void {}
SOURCE
reject 'conflicts with an import binding'
cat > "$work/main.panack" <<'SOURCE'
import "library.panack"::{get}
main(): Void { get = 1; print(get) }
SOURCE
reject 'conflicts with an import binding'
# Guard's implicit value also shadows an alias; method checking remains typed.
cat > "$work/main.panack" <<'SOURCE'
import "library.panack" as value
type Ending = Str where value.ends_with("x")
main(): Void {}
SOURCE
./bin/panack check "$work/main.panack" > /dev/null

cat > "$work/main.panack" <<'SOURCE'
import "library.panack"::{Item}
main(): Void { print(Item(1).hidden()) }
SOURCE
reject 'private'
echo 'owned dot bridge: import collision, guard scope and method privacy passed'
