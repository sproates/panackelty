#!/bin/sh
set -eu
repo=$PWD
work=$(mktemp -d tests/http-errors.XXXXXX)
tls_pid=
trap 'test -z "$tls_pid" || { kill "$tls_pid" 2>/dev/null || :; wait "$tls_pid" 2>/dev/null || :; }; rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
fixture=build/vm/http_fixture
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_BOOTSTRAP_ROOT="$PWD" PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else "${PANACK_TEST_COMMAND:-./bin/panack}" "$@"; fi
}
        cat > "$work/request.cnf" <<'REQUEST'
[req]
distinguished_name=subject
[subject]
[ca]
basicConstraints=critical,CA:TRUE
keyUsage=critical,keyCertSign,cRLSign
subjectKeyIdentifier=hash
REQUEST
        openssl req -config "$work/request.cnf" -extensions ca -x509 -newkey rsa:2048 -nodes \
            -keyout "$work/ca.key" -out "$work/ca.crt" \
            -sha256 -days 2 -subj '/CN=Panackelty test CA' >"$work/certificate-generation.log" 2>&1
        openssl req -config "$work/request.cnf" -new -newkey rsa:2048 -nodes \
            -keyout "$work/test.key" -out "$work/test.csr" \
            -subj '/CN=localhost' >>"$work/certificate-generation.log" 2>&1
        cat > "$work/server.ext" <<'EXTENSIONS'
subjectAltName=DNS:localhost
basicConstraints=critical,CA:FALSE
keyUsage=critical,digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
EXTENSIONS
        openssl x509 -req -in "$work/test.csr" -CA "$work/ca.crt" -CAkey "$work/ca.key" \
            -set_serial 2 -sha256 -days 2 -extfile "$work/server.ext" -out "$work/test.crt" \
            >>"$work/certificate-generation.log" 2>&1
        openssl verify -CAfile "$work/ca.crt" -purpose sslserver "$work/test.crt" \
            >"$work/certificate-policy.log" 2>&1
        if [ "$(uname -s)" = Darwin ]; then
            security verify-cert -c "$work/test.crt" -r "$work/ca.crt" \
                -p ssl -s localhost >>"$work/certificate-policy.log" 2>&1
        fi
        "$fixture" --reserve-port "$work/tls-port"
        tls_port=$(cat "$work/tls-port")
        mkdir -p "$work/tls-root"
        printf ok >"$work/tls-root/ok"
        (
            cd "$work/tls-root"
            exec openssl s_server -accept "127.0.0.1:$tls_port" -quiet -WWW \
                -cert "$repo/$work/test.crt" -key "$repo/$work/test.key" \
                >"$repo/$work/tls-log" 2>&1
        ) &
        tls_pid=$!
        ready=0
        for attempt in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
            if "$fixture" --probe "$tls_port" >/dev/null 2>&1; then ready=1; break; fi
            sleep 0.05
        done
        test "$ready" -eq 1

cat > "$work/template.panack" <<'SOURCE'
import stdlib/network/http_streamSUFFIX as stream
import stdlib/network/http_stream/errors as errors
import stdlib/network/http_stream/errors::{HttpStreamErrorKind.TlsVerificationFailed, HttpStreamErrorKind.InvalidArguments, HttpStreamErrorKind.InvalidStream, HttpStreamErrorKind.OtherFailure, HttpStreamOperation.Read}
import stdlib/time as time
import stdlib/consoleSUFFIX as console
import stdlib/http_stream as legacy
pure expect(ok: Bool): Unit { check = [0][if ok { 0 } else { 1 }]; () }
pure require[T,E](r: Result[T,E]): T { match r { Ok(v) => v, Error(_) => { empty: [T] = []; empty[0] } } }
pure failure[T,E](r: Result[T,E]): E { match r { Error(v) => v, Ok(_) => { empty: [E] = []; empty[0] } } }
EFFECT main(): Unit {
  request = "GET /ok HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n".encode_utf8()
  invalid = failure(WAIT stream.open("localhost", PORT, true, request, 4096, time.seconds(0)))
  expect(invalid.kind == InvalidArguments())
  handle = require(WAIT stream.open("HOST", PORT, true, request, 4096, time.seconds(3)))
  problem = failure(WAIT stream.read(handle))
  expect(problem.kind == TlsVerificationFailed())
  expect(problem.operation == Read())
  expect(problem.native.starts_with_at("HTTP TLS verification failed\nlibcurl error ", 0))
  expect(problem.native.len() > 60)
  expect(problem.render().starts_with_at("Cannot read the HTTP stream:",0))
  written = require(WAIT console.write_line(problem.native))
  closed = require(WAIT stream.close(handle))
  stale = failure(WAIT stream.read(handle))
  expect(stale.kind == InvalidStream())
  unknown = failure(errors.convert[Unit](Error("future native failure"),Read()))
  expect(unknown.kind == OtherFailure() && unknown.native == "future native failure")
  ()
}
SOURCE
for trust in wrong-host untrusted; do
  if [ "$trust" = wrong-host ]; then target=127.0.0.1; export CURL_CA_BUNDLE="$repo/$work/ca.crt"
  else target=localhost; unset CURL_CA_BUNDLE; fi
  for mode in ordinary async; do
    if [ "$mode" = async ]; then suffix=/async; effect=async; wait=await; else suffix=; effect=; wait=; fi
    sed -e "s@SUFFIX@$suffix@g" -e "s/EFFECT/$effect/g" -e "s/WAIT/$wait/g" -e "s/PORT/$tls_port/g" -e "s/HOST/$target/g" "$work/template.panack" > "$work/main.panack"
    panack run "$work/main.panack" > "$work/$trust-$mode.out"
    panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
    panack run "$work/main.bc" > "$work/saved.out"
    cmp "$work/$trust-$mode.out" "$work/saved.out"
  done
  cmp "$work/$trust-ordinary.out" "$work/$trust-async.out"
done
# A stable category alone cannot establish detailed TLS preservation.
if cmp -s "$work/wrong-host-ordinary.out" "$work/untrusted-ordinary.out"; then
  echo 'distinct TLS causes lost their details' >&2; exit 1
fi
grep -Ei 'subject|host.?name|IP address|alternative' "$work/wrong-host-ordinary.out" > /dev/null
grep -Ei 'issuer|trust|self.signed|certificate chain' "$work/untrusted-ordinary.out" > /dev/null

export CURL_CA_BUNDLE="$repo/$work/ca.crt"
cat > "$work/trusted.panack" <<'SOURCE'
import stdlib/network/http_streamSUFFIX as stream
import stdlib/time as time
pure require[T,E](r: Result[T,E]): T { match r { Ok(v) => v, Error(_) => { empty: [T] = []; empty[0] } } }
EFFECT main(): Unit {
  request = "GET /ok HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n".encode_utf8()
  handle = require(WAIT stream.open("localhost", PORT, true, request, 4096, time.seconds(3)))
  bytes = require(WAIT stream.read(handle))
  check = [0][if bytes.length() > 0 { 0 } else { 1 }]
  closed = require(WAIT stream.close(handle))
  ()
}
SOURCE
for mode in ordinary async; do
  if [ "$mode" = async ]; then suffix=/async; effect=async; wait=await; else suffix=; effect=; wait=; fi
  sed -e "s@SUFFIX@$suffix@g" -e "s/EFFECT/$effect/g" -e "s/WAIT/$wait/g" -e "s/PORT/$tls_port/g" "$work/trusted.panack" > "$work/main.panack"
  panack run "$work/main.panack"
  panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
  panack run "$work/main.bc"
done
printf '%s\n' 'HTTP errors: ordinary/async source/bytecode preserve distinct real TLS causes, lifecycle, context and unknown details.'
