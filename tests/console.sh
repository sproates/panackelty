#!/bin/sh
set -eu
work=$(mktemp -d tests/console.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
    if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
        PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
    else
        ./bin/panack "$@"
    fi
}
for mode in ordinary async; do
    if [ "$mode" = async ]; then module=stdlib/console/async; effect=async; wait=await
    else module=stdlib/console; effect=; wait=; fi
    cat > "$work/$mode.panack" <<SOURCE
import $module as console
pure require(result: Result[Unit,console.HostError]): Unit {
  match result { Ok(_) => (), Error(_) => { checked = [0][1]; () } }
}
$effect main(): Unit {
  require($wait console.write("héllo"))
  require($wait console.write(""))
  require($wait console.write_line(" world"))
  require($wait console.write_line(""))
  require($wait console.write_error("error"))
  require($wait console.write_error(""))
  require($wait console.write_error_line(" details"))
  require($wait console.write_error_line(""))
}
SOURCE
    printf 'héllo world\n\n' > "$work/stdout.expected"
    printf 'error details\n\n' > "$work/stderr.expected"
    panack run "$work/$mode.panack" > "$work/stdout" 2> "$work/stderr"
    cmp "$work/stdout.expected" "$work/stdout"
    cmp "$work/stderr.expected" "$work/stderr"
    panack compile "$work/$mode.panack" -o "$work/$mode.bc" > /dev/null
    ./bin/panack run "$work/$mode.bc" > "$work/stdout" 2> "$work/stderr"
    cmp "$work/stdout.expected" "$work/stdout"
    cmp "$work/stderr.expected" "$work/stderr"
done
cat > "$work/legacy.panack" <<'SOURCE'
import stdlib/host as host
import stdlib/console as console
main(): Unit {
  host.stdout("before")
  result = console.write_line("middle")
  host.stdout("after")
  ()
}
SOURCE
printf 'before\nmiddle\nafter\n' > "$work/legacy.expected"
panack run "$work/legacy.panack" > "$work/stdout"
cmp "$work/legacy.expected" "$work/stdout"
panack compile "$work/legacy.panack" -o "$work/legacy.bc" > /dev/null
./bin/panack run "$work/legacy.bc" > "$work/stdout"
cmp "$work/legacy.expected" "$work/stdout"
reject() {
    printf '%s\n' "$1" > "$work/invalid.panack"
    if panack check "$work/invalid.panack" > "$work/stdout" 2> "$work/stderr"; then
        echo 'unexpectedly accepted invalid console program' >&2; exit 1
    fi
    grep -F "$2" "$work/stderr" > /dev/null || { cat "$work/stderr" >&2; exit 1; }
}
reject 'import stdlib/console as console
pure main(): Unit { result = console.write("x"); () }' 'pure function cannot call impure'
reject 'import stdlib/console as console
async main(): Unit { result = console.write("x"); () }' 'async'
reject 'import stdlib/console/async as console
async main(): Unit { result = console.write("x"); () }' 'async call requires await'
reject 'import stdlib/console/async as console
async main(): Unit { result = await console.write(1); () }' 'Str'
reject 'main(): Unit { result = console_output("x", false, false); () }' 'removed global primitive'
reject 'async main(): Unit { result = await console_output_async("x", false, false); () }' 'removed global primitive'
printf '%s\n' 'Ordinary/async console source, bytecode, exact output, compatibility and effect checks passed.'
