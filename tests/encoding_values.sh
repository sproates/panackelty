#!/bin/sh
set -eu
work=$(mktemp -d tests/encoding-values.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_BOOTSTRAP_ROOT="$PWD" PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else "${PANACK_TEST_COMMAND:-./bin/panack}" "$@"; fi
}
cat > "$work/main.panack" <<'SOURCE'
import stdlib/encoding/utf8 as utf8
import stdlib/encoding/utf8::{Utf8Error, Utf8ErrorKind, Utf8ErrorKind.InvalidLeadingByte, Utf8ErrorKind.InvalidContinuationByte, Utf8ErrorKind.TruncatedSequence, Utf8ErrorKind.OverlongEncoding, Utf8ErrorKind.SurrogateCodePoint, Utf8ErrorKind.CodePointOutOfRange}
import stdlib/bytes as byte_buffer
import stdlib/path as path
import stdlib/host as host
pure data(values: [Nat]): Bytes {
  mut output = byte_buffer.empty()
  for value in values { output = output.push(value) }
  output
}
expect(ok: Bool): Unit { if !ok { host.exit(1) }; () }
pure bad(values: [Nat], offset: Nat, kind: Utf8ErrorKind): Bool {
  utf8.decode(data(values)) == Error(Utf8Error(offset, kind))
}
pure p(value: Str): Path { path.from_text(value).value_or(path.current()) }
main(): Unit {
  expect(utf8.decode(byte_buffer.empty()) == Ok(""))
  expect(utf8.decode("hello é 漢 😀".encode_utf8()) == Ok("hello é 漢 😀"))
  expect(utf8.decode(data([0, 127])) == Ok(data([0, 127]).decode_utf8()))
  expect(utf8.decode(data([194,128,223,191,224,160,128,237,159,191,238,128,128,239,191,191,240,144,128,128,244,143,191,191])) == Ok(data([194,128,223,191,224,160,128,237,159,191,238,128,128,239,191,191,240,144,128,128,244,143,191,191]).decode_utf8()))
  expect(bad([128], 0, InvalidLeadingByte()))
  expect(bad([255], 0, InvalidLeadingByte()))
  expect(bad([97,194,65], 2, InvalidContinuationByte()))
  expect(bad([97,226,130], 1, TruncatedSequence()))
  expect(bad([192,128], 0, OverlongEncoding()))
  expect(bad([224,128,128], 0, OverlongEncoding()))
  expect(bad([240,128,128,128], 0, OverlongEncoding()))
  expect(bad([237,160,128], 0, SurrogateCodePoint()))
  expect(bad([244,144,128,128], 0, CodePointOutOfRange()))
  expect(bad([247,191,191,191], 0, CodePointOutOfRange()))
  expect(bad([194,169,255], 2, InvalidLeadingByte()))
  expect(bad([128,255], 0, InvalidLeadingByte()))
  mut byte: Nat = 0
  while byte < 256 {
    result = utf8.decode(data([byte]))
    if byte < 128 { expect(result == Ok(data([byte]).decode_utf8())) }
    else { match result { Error(_) => (), Ok(_) => { host.exit(2); () } } }
    byte = byte + 1
  }
  value = "abc".encode_utf8()
  expect(value.len() == value.length())
  expect(value.slice(1,3) == byte_buffer.slice(value,1,3))
  expect(value.slice(3,3) == byte_buffer.empty())
  expect(value.slice(0,3) == value)
  for name in ["/", ".", "a", "a/b", "a/b/", "/a/b", "a//b"] { expect(p(name).parent() == p(name).directory()) }
  print(Utf8Error(2, InvalidContinuationByte()).render())
  print("encoding and value methods passed")
  ()
}
SOURCE
cat > "$work/expected" <<'OUTPUT'
Cannot decode UTF-8 at byte offset 2 (counting from zero): a UTF-8 continuation byte was required.
encoding and value methods passed
OUTPUT
panack run "$work/main.panack" > "$work/source.out"
cmp "$work/expected" "$work/source.out"
panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
panack run "$work/main.bc" > "$work/bytecode.out"
cmp "$work/expected" "$work/bytecode.out"
panack disasm "$work/main.bc" > /dev/null
for expr in '"abc".encode_utf8().slice(2,1)' '"abc".encode_utf8().slice(0,4)' 'byte_buffer.empty().push(255).decode_utf8()'; do
  printf 'import stdlib/bytes as byte_buffer\nmain(): Unit { print(%s); () }\n' "$expr" > "$work/trap.panack"
  panack compile "$work/trap.panack" -o "$work/trap.bc" > /dev/null
  if panack run "$work/trap.bc" > "$work/trap.out" 2>&1; then echo 'expected byte bounds/decoding trap' >&2; exit 1; fi
  grep -q 'VM trap:' "$work/trap.out"
done
printf '%s\n' 'Checked UTF-8, structured errors, byte/path methods and compatibility traps passed in source/bytecode modes.'
