#!/bin/sh
# Public source acceptance after removal of receiver-to-global compatibility.
set -eu
work=$(mktemp -d "${TMPDIR:-/tmp}/panack-owned-removal.XXXXXX")
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
reject() {
    expected=$1
    if ./bin/panack check "$work/main.panack" > "$work/out" 2> "$work/error"; then
        echo 'removed dispatch syntax unexpectedly accepted' >&2
        exit 1
    fi
    grep -F "$expected" "$work/error" > /dev/null || { cat "$work/error" >&2; exit 1; }
}
# These names are implementation operations, not source-global receiver APIs.
for name in len append concat slice starts_with starts_with_at reverse is_digit is_letter \
    is_whitespace nat_from_str utf8_encode utf8_decode quotient nat dec map_put map_get \
    map_has set_add set_has byte_append byte_get byte_len bytes_concat duration_ticks \
    instant_add instant_difference instant_before path_append path_to_text path_native_bytes \
    path_display path_absolute path_directory path_filename command_args process_exit \
    eprint host_decode_utf8 host_sleep fs_read fs_write path_from_text path_current \
    duration_nanoseconds instant_now tcp_exchange tcp_serve tcp_serve_stream \
    tcp_server_read_chunk http_transport_exchange async_fake_read; do
    printf 'main(): Void { %s() }\n' "$name" > "$work/main.panack"
    reject "removed global primitive: $name"
done
# Surviving modules no longer export receiver wrappers.
for entry in bytes:push bytes:join bytes:length bytes:at path:join path:to_text \
    path:native_bytes path:display path:is_absolute path:directory path:filename \
    time:ticks time:add_instant time:is_before_instant time:difference time:add \
    time:subtract time:scale time:is_before time:as_seconds time:divide time:ratio \
    testing_files:workspace_remove_empty; do
    module=${entry%%:*}
    member=${entry#*:}
    printf 'import stdlib/%s as api\nmain(): Void { api.%s() }\n' "$module" "$member" > "$work/main.panack"
    reject "has no binding $member"
done
for module in text numeric option result; do
    printf 'import stdlib/%s as api\nmain(): Void {}\n' "$module" > "$work/main.panack"
    reject 'missing'
done
cat > "$work/main.panack" <<'SOURCE'
pure transform(value: Nat): Nat { value }
main(): Void { print(1.transform()) }
SOURCE
reject 'owned method'
cat > "$work/main.panack" <<'SOURCE'
pure missing(value: Str): Nat { 7 }
main(): Void { print("x"->missing()) }
SOURCE
reject 'owned method'
cat > "$work/library.panack" <<'SOURCE'
pub pure get(value: Nat): Nat { value + 1 }
SOURCE
cat > "$work/main.panack" <<'SOURCE'
import "library.panack" as api
record Item { value: Nat }
main(): Void { api = Item(1); print(api.get(2)) }
SOURCE
reject 'owned method'
cat > "$work/core.panack" <<'SOURCE'
pub pure join(): Nat { path_append() }
SOURCE
cat > "$work/main.panack" <<'SOURCE'
import "core.panack" as forged
main(): Void { print(forged.join()) }
SOURCE
reject 'removed global primitive: path_append'
# Ordinary functions can reuse freed names; namespace service calls stay explicit.
cat > "$work/main.panack" <<'SOURCE'
import "library.panack" as api
pure len(value: Nat): Nat { value + 1 }
main(): Void {
  print(len(41))
  print(api.get(41))
  print("abc"->len())
  values: Map[Str,Nat] = map()
  print(values->put("key", 7)->get("key"))
}
SOURCE
printf '42\n42\n3\n7\n' > "$work/expected"
./bin/panack run "$work/main.panack" > "$work/out"
cmp "$work/expected" "$work/out"
./bin/panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
./bin/panack run "$work/main.bc" > "$work/out"
cmp "$work/expected" "$work/out"
printf 'owned dispatch removal: rejection, namespace, shadowing and source/bytecode acceptance passed\n'

# Ordinary terminal/text-file services remain source APIs with checked signatures.
cat > "$work/main.panack" <<SOURCE
import stdlib/compiler_files as files
main(): Void {
  text = read_line()
  write_file("$work/message.txt", text)
  print(files.read_text("$work/message.txt"))
}
SOURCE
printf 'hello\n' | ./bin/panack run "$work/main.panack" > "$work/out"
printf 'hello\n' > "$work/expected"
cmp "$work/expected" "$work/out"
printf 'main(): Void { print(read_line(1)) }\n' > "$work/main.panack"
reject 'read_line expects 0 arguments'
printf 'main(): Void { write_file(1, "text") }\n' > "$work/main.panack"
reject 'expected Str'
