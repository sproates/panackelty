#!/bin/sh
set -eu
work=$(mktemp -d tests/async-stdlib.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
    if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
        PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
    else
        ./bin/panack "$@"
    fi
}
cat > "$work/main.panack" <<'SOURCE'
import stdlib/time/async as time
import stdlib/time as ordinary_time
import stdlib/environment/async as environment
import stdlib/host/async as host
import stdlib/async_host as legacy_host

pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}

async main(): Unit {
  expect(time.seconds(1) == ordinary_time.milliseconds(1000))
  expect(await environment.get("PANACK_ASYNC_VALUE") == Some("ready"))
  expect(await environment.get("PANACK_ASYNC_EMPTY") == Some(""))
  expect(await environment.get("PANACK_ASYNC_MISSING") == None())
  expect(await host.arguments() == await legacy_host.arguments())
  match await time.now() {
    Error(problem) => expect(false),
    Ok(start) => match await time.now() {
      Error(problem) => expect(false),
      Ok(end) => expect(!end.is_before(start))
    }
  }
  match await host.stdout("async stdlib: ok") {
    Ok(done) => (),
    Error(problem) => expect(false)
  }
}
SOURCE
export PANACK_ASYNC_VALUE=ready PANACK_ASYNC_EMPTY=
unset PANACK_ASYNC_MISSING
printf 'async stdlib: ok\n' > "$work/expected"
panack check "$work/main.panack" > /dev/null
panack run "$work/main.panack" > "$work/output"
cmp "$work/expected" "$work/output"
panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
./bin/panack run "$work/main.bc" > "$work/output"
cmp "$work/expected" "$work/output"
reject() {
    source=$1
    expected=$2
    printf '%s\n' "$source" > "$work/invalid.panack"
    if panack check "$work/invalid.panack" > "$work/stdout" 2> "$work/stderr"; then
        echo "unexpectedly accepted: $source" >&2
        exit 1
    fi
    grep -F "$expected" "$work/stderr" > /dev/null || { cat "$work/stderr" >&2; exit 1; }
}
reject 'import stdlib/time/async as time
async main(): Unit { reading = time.now(); () }' 'async call requires await'
reject 'import stdlib/time/async as time
pure main(): Unit { reading = await time.now(); () }' 'pure function cannot call impure'
reject 'import stdlib/time/async as time
main(): Unit { reading = await time.now(); () }' 'await'
reject 'import stdlib/time as time
async main(): Unit { reading = time.now(); () }' 'import stdlib/time/async and await this call'
reject 'import stdlib/environment as environment
async main(): Unit { value = environment.get("HOME"); () }' 'import stdlib/environment/async and await this call'
reject 'import stdlib/environment/async as environment
async main(): Unit { value = await environment.get(42); () }' 'Str'
reject 'import stdlib/environment/async as environment
pure main(): Unit { value = await environment.get("HOME"); () }' 'pure function cannot call impure'
reject 'async main(): Unit { value = await instant_now_async(); () }' 'removed global primitive'
reject 'async main(): Unit { value = await environment_lookup_async("HOME"); () }' 'removed global primitive'
reject 'import stdlib/time/async as time
async main(): Unit { value = await time.now(1); () }' 'arguments'
reject 'import stdlib/time/async as time
async main(): Unit { value = await time.seconds(1); () }' 'await requires an async call'
printf '%s\n' 'Async stdlib paths, source/bytecode execution, and effect diagnostics: ok'
