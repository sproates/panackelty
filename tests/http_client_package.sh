#!/bin/sh
set -eu
panack_bin=${PANACK_BIN:-./bin/panack}
fixture=${PANACK_HTTP_FIXTURE:-build/vm/http_fixture}
repo=$(pwd)
if [ "$#" -eq 0 ]; then
    for case_name in ok missing chunked close truncated duplicate oversize bad-url ipv6 timeout connect-failure tls-ok tls-wrong-host tls-untrusted tls-late stream tls-stream tls-stream-untrusted body-stream body-chunked body-chunk-limit body-close body-truncated body-duplicate body-oversize dns-failure dns-late installed; do
        echo "HTTP client fixture: $case_name"
        sh "$0" "$case_name"
    done
    exit 0
fi
selected_case=$1

work=$(mktemp -d tests/http-client.XXXXXX)
http_pid=
tls_pid=
tls_proxy_pid=
cleanup() {
    result=$?
    if [ "$result" -ne 0 ]; then
        echo "HTTP client fixture failed: $selected_case (exit $result)" >&2
        for diagnostic in certificate-generation.log certificate-policy.log tls-log http-log; do
            if [ -f "$work/$diagnostic" ]; then
                echo "--- $diagnostic ---" >&2
                cat "$work/$diagnostic" >&2
            fi
        done
    fi
    if [ -n "$http_pid" ]; then kill "$http_pid" 2>/dev/null || :; wait "$http_pid" 2>/dev/null || :; fi
    if [ -n "$tls_proxy_pid" ]; then kill "$tls_proxy_pid" 2>/dev/null || :; wait "$tls_proxy_pid" 2>/dev/null || :; fi
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
refused_port=0
if [ "$selected_case" = connect-failure ]; then
    "$fixture" --reserve-port "$work/refused-port"
    refused_port=$(cat "$work/refused-port")
fi

tls_port=0
case "$selected_case" in
    tls-*)
        command -v openssl >/dev/null 2>&1 || {
            echo 'HTTP client TLS fixture requires openssl on PATH' >&2
            exit 1
        }
        # Use a private CA and a distinct server leaf. A CA certificate serving
        # as its own TLS leaf is not accepted consistently by host trust engines.
        # Do not inherit host req extensions: OpenSSL 1.1.1 appends -addext
        # to configured extensions, producing duplicate basicConstraints.
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
        ;;
esac

tls_proxy_port=$tls_port
if [ "$selected_case" = tls-late ]; then
    "$fixture" --delay-tls-flight "$work/tls-proxy-port" "$tls_port" 400 "$work/tls-proxy-log" \
        >"$work/tls-proxy-output" 2>&1 &
    tls_proxy_pid=$!
    for attempt in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
        [ -s "$work/tls-proxy-port" ] && break
        sleep 0.05
    done
    test -s "$work/tls-proxy-port"
    tls_proxy_port=$(cat "$work/tls-proxy-port")
fi

cat > "$app/src/main.panack" <<SOURCE
import http/client as http
import http/client::{HttpError.ConnectFailure, HttpError.DnsFailure, HttpError.InvalidUrl, HttpError.LimitExceeded, HttpError.ProtocolError,
  HttpError.TlsFailure, HttpError.Timeout, HttpError.UnsupportedHost}

pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}

async main(): Unit {
  selected: Str = "$selected_case"
  if selected == "ok" || selected == "installed" {
    result = await http.get("http://127.0.0.1:$http_port/ok")
    expect(match result {
      Ok(response) => response.status == 200 && response.body == "ok".encode_utf8() &&
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
      Ok(response) => response.body == "missing".encode_utf8(),
      Error(error) => false
    })
  } else if selected == "chunked" {
    result = await http.get("http://127.0.0.1:$http_port/chunked")
    expect(match result {
      Ok(response) => response.status == 200 && response.body == "hello".encode_utf8() &&
        response.headers[0].name == "Transfer-Encoding" && response.headers[1].value == "one" &&
        response.headers[2].value == "two",
      Error(error) => false
    })
  } else if selected == "close" {
    result = await http.get("http://127.0.0.1:$http_port/close")
    expect(match result {
      Ok(response) => response.body == "close body".encode_utf8(),
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
      Ok(response) => response.status == 200 && response.body == "ok".encode_utf8(),
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
  } else if selected == "tls-late" {
    result = await http.get_with_limits("https://localhost:$tls_proxy_port/ok",
      http.ClientLimits(100, 65536, 1024))
    expect(match result {
      Error(error) => error == Timeout(),
      Ok(response) => false
    })
  } else if selected == "dns-late" {
    result = await http.get_with_limits("http://p5-late-dns.panackelty.invalid/",
      http.ClientLimits(100, 65536, 1024))
    expect(match result {
      Error(error) => error == Timeout(),
      Ok(response) => false
    })
    late = await http.get("http://127.0.0.1:$http_port/delay")
    expect(match late {
      Ok(response) => response.status == 200 && response.body == "ok".encode_utf8(),
      Error(error) => false
    })
  } else if selected == "dns-failure" {
    result = await http.get_with_limits("http://p5-unknown-dns.panackelty.invalid/",
      http.ClientLimits(3000, 65536, 1024))
    expect(match result {
      Error(error) => error == DnsFailure(),
      Ok(response) => false
    })
  } else if selected == "connect-failure" {
    result = await http.get("http://127.0.0.1:$refused_port/")
    expect(match result {
      Error(error) => error == ConnectFailure(),
      Ok(response) => false
    })
  }
  ()
}
SOURCE

case "$selected_case" in
    body-*)
        body_path=/ok; body_expected=ok; body_failure=false; body_limit=1048576
        case "$selected_case" in
            body-chunked) body_path=/chunked; body_expected=hello ;;
            body-chunk-limit) body_path=/chunked; body_limit=3; body_failure=true ;;
            body-close) body_path=/close; body_expected='close body' ;;
            body-truncated) body_path=/truncated; body_failure=true ;;
            body-duplicate) body_path=/duplicate; body_failure=true ;;
            body-oversize) body_path=/oversize; body_limit=3; body_failure=true ;;
        esac
        cat > "$app/src/main.panack" <<SOURCE
import http/stream as http
import stdlib/bytes as byte_buffer
pure expect(value: Bool): Unit { checked = [0][if value { 0 } else { 1 }]; () }
pure collect(state: Bytes, chunk: Bytes): Result[Bytes,http.HttpError] { Ok(state.join(chunk)) }
main(): Void {
  mut failed: Bool = false
  match http.open_response("http://127.0.0.1:$http_port$body_path", 10000, $body_limit) {
    Error(problem) => { failed = true; () },
    Ok(response) => {
      expect(response.status == 200)
      match http.fold_body(response, byte_buffer.empty(), @collect, $body_limit) {
        Error(problem) => { failed = true; () },
        Ok(data) => { expect(data == "$body_expected".encode_utf8()) }
      }
      expect(match http.close_response(response) { Ok(unit) => true, Error(problem) => false })
    }
  }
  expect(failed == $body_failure)
  print("stream passed")
}
SOURCE
        ;;
    stream|tls-stream|tls-stream-untrusted)
        stream_host=127.0.0.1; stream_port=$http_port; stream_tls=false; stream_path=/large
        stream_minimum=33554432; stream_failure=false
        if [ "$selected_case" != stream ]; then
            stream_host=localhost; stream_port=$tls_port; stream_tls=true; stream_path=/ok
            stream_minimum=2
        fi
        if [ "$selected_case" = tls-stream-untrusted ]; then stream_failure=true; fi
        cat > "$app/src/main.panack" <<SOURCE
import stdlib/http_stream as stream
pure expect(value: Bool): Unit { checked = [0][if value { 0 } else { 1 }]; () }
main(): Void {
  request = "GET $stream_path HTTP/1.1\r\nHost: $stream_host\r\nConnection: close\r\n\r\n".encode_utf8()
  match stream.open("$stream_host", $stream_port, $stream_tls, request, 67174400, 10000) {
    Error(problem) => { expect(false) },
    Ok(handle) => {
      mut total: Nat = 0
      mut done: Bool = false
      mut failed: Bool = false
      while !done {
        match stream.read(handle) {
          Error(problem) => { failed = true; done = true },
          Ok(chunk) => {
            expect(chunk.length() <= 16384)
            total = total + chunk.length()
            done = chunk.length() == 0
          }
        }
      }
      expect(failed == $stream_failure)
      if !failed { expect(total > $stream_minimum) }
      expect(match stream.close(handle) { Ok(unit) => true, Error(problem) => false })
      expect(match stream.read(handle) { Error(problem) => true, Ok(chunk) => false })
    }
  }
  print("stream passed")
}
SOURCE
        ;;
esac

"$panack_bin" deps lock --project "$app/panack.json" >/dev/null
"$panack_bin" check --project "$app/panack.json" >/dev/null
"$panack_bin" compile --project "$app/panack.json" -o "$work/client.bc" >/dev/null
case "$selected_case" in
    dns-failure|dns-late)
    cc=${CC:-cc}
    case $(uname -s) in
        Darwin)
            "$cc" -std=c11 -Wall -Wextra -Werror -dynamiclib \
                tests/http_dns_delay.c -o "$work/http_dns_delay.dylib"
            preload_name=DYLD_INSERT_LIBRARIES
            preload_path="$(pwd -P)/$work/http_dns_delay.dylib"
            ;;
        *)
            "$cc" -std=c11 -Wall -Wextra -Werror -fPIC -shared \
                tests/http_dns_delay.c -o "$work/http_dns_delay.so" -ldl
            preload_name=LD_PRELOAD
            preload_path="$(pwd -P)/$work/http_dns_delay.so"
            ;;
    esac
    # Preload must target the VM directly on macOS. Resolve through the package
    # tool first, then pass its ordinary descriptor to the source compiler.
    PANACKELTY_STDLIB_PATH="$(pwd -P)/src/stdlib" PANACKELTY_VERSION="$version" \
        ./panack-vm run build/tools/panack-packages.bc prepare --project "$app/panack.json" \
        > "$work/resolved.json"
    for mode in source bytecode; do
        rm -f "$work/dns-started" "$work/dns-finished" "$work/dns-failure"
        if [ "$mode" = source ]; then
            set -- run "$(pwd -P)/bootstrap/compiler-v9.bc" run --resolved-project "$work/resolved.json"
        else
            set -- run "$work/client.bc"
        fi
        if [ "$preload_name" = DYLD_INSERT_LIBRARIES ]; then
            DYLD_INSERT_LIBRARIES="$preload_path" \
                PANACK_HTTP_DNS_DELAY_STARTED="$work/dns-started" \
                PANACK_HTTP_DNS_DELAY_FINISHED="$work/dns-finished" \
                PANACK_HTTP_DNS_FAILURE="$work/dns-failure" \
                PANACKELTY_BOOTSTRAP_ROOT="$(pwd -P)" PANACKELTY_STDLIB_PATH="$(pwd -P)/src/stdlib" \
                PANACKELTY_VERSION="$version" PANACKELTY_COMPILER_PATH="$(pwd -P)/bootstrap/compiler-v9.bc" \
                ./panack-vm "$@"
        else
            LD_PRELOAD="$preload_path" \
                PANACK_HTTP_DNS_DELAY_STARTED="$work/dns-started" \
                PANACK_HTTP_DNS_DELAY_FINISHED="$work/dns-finished" \
                PANACK_HTTP_DNS_FAILURE="$work/dns-failure" \
                PANACKELTY_BOOTSTRAP_ROOT="$(pwd -P)" PANACKELTY_STDLIB_PATH="$(pwd -P)/src/stdlib" \
                PANACKELTY_VERSION="$version" PANACKELTY_COMPILER_PATH="$(pwd -P)/bootstrap/compiler-v9.bc" \
                ./panack-vm "$@"
        fi
        if [ "$selected_case" = dns-late ]; then
            test -f "$work/dns-started"
            test -f "$work/dns-finished"
        else
            test -f "$work/dns-failure"
        fi
    done
    ;;
esac
case "$selected_case" in
    connect-failure|dns-failure|dns-late) ;;
    stream|tls-stream|tls-stream-untrusted|body-*)
        if [ "$selected_case" = tls-stream-untrusted ]; then
            env -u CURL_CA_BUNDLE "$panack_bin" run --project "$app/panack.json" > "$work/source.stdout"
            env -u CURL_CA_BUNDLE "$panack_bin" run "$work/client.bc" > "$work/bytecode.stdout"
        else
            env CURL_CA_BUNDLE="$work/ca.crt" "$panack_bin" run --project "$app/panack.json" > "$work/source.stdout"
            env CURL_CA_BUNDLE="$work/ca.crt" "$panack_bin" run "$work/client.bc" > "$work/bytecode.stdout"
        fi
        printf 'stream passed\n' > "$work/expected.stdout"
        cmp "$work/expected.stdout" "$work/source.stdout"
        cmp "$work/expected.stdout" "$work/bytecode.stdout"
        ;;
    tls-untrusted)
        env -u CURL_CA_BUNDLE "$panack_bin" run --project "$app/panack.json"
        env -u CURL_CA_BUNDLE "$panack_bin" run "$work/client.bc"
        ;;
    *)
        env CURL_CA_BUNDLE="$work/ca.crt" "$panack_bin" run --project "$app/panack.json"
        env CURL_CA_BUNDLE="$work/ca.crt" "$panack_bin" run "$work/client.bc"
        ;;
esac
if [ "$selected_case" = tls-late ]; then
    wait "$tls_proxy_pid"
    tls_proxy_pid=
    test "$(grep -c '^late-server-flight-attempted;' "$work/tls-proxy-log")" -eq 2
fi
