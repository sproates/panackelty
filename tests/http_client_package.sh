#!/bin/sh
set -eu
panack_bin=${PANACK_BIN:-./panack}
fixture=${PANACK_HTTP_FIXTURE:-build/vm/http_fixture}
repo=$(pwd)
if [ "$#" -eq 0 ]; then
    for case_name in ok missing chunked close truncated duplicate oversize bad-url ipv6 timeout tls-ok tls-wrong-host tls-untrusted installed; do
        echo "HTTP client fixture: $case_name"
        sh "$0" "$case_name"
    done
    exit 0
fi
selected_case=$1

work=$(mktemp -d tests/http-client.XXXXXX)
http_pid=
tls_pid=
cleanup() {
    if [ -n "$http_pid" ]; then kill "$http_pid" 2>/dev/null || :; wait "$http_pid" 2>/dev/null || :; fi
    if [ -n "$tls_pid" ]; then kill "$tls_pid" 2>/dev/null || :; wait "$tls_pid" 2>/dev/null || :; fi
    rm -rf "$work"
}
trap cleanup EXIT HUP INT TERM
app="$work/app"
if [ "$selected_case" = installed ]; then
    make install DESTDIR="$work/install" PREFIX=/usr/local >/dev/null
    panack_bin="$work/install/usr/local/bin/panack"
    app="$work/install/usr/local/share/panackelty/http-client-test"
fi
mkdir -p "$app/src"
version=$(sed -n '1p' VERSION)
cat > "$app/panack.json" <<MANIFEST
{"schema":1,"package":"tests.http_client_app","version":"0.1.0","language":"modules-v1","toolchain":"$version","source":"src","entry":"main.panack","exports":{},"dependencies":{"http":{"path":"../../../packages/http"}}}
MANIFEST

"$fixture" "$work/http-port" >"$work/http-log" 2>&1 &
http_pid=$!
for attempt in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
    [ -s "$work/http-port" ] && break
    sleep 0.05
done
test -s "$work/http-port"
http_port=$(cat "$work/http-port")

openssl req -x509 -newkey rsa:2048 -nodes -keyout "$work/test.key" -out "$work/test.crt" \
    -sha256 -days 2 -subj '/CN=localhost' \
    -addext 'subjectAltName=DNS:localhost' \
    -addext 'basicConstraints=critical,CA:TRUE' \
    -addext 'keyUsage=critical,digitalSignature,keyCertSign,cRLSign' >/dev/null 2>&1
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

cat > "$app/src/main.panack" <<SOURCE
import http/client as http
import http/client::{HttpError.InvalidUrl, HttpError.LimitExceeded, HttpError.ProtocolError,
  HttpError.TlsFailure, HttpError.Timeout, HttpError.UnsupportedHost}
import stdlib/text as text

pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}

async main(): Unit {
  selected: Str = "$selected_case"
  if selected == "ok" || selected == "installed" {
    result = await http.get("http://127.0.0.1:$http_port/ok")
    expect(match result {
      Ok(response) => response.status == 200 && response.body == text.encode_utf8("ok") &&
        response.headers.len() == 4 && response.headers[1].name == "X-Repeat" &&
        response.headers[1].value == "one" && response.headers[2].value == "two",
      Error(error) => false
    })
  } else if selected == "missing" {
    result = await http.get("http://127.0.0.1:$http_port/missing")
    expect(match result {
      Ok(response) => response.status == 404,
      Error(error) => false
    })
    expect(match result {
      Ok(response) => response.body == text.encode_utf8("missing"),
      Error(error) => false
    })
  } else if selected == "chunked" {
    result = await http.get("http://127.0.0.1:$http_port/chunked")
    expect(match result {
      Ok(response) => response.status == 200 && response.body == text.encode_utf8("hello") &&
        response.headers[0].name == "Transfer-Encoding" && response.headers[1].value == "one" &&
        response.headers[2].value == "two",
      Error(error) => false
    })
  } else if selected == "close" {
    result = await http.get("http://127.0.0.1:$http_port/close")
    expect(match result {
      Ok(response) => response.body == text.encode_utf8("close body"),
      Error(error) => false
    })
  } else if selected == "truncated" {
    result = await http.get("http://127.0.0.1:$http_port/truncated")
    expect(match result {
      Error(error) => error == ProtocolError("truncated HTTP response body"),
      Ok(response) => false
    })
  } else if selected == "duplicate" {
    result = await http.get("http://127.0.0.1:$http_port/duplicate")
    expect(match result {
      Error(error) => error == ProtocolError("duplicate Content-Length"),
      Ok(response) => false
    })
  } else if selected == "oversize" {
    result = await http.get_with_limits("http://127.0.0.1:$http_port/oversize",
      http.ClientLimits(3000, 65536, 3))
    expect(match result {
      Error(error) => error == LimitExceeded("response body"),
      Ok(response) => false
    })
  } else if selected == "bad-url" {
    result = await http.get("http://user@localhost:$http_port/")
    expect(match result {
      Error(error) => error == InvalidUrl("HTTP URL authority is empty or contains userinfo"),
      Ok(response) => false
    })
  } else if selected == "ipv6" {
    result = await http.get("http://[::1]:$http_port/")
    expect(match result {
      Error(error) => error == UnsupportedHost(),
      Ok(response) => false
    })
  } else if selected == "timeout" {
    result = await http.get_with_limits("http://127.0.0.1:$http_port/delay",
      http.ClientLimits(100, 65536, 1024))
    expect(match result {
      Error(error) => error == Timeout(),
      Ok(response) => false
    })
  } else if selected == "tls-ok" {
    result = await http.get("https://localhost:$tls_port/ok")
    expect(match result {
      Ok(response) => response.status == 200 && response.body == text.encode_utf8("ok"),
      Error(error) => false
    })
  } else if selected == "tls-wrong-host" {
    result = await http.get("https://127.0.0.1:$tls_port/ok")
    expect(match result {
      Error(error) => error == TlsFailure(),
      Ok(response) => false
    })
  } else if selected == "tls-untrusted" {
    result = await http.get("https://localhost:$tls_port/ok")
    expect(match result {
      Error(error) => error == TlsFailure(),
      Ok(response) => false
    })
  }
  ()
}
SOURCE

"$panack_bin" check --project "$app/panack.json" >/dev/null
"$panack_bin" compile --project "$app/panack.json" -o "$work/client.bc" >/dev/null
if [ "$selected_case" = tls-untrusted ]; then
    env -u CURL_CA_BUNDLE "$panack_bin" run --project "$app/panack.json"
    env -u CURL_CA_BUNDLE "$panack_bin" run "$work/client.bc"
else
    env CURL_CA_BUNDLE="$work/test.crt" "$panack_bin" run --project "$app/panack.json"
    env CURL_CA_BUNDLE="$work/test.crt" "$panack_bin" run "$work/client.bc"
fi
