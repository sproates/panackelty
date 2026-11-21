#!/bin/sh
set -eu
work=$(mktemp -d tests/cpu-resources.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_BOOTSTRAP_ROOT="$PWD" PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else "${PANACK_TEST_COMMAND:-./bin/panack}" "$@"; fi
}
printf hello | gzip -n > "$work/hello.gz"
cat "$work/hello.gz" "$work/hello.gz" > "$work/members.gz"
cat > "$work/template.panack" <<'SOURCE'
import stdlib/digest/sha256SUFFIX as digest
import stdlib/compression/gzipSUFFIX as gzip
import stdlib/filesystemSUFFIX as files
import stdlib/path as path
import stdlib/bytes as byte_buffer
import stdlib/hostSUFFIX as host
pure p(value: Str): Path { path.from_text(value).value_or(path.current()) }
ASYNC fail(status: Nat): Unit { AWAIT host.exit(status); () }
ASYNC expect(ok: Bool): Unit { if !ok { AWAIT fail(1) }; () }
pure known_hash(): Bytes { digest.hash("abc".encode_utf8()) }
ASYNC main(): Unit {
  AWAIT expect(known_hash().length() == 32)
  match AWAIT digest.create() {
    Error(_) => AWAIT fail(2),
    Ok(context) => {
      AWAIT expect(AWAIT digest.update(context, "a".encode_utf8()) == Ok(()))
      AWAIT expect(AWAIT digest.update(context, "bc".encode_utf8()) == Ok(()))
      match AWAIT digest.finish(context) {
        Error(_) => AWAIT fail(3), Ok(hash) => AWAIT expect(hash == known_hash())
      }
      match AWAIT digest.close(context) { Error(problem) => AWAIT expect(problem.code == "invalid_handle"), Ok(_) => AWAIT fail(4) }
    }
  }
  match AWAIT digest.create() {
    Error(_) => AWAIT fail(5),
    Ok(context) => {
      AWAIT expect(AWAIT digest.close(context) == Ok(()))
      match AWAIT digest.update(context, "".encode_utf8()) { Error(_) => (), Ok(_) => AWAIT fail(6) }
    }
  }
  match AWAIT files.read(p("ROOT/members.gz"), 65536) {
    Error(_) => AWAIT fail(7),
    Ok(data) => {
      match AWAIT gzip.create_decoder(10) {
        Error(_) => AWAIT fail(8),
        Ok(decoder) => {
          mut offset: Nat = 0
          mut total: Nat = 0
          mut finished = false
          while !finished {
            match AWAIT gzip.decode_chunk(decoder, byte_buffer.slice(data, offset, data.length()), true) {
              Error(_) => AWAIT fail(9),
              Ok(chunk) => {
                AWAIT expect(chunk.data == "hello".encode_utf8())
                offset = offset + chunk.consumed
                total = total + chunk.data.length()
                finished = chunk.finished
                ()
              }
            }
          }
          AWAIT expect(offset == data.length() && total == 10)
          match AWAIT gzip.decode_chunk(decoder, byte_buffer.empty(), true) { Error(problem) => AWAIT expect(problem.code == "decoder_finished"), Ok(_) => AWAIT fail(10) }
          AWAIT expect(AWAIT gzip.close(decoder) == Ok(()))
          match AWAIT gzip.close(decoder) { Error(_) => (), Ok(_) => AWAIT fail(11) }
        }
      }
      match AWAIT gzip.create_decoder(4) {
        Error(_) => AWAIT fail(12),
        Ok(decoder) => {
          match AWAIT gzip.decode_chunk(decoder, data, true) { Error(problem) => AWAIT expect(problem.code == "output_limit"), Ok(_) => AWAIT fail(13) }
          AWAIT expect(AWAIT gzip.close(decoder) == Ok(()))
        }
      }
    }
  }
}
SOURCE
for effect in ordinary async; do
  if [ "$effect" = async ]; then suffix=/async; effect_keyword=async; await_keyword=await
  else suffix=; effect_keyword=; await_keyword=; fi
  sed -e "s|SUFFIX|$suffix|g" -e "s/ASYNC/$effect_keyword/g" -e "s/AWAIT/$await_keyword/g" -e "s|ROOT|$PWD/$work|g" "$work/template.panack" > "$work/$effect.panack"
  panack run "$work/$effect.panack"
  panack compile "$work/$effect.panack" -o "$work/$effect.bc" > /dev/null
  panack run "$work/$effect.bc"
  panack disasm "$work/$effect.bc" > "$work/$effect.disasm"
done
cat > "$work/forged.panack" <<'SOURCE'
import stdlib/digest/sha256 as digest
import stdlib/compression/gzip as gzip
main(): Unit { digest.finish(1); gzip.close(1); () }
SOURCE
if panack check "$work/forged.panack" > "$work/error" 2>&1; then echo 'numeric resource handles accepted' >&2; exit 1; fi
grep -q 'Sha256Context' "$work/error"
grep -q 'GzipDecoder' "$work/error"
cat > "$work/pure.panack" <<'SOURCE'
import stdlib/digest/sha256 as digest
pure bad(context: Sha256Context): Result[Bytes,digest.HostError] { digest.finish(context) }
main(): Unit { () }
SOURCE
if panack check "$work/pure.panack" > "$work/error" 2>&1; then echo 'pure mutation accepted' >&2; exit 1; fi
grep -q 'pure' "$work/error"
printf '%s\n' 'Digest/gzip ordinary and async source, bytecode, disassembly, resource ownership and effect checks passed.'
