#!/bin/sh
set -eu
work=$(mktemp -d tests/console-input.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_BOOTSTRAP_ROOT="$PWD" PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else "${PANACK_TEST_COMMAND:-./bin/panack}" "$@"; fi
}
printf 'ab\000\377cd' > "$work/input"
cat > "$work/template.panack" <<'SOURCE'
import stdlib/consoleSUFFIX as console
import stdlib/bytes as byte_buffer
pure expect(ok: Bool): Unit { n = [0][if ok { 0 } else { 1 }]; () }
ASYNC main(): Unit {
  for limit in [0,65537] {
    match AWAIT console.read(limit) { Error(problem) => expect(problem.code == "out_of_range"), Ok(_) => expect(false) }
  }
  mut data = byte_buffer.empty()
  mut done = false
  while !done {
    match AWAIT console.read(2) {
      Error(_) => expect(false),
      Ok(option) => match option {
        None() => { done = true; () },
        Some(chunk) => { expect(chunk.len() > 0 && chunk.len() <= 2); data = data.join(chunk); () }
      }
    }
  }
  expect(data == "ab".encode_utf8().push(0).push(255).join("cd".encode_utf8()))
  expect(AWAIT console.read(1) == Ok(None()))
  output = AWAIT console.write_line("bounded input passed")
  expect(output == Ok(()))
}
SOURCE
for mode in ordinary async; do
  if [ "$mode" = async ]; then suffix=/async; effect=async; wait=await; else suffix=; effect=; wait=; fi
  sed -e "s@SUFFIX@$suffix@g" -e "s/ASYNC/$effect/g" -e "s/AWAIT/$wait/g" "$work/template.panack" > "$work/main.panack"
  panack run "$work/main.panack" < "$work/input" > "$work/source.out"
  printf 'bounded input passed\n' > "$work/expected"
  cmp "$work/source.out" "$work/expected"
  panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
  panack run "$work/main.bc" < "$work/input" > "$work/bytecode.out"
  cmp "$work/bytecode.out" "$work/expected"
  panack disasm "$work/main.bc" > /dev/null
  cat > "$work/errors.panack" <<SOURCE
import stdlib/console$suffix as console
pure expect(ok: Bool): Unit { n = [0][if ok { 0 } else { 1 }]; () }
$effect main(): Unit {
  match $wait console.read(65536) { Error(problem) => expect(problem.code == "io_error"), Ok(_) => expect(false) }
}
SOURCE
  panack compile "$work/errors.panack" -o "$work/errors.bc" > /dev/null
  panack run "$work/errors.bc" <&-
  cat > "$work/eof.panack" <<SOURCE
import stdlib/console$suffix as console
pure expect(ok: Bool): Unit { n = [0][if ok { 0 } else { 1 }]; () }
$effect main(): Unit { expect($wait console.read(65536) == Ok(None())) }
SOURCE
  panack run "$work/eof.panack" < /dev/null
  panack compile "$work/eof.panack" -o "$work/eof.bc" > /dev/null
  panack run "$work/eof.bc" < /dev/null
  cat > "$work/boundary.panack" <<SOURCE
import stdlib/console$suffix as console
pure expect(ok: Bool): Unit { n = [0][if ok { 0 } else { 1 }]; () }
$effect main(): Unit {
  match $wait console.read(65536) {
    Ok(option) => match option { Some(bytes) => expect(bytes.len() == 65536), None() => expect(false) },
    Error(_) => expect(false)
  }
  match $wait console.read(65536) {
    Ok(option) => match option { Some(bytes) => expect(bytes.len() == 1), None() => expect(false) },
    Error(_) => expect(false)
  }
  expect($wait console.read(1) == Ok(None()))
}
SOURCE
  # A regular file establishes the maximum-sized read without assuming that a
  # pipe or terminal fills the caller's requested buffer.
  dd if=/dev/zero of="$work/boundary-input" bs=65537 count=1 2>/dev/null
  panack run "$work/boundary.panack" < "$work/boundary-input"
  panack compile "$work/boundary.panack" -o "$work/boundary.bc" > /dev/null
  panack run "$work/boundary.bc" < "$work/boundary-input"
 done
cat > "$work/legacy.panack" <<'SOURCE'
import stdlib/console as console
pure expect(ok: Bool): Unit { n = [0][if ok { 0 } else { 1 }]; () }
main(): Unit {
  expect(read_line() == "one")
  expect(console.read(3) == Ok(Some("two".encode_utf8())))
  expect(read_line() == "")
  expect(read_line() == "three")
  expect(console.read(1) == Ok(None()))
}
SOURCE
printf 'one\r\ntwo\nthree' > "$work/lines"
panack run "$work/legacy.panack" < "$work/lines"
panack compile "$work/legacy.panack" -o "$work/legacy.bc" > /dev/null
panack run "$work/legacy.bc" < "$work/lines"
cat > "$work/pure.panack" <<'SOURCE'
import stdlib/console as console
import stdlib/host::{HostError}
pure main(): Result[Option[Bytes],HostError] { console.read(1) }
SOURCE
if panack check "$work/pure.panack" > "$work/rejection" 2>&1; then echo 'pure stdin read accepted' >&2; exit 1; fi
grep -q 'pure function cannot call impure function' "$work/rejection"
printf '%s\n' 'Bounded console input: ordinary/async bytes, EOF, errors, limits, legacy buffering and source/bytecode passed.'
