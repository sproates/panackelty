#!/bin/sh
# Focused application-example checks, including stdin and local HTTP responses.
set -eu
work=$(mktemp -d tests/async-stdlib-examples.XXXXXX)
peer=
cleanup() {
  if [ -n "$peer" ]; then kill "$peer" 2>/dev/null || :; wait "$peer" 2>/dev/null || :; fi
  rm -rf "$work"
}
trap cleanup 0
trap 'exit 1' HUP INT TERM
fixture=${PANACK_HTTP_FIXTURE:-build/vm/http_fixture}
console=examples/async_console_input.panack
project=examples/network/http_diagnostic_client/panack.json
./bin/panack compile "$console" -o "$work/input.bc" >/dev/null
./bin/panack compile --project "$project" -o "$work/client.bc" >/dev/null
printf 'Aλ🙂' > "$work/text"
printf '\377' > "$work/invalid"
node -e 'process.stdout.write("a".repeat(511)+"λ")' > "$work/split"
node -e 'process.stdout.write("x".repeat(4096))' > "$work/boundary"
node -e 'process.stdout.write("x".repeat(4097))' > "$work/oversized"
"$fixture" "$work/port" > "$work/peer.log" 2>&1 &
peer=$!
attempt=0
while [ ! -s "$work/port" ] && [ "$attempt" -lt 100 ]; do
  sleep 0.05
  attempt=$((attempt+1))
done
test -s "$work/port"
port=$(cat "$work/port")
for mode in source bytecode; do
  input_source=$console
  if [ "$mode" = bytecode ]; then input_source=$work/input.bc; fi
  for sample in text split boundary; do
    ./bin/panack run "$input_source" < "$work/$sample" > "$work/out" 2> "$work/err"
    if [ "$sample" = text ]; then count=3; elif [ "$sample" = split ]; then count=512; else count=4096; fi
    printf 'Received %s characters.\n' "$count" > "$work/expected"
    cmp "$work/expected" "$work/out"
    test ! -s "$work/err"
  done
  for sample in invalid oversized; do
    status=0
    ./bin/panack run "$input_source" < "$work/$sample" > "$work/out" 2> "$work/err" || status=$?
    test "$status" = 1
    test ! -s "$work/out"
    if [ "$sample" = invalid ]; then printf 'Input is not valid UTF-8.\n' > "$work/expected"
    else printf 'Input exceeds the 4096-byte message limit.\n' > "$work/expected"; fi
    cmp "$work/expected" "$work/err"
  done
  for scenario in ok missing invalid; do
    case "$scenario" in
      ok) url="http://127.0.0.1:$port/ok"; expected_status=0; printf 'HTTP 200; 2 body bytes.\n' > "$work/expected" ;;
      missing) url="http://127.0.0.1:$port/missing"; expected_status=1; printf 'HTTP 404; 7 body bytes.\n' > "$work/expected" ;;
      invalid) url='http://user@localhost/'; expected_status=1; printf 'Cannot make the HTTP request: HTTP URL authority is empty or contains userinfo.\n' > "$work/expected" ;;
    esac
    status=0
    if [ "$mode" = source ]; then
      ./bin/panack run --project "$project" "$url" > "$work/out" 2> "$work/err" || status=$?
    else ./bin/panack run "$work/client.bc" "$url" > "$work/out" 2> "$work/err" || status=$?; fi
    test "$status" = "$expected_status"
    if [ "$scenario" = invalid ]; then cmp "$work/expected" "$work/err"; test ! -s "$work/out"
    else cmp "$work/expected" "$work/out"; test ! -s "$work/err"; fi
  done
done
printf '%s\n' 'Async console and diagnostic HTTP examples: source/bytecode success and failures passed.'
