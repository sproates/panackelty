#!/bin/sh
set -eu
work=$(mktemp -d tests/resource-errors.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else "${PANACK_TEST_COMMAND:-./bin/panack}" "$@"; fi
}
printf hello | gzip -n > "$work/hello.gz"
printf '' | gzip -n > "$work/empty.gz"
node - "$work" <<'JS'
const fs=require('fs'),root=process.argv[2],data=fs.readFileSync(root+'/hello.gz');
fs.writeFileSync(root+'/truncated.gz',data.subarray(0,data.length-4));
const corrupt=Buffer.from(data);corrupt[corrupt.length-8]^=1;fs.writeFileSync(root+'/corrupt.gz',corrupt);
fs.writeFileSync(root+'/oversized',Buffer.alloc(65537,120));
JS
cat > "$work/template.panack" <<'SOURCE'
import stdlib/digest/sha256SUFFIX as digest
import stdlib/compression/gzipSUFFIX as gzip
import stdlib/filesystemSUFFIX as files
import stdlib/consoleSUFFIX as console
import stdlib/hostSUFFIX as host
import stdlib/digest/sha256/errors as digest_errors
import stdlib/compression/gzip/errors as gzip_errors
import stdlib/digest/sha256/errors::{Sha256Operation.Update, Sha256Operation.Finish,
  Sha256ErrorKind.ChunkTooLarge as DigestChunkTooLarge, Sha256ErrorKind.InvalidContext,
  Sha256ErrorKind.OtherFailure as DigestOtherFailure}
import stdlib/compression/gzip/errors::{GzipOperation.CreateDecoder, GzipOperation.DecodeChunk,
  GzipErrorKind.ChunkTooLarge as GzipChunkTooLarge, GzipErrorKind.UnsupportedOutputLimit,
  GzipErrorKind.OutputLimitExceeded, GzipErrorKind.IncompleteInput, GzipErrorKind.DecodeFailed,
  GzipErrorKind.DecoderStopped, GzipErrorKind.InvalidDecoder,
  GzipErrorKind.ResourceLimit, GzipErrorKind.OtherFailure as GzipOtherFailure}
import stdlib/host::{HostError}
import stdlib/path as path
pure p(text: Str): Path { path.from_text(text).value_or(path.current()) }
pure expect(ok: Bool): Unit { check = [0][if ok { 0 } else { 1 }]; () }
pure require[T,E](result: Result[T,E]): T {
  match result { Ok(value) => value, Error(_) => { missing: [T] = []; missing[0] } }
}
pure failure[T,E](result: Result[T,E]): E {
  match result { Error(problem) => problem, Ok(_) => { missing: [E] = []; missing[0] } }
}
EFFECT main(): Unit {
  args = WAIT host.arguments()
  root = p(args[0])
  oversized = require(WAIT files.read(require(root.join(p("oversized"))),65537))
  compressed = require(WAIT files.read(require(root.join(p("hello.gz"))),100))
  empty = require(WAIT files.read(require(root.join(p("empty.gz"))),100))
  truncated = require(WAIT files.read(require(root.join(p("truncated.gz"))),100))
  corrupt = require(WAIT files.read(require(root.join(p("corrupt.gz"))),100))
  context = require(WAIT digest.create())
  chunk = failure(WAIT digest.update(context,oversized))
  expect(chunk.kind == DigestChunkTooLarge() && chunk.operation == Update(65537))
  expect(chunk.native.code == "invalid_argument")
  expect(chunk.render() == "Cannot update a SHA-256 context with 65537 bytes: an update accepts at most 65536 bytes. Split the input into smaller chunks and update the context with each chunk.")
  updated = require(WAIT digest.update(context,"abc".encode_utf8()))
  expect(require(WAIT digest.finish(context)) == digest.hash("abc".encode_utf8()))
  stale = failure(WAIT digest.finish(context))
  expect(stale.kind == InvalidContext() && stale.operation == Finish())
  expect(stale.native.code == "invalid_handle")
  expect(failure(WAIT gzip.create_decoder(268435457)).kind == UnsupportedOutputLimit())
  huge = failure(WAIT gzip.create_decoder(999999999999999999999999999999999999))
  expect(huge.kind == UnsupportedOutputLimit() && huge.native.code == "invalid_argument")
  unsupported = failure(WAIT gzip.create_decoder(268435457))
  expect(unsupported.operation == CreateDecoder(268435457))
  expect(unsupported.render() == "Cannot create a gzip decoder with an output limit of 268435457 bytes: the output limit must be at most 268435456 bytes (256 MiB). Choose a supported limit; zero permits no decompressed output.")
  decoder = require(WAIT gzip.create_decoder(5))
  too_large = failure(WAIT gzip.decode_chunk(decoder,oversized,false))
  expect(too_large.kind == GzipChunkTooLarge() && too_large.operation == DecodeChunk(65537,false))
  decoded = require(WAIT gzip.decode_chunk(decoder,compressed,true))
  expect(decoded.data == "hello".encode_utf8() && decoded.finished)
  expect(failure(WAIT gzip.decode_chunk(decoder,"".encode_utf8(),true)).kind == DecoderStopped())
  closed = require(WAIT gzip.close(decoder))
  expect(failure(WAIT gzip.close(decoder)).kind == InvalidDecoder())
  bounded = require(WAIT gzip.create_decoder(4))
  budget = failure(WAIT gzip.decode_chunk(bounded,compressed,true))
  expect(budget.kind == OutputLimitExceeded())
  expect(budget.native.code == "output_limit")
  expect(failure(WAIT gzip.decode_chunk(bounded,"".encode_utf8(),true)).kind == DecoderStopped())
  bounded_closed = require(WAIT gzip.close(bounded))
  partial = require(WAIT gzip.create_decoder(5))
  ended = failure(WAIT gzip.decode_chunk(partial,truncated,true))
  expect(ended.kind == IncompleteInput() && ended.native.code == "truncated_gzip")
  expect(failure(WAIT gzip.decode_chunk(partial,compressed,true)).kind == DecoderStopped())
  partial_closed = require(WAIT gzip.close(partial))
  damaged = require(WAIT gzip.create_decoder(5))
  bad = failure(WAIT gzip.decode_chunk(damaged,corrupt,true))
  expect(bad.kind == DecodeFailed() && bad.native.code == "invalid_gzip")
  damaged_closed = require(WAIT gzip.close(damaged))
  zero = require(WAIT gzip.create_decoder(0))
  empty_result = require(WAIT gzip.decode_chunk(zero,empty,true))
  expect(empty_result.finished && empty_result.data.length() == 0)
  zero_closed = require(WAIT gzip.close(zero))
  raw = HostError("future-operation","future-code",77)
  unknown_digest = failure(digest_errors.convert[Unit](Error(raw),Finish()))
  unknown_gzip = failure(gzip_errors.convert[Unit](Error(raw),CreateDecoder(0)))
  expect(unknown_digest.kind == DigestOtherFailure() && unknown_digest.native == raw)
  expect(unknown_gzip.kind == GzipOtherFailure() && unknown_gzip.native == raw)
  // Five decoders have been created and closed; closed slots still count.
  mut count: Nat = 5
  while count < 64 {
    extra = require(WAIT gzip.create_decoder(0))
    extra_closed = require(WAIT gzip.close(extra))
    count = count + 1
  }
  exhausted = failure(WAIT gzip.create_decoder(0))
  expect(exhausted.kind == ResourceLimit())
  expect(exhausted.render() == "Cannot create a gzip decoder with an output limit of 0 bytes: the host could not allocate another decoder slot. An invocation can create at most 64 decoders, including decoders already closed; closing a decoder does not reset that quota.")
  written = require(WAIT console.write_line("resource errors passed"))
  ()
}
SOURCE
printf 'resource errors passed\n' > "$work/expected"
for mode in ordinary async; do
  if [ "$mode" = async ]; then suffix=/async; effect=async; wait=await; else suffix=; effect=; wait=; fi
  sed -e "s@SUFFIX@$suffix@g" -e "s/EFFECT/$effect/g" -e "s/WAIT/$wait/g" "$work/template.panack" > "$work/main.panack"
  panack run "$work/main.panack" "$work" > "$work/output"
  cmp "$work/expected" "$work/output"
  panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
  panack run "$work/main.bc" "$work" > "$work/output"
  cmp "$work/expected" "$work/output"
  panack disasm "$work/main.bc" > /dev/null
done
printf '%s\n' 'Digest/gzip errors: ordinary/async categories, context, exact rendering, native preservation, retry/lifecycle, corruption, truncation and quotas passed.'
